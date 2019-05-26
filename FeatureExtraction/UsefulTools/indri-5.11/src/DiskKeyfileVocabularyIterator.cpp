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
// DiskKeyfileVocabularyIterator
//
// 20 January 2005 -- tds
//

#include "indri/DiskKeyfileVocabularyIterator.hpp"
#include "indri/Mutex.hpp"

//
// DiskKeyfileVocabularyIterator constructor
//

indri::index::DiskKeyfileVocabularyIterator::DiskKeyfileVocabularyIterator( int baseID, indri::file::BulkTreeReader& bulkTree, indri::thread::Mutex& lock, int fieldCount ) :
  _baseID(baseID),
  _bulkTree(bulkTree),
  _mutex(lock),
  _holdingLock(false),
  _fieldCount(fieldCount)
{
  _compressedData.write( disktermdata_size( _fieldCount ) * 2 );
  _decompressedData.write( disktermdata_size( _fieldCount ) );
  _bulkIterator = _bulkTree.iterator();
}

//
// DiskKeyfileVocabularyIterator destrcutor
//

indri::index::DiskKeyfileVocabularyIterator::~DiskKeyfileVocabularyIterator() {
  _release();
  delete _bulkIterator;
}

//
// _acquire
//

void indri::index::DiskKeyfileVocabularyIterator::_acquire() {
  if( !_holdingLock ) {
    _mutex.lock();
    _holdingLock = true;
  }
}

//
// _release
//

void indri::index::DiskKeyfileVocabularyIterator::_release() {
  if( _holdingLock ) {
    _mutex.unlock();
    _holdingLock = false;
  }
}

//
// startIteration
//

void indri::index::DiskKeyfileVocabularyIterator::startIteration() {
  _acquire();

  _bulkIterator->startIteration();
  _readData();
  _justStartedIteration=true;
}

//
// _readData
//

bool indri::index::DiskKeyfileVocabularyIterator::_readData() {

  if( _bulkIterator->finished() ) {
    _release();
    return false;
  }

  int actual;
  int actualKeyLen;

  memset(_termString, 0, 1024);
  _bulkIterator->get( _termString, 1024, actualKeyLen, _compressedData.front(), _compressedData.size(), actual );
  indri::utility::RVLDecompressStream stream( _compressedData.front(), actual );

  _diskTermData = ::disktermdata_decompress( stream,
                                             _decompressedData.front(),
                                             _fieldCount,
                                             DiskTermData::WithOffsets |
                                             DiskTermData::WithTermID );

  _diskTermData->termData->term = _termString;
  return true;
}

//
// nextEntry
//

bool indri::index::DiskKeyfileVocabularyIterator::nextEntry() {
  _bulkIterator->nextEntry();
  _justStartedIteration=false;

  return _readData();
}

//
// nextEntry (const char *)
//

bool indri::index::DiskKeyfileVocabularyIterator::nextEntry(const char *skipTo) {

  assert(skipTo!=NULL);

  if (_justStartedIteration) {
    // position the iterator at the first
    // item that matches...
    _justStartedIteration=false;

    // get an iterator to the first term here
    indri::file::BulkTreeIterator *findIterator=_bulkTree.findFirst(skipTo);
    if (!findIterator) return false;

    // replace the current iterator with the new one
    if (_bulkIterator) delete _bulkIterator;
    _bulkIterator=findIterator;
    if (!_readData()) return false;

  } else {
    // get the next item...
    // assume all items w/ the same prefix are clumped together
    _bulkIterator->nextEntry();
    if (!_readData()) return false;
  }

  // make sure we're not at the end
  if (_bulkIterator->finished()) {
    return false;
  }

  // get the current entry
  indri::index::DiskTermData* thisEntry=currentEntry();
  if (!thisEntry) return false;

  // make sure we're still in the patten
  if (strstr(thisEntry->termData->term, skipTo)==thisEntry->termData->term) {
    return true;
  }

  // just to be certain - check the next entry...
  // read the next item...
  _bulkIterator->nextEntry();
  if (!_readData()) return false;
        
  // ensure we're not finished
  if (_bulkIterator->finished()) return false;

  // get the next entry...
  thisEntry=currentEntry();
  if (!thisEntry) return false;

  // check it.
  if (strstr(thisEntry->termData->term, skipTo)==thisEntry->termData->term) {
    return true;
  }

  // ok - I'm satisfied that we're done...
  return false;
}

//
// currentEntry
//

indri::index::DiskTermData* indri::index::DiskKeyfileVocabularyIterator::currentEntry() {
  if( !_bulkIterator->finished() )
    return _diskTermData;

  return 0;
}

//
// finished
//

bool indri::index::DiskKeyfileVocabularyIterator::finished() {
  return _bulkIterator->finished();
}
