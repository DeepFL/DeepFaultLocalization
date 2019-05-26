/*==========================================================================
 * Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

//
// IndexWriter
//
// 18 November 2004 -- tds
//
// Writes out a single index from one or more indexes.
//

#include "indri/IndexWriter.hpp"
#include "indri/Index.hpp"
#include "indri/DocListFileIterator.hpp"
#include <vector>
#include <queue>
#include "indri/greedy_vector"
#include "indri/Path.hpp"
#include "indri/Parameters.hpp"
#include "indri/DiskTermData.hpp"
#include "indri/TermBitmap.hpp"
#include "indri/DiskDocListIterator.hpp"
#include "indri/DiskIndex.hpp"
#include "indri/DocumentDataIterator.hpp"
#include "indri/MemoryIndex.hpp"
#include "indri/BulkTree.hpp"
#include "indri/DeletedDocumentList.hpp"

#include "indri/IndriTimer.hpp"
const int KEYFILE_MEMORY_SIZE = 128*1024;
const int OUTPUT_BUFFER_SIZE = 512*1024;

using namespace indri::index;

#ifdef LOGGING
indri::utility::IndriTimer g_t;
#define LOGSTART  { g_t.start(); }
#define LOGMESSAGE(x)  { g_t.printElapsedMicroseconds(std::cout); std::cout << ": " << x << std::endl; }
#else
#define LOGMESSAGE(x) assert(1)
#define LOGSTART assert(1)
#endif

//
// IndexWriter constructor
//

IndexWriter::IndexWriter()
{
}

//
// _writeSkip
//

void IndexWriter::_writeSkip( indri::file::SequentialWriteBuffer* buffer, lemur::api::DOCID_T document, int length ) {
  buffer->write( &document, sizeof(lemur::api::DOCID_T) );
  buffer->write( &length, sizeof(int) );
}

//
// _writeBatch
//

void IndexWriter::_writeBatch( indri::file::SequentialWriteBuffer* buffer, lemur::api::DOCID_T document, int length, indri::utility::Buffer& data ) {
  assert( length < 100*1000*1000 );
  _writeSkip( buffer, document, length );
  if( data.position() != 0 ) {
    buffer->write( data.front(), data.position() );
    data.clear();
  }
}

//
// _writeManifest
//

void IndexWriter::_writeManifest( const std::string& path ) {
  indri::api::Parameters manifest;

  manifest.set( "type", "DiskIndex" );
  manifest.set( "code-build-date", __DATE__ );
  manifest.set( "indri-distribution", INDRI_DISTRIBUTION );

  manifest.set( "corpus", "" );
  indri::api::Parameters corpus = manifest["corpus"];

  corpus.set("total-documents", (UINT64) _corpus.totalDocuments);
  corpus.set("total-terms", (UINT64) _corpus.totalTerms);
  corpus.set("unique-terms", (UINT64) _corpus.uniqueTerms);
  corpus.set("document-base", _documentBase);
  corpus.set("frequent-terms", _topTermsCount);
  corpus.set("maximum-document", _corpus.maximumDocument);

  manifest.set( "fields", "" );
  indri::api::Parameters fields = manifest["fields"];

  for( size_t i=0; i<_fields.size(); i++ ) {
    fields.append("field");
    indri::api::Parameters field = fields["field"];

    field[i].set("isNumeric", _fields[i].numeric);
    field[i].set("isOrdinal", _fields[i].ordinal);
    field[i].set("isParental", _fields[i].parental);
    field[i].set("name", _fields[i].name);
    if (_fields[i].numeric) field[i].set("parserName", _fields[i].parserName);
    field[i].set("total-documents", (UINT64) _fieldData[i].documentCount);
    field[i].set("total-terms", (UINT64) _fieldData[i].totalCount);
    field[i].set("byte-offset", (UINT64) _fieldData[i].byteOffset);
  }

  manifest.writeFile( path );
}

//
// _constructFiles
//

void IndexWriter::_constructFiles( const std::string& path ) {
  indri::file::Path::create( path );

  std::string frequentStringPath = indri::file::Path::combine( path, "frequentString" );
  std::string infrequentStringPath = indri::file::Path::combine( path, "infrequentString" );
  std::string frequentIDPath = indri::file::Path::combine( path, "frequentID" );
  std::string infrequentIDPath = indri::file::Path::combine( path, "infrequentID" );
  std::string frequentTermsDataPath = indri::file::Path::combine( path, "frequentTerms" );
  std::string documentLengthsPath = indri::file::Path::combine( path, "documentLengths" );
  std::string documentStatisticsPath = indri::file::Path::combine( path, "documentStatistics" );
  std::string invertedFilePath = indri::file::Path::combine( path, "invertedFile" );
  std::string directFilePath = indri::file::Path::combine( path, "directFile" );
  std::string fieldsFilePath = indri::file::Path::combine( path, "fieldsFile" );

  // infrequent stuff
  _infrequentTerms.idMap = new indri::file::BulkTreeWriter();
  _infrequentTerms.idMap->create( infrequentIDPath );
  _infrequentTerms.stringMap = new indri::file::BulkTreeWriter();
  _infrequentTerms.stringMap->create( infrequentStringPath );

  // frequent stuff
  _frequentTerms.idMap = new indri::file::BulkTreeWriter();
  _frequentTerms.idMap->create( frequentIDPath );
  _frequentTerms.stringMap = new indri::file::BulkTreeWriter();
  _frequentTerms.stringMap->create( frequentStringPath );
  _frequentTermsData.create( frequentTermsDataPath );

  // stats, inverted file, direct file
  _documentStatistics.create( documentStatisticsPath );
  _documentLengths.create( documentLengthsPath );
  _invertedFile.create( invertedFilePath );
  _directFile.create( directFilePath );
  _fieldsFile.create( fieldsFilePath );

  _invertedOutput = new indri::file::SequentialWriteBuffer( _invertedFile, OUTPUT_BUFFER_SIZE );
}

//
// _closeFiles
//

void IndexWriter::_closeFiles( const std::string& path ) {
  std::string manifestPath = indri::file::Path::combine( path, "manifest" );

  _infrequentTermsReader.close();
  _frequentTermsReader.close();

  // delete infrequent
  delete _infrequentTerms.idMap;
  delete _infrequentTerms.stringMap;

  // delete frequent
  delete _frequentTerms.idMap;
  delete _frequentTerms.stringMap;
  _frequentTermsData.close();

  // close stats
  _documentStatistics.close();
  _documentLengths.close();
  _invertedFile.close();
  _directFile.close();
  _fieldsFile.close();

  // write a manifest file
  _writeManifest( manifestPath );
}

//
// _openTermsReaders
//

void IndexWriter::_openTermsReaders( const std::string& path ) {
  std::string frequentStringPath = indri::file::Path::combine( path, "frequentString" );
  std::string infrequentStringPath = indri::file::Path::combine( path, "infrequentString" );

  _infrequentTermsReader.openRead( infrequentStringPath );
  _frequentTermsReader.openRead( frequentStringPath );
}

//
// write
//

void IndexWriter::write( indri::index::Index& index,
                         std::vector<indri::index::Index::FieldDescription>& fields,
                         indri::index::DeletedDocumentList& deletedList,
                         const std::string& path ) {
  std::vector< indri::index::Index* > indexes;
  indexes.push_back( &index );
  write( indexes, fields, deletedList, path );
}

//
// write
//

void IndexWriter::write( std::vector<Index*>& indexes,
                         std::vector<indri::index::Index::FieldDescription>& fields,
                         indri::index::DeletedDocumentList& deletedList,
                         const std::string& path ) {
  _fields = fields;
  _dataSize = ::disktermdata_size((int)_fields.size());
  _compressedData = new char [_dataSize];
  _uncompressedData = new char [_dataSize];

  _constructFiles( path );

  std::vector<WriterIndexContext*> contexts;

  LOGSTART;
  LOGMESSAGE( "Starting write" );
  _buildIndexContexts( contexts, indexes, deletedList );
  LOGMESSAGE( "Writing Inverted Lists" );
  _writeInvertedLists( contexts );
  LOGMESSAGE( "Inverted Lists Complete" );
  _writeFieldLists( contexts, path );
  LOGMESSAGE( "Fields Complete" );

  _openTermsReaders( path );

  _writeDirectLists( contexts );
  LOGMESSAGE( "Direct Lists Complete" );

  delete[](_compressedData);
  delete[](_uncompressedData);

  indri::utility::delete_vector_contents( contexts );
  _closeFiles( path );
}

//
// write
//

void IndexWriter::write( std::vector<indri::index::Index*>& indexes,
                         std::vector<indri::index::Index::FieldDescription>& fields,
                         std::vector<indri::index::DeletedDocumentList*>& deletedLists, 
                         const std::vector<lemur::api::DOCID_T>& documentMaximums,
                         const std::string& path ) {
  _fields = fields;
  _dataSize = ::disktermdata_size((int)_fields.size());
  _compressedData = new char [_dataSize];
  _uncompressedData = new char [_dataSize];

  _constructFiles( path );

  std::vector<WriterIndexContext*> contexts;

  LOGSTART;
  LOGMESSAGE( "Starting write" );
  _buildIndexContexts( contexts, indexes, deletedLists, documentMaximums );
  LOGMESSAGE( "Writing Inverted Lists" );
  _writeInvertedLists( contexts );
  LOGMESSAGE( "Inverted Lists Complete" );
  _writeFieldLists( contexts, path );
  LOGMESSAGE( "Fields Complete" );

  _openTermsReaders( path );
  _writeDirectLists( contexts );
  LOGMESSAGE( "Direct Lists Complete" );

  delete[](_compressedData);
  delete[](_uncompressedData);

  indri::utility::delete_vector_contents( contexts );
  _closeFiles( path );
}

//
// _buildIndexContexts
//

void IndexWriter::_buildIndexContexts( std::vector<WriterIndexContext*>& contexts, std::vector<indri::index::Index*>& indexes, indri::index::DeletedDocumentList& deletedList ) {
  for( size_t i=0; i<indexes.size(); i++ )
    contexts.push_back( new WriterIndexContext( indexes[i], &deletedList, 0 ) );
}

//
// _buildIndexContexts
//

void IndexWriter::_buildIndexContexts( std::vector<WriterIndexContext*>& contexts, std::vector<indri::index::Index*>& indexes, std::vector<indri::index::DeletedDocumentList*>& deletedLists, const std::vector<lemur::api::DOCID_T>& documentMaximums ) {
  assert( indexes.size() == deletedLists.size() );
  assert( indexes.size() == documentMaximums.size() );
  lemur::api::DOCID_T documentOffset = 0;
  
  for( size_t i=0; i<indexes.size(); i++ ) {
    contexts.push_back( new WriterIndexContext( indexes[i], deletedLists[i], documentOffset ) );
    documentOffset += (documentMaximums[i] - 1);
  }
}

//
// _writeFieldLists
//

void IndexWriter::_writeFieldLists( std::vector<WriterIndexContext*>& contexts, const std::string& path ) {
  if( contexts.size() == 0 )
    return;

  indri::file::SequentialWriteBuffer* fieldsOutput = new indri::file::SequentialWriteBuffer( _fieldsFile, OUTPUT_BUFFER_SIZE );
  
  for( size_t field=0; field<_fields.size(); field++ ) {
    int fieldID = (int)field+1;

    std::vector<indri::index::DocExtentListIterator*> iterators;
    for( size_t i=0; i<contexts.size(); i++ )
      iterators.push_back( contexts[i]->index->fieldListIterator( fieldID ) ); 

    _fieldData.push_back( FieldStatistics( _fields[field].name, _fields[field].numeric, _fields[field].ordinal, _fields[field].parental, 0, 0, fieldsOutput->tell() ) );
    _writeFieldList( *fieldsOutput, (int)field, iterators, contexts );
  }

  fieldsOutput->flush();
  delete fieldsOutput;
}

//
// _fetchMatchingInvertedLists
//

void IndexWriter::_fetchMatchingInvertedLists( indri::utility::greedy_vector<WriterIndexContext*>& lists, invertedlist_pqueue& queue ) {
  lists.clear();

  WriterIndexContext* first = queue.top();
  lists.push_back( first );
  const char* firstTerm = first->iterator->currentEntry()->termData->term;
  queue.pop();

  while( queue.size() && !strcmp( firstTerm, queue.top()->iterator->currentEntry()->termData->term ) ) {
    lists.push_back( queue.top() );
    queue.pop();
  }
}

//
// _pushInvertedLists
//

void IndexWriter::_pushInvertedLists( indri::utility::greedy_vector<WriterIndexContext*>& lists, invertedlist_pqueue& queue ) {
  for( size_t i=0; i<lists.size(); i++ ) {
    lists[i]->iterator->nextEntry();

    if( !lists[i]->iterator->finished() )
      queue.push( lists[i] );
  }
}

//
// _writeStatistics
//

void IndexWriter::_writeStatistics( indri::utility::greedy_vector<WriterIndexContext*>& lists, indri::index::TermData* termData, UINT64& startOffset ) {
  indri::utility::greedy_vector<WriterIndexContext*>::iterator iter;
  ::termdata_clear( termData, (int)_fields.size() );

  // find out what term we're writing
  strcpy( const_cast<char*>(termData->term), lists[0]->iterator->currentEntry()->termData->term );

  for( iter = lists.begin(); iter != lists.end(); ++iter ) {
    indri::index::DocListFileIterator::DocListData* listData = (*iter)->iterator->currentEntry();
    ::termdata_merge( termData, listData->termData,  (int)_fields.size() );
  }

  _termDataBuffer.clear();
  indri::utility::RVLCompressStream stream( _termDataBuffer );

  stream << termData->term;
  ::termdata_compress( stream, termData,  (int)_fields.size() );

  startOffset = _invertedOutput->tell();

  UINT32 dataSize = (UINT32)stream.dataSize();
  _invertedOutput->write( &dataSize, sizeof(UINT32) );
  _invertedOutput->write( stream.data(), stream.dataSize() );
}

//
// _writeFieldList
//
// field list is:
//  control byte -- currently no options available here
//  document / 
//

void IndexWriter::_writeFieldList( indri::file::SequentialWriteBuffer& output,
                                   int fieldIndex,
                                   std::vector<indri::index::DocExtentListIterator*>& iterators,
                                   std::vector<WriterIndexContext*>& contexts ) {
  bool numeric = _fields[fieldIndex].numeric;
  bool ordinal = _fields[fieldIndex].ordinal;
  bool parental = _fields[fieldIndex].parental;

  // write a control byte -- numeric fields use 0x02 (DiskDocExtentListIterator)
  // ordinal fields use 0x04 (DiskDocExtentListIterator)
  UINT8 control = (numeric ? 0x02 : 0) |
                  (ordinal ? 0x04 : 0) |
                  (parental ? 0x08 : 0);
  output.write( &control, sizeof(UINT8) );

  indri::utility::Buffer dataBuffer;
  const int minimumSkip = 1<<12; //4k
  lemur::api::DOCID_T lastDocument = 0;

  int documents = 0;
  UINT64 terms = 0;

  for( size_t i=0; i<iterators.size(); i++ ) {
    DocExtentListIterator* iterator = iterators[i];
    WriterIndexContext* context = contexts[i];

    if( !iterator )
      continue;

    iterator->startIteration();
    indri::utility::RVLCompressStream stream( dataBuffer );

    while( !iterator->finished() ) {
      DocExtentListIterator::DocumentExtentData* entry = iterator->currentEntry();
      lemur::api::DOCID_T storedDocument = entry->document + context->documentOffset;

      if( context->deletedList->isDeleted( entry->document ) ) {
        iterator->nextEntry();
        continue;
      }
      
      if( dataBuffer.position() > minimumSkip ) {
        _writeBatch( &output, storedDocument, (int)dataBuffer.position(), dataBuffer );
        lastDocument = 0;
      }

      assert( storedDocument > lastDocument || lastDocument == 0 );

      // add document difference
      stream << ( storedDocument - lastDocument );
      lastDocument = storedDocument;

      // extent count
      int count = (int)entry->extents.size();
      stream << count;

      // extents and numbers
      int lastStart = 0;
      int lastOrdinal = 0;

      for( int j=0; j<count; j++ ) {
        Extent& extent = entry->extents[j];

        assert( extent.begin - lastStart >= 0 );
        assert( extent.end - extent.begin >= 0 );

        stream << (extent.begin - lastStart);
        lastStart = extent.begin;
        stream << (extent.end - extent.begin);
        terms += (extent.end - extent.begin);

        if ( ordinal ) {
          stream << (extent.ordinal - lastOrdinal);
          lastOrdinal = extent.ordinal;
        }

        if ( parental) {
          stream << extent.parent;
        }

        if( entry->numbers.size() )
          stream << entry->numbers[j];
      }

      iterator->nextEntry();
      documents++;
    }

    delete iterator;
  }

  assert( _fieldData.size() > fieldIndex );
  _fieldData[fieldIndex].documentCount = documents;
  _fieldData[fieldIndex].totalCount = terms;

  _writeBatch( &output, -1, (int)dataBuffer.position(), dataBuffer );
}

//
// _addInvertedListData
//
// Inverted list is:
//   termData (as written by _writeStatistics)
//   control byte -- hasTopdocs(0x1)
//   optional topdocs list: topdocsCount + (document/count/length)+
//   ( [skip: document/skipLength] (doc/positionCount/positions+)+ )
// (-1) signifies there's no more skips
// 
//

void IndexWriter::_addInvertedListData( indri::utility::greedy_vector<WriterIndexContext*>& lists,
                                        indri::index::TermData* termData,
                                        indri::utility::Buffer& listBuffer, 
                                        UINT64& endByteOffset ) {
  indri::utility::greedy_vector<WriterIndexContext*>::iterator iter;
  const int minimumSkip = 1<<12; // 4k
  int documentsWritten = 0;

  const float topdocsFraction = 0.01f;
  bool hasTopdocs = termData->corpus.documentCount > TOPDOCS_DOCUMENT_COUNT;
  bool isFrequent = termData->corpus.totalCount > FREQUENT_TERM_COUNT;
  int topdocsCount = hasTopdocs ? int(termData->corpus.documentCount * 0.01) : 0;
  int topdocsSpace = hasTopdocs ? (topdocsCount*(sizeof(lemur::api::DOCID_T) + (2*sizeof(UINT32))) + sizeof(int)) : 0;

  // write a control byte
  char control = (hasTopdocs ? 0x01 : 0) | (isFrequent ? 0x02 : 0);
  _invertedOutput->write( &control, 1 );

  UINT64 initialPosition = _invertedOutput->tell();

  // leave some room for the topdocs list
  if( hasTopdocs ) {
    _invertedOutput->seek( topdocsSpace + initialPosition );
  }

  // maintain a list of top documents
  std::priority_queue<DocListIterator::TopDocument,
    std::vector<DocListIterator::TopDocument>,
    DocListIterator::TopDocument::greater> topdocs;

  double threshold = 0;

  lemur::api::DOCID_T lastDocument = 0;
  int positions = 0;
  int docs = 0;

  // for each matching list:
  for( iter = lists.begin(); iter != lists.end(); ++iter ) {
    indri::index::DocListFileIterator::DocListData* listData = (*iter)->iterator->currentEntry();
    DocListIterator* iterator = listData->iterator;
    Index* index = (*iter)->index;
    indri::utility::RVLCompressStream stream( listBuffer );

    int listDocs = 0;
    int listPositions = 0;
    int deletedDocuments = 0;

    while( !iterator->finished() ) {
      // get the latest entry from the list
      DocListIterator::DocumentData* documentData = iterator->currentEntry();
      lemur::api::DOCID_T storedDocument = documentData->document + (*iter)->documentOffset;

      // check for deleted documents and skip them
      if( (*iter)->deletedList->isDeleted( documentData->document ) ) {
        // BUGBUG: this is an appropriate spot to delete term stats
        deletedDocuments++;
        iterator->nextEntry();
        continue;
      }

      // add to document counter
      docs++; listDocs++;

      // update the topdocs list
      if( hasTopdocs ) {
        int length = index->documentLength( documentData->document );
        int count = (int)documentData->positions.size();

        // compute DocListIterator::TopDocument::greater (current, top())
        // if false, no reason to insert this entry.
        // note that the test is inverted. 
        //  int(length * threshold) <= count is equivalent to
        // count/length > topdocs.top().count/topdocs.top().length
        // but we use < to force breaking a tie in favor of keeping
        // the first seen document.
        if( int(length * threshold) < count || int(topdocs.size()) < topdocsCount ) {
          // form a topdocs entry for this document
          DocListIterator::TopDocument topDocument( storedDocument,
                                                    count,
                                                    length );
          topdocs.push( topDocument );
          while( (int)topdocs.size() > topdocsCount )
            topdocs.pop();

          threshold = topdocs.top().count / double(topdocs.top().length);
        }
      }
      
      if( listBuffer.position() > minimumSkip ) {
        // time to write in a skip
        _writeBatch( _invertedOutput, storedDocument, (int)listBuffer.position(), listBuffer );

        // delta encode documents by batch
        lastDocument = 0;
      }

      assert( storedDocument > lastDocument || lastDocument == 0 );

      // write this entry out to the list
      stream << int(storedDocument - lastDocument);
      stream << (int) documentData->positions.size();
      lastDocument = storedDocument;

      int lastPosition = 0;

      for( size_t i=0; i<documentData->positions.size(); i++ ) {
        stream << (documentData->positions[i] - lastPosition);
        lastPosition = documentData->positions[i];
        positions++; listPositions++;
      }

      iterator->nextEntry();
    }
  }

  // write in the final skip info
  _writeBatch( _invertedOutput, -1, (int)listBuffer.position(), listBuffer );
  UINT64 finalPosition = _invertedOutput->tell();

  if( hasTopdocs ) {
    _invertedOutput->seek( initialPosition );
    _invertedOutput->write( &topdocsCount, sizeof(int) );
    assert( topdocs.size() == topdocsCount );

    // write these into the topdocs list in order from smallest fraction to largest fraction,
    // where fraction = c(w;D)/|D|
    while( topdocs.size() ) {
      DocListIterator::TopDocument topDocument = topdocs.top();
      _invertedOutput->write( &topDocument.document, sizeof(lemur::api::DOCID_T) );
      _invertedOutput->write( &topDocument.count, sizeof(int) );
      _invertedOutput->write( &topDocument.length, sizeof(int) );
      topdocs.pop();
    }
    
    assert( (_invertedOutput->tell() - initialPosition) == topdocsSpace );
    _invertedOutput->seek( finalPosition );
  }

  endByteOffset = finalPosition;
}

//
// _storeStringEntry
//

void IndexWriter::_storeStringEntry( IndexWriter::keyfile_pair& pair, indri::index::DiskTermData* diskTermData ) {
  // add term data to string map (storing termID)
  _termDataBuffer.clear();
  indri::utility::RVLCompressStream stringStream( _termDataBuffer );

  disktermdata_compress( stringStream, diskTermData, (int)_fields.size(), indri::index::DiskTermData::WithTermID |
                         indri::index::DiskTermData::WithOffsets );

  pair.stringMap->put( diskTermData->termData->term, stringStream.data(), stringStream.dataSize() );
}

//
// _storeIdEntry
//

void IndexWriter::_storeIdEntry( IndexWriter::keyfile_pair& pair, indri::index::DiskTermData* diskTermData ) {
  // add term data to id map (storing term string)
  _termDataBuffer.clear();
  indri::utility::RVLCompressStream idStream( _termDataBuffer );

  disktermdata_compress( idStream, diskTermData, (int)_fields.size(), indri::index::DiskTermData::WithString |
                         indri::index::DiskTermData::WithOffsets );

  pair.idMap->put( diskTermData->termID, idStream.data(), idStream.dataSize() );
}

//
// _storeTermEntry
//

void IndexWriter::_storeTermEntry( IndexWriter::keyfile_pair& pair, indri::index::DiskTermData* diskTermData ) {
  _storeIdEntry( pair, diskTermData );
  _storeStringEntry( pair, diskTermData );
}

//
// _storeFrequentTerms
//

void IndexWriter::_storeFrequentTerms() {
  // (frequent terms file structures)
  indri::file::SequentialWriteBuffer writeBuffer( _frequentTermsData, 1024*1024 );
  indri::utility::Buffer intermediateBuffer( 128*1024 );
  indri::utility::RVLCompressStream stream( intermediateBuffer );

  // sort the _topTerms vector by term count
  std::sort( _topTerms.begin(), _topTerms.end(), disktermdata_count_greater() );

  // store in the tree and in a flat file
  for( size_t i=0; i<_topTerms.size(); i++ ) {
    lemur::api::TERMID_T termID = lemur::api::TERMID_T(i)+1;
    _topTerms[i]->termID = termID;
    _storeIdEntry( _frequentTerms, _topTerms[i] );

    ::disktermdata_compress( stream,
                             _topTerms[i],
                             (int)_fields.size(),
                             indri::index::DiskTermData::WithOffsets |
                             indri::index::DiskTermData::WithString |
                             indri::index::DiskTermData::WithTermID );

    writeBuffer.write( intermediateBuffer.front(), intermediateBuffer.position() );
    intermediateBuffer.clear();
  }

  // now, sort in alpha order
  std::sort( _topTerms.begin(), _topTerms.end(), disktermdata_alpha_less() );

  // store in a string tree
  for( size_t i=0; i<_topTerms.size(); i++ ) {
    _storeStringEntry( _frequentTerms, _topTerms[i] );
  }

  for( size_t i=0; i<_topTerms.size(); i++ ) {
    disktermdata_delete( _topTerms[i] );
  }
  _topTermsCount = (int)_topTerms.size();
  _topTerms.clear();

  writeBuffer.flush();
}

//
// _storeMatchInformation
//

void IndexWriter::_storeMatchInformation( indri::utility::greedy_vector<WriterIndexContext*>& lists, int sequence, indri::index::TermData* termData, UINT64 startOffset, UINT64 endOffset ) {
  bool isFrequent = termData->corpus.totalCount > FREQUENT_TERM_COUNT;

  if( isFrequent )
    _isFrequentCount++;

  for( size_t i=0; i<lists.size(); i++ ) {
    WriterIndexContext* list = lists[i];
    indri::index::DiskDocListIterator* iterator = dynamic_cast<DiskDocListIterator*>(lists[i]->iterator->currentEntry()->iterator);
    bool isMemoryIndex = (iterator == 0);
    bool wasFrequent = (isMemoryIndex || iterator->isFrequent());

    if( !wasFrequent )
      list->wasInfrequentCount++;

    if( wasFrequent )
      list->wasFrequentCount++;

    list->sequenceCount++;

    if( !wasFrequent ) {
      if( !isFrequent ) {
        // common case--remaining infrequent
        assert( sequence - _isFrequentCount  - 1 >= 0 );
        assert( ((sequence -_isFrequentCount  - 1) + _isFrequentCount + 1) <= _corpus.uniqueTerms );
        list->bitmap->add( list->wasInfrequentCount - 1, sequence - _isFrequentCount - 1 );
      } else if( isFrequent ) {
        // becoming frequent
        list->newlyFrequent->add( list->wasInfrequentCount - 1, termData->term );
      }
    }
  }

  if( isFrequent ) {
    indri::index::DiskTermData* diskTermData = disktermdata_create( (int)_fields.size() );
    ::termdata_merge( diskTermData->termData, termData, (int)_fields.size() );
    diskTermData->startOffset = startOffset;
    diskTermData->length = endOffset - startOffset;
    strcpy( const_cast<char*>(diskTermData->termData->term), termData->term );

    _topTerms.push_back( diskTermData );
  } else {
    indri::index::DiskTermData diskTermData;

    diskTermData.termData = termData;
    diskTermData.startOffset = startOffset;
    diskTermData.length = endOffset - startOffset;
    diskTermData.termID = sequence - (int)_topTerms.size();

    _storeTermEntry( _infrequentTerms, &diskTermData );
  }
}

//
// writeInvertedLists
//

void IndexWriter::_writeInvertedLists( std::vector<WriterIndexContext*>& contexts ) {
  
  // write a combined inverted list in vocabulary order
  // in the process, create a new list of termIDs from the old list
  
  std::priority_queue<WriterIndexContext*,
    std::vector<WriterIndexContext*>,
    WriterIndexContext::greater> invertedLists;
  indri::utility::Buffer invertedListBuffer;

  UINT64 startOffset;
  UINT64 endOffset;

  // clear out the term buffer
  char term[lemur::file::Keyfile::MAX_KEY_LENGTH+1];
  term[0] = 0;

  _documentBase = contexts[0]->index->documentBase();
  _corpus.maximumDocument = 1;

  for( size_t i=0; i<contexts.size(); i++ ) {
    if( !contexts[i]->iterator->finished() )
      invertedLists.push( contexts[i] );
    _corpus.totalTerms += contexts[i]->index->termCount();
    _corpus.totalDocuments += (unsigned int)contexts[i]->index->documentCount();
    _corpus.maximumDocument = std::max(contexts[i]->index->documentMaximum(), _corpus.maximumDocument);
  }

    _corpus.maximumDocument = std::max(_corpus.maximumDocument, (lemur::api::DOCID_T)(_corpus.totalDocuments+_documentBase));

  indri::utility::greedy_vector<WriterIndexContext*> current;
  indri::index::TermData* termData = ::termdata_create( (int)_fields.size() );
  char termBuffer[lemur::file::Keyfile::MAX_KEY_LENGTH+1] = {0};
  termData->term = termBuffer;
  _isFrequentCount = 0;

  for( int sequence = 1; invertedLists.size(); sequence++ ) {
    // new term
    _corpus.uniqueTerms++;
    assert( sequence == _corpus.uniqueTerms );

    // fetch useful doc lists
    _fetchMatchingInvertedLists( current, invertedLists );

    // loop 1: merge statistics for term
    _writeStatistics( current, termData, startOffset );

    // go through lists one by one, adding data to the final invlist, adding skips, etc.
    _addInvertedListData( current, termData, invertedListBuffer, endOffset );

    // have to store the termData in a B-Tree (or something) for fast access later
    _storeMatchInformation( current, sequence, termData, startOffset, endOffset );

    // push back all doc lists with useful information
    _pushInvertedLists( current, invertedLists );
  }

  // at this point, we need to fill in all the "top" vocabulary data into the keyfile
  _storeFrequentTerms();

  _frequentTerms.idMap->close();
  _frequentTerms.stringMap->close();
  _infrequentTerms.idMap->close();
  _infrequentTerms.stringMap->close();

  ::termdata_delete( termData, (int)_fields.size() );
  _invertedOutput->flush();
  delete _invertedOutput;
  _invertedFile.close();
}

//
// _lookupTermID
//

lemur::api::TERMID_T IndexWriter::_lookupTermID( indri::file::BulkTreeReader& keyfile, const char* term ) {
  int actual;
  bool result = keyfile.get( term, _compressedData, actual, _dataSize );
  
  if( !result ) {
    return -1;
  }

  indri::utility::RVLDecompressStream stream( _compressedData, actual );
  DiskTermData* diskTermData = ::disktermdata_decompress( stream,
                                                          _uncompressedData,
                                                          (int)_fields.size(),
                                                          DiskTermData::WithTermID );

  lemur::api::TERMID_T termid = diskTermData->termID;
  return termid;
}

//
// _buildTermTranslator
//

indri::index::TermTranslator* IndexWriter::_buildTermTranslator( indri::file::BulkTreeReader& newInfrequentTerms,
                                                                 indri::file::BulkTreeReader& newFrequentTerms,
                                                                 TermRecorder& oldFrequentTermsRecorder,
                                                                 indri::utility::HashTable<lemur::api::TERMID_T, lemur::api::TERMID_T>* oldInfrequentHashTable,
                                                                 TermRecorder& newFrequentTermsRecorder,
                                                                 Index* index,
                                                                 TermBitmap* bitmap )
{
  int newTermCount = _corpus.uniqueTerms;
  int oldTermCount = (int)index->uniqueTermCount();
  int becameInfrequent = 0;
  int becameFrequent = 0;

  // 1. map frequent terms to frequent terms
  std::vector<lemur::api::TERMID_T>* frequent = new std::vector<lemur::api::TERMID_T>;

  std::vector< std::pair<const char*, lemur::api::TERMID_T> > missing;
  oldFrequentTermsRecorder.sort();
  newFrequentTermsRecorder.sort();

  // 2. map old frequent terms to new infrequent (or frequent) terms
  if( frequent->size() == 0 )
    frequent->resize(1);
  (*frequent)[0] = 0;

  std::vector< std::pair< size_t, lemur::api::TERMID_T > >& pairs = oldFrequentTermsRecorder.pairs();

  for( size_t i=0; i<pairs.size(); i++ ) {
    lemur::api::TERMID_T oldFrequentTermID = pairs[i].second;
    const char* oldFrequentTerm = oldFrequentTermsRecorder.buffer().front() + pairs[i].first;

    if( (lemur::api::TERMID_T)frequent->size() <= oldFrequentTermID )
      frequent->resize( oldFrequentTermID+1, -1 );
    
    lemur::api::TERMID_T mapping = _lookupTermID( newFrequentTerms, oldFrequentTerm );
    assert( mapping <= _isFrequentCount );

    if( mapping < 0 ) {
      mapping = _lookupTermID( newInfrequentTerms, oldFrequentTerm );
      assert( mapping > 0 );
      mapping += _isFrequentCount;
      becameInfrequent++;
    }

    assert( mapping > 0 );
    (*frequent)[oldFrequentTermID] = mapping;

    assert( oldFrequentTermID <= oldTermCount );
    assert( mapping <= newTermCount );
  }

  // 3. map old infrequent terms to new frequent terms
  std::vector< std::pair< size_t, lemur::api::TERMID_T > >& newlyFrequentPairs = newFrequentTermsRecorder.pairs();

  for( size_t i=0; i<newlyFrequentPairs.size(); i++ ) {
    // lookup newlyInfrequentTerms[i]
    const char* term = newlyFrequentPairs[i].first + newFrequentTermsRecorder.buffer().front();
    lemur::api::TERMID_T newTermID = _lookupTermID( newFrequentTerms, term );
    lemur::api::TERMID_T oldTermID = newlyFrequentPairs[i].second; 
    oldInfrequentHashTable->insert( oldTermID, newTermID );
    becameFrequent++;

    assert( oldInfrequentHashTable->find( oldTermID ) );
    assert( oldTermID <= oldTermCount );
    assert( newTermID <= newTermCount );
  }

  int oldFrequentCount = (int)oldFrequentTermsRecorder.pairs().size();
  int newFrequentCount = _isFrequentCount;

  TermTranslator* translator = new TermTranslator( oldFrequentCount,
                                                   newFrequentCount,
                                                   oldTermCount,
                                                   newTermCount,
                                                   frequent,
                                                   oldInfrequentHashTable,
                                                   bitmap );

  return translator;
}

//
// _writeDirectLists
//

void IndexWriter::_writeDirectLists( WriterIndexContext* context,
                                     indri::file::SequentialWriteBuffer* directOutput,
                                     indri::file::SequentialWriteBuffer* lengthsOutput,
                                     indri::file::SequentialWriteBuffer* dataOutput ) {
  VocabularyIterator* vocabulary = context->index->frequentVocabularyIterator();
  indri::index::Index* index = context->index;
  
  vocabulary->startIteration();

  while( !vocabulary->finished() ) {
    indri::index::DiskTermData* diskTermData = vocabulary->currentEntry();

    context->oldFrequent->add( diskTermData->termID, diskTermData->termData->term );
    vocabulary->nextEntry();
  }

  delete vocabulary;
  vocabulary = 0;

  TermListFileIterator* iterator = index->termListFileIterator();
  TermTranslator* translator = _buildTermTranslator( _infrequentTermsReader,
                                                     _frequentTermsReader,
                                                     *context->oldFrequent,
                                                     context->oldInfrequent,
                                                     *context->newlyFrequent,
                                                     index,
                                                     context->bitmap );
  iterator->startIteration();
  TermList writeList;
  indri::utility::Buffer outputBuffer( OUTPUT_BUFFER_SIZE );

  indri::index::DocumentDataIterator* dataIterator = context->index->documentDataIterator();
  dataIterator->startIteration();
  lemur::api::DOCID_T document = index->documentBase();

  while( !iterator->finished() ) {
    writeList.clear();
    TermList* list = iterator->currentEntry();
    assert( list );

    lemur::api::TERMID_T currentTerm;
    lemur::api::TERMID_T translated;
    bool deleted = context->deletedList->isDeleted( document );

    // if the document is not deleted, copy it
    if( !deleted ) {
    // copy and translate terms
    for( size_t i=0; i<list->terms().size(); i++ ) {
      currentTerm = list->terms()[i];
      assert( currentTerm >= 0 );
      assert( currentTerm <= index->uniqueTermCount() );
      translated = (*translator)( currentTerm );
      assert( translated > 0 || (translated == 0 && currentTerm == 0) );

      writeList.addTerm( translated );
    }

    // copy field data
    size_t fieldCount = list->fields().size();
    const indri::utility::greedy_vector<indri::index::FieldExtent>& fields = list->fields();

    for( size_t i=0; i<fieldCount; i++ ) {
      writeList.addField( fields[i] );
    }
    }
  
    // record the start position
    size_t writeStart = outputBuffer.position();
    UINT32 length = 0;

    // write the list, leaving room for a length count
    outputBuffer.write( sizeof(UINT32) );
    writeList.write( outputBuffer );

    // record the end position, compute length
    size_t writeEnd = outputBuffer.position();
    length = (UINT32)(writeEnd - (writeStart + sizeof(UINT32)));

    // store length
    assert( outputBuffer.position() >= (sizeof(UINT32) + length + writeStart) );
    memcpy( outputBuffer.front() + writeStart, &length, sizeof(UINT32) );
    assert( dataIterator );

    // get a copy of the document data
    assert( dataIterator );
    assert( !dataIterator->finished() );
    indri::index::DocumentData documentData = *dataIterator->currentEntry();

    // store offset information
    documentData.byteLength = length;
    documentData.offset = directOutput->tell() + writeStart + sizeof(UINT32);

    // tell has to happen before a write or the offset will be wrong.
    if( outputBuffer.position() > 128*1024 ) {
      directOutput->write( outputBuffer.front(), outputBuffer.position() );
      outputBuffer.clear();
    }

    dataOutput->write( &documentData, sizeof(DocumentData) );
    int termLength = documentData.totalLength;
    assert( termLength >= 0 );
    lengthsOutput->write( &termLength, sizeof(UINT32) );
    
    iterator->nextEntry();
    dataIterator->nextEntry();
    document++;
  }

  delete iterator;
  delete dataIterator;
  delete translator;
  directOutput->write( outputBuffer.front(), outputBuffer.position() );
  directOutput->flush();
  lengthsOutput->flush();
  outputBuffer.clear();
}

//
// _writeDirectLists
//

void IndexWriter::_writeDirectLists( std::vector<WriterIndexContext*>& contexts ) {
  std::vector<WriterIndexContext*>::iterator iter;
  indri::file::SequentialWriteBuffer* outputBuffer = new indri::file::SequentialWriteBuffer( _directFile, 1024*1024 );
  indri::file::SequentialWriteBuffer* lengthsBuffer = new indri::file::SequentialWriteBuffer( _documentLengths, 1024*1024 );
  indri::file::SequentialWriteBuffer* dataBuffer = new indri::file::SequentialWriteBuffer( _documentStatistics, 1024*1024 );

  for( iter = contexts.begin(); iter != contexts.end(); iter++ ) {
    _writeDirectLists( *iter, outputBuffer, lengthsBuffer, dataBuffer );
  }

  outputBuffer->flush();
  lengthsBuffer->flush();
  dataBuffer->flush();

  delete outputBuffer;
  delete lengthsBuffer;
  delete dataBuffer;
}

