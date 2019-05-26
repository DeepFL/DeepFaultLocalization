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
// Path.hpp
//
// 20 May 2004 -- tds
//


#ifndef INDRI_PATH_HPP
#define INDRI_PATH_HPP

#include <string>
namespace indri
{
  namespace file
  {
    
    class Path {
    public:
      static void create( const std::string& path );
      static bool isFile( const std::string& path );
      static bool isDirectory( const std::string& path );
      static bool exists( const std::string& path );
      static void remove( const std::string& path );
      static void rename( const std::string& oldPath, const std::string& newPath );
      static void make( const std::string& path );

      static std::string trim( const std::string& path );
      static std::string relative( const std::string& basePath, const std::string absolutePath );

      static char pathSeparator();

      static std::string combine( const std::string& root, const std::string& addition );
      static std::string extension( const std::string& path );
      static std::string directory( const std::string& path );
      static std::string filename( const std::string& path );
      static std::string basename( const std::string& path );
    };
  }
}

#endif // INDRI_PATH_HPP

