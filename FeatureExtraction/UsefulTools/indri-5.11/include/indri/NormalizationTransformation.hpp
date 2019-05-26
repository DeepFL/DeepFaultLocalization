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
// CaseNormalizationTransformation
//
// 13 August 2004 -- tds
//

#ifndef INDRI_CASENORMALIZATIONTRANSFORMATION_HPP
#define INDRI_CASENORMALIZATIONTRANSFORMATION_HPP

#include "indri/Transformation.hpp"
#include "indri/Parameters.hpp"
#include "lemur/string-set.h"
namespace indri
{
  namespace parse
  {
    
    class NormalizationTransformation : public Transformation {
    private:
      ObjectHandler<indri::api::ParsedDocument>* _handler;
      String_set* _acronyms;

    public:
      NormalizationTransformation( indri::api::Parameters* acronymList = 0 );
      ~NormalizationTransformation();

      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document );
  
      void handle( indri::api::ParsedDocument* document );
      void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler );
    };
  }
}

#endif // INDRI_CASENORMALIZATIONTRANSFORMATION_HPP



