/*==========================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software (and below), and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/
//
// LengthPriorNode
//
// 7 July 2005 -- pto
//

#include "indri/LengthPriorNode.hpp"
#include <math.h>
#include "indri/Annotator.hpp"

indri::infnet::LengthPriorNode::LengthPriorNode( const std::string& name, BeliefNode * child, double exponent ) :
  _exponent(exponent),
  _name(name),
  _child(child)
{
}

indri::infnet::LengthPriorNode::~LengthPriorNode() {
}

lemur::api::DOCID_T indri::infnet::LengthPriorNode::nextCandidateDocument() {
  return _child->nextCandidateDocument();
}

bool indri::infnet::LengthPriorNode::hasMatch( lemur::api::DOCID_T documentID ) {
  return _child->hasMatch( documentID );
}

const indri::utility::greedy_vector<bool>& indri::infnet::LengthPriorNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  return _child->hasMatch( documentID, extents );
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::LengthPriorNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  _scores.clear();
  const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = _child->score( documentID, extent, documentLength );
  for( size_t j=0; j<childResults.size(); j++ ) {
    indri::api::ScoredExtentResult result(childResults[j]);

    double score = childResults[j].score ;
    int b = childResults[j].begin;
    int e = childResults[j].end;
    double length = e - b;

    if ( length > 0 ) {
      score += _exponent * log( length );
    } else {
      score = INDRI_TINY_SCORE;
    }

    result.score=score;
    result.document=documentID;
    result.begin=b;
    result.end=e;

    _scores.push_back( result );  
  }
  return _scores;
}

void indri::infnet::LengthPriorNode::annotate( class indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  annotator.add( this, documentID, extent); 
  _child->annotate( annotator, documentID, extent );

}

double indri::infnet::LengthPriorNode::maximumScore() {
  return INDRI_HUGE_SCORE;
}

double indri::infnet::LengthPriorNode::maximumBackgroundScore() {
  return INDRI_TINY_SCORE;
}

const std::string& indri::infnet::LengthPriorNode::getName() const {
  return _name;
}

void indri::infnet::LengthPriorNode::indexChanged( indri::index::Index& index ) {
}

