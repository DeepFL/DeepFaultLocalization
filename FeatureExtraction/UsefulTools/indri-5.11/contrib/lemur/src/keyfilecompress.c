/*                                                                    */
/* Copyright 2006 by Howard Turtle                               */
/*                                                                    */

#define boolean int
#define true 1
#define false 0

#include "integer_types.h"

/* uncompress_uint uncompresses an unsigned integer        */
/*    in byte array p and returns the number of bytes consumed  */
/*    to decompress it.                                         */


int uncompress_uint1(unsigned *i, unsigned char p[])
{int j=0,shift_cnt=0; boolean done=false;

  *i = 0;
  do {
    *i = *i | ((p[j] & 127)<<shift_cnt);
    if ( (p[j] & 128)!=0 ) {
      shift_cnt = shift_cnt + 7;
      j++;
    }
    else done = true;
  } while ( !done );
  return(j+1);
}

int uncompress_uint(unsigned *i, unsigned char p[])
{int j=0,shift_cnt=0;

  *i = p[j] & 127;
  while ( (p[j] & 128)!=0 ) {
    j++;
    shift_cnt = shift_cnt + 7;
    *i = *i | ((p[j] & 127)<<shift_cnt);
  }
  return(j+1);
}

/* compress_uint compresses an unsigned int i into a byte   */
/*   string and returns the length of the compressed string.    */


int compress_uint(unsigned i, unsigned char p[])
{int j=-1;

  do {
    j++;
    p[j] = (i & 127) | 128;
    i = i>>7;
  } while ( i>0 );
  p[j] = p[j] & 127; /* high bit off */
  return(j+1);
}

/* order preserving compression.  The op_xxx_uint functions compress */
/*   an unsigned int in a manner that preserves order in the sense   */
/*   that if i<j then c(i)<c(j) where c(.) is the byte string that   */
/*   results from compression.  Preserving order is important when,  */
/*   for example, compressing doc_ids that will then be visited      */
/*   in doc_id order.  This version encodes a uint as a lead byte    */
/*   followed by a variable number of low order bytes.  The number   */
/*   low order bytes is encoded in unary in the lead byte and any    */
/*   bits not used for the unary length encoding are part of the     */
/*   encoded int.  All low order bytes are encoded from high order   */
/*   to low order.                                                   */
/*     lead byte  what follows                                       */
/*     1111 1111  0 bits in lead byte plus 8 bytes       2**64       */
/*     1111 1110  0 bits in lead byte plus 7 bytes       2**56       */
/*     1111 110x  1 bit  in lead byte plus 6 bytes       2**49       */
/*     1111 10xx  2 bits in lead byte plus 5 bytes       2**42       */
/*     1111 0xxx  3 bits in lead byte plus 4 bytes       2**35       */
/*     1110 xxxx  4 bits in lead byte plus 3 bytes       2**28       */
/*     110x xxxx  5 bits in lead byte plus 2 bytes       2**21       */
/*     10xx xxxx  6 bits in lead byte plus 1 byte        2**14       */
/*     0xxx xxxx  7 bits in lead byte plus 0 bytes       2**7        */
/*   This code should work for 64 bit uints but has only been tested */
/*   for 32 bit.                                                     */

int op_uncompress_uint(unsigned *i, unsigned char p[])
{int j,mask=128,low_order_bytes=0; unsigned char low_order_mask[9]={127,63,31,15,7,3,1,0,0};

  if ( p[0]==255 ) low_order_bytes = 8;
  else {
    while ( (p[0] & mask) > 0 ) {
      low_order_bytes++;
      mask = mask >> 1;
    }
  }
  /*  printf("low order bytes in uncompress=%d\n",low_order_bytes);*/
  *i = p[0] & low_order_mask[low_order_bytes];
  for (j=1; j<low_order_bytes+1; j++) *i = (*i<<8) + p[j];
  return(low_order_bytes+1);
}

int op_compress_uint(unsigned i, unsigned char p[])
{int j,k,zero_bytes=0,limit=(int)sizeof(unsigned),low_order_bytes;
unsigned mask=(unsigned)255<<(unsigned)((sizeof(unsigned)-1)*8);
 unsigned char ch,unary_mask[9]={0,128,192,224,240,248,252,254,255};

 /* printf("unary mask=");
 for ( j=0; j<9; j++) printf(" %02x",unary_mask[j]);
 printf("\n");*/

  while ( (i & mask)==0 && zero_bytes<(limit-1) ) {
    zero_bytes++;
    mask = mask>>8;
  }
  low_order_bytes = limit - zero_bytes - 1;
  ch = (i & mask) >> (low_order_bytes*8);


  if ( ch >= (128>>low_order_bytes) ) {
    low_order_bytes++;
    ch = 0;
  }

  k = i;
  for ( j=low_order_bytes; j>0; j--) {
    p[j] = k & 255;
    k = k >> 8;
  }
  p[0] = ch | unary_mask[low_order_bytes];

  /*  printf("compressed string for %d =",i);
  for (k=0; k<low_order_bytes+1; k++) printf(" %02x",p[k]);
  printf("\n");*/

  return(low_order_bytes+1);
}

/* uncompress_UINT32 uncompresses a UINT32 compressed in      */
/*    array p and returns the number of bytes consumed to       */
/*    decompress the int.                                       */


int uncompress_UINT32(UINT32 *i, unsigned char p[])
{int j=0,shift_cnt=0;

  *i = p[j] & 127;
  while ( (p[j] & 128)!=0 ) {
    j++;
    shift_cnt = shift_cnt + 7;
    *i = *i | ((p[j] & 127)<<shift_cnt);
  }
  return(j+1);
}

/* compress_UINT32 compresses UINT32 i into a byte string and  */
/*   returns the length of the compressed string.              */


int compress_UINT32(UINT32 i, unsigned char p[])
{
  return( compress_uint( (unsigned) i, p) );
}

/* uncompress_int uncompresses a signed int compressed in    */
/*    array p and returns the number of bytes consumed to       */
/*    decompress the int.                                       */


int uncompress_int(int *i, unsigned char p[])
{int lc,negative; unsigned val;

  lc = uncompress_uint( &val,p);
  negative = val & 1;
  val = val >>1;
  if ( negative ) *i = -val;
  else *i = val;
  return(lc);
}

/* compress_int compresses signed int i into a byte string  */
/*   and returns the length of the compressed string.  It      */
/*   assumes that a signed int fits into one bit less than an  */
/*   unsigned.  A sign bit is encoded as the low order bit of  */
/*   an unsigned int           */


int compress_int(int i, unsigned char p[])
{unsigned val,negative=0;

  if ( i<0 ) {
    negative = 1;
    val = - i;
  }
  else val = i;
  val = val << 1;
  val = val | negative;
  return( compress_uint(val,p) );
}
