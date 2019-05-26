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
// Path.cpp
//
// 20 May 2004 -- tds
//

#include <string>
#include "indri/Path.hpp"
#include "lemur/Exception.hpp"
#include "lemur/lemur-compat.hpp"
#include <sys/stat.h>
#include <errno.h>
#include <iostream>
#include <stack>
#include "indri/DirectoryIterator.hpp"
#include "indri/delete_range.hpp"
#include "indri/XMLNode.hpp"
#include "indri/XMLReader.hpp"

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

static int path_last_separator( const std::string& path ) {
  int i;

  // skip any trailing slashes
  for( i=(int)path.size()-1; i>=0; i-- ) {
    if( path[i] != PATH_SEPARATOR )
      break;
  }

  for( ; i>=0; i-- ) {
    if( path[i] == PATH_SEPARATOR )
      break;
  }

  return i;
}

void indri::file::Path::create( const std::string& path ) {
  if( lemur_compat::mkdir( path.c_str(), 0777 ) < 0 ) {
    if( errno == EACCES ) {
      LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't create directory: '" + path + "' because of inadequate permissions." );
    } else if( errno == ENOENT ) {
      LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't create directory: '" + path + "' because at least one of the parent directories does not exist." );
    } else if( errno == EEXIST ) {
      LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't create directory: '" + path + "' because something already exists there." );
    }
  }
}

void indri::file::Path::rename( const std::string& oldName, const std::string& newName ) {
#ifndef WIN32
  int result = ::rename( oldName.c_str(), newName.c_str() );

  if( result != 0 ) {
    if( errno == EEXIST ) {
      LEMUR_THROW( LEMUR_IO_ERROR, "The destination file already exists: " + oldName );
    } else if( errno == EACCES || errno == EPERM ) {
      LEMUR_THROW( LEMUR_IO_ERROR, "Insufficient permissions to rename: '" + oldName + "' to '" + newName + "'." );
    } else {
      LEMUR_THROW( LEMUR_IO_ERROR, "Unable to rename: '" + oldName + "' to '" + newName + "'." );
    }
  }
#else
  BOOL result;

  if( Path::exists( newName ) ) {
    result = ReplaceFile( newName.c_str(), oldName.c_str(), NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL );
  } else {
    result = MoveFile( oldName.c_str(), newName.c_str() );
  }

  if( !result ) {
    LEMUR_THROW( LEMUR_IO_ERROR, "Unable to rename: '" + oldName + "' to '" + newName + "'." );
  }
#endif
}

void indri::file::Path::remove( const std::string& path ) {
  std::stack<indri::file::DirectoryIterator*> iterators;
  indri::utility::StackDeleter<indri::file::DirectoryIterator> sd( iterators );
  iterators.push( new indri::file::DirectoryIterator( path ) );

  while( iterators.size() ) {
    indri::file::DirectoryIterator* top = iterators.top();
    
    // all done, so go up a level
    if( (*top) == indri::file::DirectoryIterator::end() ) {
      // release any search handles that may point
      // to this directory
      top->close();

      int result = rmdir( top->base().c_str() );
      if( result != 0 )
        LEMUR_THROW( LEMUR_IO_ERROR, "indri::file::Path::remove couldn't remove directory '" + top->base() + "'." );

      delete top;
      iterators.pop();
      continue;
    }

    std::string path = **top;
    (*top)++;

    if( indri::file::Path::isFile( path ) ) {
      int result = lemur_compat::remove( path.c_str() );
      if( result != 0 )
        LEMUR_THROW( LEMUR_IO_ERROR, "indri::file::Path::remove couldn't remove file '" + path + "'." );
    } else {
      iterators.push( new indri::file::DirectoryIterator( path ) );
    }
  }
}

std::string indri::file::Path::trim( const std::string& path ) {
  if( path.size() && path[path.length()-1] == PATH_SEPARATOR ) {
    return path.substr( 0, path.length()-1 );
  }

  return path;
}

std::string indri::file::Path::relative( const std::string& basePath, const std::string absolutePath ) {
  std::string relativePath = absolutePath.substr( basePath.length() );

  while( relativePath.length() && relativePath[0] == PATH_SEPARATOR )
    relativePath = relativePath.substr(1);

  return relativePath;
}

void indri::file::Path::make( const std::string& path ) {
  if( !indri::file::Path::isDirectory( path ) ) {
    std::string parent = indri::file::Path::directory( path );
    if( path == parent )
      return;

    indri::file::Path::make( parent );
  }

  lemur_compat::mkdir( path.c_str(), 0755 );
}

char indri::file::Path::pathSeparator() {
  return PATH_SEPARATOR;
}

bool indri::file::Path::isDirectory( const std::string& path ) {
  struct stat s;
  int result = stat( path.c_str(), &s );
  bool actualDirectory = (result >= 0) && (s.st_mode & S_IFDIR);

  return actualDirectory;
}

bool indri::file::Path::exists( const std::string& path ) {
  struct stat s;
  return stat( path.c_str(), &s ) >= 0;
}

bool indri::file::Path::isFile( const std::string& path ) {
  struct stat s;
  int result = stat( path.c_str(), &s );
  bool actualFile = (result >= 0) && (s.st_mode & S_IFREG);

  return actualFile;
}

std::string indri::file::Path::combine( const std::string& root, const std::string& addition ) {
  if( !root.size() )
    return addition;

  if( *(root.end()-1) == PATH_SEPARATOR )
    return root + addition;

  return root + PATH_SEPARATOR + addition;
}

std::string indri::file::Path::directory( const std::string& path ) {
  int last = path_last_separator( path );
  
  if( last > 0 ) {
    return path.substr( 0, last );
  }

  return path;
}

std::string indri::file::Path::filename( const std::string& path ) {
  int last = path_last_separator( path );

  if( last != std::string::npos ) {
    return path.substr( last+1 );
  }

  return path;
}

std::string indri::file::Path::extension( const std::string& path ) {
  int last = path_last_separator( path );
  std::string::size_type lastDot = path.find_last_of( '.' );

  if( int(lastDot) > last ) {
    return path.substr( lastDot+1 );
  }

  return std::string();
}

std::string indri::file::Path::basename( const std::string& path ) {
  int last = path_last_separator( path );
  std::string::size_type lastDot = path.find_last_of( '.' );

  if( int(lastDot) > last ) {
    return path.substr( 0, lastDot );
  }

  return path;
}


