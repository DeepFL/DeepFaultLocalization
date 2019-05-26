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
// VocabularyIterator
//
// 23 November 2004 -- tds
//

#ifndef INDRI_VOCABULARYITERATOR_HPP
#define INDRI_VOCABULARYITERATOR_HPP

#include "indri/TermData.hpp"
#include "indri/DiskTermData.hpp"

namespace indri {
  namespace index { 
    class VocabularyIterator {
    public:
      virtual ~VocabularyIterator() {};
      
      virtual void startIteration() = 0;
      virtual bool finished() = 0;
      virtual bool nextEntry() = 0;
      virtual bool nextEntry(const char *skipTo) = 0;
                        
      virtual bool nextEntry(std::string skipTo) {
        return nextEntry(skipTo.c_str());
      }

      virtual DiskTermData* currentEntry() = 0;
    };
  }
}
    
#endif // INDRI_VOCABULARYITERATOR_HPP



