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
// Annotator
//
// 19 July 2004 -- tds
//

#ifndef INDRI_ANNOTATOR_HPP
#define INDRI_ANNOTATOR_HPP

#include <map>
#include <vector>
#include <algorithm>
#include <set>

#include "indri/InferenceNetworkNode.hpp"
#include "indri/BeliefNode.hpp"
#include "indri/EvaluatorNode.hpp"
#include "indri/Extent.hpp"
namespace indri
{
  /*! \brief Inference net and inference net node classes. */
  namespace infnet 
  {
    
    class Annotator : public EvaluatorNode {
    private:
      BeliefNode* _belief;
      EvaluatorNode::MResults _annotations;
      std::string _name;

      std::map<std::string, std::set<indri::api::ScoredExtentResult, indri::api::ScoredExtentResult::score_greater> > _seen;

    public:
      Annotator( const std::string& name, BeliefNode* belief );
      void add( InferenceNetworkNode* node, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      void addMatches( indri::utility::greedy_vector<indri::index::Extent>& extents, InferenceNetworkNode* node, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      void evaluate( lemur::api::DOCID_T documentID, int documentLength );
  
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      EvaluatorNode::MResults& getResults();
      const std::string& getName() const;
      const EvaluatorNode::MResults& getResults() const;
    };
  }
}

#endif // INDRI_ANNOTATOR_HPP
