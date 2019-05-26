/*==========================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software (and below), and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// ContextInclusionAndNode
//
// 31 Aug 2005 -- pto
//

//
// Behaves similary to a WeightedAndNode, but requires that one child be a 
// special node that we wish to preserve the extents for during ranking.
// Other nodes are considered the "context" nodes.
//

#ifndef INDRI_CONTEXTINCLUSIONANDNODE_HPP
#define INDRI_CONTEXTINCLUSIONANDNODE_HPP

#include <vector>
#include "indri/BeliefNode.hpp"
#include "indri/SkippingCapableNode.hpp"
#include "indri/ScoredExtentResult.hpp"
#include <math.h>
namespace indri
{
  namespace infnet
  {
    
    class ContextInclusionAndNode : public SkippingCapableNode {
    private:
      struct child_type {
        struct maxscore_less {
        public:
          bool operator () ( const child_type& one, const child_type& two ) const {
            // think of these two elements as the only two elements in the
            // #wand.  What is the threshold of each ordering?  Sort by
            // the lowest threshold.

            return (one.backgroundWeightedScore) > 
              (two.backgroundWeightedScore);
          }
        };

        BeliefNode* node;
        double weight;
        double maximumWeightedScore;
        double backgroundWeightedScore;
      };

      std::vector<child_type> _children;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _scores;
      indri::utility::greedy_vector<bool> _matches;
      std::string _name;

      indri::utility::greedy_vector<lemur::api::DOCID_T> _candidates;
      size_t _candidatesIndex;

      double _threshold;
      double _recomputeThreshold;
      int _quorumIndex;
      void _computeQuorum();
      double _computeMaxScore( unsigned int start );

      BeliefNode * _preserveExtentsChild;

    public:
      ContextInclusionAndNode( const std::string& name ) : _name(name), _threshold(-DBL_MAX), _quorumIndex(0), _recomputeThreshold(-DBL_MAX), _preserveExtentsChild(0) {}

      void addChild( double weight, BeliefNode* node, bool preserveExtents = false );
      void doneAddingChildren();

      // SkippingCapableNode
      void setThreshold( double threshold );

      // InferenceNetworkNode interface
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
      double maximumScore();
      double maximumBackgroundScore();
      indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      const std::string& getName() const;
    };
  }
}

#endif // INDRI_CONTEXTINCLUSIONANDNODE_HPP

