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
// DiskTermListFileIterator
//
// 26 January 2005 -- tds 
//

#include "indri/DiskTermListFileIterator.hpp"

//
// DiskTermListFileIterator
//

indri::index::DiskTermListFileIterator::DiskTermListFileIterator( indri::file::File& termListFile ) :
  _termListFile(termListFile),
  _buffer(_termListFile, 1024*1024),
  _fileSize(termListFile.size()),
  _finished(false),
  _currentDocument(0)
{
}

//
// startIteration
//

void indri::index::DiskTermListFileIterator::startIteration() {
  _finished = false;
  nextEntry();
}

//
// currentEntry
//

indri::index::TermList* indri::index::DiskTermListFileIterator::currentEntry() {
  if( !finished() )
    return &_termList;

  return 0;
}

//
// nextEntry
//

bool indri::index::DiskTermListFileIterator::nextEntry() {
  if( _buffer.position() < _fileSize ) {
    UINT32 length;
    _buffer.read( &length, sizeof(UINT32) );
    _termList.read( (const char*) _buffer.read( length ), length );
    _currentDocument++;
    return true;
  }

  _finished = true;
  return false;
}

//
// nextEntry
//

bool indri::index::DiskTermListFileIterator::nextEntry( lemur::api::DOCID_T documentID ) {
  UINT32 length;

  if( _currentDocument >= documentID )
    return true;
  
  while( _currentDocument < documentID ) {
    if( _buffer.position() < _fileSize ) {
      _buffer.read( &length, sizeof(UINT32) );
      _currentDocument++;
    } else {
      _finished = true;
      return false;
    }
  }

  _termList.read( (const char*) _buffer.read( length ), length );
  return true;
}

//
// finished
//

bool indri::index::DiskTermListFileIterator::finished() {
  return _finished;
}
