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
// NullListNode
//
// 11 August 2004 -- tds
//
// Like NullScorerNode, but in a list context
//

#include "indri/NullListNode.hpp"

indri::infnet::NullListNode::NullListNode( const std::string& name, bool stopword ) : _name(name), _stopword(stopword)
{
}

bool indri::infnet::NullListNode::isStopword() const {
  return _stopword;
}

const std::string& indri::infnet::NullListNode::getName() const {
  return _name;
}

lemur::api::DOCID_T indri::infnet::NullListNode::nextCandidateDocument() {
  return MAX_INT32;
}

void indri::infnet::NullListNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();
}

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::NullListNode::extents() {
  return _extents;
}

void indri::infnet::NullListNode::annotate( class indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  // do nothing
}

void indri::infnet::NullListNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}
