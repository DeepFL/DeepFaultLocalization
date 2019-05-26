/*==========================================================================
 * Copyright (c) 2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

//
// MemoryDocumentDataIterator
//
// 21 January 2005 -- tds
//

#include "indri/MemoryDocumentDataIterator.hpp"

//
// MemoryDocumentDataIterator constructor
//

indri::index::MemoryDocumentDataIterator::MemoryDocumentDataIterator( const std::vector<DocumentData>& documentData ) :
  _dataVector(documentData)
{
}

//
// MemoryDocumentDataIterator destructor
//

indri::index::MemoryDocumentDataIterator::~MemoryDocumentDataIterator()
{
}

//
// startIteration
//

void indri::index::MemoryDocumentDataIterator::startIteration() {
  _iterator = _dataVector.begin();
}

// 
// nextEntry
//

bool indri::index::MemoryDocumentDataIterator::nextEntry() {
  if( !finished() )
    _iterator++;

  return !finished();
}

//
// finished
//

bool indri::index::MemoryDocumentDataIterator::finished() {
  return _iterator == _dataVector.end();
}

//
// currentEntry
//

const indri::index::DocumentData* indri::index::MemoryDocumentDataIterator::currentEntry() {
  if( finished() )
    return 0;

  return &(*_iterator);
}

