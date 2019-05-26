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
// DiskTermData
//
// 13 December 2004 -- tds
//

#ifndef INDRI_DISKTERMDATA_HPP
#define INDRI_DISKTERMDATA_HPP

#include "indri/TermData.hpp"
#include "lemur/Keyfile.hpp"
#include "lemur/IndexTypes.hpp"

namespace indri {
  namespace index {
    struct DiskTermData {
      enum {
        WithOffsets = 0x01,
        WithString = 0x02,
        WithTermID = 0x04
      };

      TermData* termData;
      lemur::api::TERMID_T termID;

      UINT64 startOffset;
      UINT64 length;
    };
  }
}


//
// disktermdata_decompress
//

inline void disktermdata_compress( indri::utility::RVLCompressStream& stream, indri::index::DiskTermData* diskData, int fieldCount, int mode ) {
  ::termdata_compress( stream, diskData->termData, fieldCount );

  if( mode & indri::index::DiskTermData::WithTermID ) {
    stream << diskData->termID;
  }

  if( mode & indri::index::DiskTermData::WithString ) {
    stream << diskData->termData->term;
  }

  if( mode & indri::index::DiskTermData::WithOffsets ) {
    stream << diskData->startOffset
           << diskData->length;
  }
}

//
// disktermdata_decompress
//

inline indri::index::DiskTermData* disktermdata_decompress( indri::utility::RVLDecompressStream& stream, void* buffer, int fieldCount, int mode ) {
  indri::index::DiskTermData* diskData = (indri::index::DiskTermData*) buffer;

  int termDataSize = ::termdata_size( fieldCount );
  char* termLocation = (char*)buffer + sizeof(indri::index::DiskTermData) + termDataSize;
  indri::index::TermData* termDataLocation = (indri::index::TermData*) ((char*)buffer + sizeof(indri::index::DiskTermData));

  diskData->termData = termDataLocation;
  diskData->termData->term = termLocation;

  // set first byte of string to zero
  termLocation[0] = 0;

  ::termdata_decompress( stream, diskData->termData, fieldCount );

  if( mode & indri::index::DiskTermData::WithTermID ) {
    stream >> diskData->termID;
  } else {
    diskData->termID = 0;
  }

  if( mode & indri::index::DiskTermData::WithString ) {
    stream >> termLocation;
  }

  if( mode & indri::index::DiskTermData::WithOffsets ) {
    stream >> diskData->startOffset
           >> diskData->length;
  } else {
    diskData->startOffset = 0;
    diskData->length = 0;
  }

  return diskData;
}

//
// disktermdata_size
//

inline int disktermdata_size( int fieldCount ) {
  // how much space are we going to need?
  int termDataSize = ::termdata_size( fieldCount );
  int totalSize = termDataSize + (lemur::file::Keyfile::MAX_KEY_LENGTH+2) + sizeof(indri::index::DiskTermData);

  return totalSize;
}

//
// disktermdata_create
//

inline indri::index::DiskTermData* disktermdata_create( int fieldCount ) {
  char* dataBlock = (char*) malloc( disktermdata_size( fieldCount ) );

  indri::index::DiskTermData* diskTermData = (indri::index::DiskTermData*) dataBlock;

  diskTermData->termData = (indri::index::TermData*) (dataBlock +
                                                      sizeof (indri::index::DiskTermData));
  termdata_construct( diskTermData->termData, fieldCount );

  diskTermData->termData->term = dataBlock +
    sizeof (indri::index::DiskTermData) +
    termdata_size( fieldCount );
  const_cast<char*>(diskTermData->termData->term)[0] = 0;

  return diskTermData;
}

//
// disktermdata_decompress
//

inline indri::index::DiskTermData* disktermdata_decompress( indri::utility::RVLDecompressStream& stream, int fieldCount, int mode ) {
  // how much space are we going to need?
  int totalSize = disktermdata_size( fieldCount );

  return ::disktermdata_decompress( stream, malloc( totalSize ), fieldCount, mode );
}

//
// disktermdata_delete
//

inline void disktermdata_delete( indri::index::DiskTermData* diskData ) {
  free( diskData );
}

#endif // INDRI_DISKTERMDATA_HPP


