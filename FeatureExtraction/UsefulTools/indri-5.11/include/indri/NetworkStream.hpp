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
// NetworkStream
//
// 23 March 2004 -- tds
//

#ifndef INDRI_NETWORKSTREAM_HPP
#define INDRI_NETWORKSTREAM_HPP

#if defined(__APPLE__) || defined(__SVR4) || defined(WIN32)
#define MSG_NOSIGNAL 0
#endif

#include "lemur/lemur-platform.h"
#include "lemur/lemur-compat.hpp"
#include "indri/indri-platform.h"
#include <string>

namespace indri
{
  namespace net
  {
    
    class NetworkStream {
    private:
      socket_t _socket;

      struct sockaddr_in _getSockaddr( const char* name, unsigned int port ) {
        long address = 0;

#ifndef WIN32
        if( name && isdigit(name[0]) ) {
#else
        if( name && name[0] >= 0 && isdigit(name[0]) ) {
#endif
          address = inet_addr(name);
        } else {
          hostent* host = gethostbyname(name);
      
          if( host && host->h_length ) {
            address = *( (long*) host->h_addr );
          }
        }

        struct sockaddr_in sa;

        sa.sin_addr.s_addr = address;
        sa.sin_port = htons(port);
        sa.sin_family = AF_INET;
        memset(&sa.sin_zero, 0, sizeof sa.sin_zero );

        return sa;
      }

    public:
      NetworkStream() : _socket(-1) {}
      NetworkStream( socket_t s ) : _socket(s) {}
      ~NetworkStream() {
        close();
      }

      std::string peer() {
        struct sockaddr_in sa;
        socklen_t addrlen = sizeof sa;
        std::string result;

        int error = ::getpeername( _socket, (struct sockaddr*) &sa, &addrlen );

        if( !error ) {
          hostent* he = ::gethostbyaddr( (const char*) &sa.sin_addr, sizeof sa.sin_addr.s_addr, AF_INET );

          if( he && he->h_length ) {
            return he->h_name;
          }
        }

        return "unknown";
      }

      bool connect( const std::string& name, unsigned int port ) {
        return connect(name.c_str(), port);
      }

      bool connect( const char* name, unsigned int port ) {
        lemur_compat::initializeNetwork();

        _socket = ::socket( AF_INET, SOCK_STREAM, 0 );
#ifdef __APPLE__
        int set = 1;
        setsockopt(_socket, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif
        struct sockaddr_in sa = _getSockaddr( name, port );
        int result = ::connect( _socket, (const sockaddr*) &sa, sizeof sa );

        if( result ) {
          close();
        }

        return !result;
      }

      void close() {
        lemur_compat::closesocket( _socket );
        _socket = -1;
      }

      int write( const void* buffer, size_t length ) {
//        return ::send( _socket, (const char*) buffer, int(length), 0 );
        if (_socket == -1) return 0;
        int result = ::send( _socket, (const char*) buffer, int(length), MSG_NOSIGNAL );
        if (result < 0) close();
        return result;
      }

      int read( void* buffer, size_t length ) {
        return ::recv( _socket, (char*) buffer, int(length), 0 );
      }

      int blockingRead( void* buffer, size_t length ) {
        size_t bytesRead = 0;
        size_t chunkRead = 0;
        int result;
    
        while( bytesRead < length ) {
          // only try to read 100K at a time.
          chunkRead = lemur_compat::min<size_t>((size_t)1024*100, (length - bytesRead));
          result = read( (char*)buffer + bytesRead, chunkRead );

          if( result <= 0 ) {
            close();
            return int(bytesRead);
          }

          bytesRead += result;
        }

        return int(bytesRead);
      }

      int blockingWrite( const void* buffer, unsigned int length ) {
        size_t bytesWritten = 0;

        while( bytesWritten < (int) length ) {
          // only try to write 100K at a time.
          size_t chunkWrite = std::min((size_t)1024*100, (length - bytesWritten));
          int result = write( (const char*)buffer + bytesWritten, chunkWrite );

          if( result <= 0 ) {
            close();
            return int(bytesWritten);
          }

          bytesWritten += result;
        }

        return int(bytesWritten);
      }

      bool alive() {
        return _socket != -1;
      }
    };
  }
}

#endif // INDRI_NETWORKSTREAM_HPP
