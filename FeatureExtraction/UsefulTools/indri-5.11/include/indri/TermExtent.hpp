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
// TermExtent
//
// 12 May 2004 -- tds
//

#ifndef INDRI_TERMEXTENT_HPP
#define INDRI_TERMEXTENT_HPP
namespace indri
{
  namespace parse
  {
    struct TermExtent {
      int begin;
      int end;
    };
  }
}

#endif // INDRI_TERMEXTENT_HPP

