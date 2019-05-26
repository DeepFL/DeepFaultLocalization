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
// PorterStemmerTransformation
//
// 13 May 2004 -- tds
//
// Adapted from the Lemur PorterStemmer class
//

#include "indri/PorterStemmerTransformation.hpp"

indri::parse::PorterStemmerTransformation::PorterStemmerTransformation() {
  stemmer = new Porter_Stemmer();
}

indri::parse::PorterStemmerTransformation::~PorterStemmerTransformation() {
  delete stemmer;
}

indri::api::ParsedDocument* indri::parse::PorterStemmerTransformation::transform( indri::api::ParsedDocument* document ) {
  indri::utility::greedy_vector<char*>& terms = document->terms;

  for( size_t i=0; i<terms.size(); i++ ) {
    char* term = terms[i];
    
    if( !term )
      continue;

    int length = strlen( term );
    int newLength = stemmer->porter_stem( term, 0, int(length)-1 );
    assert( newLength <= length );
    term[newLength+1] = 0;
  }

  return document;
}

void indri::parse::PorterStemmerTransformation::setHandler( ObjectHandler<indri::api::ParsedDocument>& handler ) {
  _handler = &handler;
}

void indri::parse::PorterStemmerTransformation::handle( indri::api::ParsedDocument* document ) {
  _handler->handle( transform( document ) );
}
