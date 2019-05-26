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
#ifdef WIN32
#include <stdlib.h>
#include <stdio.h>
#include "fileio.h"
#include <stdarg.h>

F_HANDLE * file_open( char *filename, char *mode ) {
  /* 
     mode is one of 
       "rb"  READ_ONLY   existing
       "rb+" READ_WRITE  existing
       "wb+" WRITE_TRUNC existing
   */
  size_t len = 0;
  F_HANDLE *retval = (F_HANDLE *)malloc(sizeof(F_HANDLE));
  DWORD shareFlags = FILE_SHARE_READ;
  DWORD rwFlags = GENERIC_READ;
  DWORD createFlags = OPEN_EXISTING;
  retval->handle= INVALID_HANDLE_VALUE;
  if (! mode) return retval;
  len = strlen(mode);

  if (len == 3) {
    shareFlags = 0;
    rwFlags |= GENERIC_WRITE;
    if (mode[0] == 'w') 
      createFlags = CREATE_ALWAYS;
  }
  
  retval->handle = CreateFile( filename, rwFlags, shareFlags, NULL,
                               createFlags, FILE_ATTRIBUTE_NORMAL,
                               NULL );
  retval->position = 0;
  return retval;
}

size_t file_read( void* buffer, size_t size, 
                  size_t num, F_HANDLE *_handle ) {

  BOOL result;
  DWORD actualBytes;
  DWORD length = size * num;
  /*
    LARGE_INTEGER largePosition;
    LARGE_INTEGER actual;
    largePosition.QuadPart = _handle->position;
    result = SetFilePointerEx( _handle->handle, largePosition, &actual, FILE_BEGIN ); 
  */
  result = ReadFile( _handle->handle, buffer, length, &actualBytes, NULL );

  if( !result )
    actualBytes = 0;
  /* position currently ignored.
  _handle->position += actualBytes;
  */
  return actualBytes/size;
}

size_t file_write( void* buffer, size_t size, 
                  size_t num, F_HANDLE *_handle ) {


  DWORD actualBytes;
  DWORD length = size * num;
  BOOL result;
  /*
  LARGE_INTEGER actual;
  LARGE_INTEGER largePosition;
  largePosition.QuadPart = _handle->position;
  */
  if( length == 0 )
    return 0;
/*
  result = SetFilePointerEx( _handle->handle, largePosition, &actual, FILE_BEGIN ); 
  if( !result )
    actualBytes = 0;
    */
  result = WriteFile( _handle->handle, buffer, length, &actualBytes, NULL );

  if( !result )
    actualBytes = 0;
  /* position currently ignored.
  _handle->position += actualBytes;  
  */
  return actualBytes/size;
}

void file_close(F_HANDLE *_handle) {
  if( _handle->handle != INVALID_HANDLE_VALUE ) {
    CloseHandle( _handle->handle );
  }
  free(_handle);
}


int file_seek( F_HANDLE *file, FILE_OFFSET position, int whence ) {
  DWORD move;
  LARGE_INTEGER actual;
  BOOL result;
  LARGE_INTEGER largePosition;
  switch (whence) {
  case SEEK_CUR:
    move = FILE_CURRENT;
    break;
  case SEEK_END:
    move = FILE_END;
    break;
  case SEEK_SET:
  default:
    move = FILE_BEGIN;
    break;
  }
  largePosition.QuadPart = position;
  result = SetFilePointerEx( file->handle, largePosition, &actual, move ); 
  if( !result )
    return -1;
  /* position currently ignored
  file->position = actual.QuadPart;
  */
  return 0;
}

FILE_OFFSET file_tell( F_HANDLE *file ) {
  DWORD move;
  LARGE_INTEGER actual;
  LARGE_INTEGER largePosition;
  BOOL result;
  if( file->handle != INVALID_HANDLE_VALUE ){
    move = FILE_CURRENT;
    largePosition.QuadPart = 0;
    result = SetFilePointerEx( file->handle, largePosition, &actual, move ); 
    /* position currently ignored
    return file->position;
    */
    return actual.QuadPart;
  } else {
    return -1;
  }
}
#endif
