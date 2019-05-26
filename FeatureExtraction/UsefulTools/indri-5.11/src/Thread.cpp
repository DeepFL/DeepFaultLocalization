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
// Thread
//
// 15 November 2004 -- tds
//

#include "indri/Thread.hpp"

#ifdef WIN32
#include <process.h>
#endif

void* pthread_start( void* parameter ) {
  indri::thread::Thread* t = (indri::thread::Thread*) parameter;
  t->execute();
  return 0;
}

#ifdef WIN32
unsigned int __stdcall win32_start( void* parameter ) {
  indri::thread::Thread* t = (indri::thread::Thread*) parameter;
  t->execute();
  _endthreadex(0);
  return 0;
}
#endif

indri::thread::Thread::Thread( void (*function)(void*), void* data ) {
  _function = function;
  _data = data;

#ifdef WIN32
  _thread = _beginthreadex( 0, 0, win32_start, this, 0, &_id );
#else
  pthread_create( &_thread, 0, pthread_start, this );
#endif
}

void indri::thread::Thread::execute() {
  _function( _data );
}

void indri::thread::Thread::join() {
#ifdef WIN32
  WaitForSingleObject( (HANDLE) _thread, INFINITE );
  CloseHandle( (HANDLE) _thread );
#else
  pthread_join( _thread, 0 );
#endif
}

int indri::thread::Thread::id() {
#ifdef WIN32
  return ::GetCurrentThreadId( );
#else
  return 0;
#endif
}

void indri::thread::Thread::sleep( int milliseconds ) {
#ifdef WIN32
  ::Sleep( milliseconds );
#else
  ::usleep( 1000 * milliseconds );
#endif
}

void indri::thread::Thread::yield() {
#ifdef WIN32
  ::SwitchToThread();
#else
  indri::thread::Thread::sleep(0);
#endif
}

