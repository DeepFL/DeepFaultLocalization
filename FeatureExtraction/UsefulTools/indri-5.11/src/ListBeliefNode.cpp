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
// ListBeliefNode
//
// 6 July 2004 -- tds
//

#include "indri/ListBeliefNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"

// computes the length of the scored context
int indri::infnet::ListBeliefNode::_contextLength( indri::index::Extent &extent ) {
  //
  // There are two possible contexts at work here.  Consider the query
  // #combine[sentence]( dog.(paragraph) )
  //
  // In this case, the context for scoring is text in a paragraph field, but
  // this text will be scored for every sentence.  The paragraph field will
  // be represented by <_context>, and the sentence to be scored
  // will be represented by the term offsets <begin> and <end>.
  //

  if( !_context )
    return extent.end - extent.begin;

  int contextLength = 0;
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _context->extents();

  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin > extent.end )
      break;

    if( extents[i].end < extent.begin )
      continue; 

    // make sure to restrict the extents here to match the scored region
    int extentBegin = lemur_compat::max( extents[i].begin, extent.begin );
    int extentEnd = lemur_compat::min( extents[i].end, extent.end );

    contextLength += extentEnd - extentBegin;
  }

  return contextLength;
}

double indri::infnet::ListBeliefNode::_contextOccurrences( indri::index::Extent &extent ) {
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _list.extents();
  double count = 0;
  int lastEnd = 0;

  // look for all occurrences within bounds and that don't overlap
  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin >= extent.begin &&
        extents[i].end <= extent.end &&
        extents[i].begin >= lastEnd ) {
      count += extents[i].weight;
      lastEnd = extents[i].end;
    }
  }

  return count;
}

double indri::infnet::ListBeliefNode::_documentOccurrences() {
  assert( _raw ); // score() maintains this invariant
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _raw->extents();
  double count = 0;
  int lastEnd = 0;

  // look for all occurrences within bounds and that don't overlap
  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin >= lastEnd ) {
      count += extents[i].weight;
      lastEnd = extents[i].end;
    }
  }

  return count;
}

indri::infnet::ListBeliefNode::ListBeliefNode( const std::string& name, ListIteratorNode& child, ListIteratorNode* context, ListIteratorNode* raw, indri::query::TermScoreFunction& scoreFunction, double maximumBackgroundScore, double maximumScore )
  :
  _name(name),
  _scoreFunction(scoreFunction),
  _maximumScore(maximumScore),
  _maximumBackgroundScore(maximumBackgroundScore),
  _documentSmoothing(false),
  _context(context),
  _raw(raw),
  _list(child)
{
  _maximumScore = INDRI_HUGE_SCORE;
}

lemur::api::DOCID_T indri::infnet::ListBeliefNode::nextCandidateDocument() {
  return _list.nextCandidateDocument();
}

double indri::infnet::ListBeliefNode::maximumBackgroundScore() {
  return _maximumBackgroundScore;
}

double indri::infnet::ListBeliefNode::maximumScore() {
  return _maximumScore;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::ListBeliefNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  int contextSize = _contextLength( extent );
  double occurrences = _contextOccurrences( extent );
  double documentOccurrences = _raw ? _documentOccurrences() : occurrences;
  double score = 0;

  
  score = _scoreFunction.scoreOccurrence( occurrences, contextSize, documentOccurrences, documentLength );

  _scores.clear();
  indri::api::ScoredExtentResult result(extent);
  result.score=score;
  result.document=documentID;
  _scores.push_back( result );

  return _scores;
}

void indri::infnet::ListBeliefNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _list.extents();

  indri::index::Extent range( extent.begin, extent.end );
  indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
  iter = std::lower_bound( extents.begin(), extents.end(), range, indri::index::Extent::begins_before_less() );

  // mark the begin and end points for this list
  for( size_t i = iter - extents.begin(); i < extents.size() && extents[i].begin <= extent.end; i++ ) {
    if( extents[i].begin >= extent.begin &&
        extents[i].end <= extent.end ) {
      annotator.add( this, documentID, (indri::index::Extent &)extents[i] );
      _list.annotate( annotator, documentID, (indri::index::Extent &)extents[i] );
    }
  }
}

bool indri::infnet::ListBeliefNode::hasMatch( lemur::api::DOCID_T documentID ) {
  return _list.extents().size() > 0;
}

const indri::utility::greedy_vector<bool>& indri::infnet::ListBeliefNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& matchExtents ) {
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _list.extents();
  _matches.clear();
  _matches.resize( matchExtents.size(), false );

  size_t i=0;
  size_t j=0; 

  while( i < extents.size() && j < matchExtents.size() ) {
    if( matchExtents[j].begin > extents[i].begin ) {
      i++;
      continue;
    }

    if( matchExtents[j].end < extents[i].end ) {
      j++;    
      continue;
    }

    assert( matchExtents[j].begin <= extents[i].begin );
    assert( matchExtents[j].end >= extents[i].end );

    _matches[j] = true;
    //    i++; // don't consume the extent on a match, bug 2549304
    j++;
  }

  return _matches;
}

const std::string& indri::infnet::ListBeliefNode::getName() const {
  return _name;
}

void indri::infnet::ListBeliefNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}


