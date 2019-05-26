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
// MboxDocumentIterator
//
// 20 May 2005 -- tds
//

#include "indri/MboxDocumentIterator.hpp"
#include "lemur/Exception.hpp"

#define MBOX_MAX_LINE_LENGTH (1024*1024)
#define MBOX_MAX_HEADER_LINE_LENGTH (32*1024)

#define MBOX_FROM       ("From:")
#define MBOX_TO         ("To:")
#define MBOX_SUBJECT    ("Subject:")
#define MBOX_DATE       ("Date:")
#define MBOX_CC         ("Cc:")
#define MBOX_EMPTY_LINE ("")

struct field_t {
  const char* field;
  const char* tag;
  const int length;
};

//
// open
//
// Open a file in mbox (Unix mailbox) format
//

void indri::parse::MboxDocumentIterator::open( const std::string& filename ) {
  _in.clear();
  _in.open( filename.c_str() );
  _filename = filename;

  if( !_in.good() )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open file " + filename + "." );
}

//
// _copyMetadata
//
// Copy this line into the document buffer, but also into metadata
//

void indri::parse::MboxDocumentIterator::_copyMetadata( const char* headerLine, int ignoreBytes, const char* tagName ) {
  // copy the data into the document buffer
  int lineLength = strlen( headerLine );
  char* spot = _buffer.write( lineLength - ignoreBytes ); 
  strcpy( spot, headerLine + ignoreBytes + 1 );
  
  // replace the trailing '\0' with a space
  spot[ lineLength - ignoreBytes - 1 ] = '\n';

  spot = _metaBuffer.write( lineLength - ignoreBytes );
  strcpy( spot, headerLine + ignoreBytes + 1 );

  indri::parse::MetadataPair pair;
  // just store an offset for now, get better data later
  pair.value = (void*) (spot - _metaBuffer.front());
  pair.valueLength = lineLength - ignoreBytes;
  pair.key = tagName;

  _document.metadata.push_back( pair );
}

//
// nextDocument
//
// Fetch the next e-mail message from the mbox file as an UnparsedDocument.
// The UnparsedDocument is owned by the MboxDocumentExtractor, and should
// be assumed to be valid only until the next call to any MboxDocumentExtractor 
// method.
//

indri::parse::UnparsedDocument* indri::parse::MboxDocumentIterator::nextDocument() {
  _buffer.clear();
  _document.text = 0;
  _document.textLength = 0;
  _document.content = 0;
  _document.contentLength = 0;
  _document.metadata.clear();

  if( _in.eof() )
    return 0;

  char headerLine[ MBOX_MAX_HEADER_LINE_LENGTH ];

  // skim past all unnecessary headers
  // want to catch:
  //    recipient
  //    author
  //    subject    
  //    date
  // all of these need to be metadata and indexed content

  static const field_t fields[] = {
    { "From:", "author", 5 },
    { "To:", "recipient", 3 },
    { "Subject:", "subject", 8 },
    { "Cc:", "copied", 3 },
    { "Date:", "date", 5 }
  };

  int field = -1;

  if( !_in.eof() ) {
    _in.getline( headerLine, MBOX_MAX_HEADER_LINE_LENGTH );
  }

  while( !_in.eof() ) {
    // if the line is empty, we're done with this header
    if( !strcmp( headerLine, MBOX_EMPTY_LINE ) )
      break;

    // record the number of bytes read
    int lineLength = _in.gcount();
    int extraLength = 0;

    // is this an interesting line?
    for( int i=0; i<(sizeof fields/sizeof fields[0]); i++ ) {
      if( !strncmp( fields[i].field, headerLine, fields[i].length ) ) {
        field = i;
        break;
      }
    }

    // if this is an interesting line, do some special processing
    if( field >= 0 ) {
      // some fields are multi-line; these fields start with a tab character.
      // therefore, we'll try to fetch more lines now
      while( !_in.eof() ) {
        _in.getline( headerLine + lineLength, MBOX_MAX_HEADER_LINE_LENGTH - lineLength );
        extraLength = _in.gcount();
        
        if( headerLine[lineLength] != '\t' ) {
          break;
        } else {
          // add a newline where the '\0' was
          headerLine[lineLength-1] = '\n';
          lineLength += extraLength;
          extraLength = 0;
        }
      }

      // now, copy to metadata
      _copyMetadata( headerLine, fields[field].length, fields[field].tag );

      // move next line data to beginning of buffer
      memmove( headerLine, headerLine + lineLength, extraLength );

      // clear field
      field = -1;
    } else {
      _in.getline( headerLine, MBOX_MAX_HEADER_LINE_LENGTH );
    }
  }

  // now, we're catching message text
  // we will stop (and throw out content) as soon as we see a "From" line
  while( !_in.eof() ) {
    int readChunk = 1024*1024;
    char* textSpot = _buffer.write(readChunk);
    _in.getline( textSpot, readChunk );

    // add in the newline that was replaced by a '\0'
    int actual = _in.gcount();
    _buffer.unwrite( readChunk - actual );
    textSpot[actual-1] = '\n';

    // done reading at a "From" line
    if( !strncmp( textSpot, "From", 4 ) ) {
      _buffer.unwrite( _in.gcount() );
      break;
    }
  }

  // terminate string
  *_buffer.write(1) = 0;

  // fix up existing metadata
  for( size_t i=0; i<_document.metadata.size(); i++ ) {
    size_t offset = (size_t) _document.metadata[i].value;
    _document.metadata[i].value = _metaBuffer.front() + offset;
  }

  // add type metadata
  indri::parse::MetadataPair pair;
  pair.key = "filetype";
  pair.value = (void*) "MBOX";
  pair.valueLength = sizeof "MBOX";
  _document.metadata.push_back( pair );

  // copy subject into docno
  for( size_t i=0; i<_document.metadata.size(); i++ ) {
    if( !strcmp( "subject", _document.metadata[i].key ) ) {
      _docnostring.assign((char *)_document.metadata[i].value );
      cleanDocno();
      pair.value = _docnostring.c_str();
      pair.valueLength = _docnostring.length()+1;
      pair.key = "docno";
      _document.metadata.push_back( pair );
      break;
    }
  }

  _document.text = _buffer.front();
  _document.textLength = _buffer.position();
  _document.content = _buffer.front();
  _document.contentLength = _buffer.position();

  return &_document;
}

//
// close
//
// Close the mbox file
//

void indri::parse::MboxDocumentIterator::close() {
  _in.close();
}
