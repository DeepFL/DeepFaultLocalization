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
// DiskDocExtentListIterator
//
// 13 December 2004 -- tds
//

#include "indri/indri-platform.h"
#include "indri/DiskDocExtentListIterator.hpp"
#include "lemur/RVLCompress.hpp"

//
// DiskDocExtentListIterator constructor
//

indri::index::DiskDocExtentListIterator::DiskDocExtentListIterator( indri::file::SequentialReadBuffer* buffer, UINT64 startOffset )
  :
  _file(buffer),
  _startOffset(startOffset)
{
}

//
// DiskDocExtentListIterator destructor
//

indri::index::DiskDocExtentListIterator::~DiskDocExtentListIterator() {
  delete _file;
}

//
// setEndpoints
//

void indri::index::DiskDocExtentListIterator::setStartOffset( UINT64 startOffset ) {
  _startOffset = startOffset;
  _file->seek( _startOffset );
}

//
// startIteration
//

void indri::index::DiskDocExtentListIterator::startIteration() {
  // seek to the start:
  _file->seek( _startOffset );

  // read the control byte
  UINT8 control;
  _file->read( &control, sizeof(UINT8) );

  _numeric = (control & 0x02) ? true : false;
  _ordinal = (control & 0x04) ? true : false;
  _parental = (control & 0x08) ? true : false;

  // clear out all the internal data
  _data.document = 0;
  _data.extents.clear();
  _data.numbers.clear();
  _skipDocument = -1;
  _list = _listEnd = 0;
  _finished = false;

  // read in the first entry
  _readSkip();
  nextEntry();
}

//
// nextEntry
//

bool indri::index::DiskDocExtentListIterator::nextEntry() {
  if( _list == _listEnd ) {
    if( _skipDocument > 0 ) {
      // need to read the next segment of this list
      _readSkip();
      _readEntry();
      return true;
    } else {
      // all done
      _finished = true;
      return false;
    }
  }

  _readEntry();
  return true;
}

//
// nextEntry
//

bool indri::index::DiskDocExtentListIterator::nextEntry( lemur::api::DOCID_T documentID ) {
  // skip ahead as much as possible
  while( _skipDocument > 0 && _skipDocument <= documentID ) {
    _readSkip();
  }

  // now, read entries until we find one that's good
  while( _data.document < documentID && _list != _listEnd ) {
    _readEntry();
  }

  // it's possible that documentID < _skipDocument,
  // but we've run to the end of all the documents in this
  // skip section.  In this case, skip forward.
  if( _list == _listEnd && _data.document < documentID && _skipDocument > documentID ) {
    _readSkip();
    _readEntry();
  }

  if( _data.document >= documentID ) {
    return true;
  } else {
    _finished = true;
    return false;
  }
}

//
// currentEntry
//

indri::index::DiskDocExtentListIterator::DocumentExtentData* indri::index::DiskDocExtentListIterator::currentEntry() {
  if( !finished() )
    return &_data;

  return 0;
}

//
// finished
//

bool indri::index::DiskDocExtentListIterator::finished() const {
  return _finished;
}

//
// _readSkip
//

void indri::index::DiskDocExtentListIterator::_readSkip() {
  int skipLength; 

  _file->read( &_skipDocument, sizeof(int) );
  _file->read( &skipLength, sizeof(int) );

  _list = static_cast<const char*>(_file->read( skipLength ));
  _listEnd = _list + skipLength;
  _data.document = 0;
}

//
// _readEntry
//

void indri::index::DiskDocExtentListIterator::_readEntry() {
  _data.extents.clear();
  _data.numbers.clear();

  int deltaDocument;
  _list = lemur::utility::RVLCompress::decompress_int( _list, deltaDocument );

  _data.document += deltaDocument;

  int numPositions;
  _list = lemur::utility::RVLCompress::decompress_int( _list, numPositions );

  int lastStart = 0;
  INT64 number;
  int ordinal = 0;
  int parent = -1;
  int deltaOrdinal = 0;

  for( int i=0; i<numPositions; i++ ) {
    Extent extent;
    extent.weight = 1;

    _list = lemur::utility::RVLCompress::decompress_int( _list, extent.begin );
    _list = lemur::utility::RVLCompress::decompress_int( _list, extent.end );

    // delta-decode with respect to previous extent begin
    extent.begin += lastStart;
    lastStart = extent.begin;
    // delta decode with respect to begin.
    extent.end += extent.begin;

    if( _ordinal ) {
      _list = lemur::utility::RVLCompress::decompress_int( _list, deltaOrdinal );
      ordinal += deltaOrdinal;
    }
    extent.ordinal = ordinal;

    if (_parental) {
      _list = lemur::utility::RVLCompress::decompress_int( _list, parent );
    }
    extent.parent=parent;

    if( _numeric ) {
      _list = lemur::utility::RVLCompress::decompress_longlong( _list, number );
      _data.numbers.push_back( number );
      extent.number=number;
    }
    _data.extents.push_back( extent );
  }
}
