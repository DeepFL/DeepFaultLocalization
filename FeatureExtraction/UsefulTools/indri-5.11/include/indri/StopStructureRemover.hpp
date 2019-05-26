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

#ifndef INDRI_STOPSTRUCTUREREMOVER_HPP
#define INDRI_STOPSTRUCTUREREMOVER_HPP
#include <string>
/*
  Based on: 
  Samuel Huston and W. Bruce Croft. Evaluating verbose query processing
  techniques. In Proc. of SIGIR, pages 291-298, 2010.
 */
namespace indri
{
  namespace query
  {
    class StopStructureRemover {
    public:
      static std::string transform(std::string query);    };
  }
}

#endif
