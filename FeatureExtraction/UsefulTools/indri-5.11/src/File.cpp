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
// File
//
// 15 November 2004 -- tds
//
#include <cstdlib>
#include <string>
#include "indri/indri-platform.h"
#include "indri/File.hpp"

#ifndef WIN32
#include <fcntl.h>
#include <sys/stat.h>
#endif

#include "lemur/Exception.hpp"
#include "indri/ScopedLock.hpp"

//
// File constructor
//

indri::file::File::File() :
#ifdef WIN32
  _handle(INVALID_HANDLE_VALUE)
#else
  _handle(-1)
#endif
{
}

//
// File destructor
//

indri::file::File::~File() {
  close();
}

//
// create
// 

bool indri::file::File::create( const std::string& filename ) {
#ifdef WIN32
  _handle = ::CreateFile( filename.c_str(),
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL );

  if( _handle == INVALID_HANDLE_VALUE )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't create: " + filename );

  return true;
#else 
  // let the user's umask decide perimissions
  _handle = creat( filename.c_str(), 0666 );

  if( _handle < 0 )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't create: " + filename );

  return true;
#endif
}

bool indri::file::File::open( const std::string& filename ) {
#ifdef WIN32
  _handle = ::CreateFile( filename.c_str(),
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL );
  
  if( _handle == INVALID_HANDLE_VALUE )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open: " + filename );
  
  return true;
#else 
#ifndef O_LARGEFILE
  _handle = ::open( filename.c_str(), O_RDWR );
#else
  _handle = ::open( filename.c_str(), O_LARGEFILE | O_RDWR );
#endif
  
  if( _handle < 0 )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open: " + filename );

  return true;
#endif
}

bool indri::file::File::openRead( const std::string& filename ) {
#ifdef WIN32
  _handle = ::CreateFile( filename.c_str(),
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL );
  
  if( _handle == INVALID_HANDLE_VALUE )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open for reading: " + filename );

  return true;
#else 
#ifndef O_LARGEFILE
  _handle = ::open( filename.c_str(), O_RDONLY );
#else
  _handle = ::open( filename.c_str(), O_LARGEFILE | O_RDONLY );
#endif
  
  if( _handle < 0 )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open for reading: " + filename );
  
  return true;
#endif
}

bool indri::file::File::openTemporary( std::string& fileName ) {
#ifdef HAVE_MKSTEMP
  char name[] = "/tmp/indriXXXXXX";
  _handle = ::mkstemp( name );
  fileName = name;

  if( _handle < 0 )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't create temporary file." );
#else
  fileName = tmpnam( NULL );
  open( fileName );
#endif

  return true;
}

size_t indri::file::File::read( void* buffer, UINT64 position, size_t length ) {
#ifdef WIN32
  assert( _handle != INVALID_HANDLE_VALUE );

  indri::thread::ScopedLock sl( _mutex );
  LARGE_INTEGER actual;
  BOOL result;
  LARGE_INTEGER largePosition;
  largePosition.QuadPart = position;

  result = SetFilePointerEx( _handle, largePosition, &actual, FILE_BEGIN ); 
  assert( largePosition.QuadPart == actual.QuadPart );

  if( !result )
    LEMUR_THROW( LEMUR_IO_ERROR, "Failed to seek to some file position" );

  DWORD actualBytes;
  result = ::ReadFile( _handle, buffer, length, &actualBytes, NULL );

  if( !result )
    LEMUR_THROW( LEMUR_IO_ERROR, "Error when reading file" );

  return actualBytes;
#else // POSIX
  assert( _handle > 0 );
  ssize_t result = ::pread( _handle, buffer, length, position ); 

  if( result < 0 )
    LEMUR_THROW( LEMUR_IO_ERROR, "Error when reading file" );

  return size_t(result);
#endif
}

size_t indri::file::File::write( const void* buffer, UINT64 position, size_t length ) {
  if( length == 0 )
    return 0;

#ifdef WIN32
  assert( _handle != INVALID_HANDLE_VALUE );

  indri::thread::ScopedLock sl( _mutex );
  LARGE_INTEGER actual;
  BOOL result;
  LARGE_INTEGER largePosition;
  largePosition.QuadPart = position;

  result = SetFilePointerEx( _handle, largePosition, &actual, FILE_BEGIN ); 
  assert( largePosition.QuadPart == actual.QuadPart );

  if( !result )
    LEMUR_THROW( LEMUR_IO_ERROR, "Failed to seek to some file position" );

  DWORD actualBytes;
  result = ::WriteFile( _handle, buffer, length, &actualBytes, NULL );

  if( !result )
    LEMUR_THROW( LEMUR_IO_ERROR, "Error when writing file" );

  return actualBytes;
#else // POSIX
  assert( _handle > 0 );

  ssize_t result = ::pwrite( _handle, buffer, length, position );

  if( result < 0 )
    LEMUR_THROW( LEMUR_IO_ERROR, "Error when writing file" );

  return size_t(result);
#endif
}

void indri::file::File::close() {
#ifdef WIN32
  if( _handle != INVALID_HANDLE_VALUE ) {
    ::CloseHandle( _handle );
    _handle = INVALID_HANDLE_VALUE;
  }
#else
  if( _handle >= 0 ) {
    ::close( _handle );
    _handle = -1;
  }
#endif
}

UINT64 indri::file::File::size() {
#ifdef WIN32
  if( _handle == INVALID_HANDLE_VALUE )
    return 0;

  LARGE_INTEGER length;
  BOOL result = ::GetFileSizeEx( _handle, &length );

  if( !result )
    LEMUR_THROW( LEMUR_IO_ERROR, "Got an error while trying to retrieve file size" );

  return length.QuadPart;
#else // POSIX
  struct stat stats;
  
  if( _handle == -1 )
    return 0;

  fstat( _handle, &stats );
  return stats.st_size;
#endif
}

