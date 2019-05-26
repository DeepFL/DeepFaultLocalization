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
// CombinedVocabularyIterator
//
// 20 January 2005 -- tds
//

#include "indri/CombinedVocabularyIterator.hpp"

//
// CombinedVocabularyIterator constructor
//

indri::index::CombinedVocabularyIterator::CombinedVocabularyIterator( VocabularyIterator* first,
                                                                      VocabularyIterator* second,
                                                                      int secondBase ) :
  _first(first),
  _second(second),
  _secondBase(secondBase)
{
}

//
// CombinedVocabularyIterator destructor
//

indri::index::CombinedVocabularyIterator::~CombinedVocabularyIterator() {
  delete _first;
  delete _second;
}

//
// startIteration
//

void indri::index::CombinedVocabularyIterator::startIteration() {
  _usingSecond = false;
  _finished = false;

  _first->startIteration();

  if( _first->finished() ) {
    _usingSecond = true;
    _second->startIteration();
  
    if( _second->finished() ) {
      _finished = true;
    }
  }
}

//
// nextEntry
//

bool indri::index::CombinedVocabularyIterator::nextEntry() {
  bool result;

  if( !_usingSecond ) {
    result = _first->nextEntry();

    if( !result ) {
      _second->startIteration();
      _usingSecond = true;
    }

    result = true;
  } else {
    result = _second->nextEntry();
  }

  if( _usingSecond ) {
    DiskTermData* data = _second->currentEntry();
    if( data )
      data->termID += _secondBase;
  }

  return result;
}

//
// nextEntry (const char *)
//

bool indri::index::CombinedVocabularyIterator::nextEntry(const char *skipTo) {
  assert(skipTo!=NULL);

  bool result;

  if( !_usingSecond ) {
    result = _first->nextEntry(skipTo);

    if( !result ) {
      _second->startIteration();
      _usingSecond = true;
    }

    result = true;
  }

  if( _usingSecond ) {
    result=_second->nextEntry(skipTo);
    if (result) {
      DiskTermData* data = _second->currentEntry();
      if( data )
        data->termID += _secondBase;
    }
  }

  return result;
}

//
// currentEntry
//

indri::index::DiskTermData* indri::index::CombinedVocabularyIterator::currentEntry() {
  if( _finished )
    return 0;

  if( _usingSecond )
    return _second->currentEntry();

  return _first->currentEntry();
}

//
// finished
//

bool indri::index::CombinedVocabularyIterator::finished() {
  return _usingSecond && _second->finished();
}


