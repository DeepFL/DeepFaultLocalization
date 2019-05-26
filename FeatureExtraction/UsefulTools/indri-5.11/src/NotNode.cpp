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
// NotNode
//
// 26 August 2004 -- tds
//

#include "indri/NotNode.hpp"
#include "indri/Annotator.hpp"
#include <cmath>

indri::infnet::NotNode::NotNode( const std::string& name, BeliefNode* child )
  :
  _name(name),
  _child(child)
{
}

// for convenience, the not node never actually matches
lemur::api::DOCID_T indri::infnet::NotNode::nextCandidateDocument() {
  return MAX_INT32;
}

double indri::infnet::NotNode::maximumBackgroundScore() {
  // we just want an upper bound on the background score
  // this is a poor upper bound, but it should be accurate
  return log( 1.0 - exp( _child->maximumBackgroundScore() ) );
}

double indri::infnet::NotNode::maximumScore() {
  // without a true minimum score available, we don't have a 
  // better estimate for the max than probability = 1.0; 
  // and log(1) = 0.
  return 0.0;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::NotNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  _extents.clear();
  const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& child = _child->score( documentID, extent, documentLength );

  for( size_t i=0; i<child.size(); i++ ) {
    indri::api::ScoredExtentResult result(child[i]);
    result.score=log( 1.0 - exp(child[i].score) );
    result.document=documentID;
    result.begin=extent.begin;
    result.end=extent.end;
    _extents.push_back( result );
  }

  return _extents;
}

bool indri::infnet::NotNode::hasMatch( lemur::api::DOCID_T documentID ) {
  return !_child->hasMatch( documentID );
}

const indri::utility::greedy_vector<bool>& indri::infnet::NotNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  // flip the return vector
  _matches.resize( extents.size(), false );
  const indri::utility::greedy_vector<bool>& childMatches = _child->hasMatch( documentID, extents );

  for( size_t i=0; i<childMatches.size(); i++ ) {
    if( childMatches[i] == false ) {
      _matches[i] = true;
    }
  }

  return _matches;
}

void indri::infnet::NotNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  annotator.add( this, documentID, extent);
  _child->annotate( annotator, documentID, extent );
}

const std::string& indri::infnet::NotNode::getName() const {
  return _name;
}

void indri::infnet::NotNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}


