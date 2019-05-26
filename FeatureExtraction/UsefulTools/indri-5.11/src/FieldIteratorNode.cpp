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
// FieldIteratorNode
//
// 28 July 2004 -- tds
//

#include "indri/FieldIteratorNode.hpp"
#include "indri/FieldListIterator.hpp"
#include "indri/InferenceNetwork.hpp"
#include "indri/Annotator.hpp"

indri::infnet::FieldIteratorNode::FieldIteratorNode( const std::string& name, InferenceNetwork& network, int listID ) :
  _name(name),
  _network(network),
  _listID(listID)
{
}

void indri::infnet::FieldIteratorNode::indexChanged( indri::index::Index& index ) {
  _list = _network.getFieldIterator( _listID );

  if( _list )
    _list->startIteration();
}

void indri::infnet::FieldIteratorNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();
  _extents.clear();
  _numbers.clear();

  if( !_list )
    return;

  const indri::index::DocExtentListIterator::DocumentExtentData* info = _list->currentEntry();

  if( info && info->document == documentID ) {
    _extents = info->extents;
    _numbers = info->numbers;
  }
}

/// returns a list of intervals describing positions of children
const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::FieldIteratorNode::extents() {
  return _extents;
}

const indri::utility::greedy_vector<INT64>& indri::infnet::FieldIteratorNode::numbers() {
  return _numbers;
}

lemur::api::DOCID_T indri::infnet::FieldIteratorNode::nextCandidateDocument() {
  if( !_list )
    return MAX_INT32;

  const indri::index::DocExtentListIterator::DocumentExtentData* info = _list->currentEntry();

  if( !info ) {
    return MAX_INT32;
  } else {
    return info->document;
  }
}

const std::string& indri::infnet::FieldIteratorNode::getName() const {
  return _name;
}

void indri::infnet::FieldIteratorNode::annotate( indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  annotator.addMatches( _extents, this, documentID, extent );
}


