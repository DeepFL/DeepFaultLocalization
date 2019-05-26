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
// DiskDocumentDataIterator
//
// 21 January 2004 -- tds
//

#include "indri/DiskDocumentDataIterator.hpp"

//
// DiskDocumentDataIterator constructor
//

indri::index::DiskDocumentDataIterator::DiskDocumentDataIterator( indri::file::File& documentDataFile ) :
  _documentDataFile(documentDataFile),
  _readBuffer( new indri::file::SequentialReadBuffer( documentDataFile, 1024*1024 ) ),
  _finished(false)
{
}

//
// DiskDocumentDataIterator destructor
//

indri::index::DiskDocumentDataIterator::~DiskDocumentDataIterator() {
  delete _readBuffer;
}

//
// startIteration
//

void indri::index::DiskDocumentDataIterator::startIteration() {
  _fileSize = _documentDataFile.size();
  _readBuffer->seek( 0 );
  if( _fileSize ) {
    _readBuffer->read( &_documentData, sizeof(DocumentData) );
    _finished = false;
  } else {
    _finished = true;
  }
}

//
// nextEntry
//

bool indri::index::DiskDocumentDataIterator::nextEntry() {
  if( _readBuffer->position() < _fileSize ) {
    _readBuffer->read( &_documentData, sizeof(DocumentData) );
    return true;
  }

  _finished = true;
  return false;
}

//
// currentEntry
//

const indri::index::DocumentData* indri::index::DiskDocumentDataIterator::currentEntry() {
  if( !finished() )
    return &_documentData;

  return 0;
}

//
// finished
//

bool indri::index::DiskDocumentDataIterator::finished() {
  return _finished;
}


