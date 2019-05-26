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
// DocExtentListMemoryBuilderIterator
//
// 26 November 2004 -- tds
//

#include "indri/DocExtentListMemoryBuilder.hpp"

//
// startIteration
//

void indri::index::DocExtentListMemoryBuilderIterator::startIteration() {
  _current = _lists->begin();

  if( _current != _lists->end() ) {
    _list = _current->base;
    _listEnd = _current->data;
  } else {
    _list = 0;
    _listEnd = 0;
  }
  
  _data.document = 0;
  _data.extents.clear();
  _data.numbers.clear();

  nextEntry();
}

//
// reset
//

void indri::index::DocExtentListMemoryBuilderIterator::reset( DocExtentListMemoryBuilder& builder ) {
  builder.flush();
  reset( builder._lists, builder._numeric, builder._ordinal, builder._parental );
}

//
// reset
//

void indri::index::DocExtentListMemoryBuilderIterator::reset( const indri::utility::greedy_vector< DocExtentListMemoryBuilderSegment, 4 >& lists, bool numeric, bool ordinal, bool parental ) {
  _lists = &lists;
  _numeric = numeric;
  _ordinal = ordinal;
  _parental = parental;

  _current = _lists->begin();
  
  if( _current != _lists->end() ) {
    _list = _current->base;
    _listEnd = _current->data;
  } else {
    _list = 0;
    _listEnd = 0;
  }
  
  _data.document = 0;
  _data.numbers.clear();
  _data.extents.clear();
  _finished = false;

  nextEntry();
}

//
// DocExtentListMemoryBuilderIterator constructor
//

indri::index::DocExtentListMemoryBuilderIterator::DocExtentListMemoryBuilderIterator( const class DocExtentListMemoryBuilder& builder )
{
  reset( builder._lists, builder._numeric, builder._ordinal, builder._parental );
}

//
// nextEntry
//

bool indri::index::DocExtentListMemoryBuilderIterator::nextEntry( lemur::api::DOCID_T documentID ) {
  do {
    if( _data.document >= documentID )
      return true;
  }
  while( nextEntry() );
  
  return false;
}

//
// nextEntry
//
      
bool indri::index::DocExtentListMemoryBuilderIterator::nextEntry() {
  if( _list < _listEnd ) {
    int deltaDocument;
    int extents;
    
    _list = lemur::utility::RVLCompress::decompress_int( _list, deltaDocument );
    _data.document += deltaDocument;
    _data.extents.clear();
    _data.numbers.clear();

    _list = lemur::utility::RVLCompress::decompress_int( _list, extents );

    int deltaPosition;
    int ordinal = 0;
    int parent = 0;
    int deltaOrdinal;
    Extent extent;
    extent.begin = 0;
    extent.end = 0;
    extent.ordinal = 0;
    extent.parent = -1;
    extent.weight = 1;

    for( int i=0; i<extents; i++ ) {
      _list = lemur::utility::RVLCompress::decompress_int( _list, deltaPosition );
      // delta encode with respect to previous extent's begin
      extent.begin = extent.begin + deltaPosition;
      _list = lemur::utility::RVLCompress::decompress_int( _list, deltaPosition );
      // delta encode end with respect to begin
      extent.end = extent.begin + deltaPosition;

      if( _ordinal ) {
        _list = lemur::utility::RVLCompress::decompress_int( _list, deltaOrdinal );
        ordinal = ordinal + deltaOrdinal;
        extent.ordinal = ordinal;
      }

      if( _parental ) {
        _list = lemur::utility::RVLCompress::decompress_int( _list, parent );
        extent.parent = parent;
      }

      _data.extents.push_back( extent );

      if( _numeric ) {
        INT64 number;
        _list = lemur::utility::RVLCompress::decompress_longlong( _list, number );
        _data.numbers.push_back( number );
      }

      assert( _list <= _listEnd );
    }
  } else {    
    assert( _list == _listEnd );

    // no data left, go to the next segment
    if( _current != _lists->end() )
      _current++;
    
    if( _current != _lists->end() ) {
      _list = _current->base;
      _listEnd = _current->data;
      return nextEntry();
    }

    // no more list segments
    _finished = true;
    return false;
  }

  assert( _list <= _listEnd );
  return true;
}
      
//
// currentEntry
//

indri::index::DocExtentListIterator::DocumentExtentData* indri::index::DocExtentListMemoryBuilderIterator::currentEntry() {
  if( !_finished )
    return &_data;

  return 0;
}

//
// finished
//

bool indri::index::DocExtentListMemoryBuilderIterator::finished() const {
  return _finished;
}

