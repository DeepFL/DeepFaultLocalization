/*==========================================================================
 * Copyright (c) 2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// TaggedTextParser
//
// 15 September 2005 -- revised by mwb
//

#ifndef INDRI_TAGGEDTEXTPARSER_HPP
#define INDRI_TAGGEDTEXTPARSER_HPP

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include "indri/HashTable.hpp"
#include "indri/TagList.hpp"
#include "indri/IndriParser.hpp"
#include "indri/Buffer.hpp"
#include "indri/TokenizedDocument.hpp"
#include "lemur/string-set.h"
#include "indri/ConflationPattern.hpp"
#include "indri/Conflater.hpp"

#define MAX_DOCNO_LENGTH 128
#define PARSER_MAX_BUF_SIZE 1024

namespace indri
{
  namespace parse
  {
    
    class TaggedTextParser : public Parser {
    public:
      TaggedTextParser();
      ~TaggedTextParser();
  
      void setTags( const std::vector<std::string>& include,
                    const std::vector<std::string>& exclude,
                    const std::vector<std::string>& index,
                    const std::vector<std::string>& metadata, 
                    const std::map<indri::parse::ConflationPattern*,std::string>& conflations );

      indri::api::ParsedDocument* parse( TokenizedDocument* document );

      void handle( TokenizedDocument* document );
      void setHandler( ObjectHandler<indri::api::ParsedDocument>& h );

    protected:
      typedef indri::utility::HashTable<std::string, std::string> StrHashTable;


      virtual void initialize( TokenizedDocument* document, indri::api::ParsedDocument* parsed );
      virtual void cleanup( TokenizedDocument* document, indri::api::ParsedDocument* parsed );

      void addTag(const char *s, const char* c, int pos) { tl->addTag(s, c, pos); }
      void endTag(const char *s, const char* c, int pos) { tl->endTag(s, c, pos); }

      void addMetadataTag(const char* s, const char* c, int pos) { _metaList->addTag(s, c, pos); }
      void endMetadataTag(const char* s, const char* c, int pos) { _metaList->endTag(s, c, pos); }

      Conflater* _p_conflater;

      // tag list
      TagList* tl;
      TagList* _metaList;
      indri::utility::Buffer _termBuffer;

      struct tag_properties {
        const char* name;
        bool index;
        bool exclude;
        bool include;
        bool metadata;
      };

      tag_properties* _findTag( std::string name );
      tag_properties* _buildTag( std::string name );

      indri::utility::HashTable<const char*,tag_properties*> _tagTable;

      virtual void handleTag( TagEvent* te );

      const tag_properties* _startExcludeRegion;
      const tag_properties* _startIncludeRegion;
  
      bool _exclude;
      bool _include;
      bool _defaultInclude;
  
      unsigned int token_pos;
      unsigned int tokens_excluded;

      indri::api::ParsedDocument _document;
  
    private:
      ObjectHandler<indri::api::ParsedDocument>* _handler;

    };

  }
}

#endif // INDRI_TAGGEDTEXTPARSER_HPP
