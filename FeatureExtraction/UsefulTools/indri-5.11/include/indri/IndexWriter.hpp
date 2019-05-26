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
// 26 November 2004 -- tds
//

#ifndef INDRI_INDEXWRITER_HPP
#define INDRI_INDEXWRITER_HPP

#include <vector>
#include <utility>
#include <queue>

#include "lemur/lemur-compat.hpp"
#include "indri/indri-platform.h"
#include "indri/greedy_vector"
#include "indri/TermData.hpp"
#include "lemur/Keyfile.hpp"
#include "indri/Index.hpp"
#include "indri/DocListFileIterator.hpp"
#include "indri/File.hpp"
#include "indri/SequentialWriteBuffer.hpp"
#include "indri/CorpusStatistics.hpp"
#include "indri/FieldStatistics.hpp"
#include "indri/TermBitmap.hpp"
#include "indri/TermRecorder.hpp"
#include "indri/TermTranslator.hpp"
#include "indri/DeletedDocumentList.hpp"
#include "indri/BulkTree.hpp"

namespace indri {
  namespace index {

    struct WriterIndexContext {
      struct greater {
      private:
        indri::index::DocListFileIterator::iterator_greater _iterator_greater;
  
        int _compareTerms( const WriterIndexContext* const&  one, const WriterIndexContext* const& two ) const {
          const char* oneTerm = one->iterator->currentEntry()->termData->term;
          const char* twoTerm = two->iterator->currentEntry()->termData->term;

          return strcmp( oneTerm, twoTerm );
        }

        int _compareDocuments( const WriterIndexContext* const&  one, const WriterIndexContext* const& two ) const {
          const indri::index::DocListIterator::DocumentData* oneData = one->iterator->currentEntry()->iterator->currentEntry();
          const indri::index::DocListIterator::DocumentData* twoData = two->iterator->currentEntry()->iterator->currentEntry();

          lemur::api::DOCID_T oneDocument = oneData ? oneData->document + one->documentOffset : 0;
          lemur::api::DOCID_T twoDocument = twoData ? twoData->document + two->documentOffset : 0;

          return oneDocument > twoDocument;
        }

      public:
        bool operator () ( const WriterIndexContext* const&  one, const WriterIndexContext* const& two ) const {
          assert( !one->iterator->finished() && !two->iterator->finished() );

          int result = _compareTerms( one, two );

          // if terms don't match, we're done
          if( result != 0 )
            return result > 0;

          // terms match, so go by document
          return _compareDocuments( one, two ) > 0;
        }
      };

      WriterIndexContext( indri::index::Index* _index, indri::index::DeletedDocumentList* _deletedList, lemur::api::DOCID_T _documentOffset ) {
        deletedList = _deletedList;
        documentOffset = _documentOffset;

        bitmap = new indri::index::TermBitmap;
        index = _index;
        wasInfrequentCount = 0;
        wasFrequentCount = 0;

        if( index->iteratorLock() )
          index->iteratorLock()->lock();
    
        iterator = index->docListFileIterator();
        iterator->startIteration();

        newlyFrequent = new indri::index::TermRecorder;
        oldFrequent = new indri::index::TermRecorder;
        oldInfrequent = new indri::utility::HashTable<lemur::api::TERMID_T, lemur::api::TERMID_T>;

        // DEBUG
        sequenceCount = 0;
      }

      ~WriterIndexContext() {
        delete iterator;

        if( index->iteratorLock() )
          index->iteratorLock()->unlock();

        delete oldFrequent;
        delete newlyFrequent;
        delete oldInfrequent;
        delete bitmap;
      }

      indri::index::DocListFileIterator* iterator;
      indri::index::TermBitmap* bitmap;
      indri::index::Index* index;

      int wasFrequentCount;
      int wasInfrequentCount;
      int sequenceCount;
      indri::index::TermRecorder* newlyFrequent;
      indri::index::TermRecorder* oldFrequent;
      indri::utility::HashTable<lemur::api::TERMID_T, lemur::api::TERMID_T>* oldInfrequent;

      indri::index::DeletedDocumentList* deletedList;
      lemur::api::DOCID_T documentOffset;
    };

    typedef std::priority_queue<WriterIndexContext*,
                                std::vector<WriterIndexContext*>,
                                WriterIndexContext::greater> invertedlist_pqueue;

    class IndexWriter {
    private:
      struct disktermdata_count_greater {
        bool operator () ( const DiskTermData* one, const DiskTermData* two ) const {
          return one->termData->corpus.totalCount > two->termData->corpus.totalCount;
        }
      };

      struct disktermdata_alpha_less {
        bool operator () ( const DiskTermData* one, const DiskTermData* two ) const {
          return strcmp( one->termData->term, two->termData->term ) < 0;
        }
      };

      struct keyfile_pair {
        indri::file::BulkTreeWriter* stringMap;
        indri::file::BulkTreeWriter* idMap;
      };

      keyfile_pair _infrequentTerms;
      keyfile_pair _frequentTerms;
      indri::file::File _frequentTermsData;

      indri::file::BulkTreeReader _infrequentTermsReader;
      indri::file::BulkTreeReader _frequentTermsReader;

      indri::file::File _documentStatistics;
      indri::file::File _documentLengths;

      indri::file::File _invertedFile;
      indri::file::File _directFile;
      indri::file::File _fieldsFile;

      indri::file::SequentialWriteBuffer* _invertedOutput;

      indri::utility::greedy_vector<indri::index::DiskTermData*> _topTerms;
      int _topTermsCount;
      indri::utility::Buffer _termDataBuffer;

      int _isFrequentCount;
      lemur::api::DOCID_T _documentBase;
      indri::index::CorpusStatistics _corpus;
      std::vector<indri::index::Index::FieldDescription> _fields;
      std::vector<indri::index::FieldStatistics> _fieldData;

      void _writeManifest( const std::string& path );
      void _writeSkip( indri::file::SequentialWriteBuffer* buffer, lemur::api::DOCID_T document, int length );
      void _writeBatch( indri::file::SequentialWriteBuffer* buffer, lemur::api::DOCID_T document, int length, indri::utility::Buffer& data );

      void _writeFieldLists( std::vector<WriterIndexContext*>& contexts, const std::string& path );
      void _writeFieldList( indri::file::SequentialWriteBuffer& output, int fieldIndex, std::vector<indri::index::DocExtentListIterator*>& iterators, std::vector<WriterIndexContext*>& contexts );

      void _pushInvertedLists( indri::utility::greedy_vector<WriterIndexContext*>& lists, invertedlist_pqueue& queue );
      void _fetchMatchingInvertedLists( indri::utility::greedy_vector<WriterIndexContext*>& lists, invertedlist_pqueue& queue );
      void _writeStatistics( indri::utility::greedy_vector<WriterIndexContext*>& lists, indri::index::TermData* termData, UINT64& startOffset );
      void _writeInvertedLists( std::vector<WriterIndexContext*>& contexts );

      void _storeIdEntry( IndexWriter::keyfile_pair& pair, indri::index::DiskTermData* diskTermData );
      void _storeStringEntry( IndexWriter::keyfile_pair& pair, indri::index::DiskTermData* diskTermData );

      void _storeTermEntry( IndexWriter::keyfile_pair& pair, indri::index::DiskTermData* diskTermData );
      void _storeFrequentTerms();
      void _addInvertedListData( indri::utility::greedy_vector<WriterIndexContext*>& lists, indri::index::TermData* termData, indri::utility::Buffer& listBuffer, UINT64& endOffset );
      void _storeMatchInformation( indri::utility::greedy_vector<WriterIndexContext*>& lists, int sequence, indri::index::TermData* termData, UINT64 startOffset, UINT64 endOffset );

      lemur::api::TERMID_T _lookupTermID( indri::file::BulkTreeReader& keyfile, const char* term );

      void _buildIndexContexts( std::vector<WriterIndexContext*>& contexts, std::vector<indri::index::Index*>& indexes, indri::index::DeletedDocumentList& deletedList );
      void _buildIndexContexts( std::vector<WriterIndexContext*>& contexts, std::vector<indri::index::Index*>& indexes, std::vector<indri::index::DeletedDocumentList*>& deletedLists, const std::vector<lemur::api::DOCID_T>& documentOffsets );
      
      void _writeDirectLists( std::vector<WriterIndexContext*>& contexts );
      void _writeDirectLists( WriterIndexContext* context,
                              indri::file::SequentialWriteBuffer* directOutput,
                              indri::file::SequentialWriteBuffer* lengthsOutput,
                              indri::file::SequentialWriteBuffer* dataOutput );

      void _constructFiles( const std::string& path );
      void _closeFiles( const std::string& path );
      void _openTermsReaders( const std::string& path );

      indri::index::TermTranslator* _buildTermTranslator( indri::file::BulkTreeReader& newInfrequentTerms,
                                                          indri::file::BulkTreeReader& newFrequentTerms,
                                                          indri::index::TermRecorder& oldFrequentTermsRecorder,
                                                          indri::utility::HashTable<lemur::api::TERMID_T, lemur::api::TERMID_T>* oldInfrequent,
                                                          indri::index::TermRecorder& newFrequentTermsRecorder,
                                                          indri::index::Index* index,
                                                          indri::index::TermBitmap* bitmap );
      
      // buffers for _lookupTermID
      char *_compressedData;
      char *_uncompressedData;
      int _dataSize;

      enum {
        TOPDOCS_DOCUMENT_COUNT = 1000,
        FREQUENT_TERM_COUNT = 1000
      };

    public:
      IndexWriter();
      void write( indri::index::Index& index,
                  std::vector<indri::index::Index::FieldDescription>& fields,
                  indri::index::DeletedDocumentList& deletedList,
                  const std::string& fileName );
      void write( std::vector<indri::index::Index*>& indexes,
                  std::vector<indri::index::Index::FieldDescription>& fields,
                  indri::index::DeletedDocumentList& deletedList,
                  const std::string& fileName );
      void write( std::vector<indri::index::Index*>& indexes,
                  std::vector<indri::index::Index::FieldDescription>& fields,
                  std::vector<indri::index::DeletedDocumentList*>& deletedLists, 
                  const std::vector<lemur::api::DOCID_T>& documentMaximums,
                  const std::string& path );
    };
  }
}

#endif // INDRI_INDEXWRITER_HPP
