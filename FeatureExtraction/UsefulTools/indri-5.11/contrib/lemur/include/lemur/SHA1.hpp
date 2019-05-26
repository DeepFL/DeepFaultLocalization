/*==========================================================================
 * Copyright (c) 2004-2008 Carnegie Mellon University and University of
 * Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/

#ifndef _SHA1_HPP
#define _SHA1_HPP

namespace lemur {
  namespace utility {

  /**
   * SHA1 hashing classes - derived from work done by
   * Christophe Devine ( http://xyssl.org/ )
   */

  /*
   *  32-bit integer manipulation macros (big endian)
   */
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
  {                                                     \
    (n) = ( (unsigned long) (b)[(i)    ] << 24 )        \
          | ( (unsigned long) (b)[(i) + 1] << 16 )      \
          | ( (unsigned long) (b)[(i) + 2] <<  8 )      \
          | ( (unsigned long) (b)[(i) + 3]       );     \
  }
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                             \
  {                                                     \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
  }
#endif

    static const unsigned char sha1_padding[64] = {
      0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    class SHA1 {
    private:

      typedef struct {
        unsigned long total[2];
        unsigned long state[5];
        unsigned long buffer[64];
        unsigned long innerPad[64];
        unsigned long outerPad[64];
      } SHA1Context;

      void start(SHA1Context *context);
      void process(SHA1Context *context, unsigned char data[64]);
      void update(SHA1Context *context, unsigned char *input, int inputLen);
      void finish(SHA1Context *context, unsigned char *output);

      char intToHexDigit(char val);
      void byteToHexString(unsigned char *input, int inputLen, char *output, int maxOutputLen);

    public:
      SHA1();
      ~SHA1();

      void hash(unsigned char *input, int inputLen, unsigned char *output);
      void hashStringToHex(const char *input, char *output, int maxOutputLen);

    }; // end class SHA1

  } // end namespace utility
} // end namespace lemur

#endif // _SHA1_HPP
