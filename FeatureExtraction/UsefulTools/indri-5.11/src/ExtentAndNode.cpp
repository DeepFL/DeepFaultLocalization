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
// ExtentAndNode
//
// 6 July 2004 -- tds
//

#include "indri/ExtentAndNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"

void indri::infnet::ExtentAndNode::_and( indri::utility::greedy_vector<indri::index::Extent>& out, const indri::utility::greedy_vector<indri::index::Extent>& one, const indri::utility::greedy_vector<indri::index::Extent>& two ) {
  indri::utility::greedy_vector<indri::index::Extent>::const_iterator oneIter = one.begin();
  indri::utility::greedy_vector<indri::index::Extent>::const_iterator twoIter = two.begin();

  out.clear();

  indri::index::Extent current;
  current.begin = 0;
  current.end = 0;

  while( oneIter != one.end() && twoIter != two.end() ) {
    indri::index::Extent intersection;

    // compute the intersection (may be 0 length)
    intersection.begin = lemur_compat::max( oneIter->begin, twoIter->begin );
    intersection.end = lemur_compat::min( oneIter->end, twoIter->end );
    intersection.begin = lemur_compat::min( intersection.begin, intersection.end );

    if( current.end < intersection.begin ) {
      // if last intersection had non-zero length, put it out in the vector
      if( current.begin < current.end )
        out.push_back( current );

      current = intersection;
    } else {
      // this intersection touches the last intersection,
      // so we'll just put them together
      current.end = intersection.end;
    }

    if( oneIter->end == intersection.end ) {
      oneIter++;
    }

    if( twoIter->end == intersection.end ) {
      twoIter++;
    }
  }
  
  if( current.begin != current.end )
    _extents.push_back( current );
}


indri::infnet::ExtentAndNode::ExtentAndNode( const std::string& name, std::vector<ListIteratorNode*>& children ) :
  _children(children),
  _name(name)
{
}

void indri::infnet::ExtentAndNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();

  _extents.clear();

  if( _children.size() == 2 ) {
    _and( _extents, _children[0]->extents(), _children[1]->extents() );
  } else if( _children.size() > 2 ) {
    indri::utility::greedy_vector<indri::index::Extent> first;
    indri::utility::greedy_vector<indri::index::Extent> second;
    unsigned int i;

    // this part is a little complex because I'm trying
    // to avoid copying extent vectors too much
    _and( first, _children[0]->extents(), _children[1]->extents() );

    for( i=2; i<_children.size()-2; i+=2 ) {
      _and( second, first, _children[i]->extents() );
      _and( first, second, _children[i+1]->extents() );
    }

    if( i==_children.size()-1 ) {
      _and( _extents, first, _children[i]->extents() ); 
    } else {
      _extents = first;
    }
  }
}

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::ExtentAndNode::extents() {
  return _extents;
}

lemur::api::DOCID_T indri::infnet::ExtentAndNode::nextCandidateDocument() {
  assert( _children.size() );
  lemur::api::DOCID_T candidate = 0;

  for( size_t i=0; i<_children.size(); i++ ) {
    candidate = lemur_compat::max( candidate, _children[i]->nextCandidateDocument() );
  }

  return candidate;
}

const std::string& indri::infnet::ExtentAndNode::getName() const {
  return _name;
}

void indri::infnet::ExtentAndNode::annotate( indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent) {
  annotator.addMatches( _extents, this, documentID, extent);
}

void indri::infnet::ExtentAndNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}

