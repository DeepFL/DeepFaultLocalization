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
// BulkTree
//
// 4 March 2005 -- tds
//

#include "lemur/lemur-compat.hpp"
#include "indri/indri-platform.h"
#include <assert.h>
#include <vector>
#include "indri/File.hpp"
#include "indri/delete_range.hpp"
#include "indri/BulkTree.hpp"
#include "lemur/lemur-platform.h"
#include <iostream>

// add pair at begin block, add length at end of block
// 

const int BULK_BLOCK_SIZE = 8*1024;

inline int indri::file::BulkBlock::_remainingCapacity() {
  int startDataSize = _dataEnd();
  return BULK_BLOCK_SIZE - startDataSize - count()*2*sizeof(UINT16);
}

inline int indri::file::BulkBlock::_dataEnd() {
  return _valueEnd( count()-1 );
}

inline int indri::file::BulkBlock::_keyEnd( int index ) {
  assert( index < count() );

  if( index <= -1 ) {
    return sizeof(UINT16);
  }

  UINT16* blockEnd = (UINT16*) (_buffer + BULK_BLOCK_SIZE);
  UINT16  keyEnd = blockEnd[ -(index*2+2) ];

  return keyEnd;
}

inline int indri::file::BulkBlock::_keyStart( int index ) {
  return _valueEnd( index-1 );
}

inline int indri::file::BulkBlock::_valueStart( int index ) {
  return _keyEnd( index );
}

inline int indri::file::BulkBlock::_valueEnd( int index ) {
  assert( index < count() );

  if( index <= -1 ) {
    return sizeof(UINT16);
  }

  UINT16* blockEnd = (UINT16*) (_buffer + BULK_BLOCK_SIZE);
  UINT16  valueEnd = blockEnd[ -(index*2+1) ];

  return valueEnd;
}

inline bool indri::file::BulkBlock::_canInsert( int keyLength, int dataLength ) {
  return _remainingCapacity() >= (keyLength + dataLength + 2*int(sizeof(UINT16)));
}

inline void indri::file::BulkBlock::_storeKeyValueLength( int insertPoint, int keyLength, int valueLength ) {
  UINT16* blockEnd = (UINT16*) (_buffer + BULK_BLOCK_SIZE);
  int cnt = count();

  blockEnd[ -(cnt*2+2) ] = (UINT16) keyLength + insertPoint;
  blockEnd[ -(cnt*2+1) ] = (UINT16) valueLength + keyLength + insertPoint;
}

inline int indri::file::BulkBlock::_compare( const char* one, int oneLength, const char* two, int twoLength ) {
  int result = memcmp( one, two, lemur_compat::min( oneLength, twoLength ) );

  if( result != 0 ) {
    return result;
  }

  return oneLength - twoLength;
}

inline int indri::file::BulkBlock::_find( const char* key, int keyLength, bool& exact ) {
  int left = 0;
  int right = count() - 1;

  while( right - left > 1 ) {
    int middle = left + (right - left) / 2;
    int middleKeyStart = _keyStart( middle );
    int middleKeyEnd = _keyEnd( middle );
    const char* middleKey = _buffer + middleKeyStart;

    int result = _compare( key, keyLength, middleKey, middleKeyEnd - middleKeyStart );

    if( result < 0 ) {
      right = middle;
    } else if( result > 0 ) {
      left = middle;
    } else {
      exact = true;
      return middle;
    }
  }

  const char* leftKey = _keyStart( left ) + _buffer;
  int leftLength = _keyEnd( left ) - _keyStart( left );
  int leftResult = _compare( key, keyLength, leftKey, leftLength );

  const char* rightKey = _keyStart( right ) + _buffer;
  int rightLength = _keyEnd( right ) - _keyStart( right );
  int rightResult = _compare( key, keyLength, rightKey, rightLength );

  // matches the left key
  if( leftResult == 0 ) {
    exact = true;
    return left;
  }

  // matches the right key
  if( rightResult == 0 ) {
    exact = true;
    return right;
  }

  // bigger than the right key; choose right
  if( rightResult > 0 ) {
    exact = false;
    return right;
  }

  // smaller than the left key; invalid!
  if( leftResult < 0 ) {
    exact = false;
    return -1;
  }

  exact = false;
  return left;
}


indri::file::BulkBlock::BulkBlock( bool leaf ) {
  _buffer = new char[BULK_BLOCK_SIZE];
  *(UINT16*) _buffer = (leaf ? (1<<15) : 0);
  _previous = _next = 0;
}

indri::file::BulkBlock::~BulkBlock() {
  delete[] _buffer;
}

inline int indri::file::BulkBlock::count() {
  return (*(UINT16*)_buffer) & ~(1<<15);
}

inline bool indri::file::BulkBlock::leaf() {
  return ((*(UINT16*)_buffer) & (1<<15)) != 0;
}

bool indri::file::BulkBlock::insert( const char* key, int keyLength, const char* data, int dataLength ) {
  if( _canInsert( keyLength, dataLength ) == false )
    return false;
  
  int insertPoint = _dataEnd();
  memcpy( _buffer + insertPoint, key, keyLength );
  memcpy( _buffer + insertPoint + keyLength, data, dataLength );

  _storeKeyValueLength( insertPoint, keyLength, dataLength );
  (*(UINT16*)_buffer) += 1;

  return true;
}

bool indri::file::BulkBlock::getIndex( int index, char* key, int& keyActual, int keyLength, char* value, int& valueActual, int valueLength ) {
  int count = *(UINT16*) _buffer;

  keyActual = 0;
  valueActual = 0;

  if( index < 0 || index >= count )
    return false;

  if( key ) {
    int keyStart = _keyStart( index );
    int keyEnd = _keyEnd( index );

    keyActual = lemur_compat::min( keyEnd - keyStart, keyLength );
    memcpy( key, _buffer + keyStart, keyActual );
  }

  if( value ) {
    int valueStart = _valueStart( index );
    int valueEnd = _valueEnd( index );
    
    valueActual = lemur_compat::min( valueEnd - valueStart, valueLength );
    memcpy( value, _buffer + valueStart, valueActual );
  }

  return true;
}

bool indri::file::BulkBlock::findGreater( const char* key, int keyLength, char* value, int& actualLength, int valueBufferLength ) {
  bool exact;
  int index = _find( key, keyLength, exact );
  actualLength = 0;

  if( index < 0 )
    return false;

  int keyActual;
  return getIndex( index, 0, keyActual, 0, value, actualLength, valueBufferLength );
}

bool indri::file::BulkBlock::find( const char* key, int keyLength, char* value, int& actualLength, int valueBufferLength ) {
  bool exact;
  int index = _find( key, keyLength, exact );
  actualLength = 0;

  if( index < 0 || !exact )
    return false;

  int keyActual;
  return getIndex( index, 0, keyActual, 0, value, actualLength, valueBufferLength );
}

//
// findIndexOf
//
int indri::file::BulkBlock::findIndexOf(const char* key) {
  bool exact;
  return _find( key, (int)strlen(key), exact );
}


bool indri::file::BulkBlock::insertFirstKey( indri::file::BulkBlock& block, UINT32 blockID ) {
  assert( block.count() > 0 );

  int startKey = block._keyStart( 0 );
  int endKey = block._keyEnd( 0 );

  return insert( block._buffer + startKey, endKey - startKey, (const char*) &blockID, sizeof(blockID) );
}

inline void indri::file::BulkBlock::clear() {
  *(UINT16*) _buffer = (leaf() ? (1<<15) : 0);
}

inline char* indri::file::BulkBlock::data() {
  return _buffer;
}

UINT64 indri::file::BulkBlock::dataSize() {
  return BULK_BLOCK_SIZE;
}

//
// getID
//

UINT32 indri::file::BulkBlock::getID() {
  return _id;
}

//
// setID

void indri::file::BulkBlock::setID( UINT32 id ) {
  _id = id;
}
//

//
// link
//

void indri::file::BulkBlock::link( indri::file::BulkBlock* previous, indri::file::BulkBlock* next ) {
  _previous = previous;
  _next = next;

  if( previous )
    previous->_next = this;

  if( next )
    next->_previous = this;
}

//
// unlink
//

void indri::file::BulkBlock::unlink() {
  if( _previous )
    _previous->_next = _next;

  if( _next )
    _next->_previous = _previous;

  _next = _previous = 0;
}

//
// next
//

indri::file::BulkBlock* indri::file::BulkBlock::next() {
  return _next;
}

//
// previous
//

indri::file::BulkBlock* indri::file::BulkBlock::previous() {
  return _previous;
}


// ==============
// BulkTreeWriter
// ==============


void indri::file::BulkTreeWriter::_flush( int blockIndex ) {
  indri::file::BulkBlock& flusher = *_blocks[blockIndex];

  if( blockIndex < (int)_blocks.size() - 1 ) {
    indri::file::BulkBlock& parent = *_blocks[blockIndex+1];
    
    if( ! parent.insertFirstKey( flusher, _blockID ) ) {
      _flush( blockIndex+1 );
      parent.insertFirstKey( flusher, _blockID );
    } 
  } else {
    _blocks.push_back( new indri::file::BulkBlock );
    _blocks.back()->insertFirstKey( flusher, _blockID );
  }

  _write.write( flusher.data(), indri::file::BulkBlock::dataSize() );
  flusher.clear();
  _blockID++;
  _flushLevel = lemur_compat::max( blockIndex, _flushLevel );
}

void indri::file::BulkTreeWriter::_flushAll() {
  // note: _flushLevel may grow during this loop
  int originalSize = (int)_blocks.size();

  for( int i=0; i<(int)_blocks.size(); i++ ) {
    bool hasNotBeenFlushed = (i > _flushLevel);
    int count = _blocks[i]->count();

    if( count == 1 && hasNotBeenFlushed )
      break;
    
    if( count )
      _flush( i );
  }
}

indri::file::BulkTreeWriter::BulkTreeWriter() :
  _write( _file, 1024*1024 )
{
  _blockID = 0;
  _blocks.push_back( new BulkBlock(true) );
  _flushLevel = 0;
}

indri::file::BulkTreeWriter::~BulkTreeWriter() {
  indri::utility::delete_vector_contents( _blocks );
}

void indri::file::BulkTreeWriter::create( const std::string& filename ) {
  _file.create( filename );
}

void indri::file::BulkTreeWriter::put( UINT32 key, const char* value, int valueLength ) {
  key = htonl( key );
  put( (const char*) &key, sizeof(key), value, valueLength );
}

void indri::file::BulkTreeWriter::put( const char* key, const char* value, int valueLength ) {
  put( key, (int)strlen(key), value, valueLength );
}

void indri::file::BulkTreeWriter::put( const char* key, int keyLength, const char* value, int valueLength ) {
  bool simple = _blocks.front()->insert( key, keyLength, value, valueLength );

  if( !simple ) {
    _flush( 0 );
    _blocks.front()->insert( key, keyLength, value, valueLength );
  }
}

bool indri::file::BulkTreeWriter::get( const char* key, int keyLength, char* value, int& actual, int valueLength ) {
  indri::file::BulkTreeReader reader( _file, _blockID*indri::file::BulkBlock::dataSize() );
  return reader.get( key, keyLength, value, actual, valueLength );
}

bool indri::file::BulkTreeWriter::get( const char* key, char* value, int& actual, int valueLength ) {
  indri::file::BulkTreeReader reader( _file, _blockID*indri::file::BulkBlock::dataSize() );
  return reader.get( key, value, actual, valueLength );
}

bool indri::file::BulkTreeWriter::get( UINT32 key, char* value, int& actual, int valueLength ) {
  indri::file::BulkTreeReader reader( _file, _blockID*indri::file::BulkBlock::dataSize() );
  return reader.get( key, value, actual, valueLength );
}

void indri::file::BulkTreeWriter::close() { 
  _flushAll();
  _write.flush();
  _file.close();
}

void indri::file::BulkTreeWriter::flush() {
  _flushAll();
  _write.flush();
}

// ==============
// BulkTreeReader
// ==============

indri::file::BulkBlock* indri::file::BulkTreeReader::_fetch( UINT32 id ) {
  assert( id < _fileLength / indri::file::BulkBlock::dataSize() );
  indri::file::BulkBlock** result = _cache.find( id );
  indri::file::BulkBlock* block;

  if( !result ) {
    if( _cache.size() >= 256 ) {
      block = _tail;
      _tail = block->previous();
      _cache.remove( block->getID() );
    } else {
      block = new indri::file::BulkBlock;
    }

    _file->read( block->data(), id*indri::file::BulkBlock::dataSize(), indri::file::BulkBlock::dataSize() );
    block->setID( id );
    _cache.insert( id, block );
  } else {
    block = *result;
    
    if( _tail == block )
      _tail = block->previous();
  }

  // move to front of list
  block->unlink();
  block->link( 0, _head );
  if( _tail == 0 )
    _tail = block;
  _head = block;

  assert( _cache.size() <= 256 );
  
  return block;
}

indri::file::BulkTreeReader::BulkTreeReader( File& file ) :
  _file(&file),
  _ownFile(false),
  _head(0),
  _tail(0)
{
}

indri::file::BulkTreeReader::BulkTreeReader( File& file, UINT64 length ) :
  _file(&file),
  _ownFile(false),
  _fileLength(length),
  _head(0),
  _tail(0)
{
}

indri::file::BulkTreeReader::BulkTreeReader() :
  _ownFile(false),
  _file(0),
  _head(0),
  _tail(0)
{
}

indri::file::BulkTreeReader::~BulkTreeReader() {
  indri::utility::HashTable< UINT32, indri::file::BulkBlock* >::iterator iter;

  for( iter = _cache.begin(); iter != _cache.end(); iter++ ) {
    delete *iter->second;
  }
}

void indri::file::BulkTreeReader::openRead( const std::string& filename ) {
  _file = new File;
  _file->openRead( filename );
  _fileLength = _file->size();
  _ownFile = true;
}

void indri::file::BulkTreeReader::close() {
  if( _ownFile ) {
    _file->close();
    delete _file;
  }
}

bool indri::file::BulkTreeReader::get( const char* key, int keyLength, char* value, int& actual, int valueLength ) {
  indri::file::BulkBlock* block = 0;
  int rootID = int(_fileLength / BULK_BLOCK_SIZE) - 1;

  if( rootID < 0 )
    return false;

  int nextID = rootID;

  while( true ) {
    block = _fetch( nextID );

    if( block->leaf() )
      break;

    int actual;
    bool result = block->findGreater( key, keyLength, (char*) &nextID, actual, sizeof(nextID) );

    if( !result )
      return false;

    assert( actual == sizeof(nextID) );
  }

  // now we're at a leaf
  return block->find( key, keyLength, value, actual, valueLength );
}

bool indri::file::BulkTreeReader::get( const char* key, char* value, int& actual, int valueLength ) {
  return get(key, (int)strlen(key), value, actual, valueLength);
}

bool indri::file::BulkTreeReader::get( UINT32 key, char* value, int& actual, int valueLength ) {
  key = htonl( key );
  return get( (const char*) &key, sizeof(key), value, actual, valueLength );
}

indri::file::BulkTreeIterator* indri::file::BulkTreeReader::iterator() {
  return new BulkTreeIterator( *_file );
}

//
// findFirst
//
indri::file::BulkTreeIterator* indri::file::BulkTreeReader::findFirst(const char *key) {
  indri::file::BulkBlock* block = 0;
  int rootID = int(_fileLength / BULK_BLOCK_SIZE) - 1;

  if( rootID < 0 )
    return NULL;

  int nextID = rootID;

  int keyLength=(int)strlen(key);

  while( true ) {
    block = _fetch( nextID );

    if( block->leaf() )
      break;

    int actual;
    bool result = block->findGreater( key, keyLength, (char*) &nextID, actual, sizeof(nextID) );

    if( !result )
      return NULL;

    assert( actual == sizeof(nextID) );
  }

  // now we're at a leaf
  // we've got the block ID, now get the index ID of the entry we want.
  UINT64 thisBlockID=(UINT64)block->getID();
  int thisPairIndex=block->findIndexOf(key);

  return new BulkTreeIterator(*_file, thisBlockID, thisPairIndex);
}


// ================
// BulkTreeIterator
// ----------------

indri::file::BulkTreeIterator::BulkTreeIterator( File& file ) :
  _file(file)
{
  _pairIndex = -1;
  _blockIndex = 0;
  _fileLength = 0;
}

indri::file::BulkTreeIterator::BulkTreeIterator( File& file, UINT64 whichBlock, int whichPair ) :
  _file(file)
{
  _pairIndex=whichPair;
  _blockIndex=whichBlock;
  _fileLength = _file.size();

  if (finished()) {
    // we're past the last block!
    _pairIndex = -1;
    _blockIndex = 0;
  } else {
    if ((!readCurrentBlockData()) || (_pairIndex < 0) || (_pairIndex >= (_block.count()-1))) {
      // invalid pair index...
      _pairIndex = -1;
      _blockIndex = 0;
    }
  }
}

bool indri::file::BulkTreeIterator::readCurrentBlockData() {
  return (_file.read( _block.data(), _blockIndex*indri::file::BulkBlock::dataSize(), indri::file::BulkBlock::dataSize() ) > 0);
}

void indri::file::BulkTreeIterator::startIteration() {
  _pairIndex = -1;
  _blockIndex = 0;
  _fileLength = _file.size();

  nextEntry();
}

bool indri::file::BulkTreeIterator::finished() {
  // if we're pointing past the last block in the file, we're done
  return _blockIndex == (_fileLength / indri::file::BulkBlock::dataSize());
}

void indri::file::BulkTreeIterator::nextEntry() {
  if( finished() )
    return;
  
  // pairIndex is less than zero when we're just starting out
  // pairIndex == _block.count() - 1 at the last entry in the block
  if( _pairIndex < 0 || _pairIndex >= _block.count()-1 ) {
    do {
      // look for a suitable leaf
      if( _pairIndex >= 0 )
        _blockIndex++;
      _pairIndex = 0;
      _file.read( _block.data(), _blockIndex*indri::file::BulkBlock::dataSize(), indri::file::BulkBlock::dataSize() );
    } while( !finished() && _block.leaf() == false );
  } else {
    _pairIndex++;
  }
}

bool indri::file::BulkTreeIterator::get( char* key, int keyLength, int& keyActual, char* value, int valueLength, int& valueActual ) {
  if( finished() )
    return false;
  
  // the length / actual items were switched around previously!
  return _block.getIndex( _pairIndex, key, keyActual, keyLength, value, valueActual, valueLength);
}

bool indri::file::BulkTreeIterator::get( UINT32& key, char* value, int valueLength, int& valueActual ) {
  if( finished() )
    return false;

  key = 0;
  int keyActual;
  bool result = _block.getIndex( _pairIndex, (char*) &key, keyActual, sizeof key, value, valueActual, valueLength );
  key = ntohl( key );
  return result;
}

