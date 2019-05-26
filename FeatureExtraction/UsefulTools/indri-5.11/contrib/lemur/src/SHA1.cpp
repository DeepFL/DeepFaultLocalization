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

#include "SHA1.hpp"
#include "string.h"

lemur::utility::SHA1::SHA1() { 
}

lemur::utility::SHA1::~SHA1() { 
}

void lemur::utility::SHA1::start(SHA1Context *context) {
  context->total[0] = 0;
  context->total[1] = 0;
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0xC3D2E1F0;
}

void lemur::utility::SHA1::process(SHA1Context *context, unsigned char data[64]) {
  unsigned long temp, W[16], A, B, C, D, E;

  GET_ULONG_BE( W[ 0], data,  0 );
  GET_ULONG_BE( W[ 1], data,  4 );
  GET_ULONG_BE( W[ 2], data,  8 );
  GET_ULONG_BE( W[ 3], data, 12 );
  GET_ULONG_BE( W[ 4], data, 16 );
  GET_ULONG_BE( W[ 5], data, 20 );
  GET_ULONG_BE( W[ 6], data, 24 );
  GET_ULONG_BE( W[ 7], data, 28 );
  GET_ULONG_BE( W[ 8], data, 32 );
  GET_ULONG_BE( W[ 9], data, 36 );
  GET_ULONG_BE( W[10], data, 40 );
  GET_ULONG_BE( W[11], data, 44 );
  GET_ULONG_BE( W[12], data, 48 );
  GET_ULONG_BE( W[13], data, 52 );
  GET_ULONG_BE( W[14], data, 56 );
  GET_ULONG_BE( W[15], data, 60 );

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                                        \
(                                                   \
  temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^   \
       W[(t - 14) & 0x0F] ^ W[ t    & 0x0F],        \
  ( W[t & 0x0F] = S(temp,1) )                       \
)

#define P(a,b,c,d,e,x)                              \
{                                                   \
  e += S(a,5) + F(b,c,d) + K + x; b = S(b,30);      \
}

  A = context->state[0];
  B = context->state[1];
  C = context->state[2];
  D = context->state[3];
  E = context->state[4];

#define F(x,y,z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

  P( A, B, C, D, E, W[0]  );
  P( E, A, B, C, D, W[1]  );
  P( D, E, A, B, C, W[2]  );
  P( C, D, E, A, B, W[3]  );
  P( B, C, D, E, A, W[4]  );
  P( A, B, C, D, E, W[5]  );
  P( E, A, B, C, D, W[6]  );
  P( D, E, A, B, C, W[7]  );
  P( C, D, E, A, B, W[8]  );
  P( B, C, D, E, A, W[9]  );
  P( A, B, C, D, E, W[10] );
  P( E, A, B, C, D, W[11] );
  P( D, E, A, B, C, W[12] );
  P( C, D, E, A, B, W[13] );
  P( B, C, D, E, A, W[14] );
  P( A, B, C, D, E, W[15] );
  P( E, A, B, C, D, R(16) );
  P( D, E, A, B, C, R(17) );
  P( C, D, E, A, B, R(18) );
  P( B, C, D, E, A, R(19) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0x6ED9EBA1

  P( A, B, C, D, E, R(20) );
  P( E, A, B, C, D, R(21) );
  P( D, E, A, B, C, R(22) );
  P( C, D, E, A, B, R(23) );
  P( B, C, D, E, A, R(24) );
  P( A, B, C, D, E, R(25) );
  P( E, A, B, C, D, R(26) );
  P( D, E, A, B, C, R(27) );
  P( C, D, E, A, B, R(28) );
  P( B, C, D, E, A, R(29) );
  P( A, B, C, D, E, R(30) );
  P( E, A, B, C, D, R(31) );
  P( D, E, A, B, C, R(32) );
  P( C, D, E, A, B, R(33) );
  P( B, C, D, E, A, R(34) );
  P( A, B, C, D, E, R(35) );
  P( E, A, B, C, D, R(36) );
  P( D, E, A, B, C, R(37) );
  P( C, D, E, A, B, R(38) );
  P( B, C, D, E, A, R(39) );

#undef K
#undef F

#define F(x,y,z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

  P( A, B, C, D, E, R(40) );
  P( E, A, B, C, D, R(41) );
  P( D, E, A, B, C, R(42) );
  P( C, D, E, A, B, R(43) );
  P( B, C, D, E, A, R(44) );
  P( A, B, C, D, E, R(45) );
  P( E, A, B, C, D, R(46) );
  P( D, E, A, B, C, R(47) );
  P( C, D, E, A, B, R(48) );
  P( B, C, D, E, A, R(49) );
  P( A, B, C, D, E, R(50) );
  P( E, A, B, C, D, R(51) );
  P( D, E, A, B, C, R(52) );
  P( C, D, E, A, B, R(53) );
  P( B, C, D, E, A, R(54) );
  P( A, B, C, D, E, R(55) );
  P( E, A, B, C, D, R(56) );
  P( D, E, A, B, C, R(57) );
  P( C, D, E, A, B, R(58) );
  P( B, C, D, E, A, R(59) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6

  P( A, B, C, D, E, R(60) );
  P( E, A, B, C, D, R(61) );
  P( D, E, A, B, C, R(62) );
  P( C, D, E, A, B, R(63) );
  P( B, C, D, E, A, R(64) );
  P( A, B, C, D, E, R(65) );
  P( E, A, B, C, D, R(66) );
  P( D, E, A, B, C, R(67) );
  P( C, D, E, A, B, R(68) );
  P( B, C, D, E, A, R(69) );
  P( A, B, C, D, E, R(70) );
  P( E, A, B, C, D, R(71) );
  P( D, E, A, B, C, R(72) );
  P( C, D, E, A, B, R(73) );
  P( B, C, D, E, A, R(74) );
  P( A, B, C, D, E, R(75) );
  P( E, A, B, C, D, R(76) );
  P( D, E, A, B, C, R(77) );
  P( C, D, E, A, B, R(78) );
  P( B, C, D, E, A, R(79) );

#undef K
#undef F

  context->state[0] += A;
  context->state[1] += B;
  context->state[2] += C;
  context->state[3] += D;
  context->state[4] += E;

#undef S
#undef R
#undef P
}

void lemur::utility::SHA1::update(SHA1Context *context, unsigned char *input, int inputLen) {
  int fill;
  unsigned long left;

  left=context->total[0] & 0x3F;
  fill=64-left;
  context->total[0] += inputLen;
  context->total[0] &= 0xFFFFFFFF;

  if (context->total[0] < (unsigned long)inputLen) {
    ++(context->total[1]);
  }

  if (left && (inputLen >= fill)) {
    memcpy((context->buffer+left), input, fill);
    process(context, (unsigned char *)context->buffer);
    input+=fill;
    inputLen-=fill;
    left=0;
  }
  
  while (inputLen >= 64) {
    process(context, input);
    input+=64;
    inputLen-=64;
  }

  if (inputLen > 0) {
    memcpy((context->buffer+left), input, inputLen);
  }
}

void lemur::utility::SHA1::finish(SHA1Context *context, unsigned char *output) {
  unsigned long last, padn;
  unsigned long high, low;
  unsigned char msglen[8];

  high = ( context->total[0] >> 29 ) | ( context->total[1] <<  3 );
  low  = ( context->total[0] <<  3 );

  PUT_ULONG_BE( high, msglen, 0 );
  PUT_ULONG_BE( low,  msglen, 4 );

  last = context->total[0] & 0x3F;
  padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

  update( context, (unsigned char *) lemur::utility::sha1_padding, padn );
  update( context, msglen, 8 );

  PUT_ULONG_BE( context->state[0], output,  0 );
  PUT_ULONG_BE( context->state[1], output,  4 );
  PUT_ULONG_BE( context->state[2], output,  8 );
  PUT_ULONG_BE( context->state[3], output, 12 );
  PUT_ULONG_BE( context->state[4], output, 16 );
}

char lemur::utility::SHA1::intToHexDigit(char val) {
  if (val > 9) {
    return (val+55);
  }
  return (val+48); 
}

void lemur::utility::SHA1::byteToHexString(unsigned char *input, int inputLen, char *output, int maxOutputLen) {
 if (!input || !output) { return; }
 unsigned char *pos=input;
 char *outPos=output;
 int outputLen=0;
 int currentInputPos=0;
 int maxOutputSize=(maxOutputLen-1);
 while ((currentInputPos < inputLen) && (outputLen < maxOutputSize)) {
   char thisChar=(char)(*pos);
   char topVal=(thisChar >> 4) & (0x0F);
   char botVal=(thisChar & 0x0F);
   (*outPos)=intToHexDigit(topVal);
   ++outPos;
   ++outputLen;
   if (outputLen < maxOutputSize) {
     (*outPos)=intToHexDigit(botVal);
     ++outPos;
     ++outputLen;
   }
   ++pos;
   ++currentInputPos;
  }
  *outPos=0; 
}

void lemur::utility::SHA1::hash(unsigned char *input, int inputLen, unsigned char *output) {
  SHA1Context context;

  memset(&context, 0, sizeof(SHA1Context));

  start(&context);
  update(&context, input, inputLen);
  finish(&context, output);
}

void lemur::utility::SHA1::hashStringToHex(const char *input, char *output, int maxOutputLen) {
  unsigned char outputBuffer[64];
  hash((unsigned char*)input, strlen(input), outputBuffer);
  // output size from hash will always be 20 bytes!
  byteToHexString(outputBuffer, 20, output, maxOutputLen);
}

