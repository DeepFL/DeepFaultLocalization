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
// RVLCompressStream
//
// 9 February 2004 -- tds
//

#ifndef INDRI_RVLCOMPRESSSTREAM_HPP
#define INDRI_RVLCOMPRESSSTREAM_HPP

#include "lemur/RVLCompress.hpp"
#include "indri/Buffer.hpp"
namespace indri
{
  namespace utility
  {
    
    /*! Provide RVL compression on a stream.
     */
    class RVLCompressStream {
    private:
      indri::utility::Buffer& _buffer;

    public:
      /// Initialize
      /// @param buffer the buffer to use for compressing
      RVLCompressStream( indri::utility::Buffer& buffer ) :
        _buffer(buffer)
      {
      }

      /// @return the buffer
      const char* data() const {
        return _buffer.front();
      }
      /// @return the size of the data in the buffer
      size_t dataSize() const {
        return _buffer.position();
      }

      /// Compress an int into the buffer
      /// @param value the value to compress
      RVLCompressStream& operator<< ( int value ) {
        char* writePosition = _buffer.write(5);
        char* endPosition = lemur::utility::RVLCompress::compress_int( writePosition, value );
        _buffer.unwrite( 5 - (endPosition - writePosition) );
        return *this;
      }

      /// Compress an unsigned int into the buffer
      /// @param value the value to compress
      RVLCompressStream& operator<< ( unsigned int value ) {
        char* writePosition = _buffer.write(5);
        char* endPosition = lemur::utility::RVLCompress::compress_int( writePosition, value );
        _buffer.unwrite( 5 - (endPosition - writePosition) );
        return *this;
      }

      /// Compress an INT64 into the buffer
      /// @param value the value to compress
      RVLCompressStream& operator<< ( INT64 value ) {
        char* writePosition = _buffer.write(10);
        char* endPosition = lemur::utility::RVLCompress::compress_longlong( writePosition, value );
        _buffer.unwrite( 10 - (endPosition - writePosition) );
        return *this;
      }

      /// Compress an UINT64 into the buffer
      /// @param value the value to compress
      RVLCompressStream& operator<< ( UINT64 value ) {
        char* writePosition = _buffer.write(10);
        char* endPosition = lemur::utility::RVLCompress::compress_longlong( writePosition, value );
        _buffer.unwrite( 10 - (endPosition - writePosition) );
        return *this;
      }

      /// Compress a float into the buffer
      /// @param value the value to compress
      RVLCompressStream& operator << ( float value ) {
        // can't compress a float, unfortunately
        memcpy( _buffer.write(sizeof(float)), &value, sizeof value );
        return *this;
      }

      /// Compress a string into the buffer
      RVLCompressStream& operator << ( const char* value ) {
        unsigned int length = (unsigned int) strlen( value );
        (*this) << length;
        memcpy( _buffer.write(length), value, length );
        return *this;
      }
    };
  }
}

#endif // INDRI_RVLCOMPRESSSTREAM_HPP

