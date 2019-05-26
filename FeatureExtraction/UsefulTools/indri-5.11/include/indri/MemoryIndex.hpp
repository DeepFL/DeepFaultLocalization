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
// 15 November 2004 -- tds
//

#ifndef INDRI_MEMORYINDEX_HPP
#define INDRI_MEMORYINDEX_HPP

#include "indri/Index.hpp"
#include "indri/Mutex.hpp"
#include "indri/HashTable.hpp"
#include "indri/DocumentData.hpp"
#include "indri/Buffer.hpp"
#include <list>
#include <vector>

#include "indri/DocListIterator.hpp"
#include "indri/DocListFileIterator.hpp"
#include "indri/TermList.hpp"
#include "indri/TermListFileIterator.hpp"
#include "indri/VocabularyIterator.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/DocListMemoryBuilder.hpp"
#include "indri/FieldStatistics.hpp"
#include "indri/CorpusStatistics.hpp"
#include "indri/DocExtentListMemoryBuilder.hpp"
#include "indri/ReadersWritersLock.hpp"
#include "indri/ReaderLockable.hpp"
#include "indri/WriterLockable.hpp"
#include "indri/RegionAllocator.hpp"

namespace indri {
  namespace index {
    class MemoryIndex : public Index {
    public:
      // vocabulary structure
      struct term_entry {
        struct term_less {
          bool operator() ( const term_entry* one, const term_entry* two ) const {
            return strcmp( one->term, two->term ) < 0;
          }
        };

        term_entry( indri::utility::RegionAllocator* allocator ) :
          list(allocator),
          next(0)
        {
        }

        void clearMark() {
          next = 0;
        }

        bool hasNext() {
          return next != 0 && next != (term_entry*) 1;
        }
          
        void mark() {
          next = (term_entry*) 1;
        }

        bool marked() {
          return next != 0;
        }

        char* term;
        lemur::api::TERMID_T termID;
        TermData* termData;
        term_entry* next;
        indri::index::DocListMemoryBuilder list;
      };
      
    private:
      indri::utility::RegionAllocator _allocator;

      indri::thread::ReadersWritersLock _lock;
      indri::thread::ReaderLockable _readLock;
      indri::thread::WriterLockable _writeLock;

      CorpusStatistics _corpusStatistics;
      lemur::api::DOCID_T _baseDocumentID;
      
      // document buffers
      indri::index::TermList _termList;
      indri::utility::greedy_vector<term_entry*> _seenTerms;

      // term lookups
      indri::utility::HashTable<const char*, term_entry*> _stringToTerm;
      std::vector<term_entry*> _idToTerm;

      // field statistics
      indri::utility::HashTable<const char*, int> _fieldLookup;
      std::vector<FieldStatistics> _fieldData;
      std::vector<indri::index::DocExtentListMemoryBuilder*> _fieldLists;
      
      // document statistics
      std::vector<indri::index::DocumentData> _documentData;
      
      // document vector buffers
      std::list<indri::utility::Buffer*> _termLists;
      UINT64 _termListsBaseOffset;
      
      void _addOpenTags( indri::utility::greedy_vector<indri::parse::TagExtent *>& indexedTags,
                         indri::utility::greedy_vector<indri::parse::TagExtent *>& openTags,
                         indri::utility::greedy_vector<indri::parse::TagExtent *>& extents,
                         unsigned int& extentIndex, 
                         unsigned int position );
      void _removeClosedTags( indri::utility::greedy_vector<indri::parse::TagExtent *>& tags, unsigned int position );
      void _writeFieldExtents( lemur::api::DOCID_T documentID, indri::utility::greedy_vector<indri::parse::TagExtent *>& indexedTags );
      void _writeDocumentTermList( UINT64& offset, int& byteLength, lemur::api::DOCID_T documentID, int documentLength, indri::index::TermList& locatedTerms );
      void _writeDocumentStatistics( UINT64 offset, int byteLength, int indexedLength, int totalLength, int uniqueTerms );
      term_entry* _lookupTerm( const char* term );
      void _destroyTerms();

      int _fieldID( const std::string& fieldName );
      int _fieldID( const char* fieldName );

    public:
      MemoryIndex();
      MemoryIndex( lemur::api::DOCID_T docBase );
      MemoryIndex( lemur::api::DOCID_T docBase, const std::vector<Index::FieldDescription>& fields );
      ~MemoryIndex();

      void close();

      lemur::api::DOCID_T documentBase();
      lemur::api::DOCID_T documentMaximum();
      
      lemur::api::TERMID_T term( const std::string& t );
      lemur::api::TERMID_T term( const char* t );
      std::string term( lemur::api::TERMID_T termID );

      int field( const char* fieldName );
      int field( const std::string& fieldName );
      std::string field( int fieldID );

      int documentLength( lemur::api::DOCID_T documentID );
      UINT64 documentCount();
      UINT64 documentCount( const std::string& term );
      UINT64 uniqueTermCount();
      
      UINT64 termCount( const std::string& term );
      UINT64 termCount();
      
      UINT64 fieldTermCount( const std::string& field );
      UINT64 fieldTermCount( const std::string& field, const std::string& term );
      
      UINT64 fieldDocumentCount( const std::string& field );
      UINT64 fieldDocumentCount( const std::string& field, const std::string& term );
      
      DocListIterator* docListIterator( lemur::api::TERMID_T termID );
      DocListIterator* docListIterator( const std::string& term );
      DocListFileIterator* docListFileIterator();
      DocExtentListIterator* fieldListIterator( int fieldID );
      DocExtentListIterator* fieldListIterator( const std::string& field );
      const TermList* termList( lemur::api::DOCID_T documentID );
      TermListFileIterator* termListFileIterator();

      VocabularyIterator* vocabularyIterator();
      VocabularyIterator* frequentVocabularyIterator();
      VocabularyIterator* infrequentVocabularyIterator();

      DocumentDataIterator* documentDataIterator();
      
      indri::thread::Lockable* iteratorLock();
      indri::thread::Lockable* statisticsLock();

      lemur::api::DOCID_T addDocument( indri::api::ParsedDocument& document );
      size_t memorySize();
    };
  }
}

#endif // INDRI_MEMORYINDEX_HPP
