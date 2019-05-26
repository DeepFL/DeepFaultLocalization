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
// TermListFileIterator
//
// 23 November 2004 -- tds
//

#ifndef INDRI_TERMLISTFILEITERATOR_HPP
#define INDRI_TERMLISTFILEITERATOR_HPP

#include "indri/TermList.hpp"
#include "lemur/IndexTypes.hpp"

namespace indri {
  namespace index {
    class TermListFileIterator {
    public:
      virtual ~TermListFileIterator() {};
      
      virtual void startIteration() = 0;
      virtual bool nextEntry() = 0;
      virtual bool nextEntry( lemur::api::DOCID_T documentID ) = 0;
      virtual bool finished() = 0;
      virtual TermList* currentEntry() = 0;
    };
  }
}

#endif // INDRI_TERMLISTFILEITERATOR_HPP

