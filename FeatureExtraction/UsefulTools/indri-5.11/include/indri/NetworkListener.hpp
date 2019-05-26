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
// NetworkListener
//
// 23 March 2004 -- tds
//

#ifndef INDRI_NETWORKLISTENER_HPP
#define INDRI_NETWORKLISTENER_HPP

#include "lemur/lemur-compat.hpp"
#include "indri/NetworkStream.hpp"
namespace indri
{
  /*! Network communication and support classes. */
  namespace net
  {
    
    class NetworkListener {
      socket_t _socket;

    public:
      NetworkListener() : _socket(-1) {}
      ~NetworkListener() {
        close();
      }

      bool listen( unsigned int port ) {
        int result;
 
        lemur_compat::initializeNetwork();
        _socket = ::socket( AF_INET, SOCK_STREAM, 0 );

        sockaddr_in sa;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port = htons(port);
        sa.sin_family = AF_INET;
        memset( &sa.sin_zero, 0, sizeof sa.sin_zero );

        result = ::bind( _socket, (const sockaddr*) &sa, sizeof sa );
        if( result ) {
          close();
          LEMUR_THROW( LEMUR_IO_ERROR, "Wasn't able to bind port " + i64_to_string(port) );
        }
  
        result = ::listen( _socket, 8 );

        if( result ) {
          close();
          LEMUR_THROW( LEMUR_IO_ERROR, "Wasn't able to listen on port " + i64_to_string(port) );
        }

        return true;
      }

      NetworkStream* accept() {
        assert( _socket != -1 );
        socket_t s = ::accept( _socket, 0, 0 );
        return new NetworkStream(s);
      }

      void close() {
        lemur_compat::closesocket(_socket);
        _socket = -1;
      }
    };
  }
}

#endif // INDRI_NETWORKLISTENER_HPP
