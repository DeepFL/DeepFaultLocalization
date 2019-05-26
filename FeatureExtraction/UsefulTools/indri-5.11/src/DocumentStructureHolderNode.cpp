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
// DocumentStructureHolderNode
//
// 18 June 2005 -- pto
//

#include "indri/DocumentStructureHolderNode.hpp"
#include "lemur/lemur-compat.hpp"

indri::infnet::DocumentStructureHolderNode::DocumentStructureHolderNode( const std::string& name ) :
  _name(name),
  _nextDocument(1),
  _index(0),
  _docIterID(0),
  _docIter(0),
  _documentStructure(0)
{
}

indri::infnet::DocumentStructureHolderNode::~DocumentStructureHolderNode() {
  delete _documentStructure;
  delete _docIter;
}

void indri::infnet::DocumentStructureHolderNode::prepare( lemur::api::DOCID_T documentID ) {

  if ( _documentStructure == 0 ) {
    _documentStructure = new indri::index::DocumentStructure( *_index );
  }


  if (documentID < _index->documentMaximum()) {
    while (_docIterID < documentID) {
      _docIterID++;
      _docIter->nextEntry();
    }

    _documentStructure->loadStructure( _docIter->currentEntry()->fields() );
  }
  _nextDocument = documentID + 1;
  if (_nextDocument >= _index->documentMaximum()) {
    _nextDocument = MAX_INT32;
  }
}


lemur::api::DOCID_T indri::infnet::DocumentStructureHolderNode::nextCandidateDocument() {
  return _nextDocument;
}

const std::string& indri::infnet::DocumentStructureHolderNode::getName() const {
  return _name;
}


void indri::infnet::DocumentStructureHolderNode::indexChanged( indri::index::Index& index ) { 

  _index = & index;
  _nextDocument = 1;
  _docIterID = 1;

  delete _docIter;
  _docIter = index.termListFileIterator();
  _docIter->startIteration();

  delete _documentStructure;
  _documentStructure = 0;  
}

indri::index::DocumentStructure * indri::infnet::DocumentStructureHolderNode::getDocumentStructure() {
  return _documentStructure;
}
