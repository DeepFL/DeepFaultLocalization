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
// PriorListIterator.cpp
//
// 22 July 2005 -- tds
//

#include "indri/PriorListIterator.hpp"
#include <iostream>

//
// PriorListIterator constructor
//

indri::collection::PriorListIterator::PriorListIterator( indri::file::SequentialReadBuffer* file )
  :
  _file( file )
{
}

//
// PriorListIterator destructor
//

indri::collection::PriorListIterator::~PriorListIterator() {
  delete _file;
}

//
// startIteration
//

void indri::collection::PriorListIterator::startIteration() {
  _file->seek( 0 );
  _lookup.clear();

  _entryCount = 0;
  _tableLength = 0;

  _file->read( &_entryCount, sizeof(UINT32) );
  _file->read( &_tableLength, sizeof(UINT32) );

  _finished = ( _entryCount == 0 );

  // if there is a lookup table, load it
  if( _tableLength ) {
    _entryLength = sizeof(UINT8);

    for( UINT32 i=0; i<_tableLength; i++ ) {
      double value;
      _file->read( &value, sizeof(double) );
      _lookup.push_back( value );
    }
  } else {
    _entryLength = sizeof(double);
  }

  _entry.document = 0;
  nextEntry();
}

//
// nextEntry
//

void indri::collection::PriorListIterator::nextEntry() {
  if( _finished )
    return;

  if( _entry.document >= (int)_entryCount ) {
    _finished = true;
    return;
  }

  if( _tableLength ) {
    UINT8 index;
    _file->read( &index, sizeof(UINT8) );

    _entry.document++;
    _entry.score = _lookup[index];
  } else {
    double value;

    _file->read( &value, sizeof(double) );
    _entry.document++;
    _entry.score = value;
  }
}

//
// nextEntry
//

void indri::collection::PriorListIterator::nextEntry( lemur::api::DOCID_T document ) {
  if( _finished || _entry.document >= (lemur::api::DOCID_T)_entryCount ) {
    _finished = true;
    return;
  }

  _entry.document = document-1;
  _file->seek( 2*sizeof(UINT32) + _tableLength * sizeof(double) + _entryLength * (document-1) );

  nextEntry();
}

//
// currentEntry
//

indri::collection::PriorListIterator::Entry* indri::collection::PriorListIterator::currentEntry() {
  if( !_finished )
    return &_entry;

  return 0;
}

//
// finished
//

bool indri::collection::PriorListIterator::finished() {
  return _finished;
}

