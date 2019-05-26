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
// KrovetzStemmerTransformation.hpp
//
// 13 May 2004 -- tds
//

#ifndef INDRI_KROVETZSTEMMERTRANSFORMATION_HPP
#define INDRI_KROVETZSTEMMERTRANSFORMATION_HPP

#include "indri/Transformation.hpp"
#include "indri/Parameters.hpp"
#include "indri/KrovetzStemmer.hpp"
namespace indri
{
  namespace parse
  {

    class KrovetzStemmerTransformation : public Transformation {
    private:
      KrovetzStemmer *stemmer;
      char* _stemBuffer;
      size_t _stemBufferSize;
      ObjectHandler<indri::api::ParsedDocument>* _handler;

      char* _getBuffer( size_t length );
      const char* _getBufferEnd() const;
      indri::api::ParsedDocument* _restart( indri::api::ParsedDocument* document, size_t lastIndex, char* endOfStemming );
      indri::api::ParsedDocument* _processTerms( indri::api::ParsedDocument* document, size_t start, char* stem, const char* end );
      char* _growBuffer( size_t length, char* oldEnd );

    public:
      KrovetzStemmerTransformation( indri::api::Parameters& parameters );
      ~KrovetzStemmerTransformation();

      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document );

      void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler );
      void handle( indri::api::ParsedDocument* document ); 
      static bool _indri_kstem_loaded;
    };
  }
}

#endif // INDRI_KROVETZSTEMMERTRANSFORMATION_HPP

