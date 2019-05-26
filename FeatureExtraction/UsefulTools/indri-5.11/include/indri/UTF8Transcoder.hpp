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
// UTF8Transcoder
//
// 15 September 2005 -- mwb
//

// Simple class that converts Unicode characters to and from UTF-8
// encoding.  This class is used by the TextTokenizer for interpreting
// strings identified as possibly being in UTF-8 encoding, and by the
// UTF8CaseNormalizationTransformation.

#ifndef INDRI_UTF8TRANSCODER_HPP
#define INDRI_UTF8TRANSCODER_HPP

#include "indri/indri-platform.h"
#include "indri/HashTable.hpp"
#include <string.h>

namespace indri {
  namespace parse {
    
    namespace CharClass {

      const int apostrophe = 1;
      const int percent = 2;
      const int control = 3;
      const int currency = 4;
      const int symbol = 5;
      const int letter = 6;
      const int digit = 7;
      const int punctuation = 8;
      const int whitespace = 9;
      const int decimal = 10;
      const int hyphen = 11;
      const int thousand = 12;
    }

    class UTF8Transcoder {

    private:

      // This function checks for a sequence of bytes between and
      // inclusive of 0x80 and 0xBF and returns how many exist.

      int _count_bytes( unsigned char* buf, int index, int max_index, int how_many );

      indri::utility::HashTable<UINT64,const int> u;
      void store_interval( indri::utility::HashTable<UINT64,const int>&
                                  table, UINT64 start, UINT64 end, 
                                  const int cls );

      void _initHT() ;

    public:
      UTF8Transcoder();
      ~UTF8Transcoder();

      // This function computes the UTF-8 byte sequence for the
      // specified Unicode character code.  The bytes are written into
      // the specified buffer, which must be large enough to hold the
      // byte sequence (always <= 6 bytes in length) as well as the
      // terminating null.  The number of octets, which equals the
      // number of bytes written to the buffer not including the
      // terminating null, is stored in the octets integer.

      void utf8_encode( UINT64 code, char* buf, int* octets );

      // This function decodes a char[] assumed to be in UTF-8
      // encoding.  Results are stored in as unicode codes in the
      // supplied UINT64[] array, which must have as many elements as
      // the char[] array because in the worst case, it contains an
      // ASCII string.  The number of characters decoded is stored in
      // the characters integer, and the number of malformed bytes
      // skipped is stored in the malformed integer.  Offsets and
      // lengths are optional parameters; if not NULL, they will be
      // filled with byte offsets where the UTF-8 characters begin and
      // lengths of each encoding in bytes as they occurr in the input
      // buffer.  Offsets and lengths must have as many elements as
      // the input buffer has bytes.

      void utf8_decode( const char* buf_in, UINT64** codes, int* characters,
                        int* malformed, int** offsets, int** lengths );
      
      indri::utility::HashTable<UINT64,const int>& unicode() {
        return u;
      }      
    };

  }
}

#endif // INDRI_UTF8TRANSCODER_HPP

