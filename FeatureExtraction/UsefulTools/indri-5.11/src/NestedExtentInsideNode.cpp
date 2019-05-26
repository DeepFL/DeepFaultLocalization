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
// NestedExtentInsideNode
//
// 30 August 2005 -- pto
//

#include "indri/NestedExtentInsideNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"
#include <set>


indri::infnet::NestedExtentInsideNode::NestedExtentInsideNode( const std::string& name, ListIteratorNode* inner, ListIteratorNode* outer ) :
  _inner(inner),
  _outer(outer),
  _name(name)
{
}

void indri::infnet::NestedExtentInsideNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();
  _extents.clear();
  _lastExtent.begin = -1;
  _lastExtent.end = -1;

  if( !_inner || !_outer )
    return;

  const indri::utility::greedy_vector<indri::index::Extent>& inExtents = _inner->extents();
  const indri::utility::greedy_vector<indri::index::Extent>& outExtents = _outer->extents();

  indri::utility::greedy_vector<indri::index::Extent>::const_iterator innerIter = inExtents.begin();
  indri::utility::greedy_vector<indri::index::Extent>::const_iterator outerIter = outExtents.begin();


  // Walk through the inner list.
  // As we encounter a new node in the inner list:
  // - add new extents to an active outer list in the outer list that have the same begin or less
  // - remove extents from the active outer list where the end is less then the begin of the inner
  // Scan the active outer list for an extent that contains the inner.

  // Sort the active outer list by increasing end.
  // - When removing, the extents to remove will be at the beginning
  // - When scanning, check the last active outer extent.  If its end is larger than the inner
  //   extent's end, then we can add the inner extent.

  // Active outer extents
  std::set<indri::index::Extent, indri::index::Extent::ends_before_less> activeOuterExtents;
  while ( innerIter != inExtents.end() ) {
    // remove outer extents we don't need anymore
    std::set<indri::index::Extent, indri::index::Extent::ends_before_less>::iterator activeIter = activeOuterExtents.begin();
    std::set<indri::index::Extent, indri::index::Extent::ends_before_less>::iterator activeEnd = activeOuterExtents.end();
    while ( activeIter != activeEnd ) {
      if ( activeIter->end >= innerIter->begin ) {
        break;  
      } 
      activeIter++;
    }
    activeOuterExtents.erase( activeOuterExtents.begin(), activeIter );

    // push new outer extents on that we may need
    while ( outerIter != outExtents.end() && outerIter->begin <= innerIter->begin ) {
      // only insert if still applicable
      if ( outerIter->end >= innerIter->begin ) {
        activeOuterExtents.insert( *outerIter );
      }
      outerIter++;
    }
    // check to see if the last extent in the outer list contains the inner extent
    activeIter = activeOuterExtents.end();
    if (!activeOuterExtents.empty()) {
      activeIter--;
      if ( activeIter->end >= innerIter->end ) {
        // Since we know that all active outer extents have a begin that is at or before
        // the inner iter's begin, and from the if statement we know the end of one
        // of the active outer extents is at least 
        // as large as the inner end, we know the inner iter extent is contained
        // by the last extent in the active list (and possibly others)

        // !!!!!!!!!!!!!!! This may be wrong to use the activeIter weight here !!!!!!!!!!!!!!!!!
        // What if multiple outer extents match? Here we just take the weight of the first one.
        indri::index::Extent extent( innerIter->weight * activeIter->weight, 
                                     innerIter->begin,
                                     innerIter->end,
                                     innerIter->ordinal );
        _extents.push_back( extent );
      }
    }
    innerIter++;
  }

}

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::NestedExtentInsideNode::extents() {
  return _extents;
}

lemur::api::DOCID_T indri::infnet::NestedExtentInsideNode::nextCandidateDocument() {
  return lemur_compat::max( _inner->nextCandidateDocument(), _outer->nextCandidateDocument() );
}

const std::string& indri::infnet::NestedExtentInsideNode::getName() const {
  return _name;
}

void indri::infnet::NestedExtentInsideNode::annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  if (! _lastExtent.contains(extent)) {
    // if the last extent we annotated contains this one, there is no work
    // to do.
    _lastExtent = extent;
    annotator.addMatches( _extents, this, documentID, extent );

    indri::index::Extent range( extent.begin, extent.end );
    indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
    iter = std::lower_bound( _extents.begin(), _extents.end(), range, indri::index::Extent::begins_before_less() );
  
    for( size_t i = iter-_extents.begin(); i<_extents.size() && _extents[i].begin <= extent.end; i++ ) {
      _inner->annotate( annotator, documentID, (indri::index::Extent &)_extents[i] );
      _outer->annotate( annotator, documentID, (indri::index::Extent &)_extents[i] );
    }
  }
}


void indri::infnet::NestedExtentInsideNode::indexChanged( indri::index::Index& index ) {
  _lastExtent.begin = -1;
  _lastExtent.end = -1;
}


