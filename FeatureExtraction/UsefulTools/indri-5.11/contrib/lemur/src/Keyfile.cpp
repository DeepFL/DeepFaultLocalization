
//
// Keyfile.cpp
//
// 18 September 2003 - tds
// 30 March 2004 - dmf -- update to new 8 bit clean keyfile api.
//

#include <cassert>
#include <memory>
#include <string>
#include <sstream>
#include "Keyfile.hpp"
#include "Exception.hpp"

#include "lemur-platform.h"
#include "lemur-compat.hpp"

extern "C" {
#include "keyref.h"
#include "keydef.h"
#include "keyerr.h"
}

void lemur::file::Keyfile::_buildHandle( int cacheSize ) {
  int blocks = lemur_compat::max( int((cacheSize - min_fcb_lc) / buffer_lc),
                                  int(0) );
  _handleSize = min_fcb_lc + blocks * buffer_lc;
  _handle = new char[ _handleSize ];
  memset( _handle, 0x00, _handleSize );
}

void lemur::file::Keyfile::open( const char* filename, int cacheSize, bool readOnly) {
  _buildHandle( cacheSize );
  
  int error = open_key( _handle, const_cast<char*>(filename), 
                        _handleSize, readOnly ? 1 : 0);

  if( error )
    LEMUR_THROW(LEMUR_KEYFILE_IO_ERROR, "Unable to open '" + filename + "'");
}

void lemur::file::Keyfile::open(const std::string& filename, int cacheSize, bool readOnly){
  open( filename.c_str(), cacheSize, readOnly );
}

void lemur::file::Keyfile::openRead( const std::string& filename, int cacheSize ) {
  open( filename, cacheSize, true );
}

void lemur::file::Keyfile::create( const char* filename, int cacheSize ) {
  _buildHandle( cacheSize );
  
  int error = create_key( _handle, const_cast<char*>(filename), 
                          _handleSize );

  if( error )
    LEMUR_THROW(LEMUR_KEYFILE_IO_ERROR, "Unable to create '" + filename + "'");
}

void lemur::file::Keyfile::create( const std::string& filename, int cacheSize ) {
  create( filename.c_str(), cacheSize );
}

void lemur::file::Keyfile::close() {
  // don't close an unopened key.
  if (_handle != NULL) close_key( _handle);
  delete[](_handle);
  _handle = NULL;
}

bool lemur::file::Keyfile::get( const char* key, char** value, int& actualSize ) const {
  char *buffer; 

  // clear parameters in case of size <= 0
  *value = 0;
  actualSize = 0;

  int size = getSize( key );

  if( size > 0 ) {
    // make a buffer to handle the record
    buffer = new char[size];
    get(key, buffer, actualSize, size);
    *value = buffer; 
  } 

  return size > 0;
}

bool lemur::file::Keyfile::get( const char* key, void* value, int& actualSize, 
                                int maxSize ) const {
  assert( key && "key cannot be null" );
  assert( value && "value cannot be null" );
  assert( _handle && "call open() or create() first" );
  assert( maxSize > 0 && "maxSize must be positive" );
  int len = strlen(key); // fix for UTF-8
  
  int error = get_rec( _handle, const_cast<char*>(key), len,
                       (char*)value, &actualSize, maxSize );

  if( error && error != getnokey_err )
    LEMUR_THROW( LEMUR_KEYFILE_IO_ERROR, "Caught an internal error while getting record for key: " + key );

  return error != getnokey_err;
}

void lemur::file::Keyfile::put( const char* key, const void* value, int valueSize ) {
  assert( key && "key cannot be null" );
  assert( value && "value cannot be null" );
  int len = strlen(key); // fix for UTF-8
  int error = put_rec( _handle,
                       const_cast<char*>(key), len,
                       static_cast<char*>(const_cast<void*>(value)),
                       valueSize );

  if( error )
    LEMUR_THROW( LEMUR_KEYFILE_IO_ERROR, "Caught an internal error while putting record for key: " + key );
}

bool lemur::file::Keyfile::next( char* key, int& keyLength, char* value, int& valueLength ) {
  assert( key && "key cannot be null" );
  assert( value && "value cannot be null" );

  int error = next_rec( _handle,
                        key,
                        &keyLength,
                        keyLength,
                        value,
                        &valueLength,
                        valueLength );

  if( error && error != ateof_err && !check_fcb((struct fcb *)_handle) )
    LEMUR_THROW( LEMUR_KEYFILE_IO_ERROR, "Caught an internal error while trying to fetch next record." );

  return error != ateof_err;
}

bool lemur::file::Keyfile::previous( char* key, int& keyLength, char* value, int& valueLength ) {
  assert( key && "key cannot be null" );
  assert( value && "value cannot be null" );

  int error = prev_rec( _handle,
                        key,
                        &keyLength,
                        keyLength,
                        value,
                        &valueLength,
                        valueLength );

  if( error && error != ateof_err && !check_fcb((struct fcb *)_handle)) 
    LEMUR_THROW( LEMUR_KEYFILE_IO_ERROR, "Caught an internal error while trying to fetch previous record." );

  return error != ateof_err;
}

void lemur::file::Keyfile::remove( const char* key ) {
  assert( key && "key cannot be null" );
  assert( _handle && "call open() or create() first" );
  int len = strlen(key); // fix for UTF-8
  int error = delete_rec( _handle, const_cast<char*>(key), len );

  if( error )
    LEMUR_THROW( LEMUR_KEYFILE_IO_ERROR, "Unable to delete record for key: " + key );
}

int lemur::file::Keyfile::getSize( const char* key ) const {
  assert( key && "key cannot be null" );
  assert( _handle && "call open() or create() first" );
  char pointer[buffer_lc];
  int size;
  int len = strlen(key); // fix for UTF-8
  int error = get_ptr( _handle, const_cast<char*>(key), len, pointer );

  if( error ) {
    if( error == getnokey_err ) {
      size =  -1;
    } else {
      LEMUR_THROW( LEMUR_KEYFILE_IO_ERROR, "Encountered an error while trying to fetch record size for: " + key );
    }
  } else {
    size = keyrec_lc( pointer );
  }

  return size;
}

//
// keyfile_create_key
//
// Converts an integer into a null-terminated string so
// that it can be used as a keyfile key.
//    keyBuf - character buffer to receive string key (must be at least 6 bytes long)
//    number - integer key to convert
//

#define KEYFILE_KEYBUF_SIZE (7)
#define KEYFILE_BUFFER_SHIFT(num,dig) (num >> ((5-dig)*6))
#define KEYFILE_BUFFER_DIGIT(num,dig)  ( (KEYFILE_BUFFER_SHIFT(num,dig) | 1<<6) & ~(1<<7) )

inline void lemur::file::Keyfile::_createKey( char* keyBuf, int number ) const {
  keyBuf[6] = 0;
  keyBuf[5] = KEYFILE_BUFFER_DIGIT( number, 5 );
  keyBuf[4] = KEYFILE_BUFFER_DIGIT( number, 4 );
  keyBuf[3] = KEYFILE_BUFFER_DIGIT( number, 3 );
  keyBuf[2] = KEYFILE_BUFFER_DIGIT( number, 2 );
  keyBuf[1] = KEYFILE_BUFFER_DIGIT( number, 1 );
  keyBuf[0] = KEYFILE_BUFFER_DIGIT( number, 0 );
}

inline int lemur::file::Keyfile::_decodeKey( char* keyBuf ) const {
  return ((keyBuf[5] & 0x3f) << 6*0) |
    ((keyBuf[4] & 0x3f) << 6*1) |
    ((keyBuf[3] & 0x3f) << 6*2) |
    ((keyBuf[2] & 0x3f) << 6*3) |
    ((keyBuf[1] & 0x3f) << 6*4) |
    ((keyBuf[0] & 0x3f) << 6*5);
}

void lemur::file::Keyfile::put( int key, const void* value, int valueLength ) {
  // have to handle the 0 key. Bleah.
  char keyBuf[KEYFILE_KEYBUF_SIZE];
  _createKey( keyBuf, key );
  put( keyBuf, value, valueLength );
}

bool lemur::file::Keyfile::get( int key, void* value, int& actualSize, int maxSize ) const {
  char keyBuf[KEYFILE_KEYBUF_SIZE];
  _createKey( keyBuf, key );
  return get( keyBuf, value, actualSize, maxSize );
}


bool lemur::file::Keyfile::get( int key, char** value, int& actualSize ) const {
  char keyBuf[KEYFILE_KEYBUF_SIZE];
  _createKey( keyBuf, key );
  return get( keyBuf, value, actualSize );
}

bool lemur::file::Keyfile::previous( int& key, char* value, int& valueLength ) {
  char keyBuf[KEYFILE_KEYBUF_SIZE];
  int keyLength = 6;
  bool result = false;

  try {
    result = previous( keyBuf, keyLength, value, valueLength ); 
  } catch (lemur::api::Exception &e) {
    key = _decodeKey( keyBuf );
    LEMUR_RETHROW( e, "Caught an internal error while trying to fetch previous record with an int key." );
  }
  
  if( result )
    key = _decodeKey( keyBuf );
  return result;
}

bool lemur::file::Keyfile::next( int& key, char* value, int& valueLength ) {
  char keyBuf[KEYFILE_KEYBUF_SIZE];
  int keyLength = KEYFILE_KEYBUF_SIZE;
  bool result = false ;
  try {
    
    result = next( keyBuf, keyLength, value, valueLength ); 
  } catch (lemur::api::Exception &e) {
    key = _decodeKey( keyBuf );
    LEMUR_RETHROW( e, "Caught an internal error while trying to fetch next record with an int key." );
  }
  
  if( result )
    key = _decodeKey( keyBuf );
  return result;
}

void lemur::file::Keyfile::remove( int key ) {
  char keyBuf[KEYFILE_KEYBUF_SIZE];
  _createKey( keyBuf, key );
  remove( keyBuf );
}

int lemur::file::Keyfile::getSize( int key ) const {
  char keyBuf[KEYFILE_KEYBUF_SIZE];
  _createKey( keyBuf, key );
  return getSize( keyBuf );
}

void lemur::file::Keyfile::setFirst() {
  set_bof(_handle);
}

bool lemur::file::Keyfile::getNext( char* key, int maxKeySize, void* value, 
                                    int& actualSize, int maxValueSize ) const {
  int len; // for key length
  int result = next_rec( _handle, key, &len, maxKeySize,
                         value, &actualSize, maxValueSize );
  // null terminate
  key[len] = '\0';
  return !result;
}

bool lemur::file::Keyfile::getNext( char* key, int &len, int maxKeySize, void* value, 
                                    int& actualSize, int maxValueSize ) const {
  int result = next_rec( _handle, key, &len, maxKeySize,
                         value, &actualSize, maxValueSize );
  return !result;
}

bool lemur::file::Keyfile::getNext( int& key, void* value, int& actualSize, 
                                    int maxValueSize ) const {
  char keyBuf[KEYFILE_KEYBUF_SIZE];
  bool result = getNext( keyBuf, KEYFILE_KEYBUF_SIZE, 
                         value, actualSize, maxValueSize );
  
  if( result )
    key = _decodeKey( keyBuf );

  return result;
}
