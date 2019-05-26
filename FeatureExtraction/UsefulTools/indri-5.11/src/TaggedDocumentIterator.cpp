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
// TaggedDocumentIterator
//
// 14 May 2004 -- tds
//

#include "indri/TaggedDocumentIterator.hpp"
#include <iostream>
#include "lemur/Exception.hpp"
#include "indri/XMLNode.hpp"

struct metadata_range {
  int beginText;
  int endText;
  int beginTag;
  int endTag;
};

indri::parse::TaggedDocumentIterator::TaggedDocumentIterator() {
  _in = 0;

  _startDocTag = 0;
  _endDocTag = 0;
  _endMetadataTag = 0;

  _startDocTagLength = 0;
  _endDocTagLength = 0;
  _endMetadataTagLength = 0;
}

indri::parse::TaggedDocumentIterator::~TaggedDocumentIterator() {
  close();
}

void indri::parse::TaggedDocumentIterator::setTags( const char* startDoc, const char* endDoc, const char* endMetadata ) {
  _startDocTag = startDoc;
  _startDocTagLength = (int)strlen(startDoc);
  _endDocTag = endDoc;
  _endDocTagLength = (int)strlen(endDoc);
  _endMetadataTag = endMetadata;
  _endMetadataTagLength = endMetadata ? (int)strlen(endMetadata) : 0;
}

void indri::parse::TaggedDocumentIterator::open( const std::string& filename ) {
  _fileName = filename;
  _in = gzopen( filename.c_str(), "rb" );

  if( !_in )
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open file " + filename + "." );
}

void indri::parse::TaggedDocumentIterator::close() {
  if( _in )
    gzclose( _in );
  _in = 0;
}

bool indri::parse::TaggedDocumentIterator::_readLine( char*& beginLine, size_t& lineLength ) {
  lineLength = 0;
  size_t actual;

  // make a buffer of a reasonable size so we're not always allocating
  if( _buffer.size() < 1024*1024 )
    _buffer.grow( 1024*1024 );
  // if we're running out of room, add 25MB
  if( (_buffer.size() -  _buffer.position()) < 512*1024 ) {
    _buffer.grow( _buffer.size() + 1024*1024*25 );
  }
  if( _buffer.position() ) {
    // get rid of null terminator from previous call
    _buffer.unwrite(1);
  }

  size_t readAmount = _buffer.size() - _buffer.position() - 2;

  // fetch next document line
  char* buffer = _buffer.write( readAmount );
  char* result = gzgets( _in, buffer, (int)readAmount );
 
  if(!result) {
    return false;
  }

  actual = strlen(buffer);
  lineLength += actual; 
  _buffer.unwrite( readAmount - actual );

  // all finished reading
  *_buffer.write(1) = 0;
  beginLine = _buffer.front() + _buffer.position() - lineLength - 1;

  return true;
}

indri::parse::UnparsedDocument* indri::parse::TaggedDocumentIterator::nextDocument() {
  _document.metadata.clear();
  _buffer.clear();
  _metaBuffer.clear();

  char* beginLine;
  size_t lineLength;  
  bool result;

  indri::utility::greedy_vector<metadata_range> metadata;

  // look for <DOC> tag
  do {
    result = _readLine( beginLine, lineLength );
  } while( result && strncmp( _startDocTag, beginLine, _startDocTagLength ) );
  
  if( !result ) {
    // didn't find a begin tag, so we're done
    return 0;
  }
  // copy whatever we've read so far into the _metaBuffer.
  memcpy( _metaBuffer.write(_buffer.position()), _buffer.front(), _buffer.position() * sizeof(char));
  
  // read metadata tags
  bool openTag = false;
  int doclines = 0;

  if( _endMetadataTagLength > 0 ) {
    while( true ) {
      result = _readLine( beginLine, lineLength );

      if( !result ) {
        return 0;
      }
      // copy to the metadata buffer
      memcpy( _metaBuffer.write(lineLength), beginLine, lineLength * sizeof(char) );
      beginLine = _metaBuffer.front() + _metaBuffer.position() - lineLength;
      //
      // the beginning of the line is either:
      //    an open tag
      //    text
      //    a close tag
      //
      if( *beginLine == '<' ) {
        if( openTag ) {
          // we've already seen an open tag, so we're only interesting in
          // a matching close tag
          const char* openTagText = metadata.back().beginTag + _metaBuffer.front();
          int openTagLength = metadata.back().endTag - metadata.back().beginTag;
          
          if( beginLine[1] == '/' && !strncmp( openTagText, beginLine+2, openTagLength ) ) {
            // this is a close tag and it matches the open tag,
            // so tie up loose ends in the previous open tag metadata structure
            *beginLine = 0;
            metadata.back().endText = (int)(_metaBuffer.position() - lineLength);
            openTag = false;
          }
        } else {
          // this might be a open-only tag, or there might be
          // a close tag at the end.
          char* endTag = strchr( beginLine, '>' );
          if( endTag ) {
            *endTag = 0;
            metadata_range range;
            range.beginTag = beginLine + 1 - _metaBuffer.front();
            range.endTag = endTag - _metaBuffer.front();

            char* endText = strchr( endTag + 1, '<' );

            if( (endTag - beginLine) < (int)lineLength/2 && endText ) {
              // the end tag is also on this line
              range.beginText = endTag + 1 - _metaBuffer.front();
              range.endText = endText - _metaBuffer.front();
              *endText = 0;
              openTag = false;
            } else {
              // no more text on this line
              openTag = true;
              range.beginText = (int)_metaBuffer.position();
            }

            metadata.push_back( range );
          }
        }

        if( !strncmp( _endMetadataTag + 1, beginLine + 1, _endMetadataTagLength - 2 ) ) {
          // all finished
          // have to skip the first letter because it may have been turned into a null
          
          if( openTag ) {
            // some tag was left open, so remove it
            metadata.erase( metadata.end()-1 );
          }
          break;
        }
      }
    }
  }

  // from now on, everything is text
  int startDocument = (int)_buffer.position() - 1;
  
  while(true) {
	int lineEnd = 0;
    result = _readLine( beginLine, lineLength );
    if( !result ) {
      LEMUR_THROW( LEMUR_IO_ERROR, "Malformed document: " + _fileName );
    }
	if (beginLine[lineLength - 2] == '\r') lineEnd = 2; else lineEnd = 1;
    if( (int)lineLength >= _endDocTagLength &&
        !strncmp( beginLine+lineLength-_endDocTagLength-lineEnd, _endDocTag, _endDocTagLength ) ) {
      //      beginLine[lineLength-_endDocTagLength] = 0;
      _document.content = _buffer.front() + startDocument;
      _document.contentLength = _buffer.position() - startDocument - (_endDocTagLength + lineEnd + 1);
      // don't prune the DOC/metadata tags.
      beginLine[lineLength] = 0;
      _document.text = _buffer.front();
      _document.textLength = _buffer.position();

      // terminate document
      break;
    }
  }

  // parse metadata
  for( size_t i=0; i<metadata.size(); i++ ) {
    indri::parse::MetadataPair pair;
    char* key = _metaBuffer.front() + metadata[i].beginTag;
    
    pair.key = _metaBuffer.front() + metadata[i].beginTag;
    pair.value = _metaBuffer.front() + metadata[i].beginText;
    pair.valueLength = metadata[i].endText - metadata[i].beginText + 1;

    // convert metadata key to lowercase
    for( char* c = key; *c; c++ ) {
      if( *c >= 'A' && *c <= 'Z' ) {
        *c += ('a' - 'A');
      }
    }
    
    // special handling for docno -- remove value spaces
    if( !strcmp( pair.key, "docno" ) ) {
      pair.stripValue();
    }

    _document.metadata.push_back(pair);
  }
  
  return &_document;
}

