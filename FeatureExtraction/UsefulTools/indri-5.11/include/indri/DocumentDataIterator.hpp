/*==========================================================================
 * Copyright (c) 2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

//
// DocumentDataIterator
//
// 21 January 2005 -- tds
//

#ifndef INDRI_DOCUMENTDATAITERATOR_HPP
#define INDRI_DOCUMENTDATAITERATOR_HPP

#include "indri/DocumentData.hpp"

namespace indri {
  namespace index {
    class DocumentDataIterator {
    public:
      virtual ~DocumentDataIterator() {};

      virtual void startIteration() = 0;
      virtual bool nextEntry() = 0;
      virtual const DocumentData* currentEntry() = 0;
      virtual bool finished() = 0;
    };
  }
}

#endif // INDRI_DOCUMENTDATAITERATOR_HPP

