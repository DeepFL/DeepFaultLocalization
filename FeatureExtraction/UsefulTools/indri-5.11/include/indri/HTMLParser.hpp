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
#ifndef HTMLPARSER_HPP
#define HTMLPARSER_HPP
#include "indri/TaggedTextParser.hpp"

#ifndef MAX_URL_LENGTH
#define MAX_URL_LENGTH 4096
#endif
namespace indri
{
  namespace parse
  {
    
    class HTMLParser : public TaggedTextParser {
    public:
      HTMLParser() {
      }
  
      ~HTMLParser() { }

    protected:
      virtual void initialize( TokenizedDocument* tokenized, indri::api::ParsedDocument* parsed );
      virtual void cleanup( TokenizedDocument* tokenized, indri::api::ParsedDocument* parsed );
      virtual void handleTag(TagEvent *te);
      char url[MAX_URL_LENGTH];
      char base_url[MAX_URL_LENGTH];
      void prepURL(char *s);
      bool normalizeURL(char *s);

      tag_properties* _relativeUrlTag;
      tag_properties* _absoluteUrlTag;
      tag_properties* _anchorTag;

      indri::utility::Buffer _urlBuffer;
    };
  }
}

#endif
