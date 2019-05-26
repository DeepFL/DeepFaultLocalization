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
// NullScorerNode
//
// 10 March 2004 -- tds
//

#ifndef INDRI_NULLSCORERNODE_HPP
#define INDRI_NULLSCORERNODE_HPP

#include "indri/greedy_vector"
#include "indri/ScoredExtentResult.hpp"
#include "indri/TermScoreFunction.hpp"
#include "indri/BeliefNode.hpp"
namespace indri
{
  namespace infnet
  {
    
    class NullScorerNode : public BeliefNode {
    private:
      indri::query::TermScoreFunction& _scoreFunction;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _scores;
      indri::utility::greedy_vector<bool> _matches;
      std::string _name;
      double _maximumBackgroundScore;
      double _maximumScore;

    public:
      NullScorerNode( const std::string& name, indri::query::TermScoreFunction& scoreFunction );

      void indexChanged( indri::index::Index& index );
      lemur::api::DOCID_T nextCandidateDocument();
      double maximumScore();
      double maximumBackgroundScore();
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      const std::string& getName() const;
    };
  }
}

#endif // INDRI_NULLSCORERNODE_HPP

