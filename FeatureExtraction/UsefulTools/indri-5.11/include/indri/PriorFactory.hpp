/*==========================================================================
 * Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// PriorFactory
//
// 4 May 2004 -- tds
//

#ifndef INDRI_PRIORFACTORY_HPP
#define INDRI_PRIORFACTORY_HPP

#include "indri/QuerySpec.hpp"
#include "indri/Parameters.hpp"
namespace indri
{
  namespace query
  {
    // TODO: this class is likely to be deleted soon
    class PriorFactory {
    private:

    
    public:
      PriorFactory();
      PriorFactory( indri::api::Parameters& parameters );
      void initialize();
      indri::lang::PriorNode* create( const std::string& name );
    };
  }
}

#endif // INDRI_PRIORFACTORY_HPP
