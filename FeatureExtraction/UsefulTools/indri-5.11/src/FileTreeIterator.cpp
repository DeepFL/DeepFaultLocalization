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
// FileTreeIterator
//
// 26 May 2004 -- tds
//

#include "indri/FileTreeIterator.hpp"
#include "indri/Path.hpp"
#include <iostream>

// static construction
indri::file::FileTreeIterator indri::file::FileTreeIterator::_end;

indri::file::FileTreeIterator::FileTreeIterator() {
}

indri::file::FileTreeIterator::FileTreeIterator( const std::string& path ) {
  indri::file::DirectoryIterator *top = new DirectoryIterator( path );
  if( *top == indri::file::DirectoryIterator::end() ) {
    // either the path is an empty directory or 
    // it is not a directory, so delete it.
    delete top;
  } else {
    _stack.push( top );
    // PRE: a non empty directory iterator is on the stack
    while( _stack.size() && 
           indri::file::Path::isDirectory( *(*_stack.top()) ) ) {
      // the top iterator may be at end, returning an invalid value
      top = _stack.top();
      if( *top == indri::file::DirectoryIterator::end() ) {
        // advance to the next non-empty iterator on the stack
        _nextCandidate();
      } else {
        // add a new, possibly empty, iterator to the stack
        _stack.push( new DirectoryIterator( *(*_stack.top()) ) );
      }
    }
    // POST: stack is empty OR top of stack is a non empty iterator
    //       pointing to a file (non-directory) entry.
  }
}

indri::file::FileTreeIterator::~FileTreeIterator() {
  while( _stack.size() ) {
    delete _stack.top();
    _stack.pop();
  }
}

void indri::file::FileTreeIterator::_nextCandidate() {
  // go to the next file.  If the current directory is complete,
  // go up levels until we find a directory with stuff left in it
  while( _stack.size() ) {
    indri::file::DirectoryIterator& top = (*_stack.top());
    top++;

    if( top == indri::file::DirectoryIterator::end() ) {
      delete _stack.top();
      _stack.pop();
    } else {
      break;
    }
  }
}

void indri::file::FileTreeIterator::_next() {
  _nextCandidate();

  // need to make sure we've found a file
  while( _stack.size() ) {
    indri::file::DirectoryIterator& top = (*_stack.top());
    
    if( top == indri::file::DirectoryIterator::end() ) {
      _nextCandidate();
      continue;
    } 

    if( indri::file::Path::isFile( *top ) ) {
      // found a file, so we're done
      break;
    }

    // have to recurse
    // only if a directory
    if ( indri::file::Path::isDirectory( *top ) ) {
      indri::file::DirectoryIterator* child = new indri::file::DirectoryIterator( *top );
      _stack.push(child);
    } else {
      // bad things happening here, not a file, not a directory
      // perhaps a bad symlink, skip the entry
      _nextCandidate();
    }
  }
}

void indri::file::FileTreeIterator::operator ++ ( int ) {
  _next();
}

void indri::file::FileTreeIterator::operator ++ () {
  _next();
}

const std::string& indri::file::FileTreeIterator::operator* () {
  DirectoryIterator& top = (*_stack.top());
  return *top;
}

bool indri::file::FileTreeIterator::operator== ( const indri::file::FileTreeIterator& other ) const {
  return ( &other == &_end ) && ( _stack.size() == 0 );
}

bool indri::file::FileTreeIterator::operator!= ( const indri::file::FileTreeIterator& other ) const {
  return ! this->operator== ( other );
}

const indri::file::FileTreeIterator& indri::file::FileTreeIterator::end() {
  return _end;
}
