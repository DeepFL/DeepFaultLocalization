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
// UtilityThread
//
// 31 January 2005 -- tds
//

#ifndef INDRI_UTILITYTHREAD_HPP
#define INDRI_UTILITYTHREAD_HPP

#include "indri/Thread.hpp"
#include "indri/Mutex.hpp"
#include "indri/ConditionVariable.hpp"
namespace indri
{
  namespace thread
  {
    class UtilityThread {
    private:
      Thread* _thread;
      Mutex _lock;
      ConditionVariable _quit;
      volatile bool _runThread;

    public:
      UtilityThread();
      virtual ~UtilityThread() {};

      virtual bool hasWork() = 0;
      virtual UINT64 work() = 0;
      virtual UINT64 initialize() = 0;
      virtual void deinitialize() = 0;

      virtual void run();
      virtual void start();
      virtual void signal();
      virtual void join();
    };
  }
}

#endif // INDRI_UTILITYTHREAD_HPP

