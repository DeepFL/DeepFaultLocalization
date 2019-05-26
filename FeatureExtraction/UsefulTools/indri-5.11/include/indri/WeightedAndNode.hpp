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
// WeightedAndNode
//
// 26 January 2004 - tds
//

#ifndef INDRI_WEIGHTEDANDNODE_HPP
#define INDRI_WEIGHTEDANDNODE_HPP

#include <vector>
#include "indri/BeliefNode.hpp"
#include "indri/SkippingCapableNode.hpp"
#include "indri/ScoredExtentResult.hpp"
#include <math.h>
namespace indri
{
  namespace infnet
  {
    
    class WeightedAndNode : public SkippingCapableNode {
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

    public:
      WeightedAndNode( const std::string& name ) : _name(name), _threshold(-DBL_MAX), _quorumIndex(0), _recomputeThreshold(-DBL_MAX) {}

      void addChild( double weight, BeliefNode* node );
      void doneAddingChildren();

      // override setSiblingsFlag for child node(s)
      void setSiblingsFlag(int f);

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

#endif // INDRI_WEIGHTEDANDNODE_HPP

