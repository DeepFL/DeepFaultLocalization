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
// TokenizerFactory
//
// 15 September 2005 -- mwb
//

#ifndef INDRI_TOKENIZERFACTORY_HPP
#define INDRI_TOKENIZERFACTORY_HPP

#include <string>
#include <map>

#include "indri/IndriTokenizer.hpp"

namespace indri {
  namespace parse {
    
    class TokenizerFactory {
    public:
      ~TokenizerFactory();

      static std::string preferredName( const std::string& name );
      static indri::parse::Tokenizer* get( const std::string& name );

    };
  }
}

#endif // INDRI_TOKENIZERFACTORY_HPP

