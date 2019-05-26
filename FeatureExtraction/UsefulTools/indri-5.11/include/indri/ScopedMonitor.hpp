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
// ScopedMonitor
//
// 15 November 2004 -- tds
//

#ifndef INDRI_SCOPEDMONITOR_HPP
#define INDRI_SCOPEDMONITOR_HPP

#include "indri/Mutex.hpp"
#include "indri/ConditionVariable.hpp"
namespace indri
{
  namespace thread
  {
    
    class ScopedMonitor {
    private:
      Mutex& _mutex;
      ConditionVariable& _condition;

    public:
      ScopedMonitor( Mutex& mutex, ConditionVariable& condition ) :
        _mutex(mutex),
        _condition(condition)
      {
        _mutex.lock();
      }

      ~ScopedMonitor() {
        _mutex.unlock();
      }

      void wait() {
        _condition.wait( _mutex );
      }

      void notifyOne() {
        _condition.notifyOne();
      }

      void notifyAll() {
        _condition.notifyAll();
      }
    };
  }
}

#endif // INDRI_SCOPEDMONITOR_HPP

