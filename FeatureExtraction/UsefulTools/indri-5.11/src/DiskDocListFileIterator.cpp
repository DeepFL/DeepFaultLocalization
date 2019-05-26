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
// DiskDocListFileIterator 
//
// 13 December 2004 -- tds
//

#include "indri/DiskDocListFileIterator.hpp"

//
// _readEntry
//

void indri::index::DiskDocListFileIterator::_readEntry() {
  int headerLength;
  UINT64 startPosition;

  // figure out where we are, and how much header data needs to be read
  startPosition = _file->position();
  _file->read( &headerLength, sizeof headerLength );

  // read in the header
  _header.clear();

  _file->read( _header.write( headerLength ), headerLength );
  indri::utility::RVLDecompressStream stream( _header.front(), _header.position() );

  // header is RVLCompressed with the term first, followed by a termData structure
  stream >> _term;
  ::termdata_decompress( stream, _termData, _fieldCount );
  _termData->term = _term;

  // set up the iterator to find the appropriate data
  _iterator.setStartOffset( startPosition, _termData );
  _iterator.startIteration();

  _docListData.iterator = &_iterator;
  _docListData.termData = _termData;
}

//
// DiskDocListFileIterator
//

indri::index::DiskDocListFileIterator::DiskDocListFileIterator( indri::file::File& docListFile, int fieldCount ) : 
  _file( new indri::file::SequentialReadBuffer( docListFile ) ),
  _fileLength( docListFile.size() ),
  _fieldCount( fieldCount ),
  _iterator( _file, 0, 0 ),
  _finished( false )
{
  _termData = (indri::index::TermData*) malloc( ::termdata_size( fieldCount ) );
}

//
// DiskDocListFileIterator destructor
//

indri::index::DiskDocListFileIterator::~DiskDocListFileIterator() {
  free( _termData );
}

//
// startIteration
//

void indri::index::DiskDocListFileIterator::startIteration() {
  _finished = false;
  _readEntry();
}

//
// nextEntry
//

bool indri::index::DiskDocListFileIterator::nextEntry() {
  if( _file->position() < _fileLength ) {
    _readEntry();
    return true;
  }

  _finished = true;
  return false;
}

//
// currentEntry
//

indri::index::DocListFileIterator::DocListData* indri::index::DiskDocListFileIterator::currentEntry() {
  return &_docListData;
}

//
// currentEntry
//

const indri::index::DocListFileIterator::DocListData* indri::index::DiskDocListFileIterator::currentEntry() const {
  return &_docListData;
}

//
// finished
//

bool indri::index::DiskDocListFileIterator::finished() const {
  return _finished;
}
