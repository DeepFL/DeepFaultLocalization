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
// ExtentInsideNode
//
// 28 July 2004 -- tds
//

#include "indri/ExtentInsideNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"

indri::infnet::ExtentInsideNode::ExtentInsideNode( const std::string& name, ListIteratorNode* inner, ListIteratorNode* outer ) :
  _inner(inner),
  _outer(outer),
  _name(name)
{
}

void indri::infnet::ExtentInsideNode::prepare( lemur::api::DOCID_T documentID ) {
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

  while( innerIter != inExtents.end() && outerIter != outExtents.end() ) {
    if( outerIter->contains( *innerIter ) ) {
      _extents.push_back( *innerIter );
      innerIter++;
    } else if( outerIter->begin <= innerIter->begin ) {
      outerIter++;
    } else { 
      innerIter++;
    }
  }
}

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::ExtentInsideNode::extents() {
  return _extents;
}

lemur::api::DOCID_T indri::infnet::ExtentInsideNode::nextCandidateDocument() {
  return lemur_compat::max( _inner->nextCandidateDocument(), _outer->nextCandidateDocument() );
}

const std::string& indri::infnet::ExtentInsideNode::getName() const {
  return _name;
}

void indri::infnet::ExtentInsideNode::annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
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

void indri::infnet::ExtentInsideNode::indexChanged( indri::index::Index& index ) {
  _lastExtent.begin = -1;
  _lastExtent.end = -1;
}


