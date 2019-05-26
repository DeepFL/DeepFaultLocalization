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

#ifndef INDRI_PORTERSTEMMERTRANSFORMATION_HPP
#define INDRI_PORTERSTEMMERTRANSFORMATION_HPP

#include "indri/Transformation.hpp"
#include "indri/Porter_Stemmer.hpp"
namespace indri
{
  namespace parse
  {
    
    class PorterStemmerTransformation : public Transformation {
    private:
      ObjectHandler<indri::api::ParsedDocument>* _handler;
      Porter_Stemmer *stemmer;
    public:
      PorterStemmerTransformation();
      ~PorterStemmerTransformation();
      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document );

      void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler );
      void handle( indri::api::ParsedDocument* document );
    };
  }
}

#endif // INDRI_PORTERSTEMMERTRANSFORMATION_HPP
