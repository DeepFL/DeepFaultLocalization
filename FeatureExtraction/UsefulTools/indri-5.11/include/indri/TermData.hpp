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
// TermData.hpp
//
// 4 February 2004 -- tds
//

#ifndef INDRI_TERMDATA_HPP
#define INDRI_TERMDATA_HPP

#include "indri/TermFieldStatistics.hpp"
#include <indri/greedy_vector>
#include "lemur/lemur-compat.hpp"
#include "indri/RVLCompressStream.hpp"
#include "indri/RVLDecompressStream.hpp"

#ifdef WIN32
// remove warning about zero-sized arrays
#pragma warning ( disable: 4200 )
#endif 

namespace indri {
  namespace index {
    struct TermData {
    private:
      // these are private, bogus functions so that this object can never be copied
      // we don't want to be able to copy it, because any real copy operator needs to
      // take into account the data in the fields[] array, and we don't know how long it is.
      TermData( const TermData& other ) {}
      const TermData& operator= ( const TermData& other ) { return *this; }

    public:
      TermData() :
        maxDocumentLength(0),
        minDocumentLength(MAX_INT32)
      {
        term = 0;
      }
      
      struct term_less {
      public:
        bool operator () ( const TermData* one, const TermData* two ) const {
          return strcmp( one->term, two->term ) < 0;
        }
      };

      TermFieldStatistics corpus;

      unsigned int maxDocumentLength;    // maximum length of any document containing this term
      unsigned int minDocumentLength;    // minimum length of any document containing this term

      const char* term;                  // name of this term

      TermFieldStatistics fields[0];
    };
  }
}

inline indri::index::TermData* termdata_construct( void* buffer, int fieldCount ) {
  // call the constructor in place
  new(buffer) indri::index::TermData();

  // call field data constructors in place
  for( int i=0; i<fieldCount; i++ ) {
    new((char*)buffer +
        sizeof(indri::index::TermData) +
        sizeof(indri::index::TermFieldStatistics)*i) indri::index::TermFieldStatistics();
  }

  return (indri::index::TermData*) buffer;
}

inline indri::index::TermData* termdata_create( int fieldCount ) {
  // allocate enough room for the term data, plus enough room for fields
  void* buffer = malloc( sizeof(indri::index::TermData) + sizeof(indri::index::TermFieldStatistics)*fieldCount );
  return termdata_construct( buffer, fieldCount );
}

inline void termdata_destruct( indri::index::TermData* termData, int fieldCount ) {
  if( termData ) {
    termData->~TermData();

    for( int i=0; i<fieldCount; i++ ) {
      termData->fields[i].~TermFieldStatistics();
    }
  }
}

inline void termdata_delete( indri::index::TermData* termData, int fieldCount ) {
  if( termData ) {
    termdata_destruct( termData, fieldCount );
    free(termData);
  }
}

inline void termdata_clear( indri::index::TermData* termData, int fieldCount ) {
  termData->corpus.documentCount = 0;
  termData->corpus.totalCount = 0;
  termData->corpus.lastCount = 0;
  termData->corpus.lastDocument = 0;

  for( int i=0; i<fieldCount; i++ ) {
    indri::index::TermFieldStatistics& field = termData->fields[i];

    field.documentCount = 0;
    field.totalCount = 0;
    field.lastCount = 0;
    field.lastDocument = 0;
  }

  termData->minDocumentLength = MAX_INT32;
  termData->maxDocumentLength = 0;
}

inline void termdata_merge( indri::index::TermData* termData, indri::index::TermData* merger, int fieldCount ) {
  termData->corpus.documentCount += merger->corpus.documentCount;
  termData->corpus.totalCount += merger->corpus.totalCount;

  for( int i=0; i<fieldCount; i++ ) {
    indri::index::TermFieldStatistics& field = termData->fields[i];
    indri::index::TermFieldStatistics& mergeField = merger->fields[i];

    field.documentCount += mergeField.documentCount;
    field.totalCount += mergeField.totalCount;
  }

  termData->maxDocumentLength = lemur_compat::max( termData->maxDocumentLength, merger->maxDocumentLength );
  termData->minDocumentLength = lemur_compat::min( termData->minDocumentLength, merger->minDocumentLength );
}

inline int termdata_size( int fieldCount ) {
  return sizeof(indri::index::TermData) + fieldCount * sizeof(indri::index::TermFieldStatistics);
}

inline void termdata_compress( indri::utility::RVLCompressStream& stream, indri::index::TermData* termData, int fieldCount ) {
  // corpus statistics
  stream << termData->corpus.totalCount
         << termData->corpus.documentCount;

  // max-score statistics
  stream << termData->maxDocumentLength
         << termData->minDocumentLength;
  // field statistics
  for( int i=0; i<fieldCount; i++ ) {
    stream << termData->fields[i].totalCount
           << termData->fields[i].documentCount;
  }
}

inline void termdata_decompress( indri::utility::RVLDecompressStream& stream, indri::index::TermData* termData, int fieldCount ) {
  // corpus statistics
  stream >> termData->corpus.totalCount
         >> termData->corpus.documentCount;

  // max-score statistics
  stream >> termData->maxDocumentLength
         >> termData->minDocumentLength;

  // field statistics
  for( int i=0; i<fieldCount; i++ ) {
    stream >> termData->fields[i].totalCount
           >> termData->fields[i].documentCount;
  }
}

#endif // INDRI_TERMDATA_HPP
