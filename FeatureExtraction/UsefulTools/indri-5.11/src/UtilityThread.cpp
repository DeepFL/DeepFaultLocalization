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

#include "indri/UtilityThread.hpp"
#include "indri/ScopedLock.hpp"
#include "lemur/Exception.hpp"

//
// utility_thread_run
//

void utility_thread_run( void* pointer ) {
  ( (indri::thread::UtilityThread*) pointer )->run();
}

//
// UtilityThread
//

indri::thread::UtilityThread::UtilityThread() :
  _thread(0)
{
}

//
// run
//

void indri::thread::UtilityThread::run() {
  indri::thread::ScopedLock lock( _lock );

  try {
    UINT64 waitTime = initialize();

    while( _runThread ) {
      bool noTimeout = _quit.wait( _lock, waitTime );

      if( noTimeout )
        continue; // interrupted

      waitTime = work();
    }

    while( hasWork() )
      work();

    deinitialize();
  } catch( lemur::api::Exception& e ) {
    std::cout << "UtilityThread exiting from exception " << e.what() << std::endl;
  }

}

//
// start
//

void indri::thread::UtilityThread::start() {
  if( _thread )
    return;

  _runThread = true;
  _thread = new Thread( utility_thread_run, this ); 
}

//
// signal
//

void indri::thread::UtilityThread::signal() {
  _runThread = false;
  _quit.notifyAll();
}

//
// join
//

void indri::thread::UtilityThread::join() {
  signal();

  if( _thread ) {
    _thread->join();
    delete _thread;
    _thread = 0;
  }
}

