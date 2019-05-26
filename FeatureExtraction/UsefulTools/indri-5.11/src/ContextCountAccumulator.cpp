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
// ContextCountAccumulator
//
// 24 February 2004 -- tds
//

#include "indri/ContextCountAccumulator.hpp"
#include <assert.h>
#include <vector>
#include "indri/QuerySpec.hpp"
#include "indri/EvaluatorNode.hpp"
#include "indri/DocumentCount.hpp"

indri::infnet::ContextCountAccumulator::ContextCountAccumulator( const std::string& name, ListIteratorNode* matches, ListIteratorNode* context ) :
  _name(name),
  _matches(matches),
  _context(context),
  _occurrences(0),
  _contextSize(0),
  _documentOccurrences(0),
  _documentCount(0)
{
}

indri::infnet::ContextCountAccumulator::~ContextCountAccumulator() {
}

const std::string& indri::infnet::ContextCountAccumulator::getName() const {
  return _name;
}

double indri::infnet::ContextCountAccumulator::getOccurrences() const {
  return _occurrences;
}

double indri::infnet::ContextCountAccumulator::getContextSize() const {
  return _contextSize;
}

int indri::infnet::ContextCountAccumulator::getDocumentOccurrences() const {
  return _documentOccurrences;
}

int indri::infnet::ContextCountAccumulator::getDocumentCount() const {
  return _documentCount;
}
    
const indri::infnet::EvaluatorNode::MResults& indri::infnet::ContextCountAccumulator::getResults() {
  // we must be finished, so now is a good time to add our results to the ListCache
  _results.clear();

  _results[ "occurrences" ].push_back( indri::api::ScoredExtentResult( _occurrences, 0 ) );
  _results[ "contextSize" ].push_back( indri::api::ScoredExtentResult( _contextSize, 0 ) );
  _results[ "documentOccurrences" ].push_back( indri::api::ScoredExtentResult(UINT64(_documentOccurrences), 0 ) );
  _results[ "documentCount" ].push_back( indri::api::ScoredExtentResult( UINT64(_documentCount), 0 ) );
   
  return _results;
}

const indri::infnet::ListIteratorNode* indri::infnet::ContextCountAccumulator::getContextNode() const {
  return _context;
}

const indri::infnet::ListIteratorNode* indri::infnet::ContextCountAccumulator::getMatchesNode() const {
  return _matches;
}

void indri::infnet::ContextCountAccumulator::evaluate( lemur::api::DOCID_T documentID, int documentLength ) {
  double documentOccurrences = 0; 
  double documentContextSize = 0;

  if( !_context ) {
    int lastEnd = 0;
    for( size_t i=0; i<_matches->extents().size(); i++ ) {
      const indri::index::Extent& extent = _matches->extents()[i];
      if( extent.begin >= lastEnd ) {
        // if this isn't a duplicate
        documentOccurrences += extent.weight;
        lastEnd = extent.end;
      }
    }
    if (_matches->extents().size() > 0)
      _documentOccurrences++;
    _occurrences += documentOccurrences;
  } else {

    const indri::utility::greedy_vector<indri::index::Extent>& matches = _matches->extents();
    const indri::utility::greedy_vector<indri::index::Extent>& extents = _context->extents();
    size_t ex = 0;
    int lastEnd = 0;
    for( size_t i=0; i<matches.size() && ex < extents.size(); i++ ) {
      // find a context extent that might possibly contain this match
      // here we're relying on the following invariants: 
      //    both arrays are sorted by beginning position
      //    the extents array may have some that are inside others, like:
      //      [1, 10] and [4, 6], or even [1,10] and [1,4]
      //      but it will never have overlapping extents, such as:
      //      [1, 10] and [5, 15]
      //    also, in the event that two inner extents start at the
      //      same position, the largest end position comes first
      //      (e.g. [1,10] comes before [1,4])
      // Therefore, if a match [a,b] is in any extent, it will be
      //   in the first one [c,d] such that d>a.or d=a if a=b.
      // Proof is by contradiction: if the match is in a context extent,
      //   but it's not the first one such that d>=a, then that context
      //   extent must overlap the first extent such that d>=a (which
      //   is not allowed).

      while( extents[ex].end < matches[i].begin
             // addresses don's issue in bugzilla #38
             || ( extents[ex].end == matches[i].begin && 
                  matches[i].end > matches[i].begin ) ) {

        ex++;

        if( ex >= extents.size() ) break;
      }

      if( ex < extents.size() &&
          matches[i].begin >= extents[ex].begin &&
          matches[i].end <= extents[ex].end &&
          matches[i].begin >= lastEnd) { // filter duplicates
        documentOccurrences += matches[i].weight;
        lastEnd = matches[i].end;
      }
    }

    for( size_t i=0; i<extents.size(); i++ ) {
      documentContextSize += extents[i].end - extents[i].begin;
    }

    if (documentOccurrences > 0)
      _documentOccurrences++;
    
    _occurrences += documentOccurrences;
    _contextSize += documentContextSize;
  }
}


lemur::api::DOCID_T indri::infnet::ContextCountAccumulator::nextCandidateDocument() {
  lemur::api::DOCID_T candidate = _matches->nextCandidateDocument();

  if( _context ) {
    candidate = lemur_compat::min( candidate, _context->nextCandidateDocument() );
  }

  return candidate;
}

//
// indexChanged
//

void indri::infnet::ContextCountAccumulator::indexChanged( indri::index::Index& index ) {
  if( ! _context ) {
    _contextSize += index.termCount();
  }
  _documentCount += index.documentCount();
}




