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

#include "indri/TermFrequencyBeliefNode.hpp"
#include "indri/InferenceNetwork.hpp"
#include <math.h>

indri::infnet::TermFrequencyBeliefNode::TermFrequencyBeliefNode( const std::string& name,
                                                                 class InferenceNetwork& network,
                                                                 int listID,
                                                                 indri::query::TermScoreFunction& scoreFunction )
  :
  _name(name),
  _network(network),
  _listID(listID),
  _function(scoreFunction)
{
  _maximumBackgroundScore = INDRI_HUGE_SCORE;
  _maximumScore = INDRI_HUGE_SCORE;
}

indri::infnet::TermFrequencyBeliefNode::~TermFrequencyBeliefNode() {
}

const indri::utility::greedy_vector<indri::index::DocListIterator::TopDocument>& indri::infnet::TermFrequencyBeliefNode::topdocs() const {
  if( _list )
    return _list->topDocuments();

  return _emptyTopdocs;
}

lemur::api::DOCID_T indri::infnet::TermFrequencyBeliefNode::nextCandidateDocument() {
  if( _list ) {
    const indri::index::DocListIterator::DocumentData* entry = _list->currentEntry();
    
    if( entry ) {
      return entry->document;
    }
  }

  return MAX_INT32;
}

double indri::infnet::TermFrequencyBeliefNode::maximumBackgroundScore() {
  return _maximumBackgroundScore;
}

double indri::infnet::TermFrequencyBeliefNode::maximumScore() {
  return _maximumScore;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::TermFrequencyBeliefNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  assert( extent.begin == 0 && extent.end == documentLength ); // FrequencyListCopier ensures this condition
  _extents.clear();

  double score = 0;
  
  if( _list ) {
    const indri::index::DocListIterator::DocumentData* entry = _list->currentEntry();
    int count = ( entry && entry->document == documentID ) ? (int)entry->positions.size() : 0;
    score = _function.scoreOccurrence( count, documentLength );

    assert( score <= _maximumScore || _list->topDocuments().size() > 0 );
    assert( score <= _maximumBackgroundScore || count != 0 );
  } else {
    score = _function.scoreOccurrence( 0, documentLength );
  }
  
  indri::api::ScoredExtentResult result(extent);
  result.score=score;
  result.document=documentID;
  _extents.push_back( result );

  return _extents;
}

bool indri::infnet::TermFrequencyBeliefNode::hasMatch( lemur::api::DOCID_T documentID ) {
  if( _list ) {
    const indri::index::DocListIterator::DocumentData* entry = _list->currentEntry();
    return ( entry && entry->document == documentID );
  }

  return false;
}

const indri::utility::greedy_vector<bool>& indri::infnet::TermFrequencyBeliefNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  assert( false && "A TermFrequencyBeliefNode should never be asked for position information" );  
  
  _matches.resize( extents.size(), false );
  return _matches;
}

const std::string& indri::infnet::TermFrequencyBeliefNode::getName() const {
  return _name;
}

void indri::infnet::TermFrequencyBeliefNode::indexChanged( indri::index::Index& index ) {
  // fetch the next inverted list
  _list = _network.getDocIterator( _listID );

  if( !_list ) {
    _maximumBackgroundScore = INDRI_HUGE_SCORE;
    _maximumScore = INDRI_HUGE_SCORE;
  } else {
    // maximum fraction
    double maximumFraction = 1;
    
    if( _list->topDocuments().size() ) {
      if( indri::api::Parameters::instance().get( "topdocs", true ) == false ) {
        //        std::cout << "using no topdocs!" << std::endl;
        const indri::index::DocListIterator::TopDocument& document = _list->topDocuments().front();
        maximumFraction = double(document.count) / double(document.length);
      } else {
      const indri::index::DocListIterator::TopDocument& document = _list->topDocuments().back();
      maximumFraction = double(document.count) / double(document.length);
      }
    }

    indri::index::TermData* termData = _list->termData();

    double maxOccurrences = ceil( double(termData->maxDocumentLength) * maximumFraction );

    _maximumScore = _function.scoreOccurrence( maxOccurrences, termData->maxDocumentLength );
    _maximumBackgroundScore = _function.scoreOccurrence( 0, 1 );
  }
}

void indri::infnet::TermFrequencyBeliefNode::annotate( indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  // can't annotate -- don't have position info
}
