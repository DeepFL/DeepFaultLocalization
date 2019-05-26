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
// InferenceNetworkNode
//
// 26 January 2004 - tds
//

#ifndef INDRI_INFERENCENETWORKNODE_HPP
#define INDRI_INFERENCENETWORKNODE_HPP

#include <string>
#include "indri/Index.hpp"
namespace indri
{
  namespace infnet
  {
    
    class InferenceNetworkNode {
    public:
      virtual ~InferenceNetworkNode() {}
      virtual lemur::api::DOCID_T nextCandidateDocument() = 0;
      virtual void indexChanged( indri::index::Index& index ) = 0;
      virtual const std::string& getName() const = 0;
    };
  }
}

#endif // INDRI_INFERENCENETWORKNODE_HPP
