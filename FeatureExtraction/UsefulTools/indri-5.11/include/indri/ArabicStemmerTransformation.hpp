/*==========================================================================
 * Copyright (c) 2012 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/
//
// ArabicStemmerTransformation
//

#ifndef INDRI_ARABICSTEMMERTRANSFORMATION_HPP
#define INDRI_ARABICSTEMMERTRANSFORMATION_HPP

#include "indri/Transformation.hpp"
#include "indri/Parameters.hpp"
#include "indri/Arabic_Stemmer_utf8.hpp"
namespace indri
{
  namespace parse
  {
    
    class ArabicStemmerTransformation : public Transformation {
    private:
      char* _stemBuffer;
      size_t _stemBufferSize;
      ObjectHandler<indri::api::ParsedDocument>* _handler;
      Arabic_Stemmer_utf8 *stemmer;

      char* _getBuffer( size_t length );
      const char* _getBufferEnd() const;
      indri::api::ParsedDocument* _restart( indri::api::ParsedDocument* document, size_t lastIndex, char* endOfStemming );
      indri::api::ParsedDocument* _processTerms( indri::api::ParsedDocument* document, size_t start, char* stem, const char* end );
      char* _growBuffer( size_t length, char* oldEnd );

    public:
      ArabicStemmerTransformation(indri::api::Parameters& parameters);
      ~ArabicStemmerTransformation();
      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document );

      void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler );
      void handle( indri::api::ParsedDocument* document );
    };
  }
}

#endif
