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
// TermFieldStatistics
//
// 4 February 2004 -- tds
//

#ifndef INDRI_TERMFIELDSTATISTICS_HPP
#define INDRI_TERMFIELDSTATISTICS_HPP

#include "lemur/lemur-compat.hpp"
#include "lemur/IndexTypes.hpp"

namespace indri {
  namespace index {
    struct TermFieldStatistics {
      TermFieldStatistics() : totalCount(0), documentCount(0), lastDocument(0), lastCount(0) {}

      /// total number of times in the corpus that this term appears in this field
      UINT64 totalCount;

      /// total number of times in the corpus that this term appears in this document
      unsigned int documentCount;

      /// last document this term/field combination was seen in (used at index time)
      lemur::api::DOCID_T lastDocument;

      /// number of times that this term/field combination was seen in <lastDocument>
      unsigned int lastCount;

      void addOccurrence( lemur::api::DOCID_T documentID ) {
        if( lastDocument != documentID ) {
          lastDocument = documentID;
          lastCount = 0;
          documentCount++;
        }

        totalCount++;
        lastCount++;
      }
    };
  }
}

#endif // INDRI_TERMFIELDSTATISTICS_HPP

