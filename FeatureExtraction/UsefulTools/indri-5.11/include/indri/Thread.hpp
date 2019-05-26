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

#ifndef INDRI_THREAD_HPP
#define INDRI_THREAD_HPP

#include "indri/indri-platform.h"

#ifndef WIN32
#include <pthread.h>
#endif
namespace indri
{
  namespace thread
  {
    
    class Thread {
    private:
#ifdef WIN32
      unsigned int _id;
      uintptr_t _thread;
#else
      pthread_t _thread;
#endif
      void (*_function)( void* data );
      void* _data;

    public:
      Thread( void (*function)(void*), void* data );
      void execute();
      void join();

      static int id();
      static void sleep( int milliseconds );
      static void yield();
    };
  }
}

#endif // INDRI_THREAD_HPP

