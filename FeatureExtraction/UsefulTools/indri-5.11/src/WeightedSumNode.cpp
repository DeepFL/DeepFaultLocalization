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
// WeightedSumNode
//
// 6 July 2004 -- tds
//

#include "indri/WeightedSumNode.hpp"
#include "lemur/lemur-compat.hpp"
#include <math.h>
#include "indri/Annotator.hpp"
#include <cmath>

indri::infnet::WeightedSumNode::WeightedSumNode( const std::string& name ) : _name(name)
{
}

lemur::api::DOCID_T indri::infnet::WeightedSumNode::nextCandidateDocument() {
  lemur::api::DOCID_T candidate = MAX_INT32;

  for( size_t i=0; i<_children.size(); i++ ) {
    candidate = lemur_compat::min<lemur::api::DOCID_T>( _children[i]->nextCandidateDocument(), candidate );
  }

  return candidate;
}

void indri::infnet::WeightedSumNode::setSiblingsFlag(int f){
  // set flag for child nodes
  for(int i=0;i<_children.size();i++) {
    _children[i]->setSiblingsFlag(f);
  }
}


double indri::infnet::WeightedSumNode::maximumScore() {
  double s = 0;

  for( unsigned i=0; i<_children.size(); i++ ) {
    s += _weights[i] * exp( _children[i]->maximumScore() );
  }

  return log(s);
}

double indri::infnet::WeightedSumNode::maximumBackgroundScore() {
  double s = 0;

  for( unsigned i=0; i<_children.size(); i++ ) {
    s += _weights[i] * exp( _children[i]->maximumBackgroundScore() );
  }

  return log(s);
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::WeightedSumNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  double sumWeight = 0;
  
  double s = 0;
  bool scored = false;
  indri::utility::greedy_vector< indri::utility::greedy_vector<indri::api::ScoredExtentResult> > scores;
  for( unsigned i=0; i<_children.size(); i++ ) {
    const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = _children[i]->score( documentID, extent, documentLength );
    scores.push_back(childResults);
    // normalize over absolute values
    sumWeight += fabs(_weights[i]) * childResults.size();
  }
  for( unsigned i=0; i<_children.size(); i++ ) {
    indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = scores[i];
    for( size_t j=0; j<childResults.size(); j++ ) {
      scored=true;
      s += _weights[i] * exp( childResults[j].score )/sumWeight;
    }
  }

  _scores.clear();
  if (scored) {
    indri::api::ScoredExtentResult result(extent);
    result.score=log(s);
    result.document=documentID;
    _scores.push_back( result );
  }

  return _scores;
}

void indri::infnet::WeightedSumNode::annotate( indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  annotator.add(this, documentID, extent);

  for( unsigned i=0; i<_children.size(); i++ ) {
    _children[i]->annotate( annotator, documentID, extent );
  }
}

bool indri::infnet::WeightedSumNode::hasMatch( lemur::api::DOCID_T documentID ) {
  for( size_t i=0; i<_children.size(); i++ ) {
    if( _children[i]->hasMatch( documentID ) )
      return true;
  }

  return false;
}

//
// hasMatch
//

const indri::utility::greedy_vector<bool>& indri::infnet::WeightedSumNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  _matches.clear();
  _matches.resize( extents.size(), false );

  for( size_t i=0; i<_children.size(); i++ ) {
    const indri::utility::greedy_vector<bool>& kidMatches = _children[i]->hasMatch( documentID, extents );

    for( size_t j=0; j<kidMatches.size(); j++ ) {
      if( kidMatches[j] ) {
        _matches[j] = true;
      }
    }
  }

  return _matches;
}

void indri::infnet::WeightedSumNode::addChild( double weight, BeliefNode* child ) {
  // set sibling flag is more than 1 child to speedup
  if (_children.size() > 1) {
    child->setSiblingsFlag(1);
  }

  _children.push_back(child);
  _weights.push_back(weight);

  // if this is the second child, ensure we have set the sibling flag
  // for the first and second ones (it will skip without this!)
  if (_children.size()==2) {
    for (int i=0; i < _children.size(); i++) {
      _children[i]->setSiblingsFlag(1);
    }
  }

}

const std::string& indri::infnet::WeightedSumNode::getName() const {
  return _name;
}

void indri::infnet::WeightedSumNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}
