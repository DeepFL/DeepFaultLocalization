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
// MemoryIndexDocListFileIterator
//
// 23 November 2004 -- tds
//

#ifndef INDRI_MEMORYINDEXDOCLISTFILEITERATOR_HPP
#define INDRI_MEMORYINDEXDOCLISTFILEITERATOR_HPP

#include "indri/Mutex.hpp"
#include "indri/TermData.hpp"
#include "indri/DocListFileIterator.hpp"
#include "indri/DocListMemoryBuilder.hpp"
#include <algorithm>
#include <iostream> // DEBUG

namespace indri {
  namespace index {
    class MemoryIndexDocListFileIterator : public DocListFileIterator {
    private:
      const std::vector<MemoryIndex::term_entry*>& _termData;
      std::vector<MemoryIndex::term_entry*> _alphabetical;
      std::vector<MemoryIndex::term_entry*>::iterator _currentTerm;
      DocListMemoryBuilderIterator _iterator;
      DocListData _data;
      bool _finished;

    public:
      MemoryIndexDocListFileIterator( const std::vector<MemoryIndex::term_entry*>& termData ) :
        _termData(termData)
      {
      }

      void startIteration() {
        _finished = false;
        _alphabetical.clear();
        _alphabetical.reserve( _termData.size() );

        for( size_t i=0; i<_termData.size(); i++ ) {
          _alphabetical.push_back( _termData[i] );
        }

        std::sort( _alphabetical.begin(), _alphabetical.end(), MemoryIndex::term_entry::term_less() );

        _currentTerm = _alphabetical.begin();
        _data.termData = 0;
        _data.iterator = 0;

        if( _currentTerm != _alphabetical.end() ) {
          _data.termData = (*_currentTerm)->termData;
          _data.iterator = &_iterator;
          _iterator.reset( (*_currentTerm)->list, _data.termData );

          assert( (*_currentTerm)->list.documentFrequency() == _data.termData->corpus.documentCount );
          assert( (*_currentTerm)->list.termFrequency() == _data.termData->corpus.totalCount );
        } else {
          _finished = true;
        }
      }
      
      bool finished() const {
        return _finished;      
      }

      DocListData* currentEntry() { 
        if( !_finished )
          return &_data;

        return 0;
      }
      
      const DocListData* currentEntry() const { 
        if( !_finished )
          return &_data;

        return 0;
      }
      
      bool nextEntry() {
        if( _finished )
          return false;
        _currentTerm++;
        
        if( _currentTerm == _alphabetical.end() ) {
          _finished = true;
          return false;
        }
        
        _data.termData = (*_currentTerm)->termData;
        _iterator.reset( (*_currentTerm)->list, _data.termData );

        assert( (*_currentTerm)->list.documentFrequency() == _data.termData->corpus.documentCount );
        assert( (*_currentTerm)->list.termFrequency() == _data.termData->corpus.totalCount );
        return true;
      }
    };
  }
}

#endif // INDRI_MEMORYINDEXDOCLISTFILEITERATOR_HPP

