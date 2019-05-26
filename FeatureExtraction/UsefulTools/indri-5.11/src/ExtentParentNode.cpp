/*==========================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software (and below), and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/


//
// ExtentParentNode
//
// 31 Jan 2006 -- pto
//

#include "indri/ExtentParentNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"

indri::infnet::ExtentParentNode::ExtentParentNode( const std::string& name, 
                                                           ListIteratorNode* inner, 
                                                           ListIteratorNode* outer,
                                                           DocumentStructureHolderNode & documentStructureHolderNode ) :
  _inner(inner),
  _outer(outer),
  _docStructHolder(documentStructureHolderNode),
  _name(name)
{
}

void indri::infnet::ExtentParentNode::prepare( lemur::api::DOCID_T documentID ) {
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

  indri::index::DocumentStructure * docStruct = _docStructHolder.getDocumentStructure();  


  // check the inner extents, searching for a parent in outerNodes
  while ( innerIter != inExtents.end() ) {

    _leafs.clear();
    if ( innerIter->ordinal == 0 ) {
      docStruct->findLeafs( &_leafs, innerIter->begin, innerIter->end, true );
    } else {
      _leafs.insert( innerIter->ordinal );
    }


    std::set<int>::iterator leaf = _leafs.begin();
    bool found = false;
    while ( leaf != _leafs.end() && !found) {


      outerIter = outExtents.begin();
      while ( outerIter != outExtents.end() && !found ) {

        _ancestors.clear();
        if ( outerIter->ordinal == 0 ) {
          docStruct->findLeafs( &_ancestors, outerIter->begin, outerIter->end, true );
        } else {
          _ancestors.insert( outerIter->ordinal );
        }

        std::set<int>::iterator ancestor = _ancestors.begin();
        while ( ancestor != _ancestors.end() && !found ) {

          if ( *leaf == docStruct->parent( *ancestor ) ) {
            found = true;
            indri::index::Extent extent( innerIter->weight * outerIter->weight, 
                                         innerIter->begin,
                                         innerIter->end,
                                         innerIter->ordinal);     

            _extents.push_back( extent );
          }
          ancestor++;
        }
        outerIter++;
      } 
      leaf++;
    }
    innerIter++;
  }

}

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::ExtentParentNode::extents() {
  return _extents;
}

lemur::api::DOCID_T indri::infnet::ExtentParentNode::nextCandidateDocument() {
  return lemur_compat::max( _inner->nextCandidateDocument(), _outer->nextCandidateDocument() );
}

const std::string& indri::infnet::ExtentParentNode::getName() const {
  return _name;
}

void indri::infnet::ExtentParentNode::annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
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

void indri::infnet::ExtentParentNode::indexChanged( indri::index::Index& index ) {
  _lastExtent.begin = -1;
  _lastExtent.end = -1;
}


const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::ExtentParentNode::matches( indri::index::Extent &extent ) {

  _matches.clear();
  
  int begin = extent.begin;
  int end = extent.end;
  

  indri::index::DocumentStructure * docStruct = _docStructHolder.getDocumentStructure();  


  _ancestors.clear();
  if ( extent.ordinal == 0 ) {
    docStruct->findLeafs( &_ancestors, begin, end, true);
  } else {
    _ancestors.insert( extent.ordinal );
  }

  for( size_t i = 0; i < _extents.size(); i++ ) {    
    int innerBegin = _extents[i].begin;
    int innerEnd = _extents[i].end;

    _leafs.clear();
    if ( _extents[i].ordinal == 0 ) {
      docStruct->findLeafs( &_leafs, innerBegin, innerEnd, true);
    } else {
      _leafs.insert( _extents[i].ordinal );
    }

    bool match = false;
    std::set<int>::iterator ancestor = _ancestors.begin();
    while ( !match && ancestor != _ancestors.end() ) { 
      std::set<int>::iterator leaf = _leafs.begin();
      while ( !match && leaf != _leafs.end() ) {


        if ( *leaf == docStruct->parent(*ancestor) ) {
          match = true;
        }
        leaf++;
      } 
      ancestor++;
    }
    if ( match ) {
      _matches.push_back(_extents[i]);
    }
  }


  return _matches;
}
