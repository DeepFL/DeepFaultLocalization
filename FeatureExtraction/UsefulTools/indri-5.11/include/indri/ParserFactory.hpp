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
// ParserFactory
//
// 6 August 2004 -- tds
//

#ifndef INDRI_PARSERFACTORY_HPP
#define INDRI_PARSERFACTORY_HPP

#include <string>
#include <map>
#include <vector>
#include "indri/IndriParser.hpp"
#include "indri/ConflationPattern.hpp"
namespace indri
{
  namespace parse
  {
    
    class ParserFactory {
    public:
      ~ParserFactory();

      static std::string preferredName( const std::string& name );
      static indri::parse::Parser* get( const std::string& name );
      static indri::parse::Parser* get( const std::string& name,
                                        const std::vector<std::string>& includeTags,
                                        const std::vector<std::string>& excludeTags,
                                        const std::vector<std::string>& indexTags,
                                        const std::vector<std::string>& metadataTags,
                                        const std::map<ConflationPattern*, std::string>& conflations );
    };
  }
}

#endif // INDRI_PARSERFACTORY_HPP

