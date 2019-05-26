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
// Tokenizer
// 
// 15 September 2005 -- mwb
//

#ifndef INDRI_TOKENIZER_HPP
#define INDRI_TOKENIZER_HPP

#include "indri/ObjectHandler.hpp"
#include "indri/UnparsedDocument.hpp"
#include "indri/TokenizedDocument.hpp"
// #include <map>
// #include <vector>

namespace indri {
  namespace parse {
      
    class Tokenizer : public ObjectHandler<UnparsedDocument> {
    public:
      Tokenizer() {}
      virtual ~Tokenizer() {}

      virtual TokenizedDocument* tokenize( UnparsedDocument* document ) = 0;
      virtual void handle( UnparsedDocument* document ) = 0;
      virtual void setHandler( ObjectHandler<TokenizedDocument>& handler ) = 0;
    };
  }
}

#endif // INDRI_TOKENIZER_HPP


