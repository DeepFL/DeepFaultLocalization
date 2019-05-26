/*==========================================================================
  Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.

  Use of the Lemur Toolkit for Language Modeling and Information Retrieval
  is subject to the terms of the software license set forth in the LICENSE
  file included with this software, and also available at
  http://www.lemurproject.org/license.html 
  ==========================================================================
*/

//
// lemur-compat
//
// 24 March 2004 -- tds
//

#include "lemur-compat.hpp"

#ifdef WIN32
#include <windows.h>
#endif

namespace lemur_compat {
  void initializeNetwork() {
#ifdef WIN32
    WSADATA data;
    memset( &data, 0, sizeof data );
    ::WSAStartup( MAKEWORD(2,0), &data );
#endif
  }

  void closesocket( socket_t s ) {
#ifdef WIN32
    ::closesocket(s);
#else
    ::close(s);
#endif
  }
}


