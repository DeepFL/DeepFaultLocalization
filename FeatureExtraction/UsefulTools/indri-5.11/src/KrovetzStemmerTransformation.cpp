/*==========================================================================
 * Copyright (c) 2003-2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/


//
// KrovetzStemmerTransformation
//
// 13 May 2004 -- tds
//

#include "indri/KrovetzStemmerTransformation.hpp"

#define KSTEM_MAX_WORD_LENGTH (indri::parse::KrovetzStemmer::MAX_WORD_LENGTH+15)  // add extra space here in case kstem goes over its 25 byte window
#define KSTEM_EXTRA_SPACE     (256)


indri::parse::KrovetzStemmerTransformation::KrovetzStemmerTransformation( indri::api::Parameters& parameters ) {
  stemmer = new KrovetzStemmer();
  
  _stemBuffer = 0;
  _stemBufferSize = 0;

  indri::api::Parameters pheadwords;
  indri::api::Parameters pconflations;

  // figure out how many words we're dealing with here
  if( parameters.exists( "h" ) ) {
    pheadwords = parameters["h"];
  }

  if( parameters.exists( "conflation" ) ) {
    pconflations = parameters["conflation"];
  }
  unsigned int i;
    for( i=0; i<pheadwords.size(); i++ ) {
    std::string variant = std::string(pheadwords[i]);
    stemmer->kstem_add_table_entry( variant.c_str(), "" );
  }

  for( i=0; i<pconflations.size(); i++ ) {
    std::string variant = std::string(pconflations[i]["variant"]);
    std::string word = std::string(pconflations[i]["word"]);
    stemmer->kstem_add_table_entry( variant.c_str(), word.c_str() );
  }

}

indri::parse::KrovetzStemmerTransformation::~KrovetzStemmerTransformation() {
  delete[] _stemBuffer;
  delete(stemmer);
  //  kstem_release_memory(); // don't do this, multiple instances.
}

char* indri::parse::KrovetzStemmerTransformation::_growBuffer( size_t length, char* oldEnd ) {
  char* newBuffer = new char[length];
  memcpy( newBuffer, _stemBuffer, oldEnd - _stemBuffer );
  char* startPoint = (oldEnd - _stemBuffer) + newBuffer;

  _stemBufferSize = length;
  delete[] _stemBuffer;
  _stemBuffer = newBuffer;

  return startPoint;
}

char* indri::parse::KrovetzStemmerTransformation::_getBuffer( size_t length ) {
  if( _stemBufferSize < length ) {
    delete[] _stemBuffer;
    _stemBuffer = new char[length];
    _stemBufferSize = length;
  }

  return _stemBuffer;
}

const char* indri::parse::KrovetzStemmerTransformation::_getBufferEnd() const {
  return _stemBuffer + _stemBufferSize;
}

indri::api::ParsedDocument* indri::parse::KrovetzStemmerTransformation::_restart( indri::api::ParsedDocument* document, size_t lastIndex, char* endOfStemming ) {
  int stemmedRegion = endOfStemming - _stemBuffer;
  float proportion = (float(document->terms.size()) / float(lastIndex+1)) * 1.5;
  int expectedLength = int(proportion * stemmedRegion) + KSTEM_MAX_WORD_LENGTH;

  char* oldBuffer = _getBuffer(0);
  char* newStart = _growBuffer( expectedLength, endOfStemming );
  char* newBuffer = _getBuffer(0);
  const char* newEnd = _getBufferEnd();

  int fixup = newBuffer - oldBuffer;

  for( size_t i=0; i<=lastIndex; i++ ) {
    if( document->terms[i] >= oldBuffer &&
        document->terms[i] <= endOfStemming )
    {
      // this pointer points into the old buffer, so we have to fix it up
      document->terms[i] += fixup;
    }
  }

  return _processTerms( document, lastIndex+1, newStart, newEnd );
}

indri::api::ParsedDocument* indri::parse::KrovetzStemmerTransformation::_processTerms( indri::api::ParsedDocument* document, size_t start, char* stem, const char* end ) {
  indri::utility::greedy_vector<char*>& terms = document->terms;

  for( size_t i=start; i<terms.size(); i++ ) {
    char* term = terms[i];

    if( !term )
      continue;

    int increment = stemmer->kstem_stem_tobuffer( term, stem );

    if( increment ) {
      terms[i] = stem;
      stem += increment;
    }

    if( stem >= end ) {
      // we're dangerously close to the end of the buffer
      // so restart the stemming
      return _restart( document, i, stem );
    }
  }

  return document;
}

indri::api::ParsedDocument* indri::parse::KrovetzStemmerTransformation::transform( indri::api::ParsedDocument* document ) {
  indri::utility::greedy_vector<char*>& terms = document->terms;

  // this is the minimum amount of space we're willing to start with, but we may get more
  //  int bufferLength = document->textLength * 2 + KSTEM_MAX_WORD_LENGTH + KSTEM_EXTRA_SPACE;
  int bufferLength = document->terms.size() * KSTEM_MAX_WORD_LENGTH + KSTEM_EXTRA_SPACE;
  char* stem = _getBuffer( bufferLength );
  const char* end = _getBufferEnd() - KSTEM_MAX_WORD_LENGTH;

  return _processTerms( document, 0, stem, end );
}

void indri::parse::KrovetzStemmerTransformation::setHandler( ObjectHandler<indri::api::ParsedDocument>& handler ) {
  _handler = &handler;
}

void indri::parse::KrovetzStemmerTransformation::handle( indri::api::ParsedDocument* document ) {
  _handler->handle( transform( document ) );
}
