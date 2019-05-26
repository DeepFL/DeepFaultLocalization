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

#ifndef INDRI_QUERYSTOPPER_HPP
#define INDRI_QUERYSTOPPER_HPP
#include <string>
namespace indri
{
  namespace query
  {
    class QueryStopper {
    public:
      static std::string transform(std::string query);
    };
  }
}

#endif
