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
// DiskFrequentVocabularyIterator
//
// 19 January 2005 -- tds
//

#include "indri/DiskFrequentVocabularyIterator.hpp"
#include "indri/DiskTermData.hpp"

//
// DiskFrequentVocabularyIterator constructor
//

indri::index::DiskFrequentVocabularyIterator::DiskFrequentVocabularyIterator( indri::file::File& frequentTermsData, int fieldCount ) :
  _file(frequentTermsData),
  _fieldCount(fieldCount),
  _stream(0, 0)
{
  int dataSize = ::disktermdata_size(_fieldCount);
  _dataBuffer = new char [dataSize];
}

//
// startIteration
//

void indri::index::DiskFrequentVocabularyIterator::startIteration() {
  if( _buffer.size() == 0 ) {
    UINT64 length = _file.size();
    _file.read( _buffer.write( length ), 0, length );
  }

  _finished = false;
  _stream.setBuffer( _buffer.front(), _buffer.position() );
  nextEntry();
  _justStartedIteration=true;
}

//
// nextEntry
//

bool indri::index::DiskFrequentVocabularyIterator::nextEntry() {
  if( !_stream.done() ) {
    _data = ::disktermdata_decompress( _stream, _dataBuffer, _fieldCount, indri::index::DiskTermData::WithOffsets |
                                       indri::index::DiskTermData::WithTermID | 
                                       indri::index::DiskTermData::WithString );
    _justStartedIteration=false;

    return true;
  } else {
    _finished = true;
    return false;
  }
}

//
// nextEntry (const char *)
//

bool indri::index::DiskFrequentVocabularyIterator::nextEntry(const char *skipTo) {
  // we have to scan through each item here....

  assert(skipTo!=NULL);

  int entryTermLen=strlen(skipTo);
  if (!entryTermLen) {
    startIteration();
    return true;
  }

  // start from the current iterator
  while( !_stream.done() ) {

    if (!_justStartedIteration) {
      _data = ::disktermdata_decompress( _stream, _dataBuffer, _fieldCount, indri::index::DiskTermData::WithOffsets |
                                         indri::index::DiskTermData::WithTermID | 
                                         indri::index::DiskTermData::WithString );
    }

    _justStartedIteration=false;

    if (strstr(_data->termData->term, skipTo)==_data->termData->term) {
      return true;
    }
  } // end while( !_stream.done() )

  _finished = true;
  return false;
}


//
// finished
//

bool indri::index::DiskFrequentVocabularyIterator::finished() {
  return _finished;
}

//
// currentEntry
//

indri::index::DiskTermData* indri::index::DiskFrequentVocabularyIterator::currentEntry() {
  if( !_finished )
    return _data;

  return 0;
}
