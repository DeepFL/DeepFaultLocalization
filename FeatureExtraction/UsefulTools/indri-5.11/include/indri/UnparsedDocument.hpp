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
// UnparsedDocument
//
// 12 May 2004 -- tds
//

#ifndef INDRI_UNPARSEDDOCUMENT_HPP
#define INDRI_UNPARSEDDOCUMENT_HPP

#include "indri/MetadataPair.hpp"
#include "indri/greedy_vector"
namespace indri
{
  namespace parse
  {
    
    struct UnparsedDocument {
      const char* text;
      size_t textLength;
      const char* content;
      size_t contentLength;

      indri::utility::greedy_vector<MetadataPair> metadata;
    };
  }
}

#endif // INDRI_UNPARSEDDOCUMENT_HPP
