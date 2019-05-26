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
// DocExtentListMemoryBuilder
//
// 24 November 2004 -- tds
//

#include "indri/DocExtentListMemoryBuilder.hpp"
#include "lemur/lemur-compat.hpp"
#include "lemur/RVLCompress.hpp"

const int MIN_SIZE = 128;
const int GROW_TIMES = 12;
const size_t PLENTY_OF_SPACE = 30; // docID, count, begin, end: 5 bytes each; number = 10 bytes; total is 30 bytes

//
// DocExtentListMemoryBuilder constructor
//

indri::index::DocExtentListMemoryBuilder::DocExtentListMemoryBuilder( bool numeric, bool ordinal, bool parental ) :
  _documentFrequency(0),
  _extentFrequency(0),
  _list(0),
  _listBegin(0),
  _listEnd(0),
  _lastLocation(0),
  _lastDocument(0),
  _lastOrdinal(0),
  _lastExtentFrequency(0),
  _documentPointer(0),
  _locationCountPointer(0),
  _numeric(numeric),
  _ordinal(ordinal),
  _parental(parental)
{
}

//
// ~DocExtentListMemoryBuilder
//

indri::index::DocExtentListMemoryBuilder::~DocExtentListMemoryBuilder() {
  for( size_t i=0; i<_lists.size(); i++ ) {
    delete[] _lists[i].base;
  }
}


//
// _roundUP
//
size_t indri::index::DocExtentListMemoryBuilder::_roundUp( size_t amount ) {
  // round up by MIN_SIZE << GROW_TIMES if it's big enough
  if( amount >= (MIN_SIZE << GROW_TIMES) ) {
    return (amount + (MIN_SIZE << GROW_TIMES)) & ~((MIN_SIZE << GROW_TIMES) - 1);
  }
  // didn't actually round up, but _grow will take care of that
  return amount;
}

//
// _grow
// 

void indri::index::DocExtentListMemoryBuilder::_grow() {
  char* lastList = _list;
  char* lastListBegin = _listBegin;
  char* lastListEnd = _listEnd;
  size_t documentCopyAmount = _documentPointer ? (lastList - _documentPointer) : 0;

  // fix data pointer of previous list
  if( lastList != 0 ) {
    if( _locationCountPointer ) {
      _lists.back().data = _documentPointer;
    } else {
      _lists.back().data = lastList;
    }

    assert( _lists.back().data <= _lists.back().capacity );
  }

  // actually add the new list
  unsigned int iterations = std::min<unsigned int>( GROW_TIMES, _lists.size() );
  size_t newSize = (MIN_SIZE << iterations);
  // ensure we have space for a new entry after copying.
  newSize = std::max<unsigned int>( newSize, _roundUp( documentCopyAmount + PLENTY_OF_SPACE ) );

  _list = new char[ newSize ];
  _listBegin = _list;
  _listEnd = _list + newSize;

  _lists.push_back( DocExtentListMemoryBuilderSegment( _listBegin, _listBegin, _listEnd ) );

  // if there's an unterminated document, we have to move it
  if( _locationCountPointer ) {
    memcpy( _list, _documentPointer, documentCopyAmount );
    assert( memset( _documentPointer, 0xcd, lastListEnd - _documentPointer ) );
    // update the _locationCountPointer
    _locationCountPointer = _listBegin + (_locationCountPointer - _documentPointer);
    _list = _listBegin + documentCopyAmount;
    _documentPointer = _listBegin;
  } else {
    _documentPointer = 0;
  }

  assert( !_locationCountPointer || _listBegin < _locationCountPointer );
  assert( !_locationCountPointer || _listEnd > _locationCountPointer );
  assert( !_locationCountPointer || _list > _locationCountPointer );
  assert( _listEnd >= _list );
  assert( !_documentPointer || _listBegin <= _documentPointer );
  assert( !_documentPointer || _listEnd > _documentPointer );
  assert( _lists.back().data <= _lists.back().capacity );
}

//
// _terminateDocument
//

void indri::index::DocExtentListMemoryBuilder::_terminateDocument() {
  assert( _locationCountPointer );
  int locations = _extentFrequency - _lastExtentFrequency;
  int locationsSize = lemur::utility::RVLCompress::compressedSize( locations );

  if( locationsSize > 1 ) {
    // have to move everything around to make room, because we need more than
    // one byte to store this length.
    assert( _list > _locationCountPointer );
    assert( _listEnd > _locationCountPointer );
    assert( _listBegin < _locationCountPointer );

    memmove( _locationCountPointer + locationsSize,
             _locationCountPointer + 1,
             _list - _locationCountPointer - 1 );

    _list += locationsSize - 1;
    assert( _list <= _listEnd );
  }

  // we left one byte around for the location count for the common case
  lemur::utility::RVLCompress::compress_int( _locationCountPointer, locations );
  _documentFrequency++;
  _lastExtentFrequency = _extentFrequency;
  _locationCountPointer = 0;
  _lastLocation = 0;
  _lastOrdinal = 0;
  _documentPointer = 0;

  assert( !_locationCountPointer );
  assert( _list <= _listEnd );
}

//
// _safeAddLocation
//

void indri::index::DocExtentListMemoryBuilder::_safeAddLocation( lemur::api::DOCID_T documentID, int begin, int end, INT64 number, int ordinal, int parent ) {
  assert( !_locationCountPointer || _listBegin < _locationCountPointer );
  assert( !_locationCountPointer || _listEnd > _locationCountPointer );
  assert( !_locationCountPointer || _list > _locationCountPointer );
  assert( !_ordinal || ordinal > 0 );
  assert( !_parental || parent >= 0 );

  bool hasPointer = _locationCountPointer ? true : false;
  lemur::api::DOCID_T lastdoc = _lastDocument;

  // if this is a new document, put a document header in (and terminate the old one)
  if( _lastDocument != documentID ) {
    if( _locationCountPointer )
      _terminateDocument();
    
    _documentPointer = _list;
    _list = lemur::utility::RVLCompress::compress_int( _list, documentID - _lastDocument );
    _locationCountPointer = _list;

    // leave a byte for location of extent count
    _list++;
    _lastDocument = documentID;
    _lastLocation = 0;
    _lastOrdinal = 0;
    _lastExtentFrequency = _extentFrequency;
  }

  _list = lemur::utility::RVLCompress::compress_int( _list, begin - _lastLocation );
  _list = lemur::utility::RVLCompress::compress_int( _list, end - begin );
  _lastLocation = begin;
  _extentFrequency++;

  if( _ordinal ) {
    _list = lemur::utility::RVLCompress::compress_int( _list, ordinal - _lastOrdinal );
    _lastOrdinal = ordinal;
  }

  if( _parental) {
    _list = lemur::utility::RVLCompress::compress_int( _list, parent );
  }
  
  if( _numeric )
    _list = lemur::utility::RVLCompress::compress_longlong( _list, number );

  assert( _locationCountPointer );
  assert( _listBegin < _locationCountPointer );
  assert( _listEnd > _locationCountPointer );
  assert( _list > _locationCountPointer );
  assert( _listEnd >= _list );
}

//
// _compressedSize
//

size_t indri::index::DocExtentListMemoryBuilder::_compressedSize( lemur::api::DOCID_T documentID, int begin, int end, INT64 number, int ordinal, int parent ) {
  size_t size = 0;

  if( _lastDocument != documentID ) {
    size += lemur::utility::RVLCompress::compressedSize( documentID - _lastDocument ) + 1;
    size += lemur::utility::RVLCompress::compressedSize( begin );
    size += lemur::utility::RVLCompress::compressedSize( end - begin );
    size += lemur::utility::RVLCompress::compressedSize( _extentFrequency - _lastExtentFrequency ) - 1;
    
    if( _ordinal )
      size += lemur::utility::RVLCompress::compressedSize( ordinal );

    if ( _parental ) 
      size += lemur::utility::RVLCompress::compressedSize( parent );

    if( _numeric )
      size += lemur::utility::RVLCompress::compressedSize( number );
  } else {
    size += lemur::utility::RVLCompress::compressedSize( begin - _lastLocation );
    size += lemur::utility::RVLCompress::compressedSize( end - begin );

    if( _ordinal )
      size += lemur::utility::RVLCompress::compressedSize( ordinal - _lastOrdinal );

    if ( _parental ) 
      size += lemur::utility::RVLCompress::compressedSize( parent );

    if( _numeric )
      size += lemur::utility::RVLCompress::compressedSize( number );
  }

  return size;
}

//
// _growAddLocation
//

void indri::index::DocExtentListMemoryBuilder::_growAddLocation( lemur::api::DOCID_T documentID, int begin, int end, INT64 number, int ordinal, int parent, size_t newDataSize ) {
  // have to copy the last document if it's not complete, or if there's not enough room to complete it
  bool documentMismatch = (_lastDocument != documentID);
  bool terminateSpace = (lemur::utility::RVLCompress::compressedSize( _extentFrequency - _lastExtentFrequency ) - 1) <= _listEnd - _list;

  // by terminating the document now, we save a document copy and a bit of space
  if( _locationCountPointer && terminateSpace && documentID != _lastDocument )
    _terminateDocument();

  // grow the list, adding space for a document if necessary
  _grow();

  assert( newDataSize <= size_t(_listEnd - _list) );
  _safeAddLocation( documentID, begin, end, number, ordinal, parent );
}

//
// addLocation
//

void indri::index::DocExtentListMemoryBuilder::addLocation( lemur::api::DOCID_T documentID, int begin, int end, INT64 number, int ordinal, int parent ) {
  size_t remaining = _listEnd - _list;
  assert( _listEnd >= _list );

  if( remaining >= PLENTY_OF_SPACE ) {
    // common case -- lots of memory; just compress the posting and shove it in
    _safeAddLocation( documentID, begin, end, number, ordinal, parent );
  } else {
    size_t size = _compressedSize( documentID, begin, end, number, ordinal, parent );

    if( remaining >= size ) {
      _safeAddLocation( documentID, begin, end, number, ordinal, parent );
    } else {
      _growAddLocation( documentID, begin, end, number, ordinal, parent, size );
    }
  }

  assert( _listEnd >= _list );
}

//
// flush
//

void indri::index::DocExtentListMemoryBuilder::flush() {
  if( _locationCountPointer ) {
    // need to terminate document
    bool terminateSpace = (lemur::utility::RVLCompress::compressedSize( _extentFrequency - _lastExtentFrequency ) - 1) <= _listEnd - _list;

    if( !terminateSpace )
      _grow();

    _terminateDocument();
  }

  if( _lists.size() ) {
    _lists.back().data = _list;
    assert( _lists.back().data <= _lists.back().capacity );
  }

  assert( _documentPointer == 0 );
  assert( _locationCountPointer == 0 );

  for( size_t i=0; i<_lists.size(); i++ ) {
    assert( _lists[i].base <= _lists[i].capacity );
    assert( _lists[i].base <= _lists[i].data );
    assert( _lists[i].data <= _lists[i].capacity );
  }
}

//
// memorySize
//

size_t indri::index::DocExtentListMemoryBuilder::memorySize() const {
  size_t total = 0;

  // the lists follow the sequence MIN_SIZE, MIN_SIZE*2, MIN_SIZE*4, etc.
  // so, total size is (2^(lists+1)-1)*MIN_SIZE.
  int truncLists = std::min<int>( int(_lists.size()), GROW_TIMES );
  total = ((1 << (truncLists+1)) - 1) * MIN_SIZE;

  // each remaining list is max size
  int remainingLists = std::max<int>( (int)_lists.size() - GROW_TIMES, 0 );
  total += (MIN_SIZE << GROW_TIMES) * remainingLists;

  return total;
}

//
// extentFrequency
//

int indri::index::DocExtentListMemoryBuilder::extentFrequency() const {
  return _extentFrequency;
}

//
// documentFrequency
//

int indri::index::DocExtentListMemoryBuilder::documentFrequency() const {
  return _documentFrequency;
}

//
// getIterator
//

indri::index::DocExtentListMemoryBuilderIterator* indri::index::DocExtentListMemoryBuilder::getIterator() {
  flush();
  return new DocExtentListMemoryBuilderIterator( *this );
}

