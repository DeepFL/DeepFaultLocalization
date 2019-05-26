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
// ReaderLockable
//
// 31 January 2005 -- tds
//
// Lockable adaptor for ReadersWritersLocks
//

#ifndef INDRI_READERLOCKABLE_HPP
#define INDRI_READERLOCKABLE_HPP

#include "indri/ReadersWritersLock.hpp"
#include "indri/Thread.hpp"
namespace indri
{
  namespace thread
  {
    
    class ReaderLockable : public Lockable {
    private:
      ReadersWritersLock& _lock;

    public:
      ReaderLockable( ReadersWritersLock& lock ) :
        _lock( lock )
      {
      }

      void lock() {
        _lock.lockRead();
      }

      void unlock() {
        _lock.unlockRead();
      }
    };
  }
}

#endif // INDRI_READERLOCKABLE_HPP

