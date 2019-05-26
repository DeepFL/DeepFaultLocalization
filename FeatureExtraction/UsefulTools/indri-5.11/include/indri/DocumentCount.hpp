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
// DocumentCount
// 
// 24 September 2004 -- tds
//

#ifndef INDRI_DOCUMENTCOUNT_HPP
#define INDRI_DOCUMENTCOUNT_HPP
namespace indri
{
  namespace index
  {
    
    struct DocumentCount {
      DocumentCount() {}

      DocumentCount( lemur::api::DOCID_T document, int count ) {
        this->document = document;
        this->count = count;
      }

      lemur::api::DOCID_T document;
      int count;
    };

    struct DocumentContextCount {
      DocumentContextCount( lemur::api::DOCID_T document, int count, int contextSize ) {
        this->document = document;
        this->count = count;
        this->contextSize = contextSize;
      }

      lemur::api::DOCID_T document;
      int count;
      int contextSize;
    };
  }
}

#endif // INDRI_DOCUMENTCOUNT_HPP
