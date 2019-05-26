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
// DiskDocListIterator
//
// 10 December 2004 -- tds
//

#include "indri/DiskDocListIterator.hpp"
#include "lemur/RVLCompress.hpp"

//
// ---------------------
// Inverted list format:
// ---------------------
//      int (4b)    headerLength
//         RVLCompressed section (size is headerLength)
//            termString
//            termData (indri::index::TermData structure)
//      byte (1b)   controlByte   (0x01 = hasTopdocs, 0x02 = isFrequent)
//      topdocsCount (4b)  (if hasTopdocs)
//         for each topdoc:
//         docID (4b)
//         count (4b)
//         length (4b)
//      raw inverted list data:
//        skip: (if hasSkips)
//          (4b) document
//          (4b) length
//        for each in a batch of documents:
//          RVLCompressed:
//            delta document ID (delta encoded by batch)
//            position count
//            delta encoded positions
//
// ----------------------------
// More explanation about skips:
// ----------------------------
//
// A skip is a 8 byte structure that helps the query processor skip
// through the inverted list efficiently.  The first field, document,
// is the ID of some document, and the length parameter indicates the
// number bytes that need to be skipped to find that document.
// The number of bytes is measured from the end of the skip structure.
//
// At the beginning of the last chunk of documents, the skip will be {-1,-1}.
//

//
// DiskDocListIterator constructor
//

indri::index::DiskDocListIterator::DiskDocListIterator( indri::file::SequentialReadBuffer* buffer, UINT64 startOffset, int fieldCount )
  :
  _file(buffer),
  _startOffset(startOffset),
  _fieldCount(fieldCount),
  _termData(0),
  _ownTermData(false)
{
}

//
// DiskDocListIterator destructor
//

indri::index::DiskDocListIterator::~DiskDocListIterator() {
  delete _file;
  if( _ownTermData )
    free(_termData);
}

//
// setEndpoints
//

void indri::index::DiskDocListIterator::setStartOffset( UINT64 startOffset, TermData* termData ) {
  _startOffset = startOffset;
  _topdocs.clear();
  _file->seek( _startOffset );
  _termData = termData;
  _ownTermData = false;
}

//
// topDocuments
//

const indri::utility::greedy_vector<indri::index::DocListIterator::TopDocument>& indri::index::DiskDocListIterator::topDocuments() {
  return _topdocs;
}

//
// _readTermData
//

void indri::index::DiskDocListIterator::_readTermData( int headerLength ) {
  if( !_termData ) {
    indri::utility::Buffer header;

    _file->read( header.write( headerLength ), headerLength );
    indri::utility::RVLDecompressStream stream( header.front(), header.position() );

    // header is RVLCompressed with the term first, followed by a termData structure
    stream >> _term;
    _termData = (TermData*) malloc( ::termdata_size( _fieldCount ) );
    ::termdata_decompress( stream, _termData, _fieldCount );
    _termData->term = _term;
    _ownTermData = true;
  } else {
    // skip termData, we already have it
    _file->seek( _file->position() + headerLength );
  }
}

//
// startIteration
//

void indri::index::DiskDocListIterator::startIteration() {
  // seek to the start:
  _file->seek( _startOffset );

  // read the header length
  UINT32 headerLength;
  _file->read( &headerLength, sizeof(UINT32) );

  // read in termdata if necessary
  _readTermData( headerLength );

  // read the control byte
  UINT8 control;
  _file->read( &control, sizeof(UINT8) );

  _hasTopdocs = (control & 0x01) ? true : false;
  _isFrequent = (control & 0x02) ? true : false;
  
  // clear out all the internal data
  _data.document = 0;
  _data.positions.clear();
  _skipDocument = -1;
  _list = _listEnd = 0;

  // read in the term data, if necessary

  // read in the topdocs information
  _readTopdocs();

  // read in skip data
  _readSkip();
  
  // read the first entry, unless there's nothing here
  if( _list == _listEnd ) {
    _result = 0;
  } else {
  _readEntry();
  _result = &_data;
  }
}

//
// nextEntry
//

bool indri::index::DiskDocListIterator::nextEntry() {
  if( _list == _listEnd ) {
    if( _skipDocument > 0 ) {
      // need to read the next segment of this list
      _readSkip();
      _readEntry();
      return true;
    } else {
      // all done
      _result = 0;
      return false;
    }
  }

  _readEntry();
  return true;
}

//
// nextEntry
//

bool indri::index::DiskDocListIterator::nextEntry( lemur::api::DOCID_T documentID ) {
  // skip ahead as much as possible
  while( _skipDocument > 0 && _skipDocument <= documentID ) {
    _readSkip();
  }

  // now, read entries until we find one that's good
  while( _data.document < documentID ) {
    if( !nextEntry() ) {
      return false;
    }
  }

  return true;
}

//
// currentEntry
//

indri::index::DiskDocListIterator::DocumentData* indri::index::DiskDocListIterator::currentEntry() {
  return _result;
}

//
// finished
//

bool indri::index::DiskDocListIterator::finished() {
  return _result == 0;
}

//
// _readTopdocs
//

void indri::index::DiskDocListIterator::_readTopdocs() {
  if( !_hasTopdocs ) return;

  UINT32 topdocsCount;
  _topdocs.clear();
  _file->read( &topdocsCount, sizeof(UINT32) );

  for( UINT32 i=0; i<topdocsCount; i++ ) {
    lemur::api::DOCID_T documentID;
    UINT32 count, length;

    _file->read( &documentID, sizeof(lemur::api::DOCID_T) );
    _file->read( &count, sizeof(UINT32) );
    _file->read( &length, sizeof(UINT32) );

    assert( documentID > 0 );
    assert( count <= length );
    assert( length > 0 );

    _topdocs.push_back( TopDocument( documentID, count, length ) );
  }
}

//
// _readSkip
//

inline void indri::index::DiskDocListIterator::_readSkip() {
  int skipLength; 

  _file->read( &_skipDocument, sizeof(lemur::api::DOCID_T) );
  _file->read( &skipLength, sizeof(int) );

  assert( _skipDocument > -2 );
  assert( skipLength >= 0 );

  _list = static_cast<const char*>(_file->read( skipLength ));
  _listEnd = _list + skipLength;
  _data.document = 0;
}

//
// _readEntry
//

inline void indri::index::DiskDocListIterator::_readEntry() {
  _data.positions.clear();
  
  int deltaDocument;
  _list = lemur::utility::RVLCompress::decompress_int( _list, deltaDocument );
  _data.document += deltaDocument;

  int numPositions;
  _list = lemur::utility::RVLCompress::decompress_int( _list, numPositions );

  int lastPosition = 0;
  int deltaPosition;
  
  for( int i=0; i<numPositions; i++ ) {
    _list = lemur::utility::RVLCompress::decompress_int( _list, deltaPosition );
    _data.positions.push_back( deltaPosition + lastPosition );
    lastPosition += deltaPosition;
  }
}

//
// isFrequent
//

bool indri::index::DiskDocListIterator::isFrequent() const {
  return _isFrequent;
}

//
// termData
//

indri::index::TermData* indri::index::DiskDocListIterator::termData() {
  return _termData;
}


