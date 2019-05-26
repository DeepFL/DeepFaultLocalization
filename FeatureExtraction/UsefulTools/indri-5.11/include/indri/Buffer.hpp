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
// Buffer
//
// 14 May 2004 -- tds
//

#ifndef INDRI_BUFFER_HPP
#define INDRI_BUFFER_HPP

#include <assert.h>
#include "lemur/lemur-compat.hpp"
namespace indri
{
  /*! \brief Utility classes for indri components. */
  namespace utility
  {
    
    class Buffer {
    private:
      char* _buffer;
      size_t _size;
      size_t _position;

    public:
      Buffer( size_t length ) :
        _buffer( (char*) malloc( length ) ),
        _size( length ),
        _position(0)
      {
      }

      Buffer() :
        _buffer(0),
        _size(0),
        _position(0)
      {
      }

      ~Buffer() {
        free( _buffer );
      }

      inline size_t size() const {
        return _size;
      }

      inline size_t position() const {
        return _position;
      }

      inline void clear() {
        _position = 0;
      }

      inline char* front() {
        return _buffer;
      }

      inline char* write( size_t length ) {
        if( _position + length > _size )
          grow( _position + length );
        char* spot = _buffer + _position;
        _position += length;
        return spot;
      }

      inline void unwrite( size_t length ) {
        assert( length >= 0 );
        assert( length <= _position );
        _position -= length;
      }
  
      void grow( size_t newSize ) {
        if( newSize > _size ) {
          if( newSize < 1024*1024 ) {
            // find next larger power of two, up to one megabyte
            size_t powSize;
            for( powSize = 64; powSize < newSize; powSize *= 2 )
              ;
            newSize = powSize;
          } else {
            // round to nearest megabyte
            newSize = (newSize + 1024*1024) & ~(1024*1024-1);
          }

          char* newBuffer = (char*) malloc( newSize );
          memcpy( newBuffer, _buffer, _position );
          free( _buffer );
          _buffer = newBuffer;
          _size = newSize;
        }
      }

      void grow() {
        if( _size == 0 )
          grow(64);
        else
          grow(_size*2);
      }

      size_t remaining() {
        return size() - position();
      }

      void remove( size_t start ) {
        memmove( _buffer, _buffer + start, _position - start );
        _position -= start;
      }

      void detach() {
        _size = 0;
        _buffer = 0;
      }
    };
  }
}

#endif // INDRI_BUFFER_HPP
