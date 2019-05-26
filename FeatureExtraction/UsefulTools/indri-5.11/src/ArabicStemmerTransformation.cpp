/*==========================================================================
 * Copyright (c) 2012 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/


//
// ArabicStemmerTransformation
//
// Adapted from the Lemur ArabicStemmer class
//

#include "indri/ArabicStemmerTransformation.hpp"

#define STEM_MAX_WORD_LENGTH (115)
#define STEM_EXTRA_SPACE     (256)

indri::parse::ArabicStemmerTransformation::ArabicStemmerTransformation( indri::api::Parameters& parameters ) {
  std::string stemFunc = parameters.get("name", "none");
  stemmer = new Arabic_Stemmer_utf8(stemFunc);
  _stemBuffer = 0;
  _stemBufferSize = 0;
}

indri::parse::ArabicStemmerTransformation::~ArabicStemmerTransformation() {
  delete[] _stemBuffer;
  delete(stemmer);
}

char* indri::parse::ArabicStemmerTransformation::_growBuffer( size_t length, char* oldEnd ) {
  char* newBuffer = new char[length];
  memcpy( newBuffer, _stemBuffer, oldEnd - _stemBuffer );
  char* startPoint = (oldEnd - _stemBuffer) + newBuffer;

  _stemBufferSize = length;
  delete[] _stemBuffer;
  _stemBuffer = newBuffer;
  return startPoint;
}

char* indri::parse::ArabicStemmerTransformation::_getBuffer( size_t length ) {
  if( _stemBufferSize < length ) {
    delete[] _stemBuffer;
    _stemBuffer = new char[length];
    _stemBufferSize = length;
  }
  return _stemBuffer;
}

const char* indri::parse::ArabicStemmerTransformation::_getBufferEnd() const {
  return _stemBuffer + _stemBufferSize;
}

indri::api::ParsedDocument* indri::parse::ArabicStemmerTransformation::_restart( indri::api::ParsedDocument* document, size_t lastIndex, char* endOfStemming ) {
  int stemmedRegion = endOfStemming - _stemBuffer;
  float proportion = (float(document->terms.size()) / float(lastIndex+1)) * 1.5;
  int expectedLength = int(proportion * stemmedRegion) + STEM_MAX_WORD_LENGTH;

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

indri::api::ParsedDocument* indri::parse::ArabicStemmerTransformation::_processTerms( indri::api::ParsedDocument* document, size_t start, char* stem, const char* end ) {
  indri::utility::greedy_vector<char*>& terms = document->terms;

  for( size_t i=start; i<terms.size(); i++ ) {
    char* term = terms[i];

    if( !term )
      continue;
    stemmer->stemTerm( term, stem );
    int increment = strlen(stem) + 1;
    

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

indri::api::ParsedDocument* indri::parse::ArabicStemmerTransformation::transform( indri::api::ParsedDocument* document ) {
  indri::utility::greedy_vector<char*>& terms = document->terms;

  // this is the minimum amount of space we're willing to start with, but we may get more
  //  int bufferLength = document->textLength * 2 + STEM_MAX_WORD_LENGTH + STEM_EXTRA_SPACE;
  int bufferLength = document->terms.size() * STEM_MAX_WORD_LENGTH + STEM_EXTRA_SPACE;
  char* stem = _getBuffer( bufferLength );
  const char* end = _getBufferEnd() - STEM_MAX_WORD_LENGTH;

  return _processTerms( document, 0, stem, end );
}

void indri::parse::ArabicStemmerTransformation::setHandler( ObjectHandler<indri::api::ParsedDocument>& handler ) {
  _handler = &handler;
}

void indri::parse::ArabicStemmerTransformation::handle( indri::api::ParsedDocument* document ) {
  _handler->handle( transform( document ) );
}
