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
// ShrinkageBeliefNode
//
// 29 June 2005 - pto
//

#ifndef INDRI_SHRINKAGEBELIEFNODE_HPP
#define INDRI_SHRINKAGEBELIEFNODE_HPP

#include "indri/BeliefNode.hpp"
#include "indri/TermScoreFunction.hpp"
#include "indri/greedy_vector"
#include "indri/ScoredExtentResult.hpp"
#include "indri/DocumentStructureHolderNode.hpp"
#include <vector>
#include <set>
#include <map>

namespace indri
{
  namespace infnet
  {
    
    class ShrinkageBeliefNode : public BeliefNode {
    protected:
      ListIteratorNode& _list;
      DocumentStructureHolderNode& _docStructHolder;
      double _maximumScore;
      double _maximumBackgroundScore;
      indri::query::TermScoreFunction& _scoreFunction;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _results;
      indri::utility::greedy_vector<bool> _matches;
      std::string _name;

      indri::utility::greedy_vector<double> _down;
      indri::utility::greedy_vector<double> _up;
      indri::utility::greedy_vector<double> _base;
      indri::utility::greedy_vector<double> _counts;
      lemur::api::DOCID_T _documentID;


      typedef struct sr {
        std::string fieldName;
        double weight;
        bool lengthProportional;        
      } smoothing_rule;

      struct lt_rule {
        bool operator()( smoothing_rule r1, smoothing_rule r2 ) const {
          int cmpVal = strcmp( r1.fieldName.c_str(), r2.fieldName.c_str() );
          if ( cmpVal != 0 ) {
            return cmpVal < 0;
          }
          if ( r1.weight != r2.weight ) {
            return r1.weight < r2.weight;
          } 
          if ( r1.lengthProportional != r2.lengthProportional) {
            return r1.lengthProportional;
          }
          return false;
        }
      };

      std::set<smoothing_rule, lt_rule> _ruleSet;
      std::map<int, smoothing_rule> _ruleMap;

      double _parentWeight;
      double _docWeight;
      double _otherWeight;
      bool _recursive;
      bool _queryLevelCombine;
      double _defaultScore;
      

      std::set<int> _roots;
      indri::utility::greedy_vector<int> _topDownOrder;

      void _buildScoreCache( lemur::api::DOCID_T documentID );

    public:
      ShrinkageBeliefNode( const std::string& name,
                           ListIteratorNode& child,
                           DocumentStructureHolderNode& documentStructureHolderNode,
                           indri::query::TermScoreFunction& scoreFunction,
                           double maximumBackgroundScore,
                           double maximumScore );

      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
      double maximumBackgroundScore();
      double maximumScore();
      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      const std::string& getName() const;
      void addShrinkageRule( std::string rule );
      // so that we can know collection/document lambdas of linear interpolation
      void setSmoothing( const std::string & stringSpec );
    };
  }
}

#endif // INDRI_SHRINKAGEBELIEFNODE_HPP
