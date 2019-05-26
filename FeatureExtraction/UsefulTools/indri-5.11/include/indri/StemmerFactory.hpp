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
// StemmerFactory
//
// 5 August 2004 -- tds
//

#ifndef INDRI_STEMMERFACTORY_HPP
#define INDRI_STEMMERFACTORY_HPP

#include "indri/Transformation.hpp"
#include <string>
#include "indri/Parameters.hpp"
namespace indri
{
  namespace parse
  {
    
    class StemmerFactory {
    public:
      static Transformation* get( const std::string& stemmerName, indri::api::Parameters& stemmerParams );
      static std::string preferredName( const std::string& stemmerName );
    };
  }
}

#endif // INDRI_STEMMERFACTORY_HPP
