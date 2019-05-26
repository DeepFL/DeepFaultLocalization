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
// WriterLockable
//
// 31 January 2005 -- tds
//
// Lockable adaptor for ReadersWritersLocks
//

#ifndef INDRI_WRITERLOCKABLE_HPP
#define INDRI_WRITERLOCKABLE_HPP

#include "indri/ReadersWritersLock.hpp"
namespace indri
{
  namespace thread
  {
    
    class WriterLockable : public Lockable {
    private:
      ReadersWritersLock& _lock;

    public:
      WriterLockable( ReadersWritersLock& lock ) :
        _lock( lock )
      {
      }

      void lock() {
        _lock.lockWrite();
      }

      void unlock() {
        _lock.unlockWrite();
      }
    };
  }
}

#endif // INDRI_WRITERLOCKABLE_HPP



