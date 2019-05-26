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
// NormalizationTransformation
//
// 13 August 2004 -- tds
//

#include "indri/NormalizationTransformation.hpp"
#include "indri/Parameters.hpp"
#include "lemur/string-set.h"

indri::parse::NormalizationTransformation::NormalizationTransformation( indri::api::Parameters* acronymList )
  :
  _handler(0),
  _acronyms(0)
{
  if( acronymList && acronymList->exists("word") ) {
    indri::api::Parameters words = (*acronymList)["word"];
    _acronyms = string_set_create();
    
    for( size_t i=0; i<words.size(); i++ ) {
      std::string acronym;
      acronym = (std::string) words[i];
      string_set_add( acronym.c_str(), _acronyms );
    }
  }
}

indri::parse::NormalizationTransformation::~NormalizationTransformation() {
  if(_acronyms)
    string_set_delete(_acronyms);
}

indri::api::ParsedDocument* indri::parse::NormalizationTransformation::transform( indri::api::ParsedDocument* document ) {
  indri::utility::greedy_vector<char*>& terms = document->terms;
  unsigned int i, j, k;

  for( i=0; i<terms.size(); i++ ) {
    char* term = terms[i];

    // another step may have removed this term
    if( !term )
      continue;

    bool process = false;

    // look for uppercase letters, periods and apostrophes
    for( j=0; term[j]; j++ ) {
      char c = term[j];

      if( (c >= 'A' && c <= 'Z') || c == '.' || c == '\'' ) {
        process = true;
        break;
      }
    }

    // word is fine, no further processing is needed
    if( !process )
      continue;

    // remove periods and apostrophes
    for( j=0, k=0; term[j]; j++ ) {
      if( term[j] != '.' && term[j] != '\'' ) {
        term[k] = term[j];      
        k++;
      }
    }
    term[k] = 0;

    // if this is an acronym, skip it, otherwise, case normalize
    if( !_acronyms || !string_set_lookup( term, _acronyms ) ) {
      char* letter = term;
      
      for( ; *letter; letter++ )
        if( *letter >= 'A' && *letter <= 'Z' )
          *letter += 'a' - 'A';
    }
  }

  return document;
}

void indri::parse::NormalizationTransformation::handle( indri::api::ParsedDocument* document ) {
  _handler->handle( transform( document ) );
}

void indri::parse::NormalizationTransformation::setHandler( ObjectHandler<indri::api::ParsedDocument>& handler ) {
  _handler = &handler;
}


