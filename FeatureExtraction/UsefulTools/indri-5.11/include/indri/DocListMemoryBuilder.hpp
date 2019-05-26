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
// DocListMemoryBuilder.hpp
//
// tds - 17 December 2003
//

#ifndef LEMUR_KEYFILEDOCLISTMEMORYBUILDER_HPP
#define LEMUR_KEYFILEDOCLISTMEMORYBUILDER_HPP

#include "lemur/RVLCompress.hpp"
#include <vector>
#include <assert.h>
#include "indri/greedy_vector"
#include "indri/DocListIterator.hpp"
#include "indri/RegionAllocator.hpp"

namespace indri {
  namespace index {
    struct DocListMemoryBuilderSegment {
      DocListMemoryBuilderSegment( char* b, char* d, char* c ) {
        base = b;
        data = d;
        capacity = c;
      }

      char* base;
      char* data;
      char* capacity;
    };

    class DocListMemoryBuilderIterator : public DocListIterator {
      const indri::utility::greedy_vector< DocListMemoryBuilderSegment, 4 >* _lists;
      indri::utility::greedy_vector< DocListMemoryBuilderSegment, 4 >::const_iterator _current;
      indri::index::DocListIterator::DocumentData _data;
      indri::utility::greedy_vector<DocListIterator::TopDocument> _emptyTopDocuments;
      
      const char* _list;
      const char* _listEnd;
      bool _finished;

      TermData* _termData;

    public:
      DocListMemoryBuilderIterator();
      DocListMemoryBuilderIterator( class DocListMemoryBuilder& builder, TermData* termData );

      void reset( class DocListMemoryBuilder& builder, TermData* termData );
      void reset( const indri::utility::greedy_vector< DocListMemoryBuilderSegment, 4 >& lists, TermData* termData );

      void startIteration();
      bool finished();
      bool nextEntry( lemur::api::DOCID_T documentID );
      bool nextEntry();
      TermData* termData();
      DocListIterator::DocumentData* currentEntry();
      indri::utility::greedy_vector<DocListIterator::TopDocument>& topDocuments();
    };

    class DocListMemoryBuilder {
    public:
      typedef DocListMemoryBuilderIterator iterator;
      friend class DocListMemoryBuilderIterator;

    private:
      int _documentFrequency;
      int _termFrequency;

      indri::utility::greedy_vector< DocListMemoryBuilderSegment, 4 > _lists;

      char* _list;
      char* _listBegin;
      char* _listEnd;

      char* _documentPointer;
      char* _locationCountPointer;

      int _lastLocation;
      int _lastDocument;
      int _lastTermFrequency;

      indri::utility::RegionAllocator* _allocator;

      inline void _safeAddLocation( int position );
      size_t _roundUp( size_t amount );
      void _grow();
      void _terminateDocument();

    public:
      DocListMemoryBuilder( indri::utility::RegionAllocator* allocator );
      ~DocListMemoryBuilder();
      const DocListMemoryBuilder& operator=( DocListMemoryBuilder& other );
      
      void startDocument( int docID );
      void addLocation( int location );
      void endDocument();

      void clear();
      void flush();
      bool empty();

      int documentFrequency() const;
      int termFrequency() const;
      size_t memorySize() const;
    };
  }
}

#endif // LEMUR_DOCLISTMEMORYBUILDER_HPP
