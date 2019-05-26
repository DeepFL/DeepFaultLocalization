%option noyywrap
%option never-interactive
%option prefix="tok"

%{

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
// TextTokenizer
//
// 15 September 2005 -- mwb
//

#include <iostream>
#include <string.h>
#include <ctype.h>
#include "indri/TextTokenizer.hpp"
#include "indri/TermExtent.hpp"
#include "indri/TagEvent.hpp"
#include "indri/TokenizedDocument.hpp"
#include "indri/UnparsedDocument.hpp"
#include "indri/UTF8Transcoder.hpp"
#include "indri/AttributeValuePair.hpp"

static long byte_position;

#define ZAP           1
#define TAG           2
#define ASCII_TOKEN   3
#define UTF8_TOKEN    4

%}
%x COMMENT
%%

"<!--" { BEGIN(COMMENT); byte_position += tokleng; return ZAP; }
<COMMENT>[^-]+ { byte_position += tokleng; return ZAP; }
<COMMENT>"-->" { BEGIN(INITIAL); byte_position += tokleng; return ZAP; }
<COMMENT>"-"[^-]* { byte_position += tokleng; return ZAP; }
"<!"[^-][^\>]*">" { byte_position += tokleng; return ZAP; }
"<%"[^%\>]+"%>" { byte_position += tokleng; return ZAP; }
"<?xml"[^\>]*">" { byte_position += tokleng; return ZAP; }
\<[a-zA-Z/][^\>]*\>                                             { byte_position += tokleng; return TAG; }
[&]([a-zA-Z]+|[#]([0-9]+|[xX][a-fA-F0-9]+))[;]         { byte_position += tokleng; return ZAP; /* symbols */ }
[A-Z0-9]"."([A-Z0-9]".")*                                        { byte_position += tokleng; return ASCII_TOKEN; }
[a-zA-Z0-9']+                                        { byte_position += tokleng; return ASCII_TOKEN; }
"-"[0-9]+("."[0-9]+)?                                  { byte_position += tokleng; return ASCII_TOKEN; }
[a-zA-Z0-9\x80-\xFD]+                               { byte_position += tokleng; return UTF8_TOKEN; }

[\n]                                                   { byte_position += tokleng; return ZAP; }
.                                                      { byte_position += tokleng; return ZAP; }

%%

indri::parse::TokenizedDocument* indri::parse::TextTokenizer::tokenize( indri::parse::UnparsedDocument* document ) {

  _termBuffer.clear();
  if ( _tokenize_entire_words)
    _termBuffer.grow( document->textLength * 4);
  else
    _termBuffer.grow( document->textLength * 8 ); // extra null per char.

  _document.terms.clear();
  _document.tags.clear();
  _document.positions.clear();
  
  _document.metadata = document->metadata;
  _document.text = document->text;
  _document.textLength = document->textLength;
  _document.content = document->content;
  _document.contentLength = document->contentLength;

  BEGIN(INITIAL);
  // byte offset
  byte_position = document->content - document->text;

  tok_scan_bytes( document->content, document->contentLength );

  // Main Tokenizer loop

  int type;

  while ( (type = toklex()) ) {

    switch ( type ) {

    case ASCII_TOKEN: processASCIIToken(); break;

    case UTF8_TOKEN: processUTF8Token(); break;

    case TAG: if ( _tokenize_markup ) processTag(); break;

    default:
    case ZAP:
      break;

    }

  }

  tok_delete_buffer( YY_CURRENT_BUFFER );

  return &_document;
}

// Member functions for processing tokenization events as dispatched
// from the main tokenizer loop

void indri::parse::TextTokenizer::processTag() {

  // Here, we parse the tag in a fashion that is relatively robust to
  // malformed markup.  toktext matches this pattern: <[^>]+>

  if ( toktext[1] == '?' || toktext[1] == '!' ) { 
    
    // XML declaration like <? ... ?> and <!DOCTYPE ... >
    return; // ignore

  } else if ( toktext[1] == '/' ) { // close tag, eg. </FOO>

    // Downcase the tag name.

    int len = 0;

    for ( char *c = toktext + 2; 
#ifndef WIN32
          isalnum( *c ) || *c == '-' || *c == '_' || *c == ':' ; c++ ) {
#else
          ((*c >= 0) && isalnum( *c )) || *c == '-' || *c == '_' || *c == ':' ; c++ ) {
#endif

      *c = tolower( *c );
      if ( *c == ':' ) *c = '_'; /* replace colon (from namespaces) */
      len++;
    }

    TagEvent te;

    te.open_tag = false;

    // We need to write len characters, plus a NULL
    char* write_loc = _termBuffer.write( len + 1 );
    strncpy( write_loc, toktext + 2, len );
    write_loc[len] = '\0';
    te.name = write_loc;

    // token position of tag event w/r/t token string
    te.pos = _document.terms.size();

    te.begin = byte_position - tokleng;
    te.end = byte_position;

    _document.tags.push_back( te );
    
#ifndef WIN32
    } else if ( isalpha( toktext[1] ) ) {
#else
    } else if ( (toktext[1]  >= 0) && (isalpha( toktext[1] ) )) {
#endif

    // Try to extract the tag name:

    char* c = toktext + 1;
    int i = 0;
    int offset = 1; // current offset w/r/t byte_position - tokleng
    // it starts at one because it is incremented when c is, and c starts at one.
    char* write_loc;

#ifndef WIN32
    while ( isalnum( c[i] ) || c[i] == '-' || c[i] == '_' || c[i] == ':' ) i++;
#else
    while ( ( (c[i] >= 0) && isalnum( c[i] )) || c[i] == '-' || c[i] == '_' || c[i] == ':' ) i++;
#endif
    if ( c[i] == '>' ) {

      // open tag with no attributes, eg. <title>

      // Ensure tag name is downcased
      for ( int j = 0; j < i; j++ ) {
        c[j] = tolower( c[j] );
        if ( c[j] == ':' ) c[j] = '_'; /* replace colon (from namespaces) */
      }

      TagEvent te;

      te.open_tag = true;
      
      // need to write i characters, plus a NULL
      char* write_loc = _termBuffer.write( i + 1 );
      strncpy( write_loc, c, i );
      write_loc[i] = '\0';
      te.name = write_loc;

      te.pos = _document.terms.size();

      te.begin = byte_position - tokleng;
      te.end = byte_position;
      
      _document.tags.push_back( te );

#ifndef WIN32
    } else if ( isspace( c[i] ) ) {
#else
    } else if ( (c[i]  >= 0) && (isspace( c[i] ) )) {
#endif

      // open tag with attributes, eg. <A HREF="www.foo.com/bar">

      TagEvent te;

      te.open_tag = true;

      // Ensure tag name is downcased
      for ( int j = 0; j < i; j++ ) {
        c[j] = tolower( c[j] );
        if ( c[j] == ':' ) c[j] = '_'; /* replace colon (from namespaces) */
      }

      // need to write i characters, plus a NULL
      char* write_loc = _termBuffer.write( i + 1 );
      strncpy( write_loc, c, i );
      write_loc[i] = '\0';
      te.name = write_loc;
      c += i;
      offset += i;

#ifndef WIN32
    while ( isspace( *c ) ) { c++; offset++; }
#else
    while (((*c) >=0) &&  isspace( *c )) { c++; offset++; }
#endif

      te.pos = _document.terms.size();

      te.begin = byte_position - tokleng;
      te.end = byte_position;

      // Now search for attributes:

      while ( *c != '>' && *c != '\0' ) { 

        AttributeValuePair avp;

        // Try to extract attribute name:

        i = 0;
#ifndef WIN32
        while ( isalnum( c[i] ) || c[i] == '-' || c[i] == '_' ) i++;
#else
        while ( (c[i] >= 0) && isalnum( c[i] ) || c[i] == '-' || c[i] == '_') i++;
#endif

        if ( i == 0 ) break;

        // Ensure attribute name is downcased
        for ( int j = 0; j < i; j++ )
          c[j] = tolower( c[j] );

        // need to write i characters, plus a NULL
        write_loc = _termBuffer.write( i + 1 );
        strncpy( write_loc, c, i );
        write_loc[i] = '\0';
        avp.attribute = write_loc;
        c += i;
        offset += i;

        // attributes can be foo\s*=\s*"bar[">] or foo\s*=\s*bar

		// ignore any spaces
#ifndef WIN32
    while ( isspace( *c ) ) { c++; offset++; }
#else
    while (((*c) >=0) &&  isspace( *c )) { c++; offset++; }
#endif

        if ( *c == '=' ) {

          c++; // get past the '=' sign.
          offset++;

#ifndef WIN32
    while ( isspace( *c ) ) { c++; offset++; }
#else
    while (((*c) >=0) &&  isspace( *c )) { c++; offset++; }
#endif

          if ( *c == '>' ) {

            // common malformed markup <a href=>

            // Insert empty attribute value
            // need to write a single NULL
            write_loc = _termBuffer.write( 1 );
            write_loc[0] = '\0';
            avp.value = write_loc;
            avp.begin = byte_position - tokleng + offset;
            avp.end = byte_position - tokleng + offset;

          } else {

            bool quoted = true;
            char quote_char;
            if ( *c == '"' || *c =='\'' ) { quote_char = *c; c++; offset++; }
            else quoted = false;

            // Attribute value starts here.

            i = 0;
// make sure the opening and closing quote character match...
            if ( quoted ) 
//              while ( c[i] != '"' && c[i] != '>' && c[i] !='\'') i++;
              while ( c[i] != quote_char && c[i] != '>') i++;
            else
#ifndef WIN32
              while ( ! isspace( c[i] ) && c[i] != '>' ) i++;
#else
              while ( ((c[i] >= 0)  && ! isspace( c[i] ) ) && c[i] != '>' ) i++;
#endif

            // need to write i characters, plus a NULL
            write_loc = _termBuffer.write( i + 1 );
            strncpy( write_loc, c, i );
            write_loc[i] = '\0';
            avp.value = write_loc;
            avp.begin = byte_position - tokleng + offset;
            avp.end = byte_position - tokleng + offset + i;
            c += i;
            offset += i;

          }
        } else {

          // Insert empty attribute value
          // need to write a single NULL
          write_loc = _termBuffer.write( 1 );
          write_loc[0] = '\0';
          avp.value = write_loc;
          avp.begin = byte_position - tokleng + offset;
          avp.end = byte_position - tokleng + offset;
        }
#ifndef WIN32
        while ( isspace( *c ) || *c == '"' ) { c++; offset++; }
#else
        while ( ((*c >= 0) && isspace( *c )) || *c == '"' ) { c++; offset++; }
#endif

        te.attributes.push_back( avp );
      }

      _document.tags.push_back( te );

    }

    // One of the cases that is ignored is this common malformed
    // markup <foo=bar> with no tag name.  Another is the case
    // of an email address <foo@bar.com>


  }
}

void indri::parse::TextTokenizer::processUTF8Token() {

  // A UTF-8 token, as recognized by flex, could actually be
  // a mixed ASCII/UTF-8 string containing any number of 
  // UTF-8 characters, so we re-tokenize it here.

  indri::utility::HashTable<UINT64,const int>& unicode = _transcoder.unicode();

  int len = strlen( toktext );

  UINT64* unicode_chars = new UINT64[len + 1];
  int* offsets = new int[len + 1];
  int* lengths = new int[len + 1];
  _transcoder.utf8_decode( toktext, &unicode_chars, NULL, NULL,
                           &offsets, &lengths );

  const int* p;
  int cls;             // Character class of current UTF-8 character
  // offset of current UTF-8 character w/r/t toktext stored in offsets[i]
  // byte length of current UTF-8 character stored in lengths[i]

  int offset = 0;      // Position of start of current *token* (not character) w/r/t toktext
  int extent = 0;      // Extent for this *token* including trailing punct
  int token_len = 0;   // Same as above, minus the trailing punctuation

  char buf[64];

  // If this flag is true, we have punctuation symbols at the end of a
  // token, so do not attach another letter to this token.
  bool no_letter = false; 

  // In case there are malformed characters preceding the good
  // characters:
  offset = offsets[0];

  for ( int i = 0; unicode_chars[i] != 0; i++ ) {

    p = unicode.find( unicode_chars[i] );
    cls = p ? *p : 0;

    if ( ! _tokenize_entire_words ) { // Tokenize by character

      if ( cls != 0 && cls != 3 && cls != 5 && cls != 9 ) {

        writeToken( toktext + offsets[i], lengths[i],
                    byte_position - tokleng + offsets[i], 
                    byte_position - tokleng + offsets[i] + lengths[i] );
      }
      continue;
    }

    // If this is not the first time through this loop, we need
    // to check to see if any bytes in toktext were skipped 
    // during the UTF-8 analysis:

    if ( i != 0 && offset + token_len != offsets[i] ) {

      // Write out the token we are working on, if any:

      if ( token_len > 0 ) {

        writeToken( toktext + offset, token_len,
                    byte_position - tokleng + offset,
                    byte_position - tokleng + offset + extent );
      }

      extent = 0;
      token_len = 0;
      no_letter = false;
      offset = offsets[i];
    }

    // Tokenize by word:

    switch ( cls ) {

    case 4: // Currency symbol: always extracted alone
      // Action: write the token we are working on,
      // and write this symbol as a separate token
      writeToken( toktext + offset, extent,
                  byte_position - tokleng + offset,
                  byte_position - tokleng + offset + extent );

      offset += extent;

      writeToken( toktext + offset, lengths[i], 
                  byte_position - tokleng + offset, 
                  byte_position - tokleng + offset + lengths[i] );

      offset += lengths[i];
      token_len = 0;
      extent = 0;
      no_letter = false;
      break;

    case 1: // Apostrophe
    case 10: // Decimal separator
    case 6: // Letter
    case 7: // Digit
      // Action: add this character to the end of the token we are
      // working on
      if ( no_letter ) { // This is a token boundary
        writeToken( toktext + offset, token_len,
                    byte_position - tokleng + offset,
                    byte_position - tokleng + offset + extent );

        offset += extent;
        extent = 0;
        token_len = 0;
        no_letter = false;

      }

      extent += lengths[i];
      token_len += lengths[i];
      break;

    case 2: // Percent
    case 8: // Punctuation
    case 12: // Thousands separator
    case 11: // Hyphen
      // Action: These characters are included in the extent of the
      // token we are working on.
      no_letter = true;
      extent += lengths[i];
      break;

    case 0: // No character class!
    case 3: // Control character
    case 5: // Non-punctuation symbol
    case 9: // Whitespace
    default:
      // Action: write the token we are working on.  Do not include
      // this character in any future token.
      writeToken( toktext + offset, token_len,
                  byte_position - tokleng + offset,
                  byte_position - tokleng + offset + extent );

      offset += (extent + lengths[i]); // Include current character
      extent = 0;
      token_len = 0;
      no_letter = false;

      break;
    }
  }

  // Write out last token
  if ( token_len > 0 )
    writeToken( toktext + offset, token_len,
                byte_position - tokleng + offset,
                byte_position - tokleng + offset + extent );
  
  delete[] unicode_chars;
  delete[] offsets;
  delete[] lengths;
}

void indri::parse::TextTokenizer::processASCIIToken() {

  int token_len = strlen( toktext );

  // token_len here is the length of the token without
  // any trailing punctuation.

  for ( int i = token_len - 1; i > 0; i-- ) {

    if ( ! ispunct( toktext[i] ) )
      break;
    else
      token_len--;
  }

  if ( _tokenize_entire_words ) {

    writeToken( toktext, token_len, byte_position - tokleng, byte_position );

  } else {

    for ( int i = 0; i < token_len; i++ )
      writeToken( toktext + i, 1, byte_position - tokleng + i, 
                  byte_position - tokleng + i + 1 );
  }
}


// ObjectHandler implementation

void indri::parse::TextTokenizer::handle( indri::parse::UnparsedDocument* document ) {

  _handler->handle( tokenize( document ) );
}

void indri::parse::TextTokenizer::setHandler( ObjectHandler<indri::parse::TokenizedDocument>& h ) {

  _handler = &h;
}

void indri::parse::TextTokenizer::writeToken( char* token, int token_len, 
                                              int extent_begin, int extent_end ) {


  // The TermExtent for a token will include trailing punctuation.
  // The purpose for this is that it makes for a nicer display when a
  // sequence of tokens (say, a sentence) is retrieved and shown to
  // the user.

  TermExtent extent;
  extent.begin = extent_begin;
  extent.end = extent_end;
  _document.positions.push_back( extent );

  // The terms entry for a token won't include the punctuation.

  char* write_loc = _termBuffer.write( token_len + 1 );
  strncpy( write_loc, token, token_len );
  write_loc[token_len] = '\0';
  _document.terms.push_back( write_loc );
}


