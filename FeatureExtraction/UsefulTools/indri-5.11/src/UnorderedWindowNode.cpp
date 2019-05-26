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
// UnorderedWindowNode
//
// 28 July 2004 -- tds
//

#include "indri/UnorderedWindowNode.hpp"
#include "indri/Annotator.hpp"
#include <algorithm>

indri::infnet::UnorderedWindowNode::UnorderedWindowNode( const std::string& name, std::vector<indri::infnet::ListIteratorNode*>& children ) :
  _name(name),
  _children(children),
  _childrenAlreadySorted(false),
  _windowSize(-1) // unlimited window size
{
}

indri::infnet::UnorderedWindowNode::UnorderedWindowNode( const std::string& name, std::vector<indri::infnet::ListIteratorNode*>& children, int windowSize ) :
  _name(name),
  _children(children),
  _childrenAlreadySorted(false),
  _windowSize(windowSize)
{
}

lemur::api::DOCID_T indri::infnet::UnorderedWindowNode::nextCandidateDocument() {
  lemur::api::DOCID_T maxDocument = 0;

  for( size_t i=0; i<_children.size(); i++ ) {
    lemur::api::DOCID_T current = _children[i]->nextCandidateDocument();
    if( current > maxDocument )
      maxDocument = current;
  }

  return maxDocument;
}

//
// Let each term occurrence be a (type, position) pair (where type is the
// type of term.  Then, we're trying to return every possible subset of 
// (type, position) pairs where we have exactly one of each type.  This is
// a computationally intensive job.
//
// However, note that some returned sets will strictly dominate others.  
// Specifically, when these sets get to a ListBeliefNode, extents that
// overlap will get thrown out.  Therefore, if there is some set
// that spans positions [x,y], and another set that spans [a,b], where
// x = a and y < b, [x,y] strictly dominates [a,b].  Anywhere that 
// [a,b] would be a match, [x,y] will also be a match (and will overlap with
// [a,b]), so we can throw out [a,b].
//
// This suggests an algorithm for finding unordered windows.  We take each
// term position pair in turn, and find the smallest window that includes it
// as the first term.  That covers all possibilities.
//

void indri::infnet::UnorderedWindowNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();
  _extents.clear();
  _lastExtent.begin = -1;
  _lastExtent.end = -1;

  assert( _children.size() >= 2 );
  indri::utility::greedy_vector<term_position> allPositions;
  int termsSeen = 0;

  // sort the children by size if not already sorted
  if (!_childrenAlreadySorted) {
    std::sort(_children.begin(), _children.end(), indri::infnet::UWNodeChildLess);
    _childrenAlreadySorted=true;
  }

  // add every term position from every list
  for( size_t i=0; i<_children.size(); i++ ) {
    const indri::utility::greedy_vector<indri::index::Extent>& childPositions = _children[i]->extents();

    if( childPositions.size() ) {
      termsSeen++;
    } else {
      // every item must have at least one child!
      // if we have an extent w/out one, we can
      // exit early
      return;
    }

    for( size_t j=0; j<childPositions.size(); j++ ) {
      term_position p;

      p.type = i;
      p.begin = childPositions[j].begin;
      p.end = childPositions[j].end;
      p.last = -1;
      p.weight = childPositions[j].weight;

      allPositions.push_back( p );
    }
  }

  // if not all the terms appear in this document,
  // there's no use looking for a window
  if( termsSeen != _children.size() )
    return;

  // sort all positions by <begin> index
  std::sort( allPositions.begin(), allPositions.end() );
  indri::utility::greedy_vector<int> lastPositions;
  lastPositions.resize(_children.size());
  std::fill( lastPositions.begin(), lastPositions.end(), -1 );

  // determine last pointers
  for( size_t i=0; i<allPositions.size(); i++ ) {
    allPositions[i].last = lastPositions[allPositions[i].type];
    lastPositions[allPositions[i].type] = i;
  }

  // loop over all term occurrences
  for( int i=0; i<allPositions.size(); i++ ) {
    int termsFound = 1;
    size_t current;
    double weight = 1;

    for( current=i+1; current < allPositions.size() && termsFound != _children.size(); current++ ) {
      if( (allPositions[current].end - allPositions[i].begin > _windowSize) && (_windowSize >= 0) )
        break;

      // if the last time this term appeared was before the beginning of this window,
      // then this is a new term for this window
      if( allPositions[current].last < i ) {
        termsFound++;
        weight *= allPositions[current].weight;
      } 
    }

    if( termsFound == _children.size() ) {
      _extents.push_back( indri::index::Extent( weight, allPositions[i].begin, allPositions[current-1].end ) );
    }
  }
}

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::UnorderedWindowNode::extents() {
  return _extents;
}

const std::string& indri::infnet::UnorderedWindowNode::getName() const {
  return _name;
}

void indri::infnet::UnorderedWindowNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  if (! _lastExtent.contains(extent)) {
    // if the last extent we annotated contains this one, there is no work
    // to do.
    _lastExtent = extent;
    annotator.addMatches( _extents, this, documentID, extent );
    indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
    iter = std::lower_bound( _extents.begin(), _extents.end(), extent, indri::index::Extent::begins_before_less() );

    while( iter != _extents.end() ) {
      for( size_t j=0; j<_children.size(); j++ ) {
        indri::index::Extent e = (*iter);
        if (extent.contains(e)) {
          _children[j]->annotate( annotator, documentID, e );
        }
      }
      iter++;
    }
  }
}

void indri::infnet::UnorderedWindowNode::indexChanged( indri::index::Index& index ) {
  _lastExtent.begin = -1;
  _lastExtent.end = -1;
}

