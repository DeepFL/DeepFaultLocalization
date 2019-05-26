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
// TextParser
//
// 16 August 2004 -- tds
//

#ifndef INDRI_TEXTPARSER_HPP
#define INDRI_TEXTPARSER_HPP

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <string>
#include <vector>
#include "indri/IndriParser.hpp"
#include "indri/Buffer.hpp"
#include "indri/ConflationPattern.hpp"
#include "lemur/string-set.h"
namespace indri
{
  namespace parse
  {
    
    class TextParser : public Parser {
    public:
      TextParser();
      ~TextParser();
  
      indri::api::ParsedDocument* parse( TokenizedDocument* document );

      void handle( TokenizedDocument* document );
      void setHandler( ObjectHandler<indri::api::ParsedDocument>& h );

      void setTags( const std::vector<std::string>& include,
                    const std::vector<std::string>& exclude,
                    const std::vector<std::string>& index,
                    const std::vector<std::string>& metadata, 
                    const std::map<ConflationPattern*, std::string>& conflations );

    protected:
      void writeToken(char* token);
      void writeToken(char *token, int start, int end);
      indri::utility::Buffer _termBuffer;

    private:
      ObjectHandler<indri::api::ParsedDocument>* _handler;
      indri::api::ParsedDocument _document;
    };
  }
}

#endif // INDRI_TEXTPARSER_HPP

