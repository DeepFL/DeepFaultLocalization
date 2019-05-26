/*==========================================================================
 * Copyright (c) 2003-2005 University of Massachusetts.  All Rights Reserved.
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

#ifndef INDRI_TEXTTOKENIZER_HPP
#define INDRI_TEXTTOKENIZER_HPP

#include <stdio.h>
#include <string>
#include <map>

#include "indri/IndriTokenizer.hpp"
#include "indri/Buffer.hpp"
#include "indri/TagEvent.hpp"
#include "indri/UnparsedDocument.hpp"
#include "indri/TokenizedDocument.hpp"
#include "indri/UTF8Transcoder.hpp"

namespace indri {
  namespace parse {
    
    class TextTokenizer : public Tokenizer {

    public:
      TextTokenizer( bool tokenize_markup = true, bool tokenize_entire_words = true ) : _handler(0) {

        _tokenize_markup = tokenize_markup;
        _tokenize_entire_words = tokenize_entire_words;
      }

      ~TextTokenizer() {}
  
      TokenizedDocument* tokenize( UnparsedDocument* document );

      void handle( UnparsedDocument* document );
      void setHandler( ObjectHandler<TokenizedDocument>& h );

    protected:
      void processASCIIToken();
      void processUTF8Token();
      void processTag();

      indri::utility::Buffer _termBuffer;
      UTF8Transcoder _transcoder;

      bool _tokenize_markup;
      bool _tokenize_entire_words;

    private:
      ObjectHandler<TokenizedDocument>* _handler;
      TokenizedDocument _document;

      void writeToken( char* token, int token_len, int extent_begin, 
                       int extent_end );
    };
  }
}

#endif // INDRI_TEXTTOKENIZER_HPP

