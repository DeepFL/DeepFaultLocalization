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
// XMLReader
//
// 8 October 2003 - tds
//

#include "indri/XMLReader.hpp"
#include "lemur/Exception.hpp"

int indri::xml::XMLReader::_tryFindChar( char ch, const char* buffer, int start, int finish ) {
  int i;

  for( i=start; i<finish; i++ ) {
    if( buffer[i] == ch )
      break;
  }

  if( i==finish )
    return -1;

  return i;
}

int indri::xml::XMLReader::_findChar( char ch, const char* buffer, int start, int finish ) {
  int result = _tryFindChar( ch, buffer, start, finish );

  if( result == -1 )
    LEMUR_THROW( LEMUR_PARSE_ERROR, "Was looking for '" + ch + "', but couldn't find it." );

  return result;
}

int indri::xml::XMLReader::_tryFindBeginTag( const char* buffer, int start, int finish ) {
  return _tryFindChar( '<', buffer, start, finish );
}

int indri::xml::XMLReader::_findBeginTag( const char* buffer, int start, int finish ) {
  int result = _tryFindBeginTag( buffer, start, finish );

  if( result == -1 )
    LEMUR_THROW( LEMUR_PARSE_ERROR, "Ran off the end of a buffer while looking for a begin tag" );

  return result;
}

int indri::xml::XMLReader::_findEndTag( const char* buffer, int start, int finish ) {
  return _findChar( '>', buffer, start, finish );
}

int indri::xml::XMLReader::_tryFindText( const char* buffer, int start, int finish ) {
  int i;

  for( i=start; i<finish; i++ ) {
    if( !isspace(buffer[i]) )
      break;
  }

  return i;
}

int indri::xml::XMLReader::_findText( const char* buffer, int start, int finish ) {
  int result = _tryFindText( buffer, start, finish );
  if( result==finish )
    LEMUR_THROW( LEMUR_GENERIC_ERROR, "Was looking for text, but couldn't find any" );

  return result;
}

int indri::xml::XMLReader::_findNotName( const char* buffer, int start, int finish ) {
  int i;

  for( i=start; i<finish; i++ ) {
    // this isn't unicode-safe, but it should be good for now
#ifndef WIN32
    if( !isalpha(buffer[i]) && 
        !isdigit(buffer[i]) &&
#else
    if( (buffer[i] >= 0 && !isalpha(buffer[i])) && 
        (buffer[i] >= 0 && !isdigit(buffer[i])) &&
#endif
        buffer[i] != '-' &&
        buffer[i] != '_' &&
        buffer[i] != ':' &&
        buffer[i] != '.' ) {
      break;
    }
  }

  if( i==finish )
    LEMUR_THROW( LEMUR_PARSE_ERROR, "Was looking for the end of a tag name, but couldn't find it." );

  return i;
}

int _findSpace( char* buffer, int start, int finish ) {
  int i;

  for( i=start; i<finish; i++ ) {
    if( isspace(buffer[i]) )
      break;
  }

  if( i==finish )
    LEMUR_THROW( LEMUR_PARSE_ERROR, "Was looking for a space, but couldn't find it." );

  return i;
}

int indri::xml::XMLReader::_readTag( const char* buffer, int bufferStart, int bufferEnd, std::string* tagName, std::map<std::string, std::string>* attributes, int* tagType ) {
  // skip opening whitespace
  int startLocation = bufferStart;
  int endLocation = _findEndTag( buffer, startLocation, bufferEnd );
  int position = startLocation+1;
  int trueEndLocation = endLocation+1;

  if( endLocation - position < 1 ) 
    LEMUR_THROW( LEMUR_GENERIC_ERROR, "Found a tag with no body" );

  // is it a <!CDATA[ tag?
  // expand this test for completeness
  if ( buffer[position] == '!' && buffer[position + 1] == 'C' ) {
    if( tagType )
      *tagType = TAG_CDATA_TYPE;
    trueEndLocation = position + 7;
    return trueEndLocation;
  }
  // is it an opening tag?
  if( buffer[position] == '/' ) {
    if( tagType )
      *tagType = TAG_CLOSE_TYPE;
    position++;

    if( position >= endLocation )
      LEMUR_THROW( LEMUR_GENERIC_ERROR, "Found a tag with no body" );
  } else {
    if( buffer[endLocation-1] == '/' ) {
      if( tagType )
        *tagType = TAG_OPEN_CLOSE_TYPE;
      endLocation--;
    } else if( tagType ) {
      *tagType = TAG_OPEN_TYPE;
    }
  }

  if( tagName || attributes ) {
    int textBegin = _findText( buffer, position, endLocation );
    int textEnd = _findNotName( buffer, textBegin, endLocation+1 );

    if( tagName )
      tagName->assign( &buffer[textBegin], &buffer[textEnd] );

    position = textEnd;

    if( attributes ) {
      attributes->clear();

      textBegin = _findText( buffer, position, endLocation+1 );
      position = textBegin;

      for( ; position != endLocation; position = _tryFindText( buffer, position, endLocation ) ) {
        textEnd = _findNotName( buffer, position, endLocation );
        int equalsPosition = _findChar( '=', buffer, textEnd, endLocation );
        int quotePosition = _findText( buffer, equalsPosition+1, endLocation );
        int endQuotePosition = _findChar( buffer[quotePosition], buffer, quotePosition+1, endLocation );

        std::string attributeName;
        std::string valueText;

        assert( position <= textEnd );
        assert( quotePosition+1 <= endQuotePosition );
        assert( textEnd < quotePosition+1 );

        attributeName.assign( &buffer[position], &buffer[textEnd] );
        valueText.assign( &buffer[quotePosition+1], &buffer[endQuotePosition] );

        attributes->insert( std::make_pair( attributeName, valueText ) );
        position = endQuotePosition+1;
      }
    }
  }

  return trueEndLocation;
}

int indri::xml::XMLReader::_findClosingTag( const char* buffer, int start, int finish, std::string& openingTagName, bool* tagsBetween ) {
  int openingTags = 0;
  int closingTags = 0;
  int position = start;
  bool done = false;
  bool match = false;
  int tagType;

  if( tagsBetween )
    *tagsBetween = false;
  try {
    while( !done ) {
      std::string tagName;
      position = _findBeginTag( buffer, position, finish );
      int end = _readTag( buffer, position, finish, &tagName, NULL, &tagType );

      if( tagType == TAG_CDATA_TYPE ) {
        std::string cdata = &buffer[end];
        std::string::size_type dataEnd = cdata.find("]]>");
        position = end + dataEnd + 1;
      } else if( tagType != TAG_CLOSE_TYPE ) {
        if( tagsBetween )
          *tagsBetween = true;

        if( tagType == TAG_OPEN_TYPE )
          openingTags++;
        position = end;

        while( openingTags > closingTags ) {
          // don't need to check for matching tags here, we just need to
          // count open and closed tags
          position = _findBeginTag( buffer, position, finish );
          end = _readTag( buffer, position, finish, NULL, NULL, &tagType );
          position = end;

          if( tagType == TAG_CDATA_TYPE ) {
            std::string cdata = &buffer[end];
            std::string::size_type dataEnd = cdata.find("]]>");
            position = end + dataEnd + 1;
          } else if( tagType == TAG_OPEN_TYPE ) {
            openingTags++;
          } else if( tagType == TAG_CLOSE_TYPE ) {
            closingTags++;
          }
        }
      } else {
        match = (tagName == openingTagName);
        done = true;
      }
    }
  } catch( lemur::api::Exception& e ) {
    LEMUR_RETHROW( e, std::string() + "Caught an error while looking for an end tag for '" + openingTagName + "'" );
  }

  if( match ) {
    return position;
  } else {
    return -1;
  }
}

void indri::xml::XMLReader::_read( indri::xml::XMLNode** parent, const char* buffer, int start, int end ) {
  int tagType;

  for( int current = _tryFindBeginTag( buffer, start, end );
       current >= 0;
       current = _tryFindBeginTag( buffer, current, end ) ) {
    indri::xml::XMLNode* node;
    std::string tagName;
    std::map<std::string, std::string> attributes;
    bool tagsBetween;

    int endLevel;
    int endTag = _readTag( buffer, current, end, &tagName, &attributes, &tagType );

    if( tagType == TAG_CLOSE_TYPE )
      LEMUR_THROW( LEMUR_GENERIC_ERROR, "Found a close tag for '" + tagName + "' while looking for an open tag." );

    if( tagType == TAG_OPEN_TYPE ) {
      int closingTag = _findClosingTag( buffer, endTag, end, tagName, &tagsBetween );
      if( closingTag == -1 )
        LEMUR_THROW( LEMUR_GENERIC_ERROR, "Could not find a close tag for '" + tagName + "'");

      if( tagsBetween ) {
        node = new indri::xml::XMLNode( tagName, attributes );
        _read( &node, buffer, endTag, closingTag );
      } else {
        std::string nodeValue;
        nodeValue.assign( &buffer[endTag], &buffer[closingTag] ); 
        std::string::size_type dataStart = nodeValue.find("<!CDATA[");
        while (dataStart != std::string::npos) {          
          // munch any CDATA tags in the element's value.
          nodeValue.erase(dataStart, 8);
          std::string::size_type dataEnd = nodeValue.find("]]>");
          if (dataEnd != std::string::npos) 
            nodeValue.erase(dataEnd, 3);
          // else bad things here, should throw.
          dataStart = nodeValue.find("<!CDATA[");
        }
        node = new indri::xml::XMLNode( tagName, attributes, nodeValue );
      }

      endLevel = _findEndTag( buffer, closingTag, end )+1;
    } else {
      assert( tagType == TAG_OPEN_CLOSE_TYPE );
      node = new indri::xml::XMLNode( tagName, attributes );
      endLevel = endTag;
    }

    if( *parent ) {
      (*parent)->addChild( node );
    } else {
      *parent = node;
      break;
    }

    current = endLevel;
  }
}

indri::xml::XMLNode* indri::xml::XMLReader::read( const char* buffer, size_t length ) {
  indri::xml::XMLNode* result = NULL;
  std::string s = buffer;
  std::string::size_type commentstart = s.find("<!--",0);
  if (commentstart != std::string::npos) {
    //contains comments, strip 'em
    while (commentstart != std::string::npos) {
      std::string::size_type commentend = s.find("-->",0);
      s.erase(commentstart, (commentend + 3) - commentstart);
      commentstart = s.find("<!--",0);
    }
    _read( &result, s.c_str(), 0, int(s.length()) );
  } else {
    // no comments in string
    _read( &result, buffer, 0, int(length) );
  }
  return result;
}

indri::xml::XMLNode* indri::xml::XMLReader::read( const std::string& str ) {
  return read( str.c_str(), str.length() );
}


