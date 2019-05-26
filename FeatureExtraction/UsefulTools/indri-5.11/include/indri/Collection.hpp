/*==========================================================================
 * Copyright (c) 2003-2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// Collection
//
// 11 May 2004 -- tds
//

#ifndef INDRI_COLLECTION_HPP
#define INDRI_COLLECTION_HPP

#include "indri/ObjectHandler.hpp"
#include "indri/ParsedDocument.hpp"
#include "lemur/IndexTypes.hpp"

namespace indri
{
  namespace collection
  {
    
    class Collection {
    public:
      virtual ~Collection() {};
      virtual void addDocument( lemur::api::DOCID_T documentID, indri::api::ParsedDocument* document ) = 0;
      virtual indri::api::ParsedDocument* retrieve( lemur::api::DOCID_T  documentID ) = 0;
    };
  }
}

#endif // INDRI_COLLECTION_HPP
