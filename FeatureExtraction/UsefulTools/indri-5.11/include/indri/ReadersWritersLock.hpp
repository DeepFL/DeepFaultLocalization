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
// ReadersWritersLock
//
// 14 December 2004 -- tds
//

#include "indri/ScopedMonitor.hpp"
#include "indri/ConditionVariable.hpp"

#ifndef INDRI_READERSWRITERSLOCK_HPP
#define INDRI_READERSWRITERSLOCK_HPP
namespace indri
{
  namespace thread
  {
    
    class ReadersWritersLock {
    private:
      struct wait_queue_entry {
        bool writing;
        bool awakened;
        wait_queue_entry* next;
        ConditionVariable wakeup;
      };

      Mutex _mutex;
      wait_queue_entry* _head;
      wait_queue_entry* _tail;

      int _readers;
      int _writers;

      void _enqueue( wait_queue_entry& entry ) {
        entry.awakened = false;

        // called within the mutex
        if( _tail == 0 ) {
          entry.next = 0;
          _tail = &entry;
          _head = &entry;
        } else {
          entry.next = 0;
          _tail->next = &entry;
          _tail = &entry;
        }
      }

      void _wakeOthers() {
        bool reading = false;

        if( _head ) {
          // wakeup the next thread, no matter what
          _head->awakened = true;
          _head->wakeup.notifyOne();
          reading = !_head->writing;
          _head = _head->next;

          // if the next waiter wants to read, then we should
          // continue to wake threads up until there are no readers left
          if( reading ) {
            while( _head && _head->writing == false ) {
              _head->awakened = true;
              _head->wakeup.notifyOne();
              _head = _head->next;
            }
          }
        }

        // if we took the last thing off the queue, fix up the tail
        if( _head == 0 ) {
          _tail = 0;
        }
      }

    public:
      ReadersWritersLock() :
        _tail(0),
        _head(0),
        _readers(0),
        _writers(0)
      {
      }

      void lockRead() {
        _mutex.lock();

        if( _head != 0 || _writers ) {
          do {
            wait_queue_entry entry;

            entry.writing = false;
            entry.next = 0;

            _enqueue( entry );

            // wait for our time to come
            entry.wakeup.wait( _mutex );
          }
          while( _writers );
        }
        _readers++;
        assert( !_writers );

        _mutex.unlock();
      }
    
      void lockWrite() {
        _mutex.lock();

        if( _head != 0 || _readers || _writers ) {
          do {
            wait_queue_entry entry;

            entry.writing = true;
            entry.next = 0;

            _enqueue( entry );

            // wait for our time to come
            entry.wakeup.wait( _mutex );
          } 
          while( _readers || _writers );
        }

        assert( _writers == 0 );
        _writers++;
        _mutex.unlock();

        assert( !_readers && _writers == 1 );
      }

      void unlockWrite() {
        assert( _writers );
        assert( !_readers );

        _mutex.lock();
        _writers = 0;
        _wakeOthers();
        _mutex.unlock();
      }

      void unlockRead() {
        assert( _readers );
        assert( !_writers );

        _mutex.lock();
        _readers--;

        if( _readers == 0 )
          _wakeOthers();

        _mutex.unlock();
      }

      void yieldWrite() {
        assert( !_readers && _writers == 1 );

        if( _head ) {
          unlockWrite();
          lockWrite();
        }

        assert( !_readers && _writers == 1 );
      }

      void yieldRead() {
        assert( _readers && _writers == 0 );

        if( _head ) {
          unlockRead();
          lockRead();
        }

        assert( _readers && _writers == 0 );
      }

    };
  }
}

#endif // INDRI_READERSWRITERSLOCK_HPP

