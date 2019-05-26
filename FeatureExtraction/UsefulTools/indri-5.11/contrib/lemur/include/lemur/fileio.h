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
#ifndef FILEIO_H
#define FILEIO_H

#ifdef WIN32
#pragma warning( disable : 4133 )
  #define NOGDI
  #define _WIN32_WINNT 0x0400
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  typedef __int64 FILE_OFFSET;
  /* F_HANDLE should have the handle
     Keep the position field pending addition of 
     buffered io behavior of fread/fwrite.
     Otherwise it is unnecessary.
  */
  typedef struct F_HANDLE {
    HANDLE handle;
    FILE_OFFSET position;
  } F_HANDLE;
/* fopen a file */ 
F_HANDLE *file_open( char *fname, char *mode);
/* fclose a FILE */
void file_close( F_HANDLE *file );
/* fseeko a FILE */
int file_seek( F_HANDLE *file, FILE_OFFSET position, int whence );
/* ftello a FILE */
FILE_OFFSET file_tell( F_HANDLE *file );
/* fread a FILE */
size_t file_read( void* buffer, size_t size, 
                  size_t number, F_HANDLE *file );
/* fwrite a file */
size_t file_write( void* buffer, size_t size, 
                   size_t number, F_HANDLE *file );
#define fopen file_open
#define fclose file_close
#define fseeko file_seek
#define ftello file_tell
#define fread file_read
#define fwrite file_write
#define FILE F_HANDLE
#else
#include <sys/types.h>
typedef off_t FILE_OFFSET;
#endif
#endif // FILEIO_H
