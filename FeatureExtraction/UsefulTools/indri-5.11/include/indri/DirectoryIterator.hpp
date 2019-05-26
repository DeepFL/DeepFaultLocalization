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

#ifndef INDRI_DIRECTORYITERATOR_HPP
#define INDRI_DIRECTORYITERATOR_HPP

#include <string>
namespace indri
{
  namespace file
  {
    
    /*! Provides iteration over directory entries. 
     */
    class DirectoryIterator {
    private:
      static DirectoryIterator _end;

      bool _relative;
      std::string _current;
      void* _platform;
      std::string _path;

      void _copyCurrent();
      void _next();
  
    public:
      DirectoryIterator();
      
      /// Constructs a DirectoryIterator.  The iterator will step through
      /// the files in the directory named in the path parameter.  If 
      /// the relative flag is set to true, the iteration returns file names
      /// only; otherwise full paths to files are returned.
      DirectoryIterator( const std::string& path, bool relative = true );
      ~DirectoryIterator();

      /// Moves to the next file in the directory.
      void operator ++ (int);
      /// Moves to the next file in the directory.
      void operator ++ ();
      
      /// Used only for comparison with FileTreeIterator::end.  If false, the
      /// iteration is complete.
      bool operator == ( const DirectoryIterator& other );
      
      /// Returns the full path to a file in the directory if relative was
      /// set to false in the constructor (which is the default).  Otherwise,
      /// returns only the file name.
      const std::string& operator* ();
      
      /// Returns the path of this directory (the path passed to the constructor of
      /// this object).
      const std::string& base() const;
      
      /// Closes this iterator, releasing any operating system handles, etc.
      void close();
      
      /// Placeholder object that indicates a finished iteration.
      /// Use this with operator != to tell when iteration is finished.
      static const DirectoryIterator& end();
    };
  }
}

#endif // INDRI_DIRECTORYITERATOR_HPP

