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
// TermBitmap
//
// 12 January 2005 -- tds
//


#ifndef INDRI_TERMBITMAP_HPP
#define INDRI_TERMBITMAP_HPP

#include "indri/Buffer.hpp"
#include "indri/delete_range.hpp"

namespace indri {
  namespace index {
/*!
  TermBitmap is used to convert termIDs when many DiskIndexes are merged together.
  The add() function has very strict preconditions; both from and to must increase
  on every call, and from must always be less than to.

  This data is stored in 32-byte bitmap chunks with the following form:
  4 bytes - fromBase
  4 bytes - toBase
  24 bytes - bitmap

  Each bit set in the bitmap region corresponds to a (from, to) pair.  
  
  Suppose the beginning of the bitmap looks like this:
  000100100110000....
  This could be represented by the following pairs:
  (1, 4)
  (2, 7)
  (3, 10)
  (4, 11)
  For instance, the (2, 7) pair says that the second non-zero bit is
  at index 7.  This (2, 7) pair is translated to mean that
  (fromBase + 2, toBase + 7) is a pair stored by some explicit add() call.

  To save on heap overhead, we manage blocks of 64K each in Buffer objects, 
  which are stored in the vector called _maps.

  The TermBitmap is used because, in the ideal case, it is much more space
  efficient than the simpler approach of using an array mapping.  In an 
  array, we'd need 32 bits for each (from, to) pair.  In the case where
  the (from, to) pairs are optimally dense [e.g. (1,1), (2,2), (3,3) ... ],
  the TermBitmap uses 1.33 bits per pair.
*/

    class TermBitmap {
    private:
      std::vector<indri::utility::Buffer*> _maps;
      int _fromBase;
      int _toBase;
      int _lastFrom;
      char* _current;

      void _addBufferIfNecessary() {
        if( !_maps.size() || _maps.back()->position() == _maps.back()->size() )
          _maps.push_back( new indri::utility::Buffer( 64*1024 ) );
      }

      indri::utility::Buffer* _findBuffer( int from ) {
        assert( from >= 0 );
        
        int left = 0;
        int right = (int)_maps.size()-1;

        if( _maps.size() == 0 )
          return 0;

        while( right - left > 1 ) {
          int middle = left + (right - left) / 2;
          indri::utility::Buffer* mid = _maps[ middle ];

          if( from < *(INT32*) mid->front() ) {
            right = middle;
          } else {
            left = middle;
          }
        }

        if( right == left )
          return _maps[left];

        int rightFront = *(INT32*) _maps[right]->front();

        if( from < rightFront ) {
          return _maps[left];
        }

        return _maps[right];
      }

      const char* _findInBuffer( indri::utility::Buffer* b, int from ) {
        const char* start = b->front();
        const char* end = b->front() + b->position();

        while( end - start > 32 ) {
          const char* mid = start + (((end - start) / 2) & ~31);
          INT32 middle = *(INT32*) mid;

          if( from >= middle )
            start = mid;
          else
            end = mid;
        }

        INT32 front = *(INT32*)start;
        assert( from >= front && from < (front + 192) ); 

        return start;
      }

      int _bitsSet( unsigned char c ) {
        static int bset[] = { 0, 1, 1, 2,   // 0, 1, 2, 3
                              1, 2, 2, 3,   // 4, 5, 6, 7
                              1, 2, 2, 3,   // 8, 9, A, B
                              2, 3, 3, 4 }; // C, D, E, F

        return bset[ c & 0xf ] + bset[ (c>>4) ];
      }

    public:
      TermBitmap() {
        _lastFrom = -1;
        _toBase = -10000;
        _fromBase = -10000;
      }

      ~TermBitmap() {
        delete_vector_contents( _maps );
      }

      int lastFrom() {
        return _lastFrom;
      }

      void add( int to ) {
        add( _lastFrom+1, to );
      }

      void add( int from, int to ) {
        assert( _lastFrom < from );

        const int availableSpace = ((32 - 8) * 8);
        int difference = to - _toBase;

        assert( difference >= 0 );

        if( difference >= availableSpace || _lastFrom < from-1 ) {
          _addBufferIfNecessary();

          // get the current buffer
          indri::utility::Buffer* back = _maps.back();
          _current = back->write( 32 );

          *(INT32*)_current = from;
          *(INT32*)(_current + 4) = to;
          memset( _current + 8, 0, (32 - 8) ); 

          _toBase = to;
          _fromBase = from;
          difference = 0;
        }

        _current[8+difference/8] |= 1<<(difference%8);
        _lastFrom = from;

        assert( get(from) == to );
      }

      int get( int from ) {
        assert( from <= _lastFrom );

        // first, binary search through the buffers themselves
        indri::utility::Buffer* buffer = _findBuffer( from );
        const char* spot = _findInBuffer( buffer, from );

        // now, we have to scan through this buffer to find the right value
        int fromBase = *(INT32*)spot;
        int toBase = *(INT32*)(spot+4);
        spot += 8;
        int bits = 0;
        int found = 0;

        // first, go byte by byte
        unsigned char c;
        int need = from - fromBase + 1; // number of bits we need to find (plus 1 for the zero bit)

        c = spot[bits/8];
        int byteBits = _bitsSet( c );

        while( found + byteBits < need ) {
          bits += 8;
          found += byteBits;
          c = spot[bits/8];
          byteBits = _bitsSet( c );
        }

        // now, examine each bit
        int i;
        for( i=0; i<8; i++ ) {
          if( c & 1<<i ) {
            found++;

            if( found == need )
              break;
          }
        }

        bits += i;
        assert( (fromBase + found - 1) == from );
        return toBase + bits;
      }

      size_t memorySize() {
        return _maps.size() * (64*1024);
      }
    };
  }
}

#endif // INDRI_TERMBITMAP_HPP
