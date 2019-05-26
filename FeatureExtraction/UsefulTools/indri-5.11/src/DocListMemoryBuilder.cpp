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
// DocListMemoryBuilder.cpp
//
// tds - 17 December 2003
//

#include "indri/DocListMemoryBuilder.hpp"
#include "lemur/lemur-compat.hpp"
#include "lemur/RVLCompress.hpp"

const int MIN_SIZE = 128;
const int GROW_TIMES = 11;
const size_t PLENTY_OF_SPACE = 15; // docID, count, position: 5 bytes each
const size_t TERMINATE_SPACE = 4; // need enough in case we have to grow
const size_t LOCATION_SPACE = 5; // need enough in case we have to grow

//
// DocListMemoryBuilder constructor
//

indri::index::DocListMemoryBuilder::DocListMemoryBuilder( indri::utility::RegionAllocator* allocator ) :
  _list(0),
  _listBegin(0),
  _listEnd(0),
  _documentPointer(0),
  _locationCountPointer(0),
  _lastLocation(0),
  _lastDocument(0),
  _lastTermFrequency(0),
  _termFrequency(0),
  _documentFrequency(0),
  _allocator(allocator)
{
}

//
// DocListMemoryBuilder destructor
//

indri::index::DocListMemoryBuilder::~DocListMemoryBuilder() {
}

//
// _roundUP
//
size_t indri::index::DocListMemoryBuilder::_roundUp( size_t amount ) {
  
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

void indri::index::DocListMemoryBuilder::_grow() {
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
  size_t newSize = MIN_SIZE << iterations;

  // ensure we have enough space for a location after copying.
  newSize = std::max<unsigned int>( newSize, _roundUp( documentCopyAmount + LOCATION_SPACE) );

  _list = (char*) _allocator->allocate( newSize );
  _listBegin = _list;
  _listEnd = _list + newSize;

  _lists.push_back( DocListMemoryBuilderSegment( _listBegin, _listBegin, _listEnd ) );

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
}

//
// _terminateDocument
//

inline void indri::index::DocListMemoryBuilder::_terminateDocument() {
  assert( _locationCountPointer );
  int locations = _termFrequency - _lastTermFrequency;
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
  _lastTermFrequency = _termFrequency;
  _locationCountPointer = 0;
  _lastLocation = 0;
  _documentPointer = 0;

  assert( !_locationCountPointer );
}

//
// _safeAddLocation
//

inline void indri::index::DocListMemoryBuilder::_safeAddLocation( int position ) {
  assert( !_locationCountPointer || _listBegin < _locationCountPointer );
  assert( !_locationCountPointer || _listEnd > _locationCountPointer );
  assert( !_locationCountPointer || _list > _locationCountPointer );

  _list = lemur::utility::RVLCompress::compress_int( _list, position - _lastLocation );
  _lastLocation = position;
  _termFrequency++;

  assert( _locationCountPointer );
  assert( _listBegin < _locationCountPointer );
  assert( _listEnd > _locationCountPointer );
  assert( _list > _locationCountPointer );
  assert( _listEnd >= _list );
}

//
// startDocument
// 

void indri::index::DocListMemoryBuilder::startDocument( int documentID ) {
  size_t remaining = size_t(_listEnd - _list);

  if( remaining < PLENTY_OF_SPACE )
    _grow();

  _documentPointer = _list;
  _list = lemur::utility::RVLCompress::compress_int( _list, documentID - _lastDocument );
  _locationCountPointer = _list;    
  _list++;
  _lastDocument = documentID;
  _lastLocation = 0;
}

//
// endDocument
//

void indri::index::DocListMemoryBuilder::endDocument() {
  size_t remaining = size_t(_listEnd - _list);

  // comparison to constant saves some work in the common case
  if( remaining < TERMINATE_SPACE &&
      remaining < ((size_t)lemur::utility::RVLCompress::compressedSize( _termFrequency - _lastTermFrequency ) - 1) )
    {
      _grow();
    }

  _terminateDocument();
}

//
// addLocation
//

void indri::index::DocListMemoryBuilder::addLocation( int position ) {
  size_t remaining = size_t(_listEnd - _list);
  assert( _listEnd >= _list );

  if( remaining < LOCATION_SPACE ) {
    size_t size = lemur::utility::RVLCompress::compressedSize( position - _lastLocation );

    if( remaining < size ) {
      _grow();
    }
  }

  _safeAddLocation( position );

  assert( _listEnd >= _list );
}

//
// flush
//

void indri::index::DocListMemoryBuilder::flush() {
  if( _locationCountPointer ) {
    // need to terminate document
    bool terminateSpace = (lemur::utility::RVLCompress::compressedSize( _termFrequency - _lastTermFrequency ) - 1) <= _listEnd - _list;

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

size_t indri::index::DocListMemoryBuilder::memorySize() const {
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
// documentFrequency
//

int indri::index::DocListMemoryBuilder::documentFrequency() const {
  return _documentFrequency;
}

//
// termFrequency
//

int indri::index::DocListMemoryBuilder::termFrequency() const {
  return _termFrequency;
}
