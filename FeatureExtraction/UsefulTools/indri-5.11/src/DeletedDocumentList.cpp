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
// DeletedDocumentList
//
// 3 February 2005 -- tds
//

#include "indri/DeletedDocumentList.hpp"
#include "indri/File.hpp"
#include "lemur/Exception.hpp"
#include "indri/ScopedLock.hpp"
#include "indri/Path.hpp"
#include "lemur/IndexTypes.hpp"

//
// DeletedDocumentList constructor
//

indri::index::DeletedDocumentList::DeletedDocumentList() :
  _modified( false ),
  _readLock( _lock ),
  _writeLock( _lock ),
  _deletedCount( 0 )
{
}

//
// read_transaction constructor
//

indri::index::DeletedDocumentList::read_transaction::read_transaction( DeletedDocumentList& list ) :
  _lock(list._lock),
  _bitmap(list._bitmap)
{
  _lock.lockRead();
}

//
// ~read_transaction constructor
//

indri::index::DeletedDocumentList::read_transaction::~read_transaction() {
  _lock.unlockRead();
}

//
// grow
//

void indri::index::DeletedDocumentList::_grow( lemur::api::DOCID_T documentID ) {
  // just set an appropriate number of bytes to zero
  int growBytes = (documentID/8)+1 - _bitmap.position();
  if( growBytes <= 0 )
    return;
  
  memset( _bitmap.write( growBytes ), 0, growBytes );
  assert( _bitmap.position() > (documentID/8) );
}

//
// deletedCount
//

UINT64 indri::index::DeletedDocumentList::deletedCount() const {
  return _deletedCount;
}

//
// append
//
// Takes another DeletedDocumentList and appends it to this one.
// documentCount refers to the number of documents handled by this
// deleted list _before_ the new list is appended.
//

void indri::index::DeletedDocumentList::append( DeletedDocumentList& other, int documentCount ) {
  indri::thread::ScopedLock l( _writeLock );

  if( other._bitmap.size() == 0 )
    return;

  // We handle the first byte as a special case, then copy the rest in the loop
  UINT8 otherFirstByte = *(UINT8*)other._bitmap.front();

  size_t otherBytes = other._bitmap.position();
  _grow( documentCount + (otherBytes+1)*8 );
  assert( _bitmap.size() > 0 );

  int shift = documentCount % 8;

  if( shift == 0 ) {
    // Data is aligned on byte boundaries, so it's easy to copy it
    ::memcpy( _bitmap.front() + documentCount/8, other._bitmap.front(), other._bitmap.position() );
  } else {
    // Unaligned case
    size_t myPosition = documentCount / 8;
    size_t otherPosition = 0;

    // copy the first byte as a special case (since it contains both new and old bits)
    UINT8* lastLocalByteLocation = (UINT8*) (_bitmap.front() + myPosition);
    UINT8 lastLocalByte = *lastLocalByteLocation;

    assert( shift != 0 );
    *lastLocalByteLocation = lastLocalByte | (otherFirstByte << shift);
    myPosition += 1;

    UINT32 accumulator = (otherFirstByte >> (8-shift));

    while( otherPosition < otherBytes ) {
      // add the next byte to the accumulator
      assert( otherPosition < other._bitmap.size() );
      UINT8 nextByte = *(other._bitmap.front() + otherPosition);
      accumulator |= (nextByte << shift);

      // copy the low bits of the accumulator
      assert( myPosition < _bitmap.size() );
      *(UINT8*) (_bitmap.front() + myPosition) = (UINT8) (accumulator & 0xFF);

      myPosition += 1;
      otherPosition += 1;
      accumulator >>= 8;
    }

    // copy the remaining bits
    assert( myPosition < _bitmap.size() );
    *(UINT8*) (_bitmap.front() + myPosition) = (UINT8) (accumulator & 0xFF);
  }

  _deletedCount += other.deletedCount();
}

//
// nextDocument
//

lemur::api::DOCID_T indri::index::DeletedDocumentList::read_transaction::nextCandidateDocument( lemur::api::DOCID_T documentID ) {
  _lock.yieldRead();

  while( documentID < (lemur::api::DOCID_T)_bitmap.position()*8 ) {
    char bitmapByte = _bitmap.front()[documentID/8];
    bool marked = (bitmapByte & 1<<(documentID%8)) != 0;

    if( !marked )
      break;

    documentID++;
  }

  return documentID;
}

//
// isDeleted
//

bool indri::index::DeletedDocumentList::read_transaction::isDeleted( lemur::api::DOCID_T documentID ) const {
  if( (lemur::api::DOCID_T)_bitmap.position() < (documentID/8)+1 )
    return false;

  char bitmapByte = _bitmap.front()[documentID/8];
  bool marked = (bitmapByte & 1<<(documentID%8)) != 0;

  return marked;
}

//
// markDeleted
//

void indri::index::DeletedDocumentList::markDeleted( lemur::api::DOCID_T documentID ) {
  _modified = true;
  indri::thread::ScopedLock l( _writeLock );

  if( (lemur::api::DOCID_T)_bitmap.position() < (documentID/8)+1 ) {
    _grow( documentID );
  }

  UINT8 bit = 1<<(documentID%8);

  if( !(_bitmap.front()[documentID/8] & bit) ) {
    _deletedCount++;
    _bitmap.front()[documentID/8] |= bit;
  }
}

//
// isDeleted
//

bool indri::index::DeletedDocumentList::isDeleted( lemur::api::DOCID_T documentID ) {
  if ( _deletedCount == 0 ) return false;
  
  indri::thread::ScopedLock l( _readLock );
  if( (lemur::api::DOCID_T)_bitmap.position() < (documentID/8)+1 )
    return false;

  char bitmapByte = _bitmap.front()[documentID/8];
  bool marked = (bitmapByte & 1<<(documentID%8)) != 0;

  return marked;
}

//
// getReadTransaction
//

indri::index::DeletedDocumentList::read_transaction* indri::index::DeletedDocumentList::getReadTransaction() {
  return new read_transaction( *this );
}

//
// _calculateDeletedCount
//

void indri::index::DeletedDocumentList::_calculateDeletedCount() {
  int bitCount[256];
  
  // set up bit table
  for( int i=0; i<256; i++ ) {
    int bits = 0;
    
    for( int j=0; j<8; j++ ) {
      if( i & (1<<j) )
        bits++;
    }

    bitCount[i] = bits;
  }

  // scan the bytes, add up the bits
  UINT64 total = 0;
  unsigned char *front = (unsigned char *)_bitmap.front();
  for( size_t i=0; i<_bitmap.position(); i++ ) {
    unsigned char idx = front[i];
    total += bitCount[idx];
  }

  _deletedCount = total;
}

//
// read
//

void indri::index::DeletedDocumentList::read( const std::string& filename ) {
  indri::file::File file;

  if( !file.openRead( filename ) )
    LEMUR_THROW( LEMUR_IO_ERROR, "Unable to open file: " + filename );

  UINT64 fileSize = file.size();
  _bitmap.clear();
  file.read( _bitmap.write( fileSize ), 0, fileSize );
  file.close();

  // count number of bits set:
  _calculateDeletedCount();
}

//
// write
//

void indri::index::DeletedDocumentList::write( const std::string& filename ) {
  if ( _modified || ! indri::file::Path::exists( filename ) ) {
    indri::file::File file;

    if( indri::file::Path::exists( filename ) )
      indri::file::Path::remove( filename );
    if( !file.create( filename ) )
      LEMUR_THROW( LEMUR_IO_ERROR, "Unable to create file: "  + filename );

    file.write( _bitmap.front(), 0, _bitmap.position() );
    file.close();
  }
}
