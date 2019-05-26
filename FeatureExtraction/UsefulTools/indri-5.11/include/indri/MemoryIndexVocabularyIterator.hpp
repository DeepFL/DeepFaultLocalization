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
// MemoryIndexVocabularyIterator
//
// 23 November 2004 -- tds
//

#ifndef INDRI_MEMORYINDEXVOCABULARYITERATOR_HPP
#define INDRI_MEMORYINDEXVOCABULARYITERATOR_HPP

#include "indri/Mutex.hpp"
#include "indri/TermData.hpp"
#include "indri/DiskTermData.hpp"

namespace indri {
  namespace index {
    class MemoryIndexVocabularyIterator : public VocabularyIterator {
    private:
      typedef std::vector<MemoryIndex::term_entry*> VTermEntry;
      VTermEntry& _termData;
      VTermEntry::iterator _iterator;
      DiskTermData _diskTermData;

      // this tells us if the last nextEntry() came from 
      // a start iteration or not - needed for nextEntry(const char*)
      // call
      bool _justStartedIteration;
      
    public:
      MemoryIndexVocabularyIterator( VTermEntry& termData ) :
        _termData(termData)
      {
      }
      
      void startIteration() {
        _iterator = _termData.begin();

        _diskTermData.length = 0;
        _diskTermData.startOffset = 0;

        if( _iterator != _termData.end() ) {
          _diskTermData.termData = (*_iterator)->termData;
          _diskTermData.termID = (*_iterator)->termID;
        }

        _justStartedIteration=true;
      }
      
      DiskTermData* currentEntry() { 
        if( _iterator == _termData.end() )
          return 0;
        
        return &_diskTermData;
      }
      
      bool nextEntry() {
        if( finished() )
          return false;
        
        _iterator++;

        if( finished() )
          return false;

        _diskTermData.termID++;
        _diskTermData.termData = (*_iterator)->termData;
        return true;
      }

      bool nextEntry(const char *skipTo) {
        assert(skipTo!=NULL);

        int termLength=strlen(skipTo);
        if (!termLength) {
          startIteration();
          return true;
        }

        if (!_justStartedIteration) {
          _iterator++;
        }

        _justStartedIteration=false;

        while (_iterator!=_termData.end()) {

          if (strstr((*_iterator)->term, skipTo)==(*_iterator)->term) {
            return true;
          }
          _iterator++;
        }

        // return false...
        return false;
      }

      bool finished() {
        return _iterator == _termData.end();
      }
    };
  }
}

#endif // INDRI_MEMORYINDEXVOCABULARYITERATOR_HPP

