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
// FieldEqualsNode
//
// 26 October 2004 -- tds
//

#include "indri/FieldEqualsNode.hpp"
#include "indri/Annotator.hpp"
#include "indri/FieldIteratorNode.hpp"

indri::infnet::FieldEqualsNode::FieldEqualsNode( const std::string& name, FieldIteratorNode* iterator, INT64 constant ) {
  _name = name;
  _field = iterator;
  _constant = constant;
}

void indri::infnet::FieldEqualsNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();
  _extents.clear();
  
  if( !_field )
    return;

  const indri::utility::greedy_vector<INT64>& numbers = _field->numbers();
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _field->extents();

  for( size_t i=0; i<numbers.size(); i++ ) {
    if( numbers[i] == _constant ) {
      _extents.push_back( extents[i] );
    }
  }
}

indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::FieldEqualsNode::extents() {
  return _extents;
}

lemur::api::DOCID_T indri::infnet::FieldEqualsNode::nextCandidateDocument() {
  return _field->nextCandidateDocument();
}

const std::string& indri::infnet::FieldEqualsNode::getName() const { 
  return _name;
}

void indri::infnet::FieldEqualsNode::annotate( class indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  annotator.addMatches( _extents, this, documentID, extent );
}

void indri::infnet::FieldEqualsNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}

