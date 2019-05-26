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
// SequentialReadBuffer
//
// 10 December 2004 -- tds
//

#ifndef INDRI_SEQUENTIALREADBUFFER_HPP
#define INDRI_SEQUENTIALREADBUFFER_HPP

#include "indri/indri-platform.h"
#include "indri/File.hpp"
#include "indri/InternalFileBuffer.hpp"
#include "lemur/Exception.hpp"

namespace indri
{
  namespace file
  {
    
    class SequentialReadBuffer {
    private:
      File& _file;
      UINT64 _position;
      InternalFileBuffer _current;

    public:
      SequentialReadBuffer( File& file ) :
        _file(file),
        _position(0),
        _current( 1024*1024 )
      {
      }

      SequentialReadBuffer( File& file, size_t length ) :
        _file(file),
        _position(0),
        _current( length )
      {
      }

      void cache( UINT64 position, size_t length ) {
        _current.buffer.clear();
        _current.filePosition = position;
        _current.buffer.grow( length );

        size_t actual = _file.read( _current.buffer.write( length ), _position, length );
        _current.buffer.unwrite( length - actual );
      }

      size_t read( void* buffer, UINT64 position, size_t length ) {
        if( position >= _current.filePosition && (position + length) <= _current.filePosition + _current.buffer.position() ) {
          memcpy( buffer, _current.buffer.front() + position - _current.filePosition, length );
          return length;
        } else {
          seek(position);
          return read( buffer, length );
        }
      }

      size_t read( void* buffer, size_t length ) {
        memcpy( buffer, read( length ), length );
        return length;
      }

      const void* peek( size_t length ) {
        const void* result = 0;
      
        if( _position < _current.filePosition || (_position + length) > _current.filePosition + _current.buffer.position() ) {
          // data isn't in the current buffer
          // this isn't necessarily the most efficient way to do this, but it should work
          cache( _position, std::max( length, _current.buffer.size() ) );
          // if we get a short read
          if ( _current.buffer.position() + _current.filePosition < _position + length ) {
            LEMUR_THROW(LEMUR_IO_ERROR, "read fewer bytes than expected.");
          }
          
        }

        result = _current.buffer.front() + ( _position - _current.filePosition );
        assert( _current.filePosition <= _position );
        assert( _current.buffer.position() + _current.filePosition >= _position + length );
        return result;
      }

      const void* read( size_t length ) {
        const void* result = peek( length );
        _position += length;
        return result;
      }

      void seek( UINT64 position ) {
        _position = position;
      }

      void clear() {
        _position = 0;
        _current.filePosition = 0;
        _current.buffer.clear();
      }

      UINT64 position() {
        return _position;
      }
    };
  
  }
}

#endif // INDRI_SEQUENTIALREADBUFFER_HPP

