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
// TextDocumentExtractor
//
// 16 August 2004 -- tds
//

#include "indri/TextDocumentExtractor.hpp"
#include "lemur/Exception.hpp"

void indri::parse::TextDocumentExtractor::open( const std::string& filename ) {
  _in = gzopen( filename.c_str(), "rb" );
  _filename = filename;

  if( !_in )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open file " + filename + "." );
}

indri::parse::UnparsedDocument* indri::parse::TextDocumentExtractor::nextDocument() {
  _buffer.clear();
  _document.text = 0;
  _document.textLength = 0;
  _document.metadata.clear();

  if( gzeof( _in ) )
    return 0;

  // set up metadata
  indri::parse::MetadataPair pair;
  pair.value = _filename.c_str();
  pair.valueLength = _filename.length()+1;
  pair.key = "path";
  _document.metadata.push_back( pair );

  _docnostring.assign(_filename.c_str() );
  cleanDocno();
  pair.value = _docnostring.c_str();
  pair.valueLength = _docnostring.length()+1;
  pair.key = "docno";
  _document.metadata.push_back( pair );

  pair.key = "filetype";
  pair.value = (void*) "TEXT";
  pair.valueLength = 5;
  _document.metadata.push_back( pair );

  // get document text
  while( !gzeof(_in) ) {
    int readChunk = 1024*1024;
    char* textSpot = _buffer.write(readChunk);
    int read = gzread( _in, textSpot, readChunk );
    _buffer.unwrite( readChunk - read );
  }
  *_buffer.write(1) = 0;

  _document.text = _buffer.front();
  _document.textLength = _buffer.position();
  _document.content = _buffer.front();
  _document.contentLength = _buffer.position() - 1; // no null

  return &_document;
}

void indri::parse::TextDocumentExtractor::close() {
  gzclose(_in);
}
