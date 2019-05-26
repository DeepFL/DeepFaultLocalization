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
// BooleanAndNode
//
// 16 November 2004 -- tds
// 

#include "indri/BooleanAndNode.hpp"
#include "indri/Annotator.hpp"

indri::infnet::BooleanAndNode::BooleanAndNode( const std::string& name, std::vector<indri::infnet::ListIteratorNode*>& children ) :
  _name(name),
  _lists(children)
{
}

void indri::infnet::BooleanAndNode::prepare( lemur::api::DOCID_T documentID ) {
  _extents.clear();

  // initialize the child / sibling pointer
  initpointer();

  // check for and condition
  for( size_t i=0; i<_lists.size(); i++ ) {
    if( _lists[i]->extents().size() == 0 )
      return;
  }

  // if all here, make a null extent
  _extents.push_back( indri::index::Extent( 0, 1 ) ); // breaks match highlighting.
}

indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::BooleanAndNode::extents() {
  return _extents;
}

lemur::api::DOCID_T indri::infnet::BooleanAndNode::nextCandidateDocument() {
  lemur::api::DOCID_T document = _lists[0]->nextCandidateDocument();

  for( size_t i=1; i<_lists.size(); i++ ) {
    document = lemur_compat::max( document, _lists[i]->nextCandidateDocument() );
  }
    
  return document;
}

const std::string& indri::infnet::BooleanAndNode::getName() const {
  return _name;
}

void indri::infnet::BooleanAndNode::annotate( class indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  annotator.addMatches( _extents, this, documentID, extent);
}

void indri::infnet::BooleanAndNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}

