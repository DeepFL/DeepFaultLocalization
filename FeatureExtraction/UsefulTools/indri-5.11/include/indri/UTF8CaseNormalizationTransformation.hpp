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
// UTF8CaseNormalizationTransformation
//
// 16 September 2005 -- mwb
//

#ifndef INDRI_UTF8CASENORMALIZATIONTRANSFORMATION_HPP
#define INDRI_UTF8CASENORMALIZATIONTRANSFORMATION_HPP

#include "indri/indri-platform.h"
#include "indri/Transformation.hpp"
#include "indri/UTF8Transcoder.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/HashTable.hpp"
#include <vector>
#include <map>

namespace indri {
  namespace parse {
    
    class UTF8CaseNormalizationTransformation : public Transformation {

    private:
      ObjectHandler<indri::api::ParsedDocument>* _handler;
      UTF8Transcoder _transcoder;
      std::vector<char*> _buffers_allocated;

      indri::utility::HashTable<UINT64,UINT64> _downcase;
      void _initHT();

    public:
      UTF8CaseNormalizationTransformation();
      ~UTF8CaseNormalizationTransformation();

      void handle( indri::api::ParsedDocument* document );
      void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler );

      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document );

    };
  }
}

#endif // INDRI_UTF8CASENORMALIZATIONTRANSFORMATION_HPP

