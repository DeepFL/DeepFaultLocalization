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
// InferenceNetworkBuilder
//
// 8 March 2004 -- tds
//

#ifndef INDRI_INFERENCENETWORKBUILDER_HPP
#define INDRI_INFERENCENETWORKBUILDER_HPP

#include "indri/QuerySpec.hpp"
#include "indri/Repository.hpp"
#include "indri/InferenceNetwork.hpp"
#include "indri/ListCache.hpp"
#include <map>
#include <vector>
namespace indri
{
  namespace infnet
  {
    
    class InferenceNetworkBuilder : public indri::lang::Walker {
    private:
      std::map< indri::lang::Node*, InferenceNetworkNode* > _nodeMap;
      InferenceNetwork* _network;
      indri::collection::Repository& _repository;
      indri::lang::ListCache& _cache;
      int _resultsRequested;
      int _maxWildcardTerms;

      template<typename _To, typename _From>
      std::vector<_To*> _translate( const std::vector<_From*>& children ) {
        std::vector<_To*> translation;

        for( unsigned int i=0; i<children.size(); i++ ) {
          translation.push_back( dynamic_cast<_To*>(_nodeMap[children[i]]) );  
        }

        return translation;
      }

      indri::query::TermScoreFunction* _buildTermScoreFunction( const std::string& smoothing, double occurrences, double contextSize, int documentOccurrences, int documentCount ) const;

      void _after( indri::lang::NestedExtentInside* extentInside );
      void _after( indri::lang::NestedRawScorerNode* rawScorerNode );
      void _after( indri::lang::ShrinkageScorerNode* shrinkageScorerNode );
      void _after( indri::lang::ExtentEnforcement * eeNode );
      void _after( indri::lang::ExtentDescendant* extentInside );
      void _after( indri::lang::ExtentChild* extentInside );
      void _after( indri::lang::ExtentParent* extentInside );

    public:
      static const int DEFAULT_MAX_WILDCARD_TERMS = 100;

      InferenceNetworkBuilder( indri::collection::Repository& repository, indri::lang::ListCache& cache, int resultsRequested, int maxWildcardTerms=DEFAULT_MAX_WILDCARD_TERMS );
      ~InferenceNetworkBuilder();

      InferenceNetwork* getNetwork();

      void defaultAfter( indri::lang::Node* node );
      void after( indri::lang::IndexTerm* term );
      void after( indri::lang::Field* field );
      void after( indri::lang::ExtentAnd* extentAnd );
      void after( indri::lang::ExtentOr* extentOr );
      void after( indri::lang::WeightedExtentOr* weightedExtentOr );
      void after( indri::lang::ExtentInside* extentInside );
      void after( indri::lang::BAndNode* uwNode );
      void after( indri::lang::ODNode* odNode );
      void after( indri::lang::UWNode* uwNode );
      void after( indri::lang::FilRejNode* uwNode );
      void after( indri::lang::FilReqNode* uwNode );
      void after( indri::lang::ContextCounterNode* contextCounterNode );
      void after( indri::lang::ContextSimpleCounterNode* contextSimpleCounterNode );
      void after( indri::lang::ListAccumulator* listAccumulator );
      void after( indri::lang::ScoreAccumulatorNode* scoreAccumulatorNode );
      void after( indri::lang::AnnotatorNode* annotatorNode );
      void after( indri::lang::TermFrequencyScorerNode* termScorerNode );
      void after( indri::lang::CachedFrequencyScorerNode* cachedScorerNode );
      void after( indri::lang::RawScorerNode* rawScorerNode );
      void after( indri::lang::ExtentRestriction* erNode );
      void after( indri::lang::FixedPassage* fpNode );
      void after( indri::lang::PriorNode* priorNode );
      void after( indri::lang::WeightNode* weightNode );
      void after( indri::lang::CombineNode* combineNode );
      void after( indri::lang::PlusNode* plusNode );
      void after( indri::lang::WPlusNode* plusNode );
      void after( indri::lang::OrNode* orNode );
      void after( indri::lang::NotNode* notSpecNode );
      void after( indri::lang::MaxNode* maxNode );
      void after( indri::lang::WAndNode* wandNode );
      void after( indri::lang::WSumNode* wsumNode );
      void after( indri::lang::FieldLessNode* flNode );
      void after( indri::lang::FieldGreaterNode* fgNode );
      void after( indri::lang::FieldBetweenNode* fbNode );
      void after( indri::lang::FieldEqualsNode* fbNode );
      void after( indri::lang::FilterNode* filterNode );
      void after( indri::lang::FieldWildcard* fieldWildcard );
      void after( indri::lang::ContextInclusionNode* contextInclusionNode );
      void after( indri::lang::LengthPrior* lengthPrior );
      void after( indri::lang::DocumentStructureNode * docStruct );
      void after( indri::lang::ShrinkageScorerNode* shrinkageScorerNode );
      void after( indri::lang::ExtentDescendant* extentInside );
      void after( indri::lang::ExtentChild* extentInside );
      void after( indri::lang::ExtentParent* extentInside );
      void after( indri::lang::WildcardTerm* wildcardTerm );
    };
  }
}

#endif // INDRI_INFERENCENETWORKBUILDER


