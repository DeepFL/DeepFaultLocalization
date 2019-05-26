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
// MemoryIndexTermListFileIterator
//
// 24 November 2004 -- tds
//

#include "indri/MemoryIndexTermListFileIterator.hpp"
#include "indri/TermList.hpp"

indri::index::MemoryIndexTermListFileIterator::MemoryIndexTermListFileIterator( std::list<indri::utility::Buffer*>& buffers, std::vector<DocumentData>& data ) :
  _buffers(buffers),
  _data(data)
{
}

void indri::index::MemoryIndexTermListFileIterator::startIteration() {
  _list.clear();

  _buffersIterator = _buffers.begin();
  _bufferBase = 0;

  _finished = false;
  _index = -1;
  nextEntry();
}



bool indri::index::MemoryIndexTermListFileIterator::nextEntry() {
  _index++;

  if( _index >= (int)_data.size() ) {
    _finished = true;  
    return false;
  }

  DocumentData& data = _data[_index];

  // advance buffer iterator if necessary
  while( _bufferBase + (*_buffersIterator)->position() <= data.offset ) {
    _bufferBase += (*_buffersIterator)->position();
    _buffersIterator++;
  }
  assert( _buffersIterator != _buffers.end() );

  // determine the offset into that buffer
  size_t offset = data.offset - _bufferBase;

  // read the term list from there
  _list.read( (*_buffersIterator)->front() + offset, data.byteLength );
  return true;
}

bool indri::index::MemoryIndexTermListFileIterator::nextEntry( lemur::api::DOCID_T documentID ) {
  if( documentID >= (int)_data.size() ) {
    _finished = true;
    return false;
  }

  if( _index >= documentID ) {
    return true;
  }

  // set position to one document before the requested document
  _index = documentID - 1;
  return nextEntry();
}

indri::index::TermList* indri::index::MemoryIndexTermListFileIterator::currentEntry() {
  if( !_finished )
    return &_list;

  return 0;
}

bool indri::index::MemoryIndexTermListFileIterator::finished() {
  return _finished;
}
