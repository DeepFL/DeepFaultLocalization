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

#include "indri/CachedFrequencyBeliefNode.hpp"

indri::infnet::CachedFrequencyBeliefNode::CachedFrequencyBeliefNode( const std::string& name,
                                                                     indri::lang::ListCache::CachedList* list,
                                                                     indri::query::TermScoreFunction& scoreFunction,
                                                                     double maximumBackgroundScore,
                                                                     double maximumScore )
  :
  _name(name),
  _list(list),
  _function(scoreFunction),
  _maximumBackgroundScore(maximumBackgroundScore),
  _maximumScore(maximumScore)
{
  _iter = _list->entries.begin();
}

lemur::api::DOCID_T indri::infnet::CachedFrequencyBeliefNode::nextCandidateDocument() {
  return _iter < _list->entries.end() ? _iter->document : MAX_INT32;
}

double indri::infnet::CachedFrequencyBeliefNode::maximumBackgroundScore() {
  return _maximumBackgroundScore;
}

double indri::infnet::CachedFrequencyBeliefNode::maximumScore() {
  return _maximumScore;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::CachedFrequencyBeliefNode::score( lemur::api::DOCID_T documentID, int begin, int end, int documentLength ) {
  assert( begin == 0 && end == documentLength ); // FrequencyListCopier ensures this condition
  const indri::index::DocumentContextCount* entry = _iter < _list->entries.end() ? _iter : 0;
  _extents.clear();

  int count = 0;
  int contextSize = 0;

  if( entry && entry->document == documentID ) {
    count = entry->count;
    contextSize = entry->contextSize;

    // advance this pointer so we're looking at the next document
    _iter++;
  } else {
    count = 0;
    contextSize = documentLength;
  }

  double score = _function.scoreOccurrence( count, contextSize );

  assert( score <= _maximumScore );
  _extents.push_back( indri::api::ScoredExtentResult( score, documentID, begin, end ) );

  return _extents;
}

bool indri::infnet::CachedFrequencyBeliefNode::hasMatch( lemur::api::DOCID_T documentID ) {
  return ( _iter < _list->entries.end() && _iter->document == documentID );
}

const indri::utility::greedy_vector<bool>& indri::infnet::CachedFrequencyBeliefNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  // bogus result
  _matches.resize( extents.size(), false );
  return _matches;
}

const std::string& indri::infnet::CachedFrequencyBeliefNode::getName() const {
  return _name;
}

void indri::infnet::CachedFrequencyBeliefNode::annotate( indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, int begin, int end ) {
  // can't annotate -- don't have position info
}

void indri::infnet::CachedFrequencyBeliefNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}


