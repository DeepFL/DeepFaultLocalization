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
// DocListIteratorNode
//
// 28 July 2004 -- tds
//

#include "indri/DocListIterator.hpp"
#include "indri/DocListIteratorNode.hpp"
#include "indri/Annotator.hpp"
#include "indri/InferenceNetwork.hpp"

indri::infnet::DocListIteratorNode::DocListIteratorNode( const std::string& name, class InferenceNetwork& network, int listID ) :
  _name(name),
  _network(network),
  _listID(listID)
{
}

lemur::api::DOCID_T indri::infnet::DocListIteratorNode::nextCandidateDocument() {
  if( _list ) {
    indri::index::DocListIterator::DocumentData* info = _list->currentEntry();
    if( info ) { 
      return info->document;
    }
  }

  return MAX_INT32;
}

void indri::infnet::DocListIteratorNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();

  _extents.clear();
  _lastExtent.begin = -1;
  _lastExtent.end = -1;

  if( !_list )
    return;

  indri::index::DocListIterator::DocumentData* info = _list->currentEntry();

  if( !info || info->document != documentID )
    return;
  
  indri::utility::greedy_vector<int>& positions = info->positions;

  for( size_t i = 0; i < positions.size(); i++ ) {
    _extents.push_back( indri::index::Extent( positions[i], positions[i]+1 ) );
  }
}

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::DocListIteratorNode::extents() {
  return _extents;
}

const std::string& indri::infnet::DocListIteratorNode::getName() const {
  return _name;
}

void indri::infnet::DocListIteratorNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  if (! _lastExtent.contains(extent)) {
    // if the last extent we annotated contains this one, there is no work
    // to do.
    _lastExtent = extent;
    annotator.addMatches( _extents, this, documentID, extent );
  }
}

void indri::infnet::DocListIteratorNode::indexChanged( indri::index::Index& index ) {
  _list = _network.getDocIterator( _listID );
  _lastExtent.begin = -1;
  _lastExtent.end = -1;
}

