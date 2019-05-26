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
// ScopedLock
//
// 15 November 2004 -- tds
//

#ifndef INDRI_SCOPEDLOCK_HPP
#define INDRI_SCOPEDLOCK_HPP
namespace indri
{
  namespace thread
  {
    class ScopedLock {
    private:
      Lockable* _lockable;

    public:
      ScopedLock( Lockable& lockable ) {
        _lockable = &lockable;
        _lockable->lock();
      }

      ScopedLock( Lockable* lockable ) {
        _lockable = lockable;

        if( _lockable )
          _lockable->lock();
      }

      ~ScopedLock() {
        if( _lockable )
          _lockable->unlock();
      }

      void unlock() {
        if( _lockable )
          _lockable->unlock();
        _lockable = 0;
      }
    };
  }
}

#endif // INDRI_SCOPEDLOCK_HPP

