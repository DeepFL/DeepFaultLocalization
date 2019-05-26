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
// DirectoryIterator
//
// 26 May 2004 -- tds
//

#include "indri/DirectoryIterator.hpp"
#include "lemur/lemur-platform.h"
#include <assert.h>
#include "indri/Path.hpp"
#include <vector>
#include <iostream>
#include <cstdlib>

// static construction
indri::file::DirectoryIterator indri::file::DirectoryIterator::_end;

void* directoryiterator_init( const std::string& path );
void directoryiterator_destroy(void* opaque);
std::string directoryiterator_current();
bool directoryiterator_next(void* opaque);
bool directoryiterator_done(void* opaque);


#ifdef LEMUR_USING_FINDFIRST

//
// Windows platform-specific directory iterator code
//

struct win_iter_data {
  HANDLE handle;
  WIN32_FIND_DATA data;
  bool done;
};

void* directoryiterator_init( const std::string& path ) {
  win_iter_data* d = new win_iter_data;
  std::string searchPath = indri::file::Path::combine( path, "*" );

  d->handle = ::FindFirstFile( searchPath.c_str(), &d->data );
  d->done = (d->handle == INVALID_HANDLE_VALUE);

  return d;
}

void directoryiterator_destroy( void* opaque ) {
  win_iter_data* d = (win_iter_data*) opaque;
  ::FindClose( d->handle );
  delete d;
}

std::string directoryiterator_current( void* opaque ) {
  assert( opaque );
  win_iter_data* d = (win_iter_data*) opaque;
  return d->data.cFileName;
}

bool directoryiterator_next( void* opaque ) {
  assert( opaque );
  win_iter_data* d = (win_iter_data*) opaque;

  if( !d->done )
    d->done = (::FindNextFile( d->handle, &d->data ) != TRUE);

  return d->done;
}

bool directoryiterator_done( void* opaque ) {
  assert( opaque );
  return ( (win_iter_data*) opaque )->done;
}

#else

#include <sys/types.h>
#include <dirent.h>
#include <limits.h>

//
// Linux/Unix platform specific directory iterator code
//

struct unix_iter_data {
  DIR* directory;
  bool done;
  struct dirent* entry;
};

#ifdef HAS_READDIR_R 
void directoryiterator_dirent_init( unix_iter_data& d ) {
  d.entry = (struct dirent*) malloc( sizeof(struct dirent) + PATH_MAX );
}

void directoryiterator_dirent_destroy( unix_iter_data& d ) {
  free( d.entry );
}

bool directoryiterator_next( void* opaque ) {
  unix_iter_data* d = (unix_iter_data*) opaque;
  struct dirent* result = 0;
  readdir_r( d->directory, d->entry, &result );  
  d->done = ( result ? false : true );
  return d->done;
}
#else
void directoryiterator_dirent_init( unix_iter_data& d ) {
  d.entry = 0;
}

void directoryiterator_dirent_destroy( unix_iter_data& d ) {
}

bool directoryiterator_next( void* opaque ) {
  unix_iter_data* d = (unix_iter_data*) opaque;
  d->entry = readdir( d->directory );  
  d->done = ( d->entry ? false : true );
  return d->done;
}
#endif // HAS_READDIR_R

void* directoryiterator_init( const std::string& path ) {
  unix_iter_data* d = new unix_iter_data;
  d->directory = opendir( path.c_str() );
  d->done = (d->directory == 0);
  d->entry = (struct dirent*) malloc( sizeof(struct dirent) + PATH_MAX + 1);

  if( !d->done )
    directoryiterator_next(d);

  return d;
}

void directoryiterator_destroy( void* opaque ) {
  unix_iter_data* d = (unix_iter_data*) opaque;
  if (d->directory) closedir( d->directory );
  free( d->entry );
  delete d;
}

std::string directoryiterator_current( void* opaque ) {
  unix_iter_data* d = (unix_iter_data*) opaque;
  if (! d->entry) return "";
  return d->entry->d_name;
}

bool directoryiterator_done( void* opaque ) {
  unix_iter_data* d = (unix_iter_data*) opaque;
  return d->done;
}

#endif // LEMUR_USING_FINDFIRST

//
// Begin platform independent code
//

const indri::file::DirectoryIterator& indri::file::DirectoryIterator::end() {
  return _end;
}

void indri::file::DirectoryIterator::_copyCurrent() {
  if( _relative ) {
    _current = indri::file::Path::combine( _path, directoryiterator_current( _platform ) );
  } else {
    _current = directoryiterator_current( _platform );
  }

  _current = indri::file::Path::trim( _current );
}

void indri::file::DirectoryIterator::_next() {
  directoryiterator_next( _platform );
  _copyCurrent();

  std::string current = directoryiterator_current( _platform );

  if( !directoryiterator_done( _platform ) &&
      (current == "." || current == "..") ) {
    _next();
  }
}

indri::file::DirectoryIterator::DirectoryIterator() :
  _relative(false),
  _platform(0)
{
}

indri::file::DirectoryIterator::DirectoryIterator( const std::string& path, bool relative ) :
  _relative(relative)
{
  _path = indri::file::Path::trim( path );
  _platform = directoryiterator_init( _path );

  std::string current = directoryiterator_current( _platform );

  if( !directoryiterator_done( _platform ) &&
      (current == "." || current == "..") ) {
    _next();
  }

  _copyCurrent();
}

indri::file::DirectoryIterator::~DirectoryIterator() {
  close();
}

const std::string& indri::file::DirectoryIterator::base() const {
  return _path;
}

void indri::file::DirectoryIterator::close() {
  if( _platform ) {
    directoryiterator_destroy( _platform );
    _platform = 0;
  }
}

void indri::file::DirectoryIterator::operator ++ () {
  if( !directoryiterator_done( _platform ))
    _next();
}

void indri::file::DirectoryIterator::operator ++ (int) {
  if( !directoryiterator_done( _platform ))
    _next();
}

bool indri::file::DirectoryIterator::operator == ( const DirectoryIterator& other ) {
  // this is a hack, but I think it's decent: we assume
  // the user is comparing against end() in an iteration sense
  return directoryiterator_done( _platform ) && &other == &_end;
}

const std::string& indri::file::DirectoryIterator::operator * () {
  return _current;
}
