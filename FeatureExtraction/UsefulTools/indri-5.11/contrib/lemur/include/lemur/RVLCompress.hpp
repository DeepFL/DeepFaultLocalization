/*==========================================================================
 * Copyright (c) 2002 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

#ifndef _RVLCOMPRESS_HPP
#define _RVLCOMPRESS_HPP

#include "lemur-platform.h"
namespace lemur 
{
  namespace utility 
  {
    
#define pow2_7  128
#define pow2_14 16384
#define pow2_21 2097152
#define pow2_28 268435456

#define pow2_31 2147483648U

#define RVL_COMPRESS_MASK                      ((1<<7)-1)
#define RVL_COMPRESS_TERMINATE_BIT             (1<<7)
#define RVL_COMPRESS_BYTE( d, in, b )          d[b] = (char) ((in >> 7*b) & ((1<<7)-1))
#define RVL_COMPRESS_TERMINATE( d, in, b )     d[b] = (char) ((in >> 7*b) | (1<<7))
    /// Provide RVL compression of arbitrary data.
    class RVLCompress {
    public:
      ///return number of bytes in result
      static int compress_ints (int *data_ptr, unsigned char *out_ptr, int size);

      /// returns number of ints decompressed
      static int decompress_ints(unsigned char *data_ptr, int *out_ptr, int num_bytes);

      /// size of data when compressed with RVLCompress
      static int compressedSize( int data );
      /// size of data when compressed with RVLCompress
      static int compressedSize( INT64 data );
      /// size of data when compressed with RVLCompress
      static int compressedSize( UINT64 data );

      static UINT64 foldNegatives( INT64 number );
      static INT64 unfoldNegatives( UINT64 number );

      static char* compress_int( char* dest, int data );
      static char* compress_longlong( char* dest, UINT64 data );
      static char* compress_longlong( char* dest, INT64 data );
      static const char* decompress_int( const char* source, int& data );
      static const char* decompress_longlong( const char* source, UINT64& data );
      static const char* decompress_longlong( const char* source, INT64& data );
      static const char* decompress_int_count( const char* source, int* result, int numInts );
      static const char* skip_ints( const char* source, int numInts );

    private:
      static char* _compress_bigger_int( char* dest, int data );
      static char* _compress_bigger_longlong( char* dest, UINT64 data );
    };

    inline UINT64 RVLCompress::foldNegatives( INT64 number ) {
      // fold negative numbers into positive ones, use low bit as negative sign
      UINT64 folded;

      if( number < 0 )
        folded = (2 * -number) - 1;
      else
        folded = 2 * number;

      return folded;
    }

    inline INT64 RVLCompress::unfoldNegatives( UINT64 number ) {
      INT64 unfolded;

      if( number & 1 ) {
        // number is negative
        unfolded = -INT64((number + 1) / 2);
      } else {
        // number is positive
        unfolded = number / 2;
      }

      return unfolded;
    }

    inline int RVLCompress::compressedSize( int data ) {
      return compressedSize( UINT64( data ) );
    }

    inline int RVLCompress::compressedSize( INT64 data ) {
      return compressedSize( foldNegatives( data ) );
    }

    inline const char* RVLCompress::decompress_int( const char* source, int& data ) {
      const unsigned int terminator = (1<<7);
      const unsigned int mask = ((1<<7)-1);

      if( source[0] & terminator ) {
        data = source[0] & mask;
        return source + 1;
      } else if ( source[1] & terminator ) {
        data = (source[0])       | 
          ((source[1]&mask) << 7);
        return source + 2;
      } else if ( source[2] & terminator ) {
        data = (source[0])       | 
          (source[1] << 7)  |
          ((source[2]&mask) << 14);
        return source + 3;
      } else if ( source[3] & terminator ) {
        data = (source[0])       | 
          (source[1] << 7)  |
          (source[2] << 14) |
          ((source[3]&mask) << 21);
        return source + 4;
      } else {
        data = (source[0])       | 
          (source[1] << 7)  |
          (source[2] << 14) |
          (source[3] << 21) |
          ((source[4]&mask) << 28);  
        return source + 5;
      }
    }

    inline const char* RVLCompress::decompress_longlong( const char* source, UINT64& data ) {
      const unsigned int terminator = (1<<7);
      const unsigned int mask = ((1<<7)-1);
      unsigned int i;

      data = 0;

      for( i=0; i<10; i++ ) {
        if( source[i] & terminator ) {
          data |= (UINT64(source[i] & mask) << 7*i);
          break;
        } else {
          data |= (UINT64(source[i]) << 7*i);
        }
      }

      return source + i + 1;
    }

    inline const char* RVLCompress::decompress_int_count( const char* source, int* result, int numInts ) {
      const char* ptr = source;

      for( int i=0; i<numInts; i++ ) {
        ptr = decompress_int( ptr, result[i] );
      }

      return ptr;
    }

    inline const char* RVLCompress::skip_ints( const char* source, int numInts ) {
      while( numInts-- ) {
        while( !(*source & 0x80) )
          source++;
        source++;
      }
      return source;
    }

    inline int RVLCompress::compressedSize( UINT64 data ) {
      if( data < pow2_7 ) {
        return 1;
      } else if ( data < pow2_14 ) {
        return 2;
      } else if ( data < pow2_21 ) {
        return 3;
      } else if ( data < pow2_28 ) {
        return 4;
      } else if ( data < UINT64(1)<<35 ) {
        return 5;
      } else if ( data < UINT64(1)<<42 ) {
        return 6;
      } else if ( data < UINT64(1)<<49 ) {
        return 7;
      } else if ( data < UINT64(1)<<56 ) {
        return 8;
      } else if ( data < UINT64(1)<<63 ) {
        return 9;
      } else {
        return 10;
      }
    }

    inline char* RVLCompress::compress_int( char* dest, int data ) {
      if( data < (1<<7) ) {
        RVL_COMPRESS_TERMINATE( dest, data, 0 );
        return dest + 1;
      } else if( data < (1<<14) ) {
        RVL_COMPRESS_BYTE( dest, data, 0 );
        RVL_COMPRESS_TERMINATE( dest, data, 1 );
        return dest + 2;
      } else {
        return _compress_bigger_int( dest, data );
      }
    }

    inline char* RVLCompress::compress_longlong( char* source, INT64 data ) {
      UINT64 number;
      number = foldNegatives( data );
      return compress_longlong( source, number );
    }

    inline char* RVLCompress::compress_longlong( char* dest, UINT64 data ) {
      if( data < (UINT64(1)<<7) ) {
        RVL_COMPRESS_BYTE( dest, data, 0 );
        RVL_COMPRESS_TERMINATE( dest, data, 0 );
        return dest + 1;
      } else if( data < (UINT64(1)<<14) ) {
        RVL_COMPRESS_BYTE( dest, data, 0 );
        RVL_COMPRESS_TERMINATE( dest, data, 1 );
        return dest + 2;
      } else {
        return _compress_bigger_longlong( dest, data );
      }
    }

    inline const char* RVLCompress::decompress_longlong( const char* source, INT64& data ) {
      UINT64 number;
      source = decompress_longlong( source, number );
      data = unfoldNegatives( number );
      return source;
    }
  }
}

#endif
