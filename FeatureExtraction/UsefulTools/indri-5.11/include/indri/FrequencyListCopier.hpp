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
// FrequencyListCopier
//
// 24 August 2004 -- tds
//
// Finds IndexTerm nodes that only need to return frequency information,
// not positions, and inserts IndexFrequencyTerms instead.
//

#ifndef INDRI_FREQUENCYLISTCOPIER_HPP
#define INDRI_FREQUENCYLISTCOPIER_HPP

#include "ListCache.hpp"
namespace indri
{
  namespace lang
  {
    
    class FrequencyListCopier : public indri::lang::Copier {
    private:
      std::vector<indri::lang::Node*> _nodes;
      std::stack<indri::lang::Node*> _disqualifiers;
      indri::lang::IndexTerm* _lastTerm;
      bool _disqualifiedTree;

      ListCache* _listCache;

    public:
      FrequencyListCopier( ListCache* listCache ) : _listCache(listCache), _lastTerm(0), _disqualifiedTree(false) {}

      indri::lang::Node* defaultAfter( indri::lang::Node* oldNode, indri::lang::Node* newNode ) {
        if( _disqualifiers.size() && oldNode == _disqualifiers.top() )
          _disqualifiers.pop();
    
        _nodes.push_back( newNode );
        return newNode;
      }

      ~FrequencyListCopier() {
        indri::utility::delete_vector_contents<indri::lang::Node*>( _nodes );
      }

      void before( indri::lang::ExtentAnd* exAnd ) {
        _disqualifiedTree = true;
        _disqualifiers.push(exAnd);
      }

      void before( indri::lang::ExtentOr* exOr ) {
        _disqualifiedTree = true;
      }

      void before( indri::lang::ExtentInside* exInside ) {
        _disqualifiedTree = true;       
      }

      void before( indri::lang::NestedExtentInside* nestExInside ) {
        _disqualifiedTree = true;       
      }

      void before( indri::lang::ExtentRestriction* exRestrict ) {
        _disqualifiedTree = true;       
        _disqualifiers.push(exRestrict);
      }

      void before( indri::lang::ExtentEnforcement* exEnforce ) {
        _disqualifiedTree = true;       
        _disqualifiers.push(exEnforce);
      }

      void before( indri::lang::FixedPassage* fixedPassage ) {
        _disqualifiedTree = true;       
        _disqualifiers.push(fixedPassage);
      }

      void before( indri::lang::FieldGreaterNode* fieldGreater ) {
        _disqualifiedTree = true;
        _disqualifiers.push(fieldGreater);
      }

      void before( indri::lang::FieldLessNode* fieldLess ) {
        _disqualifiedTree = true;       
        _disqualifiers.push(fieldLess);
      }
      
      void before( indri::lang::FieldEqualsNode* fieldEquals ) {
        _disqualifiedTree = true;
        _disqualifiers.push(fieldEquals);
      }

      void before( indri::lang::FieldBetweenNode* fieldBetween ) {
        _disqualifiedTree = true;
        _disqualifiers.push(fieldBetween);
      }

      void before( indri::lang::ContextCounterNode* context ) {
        if( context->getContext() != NULL ) {
          _disqualifiedTree = true;
        }
      }
      
      void before( indri::lang::WeightedExtentOr* wExOr ) {
        _disqualifiedTree = true;
      }

      void before( indri::lang::ODNode* odNode ) {
        _disqualifiedTree = true;
      }

      void before( indri::lang::UWNode* uwNode ) {
        _disqualifiedTree = true;
      }

      void before( indri::lang::BAndNode* bandNode ) {
        _disqualifiedTree = true;
      }

      indri::lang::Node* after( indri::lang::IndexTerm* oldNode, indri::lang::IndexTerm* newNode ) {
        _lastTerm = newNode;
        return defaultAfter( oldNode, newNode );
      }

      void before( indri::lang::RawScorerNode* oldNode, indri::lang::RawScorerNode* newNode ) {
        _lastTerm = 0;
        _disqualifiedTree = false;
      }


      void before( indri::lang::NestedRawScorerNode* oldNode, indri::lang::NestedRawScorerNode* newNode ) {
        before( (indri::lang::RawScorerNode*) oldNode, (indri::lang::RawScorerNode*) newNode );
      }

      indri::lang::Node* after( indri::lang::RawScorerNode* oldNode, indri::lang::RawScorerNode* newNode ) {
        indri::lang::Node* result = 0;

        if( _lastTerm && !_disqualifiers.size() && !_disqualifiedTree && oldNode->getContext() == NULL ) {
          indri::lang::TermFrequencyScorerNode* scorerNode;
          // there's a term to score, and nothing to disqualify us from doing frequency scoring
          scorerNode = new indri::lang::TermFrequencyScorerNode( _lastTerm->getText(),
                                                                 _lastTerm->getStemmed() );

          scorerNode->setNodeName( oldNode->nodeName() );
          scorerNode->setSmoothing( oldNode->getSmoothing() );
          scorerNode->setStatistics( oldNode->getOccurrences(), oldNode->getContextSize(), oldNode->getDocumentOccurrences(), oldNode->getDocumentCount() );

          delete newNode;
          result = defaultAfter( oldNode, scorerNode );
        } else if( !_disqualifiers.size() ) {
          ListCache::CachedList* list = 0; 

          if( _listCache )
            list = _listCache->find( newNode->getRawExtent(), newNode->getContext() );
      
          if( list ) {
            indri::lang::CachedFrequencyScorerNode* cachedNode;
            cachedNode = new indri::lang::CachedFrequencyScorerNode( newNode->getRawExtent(), newNode->getContext() );
            cachedNode->setNodeName( newNode->nodeName() );
            cachedNode->setSmoothing( newNode->getSmoothing() );
            cachedNode->setList( list );

            delete newNode;
            result = defaultAfter( oldNode, cachedNode );
          } else {
            result = defaultAfter( oldNode, newNode );
          }
        } else {
          result = defaultAfter( oldNode, newNode );
        }

        _disqualifiedTree = false;
        return result; 
      }

      indri::lang::Node* after( indri::lang::NestedRawScorerNode* oldNode, indri::lang::NestedRawScorerNode* newNode ) {
        return after( (indri::lang::RawScorerNode*) oldNode, (indri::lang::RawScorerNode*) newNode );
      }
    };
  }
}

#endif // INDRI_FREQUENCYLISTCOPIER_HPP

