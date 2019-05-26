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
// ExtentOrNode
//
// 26 July 2004 -- tds
//

#include "indri/ExtentOrNode.hpp"
#include <algorithm>
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"

indri::infnet::ExtentOrNode::ExtentOrNode( const std::string& name, std::vector<ListIteratorNode*>& children ) :
  _children(children),
  _name(name)
{
}

void indri::infnet::ExtentOrNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();
  _extents.clear();
  _lastExtent.begin = -1;
  _lastExtent.end = -1;

  indri::utility::greedy_vector<indri::index::Extent> allExtents;

  // put all extents in the same bag
  for( size_t i=0; i<_children.size(); i++ ) {
    _extents.append( _children[i]->extents().begin(), _children[i]->extents().end() );
  }

  // sort all extents in order of beginning
  std::sort( _extents.begin(), _extents.end(), indri::index::Extent::begins_before_less() );
}

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::ExtentOrNode::extents() {
  return _extents;
}

lemur::api::DOCID_T indri::infnet::ExtentOrNode::nextCandidateDocument() {
  lemur::api::DOCID_T candidate = INT_MAX;
  
  for( size_t i=0; i<_children.size(); i++ ) {
    candidate = lemur_compat::min( _children[i]->nextCandidateDocument(), candidate );
  }

  return candidate;
}

const std::string& indri::infnet::ExtentOrNode::getName() const {
  return _name;
}

void indri::infnet::ExtentOrNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
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


void indri::infnet::ExtentOrNode::indexChanged( indri::index::Index& index ) {
  _lastExtent.begin = -1;
  _lastExtent.end = -1;
}


