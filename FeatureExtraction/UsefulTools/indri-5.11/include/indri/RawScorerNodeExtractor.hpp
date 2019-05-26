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
// RawScorerNodeExtractor.hpp
//
// 5 March 2004 -- tds
//

#ifndef INDRI_RAWSCORERNODEEXTRACTOR_HPP
#define INDRI_RAWSCORERNODEEXTRACTOR_HPP

#include "indri/Walker.hpp"
#include "indri/QuerySpec.hpp"
namespace indri
{
  namespace lang
  {
    
    class RawScorerNodeExtractor : public indri::lang::Walker {
    private:
      std::vector<indri::lang::RawScorerNode*> _scorerNodes;
      std::vector<indri::lang::Node*> _reachableNodes;
      int _insideCount; // used unitialized

    public:
      RawScorerNodeExtractor() : _insideCount(0) { }
  
      void defaultBefore( indri::lang::Node* n ) {
        if( _insideCount ) {
          _reachableNodes.push_back(n);
        }
      }

      void before( indri::lang::RawScorerNode* node ) {
        _insideCount++;
        _scorerNodes.push_back(node);
        _reachableNodes.push_back(node);
      }

      void after( indri::lang::RawScorerNode* node ) {
        _insideCount--;
      }

      void before( indri::lang::NestedRawScorerNode* node ) {
        before( (indri::lang::RawScorerNode *) node );
      }

      void after( indri::lang::NestedRawScorerNode* node ) {
        after( (indri::lang::RawScorerNode *) node );
      }

      void before( indri::lang::ShrinkageScorerNode* node ) {
        before( (indri::lang::RawScorerNode *) node );
      }

      void after( indri::lang::ShrinkageScorerNode* node ) {
        after( (indri::lang::RawScorerNode *) node );
      }

      std::vector<indri::lang::RawScorerNode*>& getScorerNodes() {
        return _scorerNodes;
      }

      std::vector<indri::lang::Node*>& getReachableNodes() {
        return _reachableNodes;
      }
    };
  }
}

#endif // INDRI_RAWSCORERNODEEXTRACTOR_HPP

