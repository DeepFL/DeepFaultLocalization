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
// ParsedDocument
//
// 12 May 2004 -- tds
//

#ifndef INDRI_PARSEDDOCUMENT_HPP
#define INDRI_PARSEDDOCUMENT_HPP

#include "indri/greedy_vector"
#include "indri/TagExtent.hpp"
#include "indri/TermExtent.hpp"
#include "indri/MetadataPair.hpp"
#include <string>
namespace indri
{
  namespace api 
  {
    
    struct ParsedDocument {  
      const char* text;
      size_t textLength;

      const char* content;
      size_t contentLength;

      std::string getContent() {
        return std::string (content, contentLength);
      }
      
      indri::utility::greedy_vector<char*> terms;
      indri::utility::greedy_vector<indri::parse::TagExtent *> tags;
      indri::utility::greedy_vector<indri::parse::TermExtent> positions;
      indri::utility::greedy_vector<indri::parse::MetadataPair> metadata;
    };
  }
}

#endif // INDRI_PARSEDDOCUMENT_HPP

