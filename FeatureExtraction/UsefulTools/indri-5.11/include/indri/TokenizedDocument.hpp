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
// TokenizedDocument
//
// 15 September 2005 -- mwb
//

#ifndef INDRI_TOKENIZEDDOCUMENT_HPP
#define INDRI_TOKENIZEDDOCUMENT_HPP

#include "indri/greedy_vector"
#include "indri/TagEvent.hpp"
#include "indri/TermExtent.hpp"
#include "indri/MetadataPair.hpp"

namespace indri {
  namespace parse {
    
    struct TokenizedDocument {

      const char* text;
      size_t textLength;

      const char* content;
      size_t contentLength;

      indri::utility::greedy_vector<char*> terms;
      indri::utility::greedy_vector<TagEvent> tags;
      indri::utility::greedy_vector<TermExtent> positions;
      indri::utility::greedy_vector<MetadataPair> metadata;
    };
  }
}

#endif // INDRI_TOKENIZEDDOCUMENT_HPP

