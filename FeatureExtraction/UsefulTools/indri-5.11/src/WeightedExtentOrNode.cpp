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
// 8 April 2005 -- tds
//

#include "indri/WeightedExtentOrNode.hpp"
#include <algorithm>
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"

indri::infnet::WeightedExtentOrNode::WeightedExtentOrNode( const std::string& name, std::vector<ListIteratorNode*>& children, const std::vector<double>& weights ) :
  _weights(weights),  
  _children(children),
  _name(name)
{
  double sumWeight = 0;
  std::vector<double>::iterator iter;
  for( iter = _weights.begin(); iter != _weights.end(); iter++ ) {
    sumWeight += (*iter); 
  }
  for( iter = _weights.begin(); iter != _weights.end(); iter++ ) {
    *iter /=sumWeight;
  }
}

void indri::infnet::WeightedExtentOrNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();
  _extents.clear();
  _lastExtent.begin = -1;
  _lastExtent.end = -1;

  indri::utility::greedy_vector<indri::index::Extent> allExtents;

  for( size_t i=0; i<_children.size(); i++ ) {
    ListIteratorNode* child = _children[i];
    double weight = _weights[i];
    
    for( size_t j=0; j<child->extents().size(); j++ ) {
      const indri::index::Extent& extent = child->extents()[j];
      _extents.push_back( indri::index::Extent( weight * extent.weight, extent.begin, extent.end ) );
    }
  }

  // sort all extents in order of beginning
  std::sort( _extents.begin(), _extents.end(), indri::index::Extent::begins_before_less() );
}

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::WeightedExtentOrNode::extents() {
  return _extents;
}

lemur::api::DOCID_T indri::infnet::WeightedExtentOrNode::nextCandidateDocument() {
  lemur::api::DOCID_T candidate = INT_MAX;
  
  for( size_t i=0; i<_children.size(); i++ ) {
    candidate = lemur_compat::min( _children[i]->nextCandidateDocument(), candidate );
  }

  return candidate;
}

const std::string& indri::infnet::WeightedExtentOrNode::getName() const {
  return _name;
}

void indri::infnet::WeightedExtentOrNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
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

void indri::infnet::WeightedExtentOrNode::indexChanged( indri::index::Index& index ) {
  _lastExtent.begin = -1;
  _lastExtent.end = -1;
}
