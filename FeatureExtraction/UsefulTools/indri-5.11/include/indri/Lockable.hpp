/*==========================================================================
 * Copyright (c) 2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

//
// Lockable
//
// 31 January 2005 -- tds
//

#ifndef INDRI_LOCKABLE_HPP
#define INDRI_LOCKABLE_HPP
namespace indri
{
  namespace thread
  {
     
    class Lockable {
    public:
      virtual ~Lockable() {};

      virtual void lock() = 0;
      virtual void unlock() = 0;
    };
  }
}

#endif // INDRI_LOCKABLE_HPP

