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
// MemoryIndex
//
// 24 November 2004 -- tds
//

#include "indri/MemoryIndex.hpp"
#include "indri/TermList.hpp"

#include "indri/MemoryIndexDocListFileIterator.hpp"
#include "indri/MemoryIndexVocabularyIterator.hpp"
#include "indri/MemoryIndexTermListFileIterator.hpp"
#include "indri/MemoryDocumentDataIterator.hpp"

#include "indri/FieldStatistics.hpp"
#include "indri/ScopedLock.hpp"

#include "lemur/Keyfile.hpp"
#include "indri/greedy_vector"
#include "indri/delete_range.hpp"

#include <algorithm>

const int HASH_TABLE_SIZE = 10*1024*1024;
const int ONE_MEGABYTE = 1024*1024;

//----------------------------
// Constructors
//----------------------------

indri::index::MemoryIndex::MemoryIndex() :
  _readLock(_lock),
  _writeLock(_lock),
  _stringToTerm( ONE_MEGABYTE, &_allocator )
{
  _corpusStatistics.baseDocument = 0;
  _corpusStatistics.maximumDocument = 0;
  _termListsBaseOffset = 0;
}

indri::index::MemoryIndex::MemoryIndex( lemur::api::DOCID_T docBase ) :
  _readLock(_lock),
  _writeLock(_lock),
  _stringToTerm( ONE_MEGABYTE, &_allocator )
{
  _corpusStatistics.baseDocument = docBase;
  _corpusStatistics.maximumDocument = docBase;
  _termListsBaseOffset = 0;
}

indri::index::MemoryIndex::MemoryIndex( lemur::api::DOCID_T docBase, const std::vector<Index::FieldDescription>& fields ) :
  _readLock(_lock),
  _writeLock(_lock),
  _stringToTerm( ONE_MEGABYTE, &_allocator )
{
  _corpusStatistics.baseDocument = docBase;
  _corpusStatistics.maximumDocument = docBase;
  _termListsBaseOffset = 0;
  _fieldData.reserve( fields.size() );

  for( size_t i=0; i<fields.size(); i++ ) {
    int fieldID = i+1;

    _fieldData.push_back( FieldStatistics( fields[i].name, fields[i].numeric, fields[i].ordinal, fields[i].parental, 0, 0, 0 ) );
    _fieldLists.push_back( new DocExtentListMemoryBuilder( fields[i].numeric, fields[i].ordinal, fields[i].parental ) );
    _fieldLookup.insert( _fieldData.back().name.c_str(), fieldID );
  }
}

//----------------------------
// Destructors
//----------------------------

indri::index::MemoryIndex::~MemoryIndex() {
  //get the lock back before destroying the terms if the vocabulary iterator 
  //is running
  indri::thread::ScopedLock sl( _readLock );
  // delete term lists
  std::list<indri::utility::Buffer*>::iterator bufferIter;
  for( bufferIter = _termLists.begin(); bufferIter != _termLists.end(); bufferIter++ ) {
    delete *bufferIter; 
  }

  // delete field lists
  indri::utility::delete_vector_contents<DocExtentListMemoryBuilder*>( _fieldLists );

  // delete term entries
  _destroyTerms();
}

//
// close
//

void indri::index::MemoryIndex::close() {
  // does nothing
}

// ---------------------------
// Corpus statistics accessors
// ---------------------------

//
// documentBase
//

lemur::api::DOCID_T indri::index::MemoryIndex::documentBase() {
  return _corpusStatistics.baseDocument;
}

//
// documentLength
//

int indri::index::MemoryIndex::documentLength( lemur::api::DOCID_T documentID ) {
  lemur::api::DOCID_T base = _corpusStatistics.baseDocument;

  if( base > documentID || (documentID - base) > (int)_documentData.size() )
    return 0;

  assert( documentID - base >= 0 );
  assert( (documentID - base) < _documentData.size() );

  return _documentData[ documentID - base ].totalLength;
}

//
// term
//

lemur::api::TERMID_T indri::index::MemoryIndex::term( const char* term ) {
  term_entry** entry = _stringToTerm.find( term );

  if( entry )
    return (*entry)->termID;

  return 0;
}

//
// term
//

lemur::api::TERMID_T indri::index::MemoryIndex::term( const std::string& term ) {
  term_entry** entry = _stringToTerm.find( term.c_str() );

  if( entry )
    return (*entry)->termID;

  return 0;
}

//
// term
//

std::string indri::index::MemoryIndex::term( lemur::api::TERMID_T termID ) {
  if( termID <= 0 || termID > (int)_idToTerm.size() )
    return std::string();

  term_entry* entry = _idToTerm[ termID - 1 ];
  return entry->term;
}

//
// field
//

std::string indri::index::MemoryIndex::field( int fieldID ) {
  if( fieldID <= 0 || fieldID > (int)_fieldData.size() )
    return "";
  
  return _fieldData[fieldID-1].name;
}

//
// field
//

int indri::index::MemoryIndex::field( const std::string& fieldName ) {
  return field( fieldName.c_str() );
}

//
// field
//

int indri::index::MemoryIndex::field( const char* fieldName ) {
  return _fieldID( fieldName );
}

//
// documentCount
//

UINT64 indri::index::MemoryIndex::documentCount( const std::string& term ) {
  term_entry** entry = _stringToTerm.find( term.c_str() );

  if( !entry )
    return 0;

  return (*entry)->termData->corpus.documentCount;
}

//
// documentMaximum
//

lemur::api::DOCID_T indri::index::MemoryIndex::documentMaximum() {
  return _corpusStatistics.maximumDocument;
}

//
// fieldDocumentCount
//

UINT64 indri::index::MemoryIndex::fieldDocumentCount( const std::string& field, const std::string& term ) {
  term_entry** entry = _stringToTerm.find( term.c_str() );
  int id = _fieldID( field );

  if( !entry || id == 0 )
    return 0;

  return (*entry)->termData->fields[id-1].documentCount;
}

//
// fieldDocumentCount
//

UINT64 indri::index::MemoryIndex::fieldDocumentCount( const std::string& field ) {
  int id = _fieldID( field );

  if( id == 0 )
    return 0;

  return _fieldData[id-1].documentCount;
}

//
// fieldTermCount
//

UINT64 indri::index::MemoryIndex::fieldTermCount( const std::string& field ) {
  int id = _fieldID( field );

  if( id == 0 )
    return 0;

  return _fieldData[id-1].totalCount;
}

//
// fieldTermCount
//

UINT64 indri::index::MemoryIndex::fieldTermCount( const std::string& field, const std::string& term ) {
  term_entry** entry = _stringToTerm.find( term.c_str() );
  int id = _fieldID( field );

  if( !entry || id == 0 )
    return 0;

  return (*entry)->termData->fields[id-1].totalCount;
}

//
// termCount
//

UINT64 indri::index::MemoryIndex::termCount() {
  return _corpusStatistics.totalTerms;
}

//
// termCount
//

UINT64 indri::index::MemoryIndex::termCount( const std::string& term ) {
  term_entry** entry = _stringToTerm.find( term.c_str() );

  if( !entry )
    return 0;

  return (*entry)->termData->corpus.totalCount;
}

//
// uniqueTermCount
//

UINT64 indri::index::MemoryIndex::uniqueTermCount() {
  return _corpusStatistics.uniqueTerms;
}

//
// documentCount
//

UINT64 indri::index::MemoryIndex::documentCount() {
  return _corpusStatistics.totalDocuments;
}

//
// _fieldID
//

int indri::index::MemoryIndex::_fieldID( const char* fieldName ) {
  int* entry = _fieldLookup.find( fieldName );

  if( entry )
    return *entry;

  return 0;
}

//
// _fieldID
//

int indri::index::MemoryIndex::_fieldID( const std::string& fieldName ) {
  return _fieldID( fieldName.c_str() );
}

//
// _writeFieldExtents
//

void indri::index::MemoryIndex::_writeFieldExtents( lemur::api::DOCID_T documentID, indri::utility::greedy_vector<indri::parse::TagExtent *>& indexedTags ) {
  indri::utility::HashTable< indri::parse::TagExtent *, int> tagIdMap;
  
  // sort fields
  std::sort( indexedTags.begin(), indexedTags.end(), indri::parse::LessTagExtent() );
  
  // we'll add to the end of the fields greedy_vector
  indri::utility::greedy_vector<indri::index::FieldExtent> & fields = _termList.fields();
  // this is used to set the parentOrdinals
  int offset = fields.size();
  
  // convert to field extents, set ids, and create the node map
  for( size_t i=0; i<indexedTags.size(); i++ ) {
    indri::parse::TagExtent * extent = indexedTags[i];
    
    int ordinal = int(i) + 1;

    // this is the id for the field type
    int tagId = _fieldID( extent->name );
    
    // convert the field
    indri::index::FieldExtent converted( tagId, extent->begin, extent->end, extent->number, ordinal);

    // add this node to the map;
    tagIdMap.insert( extent, ordinal );

    // add his field to the field list for the document
    fields.push_back( converted );

    // add this location to the inverted list for fields - deferred to below - 
    // _fieldLists[tagId - 1]->addLocation( documentID, extent->begin, extent->end, extent->number, ordinal );
  }

  // set the parent ordinals
  for( size_t j=0; j<indexedTags.size(); j++ ) {
    indri::parse::TagExtent * extent = indexedTags[j];
    
    // look up the parent 
    int parentOrdinal = 0;
    int * parentIter;
    if ( extent->parent != 0 ) {
      parentIter = tagIdMap.find( extent->parent );
      if( parentIter != 0 ) {
        parentOrdinal = *parentIter;
      } else {
        parentOrdinal = 0;
      }
    }
    // set the parent
    int ordinal = fields[ offset + j ].ordinal;
    int tagId = fields[ offset + j ].id;
    fields[ offset + j ].parentOrdinal = parentOrdinal;

    // add this location to the inverted list for fields
    _fieldLists[tagId - 1]->addLocation( documentID, extent->begin, extent->end, extent->number, ordinal, parentOrdinal );
  }
}

//
// _writeDocumentTermList
//

void indri::index::MemoryIndex::_writeDocumentTermList( UINT64& offset, int& byteLength, lemur::api::DOCID_T documentID, int documentLength, indri::index::TermList& locatedTerms ) {
  indri::utility::Buffer* addBuffer = 0;
  int docDataLength = 10 + 5 * locatedTerms.terms().size() + 2 * sizeof(FieldExtent) * locatedTerms.fields().size();
  
  // find a buffer to store this term list in, making a new one if necessary
  if( !_termLists.size() || _termLists.back()->size() - _termLists.back()->position() < (size_t)docDataLength ) {
    // we need a new Buffer
    if( !_termLists.size() )
      _termListsBaseOffset = 0;
    else
      _termListsBaseOffset += _termLists.back()->position();

    addBuffer = new indri::utility::Buffer(ONE_MEGABYTE);
    _termLists.push_back( addBuffer );
  } else {
    addBuffer = _termLists.back();
  }
  
  // found a buffer, now add the term list data
  offset = _termListsBaseOffset + addBuffer->position();
  locatedTerms.write( *addBuffer );
  byteLength = addBuffer->position() + _termListsBaseOffset - offset;
}

//
// _writeDocumentStatistics
//

void indri::index::MemoryIndex::_writeDocumentStatistics( UINT64 offset, int byteLength, int indexedLength, int totalLength, int uniqueTerms ) {
  indri::index::DocumentData data;
  
  data.offset = offset;
  data.byteLength = byteLength;
  data.totalLength = totalLength;
  data.indexedLength = indexedLength;
  data.totalLength = totalLength;
  data.uniqueTermCount = uniqueTerms;
  
  _documentData.push_back( data );
}

//
// _addOpenTags
//

void indri::index::MemoryIndex::_addOpenTags( indri::utility::greedy_vector<indri::parse::TagExtent *>& indexedTags,
                                              indri::utility::greedy_vector<indri::parse::TagExtent *>& openTags,
                                              indri::utility::greedy_vector<indri::parse::TagExtent *>& extents,
                                              unsigned int& extentIndex, 
                                              unsigned int position ) {
  for( ; extentIndex < extents.size(); extentIndex++ ) {
    indri::parse::TagExtent* extent = extents[extentIndex];
    
    if( extent->begin > (int)position )
      break;
    
    int tagId = _fieldID( extent->name );
    
    if( tagId == 0 )
      continue;
     
    openTags.push_back( extent );
    indexedTags.push_back( extent );
  }
}

//
// _removeClosedTags
//

void indri::index::MemoryIndex::_removeClosedTags( indri::utility::greedy_vector<indri::parse::TagExtent *>& tags, unsigned int position ) {
  for( size_t i=0; i<tags.size(); ) {
    if( tags[i]->end <= int(position) ) {
      tags.erase( tags.begin() + i );
    } else {
      i++;
    }
  }
}

//
// _lookupTerm
//
// Tries to find this term in a hash table--if it isn't there, it gets added.
//

indri::index::MemoryIndex::term_entry* indri::index::MemoryIndex::_lookupTerm( const char* term ) {
  term_entry** entry = _stringToTerm.find( term );

  // if we've seen it, return it
  if( entry )
    return *entry;


  // this is a term we haven't seen before
  _corpusStatistics.uniqueTerms++;
  lemur::api::TERMID_T termID = _corpusStatistics.uniqueTerms;
  // create a term data structure
  TermData* termData = termdata_construct( _allocator.allocate( termdata_size( _fieldData.size() ) ),
                                           _fieldData.size() );
  
  term_entry* newEntry = 0;
  int termLength = strlen(term);
  
  newEntry = (term_entry*) _allocator.allocate( termLength+1 + sizeof(term_entry) );
  newEntry->term = (char*) newEntry + sizeof(term_entry);
  strcpy( newEntry->term, term );
  new (newEntry) term_entry( &_allocator );
  
  // store in [termString->termData] cache
  entry = _stringToTerm.insert( newEntry->term );
  *entry = newEntry;

  // store termData structure in the  [termID->termData] cache
  _idToTerm.push_back( newEntry );
  
  newEntry->termID = termID;
  newEntry->termData = termData;
  newEntry->termData->term = newEntry->term;

  return newEntry;
}

//
// _destroyTerms
//

void indri::index::MemoryIndex::_destroyTerms() {
  for( unsigned int i=0; i<_idToTerm.size(); i++ ) {
    term_entry* entry = _idToTerm[i];
    termdata_destruct( entry->termData, _fieldData.size() );
    entry->~term_entry();
  }
}

//
// addDocument
//

lemur::api::DOCID_T indri::index::MemoryIndex::addDocument( indri::api::ParsedDocument& document ) {
  indri::thread::ScopedLock sl( _writeLock );
  
  unsigned int position = 0;
  unsigned int extentIndex = 0;
  indri::utility::greedy_vector<indri::parse::TagExtent *> openTags;
  indri::utility::greedy_vector<indri::parse::TagExtent *> indexedTags;
  unsigned int indexedTerms = 0;
  indri::utility::greedy_vector<char*>& words = document.terms;
  term_entry* entries = 0;

  // assign a document ID
  lemur::api::DOCID_T documentID = _corpusStatistics.maximumDocument;
  _corpusStatistics.totalDocuments++;
  _corpusStatistics.maximumDocument++;
  
  _termList.clear();

  // move words into inverted lists, recording model statistics as we go
  for( position = 0; position < words.size(); position++ ) {
    const char* word = words[position];
    
    if( !word || *word == 0 ) {
      _termList.addTerm(0);
      continue;
    }

    int wordLength = strlen(word);

    if( wordLength >= lemur::file::Keyfile::MAX_KEY_LENGTH-1 ) {
      _termList.addTerm(0);     
      continue;
    }

    // fetch everything we know about this word so far
    term_entry* entry = _lookupTerm( word );

    // store information about this term location
    indri::index::TermData* termData = entry->termData;

    // store this term in the direct list
    _termList.addTerm( entry->termID );

    assert( entry->list.termFrequency() == termData->corpus.totalCount );

    // store this term in the inverted list
    if( !entry->marked() )
      entry->list.startDocument( documentID );
    entry->list.addLocation( position ); 
    termData->corpus.totalCount++;

    assert( entry->list.termFrequency() == termData->corpus.totalCount );

    // link this term_entry onto a list of ones we've seen
    if( entries == 0 ) {
      entry->mark();
      entries = entry;
    } else if( !entry->marked() ) {
      entry->next = entries;
      entries = entry;
    }

    // update our open tag knowledge
    _addOpenTags( indexedTags, openTags, document.tags, extentIndex, position );
    _removeClosedTags( openTags, position );

    // for every open tag, we want to record that we've seen the 
    for( indri::utility::greedy_vector<indri::parse::TagExtent *>::iterator tag = openTags.begin(); tag != openTags.end(); tag++ ) {
      int id = _fieldID( (*tag)->name );
      indri::index::TermFieldStatistics* termField = &entry->termData->fields[id - 1];
      termField->addOccurrence( documentID );

      indri::index::FieldStatistics* field = &_fieldData[id - 1];
      field->addOccurrence( documentID );
    }

    indexedTerms++;
  }

  _corpusStatistics.totalTerms += words.size();

  // need to add any tags that contain no text at the end of a document
  _addOpenTags( indexedTags, openTags, document.tags, extentIndex, position );
  _removeClosedTags( openTags, position );

  // go through the list of terms we've seen and update doc length counts
  term_entry* entry = entries;
  int uniqueTerms = 0;

  while( entry ) {
    indri::index::TermData* termData = entry->termData;
    term_entry* old = entry;

    termData->maxDocumentLength = lemur_compat::max<int>( termData->maxDocumentLength, words.size() );
    termData->minDocumentLength = lemur_compat::min<int>( termData->minDocumentLength, words.size() );
    termData->corpus.documentCount++;

    entry->list.endDocument();
    entry = entry->hasNext() ? entry->next : 0;
    old->clearMark();
    uniqueTerms++;
  }

  // write out any field data we've encountered
  _writeFieldExtents( documentID, indexedTags );

  UINT64 offset;
  int byteLength;

  _writeDocumentTermList( offset, byteLength, documentID, int(words.size()), _termList );
  _writeDocumentStatistics( offset, byteLength, indexedTerms, int(words.size()), uniqueTerms );

  return documentID;
}

//
// docListIterator
//

indri::index::DocListIterator* indri::index::MemoryIndex::docListIterator( lemur::api::TERMID_T termID ) {
  assert( termID >= 0 );
  assert( termID < _corpusStatistics.uniqueTerms );
  
  if( termID == 0 )
    return 0;
  
  term_entry* entry = _idToTerm[termID - 1];
  return new DocListMemoryBuilderIterator( entry->list, entry->termData );
}

//
// docListIterator
//

indri::index::DocListIterator* indri::index::MemoryIndex::docListIterator( const std::string& term ) {
  term_entry** entry = _stringToTerm.find( term.c_str() );

  if( !entry )
    return 0;
  
  return new DocListMemoryBuilderIterator( (*entry)->list, (*entry)->termData );
}  

//
// fieldListIterator
//

indri::index::DocExtentListIterator* indri::index::MemoryIndex::fieldListIterator( int fieldID ) {
  if( fieldID <= 0 || fieldID > (int)_fieldData.size() )
    return 0;
  
  DocExtentListMemoryBuilder* builder = _fieldLists[fieldID-1];
  return builder->getIterator();
}

//
// fieldListIterator
//

indri::index::DocExtentListIterator* indri::index::MemoryIndex::fieldListIterator( const std::string& field ) {
  int fieldID = _fieldID( field );
  if( fieldID <= 0 || fieldID > (int)_fieldData.size() )
    return 0;
  
  DocExtentListMemoryBuilder* builder = _fieldLists[fieldID-1];
  return builder->getIterator();
}

//
// termList
//

const indri::index::TermList* indri::index::MemoryIndex::termList( lemur::api::DOCID_T documentID ) {
  int documentIndex = documentID - documentBase();
  if( documentIndex < 0 || documentIndex >= (int)_documentData.size() )
    return 0;

  const DocumentData& data = _documentData[documentIndex];
  UINT64 documentOffset = data.offset;
  indri::utility::Buffer* documentBuffer = 0;
  std::list<indri::utility::Buffer*>::const_iterator iter;

  for( iter = _termLists.begin(); iter != _termLists.end(); ++iter ) {
    if( documentOffset < (*iter)->position() ) {
      documentBuffer = (*iter);
      break;
    }

    documentOffset -= (*iter)->position();
  }

  assert( documentBuffer );
  TermList* list = new TermList();

  list->read( documentBuffer->front() + documentOffset, data.byteLength );
  return list;
}

//
// termListFileIterator
//

indri::index::TermListFileIterator* indri::index::MemoryIndex::termListFileIterator() {
  return new MemoryIndexTermListFileIterator( _termLists, _documentData );
}

//
// docListFileIterator
//

indri::index::DocListFileIterator* indri::index::MemoryIndex::docListFileIterator() {
  // has to be in alphabetical order
  return new indri::index::MemoryIndexDocListFileIterator( _idToTerm );
}

//
// vocabularyIterator
//

indri::index::VocabularyIterator* indri::index::MemoryIndex::vocabularyIterator() {
  return new indri::index::MemoryIndexVocabularyIterator( _idToTerm );
}

//
// infrequentVocabularyIterator
//

indri::index::VocabularyIterator* indri::index::MemoryIndex::infrequentVocabularyIterator() {
  return 0;
}

//
// frequentVocabularyIterator
//

indri::index::VocabularyIterator* indri::index::MemoryIndex::frequentVocabularyIterator() {
  return new indri::index::MemoryIndexVocabularyIterator( _idToTerm );
}

//
// documentDataIterator
//

indri::index::DocumentDataIterator* indri::index::MemoryIndex::documentDataIterator() {
  return new MemoryDocumentDataIterator( _documentData );
}

//
// statisticsLock
//

indri::thread::Lockable* indri::index::MemoryIndex::statisticsLock() {
  // technically, this should be _readLock, but statisticsLock is supposed to be
  // acquired after iteratorLock() has been acquired, so that one should cover it.
  return 0;
}

//
// iteratorLock
//

indri::thread::Lockable* indri::index::MemoryIndex::iteratorLock() {
  return &_readLock;
}

//
// memorySize
//

size_t indri::index::MemoryIndex::memorySize() {
  indri::thread::ScopedLock l( _readLock );

  indri::utility::HashTable<const char*, term_entry*>::iterator iter;

  // inverted list data
  size_t listDataSize = _allocator.allocatedBytes();

  // document metadata
  size_t documentDataSize = _documentData.size() * sizeof(indri::index::DocumentData);

  // document direct list data
  size_t termListsSize = 0;
  std::list<indri::utility::Buffer*>::iterator biter;

  for( biter = _termLists.begin(); biter != _termLists.end(); biter++ ) {
    termListsSize += (*biter)->size();
  }

  // field inverted lists
  std::vector<indri::index::DocExtentListMemoryBuilder*>::iterator fiter;
  size_t fieldListsSize = 0;

  for( fiter = _fieldLists.begin(); fiter != _fieldLists.end(); fiter++ ) {
    fieldListsSize += (*fiter)->memorySize();
  }

  return listDataSize +
    documentDataSize +
    termListsSize +
    fieldListsSize;
}
