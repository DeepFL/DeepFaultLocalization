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
// EvaluatorNode.hpp
//
// 24 February 2004 -- tds
//

#ifndef INDRI_EVALUATORNODE_HPP
#define INDRI_EVALUATORNODE_HPP

#include "indri/InferenceNetworkNode.hpp"
#include "indri/ScoredExtentResult.hpp"
#include <string>
#include <vector>
#include <map>
namespace indri 
{
  namespace infnet 
  {
    
    class EvaluatorNode : public InferenceNetworkNode {
    public:
      typedef std::map< std::string, std::vector<indri::api::ScoredExtentResult> > MResults;

      virtual ~EvaluatorNode() {};

      // Called once for every document ID returned by nextCandidateDocument().
      // May be called for documents other than those returned by nextCandidateDocument().
      virtual void evaluate( lemur::api::DOCID_T documentID, int documentLength ) = 0;
      virtual const MResults& getResults() = 0;
    };
  }
}

#endif // INDRI_EVALUATORNODE_HPP
