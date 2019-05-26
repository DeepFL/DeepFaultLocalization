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
// CorpusStatistics
//
// 4 February 2004 -- tds
//

#ifndef INDRI_CORPUSSTATISTICS_HPP
#define INDRI_CORPUSSTATISTICS_HPP

#include "lemur/IndexTypes.hpp"

namespace indri {
  namespace index {
    struct CorpusStatistics {
      CorpusStatistics() : totalTerms(0), totalDocuments(0), baseDocument(0), maximumDocument(0), uniqueTerms(0), maximumDocumentLength(0) {}
      UINT64 totalTerms;
      
      lemur::api::DOCID_T baseDocument;
      lemur::api::DOCID_T maximumDocument;
      unsigned int totalDocuments;
      unsigned int uniqueTerms;
      UINT64 maximumDocumentLength;
    };
  }
}

#endif // INDRI_KEYFILECORPUSSTATISTICS_HPP
