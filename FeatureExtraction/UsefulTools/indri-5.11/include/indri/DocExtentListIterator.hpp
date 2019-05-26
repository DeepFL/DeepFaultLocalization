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
// DocExtentListIterator
//
// 24 November 2004 -- tds
//

#ifndef INDRI_DOCEXTENTLISTITERATOR_HPP
#define INDRI_DOCEXTENTLISTITERATOR_HPP

#include "indri/Extent.hpp"
#include "indri/indri-platform.h"
#include "indri/greedy_vector"
#include "lemur/IndexTypes.hpp"

namespace indri {
  namespace index {
    class DocExtentListIterator {
    public:
      struct DocumentExtentData {
        int document;
        indri::utility::greedy_vector<Extent> extents;
        indri::utility::greedy_vector<INT64> numbers;
      };

      virtual ~DocExtentListIterator() {};
      
      virtual void startIteration() = 0;
      virtual bool finished() const = 0;
      virtual bool nextEntry() = 0;
      virtual bool nextEntry( lemur::api::DOCID_T documentID ) = 0;
      virtual DocumentExtentData* currentEntry() = 0;
    };
  }
}

#endif // INDRI_DOCEXTENTLISTITERATOR_HPP
