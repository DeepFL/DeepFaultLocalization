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
// ScoredExtentAccumulator
//
// 24 February 2004 -- tds
//

#ifndef INDRI_SCOREDEXTENTACCUMULATOR_HPP
#define INDRI_SCOREDEXTENTACCUMULATOR_HPP

#include "indri/SkippingCapableNode.hpp"
#include <queue>
namespace indri
{
  namespace infnet
  {
    
    class ScoredExtentAccumulator : public EvaluatorNode {
    private:
      BeliefNode* _belief;
      SkippingCapableNode* _skipping;
      std::priority_queue<indri::api::ScoredExtentResult> _scores;
      std::vector<indri::api::ScoredExtentResult> _finalScores;
      int _resultsRequested;
      std::string _name;
      EvaluatorNode::MResults _results;

    public:
      ScoredExtentAccumulator( std::string name, BeliefNode* belief, int resultsRequested = -1 ) :
        _belief(belief),
        _resultsRequested(resultsRequested),
        _name(name),
        _skipping(0)
      {
        if( indri::api::Parameters::instance().get( "skipping", 1 ) )
          _skipping = dynamic_cast<SkippingCapableNode*>(belief);
      }

      void evaluate( lemur::api::DOCID_T documentID, int documentLength ) {
        if( _belief->hasMatch( documentID ) ) {
          indri::index::Extent docExtent(0, documentLength);
          const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& documentScores = _belief->score( documentID, docExtent, documentLength );

          for( size_t i=0; i<documentScores.size(); i++ ) {
            _scores.push( documentScores[i] );
          }

          while( int(_scores.size()) > _resultsRequested && _resultsRequested > 0 ) {
            _scores.pop();
            if( _skipping ) {
              double worstScore = _scores.top().score;
              _skipping->setThreshold( worstScore - DBL_MIN );
            }
          }
        }
      }
  
      lemur::api::DOCID_T nextCandidateDocument() {
        return _belief->nextCandidateDocument();
      }

      const std::string& getName() const {
        return _name;
      }

      const EvaluatorNode::MResults& getResults() {
        _results.clear();

        if( !_scores.size() )
          return _results;
    
        // making a copy of the heap here so the method can be const
        std::priority_queue<indri::api::ScoredExtentResult> heapCopy = _scores;
        std::vector<indri::api::ScoredExtentResult>& scoreVec = _results["scores"];

        // puts scores into the vector in descending order
        scoreVec.reserve( heapCopy.size() );
        for( int i=heapCopy.size()-1; i>=0; i-- ) {
          scoreVec.push_back( heapCopy.top() );
          heapCopy.pop();
        }

        return _results;
      }

      void indexChanged( indri::index::Index& index ) {
        // do nothing
      }
    };
  }
}

#endif // INDRI_SCOREDEXTENTACCUMULATOR_HPP

