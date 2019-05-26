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
// OrNode
//
// 6 July 2004 -- tds
//

#include "indri/OrNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/ScoredExtentResult.hpp"
#include "indri/Annotator.hpp"

indri::infnet::OrNode::OrNode( const std::string& name ) :
  _name(name)
{
}

indri::infnet::OrNode::OrNode( const std::string& name, const std::vector<BeliefNode*>& children ) :
  _children( children ),
  _name( name )
{
  // if we have children - set the sibling flag...
  if (_children.size() > 1) {
    for (int i=0; i < _children.size(); i++) {
      if (_children[i]) {
        _children[i]->setSiblingsFlag(1);
      }
    }
  }
}

void indri::infnet::OrNode::setSiblingsFlag(int f){
  // set flag for child nodes
  for(int i=0;i<_children.size();i++) {
    _children[i]->setSiblingsFlag(f);
  }
}


const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::OrNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  double notScore = 1;

  for( size_t i=0; i<_children.size(); i++ ) {
    const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = _children[i]->score( documentID, extent, documentLength );

    for( size_t j=0; j<childResults.size(); j++ ) {
      notScore *= 1. - exp( childResults[j].score );
    }
  }
  
  double score = log( 1. - notScore );
  _scores.clear();
  indri::api::ScoredExtentResult result(extent);
  result.score=score;
  result.document=documentID;
  _scores.push_back( result );

  return _scores;
}

void indri::infnet::OrNode::annotate( class indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  annotator.add(this, documentID, extent);

  for( size_t i=0; i<_children.size(); i++ ) {
    _children[i]->annotate( annotator, documentID, extent );
  }
}

double indri::infnet::OrNode::maximumScore() {
  double notScore = 1;

  for( size_t i=0; i<_children.size(); i++ ) {
    notScore *= 1. - exp( _children[i]->maximumBackgroundScore() );
  }

  return log( 1. - notScore );
}

double indri::infnet::OrNode::maximumBackgroundScore() {
  double notScore = 1;

  for( size_t i=0; i<_children.size(); i++ ) {
    notScore *= 1. - exp( _children[i]->maximumScore() );
  }

  return log( 1. - notScore );
}

bool indri::infnet::OrNode::hasMatch( lemur::api::DOCID_T documentID ) {
  for( size_t i=0; i<_children.size(); i++ ) {
    if( _children[i]->hasMatch( documentID ) )
      return true;
  }

  return false;
}

const indri::utility::greedy_vector<bool>& indri::infnet::OrNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
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

lemur::api::DOCID_T indri::infnet::OrNode::nextCandidateDocument() {
  lemur::api::DOCID_T nextCandidate = MAX_INT32;
  
  for( size_t i=0; i<_children.size(); i++ ) {
    nextCandidate = lemur_compat::min<lemur::api::DOCID_T>( nextCandidate, _children[i]->nextCandidateDocument() );
  }

  return nextCandidate;
}

void indri::infnet::OrNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}

const std::string& indri::infnet::OrNode::getName() const {
  return _name;
}

