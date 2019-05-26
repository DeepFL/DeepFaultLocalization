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
// HTMLParser
//
// March 2004 -- metzler
//

#include "indri/HTMLParser.hpp"
#include <ctype.h>
#include "lemur/lemur-compat.hpp"
#include "indri/TagEvent.hpp"

void indri::parse::HTMLParser::initialize( TokenizedDocument* tokenized, indri::api::ParsedDocument* parsed ) {
  indri::parse::TaggedTextParser::initialize( tokenized, parsed );

  // clear URL
  url[0] = 0;
  base_url[0] = 0;

  bool have_URL = false;

  // find the DOCHDR tag, so we can yank out the URL
  for( size_t i=0; i<tokenized->metadata.size(); i++ ) {
    if( !strcmp(tokenized->metadata[i].key, "url") ) have_URL = true;
    if( !strcmp(tokenized->metadata[i].key, "dochdr") ) {
      char* beginURL = (char*) tokenized->metadata[i].value;
      char* endURL = beginURL + strcspn( (char*) tokenized->metadata[i].value, " \t\r\n" );
      int length = lemur_compat::min<int>( endURL-beginURL, sizeof url-1 );
      memcpy( url, beginURL, length );
      url[length] = 0;

      strncpy( base_url, url, sizeof url-1 );
      base_url[length] = 0;
      char* lastSlash = strrchr( base_url, '/' );
      if( lastSlash ) *lastSlash = 0;
      break;
    }
  }

  // set url
  normalizeURL(url);

  // set base_url
  normalizeURL(base_url);

  // get tag definitions
  _absoluteUrlTag = _findTag("absolute-url");
  _relativeUrlTag = _findTag("relative-url");
  _anchorTag = _findTag("a");

  // add URL to metadata
  if ( ! have_URL ) {
    indri::parse::MetadataPair pair;
    pair.key = "url";
    pair.value = url;
    pair.valueLength = (int)strlen(url)+1;
    parsed->metadata.push_back( pair );
  }

  _urlBuffer.clear();
  //  _urlBuffer.grow( parsed->textLength * 4 ); // will this be large enough?
  _urlBuffer.grow( 1024 * 1024 * 30 );
}

void indri::parse::HTMLParser::cleanup( indri::parse::TokenizedDocument* tokenized, indri::api::ParsedDocument* parsed ) {
  indri::parse::TaggedTextParser::cleanup( tokenized, parsed );
}

void indri::parse::HTMLParser::handleTag( TagEvent* te ) {

  // All tag names and attribute names will have been case folded by
  // the Tokenizer.

  if ( ! strcmp( te->name, "a" ) ) {  // <A HREF ...> tag

    bool handled_tag = false;

    // Check for an "href" attribute:

    for ( indri::utility::greedy_vector<indri::parse::AttributeValuePair>::iterator
            i = te->attributes.begin(); i != te->attributes.end(); i++ ) {
            
      if ( ! strcmp( (*i).attribute, "href" ) ) {

        if ( ! _anchorTag && ! _relativeUrlTag && ! _absoluteUrlTag )
          return;

        // URL has already been extracted and is stored in (*i).value

        prepURL( (*i).value );

        char tmp_buf[MAX_URL_LENGTH*4];
        strncpy( tmp_buf, (*i).value, lemur_compat::min<int>( strlen( (*i).value ), MAX_URL_LENGTH - 1 ) );
        tmp_buf[lemur_compat::min<int>( strlen( (*i).value ), MAX_URL_LENGTH - 1 )] = '\0';
        
        bool relative = normalizeURL( tmp_buf );

        // if special url tags are requested, we'll
        // store the url of the anchor text in the document itself
        
        const TaggedTextParser::tag_properties* tagProps;
        if( !relative ) {
          tagProps = _absoluteUrlTag;
        } else {
          tagProps = _relativeUrlTag;
        }
        
        _p_conflater->conflate( te );
        //hack to count number of terms injected
        int cnt=0;
        if( tagProps && !tagProps->exclude && !_exclude ) {
          
          // Original flag check from TaggedTextParser::writeToken
          if ( ! ( _exclude || ! _include ) ) {

            // A HREF attribute value needs to be inserted at the
            // current position in the terms vector.  A TermExtent for
            // the attribute value needs to be inserted at the current
            // position in the positions vector.
          
            // Need to get position of attribute value from
            // AttributeValuePair

            //strip scheme, tokenize url, inject into positions and terms

            int len = (int)strlen( tmp_buf );
            // Allocate space within HTMLParser's Buffer
            char* write_location = _urlBuffer.write( len + 1 );
            memcpy( write_location, tmp_buf, len + 1 );
            // hack to make whole url available to harvest links
            _document.terms.push_back( write_location );
	    size_t bufferSize1 = _urlBuffer.size();
	    write_location = _urlBuffer.write( len + 1 );
	    size_t bufferSize2 = _urlBuffer.size();
	    // This is a hack to prevent a SegFault in Harvest links
	    // Throws in exception if the buffer grows because if the buffer grows
	    // the pointers in the document are not updated and a SegFault is thrown
	    if (bufferSize2 > bufferSize1) {
	      throw std::out_of_range("_urlBuffer is not big enough.  Increase _urlBuffer.grow in HTMLParser.cpp");
	    }
            memcpy( write_location, tmp_buf, len + 1 );
            cnt++; tokens_excluded--;
            // end hack -- dmf
            char *c;
            char *urlText=write_location;
            bool lastSkipped = true; 
        
            // skip the beginning stuff (http://)
            for( c = urlText; *c; c++ ) {
              if( *c == '/' && c[1] && c[1] == '/' ) {
                urlText = c + 2;                            
              }
            }
            for( c = urlText; *c; c++ ) {              
              if( (*c >= 'A' && *c <= 'Z') ||
                  (*c >= 'a' && *c <= 'z') ||
                  (*c >= '0' && *c <= '9') ) 
                {
                  if( lastSkipped ) {
                    lastSkipped = false;
                    _document.terms.push_back( c );
                    // decrement number of tokens removed from the stream 
                    // so that future field positions line up correctly.
                    tokens_excluded--;
                    cnt++;
                  }
                } else {
                  lastSkipped = true;
                  *c = 0;
                }            
            }

            int tokBegin = (*i).begin;
            // update the positions.
            for (size_t n = _document.terms.size()-cnt; n < _document.terms.size(); n++) {
              // cant be sure there's actually text in document with relative
              TermExtent extent;
              extent.begin = tokBegin++;
              extent.end = tokBegin;
              _document.positions.push_back( extent );
            }
          }          
          addTag( tagProps->name, tagProps->name, te->pos );
          endTag( tagProps->name, tagProps->name, te->pos + cnt );
        }
        
        tagProps = _anchorTag;
        if( tagProps && !tagProps->exclude && !_exclude )
          addTag( tagProps->name, tagProps->name, te->pos + cnt );

        handled_tag = true;
      }
    }

    if ( ! handled_tag ) indri::parse::TaggedTextParser::handleTag( te );
  
  } else if ( ! strcmp( te->name, "base" ) ) { // <BASE HREF ...> tag

    bool handled_tag = false;

    for ( indri::utility::greedy_vector<indri::parse::AttributeValuePair,2>::iterator
            i = te->attributes.begin(); i != te->attributes.end(); i++ ) {

      if ( ! strcmp( (*i).attribute, "href" ) ) {

        // URL has already been extracted and is stored in (*i).value

        prepURL( (*i).value );

        int len = (int)strlen( (*i).value );

        char tmp_buf[MAX_URL_LENGTH*4];
        strncpy( tmp_buf, (*i).value, lemur_compat::min<int>( len, MAX_URL_LENGTH - 1) );
        tmp_buf[lemur_compat::min<int>( strlen( (*i).value ), MAX_URL_LENGTH - 1 )] = '\0';

        normalizeURL( tmp_buf );
        
        len = (int)strlen( tmp_buf );
        strncpy( base_url, tmp_buf, lemur_compat::min<int>( len, MAX_URL_LENGTH-1 ) );
        base_url[lemur_compat::min<int>( len, MAX_URL_LENGTH - 1 )] = '\0';

        handled_tag = true;
      }
    }

    if ( ! handled_tag ) TaggedTextParser::handleTag( te );

  } else { // any other tag

    TaggedTextParser::handleTag( te );
  }
}


// normalizes a URL (in place)
// largely based on information contained in RFC 1808
// Note: returns true if the URL was a relative one, false if it was absolute
//       the return value is not an error code here; the function should always succeed
bool indri::parse::HTMLParser::normalizeURL(char *s) {
  char *normurl = s;

  // remove the fragment identifier, query information and parameter information
  char *c;
  //  for(c = s; *c != '\0' && *c != '#' && *c != '?' && *c != ';'; c++);
  //  *c = '\0';

  // extract scheme, if given
  bool found_scheme = false;
  char scheme[MAX_URL_LENGTH];
  char netloc[MAX_URL_LENGTH];
  char path[MAX_URL_LENGTH];
  int path_len = 0;
  int scheme_len = 0;
  int netloc_len = 0;

  for(c = s; *c != '\0'; c++) {
    // scheme must have length > 0 and end with a ':'
    if(scheme_len > 0 && *c == ':') {
      found_scheme = true;
      strncpy(scheme, s, scheme_len);
      scheme[scheme_len] = '\0';
      // convert scheme to lowercase
      for(int i = 0; i < scheme_len; i++) {
        if( scheme[i] >='A' && scheme[i] <='Z' )
          scheme[i] = scheme[i] + ('a' - 'A');
      }
      c++;

      // extract network location
      if(*c == '/' && *(c+1) == '/') c+=2; // skip "//"
      char *netloc_begin = c;
      for(; *c != '\0' && *c != '/'; c++)
        netloc_len++;
      strncpy(netloc, netloc_begin, netloc_len);
      netloc[netloc_len] = '\0';
      // convert netloc to lowercase
      int colon_loc = -1;
      for(int i = 0; i < netloc_len; i++) {
        if(netloc[i] == ':')
          colon_loc = i;
        else if(colon_loc > -1 && !isdigit((unsigned char) netloc[i]))
          colon_loc = -1;

        if( netloc[i] > 'A' && netloc[i] < 'Z' )
          netloc[i] = netloc[i] + ('a' - 'A');
      }
      if(colon_loc > -1)
        netloc[colon_loc] = '\0';

      // extract the path
      for(; *c == '/'; c++); // skip leading slashes
      path_len = s + (int)strlen(s) - c;
      if(path_len > 0) {
        strncpy(path, c, path_len);
        path[path_len] = '\0';
      }
      break;
    }

    char ch = *c;

    // only alpha + num + '+' + '-' + '.' are allowed to appear in scheme
    if( ((ch < 'A' || ch > 'Z') && (ch < 'a' || ch > 'z')) && // not alpha
        (ch < '0' || ch > '9') && // not digit
        ch != '+' &&
        ch != '-' && 
        ch != '.' )
      break;

    scheme_len++;
  }

  // absolute url
  int dotCleanStart = -1;

  if(found_scheme) {
    if(strcmp(scheme, "http") == 0 || strcmp(scheme, "https") == 0) {
      if(path_len == 0) {
        memcpy( normurl, scheme, scheme_len );
        memcpy( normurl + scheme_len, "://", 3 );
        memcpy( normurl + scheme_len + 3, netloc, netloc_len+1 );
      } else {
        strcpy( normurl, scheme );
        strcat( normurl, "://" );
        strcat( normurl, netloc );
        strcat( normurl, "/" );
        strcat( normurl, path );
        dotCleanStart = scheme_len + 3 + netloc_len + 1;
      }
    }
    else {
      sprintf(normurl, "%s:%s", scheme, netloc);
    }
  }
  // relative url
  else {
    // while unlikely, base_url may be a relative url, in which case
    // we do not want to be performing the strcat operations on it.    
    if (s != base_url) {
      char tmp_buf[MAX_URL_LENGTH*4];
      strncpy(tmp_buf, s, MAX_URL_LENGTH-1);
      tmp_buf[MAX_URL_LENGTH-1] = 0;
      if(*s == '/') {
        normurl[0] = 0;
        strcat( normurl, base_url );
        strcat( normurl, tmp_buf );
      } else {
        normurl[0] = 0;
        strcat( normurl, base_url );
        strcat( normurl, "/" );
        strcat( normurl, tmp_buf );
      }
    }
    
    char* colonSlashSlash = 0;
    char* slash = 0;

    colonSlashSlash = strstr( normurl, "://" );
    if( colonSlashSlash )
      slash = strstr( colonSlashSlash + 3, "/" );
    if( slash )
      dotCleanStart = slash - normurl;
  }

  indri::utility::greedy_vector<int,32> slashes;
  indri::utility::greedy_vector<bool,32> usable;
  int normLength = (int)strlen(normurl);

  // get rid of ".." directories
  if( dotCleanStart >= 0 ) {
    bool unusableFound = false;

    // find slashes
    // if the space between two slashes is "../", that's not useful to us,
    // so mark that fact
    for( int i=dotCleanStart; i<normLength; i++ ) {
      if( normurl[i] == '/' ) {
        if( slashes.size() ) {
          int lastSlash = slashes[slashes.size()-1];
          if( !strncmp( "../", normurl + lastSlash + 1, 3 ) ) {
            usable.push_back(false);
            unusableFound = true;
          } else {
            usable.push_back(true);
          }
        }
        
        slashes.push_back(i);
      }
    }

    if( normLength && normurl[normLength-1] != '/' ) {
      if( strncmp( "..", normurl + normLength - 2, 2 ) == 0 ) {
        unusableFound = true;
        usable.push_back(false);
      } else {
        usable.push_back(true);
      }
    }

    // some ".." dir was found, so we're going to clean it out
    if( unusableFound ) {
      for( size_t i=0; i<usable.size(); i++ ) {
        if( !usable[i] ) {
          // search back to find something to mark false
          // to account for this '..'
          for( int j=(int)i; j>=0; j-- ) {
            if( usable[j] ) {
              usable[j] = false;
              break;
            }
          }
        }
      }

      // now that we've marked the usable spots, we just have to copy
      // the url parts we want

      char tmp_buf[MAX_URL_LENGTH*4];
      tmp_buf[0] = 0;
      strncpy(tmp_buf, normurl, dotCleanStart);
      tmp_buf[dotCleanStart] = 0;

      for( size_t i=0; i<usable.size(); i++ ) {
        if( usable[i] ) {
          if( slashes.size() == i+1 )
            strcat( tmp_buf, normurl + slashes[i] );
          else
            strncat( tmp_buf, normurl + slashes[i], slashes[i+1] - slashes[i] );
        }
      }

      strncpy(s, tmp_buf, MAX_URL_LENGTH-1);
      s[MAX_URL_LENGTH-1] = 0;
    }
  }

  return !found_scheme;
}

// preps a URL (in place)
// Remove trailing whitespace by terminating the string.
void indri::parse::HTMLParser::prepURL( char *s ) {

  int len = (int)strlen( s );
  int i;

  for ( i = 0; i < len; i++ ) {
#ifndef WIN32
    if ( isspace( s[i] ) ) {
#else
    if ( s[i] >= 0 && isspace( s[i] ) ) {
#endif
      s[i] = '\0';
      break;
    }
  }
}

