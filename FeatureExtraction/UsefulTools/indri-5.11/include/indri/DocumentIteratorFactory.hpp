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
// DocumentIteratorFactory
//
// 5 August 2004 -- tds
//

#ifndef INDRI_DOCUMENTITERATORFACTORY_HPP
#define INDRI_DOCUMENTITERATORFACTORY_HPP

#include "indri/DocumentIterator.hpp"
#include <map>
#include <string>
namespace indri
{
  namespace parse
  {
    
    class DocumentIteratorFactory {
    public:
      ~DocumentIteratorFactory();

      static DocumentIterator* get( const std::string& type );
      static DocumentIterator* get( const std::string& type, const char* startDocTag, const char* endDocTag, const char* startMetadataTag );

      static std::string preferredName( const std::string& type );
    };
  }
}

#endif // INDRI_DOCUMENTITERATORFACTORY_HPP

