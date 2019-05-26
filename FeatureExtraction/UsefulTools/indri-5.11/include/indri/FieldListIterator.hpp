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
// KeyfileFieldListIterator
//
// 10 January 2004 -- tds
//

#ifndef INDRI_FIELDLISTITERATOR_HPP
#define INDRI_FIELDLISTITERATOR_HPP

#include "indri/Extent.hpp"

namespace indri {
  namespace index {

    struct FieldExtentInfo {
      lemur::api::DOCID_T documentID;
      indri::utility::greedy_vector<indri::index::Extent> extents;
      indri::utility::greedy_vector<INT64> numbers;
    };

    class FieldListIterator {
    public:
      virtual void startIteration();
      virtual FieldExtentInfo* currentEntry();
      virtual bool nextEntry();
      virtual bool nextEntry( lemur::api::DOCID_T documentID );
    };
  }
}

#endif // INDRI_FIELDLISTITERATOR_HPP
