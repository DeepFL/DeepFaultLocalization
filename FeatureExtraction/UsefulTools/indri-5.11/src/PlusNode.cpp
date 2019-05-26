/*==========================================================================
 * Copyright (c) 2009 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */



//
// PlusNode
//
// 06 August 2009 - dmf
//

#include "indri/PlusNode.hpp"
#include <algorithm>
#include "lemur/lemur-platform.h"
#include <iostream>
#include "indri/Annotator.hpp"
#include "indri/TermFrequencyBeliefNode.hpp"
#include "indri/greedy_vector"
#include "indri/delete_range.hpp"
#include "indri/Parameters.hpp"
#include "indri/ExtentRestrictionNode.hpp"


void indri::infnet::PlusNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}
  
lemur::api::DOCID_T indri::infnet::PlusNode::nextCandidateDocument() {
  lemur::api::DOCID_T candidate = MAX_INT32;
  for( unsigned int i=0; i<_children.size(); i++ ) {
    candidate = lemur_compat::min<lemur::api::DOCID_T>( candidate, _children[i]->nextCandidateDocument() );
  }
  return candidate;
}

void indri::infnet::PlusNode::annotate( indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  std::vector<BeliefNode*>::iterator iter;
  annotator.add( this, documentID, extent );

  for( iter = _children.begin(); iter != _children.end(); iter++ ) {
    (*iter)->annotate( annotator, documentID, extent );
  }
}

double indri::infnet::PlusNode::maximumBackgroundScore() {
  std::vector<BeliefNode*>::iterator iter;
  double minimum = 0.0;

  for( iter = _children.begin(); iter != _children.end(); iter++ ) {
    minimum += (*iter)->maximumBackgroundScore();
  }

  return minimum;
}

double indri::infnet::PlusNode::maximumScore() {
  std::vector<BeliefNode*>::iterator iter;
  double maximum = 0.0;

  for( iter = _children.begin(); iter != _children.end(); iter++ ) {
    maximum += (*iter)->maximumScore();
  }

  return maximum;
}

indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::PlusNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  std::vector<BeliefNode*>::iterator iter;
  double score = 0;
  bool scored = false;
  indri::utility::greedy_vector< indri::utility::greedy_vector<indri::api::ScoredExtentResult> > scores;
  for( iter = _children.begin(); iter != _children.end(); iter++ ) {
    const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = (*iter)->score( documentID, extent, documentLength );
    scores.push_back(childResults);
  }
  int i = 0;
  for( iter = _children.begin(); iter != _children.end(); iter++ ) {
    double childScore = 0;
    indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = scores[i];
    for( size_t j=0; j<childResults.size(); j++ ) {
      scored = true;
      childScore += childResults[j].score;
    }
    i++;
    score += childScore;
  }

  _scores.clear();
  if (scored) {
    // dmf 12/03 if no child returned an extent, don't push one
    indri::api::ScoredExtentResult result(extent);
    result.score=score;
    result.document=documentID;
    _scores.push_back( result );
  }

  return _scores;
}

//
// hasMatch
//

bool indri::infnet::PlusNode::hasMatch( lemur::api::DOCID_T documentID ) {
  for( size_t i=0; i<_children.size(); i++ ) {
    if( _children[i]->hasMatch( documentID ) )
      return true;
  }
  return false;
}

//
// hasMatch
//

const indri::utility::greedy_vector<bool>& indri::infnet::PlusNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
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

//
// getName
//

const std::string& indri::infnet::PlusNode::getName() const {
  return _name;
}

