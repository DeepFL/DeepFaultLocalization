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
// SumNode
//
// 3 February 2004 -- tds
//
// True sum of belief inputs (not a weighted or
// normalized sum).
//

#ifndef INDRI_SUMNODE_HPP
#define INDRI_SUMNODE_HPP

#include <vector>
#include "lemur/lemur-platform.h"
namespace indri
{
  namespace lang
  {
    class SumNode : public BeliefNode {
    private:
      std::vector<BeliefNode*> _children;
      double _threshold;

    public:

      virtual void setSiblingsFlag(int f){
        bSiblings=f; // need to set the flag for the current node itself.
        if (_child) {  _child->setSiblingsFlag(f); }
      }

      void addChild( BeliefNode* beliefNode ) {
        // set sibling flag is more than 1 child to speedup
        if (_children.size() > 1) {
          beliefNode->setSiblingsFlag(1);
        }

        _children.push_back( beliefNode );

        // if this is the second child, ensure we have set the sibling flag
        // for the first and second ones (it will skip without this!)
        if (_children.size()==2) {
          for (int i=0; i < _children.size(); i++) {
            _children[i]->setSiblingsFlag(1);
          }
        }

      }

      lemur::api::DOCID_T nextCandidateDocument() {
        std::vector<BeliefNode*>::iterator iter;
        lemur::api::DOCID_T next = MAX_INT32;

        for( iter = _children.begin(); iter != _children.end(); iter++ ) {
          next = lemur_compat::min( next, (*iter)->nextCandidateDocument() );
        }

        return next;
      }

      double maximumScore() {
        std::vector<BeliefNode*>::iterator iter;
        double score = 0;

        for( iter = _children.begin(); iter != _children.end(); iter++ ) {
          score += (*iter)->maximumScore();
        }

        return score;
      }

      double score( lemur::api::DOCID_T documentID, int documentLength ) {
        std::vector<BeliefNode*>::iterator iter;
        double score = 0;

        for( iter = _children.begin(); iter != _children.end(); iter++ ) {
          score += (*iter)->score( documentID, documentLength );
        }

        return score;
      }
    };
  }
}

#endif // INDRI_SUMNODE_HPP


