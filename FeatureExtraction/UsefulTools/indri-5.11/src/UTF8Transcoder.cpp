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
// 27 June 2006 -- refactor to put the code in a separate compilation unit.

#include "indri/UTF8Transcoder.hpp"

int indri::parse::UTF8Transcoder::_count_bytes( unsigned char* buf, int index, int max_index, int how_many ) {

  int count = 0;

  for ( int i = index + 1; i <= index + how_many; i++ )
    if ( i < max_index && buf[i] && buf[i] != '\0' && 
         0x80 <= buf[i] && buf[i] <= 0xBF ) 
      count++;

  return count;
}

void indri::parse::UTF8Transcoder::store_interval( indri::utility::HashTable<UINT64,const int>&
                                                   table, UINT64 start, UINT64 end, 
                                                   const int cls ) {

  for ( UINT64 i = start; i <= end; i++ )
    table.insert( i, cls );
}


indri::parse::UTF8Transcoder::UTF8Transcoder() { _initHT();}
indri::parse::UTF8Transcoder::~UTF8Transcoder() {}

void indri::parse::UTF8Transcoder::utf8_encode( UINT64 code, char* buf, int* octets ) {

  int bytes = 0;

  if ( code <= 0x7F ) {

    if ( buf ) {

      buf[0] = (char)code;
      buf[1] = '\0';
    }

    bytes = 1;

  } else if ( code >= 0x80 && code <= 0x7FF ) {

    if ( buf ) {

      buf[0] = 0xC0 | ( 0x1F & ( code >> 6 ) );
      buf[1] = 0x80 | ( 0x3F & code );
      buf[2] = '\0';
    }

    bytes = 2;

  } else if ( code >= 0x800 && code <= 0xFFFF ) {

    if ( buf ) {

      buf[0] = 0xE0 | ( 0xF & ( code >> 12 ) );
      buf[1] = 0x80 | ( 0x3F & ( code >> 6 ) );
      buf[2] = 0x80 | ( 0x3F & code );
      buf[3] = '\0';
    }

    bytes = 3;

  } else if ( code >= 0x1000 && code <= 0x1FFFFF ) {

    if ( buf ) {

      buf[0] = 0xF0 | ( 0x7 & ( code >> 18 ) );
      buf[1] = 0x80 | ( 0x3F & ( code >> 12 ) );
      buf[2] = 0x80 | ( 0x3F & ( code >> 6 ) );
      buf[3] = 0x80 | ( 0x3F & code );
      buf[4] = '\0';
    }

    bytes = 4;

  } else if ( code >= 0x200000 && code <= 0x3FFFFFF ) {

    if ( buf ) {

      buf[0] = 0xF8 | ( 0x7 & ( code >> 24 ) );
      buf[1] = 0x80 | ( 0x3F & ( code >> 18 ) );
      buf[2] = 0x80 | ( 0x3F & ( code >> 12 ) );
      buf[3] = 0x80 | ( 0x3F & ( code >> 6 ) );
      buf[4] = 0x80 | ( 0x3F & code );
      buf[5] = '\0';
    }

    bytes = 5;

  } else if ( code >= 0x4000000 && code <= 0x7FFFFFFF ) {

    if ( buf ) {

      buf[0] = 0xFC | ( 0x1 & ( code >> 30 ) );
      buf[1] = 0x80 | ( 0x3F & ( code >> 24 ) );
      buf[2] = 0x80 | ( 0x3F & ( code >> 18 ) );
      buf[3] = 0x80 | ( 0x3F & ( code >> 12 ) );
      buf[4] = 0x80 | ( 0x3F & ( code >> 6 ) );
      buf[5] = 0x80 | ( 0x3F & code );
      buf[6] = '\0';
    }

    bytes = 5;

  } else {

    if ( buf ) buf[0] = '\0';
  }

  if ( octets ) *octets = bytes;
}

void indri::parse::UTF8Transcoder::utf8_decode( const char* buf_in, UINT64** codes, int* characters,
                                                int* malformed, int** offsets, int** lengths ) {

  int len = strlen( buf_in );
  int i = 0; // index into codes array
  int j = 0; // index into input buf
  UINT64 code = 0;
  int bytes;

  unsigned char* buf = (unsigned char*)buf_in;
        
  while ( j < len ) {

    bool code_ok = false;
    bytes = 0;
          
    if ( buf[j] <= 0x7F ) { // 7-bit ASCII character (One-byte sequence)

      code = (UINT64)buf[j];
      bytes = 1;
      code_ok = true;

    } else if ( 0xC0 <= buf[j] && buf[j] <= 0xDF ) { // Two-byte sequence

      bytes = 1 + _count_bytes( buf, j, len, 1 );

      if ( bytes == 2 ) {

        code = ( ( 0x1F & buf[j] ) << 6 ) + ( 0x3F & buf[j + 1] );
              
        if ( code < 0x80 ) { // Overlong sequence detected

          if ( malformed ) *malformed += 2;

        } else {

          code_ok = true;
        }
      } else { // Subsequent bytes of sequence malformed

        bytes = 1;
        if ( malformed ) *malformed++;
      }

    } else if ( 0xE0 <= buf[j] && buf[j] <= 0xEF ) { // Three-byte sequence

      bytes = 1 + _count_bytes( buf, j, len, 2 );

      if ( bytes == 3 ) {

        code = ( ( 0xF & buf[j] ) << 12 ) + ( ( 0x3F & buf[j + 1] ) << 6 ) +
          ( 0x3F & buf[j + 2] );

        if ( code < 0x800 ) { // Overlong sequence detected
                
          if ( malformed ) *malformed += 3;

        } else {
                
          code_ok = true;
        }
      } else { // Subsequent bytes of sequence malformed
          
        bytes = 1;
        if ( malformed ) *malformed += 3;
      }

    } else if ( 0xF0 <= buf[j] && buf[j] <= 0xF7 ) { // Four-byte sequence

      bytes = 1 + _count_bytes( buf, j, len, 3 );

      if ( bytes == 4 ) {

        code = ( ( 0x7 & buf[j] ) << 18 ) + ( ( 0x3F & buf[j + 1] ) << 12 ) +
          ( ( 0x3F & buf[j + 2] ) << 6 ) + ( 0x3F & buf[j + 3] );

        if ( code < 0x10000 ) { // Overlong sequence detected
                
          if ( malformed ) *malformed += 4;

        } else {

          code_ok = true;
        }
      } else { // Subsequent bytes of sequence malformed

        bytes = 1;
        if ( malformed ) *malformed += 4;
      }

    } else if ( 0xF8 <= buf[j] && buf[j] <= 0xFB ) { // Five-byte sequence

      bytes = 1 + _count_bytes( buf, j, len, 4 );

      if ( bytes == 5 ) {

        code = ( ( 0x3 & buf[j] ) << 24 ) + ( ( 0x3F & buf[j + 1] ) << 18 ) +
          ( ( 0x3F & buf[j + 2] ) << 12 ) + ( ( 0x3F & buf[j + 3] ) << 6 ) +
          ( 0x3F & buf[j + 4] );
              
        if ( code < 0x200000 ) { // Overlong sequence detected
                
          if ( malformed ) *malformed += 5;
        } else {

          code_ok = true;
        }
      } else { // Subsequent bytes of sequence malformed

        bytes = 1;
        if ( malformed ) *malformed += 5;
      }

    } else if ( 0xFC <= buf[j] && buf[j] <= 0xFD ) { // Six-byte sequence

      bytes = 1 + _count_bytes( buf, j, len, 5 );

      if ( bytes == 6 ) {

        code = ( ( 0x1 & buf[j] ) << 30 ) + ( ( 0x3F & buf[j + 1] ) << 24 ) +
          ( ( 0x3F & buf[j + 2] ) << 18 ) + ( ( 0x3F & buf[j + 3] ) << 12 ) + 
          ( ( 0x3F & buf[j + 4] ) << 6 ) + ( 0x3F & buf[j + 5] );

        if ( code < 0x4000000 ) { // Overlong sequence detected
              
          if ( malformed ) *malformed += 6;

        } else {

          code_ok = true;
        }
      } else { // Subsequent bytes of sequence malformed

        bytes = 1;
        if ( malformed ) *malformed += 6;
      }

    } else { // Bytes 0xFE and 0xFF are invalid in UTF-8
      // Bytes 0x80-0xBF are invalid as the first byte of a
      // sequence.

      bytes = 1;
      if ( malformed ) *malformed += 1;
    }

    if ( code_ok ) {

      if ( offsets ) (*offsets)[i] = j;
      if ( lengths ) (*lengths)[i] = bytes;
      if ( codes ) (*codes)[i] = code;
      i++;
      if ( characters ) (*characters)++;
    }

    j += bytes;
  }

  // Terminate the arrays:
  if ( offsets ) (*offsets)[i] = 0;
  if ( lengths ) (*lengths)[i] = 0;
  if ( codes ) (*codes)[i] = 0;
}

struct intervalEntry {
  UINT64 start;
  UINT64 end;
  const int cls ;
};


static const struct intervalEntry intervals[] = { 
  { 0xFF0C, 0xFF0C, indri::parse::CharClass::thousand },
  { 0x002C, 0x002C, indri::parse::CharClass::thousand }, 
  { 0x066C, 0x066C, indri::parse::CharClass::thousand },

  { 0x002D, 0x002D, indri::parse::CharClass::hyphen },
  { 0x2010, 0x2011, indri::parse::CharClass::hyphen },
  { 0xFF0D, 0xFF0D, indri::parse::CharClass::hyphen },

  { 0x002E, 0x002E, indri::parse::CharClass::decimal },
  { 0x066B, 0x066B, indri::parse::CharClass::decimal },
  { 0xFF0E, 0xFF0E, indri::parse::CharClass::decimal },

  { 0x2000, 0x200A, indri::parse::CharClass::whitespace },
  { 0x2028, 0x2029, indri::parse::CharClass::whitespace },
  { 0x202F, 0x202F, indri::parse::CharClass::whitespace },
  { 0x205F, 0x205F, indri::parse::CharClass::whitespace },
  { 0x0020, 0x0020, indri::parse::CharClass::whitespace },
  { 0x1680, 0x1680, indri::parse::CharClass::whitespace },
  { 0x180E, 0x180E, indri::parse::CharClass::whitespace },
  { 0x00A0, 0x00A0, indri::parse::CharClass::whitespace },
  { 0x3000, 0x3000, indri::parse::CharClass::whitespace },

  { 0x17D4, 0x17D6, indri::parse::CharClass::punctuation },
  { 0x17D8, 0x17DA, indri::parse::CharClass::punctuation },
  { 0x10A50, 0x10A58, indri::parse::CharClass::punctuation },
  { 0x0021, 0x0023, indri::parse::CharClass::punctuation },
  { 0x0026, 0x0026, indri::parse::CharClass::punctuation },
  { 0x0028, 0x002A, indri::parse::CharClass::punctuation },
  { 0x002F, 0x002F, indri::parse::CharClass::punctuation },
  { 0x003A, 0x003B, indri::parse::CharClass::punctuation },
  { 0x003F, 0x0040, indri::parse::CharClass::punctuation },
  { 0x005B, 0x0060, indri::parse::CharClass::punctuation },
  { 0x007B, 0x007B, indri::parse::CharClass::punctuation },
  { 0x007D, 0x007E, indri::parse::CharClass::punctuation },
  { 0x1039F, 0x1039F, indri::parse::CharClass::punctuation },
  { 0x0E4F, 0x0E4F, indri::parse::CharClass::punctuation },
  { 0x0E5A, 0x0E5B, indri::parse::CharClass::punctuation },
  { 0x0374, 0x0375, indri::parse::CharClass::punctuation },
  { 0x037E, 0x037E, indri::parse::CharClass::punctuation },
  { 0x0384, 0x0385, indri::parse::CharClass::punctuation },
  { 0x0387, 0x0387, indri::parse::CharClass::punctuation },
  { 0x10FB, 0x10FB, indri::parse::CharClass::punctuation },
  { 0x0DF4, 0x0DF4, indri::parse::CharClass::punctuation },
  { 0x1800, 0x180A, indri::parse::CharClass::punctuation },
  { 0x060C, 0x060D, indri::parse::CharClass::punctuation },
  { 0x061B, 0x061B, indri::parse::CharClass::punctuation },
  { 0x061E, 0x061F, indri::parse::CharClass::punctuation },
  { 0x066D, 0x066D, indri::parse::CharClass::punctuation },
  { 0x06D4, 0x06D4, indri::parse::CharClass::punctuation },
  { 0xFD3E, 0xFD3F, indri::parse::CharClass::punctuation },
  { 0xFE10, 0xFE19, indri::parse::CharClass::punctuation },
  { 0xFE30, 0xFE52, indri::parse::CharClass::punctuation },
  { 0xFE54, 0xFE61, indri::parse::CharClass::punctuation },
  { 0xFE63, 0xFE63, indri::parse::CharClass::punctuation },
  { 0xFE68, 0xFE68, indri::parse::CharClass::punctuation },
  { 0xFE6A, 0xFE6B, indri::parse::CharClass::punctuation },
  { 0x02C2, 0x02C5, indri::parse::CharClass::punctuation },
  { 0x02D2, 0x02DF, indri::parse::CharClass::punctuation },
  { 0x02E5, 0x02ED, indri::parse::CharClass::punctuation },
  { 0x02EF, 0x02FF, indri::parse::CharClass::punctuation },
  { 0x0F04, 0x0F12, indri::parse::CharClass::punctuation },
  { 0x0F3A, 0x0F3D, indri::parse::CharClass::punctuation },
  { 0x0F85, 0x0F85, indri::parse::CharClass::punctuation },
  { 0x0FD0, 0x0FD1, indri::parse::CharClass::punctuation },
  { 0x1735, 0x1736, indri::parse::CharClass::punctuation },
  { 0x3001, 0x3003, indri::parse::CharClass::punctuation },
  { 0x3008, 0x3011, indri::parse::CharClass::punctuation },
  { 0x3014, 0x301F, indri::parse::CharClass::punctuation },
  { 0x3030, 0x3030, indri::parse::CharClass::punctuation },
  { 0x303D, 0x303D, indri::parse::CharClass::punctuation },
  { 0x30A0, 0x30A0, indri::parse::CharClass::punctuation },
  { 0x30FB, 0x30FB, indri::parse::CharClass::punctuation },
  { 0xFF65, 0xFF65, indri::parse::CharClass::punctuation },
  { 0x169B, 0x169C, indri::parse::CharClass::punctuation },
  { 0x1361, 0x1368, indri::parse::CharClass::punctuation },
  { 0x0589, 0x058A, indri::parse::CharClass::punctuation },
  { 0x19DE, 0x19DF, indri::parse::CharClass::punctuation },
  { 0x2012, 0x2015, indri::parse::CharClass::punctuation },
  { 0x2018, 0x2018, indri::parse::CharClass::punctuation },
  { 0x201A, 0x201F, indri::parse::CharClass::punctuation },
  { 0x2026, 0x2026, indri::parse::CharClass::punctuation },
  { 0x2039, 0x203A, indri::parse::CharClass::punctuation },
  { 0x203C, 0x2040, indri::parse::CharClass::punctuation },
  { 0x2044, 0x2049, indri::parse::CharClass::punctuation },
  { 0x204F, 0x204F, indri::parse::CharClass::punctuation },
  { 0x2053, 0x2054, indri::parse::CharClass::punctuation },
  { 0x207D, 0x207E, indri::parse::CharClass::punctuation },
  { 0x208D, 0x208E, indri::parse::CharClass::punctuation },
  { 0x2768, 0x2775, indri::parse::CharClass::punctuation },
  { 0x27C5, 0x27C6, indri::parse::CharClass::punctuation },
  { 0x27E6, 0x27EB, indri::parse::CharClass::punctuation },
  { 0x2983, 0x2998, indri::parse::CharClass::punctuation },
  { 0x29D8, 0x29DB, indri::parse::CharClass::punctuation },
  { 0x29FC, 0x29FD, indri::parse::CharClass::punctuation },
  { 0x2E00, 0x2E17, indri::parse::CharClass::punctuation },
  { 0x2E1C, 0x2E1D, indri::parse::CharClass::punctuation },
  { 0xFF01, 0xFF03, indri::parse::CharClass::punctuation },
  { 0xFF06, 0xFF06, indri::parse::CharClass::punctuation },
  { 0xFF08, 0xFF0A, indri::parse::CharClass::punctuation },
  { 0xFF0F, 0xFF0F, indri::parse::CharClass::punctuation },
  { 0xFF1A, 0xFF1B, indri::parse::CharClass::punctuation },
  { 0xFF1F, 0xFF20, indri::parse::CharClass::punctuation },
  { 0xFF3B, 0xFF40, indri::parse::CharClass::punctuation },
  { 0xFF5B, 0xFF64, indri::parse::CharClass::punctuation },
  { 0x2CF9, 0x2CFC, indri::parse::CharClass::punctuation },
  { 0x2CFE, 0x2CFF, indri::parse::CharClass::punctuation },
  { 0x0964, 0x0965, indri::parse::CharClass::punctuation },
  { 0x0970, 0x0970, indri::parse::CharClass::punctuation },
  { 0x0700, 0x070D, indri::parse::CharClass::punctuation },
  { 0x00A1, 0x00A1, indri::parse::CharClass::punctuation },
  { 0x00A8, 0x00A8, indri::parse::CharClass::punctuation },
  { 0x00AB, 0x00AB, indri::parse::CharClass::punctuation },
  { 0x00AF, 0x00AF, indri::parse::CharClass::punctuation },
  { 0x00B4, 0x00B4, indri::parse::CharClass::punctuation },
  { 0x00B7, 0x00B8, indri::parse::CharClass::punctuation },
  { 0x00BB, 0x00BB, indri::parse::CharClass::punctuation },
  { 0x00BF, 0x00BF, indri::parse::CharClass::punctuation },
  { 0x05BE, 0x05BE, indri::parse::CharClass::punctuation },
  { 0x05C0, 0x05C0, indri::parse::CharClass::punctuation },
  { 0x05C3, 0x05C3, indri::parse::CharClass::punctuation },
  { 0x05C6, 0x05C6, indri::parse::CharClass::punctuation },
  { 0x05F3, 0x05F4, indri::parse::CharClass::punctuation },
  { 0x10100, 0x10101, indri::parse::CharClass::punctuation },
  { 0x1A1E, 0x1A1F, indri::parse::CharClass::punctuation },
  { 0x104A, 0x104F, indri::parse::CharClass::punctuation },
  { 0x16EB, 0x16ED, indri::parse::CharClass::punctuation },
  { 0x1944, 0x1945, indri::parse::CharClass::punctuation },

  { 0x1369, 0x137C, indri::parse::CharClass::digit },
  { 0x0660, 0x0669, indri::parse::CharClass::digit },
  { 0x06F0, 0x06F9, indri::parse::CharClass::digit },
  { 0x1040, 0x1049, indri::parse::CharClass::digit },
  { 0x0AE6, 0x0AEF, indri::parse::CharClass::digit },
  { 0x0D66, 0x0D6F, indri::parse::CharClass::digit },
  { 0x1810, 0x1819, indri::parse::CharClass::digit },
  { 0x0BE6, 0x0BF2, indri::parse::CharClass::digit },
  { 0x1946, 0x194F, indri::parse::CharClass::digit },
  { 0x09E6, 0x09EF, indri::parse::CharClass::digit },
  { 0x09F4, 0x09F9, indri::parse::CharClass::digit },
  { 0x104A0, 0x104A9, indri::parse::CharClass::digit },
  { 0x0ED0, 0x0ED9, indri::parse::CharClass::digit },
  { 0x0F20, 0x0F33, indri::parse::CharClass::digit },
  { 0x0B66, 0x0B6F, indri::parse::CharClass::digit },
  { 0x10107, 0x10133, indri::parse::CharClass::digit },
  { 0x10320, 0x10323, indri::parse::CharClass::digit },
  { 0x00B2, 0x00B3, indri::parse::CharClass::digit },
  { 0x00B9, 0x00B9, indri::parse::CharClass::digit },
  { 0x00BC, 0x00BE, indri::parse::CharClass::digit },
  { 0x1034A, 0x1034A, indri::parse::CharClass::digit },
  { 0x1D7CE, 0x1D7FF, indri::parse::CharClass::digit },
  { 0x0C66, 0x0C6F, indri::parse::CharClass::digit },
  { 0x19D0, 0x19D9, indri::parse::CharClass::digit },
  { 0x0030, 0x0039, indri::parse::CharClass::digit },
  { 0x16EE, 0x16F0, indri::parse::CharClass::digit },
  { 0x0E50, 0x0E59, indri::parse::CharClass::digit },
  { 0x17E0, 0x17E9, indri::parse::CharClass::digit },
  { 0x17F0, 0x17F9, indri::parse::CharClass::digit },
  { 0x3007, 0x3007, indri::parse::CharClass::digit },
  { 0x3021, 0x3029, indri::parse::CharClass::digit },
  { 0x3038, 0x303A, indri::parse::CharClass::digit },
  { 0x3192, 0x3195, indri::parse::CharClass::digit },
  { 0x3220, 0x3229, indri::parse::CharClass::digit },
  { 0x3251, 0x325F, indri::parse::CharClass::digit },
  { 0x3280, 0x3289, indri::parse::CharClass::digit },
  { 0x32B1, 0x32BF, indri::parse::CharClass::digit },
  { 0x0A66, 0x0A6F, indri::parse::CharClass::digit },
  { 0x2070, 0x2070, indri::parse::CharClass::digit },
  { 0x2074, 0x2079, indri::parse::CharClass::digit },
  { 0x2080, 0x2089, indri::parse::CharClass::digit },
  { 0x2153, 0x2183, indri::parse::CharClass::digit },
  { 0x2460, 0x249B, indri::parse::CharClass::digit },
  { 0x24EA, 0x24FF, indri::parse::CharClass::digit },
  { 0x2776, 0x2793, indri::parse::CharClass::digit },
  { 0xFF10, 0xFF19, indri::parse::CharClass::digit },
  { 0x2CFD, 0x2CFD, indri::parse::CharClass::digit },
  { 0x0966, 0x096F, indri::parse::CharClass::digit },
  { 0x103D1, 0x103D5, indri::parse::CharClass::digit },
  { 0x0CE6, 0x0CEF, indri::parse::CharClass::digit },
  { 0x10140, 0x10178, indri::parse::CharClass::digit },
  { 0x1018A, 0x1018A, indri::parse::CharClass::digit },

  { 0x0981, 0x0983, indri::parse::CharClass::letter },
  { 0x09BC, 0x09BC, indri::parse::CharClass::letter },
  { 0x09BE, 0x09C4, indri::parse::CharClass::letter },
  { 0x09C7, 0x09C8, indri::parse::CharClass::letter },
  { 0x09CB, 0x09CD, indri::parse::CharClass::letter },
  { 0x09D7, 0x09D7, indri::parse::CharClass::letter },
  { 0x09E2, 0x09E3, indri::parse::CharClass::letter },
  { 0x10400, 0x1044F, indri::parse::CharClass::letter },
  { 0x10800, 0x10805, indri::parse::CharClass::letter },
  { 0x10808, 0x10808, indri::parse::CharClass::letter },
  { 0x1080A, 0x10835, indri::parse::CharClass::letter },
  { 0x10837, 0x10838, indri::parse::CharClass::letter },
  { 0x1083C, 0x1083C, indri::parse::CharClass::letter },
  { 0x1083F, 0x1083F, indri::parse::CharClass::letter },
  { 0x0300, 0x036F, indri::parse::CharClass::letter },
  { 0x1DC0, 0x1DC3, indri::parse::CharClass::letter },
  { 0x0904, 0x0939, indri::parse::CharClass::letter },
  { 0x093D, 0x093D, indri::parse::CharClass::letter },
  { 0x0950, 0x0950, indri::parse::CharClass::letter },
  { 0x0958, 0x0961, indri::parse::CharClass::letter },
  { 0x097D, 0x097D, indri::parse::CharClass::letter },
  { 0x0610, 0x0615, indri::parse::CharClass::letter },
  { 0x064B, 0x065E, indri::parse::CharClass::letter },
  { 0x0670, 0x0670, indri::parse::CharClass::letter },
  { 0x06D6, 0x06DC, indri::parse::CharClass::letter },
  { 0x06DE, 0x06E4, indri::parse::CharClass::letter },
  { 0x06E7, 0x06E8, indri::parse::CharClass::letter },
  { 0x06EA, 0x06ED, indri::parse::CharClass::letter },
  { 0xFE00, 0xFE0F, indri::parse::CharClass::letter },
  { 0xFE20, 0xFE23, indri::parse::CharClass::letter },
  { 0x05D0, 0x05EA, indri::parse::CharClass::letter },
  { 0x05F0, 0x05F2, indri::parse::CharClass::letter },
  { 0xFB1D, 0xFB1D, indri::parse::CharClass::letter },
  { 0xFB1F, 0xFB28, indri::parse::CharClass::letter },
  { 0xFB2A, 0xFB36, indri::parse::CharClass::letter },
  { 0xFB38, 0xFB3C, indri::parse::CharClass::letter },
  { 0xFB3E, 0xFB3E, indri::parse::CharClass::letter },
  { 0xFB40, 0xFB41, indri::parse::CharClass::letter },
  { 0xFB43, 0xFB44, indri::parse::CharClass::letter },
  { 0xFB46, 0xFB4F, indri::parse::CharClass::letter },
  { 0x0A05, 0x0A0A, indri::parse::CharClass::letter },
  { 0x0A0F, 0x0A10, indri::parse::CharClass::letter },
  { 0x0A13, 0x0A28, indri::parse::CharClass::letter },
  { 0x0A2A, 0x0A30, indri::parse::CharClass::letter },
  { 0x0A32, 0x0A33, indri::parse::CharClass::letter },
  { 0x0A35, 0x0A36, indri::parse::CharClass::letter },
  { 0x0A38, 0x0A39, indri::parse::CharClass::letter },
  { 0x0A59, 0x0A5C, indri::parse::CharClass::letter },
  { 0x0A5E, 0x0A5E, indri::parse::CharClass::letter },
  { 0x0A72, 0x0A74, indri::parse::CharClass::letter },
  { 0x180B, 0x180D, indri::parse::CharClass::letter },
  { 0x18A9, 0x18A9, indri::parse::CharClass::letter },
  { 0x0250, 0x02C1, indri::parse::CharClass::letter },
  { 0x02C6, 0x02D1, indri::parse::CharClass::letter },
  { 0x02E0, 0x02E4, indri::parse::CharClass::letter },
  { 0x02EE, 0x02EE, indri::parse::CharClass::letter },
  { 0x0C01, 0x0C03, indri::parse::CharClass::letter },
  { 0x0C3E, 0x0C44, indri::parse::CharClass::letter },
  { 0x0C46, 0x0C48, indri::parse::CharClass::letter },
  { 0x0C4A, 0x0C4D, indri::parse::CharClass::letter },
  { 0x0C55, 0x0C56, indri::parse::CharClass::letter },
  { 0x2D30, 0x2D65, indri::parse::CharClass::letter },
  { 0x2D6F, 0x2D6F, indri::parse::CharClass::letter },
  { 0x037A, 0x037A, indri::parse::CharClass::letter },
  { 0x0386, 0x0386, indri::parse::CharClass::letter },
  { 0x0388, 0x038A, indri::parse::CharClass::letter },
  { 0x038C, 0x038C, indri::parse::CharClass::letter },
  { 0x038E, 0x03A1, indri::parse::CharClass::letter },
  { 0x03A3, 0x03CE, indri::parse::CharClass::letter },
  { 0x03D0, 0x03F5, indri::parse::CharClass::letter },
  { 0x03F7, 0x03FF, indri::parse::CharClass::letter },
  { 0x302A, 0x302F, indri::parse::CharClass::letter },
  { 0x3099, 0x309A, indri::parse::CharClass::letter },
  { 0x1920, 0x192B, indri::parse::CharClass::letter },
  { 0x1930, 0x193B, indri::parse::CharClass::letter },
  { 0x0E01, 0x0E30, indri::parse::CharClass::letter },
  { 0x0E32, 0x0E33, indri::parse::CharClass::letter },
  { 0x0E40, 0x0E46, indri::parse::CharClass::letter },
  { 0x1780, 0x17B3, indri::parse::CharClass::letter },
  { 0x17D7, 0x17D7, indri::parse::CharClass::letter },
  { 0x17DC, 0x17DC, indri::parse::CharClass::letter },
  { 0x07A6, 0x07B0, indri::parse::CharClass::letter },
  { 0x10380, 0x1039D, indri::parse::CharClass::letter },
  { 0x0483, 0x0486, indri::parse::CharClass::letter },
  { 0x0488, 0x0489, indri::parse::CharClass::letter },
  { 0x17B6, 0x17D3, indri::parse::CharClass::letter },
  { 0x17DD, 0x17DD, indri::parse::CharClass::letter },
  { 0x0E31, 0x0E31, indri::parse::CharClass::letter },
  { 0x0E34, 0x0E3A, indri::parse::CharClass::letter },
  { 0x0E47, 0x0E4E, indri::parse::CharClass::letter },
  { 0x3005, 0x3006, indri::parse::CharClass::letter },
  { 0x3031, 0x3035, indri::parse::CharClass::letter },
  { 0x303B, 0x303C, indri::parse::CharClass::letter },
  { 0x3041, 0x3096, indri::parse::CharClass::letter },
  { 0x309D, 0x309F, indri::parse::CharClass::letter },
  { 0x30A1, 0x30FA, indri::parse::CharClass::letter },
  { 0x30FC, 0x30FF, indri::parse::CharClass::letter },
  { 0x3105, 0x312C, indri::parse::CharClass::letter },
  { 0x3131, 0x318E, indri::parse::CharClass::letter },
  { 0x31A0, 0x31B7, indri::parse::CharClass::letter },
  { 0x31F0, 0x31FF, indri::parse::CharClass::letter },
  { 0x3400, 0x4DB5, indri::parse::CharClass::letter },
  { 0x4E00, 0x9FBB, indri::parse::CharClass::letter },
  { 0xAC00, 0xD7A3, indri::parse::CharClass::letter },
  { 0xF900, 0xFAD9, indri::parse::CharClass::letter },
  { 0xFF66, 0xFFBE, indri::parse::CharClass::letter },
  { 0xFFC2, 0xFFC7, indri::parse::CharClass::letter },
  { 0xFFCA, 0xFFCF, indri::parse::CharClass::letter },
  { 0xFFD2, 0xFFD7, indri::parse::CharClass::letter },
  { 0xFFDA, 0xFFDC, indri::parse::CharClass::letter },
  { 0x20000, 0x2A6D6, indri::parse::CharClass::letter },
  { 0x2F800, 0x2FA1D, indri::parse::CharClass::letter },
  { 0x0A85, 0x0A8D, indri::parse::CharClass::letter },
  { 0x0A8F, 0x0A91, indri::parse::CharClass::letter },
  { 0x0A93, 0x0AA8, indri::parse::CharClass::letter },
  { 0x0AAA, 0x0AB0, indri::parse::CharClass::letter },
  { 0x0AB2, 0x0AB3, indri::parse::CharClass::letter },
  { 0x0AB5, 0x0AB9, indri::parse::CharClass::letter },
  { 0x0ABD, 0x0ABD, indri::parse::CharClass::letter },
  { 0x0AD0, 0x0AD0, indri::parse::CharClass::letter },
  { 0x0AE0, 0x0AE1, indri::parse::CharClass::letter },
  { 0x0531, 0x0556, indri::parse::CharClass::letter },
  { 0x0559, 0x0559, indri::parse::CharClass::letter },
  { 0x055B, 0x055F, indri::parse::CharClass::letter },
  { 0x0561, 0x0587, indri::parse::CharClass::letter },
  { 0xFB13, 0xFB17, indri::parse::CharClass::letter },
  { 0x0D05, 0x0D0C, indri::parse::CharClass::letter },
  { 0x0D0E, 0x0D10, indri::parse::CharClass::letter },
  { 0x0D12, 0x0D28, indri::parse::CharClass::letter },
  { 0x0D2A, 0x0D39, indri::parse::CharClass::letter },
  { 0x0D60, 0x0D61, indri::parse::CharClass::letter },
  { 0x19B0, 0x19C0, indri::parse::CharClass::letter },
  { 0x19C8, 0x19C9, indri::parse::CharClass::letter },
  { 0x0901, 0x0903, indri::parse::CharClass::letter },
  { 0x093C, 0x093C, indri::parse::CharClass::letter },
  { 0x093E, 0x094D, indri::parse::CharClass::letter },
  { 0x0951, 0x0954, indri::parse::CharClass::letter },
  { 0x0962, 0x0963, indri::parse::CharClass::letter },
  { 0x1681, 0x169A, indri::parse::CharClass::letter },
  { 0x0D85, 0x0D96, indri::parse::CharClass::letter },
  { 0x0D9A, 0x0DB1, indri::parse::CharClass::letter },
  { 0x0DB3, 0x0DBB, indri::parse::CharClass::letter },
  { 0x0DBD, 0x0DBD, indri::parse::CharClass::letter },
  { 0x0DC0, 0x0DC6, indri::parse::CharClass::letter },
  { 0x1820, 0x1877, indri::parse::CharClass::letter },
  { 0x1880, 0x18A8, indri::parse::CharClass::letter },
  { 0x0B83, 0x0B83, indri::parse::CharClass::letter },
  { 0x0B85, 0x0B8A, indri::parse::CharClass::letter },
  { 0x0B8E, 0x0B90, indri::parse::CharClass::letter },
  { 0x0B92, 0x0B95, indri::parse::CharClass::letter },
  { 0x0B99, 0x0B9A, indri::parse::CharClass::letter },
  { 0x0B9C, 0x0B9C, indri::parse::CharClass::letter },
  { 0x0B9E, 0x0B9F, indri::parse::CharClass::letter },
  { 0x0BA3, 0x0BA4, indri::parse::CharClass::letter },
  { 0x0BA8, 0x0BAA, indri::parse::CharClass::letter },
  { 0x0BAE, 0x0BB9, indri::parse::CharClass::letter },
  { 0x1900, 0x191C, indri::parse::CharClass::letter },
  { 0x1100, 0x1159, indri::parse::CharClass::letter },
  { 0x115F, 0x11A2, indri::parse::CharClass::letter },
  { 0x11A8, 0x11F9, indri::parse::CharClass::letter },
  { 0x1F00, 0x1F15, indri::parse::CharClass::letter },
  { 0x1F18, 0x1F1D, indri::parse::CharClass::letter },
  { 0x1F20, 0x1F45, indri::parse::CharClass::letter },
  { 0x1F48, 0x1F4D, indri::parse::CharClass::letter },
  { 0x1F50, 0x1F57, indri::parse::CharClass::letter },
  { 0x1F59, 0x1F59, indri::parse::CharClass::letter },
  { 0x1F5B, 0x1F5B, indri::parse::CharClass::letter },
  { 0x1F5D, 0x1F5D, indri::parse::CharClass::letter },
  { 0x1F5F, 0x1F7D, indri::parse::CharClass::letter },
  { 0x1F80, 0x1FB4, indri::parse::CharClass::letter },
  { 0x1FB6, 0x1FC4, indri::parse::CharClass::letter },
  { 0x1FC6, 0x1FD3, indri::parse::CharClass::letter },
  { 0x1FD6, 0x1FDB, indri::parse::CharClass::letter },
  { 0x1FDD, 0x1FEC, indri::parse::CharClass::letter },
  { 0x1FF2, 0x1FF4, indri::parse::CharClass::letter },
  { 0x1FF6, 0x1FFE, indri::parse::CharClass::letter },
  { 0x1712, 0x1714, indri::parse::CharClass::letter },
  { 0xA000, 0xA48C, indri::parse::CharClass::letter },
  { 0x0591, 0x05B9, indri::parse::CharClass::letter },
  { 0x05BB, 0x05BD, indri::parse::CharClass::letter },
  { 0x05BF, 0x05BF, indri::parse::CharClass::letter },
  { 0x05C1, 0x05C2, indri::parse::CharClass::letter },
  { 0x05C4, 0x05C5, indri::parse::CharClass::letter },
  { 0x05C7, 0x05C7, indri::parse::CharClass::letter },
  { 0xFB1E, 0xFB1E, indri::parse::CharClass::letter },
  { 0x1401, 0x1676, indri::parse::CharClass::letter },
  { 0x1A00, 0x1A16, indri::parse::CharClass::letter },
  { 0x1732, 0x1734, indri::parse::CharClass::letter },
  { 0x1772, 0x1773, indri::parse::CharClass::letter },
  { 0x0985, 0x098C, indri::parse::CharClass::letter },
  { 0x098F, 0x0990, indri::parse::CharClass::letter },
  { 0x0993, 0x09A8, indri::parse::CharClass::letter },
  { 0x09AA, 0x09B0, indri::parse::CharClass::letter },
  { 0x09B2, 0x09B2, indri::parse::CharClass::letter },
  { 0x09B6, 0x09B9, indri::parse::CharClass::letter },
  { 0x09BD, 0x09BD, indri::parse::CharClass::letter },
  { 0x09CE, 0x09CE, indri::parse::CharClass::letter },
  { 0x09DC, 0x09DD, indri::parse::CharClass::letter },
  { 0x09DF, 0x09E1, indri::parse::CharClass::letter },
  { 0x09F0, 0x09F1, indri::parse::CharClass::letter },
  { 0x10480, 0x1049D, indri::parse::CharClass::letter },
  { 0x0E81, 0x0E82, indri::parse::CharClass::letter },
  { 0x0E84, 0x0E84, indri::parse::CharClass::letter },
  { 0x0E87, 0x0E88, indri::parse::CharClass::letter },
  { 0x0E8A, 0x0E8A, indri::parse::CharClass::letter },
  { 0x0E8D, 0x0E8D, indri::parse::CharClass::letter },
  { 0x0E94, 0x0E97, indri::parse::CharClass::letter },
  { 0x0E99, 0x0E9F, indri::parse::CharClass::letter },
  { 0x0EA1, 0x0EA3, indri::parse::CharClass::letter },
  { 0x0EA5, 0x0EA5, indri::parse::CharClass::letter },
  { 0x0EA7, 0x0EA7, indri::parse::CharClass::letter },
  { 0x0EAA, 0x0EAB, indri::parse::CharClass::letter },
  { 0x0EAD, 0x0EB0, indri::parse::CharClass::letter },
  { 0x0EB2, 0x0EB3, indri::parse::CharClass::letter },
  { 0x0EBD, 0x0EBD, indri::parse::CharClass::letter },
  { 0x0EC0, 0x0EC4, indri::parse::CharClass::letter },
  { 0x0EC6, 0x0EC6, indri::parse::CharClass::letter },
  { 0x0EDC, 0x0EDD, indri::parse::CharClass::letter },
  { 0x0F00, 0x0F00, indri::parse::CharClass::letter },
  { 0x0F40, 0x0F47, indri::parse::CharClass::letter },
  { 0x0F49, 0x0F6A, indri::parse::CharClass::letter },
  { 0x0F88, 0x0F8B, indri::parse::CharClass::letter },
  { 0x0A81, 0x0A83, indri::parse::CharClass::letter },
  { 0x0ABC, 0x0ABC, indri::parse::CharClass::letter },
  { 0x0ABE, 0x0AC5, indri::parse::CharClass::letter },
  { 0x0AC7, 0x0AC9, indri::parse::CharClass::letter },
  { 0x0ACB, 0x0ACD, indri::parse::CharClass::letter },
  { 0x0AE2, 0x0AE3, indri::parse::CharClass::letter },
  { 0x0B05, 0x0B0C, indri::parse::CharClass::letter },
  { 0x0B13, 0x0B28, indri::parse::CharClass::letter },
  { 0x0B2A, 0x0B30, indri::parse::CharClass::letter },
  { 0x0B32, 0x0B33, indri::parse::CharClass::letter },
  { 0x0B35, 0x0B39, indri::parse::CharClass::letter },
  { 0x0B3D, 0x0B3D, indri::parse::CharClass::letter },
  { 0x0B5C, 0x0B5D, indri::parse::CharClass::letter },
  { 0x0B5F, 0x0B61, indri::parse::CharClass::letter },
  { 0x0B71, 0x0B71, indri::parse::CharClass::letter },
  { 0x1752, 0x1753, indri::parse::CharClass::letter },
  { 0x0780, 0x07A5, indri::parse::CharClass::letter },
  { 0x07B1, 0x07B1, indri::parse::CharClass::letter },
  { 0x10300, 0x1031E, indri::parse::CharClass::letter },
  { 0x0710, 0x0710, indri::parse::CharClass::letter },
  { 0x0712, 0x072F, indri::parse::CharClass::letter },
  { 0x1760, 0x176C, indri::parse::CharClass::letter },
  { 0x176E, 0x1770, indri::parse::CharClass::letter },
  { 0x0B0F, 0x0B10, indri::parse::CharClass::letter },
  { 0x1200, 0x1248, indri::parse::CharClass::letter },
  { 0x124A, 0x124D, indri::parse::CharClass::letter },
  { 0x1250, 0x1256, indri::parse::CharClass::letter },
  { 0x1258, 0x1258, indri::parse::CharClass::letter },
  { 0x125A, 0x125D, indri::parse::CharClass::letter },
  { 0x1260, 0x1288, indri::parse::CharClass::letter },
  { 0x128A, 0x128D, indri::parse::CharClass::letter },
  { 0x1290, 0x12B0, indri::parse::CharClass::letter },
  { 0x12B2, 0x12B5, indri::parse::CharClass::letter },
  { 0x12B8, 0x12BE, indri::parse::CharClass::letter },
  { 0x12C0, 0x12C0, indri::parse::CharClass::letter },
  { 0x12C2, 0x12C5, indri::parse::CharClass::letter },
  { 0x12C8, 0x12D6, indri::parse::CharClass::letter },
  { 0x12D8, 0x1310, indri::parse::CharClass::letter },
  { 0x1312, 0x1315, indri::parse::CharClass::letter },
  { 0x1318, 0x135A, indri::parse::CharClass::letter },
  { 0x1380, 0x138F, indri::parse::CharClass::letter },
  { 0x2D80, 0x2D96, indri::parse::CharClass::letter },
  { 0x2DA0, 0x2DA6, indri::parse::CharClass::letter },
  { 0x2DA8, 0x2DAE, indri::parse::CharClass::letter },
  { 0x2DB0, 0x2DB6, indri::parse::CharClass::letter },
  { 0x2DB8, 0x2DBE, indri::parse::CharClass::letter },
  { 0x2DC0, 0x2DC6, indri::parse::CharClass::letter },
  { 0x2DC8, 0x2DCE, indri::parse::CharClass::letter },
  { 0x2DD0, 0x2DD6, indri::parse::CharClass::letter },
  { 0x2DD8, 0x2DDE, indri::parse::CharClass::letter },
  { 0x1E00, 0x1E9B, indri::parse::CharClass::letter },
  { 0x1EA0, 0x1EF9, indri::parse::CharClass::letter },
  { 0x1950, 0x196D, indri::parse::CharClass::letter },
  { 0x1970, 0x1974, indri::parse::CharClass::letter },
  { 0x0EB1, 0x0EB1, indri::parse::CharClass::letter },
  { 0x0EB4, 0x0EB9, indri::parse::CharClass::letter },
  { 0x0EBB, 0x0EBC, indri::parse::CharClass::letter },
  { 0x0EC8, 0x0ECD, indri::parse::CharClass::letter },
  { 0x1D00, 0x1DBF, indri::parse::CharClass::letter },
  { 0x0621, 0x063A, indri::parse::CharClass::letter },
  { 0x0640, 0x064A, indri::parse::CharClass::letter },
  { 0x066E, 0x066F, indri::parse::CharClass::letter },
  { 0x0671, 0x06D3, indri::parse::CharClass::letter },
  { 0x06D5, 0x06D5, indri::parse::CharClass::letter },
  { 0x06E5, 0x06E6, indri::parse::CharClass::letter },
  { 0x06EE, 0x06EF, indri::parse::CharClass::letter },
  { 0x06FA, 0x06FC, indri::parse::CharClass::letter },
  { 0x06FF, 0x06FF, indri::parse::CharClass::letter },
  { 0x0750, 0x076D, indri::parse::CharClass::letter },
  { 0xFB50, 0xFBB1, indri::parse::CharClass::letter },
  { 0xFBD3, 0xFD3D, indri::parse::CharClass::letter },
  { 0xFD50, 0xFD8F, indri::parse::CharClass::letter },
  { 0xFD92, 0xFDC7, indri::parse::CharClass::letter },
  { 0xFDF0, 0xFDFB, indri::parse::CharClass::letter },
  { 0xFE70, 0xFE74, indri::parse::CharClass::letter },
  { 0xFE76, 0xFEFC, indri::parse::CharClass::letter },
  { 0x0C82, 0x0C83, indri::parse::CharClass::letter },
  { 0x0CBC, 0x0CBC, indri::parse::CharClass::letter },
  { 0x0CBE, 0x0CC4, indri::parse::CharClass::letter },
  { 0x0CC6, 0x0CC8, indri::parse::CharClass::letter },
  { 0x0CCA, 0x0CCD, indri::parse::CharClass::letter },
  { 0x0CD5, 0x0CD6, indri::parse::CharClass::letter },
  { 0x0B82, 0x0B82, indri::parse::CharClass::letter },
  { 0x0BBE, 0x0BC2, indri::parse::CharClass::letter },
  { 0x0BC6, 0x0BC8, indri::parse::CharClass::letter },
  { 0x0BCA, 0x0BCD, indri::parse::CharClass::letter },
  { 0x0BD7, 0x0BD7, indri::parse::CharClass::letter },
  { 0x10A0, 0x10C5, indri::parse::CharClass::letter },
  { 0x10D0, 0x10FA, indri::parse::CharClass::letter },
  { 0x10FC, 0x10FC, indri::parse::CharClass::letter },
  { 0x2D00, 0x2D25, indri::parse::CharClass::letter },
  { 0x0F18, 0x0F19, indri::parse::CharClass::letter },
  { 0x0F35, 0x0F35, indri::parse::CharClass::letter },
  { 0x0F37, 0x0F37, indri::parse::CharClass::letter },
  { 0x0F39, 0x0F39, indri::parse::CharClass::letter },
  { 0x0F3E, 0x0F3F, indri::parse::CharClass::letter },
  { 0x0F71, 0x0F84, indri::parse::CharClass::letter },
  { 0x0F86, 0x0F87, indri::parse::CharClass::letter },
  { 0x0F90, 0x0F97, indri::parse::CharClass::letter },
  { 0x0F99, 0x0FBC, indri::parse::CharClass::letter },
  { 0x0FC6, 0x0FC6, indri::parse::CharClass::letter },
  { 0xA800, 0xA801, indri::parse::CharClass::letter },
  { 0xA803, 0xA805, indri::parse::CharClass::letter },
  { 0xA807, 0xA80A, indri::parse::CharClass::letter },
  { 0xA80C, 0xA822, indri::parse::CharClass::letter },
  { 0x1000, 0x1021, indri::parse::CharClass::letter },
  { 0x1023, 0x1027, indri::parse::CharClass::letter },
  { 0x1029, 0x102A, indri::parse::CharClass::letter },
  { 0x1050, 0x1055, indri::parse::CharClass::letter },
  { 0x20D0, 0x20EB, indri::parse::CharClass::letter },
  { 0x00AA, 0x00AA, indri::parse::CharClass::letter },
  { 0x00B5, 0x00B5, indri::parse::CharClass::letter },
  { 0x00BA, 0x00BA, indri::parse::CharClass::letter },
  { 0x00C0, 0x00D6, indri::parse::CharClass::letter },
  { 0x00D8, 0x00F6, indri::parse::CharClass::letter },
  { 0x00F8, 0x0241, indri::parse::CharClass::letter },
  { 0x2071, 0x2071, indri::parse::CharClass::letter },
  { 0x207F, 0x207F, indri::parse::CharClass::letter },
  { 0x2090, 0x2094, indri::parse::CharClass::letter },
  { 0xFB00, 0xFB06, indri::parse::CharClass::letter },
  { 0xFF21, 0xFF3A, indri::parse::CharClass::letter },
  { 0xFF41, 0xFF5A, indri::parse::CharClass::letter },
  { 0x0711, 0x0711, indri::parse::CharClass::letter },
  { 0x0730, 0x074A, indri::parse::CharClass::letter },
  { 0x074D, 0x074F, indri::parse::CharClass::letter },
  { 0x10330, 0x10349, indri::parse::CharClass::letter },
  { 0x10000, 0x1000B, indri::parse::CharClass::letter },
  { 0x1000D, 0x10026, indri::parse::CharClass::letter },
  { 0x10028, 0x1003A, indri::parse::CharClass::letter },
  { 0x1003C, 0x1003D, indri::parse::CharClass::letter },
  { 0x1003F, 0x1004D, indri::parse::CharClass::letter },
  { 0x10050, 0x1005D, indri::parse::CharClass::letter },
  { 0x10080, 0x100FA, indri::parse::CharClass::letter },
  { 0x1D400, 0x1D454, indri::parse::CharClass::letter },
  { 0x1D456, 0x1D49C, indri::parse::CharClass::letter },
  { 0x1D49E, 0x1D49F, indri::parse::CharClass::letter },
  { 0x1D4A2, 0x1D4A2, indri::parse::CharClass::letter },
  { 0x1D4A5, 0x1D4A6, indri::parse::CharClass::letter },
  { 0x1D4A9, 0x1D4AC, indri::parse::CharClass::letter },
  { 0x1D4AE, 0x1D4B9, indri::parse::CharClass::letter },
  { 0x1D4BB, 0x1D4BB, indri::parse::CharClass::letter },
  { 0x1D4BD, 0x1D4C3, indri::parse::CharClass::letter },
  { 0x1D4C5, 0x1D505, indri::parse::CharClass::letter },
  { 0x1D507, 0x1D50A, indri::parse::CharClass::letter },
  { 0x1D50D, 0x1D514, indri::parse::CharClass::letter },
  { 0x1D516, 0x1D51C, indri::parse::CharClass::letter },
  { 0x1D51E, 0x1D539, indri::parse::CharClass::letter },
  { 0x1D53B, 0x1D53E, indri::parse::CharClass::letter },
  { 0x1D540, 0x1D544, indri::parse::CharClass::letter },
  { 0x1D546, 0x1D546, indri::parse::CharClass::letter },
  { 0x1D54A, 0x1D550, indri::parse::CharClass::letter },
  { 0x1D552, 0x1D6A5, indri::parse::CharClass::letter },
  { 0x1D6A8, 0x1D6C0, indri::parse::CharClass::letter },
  { 0x1D6C2, 0x1D6DA, indri::parse::CharClass::letter },
  { 0x1D6DC, 0x1D6FA, indri::parse::CharClass::letter },
  { 0x1D6FC, 0x1D714, indri::parse::CharClass::letter },
  { 0x1D716, 0x1D734, indri::parse::CharClass::letter },
  { 0x1D736, 0x1D74E, indri::parse::CharClass::letter },
  { 0x1D750, 0x1D76E, indri::parse::CharClass::letter },
  { 0x1D770, 0x1D788, indri::parse::CharClass::letter },
  { 0x1D78A, 0x1D7A8, indri::parse::CharClass::letter },
  { 0x1D7AA, 0x1D7C2, indri::parse::CharClass::letter },
  { 0x1D7C4, 0x1D7C9, indri::parse::CharClass::letter },
  { 0x10A00, 0x10A00, indri::parse::CharClass::letter },
  { 0x10A10, 0x10A13, indri::parse::CharClass::letter },
  { 0x10A15, 0x10A17, indri::parse::CharClass::letter },
  { 0x10A19, 0x10A33, indri::parse::CharClass::letter },
  { 0x10A01, 0x10A03, indri::parse::CharClass::letter },
  { 0x10A05, 0x10A06, indri::parse::CharClass::letter },
  { 0x10A0C, 0x10A0F, indri::parse::CharClass::letter },
  { 0x10A38, 0x10A3B, indri::parse::CharClass::letter },
  { 0x10A3F, 0x10A47, indri::parse::CharClass::letter },
  { 0x0D82, 0x0D83, indri::parse::CharClass::letter },
  { 0x0DCA, 0x0DCA, indri::parse::CharClass::letter },
  { 0x0DCF, 0x0DD4, indri::parse::CharClass::letter },
  { 0x0DD6, 0x0DD6, indri::parse::CharClass::letter },
  { 0x0DD8, 0x0DDF, indri::parse::CharClass::letter },
  { 0x0DF2, 0x0DF3, indri::parse::CharClass::letter },
  { 0x2102, 0x2102, indri::parse::CharClass::letter },
  { 0x2107, 0x2107, indri::parse::CharClass::letter },
  { 0x210A, 0x2113, indri::parse::CharClass::letter },
  { 0x2115, 0x2115, indri::parse::CharClass::letter },
  { 0x2119, 0x211D, indri::parse::CharClass::letter },
  { 0x2124, 0x2124, indri::parse::CharClass::letter },
  { 0x2126, 0x2126, indri::parse::CharClass::letter },
  { 0x2128, 0x2128, indri::parse::CharClass::letter },
  { 0x212A, 0x212D, indri::parse::CharClass::letter },
  { 0x212F, 0x2131, indri::parse::CharClass::letter },
  { 0x2133, 0x2139, indri::parse::CharClass::letter },
  { 0x213C, 0x213F, indri::parse::CharClass::letter },
  { 0x2145, 0x2149, indri::parse::CharClass::letter },
  { 0x1720, 0x1731, indri::parse::CharClass::letter },
  { 0x2C80, 0x2CE4, indri::parse::CharClass::letter },
  { 0x1740, 0x1751, indri::parse::CharClass::letter },
  { 0x2C00, 0x2C2E, indri::parse::CharClass::letter },
  { 0x2C30, 0x2C5E, indri::parse::CharClass::letter },
  { 0x0A01, 0x0A03, indri::parse::CharClass::letter },
  { 0x0A3C, 0x0A3C, indri::parse::CharClass::letter },
  { 0x0A3E, 0x0A42, indri::parse::CharClass::letter },
  { 0x0A47, 0x0A48, indri::parse::CharClass::letter },
  { 0x0A4B, 0x0A4D, indri::parse::CharClass::letter },
  { 0x0A70, 0x0A71, indri::parse::CharClass::letter },
  { 0x13A0, 0x13F4, indri::parse::CharClass::letter },
  { 0x10450, 0x1047F, indri::parse::CharClass::letter },
  { 0x0B01, 0x0B03, indri::parse::CharClass::letter },
  { 0x0B3C, 0x0B3C, indri::parse::CharClass::letter },
  { 0x0B3E, 0x0B43, indri::parse::CharClass::letter },
  { 0x0B47, 0x0B48, indri::parse::CharClass::letter },
  { 0x0B4B, 0x0B4D, indri::parse::CharClass::letter },
  { 0x0B56, 0x0B57, indri::parse::CharClass::letter },
  { 0x0C85, 0x0C8C, indri::parse::CharClass::letter },
  { 0x0C8E, 0x0C90, indri::parse::CharClass::letter },
  { 0x0C92, 0x0CA8, indri::parse::CharClass::letter },
  { 0x0CAA, 0x0CB3, indri::parse::CharClass::letter },
  { 0x0CB5, 0x0CB9, indri::parse::CharClass::letter },
  { 0x0CBD, 0x0CBD, indri::parse::CharClass::letter },
  { 0x0CDE, 0x0CDE, indri::parse::CharClass::letter },
  { 0x0CE0, 0x0CE1, indri::parse::CharClass::letter },
  { 0x103A0, 0x103C3, indri::parse::CharClass::letter },
  { 0x103C8, 0x103D0, indri::parse::CharClass::letter },
  { 0x135F, 0x135F, indri::parse::CharClass::letter },
  { 0x0D02, 0x0D03, indri::parse::CharClass::letter },
  { 0x0D3E, 0x0D43, indri::parse::CharClass::letter },
  { 0x0D46, 0x0D48, indri::parse::CharClass::letter },
  { 0x0D4A, 0x0D4D, indri::parse::CharClass::letter },
  { 0x0D57, 0x0D57, indri::parse::CharClass::letter },
  { 0x0C05, 0x0C0C, indri::parse::CharClass::letter },
  { 0x0C0E, 0x0C10, indri::parse::CharClass::letter },
  { 0x0C12, 0x0C28, indri::parse::CharClass::letter },
  { 0x0C2A, 0x0C33, indri::parse::CharClass::letter },
  { 0x0C35, 0x0C39, indri::parse::CharClass::letter },
  { 0x0C60, 0x0C61, indri::parse::CharClass::letter },
  { 0x1700, 0x170C, indri::parse::CharClass::letter },
  { 0x170E, 0x1711, indri::parse::CharClass::letter },
  { 0x0400, 0x0481, indri::parse::CharClass::letter },
  { 0x048A, 0x04CE, indri::parse::CharClass::letter },
  { 0x04D0, 0x04F9, indri::parse::CharClass::letter },
  { 0x0500, 0x050F, indri::parse::CharClass::letter },
  { 0x1980, 0x19A9, indri::parse::CharClass::letter },
  { 0x19C1, 0x19C7, indri::parse::CharClass::letter },
  { 0x102C, 0x1032, indri::parse::CharClass::letter },
  { 0x1036, 0x1039, indri::parse::CharClass::letter },
  { 0x1056, 0x1059, indri::parse::CharClass::letter },
  { 0xA802, 0xA802, indri::parse::CharClass::letter },
  { 0xA806, 0xA806, indri::parse::CharClass::letter },
  { 0xA80B, 0xA80B, indri::parse::CharClass::letter },
  { 0xA823, 0xA827, indri::parse::CharClass::letter },
  { 0x1A17, 0x1A1B, indri::parse::CharClass::letter },
  { 0x0041, 0x005A, indri::parse::CharClass::letter },
  { 0x0061, 0x007A, indri::parse::CharClass::letter },
  { 0x16A0, 0x16EA, indri::parse::CharClass::letter },

  { 0x1D100, 0x1D126, indri::parse::CharClass::symbol },
  { 0x1D12A, 0x1D1DD, indri::parse::CharClass::symbol },
  { 0x1D300, 0x1D356, indri::parse::CharClass::symbol },
  { 0x10179, 0x10189, indri::parse::CharClass::symbol },
  { 0x002B, 0x002B, indri::parse::CharClass::symbol },
  { 0x003C, 0x003E, indri::parse::CharClass::symbol },
  { 0x007C, 0x007C, indri::parse::CharClass::symbol },
  { 0x1FED, 0x1FEF, indri::parse::CharClass::symbol },
  { 0x1940, 0x1940, indri::parse::CharClass::symbol },
  { 0x09FA, 0x09FA, indri::parse::CharClass::symbol },
  { 0x0B70, 0x0B70, indri::parse::CharClass::symbol },
  { 0x2CE5, 0x2CEA, indri::parse::CharClass::symbol },
  { 0x10102, 0x10102, indri::parse::CharClass::symbol },
  { 0x10137, 0x1013F, indri::parse::CharClass::symbol },
  { 0x1D6C1, 0x1D6C1, indri::parse::CharClass::symbol },
  { 0x1D6DB, 0x1D6DB, indri::parse::CharClass::symbol },
  { 0x1D6FB, 0x1D6FB, indri::parse::CharClass::symbol },
  { 0x1D715, 0x1D715, indri::parse::CharClass::symbol },
  { 0x1D735, 0x1D735, indri::parse::CharClass::symbol },
  { 0x1D74F, 0x1D74F, indri::parse::CharClass::symbol },
  { 0x1D76F, 0x1D76F, indri::parse::CharClass::symbol },
  { 0x1D789, 0x1D789, indri::parse::CharClass::symbol },
  { 0x1D7A9, 0x1D7A9, indri::parse::CharClass::symbol },
  { 0x1D7C3, 0x1D7C3, indri::parse::CharClass::symbol },
  { 0xA828, 0xA82B, indri::parse::CharClass::symbol },
  { 0x060E, 0x060F, indri::parse::CharClass::symbol },
  { 0x06E9, 0x06E9, indri::parse::CharClass::symbol },
  { 0x06FD, 0x06FE, indri::parse::CharClass::symbol },
  { 0xFDFD, 0xFDFD, indri::parse::CharClass::symbol },
  { 0xFE62, 0xFE62, indri::parse::CharClass::symbol },
  { 0xFE64, 0xFE66, indri::parse::CharClass::symbol },
  { 0x2016, 0x2017, indri::parse::CharClass::symbol },
  { 0x2020, 0x2025, indri::parse::CharClass::symbol },
  { 0x2032, 0x2038, indri::parse::CharClass::symbol },
  { 0x203B, 0x203B, indri::parse::CharClass::symbol },
  { 0x2041, 0x2043, indri::parse::CharClass::symbol },
  { 0x204A, 0x204E, indri::parse::CharClass::symbol },
  { 0x2050, 0x2052, indri::parse::CharClass::symbol },
  { 0x2055, 0x205E, indri::parse::CharClass::symbol },
  { 0x207A, 0x207C, indri::parse::CharClass::symbol },
  { 0x208A, 0x208C, indri::parse::CharClass::symbol },
  { 0x2100, 0x2101, indri::parse::CharClass::symbol },
  { 0x2103, 0x2106, indri::parse::CharClass::symbol },
  { 0x2108, 0x2109, indri::parse::CharClass::symbol },
  { 0x2114, 0x2114, indri::parse::CharClass::symbol },
  { 0x2116, 0x2118, indri::parse::CharClass::symbol },
  { 0x211E, 0x2123, indri::parse::CharClass::symbol },
  { 0x2125, 0x2125, indri::parse::CharClass::symbol },
  { 0x2127, 0x2127, indri::parse::CharClass::symbol },
  { 0x2129, 0x2129, indri::parse::CharClass::symbol },
  { 0x212E, 0x212E, indri::parse::CharClass::symbol },
  { 0x2132, 0x2132, indri::parse::CharClass::symbol },
  { 0x213A, 0x213B, indri::parse::CharClass::symbol },
  { 0x2140, 0x2144, indri::parse::CharClass::symbol },
  { 0x214A, 0x214C, indri::parse::CharClass::symbol },
  { 0x2190, 0x23DB, indri::parse::CharClass::symbol },
  { 0x2400, 0x2426, indri::parse::CharClass::symbol },
  { 0x2440, 0x244A, indri::parse::CharClass::symbol },
  { 0x249C, 0x24E9, indri::parse::CharClass::symbol },
  { 0x2500, 0x269C, indri::parse::CharClass::symbol },
  { 0x26A0, 0x26B1, indri::parse::CharClass::symbol },
  { 0x2701, 0x2704, indri::parse::CharClass::symbol },
  { 0x2706, 0x2709, indri::parse::CharClass::symbol },
  { 0x270C, 0x2727, indri::parse::CharClass::symbol },
  { 0x2729, 0x274B, indri::parse::CharClass::symbol },
  { 0x274D, 0x274D, indri::parse::CharClass::symbol },
  { 0x274F, 0x2752, indri::parse::CharClass::symbol },
  { 0x2756, 0x2756, indri::parse::CharClass::symbol },
  { 0x2758, 0x275E, indri::parse::CharClass::symbol },
  { 0x2761, 0x2767, indri::parse::CharClass::symbol },
  { 0x2794, 0x2794, indri::parse::CharClass::symbol },
  { 0x2798, 0x27AF, indri::parse::CharClass::symbol },
  { 0x27B1, 0x27BE, indri::parse::CharClass::symbol },
  { 0x27C0, 0x27C4, indri::parse::CharClass::symbol },
  { 0x27D0, 0x27E5, indri::parse::CharClass::symbol },
  { 0x27F0, 0x2982, indri::parse::CharClass::symbol },
  { 0x2999, 0x29D7, indri::parse::CharClass::symbol },
  { 0x29DC, 0x29FB, indri::parse::CharClass::symbol },
  { 0x29FE, 0x2B13, indri::parse::CharClass::symbol },
  { 0xFF0B, 0xFF0B, indri::parse::CharClass::symbol },
  { 0xFF1C, 0xFF1E, indri::parse::CharClass::symbol },
  { 0x2E80, 0x2E99, indri::parse::CharClass::symbol },
  { 0x2E9B, 0x2EF3, indri::parse::CharClass::symbol },
  { 0x2F00, 0x2FD5, indri::parse::CharClass::symbol },
  { 0x2FF0, 0x2FFB, indri::parse::CharClass::symbol },
  { 0x3004, 0x3004, indri::parse::CharClass::symbol },
  { 0x3012, 0x3013, indri::parse::CharClass::symbol },
  { 0x3020, 0x3020, indri::parse::CharClass::symbol },
  { 0x3036, 0x3037, indri::parse::CharClass::symbol },
  { 0x303E, 0x303F, indri::parse::CharClass::symbol },
  { 0x309B, 0x309C, indri::parse::CharClass::symbol },
  { 0x3200, 0x321E, indri::parse::CharClass::symbol },
  { 0x322A, 0x3243, indri::parse::CharClass::symbol },
  { 0x32C0, 0x32FE, indri::parse::CharClass::symbol },
  { 0x3300, 0x33FF, indri::parse::CharClass::symbol },
  { 0xA700, 0xA716, indri::parse::CharClass::symbol },
  { 0xFFE2, 0xFFE4, indri::parse::CharClass::symbol },
  { 0xFFE8, 0xFFEE, indri::parse::CharClass::symbol },
  { 0xFFFC, 0xFFFD, indri::parse::CharClass::symbol },
  { 0x1360, 0x1360, indri::parse::CharClass::symbol },
  { 0x1390, 0x1399, indri::parse::CharClass::symbol },
  { 0x0F01, 0x0F03, indri::parse::CharClass::symbol },
  { 0x0F13, 0x0F17, indri::parse::CharClass::symbol },
  { 0x0F1A, 0x0F1F, indri::parse::CharClass::symbol },
  { 0x0F34, 0x0F34, indri::parse::CharClass::symbol },
  { 0x0F36, 0x0F36, indri::parse::CharClass::symbol },
  { 0x0F38, 0x0F38, indri::parse::CharClass::symbol },
  { 0x0FBE, 0x0FC5, indri::parse::CharClass::symbol },
  { 0x0FC7, 0x0FCC, indri::parse::CharClass::symbol },
  { 0x0FCF, 0x0FCF, indri::parse::CharClass::symbol },
  { 0x1D200, 0x1D245, indri::parse::CharClass::symbol },
  { 0x4DC0, 0x4DFF, indri::parse::CharClass::symbol },
  { 0xFB29, 0xFB29, indri::parse::CharClass::symbol },
  { 0xA490, 0xA4C6, indri::parse::CharClass::symbol },
  { 0x00A6, 0x00A7, indri::parse::CharClass::symbol },
  { 0x00A9, 0x00A9, indri::parse::CharClass::symbol },
  { 0x00AC, 0x00AC, indri::parse::CharClass::symbol },
  { 0x00AE, 0x00AE, indri::parse::CharClass::symbol },
  { 0x00B0, 0x00B1, indri::parse::CharClass::symbol },
  { 0x00B6, 0x00B6, indri::parse::CharClass::symbol },
  { 0x00D7, 0x00D7, indri::parse::CharClass::symbol },
  { 0x00F7, 0x00F7, indri::parse::CharClass::symbol },
  { 0x0482, 0x0482, indri::parse::CharClass::symbol },
  { 0x1D000, 0x1D0F5, indri::parse::CharClass::symbol },
  { 0x3190, 0x3191, indri::parse::CharClass::symbol },
  { 0x3196, 0x319F, indri::parse::CharClass::symbol },
  { 0x31C0, 0x31CF, indri::parse::CharClass::symbol },
  { 0x3260, 0x327F, indri::parse::CharClass::symbol },
  { 0x328A, 0x32B0, indri::parse::CharClass::symbol },
  { 0x19E0, 0x19FF, indri::parse::CharClass::symbol },
  { 0x03F6, 0x03F6, indri::parse::CharClass::symbol },
  { 0x0BF3, 0x0BF8, indri::parse::CharClass::symbol },
  { 0x0BFA, 0x0BFA, indri::parse::CharClass::symbol },
  { 0x3250, 0x3250, indri::parse::CharClass::symbol },

  { 0x09F2, 0x09F3, indri::parse::CharClass::currency },
  { 0x060B, 0x060B, indri::parse::CharClass::currency },
  { 0xFDFC, 0xFDFC, indri::parse::CharClass::currency },
  { 0xFE69, 0xFE69, indri::parse::CharClass::currency },
  { 0xFFE0, 0xFFE1, indri::parse::CharClass::currency },
  { 0xFFE5, 0xFFE6, indri::parse::CharClass::currency },
  { 0x17DB, 0x17DB, indri::parse::CharClass::currency },
  { 0x0E3F, 0x0E3F, indri::parse::CharClass::currency },
  { 0x0AF1, 0x0AF1, indri::parse::CharClass::currency },
  { 0x0BF9, 0x0BF9, indri::parse::CharClass::currency },
  { 0x0024, 0x0024, indri::parse::CharClass::currency },
  { 0x20A0, 0x20B5, indri::parse::CharClass::currency },
  { 0xFF04, 0xFF04, indri::parse::CharClass::currency },
  { 0x00A2, 0x00A5, indri::parse::CharClass::currency },

  { 0x0000, 0x001F, indri::parse::CharClass::control },
  { 0x007F, 0x007F, indri::parse::CharClass::control },
  { 0x070F, 0x070F, indri::parse::CharClass::control },
  { 0x200B, 0x200F, indri::parse::CharClass::control },
  { 0x2027, 0x2027, indri::parse::CharClass::control },
  { 0x202A, 0x202E, indri::parse::CharClass::control },
  { 0x2060, 0x2063, indri::parse::CharClass::control },
  { 0x206A, 0x206F, indri::parse::CharClass::control },
  { 0xFEFF, 0xFEFF, indri::parse::CharClass::control },
  { 0xFFF9, 0xFFFB, indri::parse::CharClass::control },
  { 0xE0001, 0xE0001, indri::parse::CharClass::control },
  { 0xE0020, 0xE007F, indri::parse::CharClass::control },
  { 0xE0100, 0xE01EF, indri::parse::CharClass::control },
  { 0x0080, 0x009F, indri::parse::CharClass::control },
  { 0x00AD, 0x00AD, indri::parse::CharClass::control },
  { 0x0600, 0x0603, indri::parse::CharClass::control },
  { 0x06DD, 0x06DD, indri::parse::CharClass::control },
  { 0x17B4, 0x17B5, indri::parse::CharClass::control },

  { 0x066A, 0x066A, indri::parse::CharClass::percent },
  { 0x2030, 0x2031, indri::parse::CharClass::percent },
  { 0xFF05, 0xFF05, indri::parse::CharClass::percent },
  { 0x0025, 0x0025, indri::parse::CharClass::percent },

  { 0x055A, 0x055A, indri::parse::CharClass::apostrophe },
  { 0x0027, 0x0027, indri::parse::CharClass::apostrophe },
  { 0x2019, 0x2019, indri::parse::CharClass::apostrophe },
  { 0xFF07, 0xFF07, indri::parse::CharClass::apostrophe },
  { 0, 0, 0 }
};

void indri::parse::UTF8Transcoder::_initHT() 
{        
  if ( u.size() == 0 ) {
    for( unsigned int i=0; intervals[i].start; i++ ) {
      store_interval( u, intervals[i].start, intervals[i].end, 
                      intervals[i].cls );
    }
  }
}
