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
// FieldBetweenNode
//
// 28 July 2004 -- tds
//

#include "indri/FieldBetweenNode.hpp"
#include "indri/Annotator.hpp"
#include "indri/FieldIteratorNode.hpp"

indri::infnet::FieldBetweenNode::FieldBetweenNode( const std::string& name, FieldIteratorNode* iterator, INT64 low, INT64 high ) {
  _name = name;
  _field = iterator;
  _low = low;
  _high = high;
}

void indri::infnet::FieldBetweenNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();
  _extents.clear();
  
  if( !_field )
    return;

  const indri::utility::greedy_vector<INT64>& numbers = _field->numbers();
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _field->extents();

  for( size_t i=0; i<numbers.size(); i++ ) {
    if( numbers[i] >= _low && numbers[i] <= _high ) {
      _extents.push_back( extents[i] );
    }
  }
}

indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::FieldBetweenNode::extents() {
  return _extents;
}

lemur::api::DOCID_T indri::infnet::FieldBetweenNode::nextCandidateDocument() {
  return _field->nextCandidateDocument();
}

const std::string& indri::infnet::FieldBetweenNode::getName() const { 
  return _name;
}

void indri::infnet::FieldBetweenNode::annotate( class indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  annotator.addMatches( _extents, this, documentID, extent );
}

void indri::infnet::FieldBetweenNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}


