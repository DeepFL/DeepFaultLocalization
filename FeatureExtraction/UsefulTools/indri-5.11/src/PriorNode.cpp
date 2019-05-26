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
// PriorNode
//
// 6 July 2004 -- tds
//

#include "indri/PriorNode.hpp"
#include "indri/greedy_vector"
#include <math.h>
#include "indri/Annotator.hpp"
#include "indri/InferenceNetwork.hpp"

indri::infnet::PriorNode::PriorNode( const std::string& name,
                                     class InferenceNetwork& network,
                                     int listID ) :
  _name(name),
  _listID(listID),
  _network(network),
  _iterator(0)
{
}

indri::infnet::PriorNode::~PriorNode() {
}

lemur::api::DOCID_T indri::infnet::PriorNode::nextCandidateDocument() {
  return MAX_INT32;
}

bool indri::infnet::PriorNode::hasMatch( lemur::api::DOCID_T documentID ) {
  // priors don't match; they only boost or cut
  return false;
}

const indri::utility::greedy_vector<bool>& indri::infnet::PriorNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  _matches.resize( extents.size(), false );
  return _matches;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::PriorNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  double score = -1e100;
  
  if( _iterator && !_iterator->finished() )
    score = _iterator->currentEntry()->score;
  
  _scores.clear();
  indri::api::ScoredExtentResult result(extent);
  result.score=score;
  result.document=documentID;
  _scores.push_back( result );
  return _scores;
}

void indri::infnet::PriorNode::annotate( class indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  score( documentID, extent, extent.end );
  
  for( unsigned int i=0; i<_scores.size(); i++ ) {
    annotator.add( this, documentID, (indri::index::Extent &)_scores[i]); 
  }
}

double indri::infnet::PriorNode::maximumScore() {
  return INDRI_HUGE_SCORE;
}

double indri::infnet::PriorNode::maximumBackgroundScore() {
  return INDRI_HUGE_SCORE;
}

const std::string& indri::infnet::PriorNode::getName() const {
  return _name;
}

void indri::infnet::PriorNode::indexChanged( indri::index::Index& index ) {
  _iterator = _network.getPriorIterator( _listID );
}

