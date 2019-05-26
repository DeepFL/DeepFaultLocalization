/*                                                               */
/* Copyright 1984,1985,1986,1988,1989,1990,2003,2004,2005,2006,  */
/*   2007 by Howard Turtle                                       */

#define boolean int
#define true 1
#define false 0

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "keyerr.h"
#include "keydef.h"
#include "keyprint.h"
#include "keyfile.h"
/* platform specific file io bits */
#include "fileio.h"

#define log_errors true
#define trace_io false


/*#define COMPILE_MULTI_VERSION*/
/*#define log_buffers*/
/*#define VERIFY_IX_COMPRESS*/
/*#define VERIFY_WRITES*/
/*#define VERIFY_SHUFFLES*/
/*#define TRACE_SHUFFLE_BLOCKS*/
/*#define TRACE_LOCKS*/

/*int fn_log_f(char * fi_fmt, ...);*/
#define fn_log_f printf

#ifdef log_buffers
FILE
*buffer_log;
#endif

struct shuffle_candidate {
  int
    lt_move_cnt,
    rt_move_cnt;
  unsigned
    lt_lc,
    lt_prefix_lc,
    mid_lc,
    mid_prefix_lc,
    rt_lc,
    rt_prefix_lc;
};

static int
  read_cnt=0,
  write_cnt=0,
  shuffle_cnt,
  shuffle_lt_zero_cnt=0,
  shuffle_rt_zero_cnt=0;

static level0_pntr
  null0_ptr = {max_segment,0,0},
  dummy_ptr = {0,0,0};
static struct leveln_pntr
  nulln_ptr = {max_segment,0};
static int
  power_of_two[20];

/*static int allocate_block();*/
static boolean allocate_rec();
static void replace_max_key();
static void deallocate_rec();
static void check_ix_block_compression();
static void update_index();
static void index_delete();
static void split_block();
static FILE *file_index();


/* Print procedures */

static void print_index_type(FILE *list, int index_type)
{
  if (index_type==user_ix) fprintf(list,"user");
  else if (index_type==free_lc_ix) fprintf(list,"free_lc");
  else if (index_type==free_rec_ix) fprintf(list,"free_rec");
  else fprintf(list,"unknown");
}

/*                                                              */
/* Compression                                                  */
/*                                                              */

/* compressed_int_lc returns the length of a compressed integer */
/*   without actually uncompressing it.  Note that it doesn't   */
/*   care what the size of the uncompressed int would be.       */

static int compressed_int_lc(unsigned char buf[])
{int i=0;

  while ( (buf[i] & 128)!=0 ) i++;
  return(i+1);
}

/* uncompress_key_lc uncompresses the lc entry in a key in an   */
/*   index block.  Since the lc is much less that 2**14 we know */
/*   that the compressed lc must be in one or two bytes.        */

static int uncompress_key_lc(UINT16 *key_lc, unsigned char p[])
{
  if ( p[0]<128 ) {
    *key_lc = p[0];
    return(1);
  }
  else {
    *key_lc = (p[0] & 127) * 128 + p[1];
    return(2);
  }
}

/* uncompress_UINT16 uncompresses an integer compressed in      */
/*    array p and returns the number of bytes consumed to       */
/*    decompress the int.                                       */


static int uncompress_UINT16(UINT16 *i, unsigned char p[])
{int j=0; boolean done=false;

  if ( p[0]<128 ) {
    *i = p[0];
    return(1);
  }
  else {
    *i = 0;
    do {
      *i = *i | (p[j] & 127);
      if ( (p[j] & 128)!=0 ) {
        *i = *i << 7;
        j++;
      }
      else done = true;
    } while ( !done );
    return(j+1);
  }
}

/* uncompress_UINT32 uncompresses an integer compressed in      */
/*    array p and returns the number of bytes consumed to       */
/*    decompress the int.                                       */

static int uncompress_UINT32(UINT32 *i, unsigned char p[])
{int j=0; boolean done=false;

  *i = 0;
  do {
    *i = *i | (p[j] & 127);
    if ( (p[j] & 128)!=0 ) {
      *i = *i << 7;
      j++;
    }
    else done = true;
  } while ( !done );
  return(j+1);
}

/* uncompress_UINT64 uncompresses an integer compressed in      */
/*    array p and returns the number of bytes consumed to       */
/*    decompress the int.                                       */

static int uncompress_UINT64(UINT64 *i, unsigned char p[])
{int j=0; boolean done=false;

  *i = 0;
  do {
    *i = *i | (p[j] & 127);
    if ( (p[j] & 128)!=0 ) {
      *i = *i << 7;
      j++;
    }
    else done = true;
  } while ( !done );
  return(j+1);
}

/* compress_UINT32 compresses UINT32 i into a byte string and     */
/*   returns the length of the compressed string. Note that ptr   */
/*   points to the rightmost character in the compressed string,  */
/*   i.e. the int is compressed from ptr to the left.             */


static int compress_UINT32(UINT32 i, unsigned char *ptr)
{unsigned char *p;

  p = ptr;
  do {
    *p = (i & 127) | 128;
    p--;
    i = i>>7;
  } while ( i>0 );
  *ptr = *ptr & 127; /* high bit off */
  return((int)(ptr-p));
}

/* compress_UINT64 compresses UINT64 i into a byte string and     */
/*   returns the length of the compressed string. Note that ptr   */
/*   points to the rightmost character in the compressed string,  */
/*   i.e. the int is compressed from ptr to the left.             */


static int compress_UINT64(UINT64 i, unsigned char *ptr)
{unsigned char *p;

  p = ptr;
  do {
    *p = (i & 127) | 128;
    p--;
    i = i>>7;
  } while ( i>0 );
  *ptr = *ptr & 127; /* high bit off */
  return((int)(ptr-p));
}

/* UINT32_lc_if_compressed returns the length that UINT32 i      */
/*   will occupy once compressed.  This function is used to      */
/*   check the compressed size of pointer segments and lengths.  */
/*   Segments are always short and lengths tend to be short so   */
/*   favoring small values of i makes sense.                     */

static int UINT32_lc_if_compressed(UINT32 i)
{
  if      ( i<128    )    return(1);
  else if ( i<16384  )    return(2);
  else if ( i<2097152 )   return(3);
  else if ( i<268435456 ) return(4);
  else return(5);
}

/* UINT64_lc_if_compressed returns the length that UINT64 i      */
/*   will occupy once compressed.                                */

static int UINT64_lc_if_compressed(UINT64 i)
{
  if      ( i<        UINT64_C(34359738368) ) { /* <=5 */
    if      ( i<                UINT64_C(128) ) return(1);
    else if ( i<              UINT64_C(16384) ) return(2);
    else if ( i<            UINT64_C(2097152) ) return(3);
    else if ( i<          UINT64_C(268435456) ) return(4);
    else                                        return(5);
  }
  else { /* >5 */
    if      ( i<      UINT64_C(4398046511104) ) return(6);
    else if ( i<    UINT64_C(562949953421312) ) return(7);
    else if ( i<  UINT64_C(72057594037927936) ) return(8);
    else if ( i<UINT64_C(9223372036854775808) ) return(9);
    else                                        return(10);
  }
}

/* static int UINT64_lc_if_compressed(UINT64 i)
{
  if      ( i<                UINT64_C(128) ) return(1);
  else if ( i<              UINT64_C(16384) ) return(2);
  else if ( i<            UINT64_C(2097152) ) return(3);
  else if ( i<          UINT64_C(268435456) ) return(4);
  else if ( i<        UINT64_C(34359738368) ) return(5);
  else if ( i<      UINT64_C(4398046511104) ) return(6);
  else if ( i<    UINT64_C(562949953421312) ) return(7);
  else if ( i<  UINT64_C(72057594037927936) ) return(8);
  else if ( i<UINT64_C(9223372036854775808) ) return(9);
  else return(10);
}*/

static unsigned allocation_lc(unsigned lc, unsigned unit)
{
  if ( lc==0 ) return(0);
  else return(((lc-1) / unit + 1) * unit);
}

static unsigned rec_allocation_lc(unsigned lc)
{
  if ( lc==0 ) return(0);
  else return(((lc-1) / rec_allocation_unit + 1) * rec_allocation_unit);
}

/* Error handling.  All errors that require logging are       */
/*   signalled using one of the set_error calls.  These, in   */
/*   turn, call set_err which sets the fcb error code, sets   */
/*   file_ok to false if the error is considered fatal, and   */
/*   logs the error using fn_log_f.  "Normal" errors          */
/*   (xx_nokey, ateof, atbof, badkey, longrec, longkey) are   */
/*   not logged but are simply returned in f->error_code and  */
/*   will be reset on the next call.                          */
/* If log_errors==true then f->log file is opened (if         */
/*   necessary) and errors are logged to that file as well.   */
/*   Additional information may be written to the log file by */
/*   the caller (if log_errors is true).  Note that the same  */
/*   log_file is used for capturing trace information.        */

static boolean error_is_fatal(UINT32 error_code)
{boolean fatal=false;

  switch ( error_code ) {
    case  0: break;
    case  1: /* badopen_err */      fatal = true; break;
    case  2: /* badcreate_err */    fatal = true; break;
    case  3: /* smallfcb_err */     fatal = true; break;
    case  4: /* dltnokey_err */                 break;
    case  5: /* getnokey_err */                 break;
    case  6: /* notkeyfil_err */    fatal = true; break;
    case  7: /* filenotok_err */    fatal = true; break;
    case  8: /* badkey_err */                   break;
    case  9: /* maxlevel_err */     fatal = true; break;
    case 10: /* ateof_err */                    break;
    case 11: /* atbof_err */                    break;
    case 12: /* longrec_err */                  break;
    case 13: /* longkey_err */                  break;
    case 14: /* version_err */      fatal = true; break;
    case 15: /* seek_err */         fatal = true; break;
    case 16: /* read_err */         fatal = true; break;
    case 17: /* write_err */        fatal = true; break;
    case 18: /* segment_open_err */ fatal = true; break;
    case 19: /* notused */          fatal = true; break;
    case 20: /* bad_name_err */     fatal = true; break;
    case 21: /* bad_dlt_err */      fatal = true; break;
    case 22: /* max_key_err */      fatal = true; break;
    case 23: /* nospace_err */      fatal = true; break;
    case 24: /* free_insrt_err */   fatal = true; break;
    case 25: /* free_dlt_err */     fatal = true; break;
    case 26: /* alloc_rec_err */    fatal = true; break;
    case 27: /* dealloc_rec_err */  fatal = true; break;
    case 28: /* alloc_buf_err */    fatal = true; break;
    case 29: /* move_rec_err */     fatal = true; break;
    case 30: /* bad_close_err */    fatal = true; break;
    case 31: /* ix_struct_err */    fatal = true; break;
    case 32: /* read_only_err */                  break;
    case 33: /* repl_max_key_err */ fatal = true; break;
    case 34: /* data_lc_err */                    break;
    case 35: /* insert_err */       fatal = true; break;
    case 36: /* ix_compress_err */                break;
    case 37: /* not_supported_err */              break;
    case 38: /* move_keys_err */    fatal = true; break;
    default: /* illegal_err code */ fatal = true; break;
  }
  return(fatal);
}

static void set_err(struct fcb *f, UINT32 err)
{
   f->error_code = err;
   if ( error_is_fatal(err) ) f->file_ok = false;
   if ( log_errors && f->log_file==NULL ) {
     f->log_file = fopen("kf_error_log","wb");
   }
}

static void set_error(struct fcb *f, int err, char caption[])
{
  set_err(f,(UINT32)err);
  fn_log_f("%s\n",caption);
  if ( log_errors ) fprintf(f->log_file,"%s\n",caption);
}

static void set_error1(struct fcb *f, int err, char caption[], int code)
{
  set_err(f,(UINT32)err);
  fn_log_f("%s%d\n",caption,code);
  if ( log_errors ) fprintf(f->log_file,"%s%d\n",caption,code);
}

static void set_error2(struct fcb *f, int err, char caption[], int code1, int code2)
{
  set_err(f,(UINT32)err);
  fn_log_f("%s%d/%d\n",caption,code1,code2);
  if ( log_errors ) fprintf(f->log_file,"%s%d/%d\n",caption,code1,code2);
}

/* Error checking.  Three fields in the fcb are used for error */
/*   management.  f->marker is set to keyf when the file is    */
/*   created and is never changed.  Any fcb with a different   */
/*   value is not useable.  f->file_ok is set true when the    */
/*   file is created and is turned off if an error occurs that */
/*   is so serious that the file is probably damaged (call to  */
/*   error_is_fatal).  f->error_code is set for any error      */
/*   condition.  Some errors are considered transient (e.g.,   */
/*   ateof, atbof, xxnokey,...) and are reset on the next call */
/*   to the package.  All others are considered permanent and  */
/*   are not reset.                                            */

boolean check_fcb(struct fcb *f)
{ boolean ok;

  ok = (f->marker==keyf) && f->file_ok && !error_is_fatal(f->error_code);
  if ( ok ) f->error_code = no_err;
  return(ok);
}

static boolean set_up(struct fcb *f, unsigned char key[], unsigned key_lc, struct key *k)
{
  if ( !check_fcb(f) ) return(false);
  k->lc = key_lc;
  if ( k->lc>0 && k->lc<maxkey_lc ) {
    memcpy(k->text,key,(size_t)key_lc); return(true);
  }
  else {
    f->error_code = badkey_err; k->lc = 0;
    return(false);
  }
}

/* Pointer manipulation */

static boolean gt_n_pntr(struct leveln_pntr p1, struct leveln_pntr p2)
{
  if ( p1.segment<p2.segment ) return(false);
  else if ( p1.segment>p2.segment ) return(true);
  else return( p1.block>p2.block );
}

static int pntr_sc(struct ix_block *b, int ix)
{int sc,lc; UINT16 key_lc;

  sc = b->keys[ix];
  lc = uncompress_key_lc(&key_lc,(unsigned char *)b->keys+sc);
  return(sc+lc+key_lc);
}

/* level0_pntr encoding.  level0_pntrs are encoded as follows.  The  */
/*   first (leftmost) element is the length of the data record (lc). */
/*   If the record is sufficiently short (<=f->data_in_index_lc) it  */
/*   is packed directly into the index block after lc.               */
/*   Longer records will go on disk and will be identified by sc and */
/*   segment.  sc will be encoded following lc, followed by segment  */
/*   (if necessary).  Data recs are allocated in rec_allocation_unit */
/*   chunks so we shift sc right to get rid of unneeded bits and     */
/*   encode whether segment>0 in the low order bit (since most keyed */
/*   files use only one segment).  If the low order bit of sc is one */
/*   then segment is encoded following sc.                           */

/* pack0_ptr packs level0_pntr p into block b.  It checks to see  */
/*   if the data record should be packed directly into the index    */
/*   block (uncompressed, so no byte       */
/*   swapping).  If so, it moves the record and then compresses lc  */
/*   into the block.  Otherwise, it compresses sc, segment, and lc  */
/*   into the block.  Note that insertions go from right to left.   */

int pack0_ptr(struct fcb *f, struct ix_block *b, level0_pntr *p)
{int lc,lc1,lc2=0,return_lc=0; UINT64 esc=0; unsigned char *cp;


  cp = (unsigned char *) b->keys + (keyspace_lc - b->chars_in_use - 1);

  if ( p->lc <= f->data_in_index_lc ) {
    mvc(p->data_rec,0,cp-p->lc+1,0,p->lc);
    lc = compress_UINT32(p->lc,cp-p->lc);
    return_lc = p->lc + lc;
  }
  else {
    esc = (p->sc / rec_allocation_unit) << 1;
    if ( p->segment>0 ) {
      esc = esc | 1;
      lc = compress_UINT32(p->segment,cp);
      lc1 = compress_UINT64(esc,cp-lc);
      lc2 = compress_UINT32(p->lc,cp-lc-lc1);
    }
    else {
      lc = compress_UINT64(esc,cp);
      lc1 = compress_UINT32(p->lc,cp-lc);
    }
    return_lc =  lc + lc1 + lc2;
  }
  return(return_lc);
}

/* unpack0_lc returns the length occupied by the ix_th level0_pntr */
/*   in block b.  It does not return the pointer.                  */

static int unpack0_lc(struct fcb *f, struct ix_block *b, int ix)
{unsigned lc; UINT32 plc; UINT64 esc=0; unsigned char *cp;

  cp = (unsigned char *)b->keys + pntr_sc(b,ix);
  lc = uncompress_UINT32(&plc,cp);
  if ( plc <= f->data_in_index_lc ) {
    lc = lc + plc;
  }
  else {
    lc = lc + uncompress_UINT64(&esc,cp+lc);
    if ( (esc & 1)>0 ) lc = lc + compressed_int_lc(cp+lc);
  }
  return(lc);
}


/* unpack0_ptr uncompresses a level0 pointer.  Note   */
/*   cp points to the first character in the compressed    */
/*   string (unlike compress0_pntr).                       */

int unpack0_ptr(struct fcb *f, struct ix_block *b, int ix, level0_pntr *p)
{int lc; UINT64 esc; unsigned char *cp;

  cp = (unsigned char *)b->keys + pntr_sc(b,ix);
  lc = uncompress_UINT32(&(p->lc),cp);
  if ( p->lc <= f->data_in_index_lc ) {
    mvc(cp+lc,0,p->data_rec,0,p->lc);
    lc = lc + p->lc;
    p->sc = 0;
    p->segment = max_segment;
  }
  else {
    lc = lc + uncompress_UINT64(&esc,cp+lc);
    p->sc = (esc >> 1);
    p->sc = p->sc * rec_allocation_unit;
    if ( (esc & 1)>0 ) lc = lc + uncompress_UINT16(&(p->segment),cp+lc);
    else p->segment = 0;
  }
  return(lc);
}

/* unpack0_ptr_and_rec unpacks the level0_pntr for buf[ix] and extracts the   */
/*   data into rec (up to max_rec_lc bytes).  The number of bytes decoded to  */
/*   get the pointer is returned as the function value.  The number of data   */
/*   bytes in rec is returned in rec_lc.                                      */
/* If max_rec_lc==0 then no data is returned.  If the data rec is on disk     */
/*   it is read and p will have the normal on-disk segment/lc/sc values.  If  */
/*   the data rec is in the index block it is copied directly, segment is set */
/*   to max_segment and sc is set to zero.                                   */
/* If the caller wants to unpack the pointer but not get the rec then rec     */
/*   should be pointed to the data_rec field in p and max_rec_lc set to       */
/*   f->data_in_index_lc                                                      */

static int unpack0_ptr_and_rec(struct fcb *f, buffer_t *buf, int ix, level0_pntr *p,
  unsigned char rec[], unsigned *rec_lc, unsigned max_rec_lc)
{int lc; UINT64 esc; unsigned char *cp; size_t size=0; FILE *file;

  cp = (unsigned char *)(buf->b.keys) + pntr_sc(&(buf->b),ix);
  lc = uncompress_UINT32(&(p->lc),cp);
  *rec_lc = p->lc;
  if ( (unsigned)*rec_lc>max_rec_lc ) *rec_lc = max_rec_lc;
  if ( p->lc > f->data_in_index_lc ) {
    lc = lc + uncompress_UINT64(&esc,cp+lc);
    p->sc = esc >> 1;
    p->sc = p->sc * rec_allocation_unit;
    if ( (esc & 1)>0 ) lc = lc + uncompress_UINT16(&(p->segment),cp+lc);
    else p->segment = 0;

    file = file_index(f,p->segment);
    if ( fseeko(file,(FILE_OFFSET)p->sc,0)!=0 ) f->error_code = seek_err;
    else size = fread(rec,(size_t) 1,(size_t) *rec_lc,file);
    if ( size!=(size_t)*rec_lc ) f->error_code = read_err;
  }
  else {
    p->segment = max_segment;
    p->sc = 0;
    memcpy(rec,cp+lc,(size_t)*rec_lc);
    lc = lc + p->lc;
  }
  return(lc);
}

/* level0_pntr_lc returns the length that will be occupied by      */
/*   level0_pntr p once it is compressed.                          */

static int level0_pntr_lc(struct fcb *f, level0_pntr *p)
{int lc; UINT64 esc;

  if ( p->lc<=f->data_in_index_lc ) lc = UINT32_lc_if_compressed(p->lc) + p->lc;
  else {
    esc = (p->sc / rec_allocation_unit) * 2;
    if ( p->segment==0 ) lc = UINT32_lc_if_compressed(p->lc) + UINT64_lc_if_compressed(esc);
    else  lc = UINT32_lc_if_compressed(p->lc) + UINT64_lc_if_compressed(esc)
      + UINT32_lc_if_compressed(p->segment);
  }
  return(lc);
}

/* leveln_pntr_lc returns the length that leveln_pntr p will */
/*   occupy after compression.                               */

static int leveln_pntr_lc(struct leveln_pntr *p)
{int lc; UINT64 block;

  block = p->block << 1;
  if ( p->segment==0 ) lc =  UINT64_lc_if_compressed( block );
  else lc = UINT64_lc_if_compressed(block) + UINT32_lc_if_compressed(p->segment);
  return(lc);
}

static int levelx_pntr_lc(struct fcb *f, levelx_pntr *p, int level)
{
  if ( level==0 ) return(level0_pntr_lc(f,&(p->p0)));
  else return(leveln_pntr_lc(&(p->pn)));
}

/* packn_ptr compresses leveln pointer p into block b.      */

/*static int packn_ptr(struct ix_block *b, struct leveln_pntr *p)*/
int packn_ptr(struct ix_block *b, struct leveln_pntr *p)
{int lc; unsigned char *cp; UINT64 block;

  cp = (unsigned char *) b->keys + (keyspace_lc - b->chars_in_use - 1);
  block = p->block << 1;
  if ( p->segment==0 ) {
    lc = compress_UINT64(block,cp);
  }
  else {
    lc = compress_UINT32(p->segment,cp);
    block = block | 1;
    lc = lc + compress_UINT64(block,cp-lc);
  }
  return( lc );
}


/* unpackn_lc returns the length occupied by the ix_th     */
/*   pointer in b.  It does not return the pointer.        */

static int unpackn_lc(struct ix_block *b, int ix)
{int lc; unsigned char *cp; UINT64 block;

  cp = (unsigned char *) b->keys + pntr_sc(b,ix);

  lc = uncompress_UINT64(&block,cp);
  if ( (block & 1)>0 ) lc = lc + compressed_int_lc(cp+lc);

  return( lc );
}

/* unpackn_pntr unpacks the ix_th leveln_pntr in block b   */
/*   into p and returns the number of characters occupied  */
/*   by the compressed pointer.                            */

int unpackn_ptr(struct ix_block *b, int ix, struct leveln_pntr *p)
{int lc; unsigned char *cp; UINT64 block;

  cp = (unsigned char *) b->keys + pntr_sc(b,ix);

  lc = uncompress_UINT64(&block,cp);
  p->block = block >> 1;
  if ( (block & 1)>0 ) lc = lc + uncompress_UINT16(&(p->segment),cp+lc);
  else p->segment = 0;

  return( lc );
}

/* nth_pntr_lc returns the length of the ix^th pointer in block b */

static int nth_pntr_lc(struct fcb *f, struct ix_block *b, int ix)
{unsigned lc;

  if ( b->level==0 ) lc = unpack0_lc(f,b,ix);
  else lc = unpackn_lc(b,ix);

  return(lc);
}

/* copy_ptr copies the ix_th pointer in b to the next available */
/*   space in b1 and returns the length copied.  It does not    */
/*   update counts in either b or b1.                           */

static int copy_ptr(struct fcb *f, struct ix_block *b, int ix, struct ix_block *b1)
{unsigned lc;

  lc = nth_pntr_lc(f,b,ix);
  mvc(b->keys,pntr_sc(b,ix),b1->keys,keyspace_lc-b1->chars_in_use-lc,lc);
  return(lc);
}

/* Key handling */

static boolean eq_key(struct key *k1, struct key *k2)
{
  if ( k1->lc!=k2->lc ) return(false);
  else return( memcmp(&(k1->text),&(k2->text),(size_t)k1->lc)==0 );
}

static void copy_key(struct key *from, struct key *to)
{
  to->lc = from->lc;
  mvc(from->text,0,to->text,0,to->lc);
}

/* key_entry_lc returns the number of bytes occupied the the ix^th */
/*   key in b plus the the length of it's compressed lc.           */

static int key_entry_lc(struct ix_block *b, int ix)
{int lc; UINT16 key_lc;

  lc = uncompress_key_lc(&key_lc,(unsigned char *)b->keys+b->keys[ix]);
  return(lc + key_lc);
}

/* get_nth_key returns the nth key from block b.  The length of */
/*   the compressed lc field is returned as the fn value.       */

int get_nth_key(struct ix_block *b, struct key *k, int n)
{int lc=0; UINT16 key_lc; unsigned char *entry_sc;

  if ( n<0 || n>=b->keys_in_block ) k->lc = 0;
  else {
    mvc(b->keys,keyspace_lc-b->prefix_lc,k->text,0,b->prefix_lc);
    entry_sc = (unsigned char *) b->keys + b->keys[n];
    lc = uncompress_key_lc(&key_lc,entry_sc);
    k->lc = key_lc + b->prefix_lc;
    mvc(entry_sc,lc,k->text,b->prefix_lc,key_lc);
  }
  return(lc);
}

/* get_nth_key_and_pntr extracts the nth key and pointer from    */
/*   block b and places them in k and p.  It returns the length  */
/*   of the compressed pointer.                                  */

int get_nth_key_and_pntr(struct fcb *f, struct ix_block *b, struct key *k, int n, levelx_pntr *p)
{int lc;

  get_nth_key(b,k,n);
  lc = k->lc - b->prefix_lc;
  if ( k->lc > 0 ) {
    if ( b->level==0 ) lc = unpack0_ptr(f,b,n,&(p->p0));
    else lc = unpackn_ptr(b,n,&(p->pn));
  }
  return(lc);
}

static void get_max_key(struct ix_block *b, struct key *k)
{
  if ( b->keys_in_block<1 ) k->lc = 0;
  else {
    get_nth_key(b,k,b->keys_in_block-1);
  }
}

/**** I/O ***/


/* init_file_name separates the file name and any extension */
/*   and saves the two parts in the fcb                     */

static void init_file_name(struct fcb *f, char id[])
{int i; unsigned name_lc, f_lc, ext_lc = 0;

  name_lc = (unsigned) strlen(id);
  if (name_lc > max_filename_lc + max_extension_lc)
    set_error(f,bad_name_err,"file name too long");
  i = name_lc - 1;
  /* scan  from right to left
     stop when we hit either a . or a path separator.
  */
  while ( i>=0 && id[i]!='.' && id[i]!=PATH_SEPARATOR) {
    i--;
    ext_lc++;
  }
  if (i >= 0 && id[i] == '.') {
    f_lc = i;
    ext_lc++;
  }
  else {
    f_lc = name_lc;
    ext_lc = 0;
  }
  if (f_lc>=max_filename_lc) set_error(f,bad_name_err,"file name too long");
  else {
    strncpy(f->file_name, id, (size_t)f_lc);
    f->file_name[f_lc] = '\0';
  }
  if ( ext_lc>=max_extension_lc ) set_error(f,bad_name_err,"file extension too long");
  else {
    strncpy(f->file_extension, id + i, (size_t)ext_lc);
    f->file_extension[ext_lc] = '\0';
  }
}

/* build_segment_name builds a segment name by appending the segment */
/*   number to the file name and then appending any extension.       */

static void build_segment_name(struct fcb *f, unsigned segment, char name[])
{int suffix_lc; size_t name_lc;

  strcpy(name,f->file_name);
  if (segment>0) {
    name_lc = strlen(name);
    suffix_lc = sprintf(name+name_lc,"$%d",segment);
    name[name_lc+suffix_lc] = '\0';
  }
  strcat(name,f->file_extension);
}

static void byte_swap_UINT16s(unsigned char s[], int cnt)
{unsigned int i=0; unsigned char ch;

  while ( i<cnt*sizeof(UINT16) ) {
    ch = s[i];
    s[i] = s[i+1];
    s[i+1] = ch;
    i = i + sizeof(UINT16);
  }
}

static void byte_swap_UINT32(unsigned char n[])
{unsigned char ch;

  ch = n[0];
  n[0] = n[3];
  n[3] = ch;
  ch = n[1];
  n[1] = n[2];
  n[2] = ch;
}

static void byte_swap_UINT64(unsigned char n[])
{unsigned char ch;

  ch = n[0];
  n[0] = n[7];
  n[7] = ch;
  ch = n[1];
  n[1] = n[6];
  n[6] = ch;
  ch = n[2];
  n[2] = n[5];
  n[5] = ch;
  ch = n[3];
  n[3] = n[4];
  n[4] = ch;
}

static unsigned char read_byte(struct fcb *f, FILE *file)
{unsigned char ch=0;

  if ( fread(&ch,sizeof(char),(size_t)1,file)!=1 )
    set_error(f,read_err,"read_byte failed");
  return(ch);
}


static UINT16 read_UINT16(struct fcb *f, FILE *file)
{UINT16 n; unsigned char ch;
 unsigned char *p = (unsigned char *)&n;

  if ( fread(&n,sizeof(UINT16),(size_t)1,file)!=1 ) {
    set_error(f,read_err,"read_UINT16 failed");
    return(0);
  }
  if ( f->byte_swapping_required ) {
    ch = p[1];
    p[1] = p[0];
    p[0] = ch;
  }
  return(n);
}


static UINT32 read_UINT32(struct fcb *f, FILE *file)
{UINT32 n;

  if ( fread(&n,sizeof(UINT32),(size_t)1,file)!=1 ) {
    set_error(f,read_err,"read_UINT32 failed");
    return(0);
  }
  if ( f->byte_swapping_required ) byte_swap_UINT32((unsigned char *) &n);
  return(n);
}

static UINT64 read_UINT64(struct fcb *f, FILE *file)
{UINT64 n;

  if ( fread(&n,sizeof(UINT64),(size_t)1,file)!=1 ) {
    set_error(f,read_err,"read_UINT64 failed");
    return(0);
  }
  if ( f->byte_swapping_required ) byte_swap_UINT64((unsigned char *) &n);
  return(n);
}

static boolean read_fib(struct fcb *f,char id[], boolean byte_swapping_required,
  boolean read_only)
{int i,j; FILE_OFFSET position; FILE *file;

  file = fopen(id,"rb");
  if ( file==NULL ) set_error(f,badopen_err,"Couldn't open fib");
  else if ( fseeko(file,(FILE_OFFSET) 0,0)!=0 ) set_error(f,badopen_err,"fib seek failed");
  else {
    f->byte_swapping_required = byte_swapping_required;
    f->read_only = read_only;

    f->error_code = read_UINT32(f,file);
    f->version = read_UINT32(f,file);
    f->sub_version = read_UINT32(f,file);
    f->segment_cnt = read_UINT32(f,file);
    for ( i=0; i<max_index; i++) f->primary_level[i] = read_UINT32(f,file);
    f->marker = read_UINT32(f,file);
    f->file_ok = read_UINT32(f,file);
    for (i=0; i<max_level; i++)
      for (j=0; j<max_index; j++) {
        f->first_free_block[i][j].segment = read_UINT16(f,file);
        f->first_free_block[i][j].block = read_UINT64(f,file);
      }
    for (i=0; i<max_level; i++)
      for (j=0; j<max_index; j++) {
        f->first_at_level[i][j].segment = read_UINT16(f,file);
        f->first_at_level[i][j].block = read_UINT64(f,file);
      }
    for (i=0; i<max_level; i++)
      for (j=0; j<max_index; j++) {
        f->last_pntr[i][j].segment = read_UINT16(f,file);
        f->last_pntr[i][j].block = read_UINT64(f,file);
      }
    f->max_file_lc = read_UINT64(f,file);
    for (i=0; i<max_segment; i++) f->segment_length[i] = read_UINT64(f,file);
    f->data_in_index_lc = read_UINT32(f,file);
    position = ftello(file);
    if ( position!=fib_lc_on_disk ) set_error1(f,badopen_err,"Read fib failed, position=",(int)position);
    fclose(file);
  }
  return(f->error_code==no_err);
}

static void write_UINT16(struct fcb *f, FILE *file, UINT16 *i)
{UINT16 n; unsigned char ch;
 unsigned char *p = (unsigned char *)&n;

  n = *i;
  if ( f->byte_swapping_required ) {
    ch = p[0];
    p[0] = p[1];
    p[1] = ch;
  }
  if ( fwrite(&n,sizeof(UINT16),(size_t)1,file)!=1 )
    set_error(f,write_err,"write failed in write_UINT16\n");
}

static void write_UINT16s(struct fcb *f, FILE *file, unsigned char s[], unsigned int cnt)
{unsigned int i; unsigned char swapped[keyspace_lc];

  if ( f->byte_swapping_required ) {
    i = 0;
    while ( i<cnt*sizeof(UINT16) ) {
      swapped[i] = s[i+1];
      swapped[i+1] = s[i];
      i = i + sizeof(UINT16);
    }
    if ( fwrite(swapped,sizeof(UINT16),(size_t)cnt,file)!=cnt )
    set_error(f,write_err,"write_UINT16s failed\n");
  }
  else {
    if ( fwrite(s,sizeof(UINT16),(size_t)cnt,file)!=cnt )
      set_error(f,write_err,"write_UINT16s failed\n");
  }
}

static void write_UINT32(struct fcb *f, FILE *file, UINT32 i)
{UINT32 n;

  n = i;
  if ( f->byte_swapping_required ) byte_swap_UINT32((unsigned char *) &n);
  if ( fwrite(&n,sizeof(UINT32),(size_t)1,file)!= 1 )
    set_error(f,write_err,"write failed in write_UINT32\n");
}

static void write_UINT64(struct fcb *f, FILE *file, UINT64 i)
{UINT64 n;

  n = i;
  if ( f->byte_swapping_required ) byte_swap_UINT64((unsigned char *) &n);
  if ( fwrite(&n,sizeof(UINT64),(size_t)1,file)!= 1 )
    set_error(f,write_err,"write failed in write_UINT64\n");
}

static void write_fib(struct fcb *f)
{int i,j,fill_cnt,fib_blocks; FILE_OFFSET position; FILE *file;

  if ( f->error_code!=no_err ) return;
  else if ( f->read_only ) return;
  else {
    file = file_index(f,0);
    if ( file==NULL ) set_error(f,bad_close_err,"Bad file in write_fib");
    else if ( fseeko(file,(FILE_OFFSET) 0,0)!=0 ) set_error(f,bad_close_err,"Couldn't seek to fib");
    else {
      /*      f->byte_swapping_required = false;*/

      write_UINT32(f,file,f->error_code);
      write_UINT32(f,file,f->version);
      write_UINT32(f,file,f->sub_version);
      write_UINT32(f,file,f->segment_cnt);
      for ( i=0; i<max_index; i++) write_UINT32(f,file,f->primary_level[i]);
      write_UINT32(f,file,f->marker);
      write_UINT32(f,file,f->file_ok);
      for (i=0; i<max_level; i++)
        for (j=0; j<max_index; j++) {
          write_UINT16(f,file,&(f->first_free_block[i][j].segment));
          write_UINT64(f,file,f->first_free_block[i][j].block);
        }
      for (i=0; i<max_level; i++)
        for (j=0; j<max_index; j++) {
          write_UINT16(f,file,&(f->first_at_level[i][j].segment));
          write_UINT64(f,file,f->first_at_level[i][j].block);
        }
      for (i=0; i<max_level; i++)
        for (j=0; j<max_index; j++) {
          write_UINT16(f,file,&(f->last_pntr[i][j].segment));
          write_UINT64(f,file,f->last_pntr[i][j].block);
        }
      write_UINT64(f,file,f->max_file_lc);
      for (i=0; i<max_segment; i++) write_UINT64(f,file,f->segment_length[i]);

      write_UINT32(f,file,f->data_in_index_lc);
      position = ftello(file);
      if ( position!=fib_lc_on_disk )
        set_error1(f,bad_close_err,"Wrong fib length on close, position=",(int)position);
      fib_blocks = (fib_lc_on_disk-1) / block_lc + 1;
      fill_cnt = ((fib_blocks * block_lc) - fib_lc_on_disk) / sizeof(int);
      for (i=0; i<fill_cnt; i++) write_UINT32(f,file,0);

    }
  }
}

#ifdef VERIFY_WRITES
/* eq_block checks that two blocks are identical                */

static boolean eq_block(block_type_t *b1, block_type_t *b2)
{int i,r,offset; boolean same=true;

  if ( b1->keys_in_block!=b2->keys_in_block ) same = false;
  if ( b1->chars_in_use !=b2->chars_in_use  ) same = false;
  if ( b1->index_type   !=b2->index_type    ) same = false;
  if ( b1->prefix_lc    !=b2->prefix_lc     ) same = false;
  if ( b1->unused       !=b2->unused        ) same = false;
  if ( b1->level        !=b2->level         ) same = false;
  if ( b1->next.segment !=b2->next.segment  ) same = false;
  if ( b1->next.block   !=b2->next.block    ) same = false;
  if ( b1->prev.segment !=b2->prev.segment  ) same = false;
  if ( b1->prev.block   !=b2->prev.block    ) same = false;
  for (i=0; i<b1->keys_in_block; i++ )
    if ( b1->keys[i]!=b2->keys[i] ) same = false;
  offset = keyspace_lc - b1->chars_in_use;
  r = memcmp((char *)b1->keys+offset,(char *)b2->keys+offset,(size_t) b1->chars_in_use);
  if ( r!=0 ) same = false;
  return(same);
}
#endif


#ifdef VERIFY_IX_COMPRESS

static boolean eq0_pntr(struct fcb *f, level0_pntr *p1, level0_pntr *p2)
{
  if ( p1->lc!=p2->lc ) return(false);
  else if ( p1->lc<=f->data_in_index_lc )
    return( memcmp(p1->data_rec,p2->data_rec,(size_t)p1->lc)==0 );
  else return((p1->segment==p2->segment) && (p1->sc==p2->sc) );
}

static boolean eqx_pntr(struct fcb *f, levelx_pntr *p1, levelx_pntr *p2, int level)
{
  if ( level==0 ) return(eq0_pntr(f,&(p1->p0),&(p2->p0)));
  else  return(eqn_pntr(p1->pn,p2->pn));
}

/* eq_block_content checks that two blocks have the same key and    */
/*   pointer content.  Differences in prefix length, compression,   */
/*   or key order in the pool are OK.                               */

static boolean eq_block_content(struct fcb *f, block_type_t *b1, block_type_t *b2)
{int i; boolean same=true; struct key k1,k2; levelx_pntr p1,p2;

  if ( b1->keys_in_block!=b2->keys_in_block ) same = false;
  if ( b1->index_type   !=b2->index_type    ) same = false;
  if ( b1->level        !=b2->level         ) same = false;
  if ( b1->next.segment !=b2->next.segment  ) same = false;
  if ( b1->next.block   !=b2->next.block    ) same = false;
  if ( b1->prev.segment !=b2->prev.segment  ) same = false;
  if ( b1->prev.block   !=b2->prev.block    ) same = false;
  for (i=0; i<b1->keys_in_block; i++ ) {
    get_nth_key_and_pntr(f,b1,&k1,i,&p1);
    get_nth_key_and_pntr(f,b2,&k2,i,&p2);
    if ( !eq_key(&k1,&k2) || !eqx_pntr(f,&p1,&p2,b1->level) ) same = false;
  }
  return(same);
}
#endif



static void read_page(struct fcb *f, struct leveln_pntr p, block_type_t *buf)
{FILE *file; FILE_OFFSET offset;

 read_cnt++;
  if ( f->trace ) {
    print_leveln_pntr(f->log_file,"reading page ",&p);
    fprintf(f->log_file,"\n");
  }
  file = file_index(f,p.segment);
  offset = (p.block) << f->block_shift;
  if ( file==NULL ) set_error(f,read_err,"Bad file in read_page");
  else if ( fseeko(file,offset,0)!=0 )
    set_error(f,seek_err,"Seek failed in read_page");
  else {
    buf->keys_in_block = read_UINT16(f,file);
    buf->chars_in_use = read_UINT16(f,file);
    buf->index_type = read_byte(f,file);
    buf->prefix_lc = read_byte(f,file);
    buf->unused = read_byte(f,file);
    buf->level = read_byte(f,file);
    buf->next.segment = read_UINT16(f,file);
    buf->next.block = read_UINT64(f,file);
    buf->prev.segment = read_UINT16(f,file);
    buf->prev.block = read_UINT64(f,file);
    fread(buf->keys,(size_t) 1, (size_t) keyspace_lc,file);
    if ( ftello(file)!=(FILE_OFFSET)(offset+block_lc) )
      set_error1(f,read_err,"I/O failure in read_page, bytes read=",(int)(ftello(file)-offset));
    if ( f->byte_swapping_required )
      byte_swap_UINT16s((unsigned char *)buf->keys,buf->keys_in_block);
  }
}

void get_page(struct fcb *f, struct leveln_pntr blk, block_type_t *buf)
{int i; boolean found; struct leveln_pntr p;

  found = false;
  for (i=0; i<f->buffer_pool.buffers_in_use; i++) {
    p = f->buffer_pool.buffer[i].contents;
    if ( p.segment==blk.segment && p.block==blk.block ) { *buf = f->buffer_pool.buffer[i].b; found = true; }
  }
  if ( ! found ) read_page(f,blk,buf);
}

static void write_page(struct fcb *f, struct leveln_pntr p, block_type_t *buf)
{int pntr_lc,remaining; FILE *file; FILE_OFFSET offset;

  write_cnt++;
  if ( f->read_only ) {
    f->error_code = read_only_err;
    return;
  }
  if ( f->trace ) {
    print_leveln_pntr(f->log_file,"writing page ",&p);
    fprintf(f->log_file,"\n");
  }
  file = file_index(f,p.segment);
  offset = (p.block) << f->block_shift;
  if ( file==NULL ) set_error(f,write_err,"Bad file in write_page");
  else if ( fseeko(file,offset,0)!=0 )
    set_error(f,seek_err,"Seek error in write_page");
  else {
    write_UINT16(f,file,&(buf->keys_in_block));
    write_UINT16(f,file,&(buf->chars_in_use));
    if ( fwrite(&(buf->index_type),sizeof(char),(size_t)1,file)!=1 )
      set_error(f,write_err,"write byte failed");
    if ( fwrite(&(buf->prefix_lc), sizeof(char),(size_t)1,file)!=1 )
      set_error(f,write_err,"write byte failed");
    if ( fwrite(&(buf->unused),sizeof(char),(size_t)1,file)!=1 )
      set_error(f,write_err,"write byte failed");
    if ( fwrite(&(buf->level),     sizeof(char),(size_t)1,file)!=1 )
      set_error(f,write_err,"write byte failed");
    write_UINT16(f,file,&(buf->next.segment));
    write_UINT64(f,file,buf->next.block);
    write_UINT16(f,file,&(buf->prev.segment));
    write_UINT64(f,file,buf->prev.block);
    write_UINT16s(f,file,(unsigned char *)buf->keys,(unsigned)buf->keys_in_block);
    pntr_lc = buf->keys_in_block * sizeof(UINT16);
    remaining = keyspace_lc - pntr_lc;
    fwrite((char *)buf->keys+pntr_lc,(size_t) 1, (size_t) remaining,file);
    if ( ftello(file)!=(FILE_OFFSET)(offset+block_lc) )
      set_error1(f,read_err,"I/O failure in write_page, bytes written=",(int)(ftello(file)-offset));
#ifdef VERIFY_WRITES
    block_type_t temp;
    read_page(f,p,&temp);
    if ( !eq_block(buf,&temp) ) set_error(f,write_err,"**write_page failed, doesn't match orig\n");
#endif
  }
}

/* vacate_file_index finds the LRU file_index, closes the segment */
/*   currently in use, marks the segment as closed and returns an */
/*   index into open_file to be used for segment I/O              */

static int vacate_file_index(struct fcb *f)
{int i,oldest,age,max_age;

  oldest = 0; max_age = 0;
  for ( i=0; i<f->open_file_cnt; i++ ) {
    age = f->current_age - f->file_age[i];
    if ( age>max_age ) {
      oldest = i; max_age = age;
    }
  }
  f->segment_ix[f->open_segment[oldest]] = max_files;
  fclose(f->open_file[oldest]);
  return(oldest);
}

/* open_segment opens a file segment.  If it is new it is opened in */
/*   write+ mode otherwise it is opened in read+ mode.  If the open */
/*   fails then f->open_file[ix] is set to NULL and f->error_code is*/
/*   set.  In any case the directories segment_ix[] and             */
/*   open_segment[] are set.  */

static void open_segment(struct fcb *f, unsigned segment, int ix)
{char name[max_filename_lc+10]; char* mode;

  build_segment_name(f,segment,name);
  if ( segment >= (unsigned)f->segment_cnt ) {
    if ( f->read_only ) {
      set_error(f,read_only_err,"Read only_err");
      return;
    }
    else f->open_file[ix] = fopen(name,"wb+");
  }
  else {
    mode = f->read_only ? "rb" : "rb+";
    f->open_file[ix] = fopen(name,mode);
  }
  if (f->open_file[ix]==NULL) set_error(f,segment_open_err,"Bad file in open_segment");
  f->segment_ix[segment] = ix;
  f->open_segment[ix] = segment;
  if ( f->trace)  fprintf(f->log_file,"Opening segment %s on file index %d\n",name,ix);
}

static int file_ix(struct fcb *f, unsigned segment)
{int ix;

  ix = f->segment_ix[segment];
  if (ix<max_files) /* have a file open */;
  else if (f->open_file_cnt<max_files) {
    ix = f->open_file_cnt;
    f->open_file_cnt++;
    open_segment(f,segment,ix);
  }
  else {
    ix = vacate_file_index(f);
    open_segment(f,segment,ix);
  }
  f->file_age[ix] = f->current_age;
  if ( f->trace ) fprintf(f->log_file,"  segment %d open on file index %d\n",segment,ix);
  return(ix);
}


/* file_index returns a file index to be used for I/O to a given   */
/*   segment of the keyed file.  If there is a file index open for */
/*   the segment, it is returned.  If there is an unused file   */
/*   index it is opened for the segment and returned.  Otherwise   */
/*   the LRU index is closed and reopened for the segment.         */

static FILE *file_index(struct fcb *f, unsigned segment)
{int ix;

  if ( segment<max_segment ) {
    ix = file_ix(f,segment);
    return(f->open_file[ix]);
  }
  else return(NULL);
}

static void set_position(struct fcb *f, int index, struct leveln_pntr b, int ix)
{
  f->position[index] = b; f->position_ix[index] = ix;
}


/* Buffer handling */

/* Buffers are managed using two orthogonal structures.  The    */
/*   first is a conventional hash table that has been allocated */
/*   at the end of the fcb and whose base address is in         */
/*   buf->buffer_pool.buf_hash_table.  This hash table contains the index   */
/*   of the first buffer containing a block with the            */
/*   corresponding hash_value; an empty hash table entry        */
/*   contains -1.  If there are multiple buffers containing     */
/*   blocks with the same hash value then they are chained      */
/*   using the hash_next field in each buffer.  The hash table  */
/*   is searched using search_hash_chain() and managed using    */
/*   hash_chain_insert() and hash_chain_remove().               */
/* The second structure is a pair of linked lists that list     */
/*   buffers from youngest to oldest.  One list is maintained   */
/*   for buffers containing level_0 blocks, the other is for    */
/*   level_n blocks.  youngest_buffer[x] (x=level_0 or level_n) */
/*   is an index to the buffer containing the MRU block,        */
/*   oldest_buffer[x] contains the index of the LRU block.      */
/*   buffer_in_use[x] is the number of buffers in each chain.   */

/* reset_ages is called when f->current_age reaches INT_MAX.    */
/*   The age of all open files is set to 0 and                  */
/*   f->current_age is set to 0.                                */

static void reset_ages(struct fcb *f)
{int i;

  for (i=0; i<f->open_file_cnt; i++) f->file_age[i] = 0;
  f->current_age = 0;
}


#define hash_value(b,limit)  ((((int)b.block) + b.segment) % limit)


static int search_hash_chain(struct fcb *f, struct leveln_pntr block)
{int k,next,bufix=-1/*,cnt=0*/;

  
  k =  hash_value(block,f->buffer_pool.buf_hash_entries);
  next = f->buffer_pool.buf_hash_table[k];
  while ( next>=0 ) {
    if ( eqn_pntr(f->buffer_pool.buffer[next].contents,block) ) {
      bufix = next; next = -1;
    }
    else next = f->buffer_pool.buffer[next].hash_next;
  }
  /*  fprintf(f->log_file,"  searched hash chain %d for %d/%d, ",k,block.segment,block.block);
  if ( bufix<0 ) fprintf(f->log_file,"not found, cnt=%d\n",cnt);
  else fprintf(f->log_file,"found in bufix=%d, cnt=%d\n",bufix,cnt);*/
  return(bufix);
}

/* hash_chain_insrt inserts a buffer in a hash_chain     */
/*   The block pointer is hashed and the hash table is   */
/*   checked.  If the hash table entry<0 then the        */
/*   chain is empty and the buffer inserted as a         */
/*   chain of length one.  If the entry>=0 then the      */
/*   chain is searched and the buffer inserted.          */
/* It assumes that the the buffer contents and           */
/*   hash_next fields have been set prior to call.       */

static void hash_chain_insert(struct fcb *f, int bufix)
{int k,next,last=-1; struct leveln_pntr block;

  block = f->buffer_pool.buffer[bufix].contents;
  k = hash_value(block,f->buffer_pool.buf_hash_entries);
  next = f->buffer_pool.buf_hash_table[k];
  if ( next<0 ) {
    f->buffer_pool.buf_hash_table[k] = bufix;
    f->buffer_pool.buffer[bufix].hash_next = -1;
  }
  else {
    while ( next>=0 && gt_n_pntr(block,f->buffer_pool.buffer[next].contents) ) {
      last = next; next = f->buffer_pool.buffer[next].hash_next;
    }
    if ( last<0 ) {
      f->buffer_pool.buffer[bufix].hash_next = f->buffer_pool.buf_hash_table[k];
      f->buffer_pool.buf_hash_table[k] = bufix;
    }
    else {
      f->buffer_pool.buffer[last].hash_next = bufix;
      f->buffer_pool.buffer[bufix].hash_next = next;
    }
  }
  if ( f->trace ) {
    fprintf(f->log_file,"  inserted buffer %d (",bufix);
    print_leveln_pntr(f->log_file,"",&block);
    fprintf(f->log_file,") into hash chain %d\n",k);
    print_hash_chain(f->log_file,f,k);
  }
}

/* hash_chain_remove removes buffer[bufix] from its hash chain */
/* It assumes that the the buffer contents and           */
/*   hash_next fields have been set prior to call.       */

static void hash_chain_remove(struct fcb *f, int bufix)
{int k,next,last=0; struct leveln_pntr block;

  block = f->buffer_pool.buffer[bufix].contents;
  k = hash_value(block,f->buffer_pool.buf_hash_entries);
  if ( f->trace ) {
    fprintf(f->log_file,"Removing buffer %d from hash chain %d\n  old ",bufix,k);
    print_hash_chain(f->log_file,f,k);
  }
  next = f->buffer_pool.buf_hash_table[k];
  if ( next==bufix ) f->buffer_pool.buf_hash_table[k] = f->buffer_pool.buffer[bufix].hash_next;
  else {
    while ( (next>=0) && !eqn_pntr(block,f->buffer_pool.buffer[next].contents) ) {
      last = next; next = f->buffer_pool.buffer[next].hash_next;
    }
    if ( next<0 ) {
      set_error1(f,alloc_buf_err,"Tried to remove nonexistent buffer, bufix=",bufix);
    }
    else f->buffer_pool.buffer[last].hash_next = f->buffer_pool.buffer[next].hash_next;
  }
  if ( f->trace ) {
    fprintf(f->log_file,"  new ");
    print_hash_chain(f->log_file,f,k);
  }
  f->buffer_pool.buffer[bufix].hash_next = -1;
}

/* make_buffer_youngest removes the buffer in bufix from the */
/*   age chain and inserts it as the youngest buffer         */
/* It assumes that the buffer older and younger fields are   */
/*   set prior to call.                                      */

static void make_buffer_youngest(struct fcb *f,int bufix)
{int older,younger;

  older = f->buffer_pool.buffer[bufix].older;
  younger = f->buffer_pool.buffer[bufix].younger;
  if ( younger>=0 ) { /* not allready youngest */
    if ( older==-1 ) {
      f->oldest_buffer = younger;
      f->buffer_pool.buffer[younger].older = -1;
    }
    else {
      f->buffer_pool.buffer[older].younger = younger;
      f->buffer_pool.buffer[younger].older = older;
    }
    f->buffer_pool.buffer[f->youngest_buffer].younger = bufix;
    f->buffer_pool.buffer[bufix].younger = -1;
    f->buffer_pool.buffer[bufix].older = f->youngest_buffer;
    f->youngest_buffer = bufix;
  }
}


static void init_buffer_hash_fields(struct fcb *f, int i, struct leveln_pntr *b)
{
  f->buffer_pool.buffer[i].older = -1;
  f->buffer_pool.buffer[i].younger = -1;
  f->buffer_pool.buffer[i].hash_next = -1;
}

/* initialize_buffer initializes all of the buffer header fields except the */
/*   hashing fields (older, younger, hash_next).  It may reinitialize some  */
/*   fields (e.g., contents) that were set by the hashing functions.        */

static void initialize_buffer(struct fcb *f, int bufix, struct leveln_pntr *contents)
{
  f->buffer_pool.buffer[bufix].contents = *contents;
  f->buffer_pool.buffer[bufix].modified = false;
  f->buffer_pool.buffer[bufix].lock_cnt = 0;
  f->buffer_pool.buffer[bufix].search_cnt = 0;
}

#define buddy_window 32

static int write_block_and_buddies(struct fcb *f, int bufix)
{int i,younger_buddies=0,older_buddies=0,ix,buddy_list[buddy_window];
 struct leveln_pntr buddy; boolean done=false;

  buddy = f->buffer_pool.buffer[bufix].contents;
  while ( buddy.block>0 && younger_buddies<buddy_window && !done) {
    buddy.block--;
    ix = search_hash_chain(f,buddy);
    if ( ix>=0 && f->buffer_pool.buffer[ix].modified && f->buffer_pool.buffer[ix].lock_cnt==0) {
      buddy_list[younger_buddies] = ix;
      younger_buddies++;
    }
    else done = true;
  }
  for (i=younger_buddies-1; i>=0; i--) {
    ix = buddy_list[i];
    write_page(f,f->buffer_pool.buffer[ix].contents,&(f->buffer_pool.buffer[ix].b));
    f->buffer_pool.buffer[ix].modified = false;
  }

  write_page(f,f->buffer_pool.buffer[bufix].contents,&(f->buffer_pool.buffer[bufix].b));

  buddy = f->buffer_pool.buffer[bufix].contents;
  done = false;
  while ( older_buddies<buddy_window && !done) {
    buddy.block++;
    ix = search_hash_chain(f,buddy);
    if ( ix>=0 && f->buffer_pool.buffer[ix].modified && f->buffer_pool.buffer[ix].lock_cnt==0) {
      buddy_list[older_buddies] = ix;
      older_buddies++;
    }
    else done = true;
  }
  for (i=0; i<older_buddies; i++) {
    ix = buddy_list[i];
    write_page(f,f->buffer_pool.buffer[ix].contents,&(f->buffer_pool.buffer[ix].b));
    f->buffer_pool.buffer[ix].modified = false;
  }
#ifdef log_buffers
  if ( younger_buddies>0 || older_buddies>0 ) {
    fprintf(buffer_log,"Wrote block %d/%lu, %d younger buddies, %d older buddies\n",
      f->buffer_pool.buffer[bufix].contents.segment,
      f->buffer_pool.buffer[bufix].contents.block,younger_buddies,older_buddies);
  }
#endif

  return(younger_buddies+older_buddies+1);
}

/* vacate_oldest_buffer is called when a new buffer is needed          */
/*   if there are unallocated buffers then the next one is             */
/*   added to the buffer chain and returned.  If all buffers           */
/*   are in use then the oldest unlocked buffer is flushed             */
/*   (if necessary) and returned                                       */
/* If an unallocated buffer is returned then it is initialized (by     */
/*   init_buffer_hash_fields) but further initialization by the caller */
/*   will be     */
/*   required.  If a buffer is vacated then only the buffer management */
/*   fields (older, younger, hash_next) are set.  The caller should    */
/*   not modify those fields but should initialize everything else as  */
/*   necessary.                                                        */

static int vacate_oldest_buffer(struct fcb *f, struct leveln_pntr *b)
{int oldest,cnt=0,locked_cnt=0,i; boolean done; struct leveln_pntr oldest_block;

  if ( f->buffer_pool.buffers_in_use < f->buffer_pool.buffers_allocated ) {
    oldest = f->buffer_pool.buffers_in_use;
    init_buffer_hash_fields(f,oldest,b);
    initialize_buffer(f,oldest,b);
    if ( f->buffer_pool.buffers_in_use==0 ) {
      f->youngest_buffer = oldest; f->oldest_buffer = oldest;
    }
    else {
      f->buffer_pool.buffer[f->youngest_buffer].younger = oldest;
      f->buffer_pool.buffer[oldest].older = f->youngest_buffer;
      f->youngest_buffer = oldest;
    }
    f->buffer_pool.buffers_in_use++;
#ifdef log_buffers
    fprintf(buffer_log,"Paging block %d/%lu into unused buffer %d, ",b->segment,b->block,oldest);
    fprintf(buffer_log,"MRU chain after insert:");
    print_buffer_MRU_chain(buffer_log,f);
#endif
  }
  else {
    do {
      oldest = f->oldest_buffer;
      make_buffer_youngest(f,oldest);
      cnt++;
      if ( cnt>f->buffer_pool.buffers_allocated ) {
        done = true; 
        set_error1(f,alloc_buf_err,"Couldn't allocate a buffer, allocated=",f->buffer_pool.buffers_allocated);
      }
      else if ( f->buffer_pool.buffer[oldest].lock_cnt>0 ) {
        done = false; locked_cnt++;
      }
      else done = true;
    }
    while ( !done );
    oldest_block = f->buffer_pool.buffer[oldest].contents;
    if ( f->buffer_pool.buffer[oldest].modified ) {
      /*      write_page(f,oldest_block,&(f->buffer_pool.buffer[oldest].b));*/
      write_block_and_buddies(f,oldest);

      if ( trace_io ) {
        print_leveln_pntr(f->log_file,"  wrote block ",&oldest_block);
        print_buffer_caption(f->log_file,f,oldest);
        fprintf(f->log_file," from buffer %d, %d others in window\n",oldest,i);
      }
    }
    hash_chain_remove(f,oldest);
    initialize_buffer(f,oldest,b);
#ifdef log_buffers
    fprintf(buffer_log,"Paging block %d/%lu into buffer %d",b->segment,b->block,oldest);
    if ( f->buffer_pool.buffer[oldest].modified ) fprintf(buffer_log,", flushing ");
    else fprintf(buffer_log,", replacing ");
    fprintf(buffer_log,"block %d/%lu, ",oldest_block.segment,oldest_block.block);
    fprintf(buffer_log,"MRU chain after insert:");
    print_buffer_MRU_chain(buffer_log,f);
#endif
  }
  if ( f->trace ) fprintf(f->log_file,"  just vacated oldest buffer, bufix=%d\n",oldest);
  return(oldest);
}

static void copy_index_block(struct ix_block *b1, struct ix_block *b2)
{int i,pool_sc;

  b2->keys_in_block = b1->keys_in_block;
  b2->chars_in_use  = b1->chars_in_use;
  b2->index_type    = b1->index_type;
  b2->prefix_lc     = b1->prefix_lc;
  b2->level         = b1->level;
  b2->next          = b1->next;
  b2->prev          = b1->prev;


  for (i=0; i<b1->keys_in_block; i++) b2->keys[i] = b1->keys[i];
  /*  mvc(b1->keys,0,b2->keys,0,(b1->keys_in_block*sizeof(UINT16)));*/
  pool_sc = keyspace_lc - b1->chars_in_use;
  mvc(b1->keys,pool_sc,b2->keys,pool_sc,b1->chars_in_use);
}


static void set_empty_block_prefix(struct ix_block *b, struct key *prefix, unsigned prefix_lc)
{
  mvc(prefix->text,0,b->keys,keyspace_lc-prefix_lc,prefix_lc);
  b->chars_in_use = prefix_lc;
  b->prefix_lc = prefix_lc;
}

static void initialize_index_block(struct ix_block *b, int index, unsigned lvl,
  struct key *prefix, unsigned prefix_lc)
{
  set_empty_block_prefix(b,prefix,prefix_lc);
  b->keys_in_block = 0;
  b->index_type = index;
  b->level = lvl;
  b->next = nulln_ptr;
  b->prev = nulln_ptr;
}

static int get_index(struct fcb *f, struct leveln_pntr b)
{int bufix,index_type; struct key dummy;

  f->current_age++;
  if ( f->current_age==INT_MAX ) reset_ages(f);
  bufix = search_hash_chain(f,b);

  if ( bufix>=0 ) {
    make_buffer_youngest(f,bufix);
#ifdef log_buffers
    fprintf(buffer_log,"Found lvl %d block %d/%ld in buffer %d\n",f->buffer_pool.buffer[bufix].b.level,
      b.segment,b.block,bufix);
#endif
  }
  else {
    bufix = vacate_oldest_buffer(f,&b);
    hash_chain_insert(f,bufix);
    read_page(f,b,&(f->buffer_pool.buffer[bufix].b));
    if ( trace_io ) {
      print_leveln_pntr(f->log_file,"  read block ",&b);
      print_buffer_caption(f->log_file,f,bufix);
      fprintf(f->log_file," into buffer %d\n",bufix);
    }
  }
  if ( f->error_code==no_err ) {
    index_type = f->buffer_pool.buffer[bufix].b.index_type;
    f->mru_at_level[f->buffer_pool.buffer[bufix].b.level][index_type] = b;
  }
  else initialize_index_block(&(f->buffer_pool.buffer[bufix].b),user_ix,level_zero,&dummy,(unsigned)0);
  return(bufix);
}

/* Prefix compression.  Keys in a block may have a common prefix that */
/*   is omitted when stored.   */
/*                                                                    */
/*   On block splits the prefix can never get shorter since the max   */
/*   key for the split blocks is the same as for the original block.  */

static int find_prefix_lc(struct key *k1, struct key *k2)
{int i=0,max_lc;

  max_lc = k1->lc;
  if ( k1->lc>k2->lc ) max_lc = k2->lc; 
  if ( max_lc>max_prefix_lc ) max_lc = max_prefix_lc;
  while ( i<max_lc && (k1->text[i]==k2->text[i]) ) i++;
  return(i);
}

/* block_prefix_lc computes the prefix that b should have.  Note that */
/*   a block must contain at least two keys to have a prefix.  For    */
/*   blocks with 0 or one key we don't return 0 since the block may   */
/*   have been initialized (set_empty_block_prefix) but not filled    */
/*   yet.                                                             */

static int block_prefix_lc(struct ix_block *b)
{struct key first,last;

  if ( b->keys_in_block>1 ) {
    get_nth_key(b,&first,0);
    get_max_key(b,&last);
    return( find_prefix_lc(&first,&last) );
  }
  else return(b->prefix_lc);
}

static unsigned ix_pool_lc(struct ix_block *b)
{
  return(b->chars_in_use + (b->keys_in_block * sizeof(UINT16) ));
}

static unsigned ix_entry_lc(struct fcb *f, struct key *k, levelx_pntr *p, unsigned prefix_lc,
 int level)
{UINT32 lc;

  lc = k->lc - prefix_lc;

  return(lc + UINT32_lc_if_compressed(lc) + levelx_pntr_lc(f,p,level));
}

/* ix_entries_lc returns the encoded length of the index entries  */
/*   in positions start..(start+cnt-1) if they were encoded using */
/*   prefix_lc.  Note that this version should only be used when  */
/*   it is known that all of the keys will be of                  */
/*   length>=prefix_lc.  */

static unsigned ix_entries_lc(struct fcb *f, struct ix_block *b, int start, int cnt, unsigned prefix_lc)
{int i,entry_lc=0,lc,new_key_lc,prefix_difference; UINT16 key_lc;

  prefix_difference = b->prefix_lc - prefix_lc;

  if ( (start+cnt)>b->keys_in_block )
    set_error1(f,ix_struct_err,"Request out of range in ix_entries_lc, start=",start);

  for (i=0; i<cnt; i++) {
    lc = uncompress_key_lc(&key_lc,(unsigned char *)b->keys+b->keys[start+i]);
    new_key_lc = key_lc + prefix_difference;
    if ( new_key_lc<0 ) new_key_lc = 0;
    entry_lc = entry_lc + new_key_lc + UINT32_lc_if_compressed((UINT32)new_key_lc) + nth_pntr_lc(f,b,start+i);
  }
  return(entry_lc);
}

/* new_chars_in_use returns the length of the key/pointer entries */
/*   in b (not including the keys array) if the   */
/*   prefix is changed to prefix_lc.  It's the original   */
/*   lc + the difference in the total key length + the       */
/*   stored prefix difference + any change in the compressed key  */
/*   lengths.  Note that this version should only be used when it */
/*   is known that all keys are of length>=prefix_lc.             */

static unsigned new_chars_in_use(struct ix_block *b, int prefix_lc)
{int i,expected_lc,new_key_lc,lc,prefix_difference; UINT16 key_lc;

  expected_lc = b->chars_in_use;
  prefix_difference = b->prefix_lc - prefix_lc;
  if ( prefix_difference!=0 ) {
    expected_lc = expected_lc + (prefix_difference * b->keys_in_block) - prefix_difference;
    for (i=0; i<b->keys_in_block; i++) {
      lc = uncompress_key_lc(&key_lc,(unsigned char *)b->keys+b->keys[i]);
      new_key_lc = key_lc + prefix_difference;
      if ( new_key_lc<0 ) new_key_lc = 0;
      expected_lc = expected_lc + (UINT32_lc_if_compressed((UINT32)(new_key_lc)) - lc);
    }
  }
  return(expected_lc);
}
/* ix_pool_lc_after_insert returns the size that an ix_pool would */
/*   be if key k and pointer p were inserted in entry ix of block */
/*   b.  If the insertion is at either the beginning or end of    */
/*   block then any change in length due to a new prefix must be  */
/*   computed.  The returned size may exceed keyspace_lc.  The    */
/*   prefix_lc of the block after insertion is returned in        */
/*   new_prefix_lc.                                               */


static int ix_pool_lc_after_insert(struct fcb *f, struct ix_block *b, struct key *k,
  levelx_pntr *p, int ix, unsigned *new_prefix_lc)
{int needed,pool_lc,save_pool_lc; struct key min,max;

  if ( b->keys_in_block==0 ) *new_prefix_lc = 0;
  else if ( ix==0 ) {
    get_max_key(b,&max);
    *new_prefix_lc = find_prefix_lc(k,&max);
  }
  else if ( ix==b->keys_in_block ) {
    get_nth_key(b,&min,0);
    *new_prefix_lc = find_prefix_lc(&min,k);
  }
  else *new_prefix_lc = b->prefix_lc;

  needed = ix_entry_lc(f,k,p,*new_prefix_lc,b->level) + sizeof(UINT16);
  save_pool_lc = ix_pool_lc(b) + needed;
  pool_lc = new_chars_in_use(b,(int)*new_prefix_lc) + (b->keys_in_block * sizeof(UINT16)) + needed;

  if ( f->trace ) {
    fprintf(f->log_file,"  ix_pool_aft_insrt, need=%d, k->lc=%d, orig ix_pool=%d, prefix_lc=%d/%d, ",
      needed,k->lc,ix_pool_lc(b),b->prefix_lc,*new_prefix_lc);
    fprintf(f->log_file,"keys_in_block=%d, ix=%d\n  pool before prefix adjustment=%d, after=%d\n",
      b->keys_in_block,ix,save_pool_lc,pool_lc);
  }

  return( pool_lc );
}

/* ix_pool_lc_after_replace returns the size that an ix_pool would */
/*   be if key k and pointer p replace those currently in entry ix */
/*   of block b.  If the replace is at either the beginning or end */
/*   of block then any change in length due to a new prefix must be*/
/*   computed.  The returned size may exceed keyspace_lc.  The     */
/*   prefix_lc of the block after replacement is returned in       */
/*   new_prefix_lc.                                                */

static int ix_pool_lc_after_replace(struct fcb *f, struct ix_block *b, struct key *k, levelx_pntr *p,
  int ix, unsigned *new_prefix_lc)
{int i,pool_lc; struct key min,max;

  if ( b->keys_in_block<=1 ) *new_prefix_lc = 0;
  else if ( ix==0 ) {
    get_max_key(b,&max);
    *new_prefix_lc = find_prefix_lc(k,&max);
  }
  else if ( ix==b->keys_in_block-1 ) {
    get_nth_key(b,&min,0);
    *new_prefix_lc = find_prefix_lc(&min,k);
  }
  else *new_prefix_lc = b->prefix_lc;

  if ( *new_prefix_lc==b->prefix_lc ) {
    pool_lc = ix_pool_lc(b) - ix_entries_lc(f,b,ix,1,*new_prefix_lc)
               + ix_entry_lc(f,k,p,*new_prefix_lc,b->level);
  }
  else {
    pool_lc = *new_prefix_lc + (b->keys_in_block * sizeof(UINT16));
    for (i=0; i<b->keys_in_block; i++) {
      if ( i==ix ) {
        pool_lc = pool_lc + ix_entry_lc(f,k,p,*new_prefix_lc,b->level);
      }
      else {
        pool_lc = pool_lc + ix_entries_lc(f,b,i,1,*new_prefix_lc);
      }
    }
  }

  if ( f->trace ) {
    fprintf(f->log_file,"  ix_pool_aft_repl, k->lc=%d, orig ix_pool(b)=%d, prefix_lc=%d/%d",
      k->lc,ix_pool_lc(b),b->prefix_lc,*new_prefix_lc);
    fprintf(f->log_file,", keys_in_block=%d, ix=%d, pool_lc=%d\n",b->keys_in_block,ix,pool_lc);
  }

  return( pool_lc );
}

/* ix_pool_lc_after_change returns the size that an ix_pool would */
/*   be if block b is modified with key k and pointer p.  If      */
/*   insert=true then k and p are to be inserted in entry ix.  If */
/*   insert=false then k and p replace entry ix.  If the change   */
/*   is at either the beginning or end of block then any change   */
/*   in length due to a new prefix is computed.  The returned     */
/*   size may exceed keyspace_lc.  The prefix_lc of the block     */
/*   after change is returned in new_prefix_lc.                   */

static int ix_pool_lc_after_change(struct fcb *f, struct ix_block *b, struct key *k, levelx_pntr *p,
  int ix, unsigned *new_prefix_lc, boolean insert)
{
  if ( insert ) return(ix_pool_lc_after_insert(f,b,k,p,ix,new_prefix_lc));
  else  return(ix_pool_lc_after_replace(f,b,k,p,ix,new_prefix_lc));
}

/* Index searching */

/* Interior index blocks contain entries that point to the subtree
 *   containing keys <= entry key (and greater than the previous
 *   entry key).  Each level in the index has an additional pointer
 *   that points to the last subtree -- the subtree containing keys
 *   <= the largest possible key.  This last pointer is not stored
 *   in the index block but is found in last_pntr[].
 */

/* compare_key compares a key k with the ix^th entry stored in an   */
/*   index block.                                                   */

static int compare_key(unsigned char k[], UINT32 klc, struct ix_block *b, int ix)
{int r,lc; UINT16 key_lc; unsigned char *entry_ptr;

  entry_ptr = (unsigned char *)b->keys + b->keys[ix];
  lc = uncompress_key_lc(&key_lc,entry_ptr);

  if ( klc<=key_lc ) {
    r = memcmp(k,entry_ptr+lc,(size_t)klc );
    if (r<0) return(cmp_less);
    else if (r>0) return(cmp_greater);
    else if ( klc==key_lc ) return(cmp_equal);
    else return(cmp_less);
  }
  else {
    r = memcmp(k,entry_ptr+lc,(size_t)key_lc );
    if (r<0) return(cmp_less);
    else if (r>0) return(cmp_greater);
    else if ( klc==key_lc ) return(cmp_equal);
    else return(cmp_greater);
  }
}

/* search_block searches the block for the first entry>=k             */
/*   if k = some entry then  found=true   and ix is entry             */
/*   if k < some entry then  found=false  and ix is entry             */
/*   if k > all entries then found=false ix = keys_in_block           */
/* If the block is empty (happens with freespace blocks where the     */
/*   only block is the primary) then the key is treated as greater    */
/*   than all entries.                                                */

static int search_block(struct fcb *f, int bufix, struct key *k, boolean *found)
{int mid,high,ix=0,r=0,prefix_lc,klc;
 unsigned char *t; struct ix_block *b;

  *found = false;
  b = &(f->buffer_pool.buffer[bufix].b);
  if ( b->keys_in_block>0 ) {
    /*    f->buffer_pool.buffer[bufix].search_cnt++;*/
    prefix_lc = b->prefix_lc;
    if ( k->lc<prefix_lc ) {
      r = memcmp(k->text,(char *) b->keys+keyspace_lc-prefix_lc,(size_t)k->lc );
      if (r>0) ix = b->keys_in_block;
      else ix = 0;
    }
    else {
      if ( prefix_lc>0 ) r = memcmp(k->text,(char *) b->keys+keyspace_lc-prefix_lc,(size_t)prefix_lc );

      if ( r==0 ) {
        klc = k->lc - prefix_lc;
        t = k->text + prefix_lc;
        high = b->keys_in_block-1;

        mid = (ix + high) / 2; 
        while ( ix<=high ) {
          switch ( compare_key(t,(UINT32)klc,b,mid) ) {
            case cmp_greater: 
              ix = mid + 1;
              mid = (ix + high) / 2;
              break;
            case cmp_equal:
              ix = mid;
              *found = true;
              high = -1;
              break;
            case cmp_less:
              high = mid - 1;
              mid = (ix + high) / 2;
              break;
          }
        }
      }
      else if ( r<0 ) ix = 0;
      else /* r>0 */ ix = b->keys_in_block;
    }
  }
  /* now ix points to first entry>=k or keys_in_block */
  if ( f->trace ) {
    fprintf(f->log_file,"(%s)searched level %d ",f->search_block_caller,f->buffer_pool.buffer[bufix].b.level);
    print_leveln_pntr(f->log_file,"block ",&(f->buffer_pool.buffer[bufix].contents));
    print_key(f->log_file,f->buffer_pool.buffer[bufix].b.index_type,k," for k=");
    if (ix>=b->keys_in_block) fprintf(f->log_file," larger than any in block\n");
    else if ( *found ) fprintf(f->log_file," found it, ix=%d\n",ix);
    else fprintf(f->log_file," not found, ix=%d\n",ix);
  }
  return(ix);
}

/*
 * search_index searches index blocks down to stop_lvl and returns
 *   a pointer to the block at stop_lvl-1 in which the key lies.
 *   By construction, the key must be smaller than some key in
 *   each block searched unless it is in the rightmost block at
 *   this level.  If a key is larger than any in this level, then
 *   the last_pntr pointer is the returned.
 */

static struct leveln_pntr search_index(struct fcb *f, int index, UINT32 stop_lvl,
  struct key *k)
{struct leveln_pntr child; int ix,bufix; boolean done=false,found; char *name="search_index";

  child = f->first_at_level[f->primary_level[index]][index];
  if ( f->trace ) f->search_block_caller = name;
  if ( stop_lvl<=f->primary_level[index] )
    do {
      bufix = get_index(f,child);
      ix = search_block(f,bufix,k,&found);
      done = f->buffer_pool.buffer[bufix].b.level<=stop_lvl;
      if ( ix>=f->buffer_pool.buffer[bufix].b.keys_in_block ) { /* larger than any key */
        if ( null_pntr(f->buffer_pool.buffer[bufix].b.next) )
          child = f->last_pntr[f->buffer_pool.buffer[bufix].b.level][index];
	else {
          done = true;
          child = nulln_ptr;
	  set_error(f,max_key_err,"Search_index, key larger than any in block");
	  if ( log_errors ) {
            print_key(f->log_file,index,k,"  k=");
            fprintf(f->log_file,", index_type=%d, stop_lvl=%d\n",index,stop_lvl);
	  }
	}
      }
      else unpackn_ptr(&(f->buffer_pool.buffer[bufix].b),ix,&child);
    }
    while ( !done );
  return(child);
}

/* static void get_parent_key(struct fcb *f, struct ix_block *mid, struct key *parent_key)
{int parent_level,parent_ix,index,ix; boolean found; struct leveln_pntr p; struct key k;

  parent_key->lc = 0;
  get_nth_key(mid,&k,mid->keys_in_block-1);
  parent_level = mid->level + 1;
  index = mid->index_type;
  p = nulln_ptr;
  if ( parent_level<=f->primary_level[index] ) {
    p = search_index(f,index,parent_level+1,&k);
    parent_ix = get_index(f,p);

    if ( f->trace ) f->search_block_caller = name;
    ix = search_block(f,parent_ix,&k,&found);
    get_nth_key(&(f->buffer_pool.buffer[parent_ix].b),parent_key,ix);
  }
  if ( f->trace ) {
    print_key(f->log_file,index,&k," parent key for k=");
    print_key(f->log_file,index,parent_key," is ");
    fprintf(f->log_file", parent_block is %d/%lu\n",p.segment,p.block);
  }
}*/

static struct leveln_pntr parent_block(struct fcb *f, int bufix, struct key *k)
{int parent_level,index;

/* int parent_ix,ix; boolean found; struct leveln_pntr p;
  parent_level = f->buffer_pool.buffer[bufix].b.level + 1;
  index = f->buffer_pool.buffer[bufix].b.index_type;
  parent_key->lc = 0; prev_key->lc = 0;
  p = nulln_ptr;
  if ( parent_level<=f->primary_level[index] ) {
    p = search_index(f,index,parent_level+1,k);
    parent_ix = get_index(f,p);

    if ( f->trace ) f->search_block_caller = name;
    ix = search_block(f,&(f->buffer_pool.buffer[parent_ix].b),k,&found);
    get_nth_key(&(f->buffer_pool.buffer[parent_ix].b),parent_key,ix);
    get_nth_key(&(f->buffer_pool.buffer[parent_ix].b),prev_key,ix-1);
  }
  if ( f->trace ) {
    print_key(f->log_file,index,k," parent key for k=");
    print_key(f->log_file,index,parent_key," is ");
    print_key(f->log_file,index,prev_key,", prev is ");
    fprintf(f->log_file,"\n");
  }
  if ( !eqn_pntr(p,f->mru_at_level[parent_level][index]) )
    set_error1(f,ix_struct_err,"Parent doesn't match MRU pointer, parent_level=",parent_level);
  return(p);*/

  parent_level = f->buffer_pool.buffer[bufix].b.level + 1;
  index = f->buffer_pool.buffer[bufix].b.index_type;
  return(f->mru_at_level[parent_level][index]);
}

/* file initialization */

static void set_block_shift(struct fcb *f)
{int i;

  i = block_lc;
  f->block_shift = 0;
  while (i>0) {
    i = i>>1;
    if ( i>0 ) f->block_shift++;
  }
}

static boolean machine_is_little_endian()
{int i=1; unsigned char *p=(unsigned char *)&i;

  if ( p[0]==1 ) return(true);
  else return(false);
}


/* init_key initializes the temporary part of the fcb and a few static  */
/*   variables.  It assumes that the fib has been initialized and that  */
/*   the endedness of the machine has been set.                         */

static void init_key(struct fcb *f, char id[], int lc)
{int i,j,hash_target,hash_blocks;;

  /*  printf("  version=%u, subversion=%d\n",f->version,f->sub_version);*/
  if ( f->version!=current_version || f->sub_version!=current_sub_version) { 
    f->error_code = version_err;
    return;
  }
  if ( !check_fcb(f) ) {
    f->error_code = filenotok_err;
    return;
  }
  if ( lc<(int)min_fcb_lc ) { f->error_code = smallfcb_err; return; }
  set_block_shift(f);
  j = 1;
  for (i=0; i<20; i++ ) {
    power_of_two[i] = j;
    j = j * 2;
  }
  f->trace = false; f->trace_freespace = false;
  f->log_file = NULL;
  f->log_file = stdout;
  f->open_file_cnt = 0;
  init_file_name(f,id);
  for (i=0; i<max_segment; i++) f->segment_ix[i] = max_files;
  f->current_age = 0;

  if ( lc==min_fcb_lc ) f->buffer_pool.buffers_allocated = min_buffer_cnt;
  else f->buffer_pool.buffers_allocated = min_buffer_cnt + (lc-min_fcb_lc) / buffer_lc;

  hash_target = f->buffer_pool.buffers_allocated * buf_hash_load_factor;
  hash_blocks = ((hash_target - 1) / hash_entries_per_buf) + 1;
  f->buffer_pool.buf_hash_table = (int *) &(f->buffer_pool.buffer[f->buffer_pool.buffers_allocated-hash_blocks]);
  f->buffer_pool.buf_hash_entries = hash_blocks * hash_entries_per_buf;
  f->buffer_pool.buffers_allocated = f->buffer_pool.buffers_allocated - hash_blocks;
  for (i=0; i<f->buffer_pool.buf_hash_entries; i++) f->buffer_pool.buf_hash_table[i] = -1;
  f->buffer_pool.buffers_in_use = 0;
  f->oldest_buffer = -1;
  f->youngest_buffer = -1;
  for (j=0; j<max_index; j++) {
    f->seq_cnt[j] = 0;
    for (i=0; i<max_level; i++) f->mru_at_level[i][j] = nulln_ptr;
  }
#ifdef log_buffers
  buffer_log = fopen("buffer_log","w");
#endif
}

/* record moving */

/* extract_rec moves the record identified by p into rec */

/*int extract_rec(struct fcb *f,level0_pntr *p, unsigned char rec[], int *rec_lc ,
  unsigned max_rec_lc)
{size_t size; FILE *file;

  if ( check_fcb(f) ) {
    *rec_lc = p->lc;
    if ( (unsigned)*rec_lc>max_rec_lc ) {
      set_error(f,longrec_err,"");
      *rec_lc = max_rec_lc;
    }
    if ( (unsigned)p->lc<=f->data_in_index_lc ) memcpy(rec,&p->data_rec,(size_t) *rec_lc);
    else {
      file = file_index(f,p->segment);
      if ( f->error_code!=no_err ) return(f->error_code);
      if ( fseeko(file,(FILE_OFFSET)p->sc,0)!=0 ) {
        set_error(f,seek_err,"");
        return(f->error_code);
      }
      size = fread(rec,(size_t) 1,(size_t) *rec_lc,file);
      if ( size!=(size_t)*rec_lc ) set_error(f,read_err,"");
    }
  }
  return(f->error_code);
}*/


/* insert_rec moves the record in r into the file.  If the record is   */
/*   sufficiently short it is simply copied into the level0_pntr and   */
/*   will be compressed into the index block.  Otherwise is is written */
/*   to disk.                                                          */

static void insert_rec(struct fcb *f, char r[], level0_pntr *p)
{size_t size,lc; FILE *file;

  if ( f->read_only ) set_error(f,read_only_err,"Attempting insert to read_only file");
  else if ( p->lc<=f->data_in_index_lc ) memcpy(&p->data_rec,r,(size_t)p->lc);
  else {
    file = file_index(f,p->segment);
    if ( file==NULL || f->error_code==segment_open_err ) {
      set_error(f,move_rec_err,"No file in insert_rec");
      return;
    }
    if ( fseeko(file,(FILE_OFFSET)p->sc,0)!=0 ) {
      set_error(f,seek_err,"Seek failed in insert_rec");
      return;
    }
    lc = p->lc;
    size = fwrite(r,(size_t) 1,lc,file);
    if ( size!=lc ) {
      set_error(f,move_rec_err,"Insert_rec failed");
    }
  }
}


/*** buffer handling ***/

static void lock_buffer(f,bufix)
struct fcb *f; int bufix;
{
#if defined(TRACE_LOCKS)
  if ( f->trace )
    fprintf(f->log_file,"locking buffer %d, orig lock_cnt=%d\n",bufix,f->buffer_pool.buffer[bufix].lock_cnt);
#endif
  f->buffer_pool.buffer[bufix].lock_cnt++;
}

static void unlock_buffer(f,bufix)
struct fcb *f; int bufix;
{
  f->buffer_pool.buffer[bufix].lock_cnt--;
#if defined(TRACE_LOCKS)
  if ( f->trace )
    fprintf(f->log_file,"unlocking buffer %d, new lock_cnt=%d\n",bufix,f->buffer_pool.buffer[bufix].lock_cnt);
#endif
}

#define mark_modified(f,bufix) f->buffer_pool.buffer[bufix].modified = true

/* static void mark_modified(f,bufix)
struct fcb *f; int bufix;
{
  f->buffer_pool.buffer[bufix].modified = true;
}*/

static int get_index_update(struct fcb *f, struct leveln_pntr b)
{int bufix;

  bufix = get_index(f,b); mark_modified(f,bufix); return(bufix);
}

/* Space allocation */

/* extend_file extends the current segment by lc characters or   */
/*   opens a new segment if the current segment is full.  A      */
/*   pointer to the newly allocated block is returned in p.      */
/*   allocate_rec and allocate_block manage                      */
/*   segment_cnt.  Note that nospace_err is set when we allocate */
/*   space in the last possible segment even though the          */
/*   allocation succeeds.  It simplifies error handling.         */
/*   Space is always allocated in units that are an integral     */
/*   number of blocks.  The pointer returned by extend file is a */
/*   block number rather than the offset within the segment.     */
/*   This pointer is used directly for block structured data     */
/*   (index and freespace blocks) but must be converted to an    */
/*   offset for data records.                                    */

static boolean extend_file(struct fcb *f, unsigned lc, struct leveln_pntr *p)
{int current,ix;

  current = f->segment_cnt - 1;
  if ( (f->max_file_lc - f->segment_length[current]) < lc ) { /* segment full */
    if ( f->segment_cnt>=(max_segment-1) ) {
      set_error1(f,nospace_err,"Extend_file failed, segment=",current);
    }
    current++;
    ix = file_ix(f,(unsigned)current); /* opens current segment */
    f->segment_cnt++;
  }
  p->segment =current;
  p->block = f->segment_length[current] >> f->block_shift;
  /*  p->block = f->segment_length[current];*/
  f->segment_length[current] = f->segment_length[current] + lc;
  if ( f->trace ) {
    print_leveln_pntr(f->log_file,"  extended file,",p);
    fprintf(f->log_file," lc=%d\n",lc);
  }
  return(f->error_code!=nospace_err);
}

/* deallocate_block places the block in buffer bufix at the front of the free */
/*   block chain.  It assumes that the buffer is marked modified (it needs    */
/*   to be written to preserve the chain).  Any locking required is handled   */
/*   by the caller.   */

static void deallocate_block(struct fcb *f, int bufix)
{int index_type,level; struct leveln_pntr p;

  index_type = f->buffer_pool.buffer[bufix].b.index_type;
  level = f->buffer_pool.buffer[bufix].b.level;
  p = f->buffer_pool.buffer[bufix].contents;

  if ( f->trace_freespace ) {
    print_leveln_pntr(f->log_file,"deallocating block",&p);
    fprintf(f->log_file,"\n");
  }
  f->buffer_pool.buffer[bufix].b.next = f->first_free_block[level][index_type];
  f->first_free_block[level][index_type] = p;

}

/* allocate_block allocates a new block and returns the buffer index */
/*   in which it lies.  If there are any blocks on the free block    */
/*   chain for this index level then the first block is returned.  If the */
/*   free block chain is empty then block_allocation_unit blocks are */
/*   allocated.  One is returned and the rest are initialized,       */
/*   and placed on the free block chain for this level.     */
/* Note that we've tried simply allocating a large pool of blocks    */
/*   returning them one at a time rather than adding them to the     */
/*   free block chain and running them through buffers.  This doesn't*/
/*   work well.  The reason seems to be 1) running them through the  */
/*   buffer pool primes the pool with blocks that are likely to be   */
/*   needed soon (since we are likely to be updating) and 2) the     */
/*   buddy-write we use tends to write large portions of the chain   */
/*   in a single I/O.                                                */

static int allocate_block(struct fcb *f, int index_type, unsigned level)
{int i,bufix=0,temp,blocks_to_allocate,depth; struct leveln_pntr p,p1; block_type_t b; struct key k;

  if ( null_pntr(f->first_free_block[level][index_type]) ) {
    depth = f->primary_level[index_type] - level;
    if ( depth>max_allocation_depth ) depth = max_allocation_depth;
    blocks_to_allocate = block_allocation_unit * power_of_two[depth];
    if ( extend_file(f,(unsigned)(blocks_to_allocate*block_lc),&p) ) {
      p1.segment = p.segment;
      p1.block = p.block + blocks_to_allocate - 1;
      initialize_index_block(&b,index_type,level,&k,(unsigned)0);
      for (i=0; i<blocks_to_allocate-1; i++) {
        temp = vacate_oldest_buffer(f,&p1);
	mark_modified(f,temp);
        initialize_index_block(&(f->buffer_pool.buffer[temp].b),index_type,level,&k,(unsigned)0);
        f->buffer_pool.buffer[temp].b.next = f->first_free_block[level][index_type];
        f->first_free_block[level][index_type] = p1;
        hash_chain_insert(f,temp);
        p1.block--;
      }
      bufix = vacate_oldest_buffer(f,&p);
      mark_modified(f,bufix);
      hash_chain_insert(f,bufix);
    }
  }
  else {
    p = f->first_free_block[level][index_type];
    bufix = get_index_update(f,p);
    mark_modified(f,bufix);
    f->first_free_block[level][index_type] = f->buffer_pool.buffer[bufix].b.next;
  }
  if ( f->trace_freespace ) {
    print_leveln_pntr(f->log_file,"  just allocated block ",&p);
    fprintf(f->log_file," in buf=%d, seg_cnt=%u\n",bufix,f->segment_cnt);
  }
  return(bufix);
}


static int allocate_index_block(struct fcb *f, int index, struct leveln_pntr *b, unsigned lvl,
  struct key *prefix, unsigned prefix_lc)
{int bufix;

  bufix = allocate_block(f,index,lvl);
  initialize_index_block(&(f->buffer_pool.buffer[bufix].b),index,lvl,prefix,prefix_lc);

  f->mru_at_level[lvl][index] = f->buffer_pool.buffer[bufix].contents;
  *b = f->buffer_pool.buffer[bufix].contents;
  return(bufix);
}

/*** Index deletion ***/

/* simple_delete deletes the ix^th key from block b and returns */
/*   the number of bytes deleted (compressed key+compressed     */
/*   pointer+keys entry                                         */

static int simple_delete(struct fcb *f, struct ix_block *b, int ix)
{int i,key_sc,deleted_lc,deleted_sc,plc; struct leveln_pntr pn;

  if (ix>=b->keys_in_block ) {
    set_error1(f,bad_dlt_err,"Attempted to simple delete nonexistent key, ix=",ix);
    return(0);
  }
  else {
    key_sc = keyspace_lc - b->chars_in_use;
    if ( b->level>0 ) plc = unpackn_ptr(b,ix,&pn);
    else plc = unpack0_lc(f,b,ix);
    deleted_lc = key_entry_lc(b,ix) + plc;
    deleted_sc = b->keys[ix];
    mvc(b->keys,key_sc,b->keys,key_sc+deleted_lc,(unsigned)deleted_sc-key_sc);
    b->chars_in_use = b->chars_in_use - deleted_lc;
    b->keys_in_block--;
    for (i=ix; i<b->keys_in_block; i++) b->keys[i] = b->keys[i+1];
    for (i=0; i<b->keys_in_block; i++)
      if (b->keys[i]<deleted_sc )
        b->keys[i] = b->keys[i] + deleted_lc;
    return(deleted_lc+sizeof(UINT16));
  }
}

/* delete_keys deletes cnt keys and pointers from block b.  The keys are */
/*   deleted from positions ix..(ix+cnt-1).  It first copies ix entries  */
/*   to a temporary block buf.  It then copies any entries above ix+cnt-1*/
/*   to buf and then copies the temporary over the original.             */

static void delete_keys(struct fcb *f, struct ix_block *b, int ix, int cnt)
{int i,plc,entry_lc,sc,lc_lc; UINT32 key_lc; struct ix_block buf; struct key k;
#define delete_threshold 1

  if ( f->trace ) {
    fprintf(f->log_file,"**in delete_keys, ix=%d, cnt=%d, b before is\n",ix,cnt);
    /*  fprintf(f->log_file,"**in delete_keys, ix=%d, cnt=%d, b before is\n",ix,cnt);
	print_index_block(f->log_file,f,b);*/
  }

  if ( cnt<=delete_threshold ) {
    for (i=0; i<cnt; i++) simple_delete(f,b,ix);
  }
  else {
    k.lc = b->prefix_lc;
    mvc((char *)b->keys,keyspace_lc-b->prefix_lc,k.text,0,b->prefix_lc);
    initialize_index_block(&buf,b->index_type,b->level,&k,(unsigned)b->prefix_lc);
    for (i=0; i<ix; i++) {
      plc = copy_ptr(f,b,i,&buf);
      get_nth_key(b,&k,i);
      key_lc = k.lc - b->prefix_lc;
      lc_lc = UINT32_lc_if_compressed(key_lc);
      entry_lc = key_lc + lc_lc + plc;
      buf.chars_in_use = buf.chars_in_use + entry_lc;
      b->keys[i] = keyspace_lc - buf.chars_in_use;
      mvc(k.text,b->prefix_lc,buf.keys,b->keys[i]+lc_lc,key_lc);
      compress_UINT32(key_lc,(unsigned char *)buf.keys+b->keys[i]+lc_lc-1);
    }

    for (i=ix+cnt; i<b->keys_in_block; i++) {
      plc = copy_ptr(f,b,i,&buf);
      get_nth_key(b,&k,i);
      key_lc = k.lc - b->prefix_lc;
      lc_lc = UINT32_lc_if_compressed(key_lc);
      entry_lc = key_lc + lc_lc + plc;
      buf.chars_in_use = buf.chars_in_use + entry_lc;
      b->keys[i-cnt] = keyspace_lc - buf.chars_in_use;
      mvc(k.text,b->prefix_lc,buf.keys,b->keys[i-cnt]+lc_lc,key_lc);
      compress_UINT32(key_lc,(unsigned char *)buf.keys+b->keys[i-cnt]+lc_lc-1);
    }
    sc = keyspace_lc - buf.chars_in_use;
    mvc(buf.keys,sc,b->keys,sc,buf.chars_in_use);
    b->keys_in_block = b->keys_in_block - cnt;
    b->chars_in_use = buf.chars_in_use;
  }

  if ( b->keys_in_block==0 ) {
    b->prefix_lc = 0;
    b->chars_in_use = 0;
  }
  
  /*if ( f->trace ) {
    fprintf(f->log_file,"**in delete_keys, ix=%d, cnt=%d, b after is\n",ix,cnt);
    print_index_block(f->log_file,f,b);
    }*/

}

/* remove_primary removes the primary index block and reduces primary_level */
/*   by one (if it is>0).  Note that we do not remove the free_block chain  */
/*   at the old primary level.  If the depth of the index increases later   */
/*   then these free blocks will be reused otherwise they will be unused.   */
/*   They could be removed and added to a chain at a lower level but there  */
/*   cases where the primary is repeatedly removed by a delete then         */
/*   immediately recreated by an insert (esp. in freespace management)      */
/*   which could result in very long freespace chains.                      */

static void remove_primary(struct fcb *f, int index_type)
{int old_primary_level,bufix; struct leveln_pntr b;

  if ( f->trace )
    fprintf(f->log_file,"  removing primary block at level %u, index_type=%d\n",
      f->primary_level[index_type],index_type);
  old_primary_level = f->primary_level[index_type];
  if ( old_primary_level>0 ) {
    b = f->first_at_level[old_primary_level][index_type];
    bufix = get_index_update(f,b);
    f->primary_level[index_type]--;
    deallocate_block(f,bufix);
  }
}

/* remove index block is called when a delete results in an empty index */
/*   block.  Ordinarily the block is removed and the reference to the   */
/*   block in any parent level is removed.  If the block is the last in */
/*   the chain at any level then it is pointed to by last_pntr entry    */
/*   in the parent level and we must make last_pntr point to the        */
/*   previous block at this level and delete the parent reference to    */
/*   that block.  When removing a block from a chain at primary_level-1 */
/*   results in a chain containing a single block then primary_level is */
/*   reduced by one and that singleton becomes the primary.             */

static void remove_index_block(struct fcb *f, struct key *parent_key, int bufix)
{int i,index_type,level,primary_level; struct leveln_pntr next,prev;
struct key max; level0_pntr dummy;

  index_type = f->buffer_pool.buffer[bufix].b.index_type;
  level = f->buffer_pool.buffer[bufix].b.level;
  primary_level = f->primary_level[index_type];
  next = f->buffer_pool.buffer[bufix].b.next;
  prev = f->buffer_pool.buffer[bufix].b.prev;
  if ( null_pntr(next) && null_pntr(prev) ) { /* only block at this level */
    if ( level==0 && primary_level==0 ) ; /* OK */
    else {
      set_error1(f,bad_dlt_err,"Singleton block at level=",level); 
    }
  }
  else if ( null_pntr(prev) ) { /* first block in chain */
    f->first_at_level[level][index_type] = next;
    i = get_index_update(f,next);
    f->buffer_pool.buffer[i].b.prev = nulln_ptr;
    if ( level==primary_level-1 && null_pntr(f->buffer_pool.buffer[i].b.next) )
      remove_primary(f,index_type);
    else if ( level<primary_level ) index_delete(f,index_type,*parent_key,&dummy,level+1);
    deallocate_block(f,bufix);
  }
  else if ( null_pntr(next) ) { /* last block in chain */
    i= get_index_update(f,prev);
    f->buffer_pool.buffer[i].b.next = nulln_ptr;
    if ( level==primary_level-1 && null_pntr(f->buffer_pool.buffer[i].b.prev) )
      remove_primary(f,index_type);
    else if ( level<primary_level ) {
      f->last_pntr[level+1][index_type] = prev;
      get_max_key(&(f->buffer_pool.buffer[i].b),&max);
      index_delete(f,index_type,max,&dummy,level+1);
    }
    deallocate_block(f,bufix);
  }
  else { /* somewhere in the middle of chain */
    i= get_index_update(f,prev);
    f->buffer_pool.buffer[i].b.next = next;
    i = get_index_update(f,next);
    f->buffer_pool.buffer[i].b.prev = prev;
    if ( level<primary_level ) index_delete(f,index_type,*parent_key,&dummy,level+1);
    deallocate_block(f,bufix);
  }
}



/* index_delete removes key k from index block b at level level and   */
/*   returns the pointer p associated with that key.  If key k is not */
/*   found at the given level then error_code is set to dltnokey_err  */
/*   and p is set to null.                                            */

static void index_delete(struct fcb *f, int index_type, struct key k,
  level0_pntr *p, UINT32 level)
{int bufix,ix; boolean found,at_end,update_parent; struct key old_max_key,new_separator;
 struct leveln_pntr b; char *name="ix_delete";

  *p = null0_ptr;
  b = search_index(f,index_type,level+1,&k);
  bufix = get_index(f,b);
  if ( f->buffer_pool.buffer[bufix].b.level!=level ){
    set_error(f,bad_dlt_err,"**Uh oh, wrong level in index delete\n");
  }
  if ( f->trace ) f->search_block_caller = name;
  ix = search_block(f,bufix,&k,&found);
  if ( f->trace ) {
    print_key(f->log_file,index_type,&k,"deleting key=");
    print_leveln_pntr(f->log_file," from block",&b);
    fprintf(f->log_file," keys_in_block(before)=%d, found=%d, ix=%d\n",
      f->buffer_pool.buffer[bufix].b.keys_in_block,found,ix);
  }
  if ( !found ) f->error_code = dltnokey_err;
  else {
    lock_buffer(f,bufix);
    mark_modified(f,bufix);
    at_end = ix==(f->buffer_pool.buffer[bufix].b.keys_in_block-1);
    update_parent = at_end && (ix>0) && (f->primary_level[index_type]>0);
    get_max_key(&(f->buffer_pool.buffer[bufix].b),&old_max_key);
    unpack0_ptr(f,&(f->buffer_pool.buffer[bufix].b),ix,p);
    delete_keys(f,&(f->buffer_pool.buffer[bufix].b),ix,1);
    if ( f->buffer_pool.buffer[bufix].b.keys_in_block==0 ) { /* empty block */
      remove_index_block(f,&old_max_key,bufix);
    }
    else if ( ix==0 ) { /* block not empty, deleted first key */
      check_ix_block_compression(f,&(f->buffer_pool.buffer[bufix].b));
    }
    else if ( at_end ) { /* block not empty, deleted last key */
      get_max_key(&(f->buffer_pool.buffer[bufix].b),&new_separator);
      check_ix_block_compression(f,&(f->buffer_pool.buffer[bufix].b));
      if ( f->primary_level[index_type]>0 && !null_pntr(f->buffer_pool.buffer[bufix].b.next) )
        replace_max_key(f,index_type,&old_max_key,&new_separator,
          f->buffer_pool.buffer[bufix].contents,level+1);
    }
    unlock_buffer(f,bufix);
  }
}

/*** index insertion ***/

/* simple_insert inserts key k and pointer p in entry ix of block b.   */
/*   It can be used when a) it is known that the key and pointer will  */
/*   fit in the block and b) when it is known that the key has the     */
/*   same prefix as the block.  Otherwise, prefix_simple_insert should */
/*   be used.                                                          */

/*static boolean simple_insert(struct fcb *f, struct ix_block *b, int ix, struct key *k, levelx_pntr *p)*/
boolean simple_insert(struct fcb *f, struct ix_block *b, int ix, struct key *k, levelx_pntr *p)
{int i,plc,entry_lc,lc_lc; UINT32 key_lc; boolean ok=true;

  if ( b->level==0 ) plc = pack0_ptr(f,b,&(p->p0));
  else plc = packn_ptr(b,&(p->pn));
  key_lc = k->lc - b->prefix_lc;
  lc_lc = UINT32_lc_if_compressed(key_lc);
  entry_lc = key_lc + lc_lc + plc;
  if ( (b->chars_in_use+entry_lc+(b->keys_in_block+1)*sizeof(UINT16))<=keyspace_lc ) {
    b->keys_in_block++;
    b->chars_in_use = b->chars_in_use + entry_lc;
    for (i=b->keys_in_block-1; i>ix; i--) b->keys[i] = b->keys[i-1];
    b->keys[ix] = keyspace_lc - b->chars_in_use;
    mvc(k->text,b->prefix_lc,b->keys,b->keys[ix]+lc_lc,key_lc);
    compress_UINT32(key_lc,(unsigned char *)b->keys+b->keys[ix]+lc_lc-1);
  }
  else {
    ok = false;
    set_error1(f,insert_err,"Simple insert overflow, entry_lc=",entry_lc);
    if ( log_errors ) {
      print_key(f->log_file,b->index_type,k,"   key=");
      fprintf(f->log_file,", chars_in_use=%d\n",b->chars_in_use);
    }
  }
  return(ok);
}

/* move_keys moves cnt keys and pointers from b to b1.  The first key */
/*   moved is the ix_th entry in b which is moved to the ix1_th entry */
/*   in b1.  Entries in b1 are moved if necessary and any prefix      */
/*   difference between the blocks is accounted for.                  */
/* Note that move_keys does not check or alter the prefix_lc of       */
/*   either block.  In general, the caller should set the prefix_lc   */
/*   of b1 prior to calling.                                          */

static boolean move_keys(struct fcb *f, struct ix_block *b, int ix, struct ix_block *b1, int ix1,
  int cnt)
{int i,current_lc,old_cnt,move_cnt,entry_lc,plc,lc_lc; UINT32 key_lc; boolean ok=true; struct key k;

  current_lc = ix_pool_lc(b1);
  if ( current_lc+(cnt*sizeof(UINT16))<=keyspace_lc ) {
    old_cnt = b1->keys_in_block;
    move_cnt = old_cnt - ix1;
    for (i=1; i<=move_cnt; i++) b1->keys[old_cnt+cnt-i] = b1->keys[old_cnt-i];
    b1->keys_in_block = b1->keys_in_block + cnt;
  }

  i = 0;
  while ( ok && i<cnt ) {
    plc = copy_ptr(f,b,ix+i,b1);
    get_nth_key(b,&k,ix+i);

    key_lc = k.lc - b1->prefix_lc;
    lc_lc = UINT32_lc_if_compressed(key_lc);

    entry_lc = key_lc + lc_lc + plc;
    if ( (b1->chars_in_use+entry_lc+(b1->keys_in_block*sizeof(UINT16)))<=keyspace_lc ) {
      b1->chars_in_use = b1->chars_in_use + entry_lc;
      b1->keys[ix1+i] = keyspace_lc - b1->chars_in_use;
      /*      mvc(k.text,b1->prefix_lc,b1->keys,b1->keys[ix1+i]+lc_lc,key_lc);*/
      memmove((unsigned char *)(b1->keys)+b1->keys[ix1+i]+lc_lc, k.text+b1->prefix_lc,(size_t)key_lc);
      compress_UINT32(key_lc,(unsigned char *)b1->keys+b1->keys[ix1+i]+lc_lc-1);
    }
    else {
      ok = false;
      set_error(f,move_keys_err,"Overflow in move_keys\n");
    }
    i++;
  }

  return(ok);
}


/* compress_ix_block adds or removes prefixes from keys in an index block  */
/*   to give the block a given prefix_lc.  We assume that all keys in the  */
/*   block have a common prefix of length>=prefix_lc.  Decreasing the      */
/*   prefix length (inserts at either end) is never a problem (all keys    */
/*   have the prefix).  Increasing the prefix length (delete or block      */
/*   split) can only occur if all keys have the new prefix length.         */
/* We make a copy of the index pool (but not the keys entries) in copy,    */
/*   then move keys and pointers back into b.                              */

static int compress_ix_block(struct fcb *f,struct ix_block *b, unsigned prefix_lc)
{int i,err=0,prefix_difference,old_key_sc,chars_in_use,old_lc_lc,new_lc_lc,original_prefix_lc,
original_pool_lc;
 UINT32 old_key_lc,new_key_lc;
unsigned expected_pool_lc,pntr_lc;
 struct ix_block copy; struct key prefix; char expansion[max_prefix_lc];/* struct leveln_pntr pn;*/

  if ( b->prefix_lc==prefix_lc ) { /* do nothing */ }
  else {
#ifdef VERIFY_IX_COMPRESS
    struct ix_block temp_block;
    temp_block = *b;
#endif
    original_pool_lc = ix_pool_lc(b);
    original_prefix_lc = b->prefix_lc;
    prefix_difference = b->prefix_lc - prefix_lc;
    expected_pool_lc = new_chars_in_use(b,(int)prefix_lc) + b->keys_in_block * sizeof(UINT16);
    if ( f->trace ) {
      fprintf(f->log_file,"Comprssing ix block from prefix=%d to %d, pool_lc before=%d, after=%d\n",
        b->prefix_lc,prefix_lc,ix_pool_lc(b),expected_pool_lc);
    }
    if ( expected_pool_lc>keyspace_lc ) {
      err = 1;
      set_error2(f,ix_compress_err,"Overflow compressing ix_block, old/new prefix=",
        b->prefix_lc,(int)prefix_lc);
    }
    else if (b->keys_in_block==0 ) { /* nothing to compress */
      b->chars_in_use = 0;
      b->prefix_lc = 0;
    }
    else {
      copy_index_block(b,&copy);
      get_nth_key(b,&prefix,0);
      if ( prefix.lc<prefix_lc && log_errors )
        set_error2(f,ix_compress_err,"Key used for prefix compression too short, lc/prefix_lc=",
          prefix.lc,(int)prefix_lc);
      b->prefix_lc = prefix_lc;
      b->chars_in_use = prefix_lc;
      chars_in_use = b->chars_in_use;
      mvc(prefix.text,0,b->keys,keyspace_lc-prefix_lc,prefix_lc);
      if ( prefix_difference>0 ) mvc(prefix.text,prefix_lc,expansion,0,(unsigned)prefix_difference);
      for (i=0; i<b->keys_in_block; i++) {

        old_key_sc = b->keys[i];
	old_lc_lc = uncompress_UINT32(&old_key_lc,(unsigned char *)copy.keys+old_key_sc);
	new_key_lc = old_key_lc + prefix_difference;
	new_lc_lc = UINT32_lc_if_compressed(new_key_lc);

	pntr_lc = nth_pntr_lc(f,&copy,i);

	if ( f->trace ) {
	  fprintf(f->log_file,"moving key %d, old_sc=%d, old_key_lc=%d/%d, pntr_lc=%d",
            i,old_key_sc,old_key_lc,old_lc_lc,pntr_lc);
	  fprintf(f->log_file,", new_key_lc=%d/%d\n",new_key_lc,new_lc_lc);
	}

        chars_in_use = chars_in_use + new_key_lc + new_lc_lc + pntr_lc;

        b->keys[i] = keyspace_lc - chars_in_use;
        if ( prefix_difference>0 ) {
          mvc(expansion,0,b->keys,b->keys[i]+new_lc_lc,(unsigned)prefix_difference);;
          mvc(copy.keys,old_key_sc+old_lc_lc,b->keys,b->keys[i]+new_lc_lc+prefix_difference,
            new_key_lc+pntr_lc-prefix_difference);
	}
        else {
	  mvc(copy.keys,old_key_sc+old_lc_lc-prefix_difference,b->keys,b->keys[i]+new_lc_lc,
		      new_key_lc+pntr_lc);
	}
        compress_UINT32(new_key_lc,(unsigned char *)b->keys+b->keys[i]+new_lc_lc-1);
      }
      b->chars_in_use = chars_in_use;
      if ( ix_pool_lc(b)!=expected_pool_lc ) {
	err = 2;
	set_error2(f,ix_compress_err,"ix_compress lc != expected, lc/expected=",
          (int)ix_pool_lc(b),(int)expected_pool_lc);
	if ( log_errors ) {
          fprintf(f->log_file,"  orig prefx_lc=%d, prefix_lc=%d, keys=%d, b->level=%d, copy.level=%d\n",
            original_prefix_lc,prefix_lc,b->keys_in_block,b->level,copy.level);
	}
      }
#ifdef VERIFY_IX_COMPRESS
      if ( !eq_block_content(f,&temp_block,b) ) {
	err = 3;
	set_error2(f,ix_compress_err,"Compress_ix_block failed, from/to prefix_lc=",
          temp_block.prefix_lc,(int)prefix_lc);
	if ( log_errors ) {
	  fprintf(f->log_file,"Original block:");
	  print_index_block(f->log_file,f,&temp_block);
	  fprintf(f->log_file,"Compressed block:");
	  print_index_block(f->log_file,f,b);
	}
      }
#endif
    }
  }
  return(err);
}

/* check_ix_block_compression checks that the prefix length recorded in */
/*   the block is consistent with the keys in the block.  If it is not  */
/*   then the block is recompressed.                                    */

static void check_ix_block_compression(struct fcb *f, struct ix_block *b)
{unsigned prefix_lc;

  if ( b->keys_in_block>1 ) {
    prefix_lc = block_prefix_lc(b);
    if ( prefix_lc!=b->prefix_lc ) {
      compress_ix_block(f,b,prefix_lc);
    }
  }
}


/* prefix_simple_insert inserts key k and pointer p in entry ix of block b.  */
/*   If the insert succeeds then true is returned otherwise false is         */
/*   is returned and block b is unchanged.  If the key prefix does not match */
/*   the prefix used in b then b is uncompressed as far as necessary.  The   */
/*   key is inserted and the block is recompressed.                          */


static boolean prefix_simple_insert(struct fcb *f, struct ix_block *b, int ix, struct key *k,
  levelx_pntr *p)
{int err,actual_lc,expected_lc; unsigned prefix_lc; boolean fits,ok=true;

  fits = (unsigned)ix_pool_lc_after_insert(f,b,k,p,ix,&prefix_lc) <= keyspace_lc;
  if ( fits ) {
    if ( ix==0 || ix==b->keys_in_block ) { /* k inserted at beginning or end */
      expected_lc = new_chars_in_use(b,(int)prefix_lc);
      if ( prefix_lc!=b->prefix_lc ) {
        err = compress_ix_block(f,b,prefix_lc);
	if ( err!=0 ) set_error(f,ix_compress_err,"Prefix_simple_insert, compress_ix_block failed");
      }
      actual_lc = b->chars_in_use;
      if ( expected_lc!=actual_lc ) {
        ok = false;
	set_error(f,insert_err,"Prefix_simple_insert failed");
        if ( log_errors ) {
          fprintf(f->log_file,"Prefix_simple_insert at ");
          if ( ix==0 ) fprintf(f->log_file,"beginning got wrong compressed ix block length\n");
          else fprintf(f->log_file,"end got wrong compressed ix block length\n");
	  fprintf(f->log_file,"    orig keys_in_block=%d,",b->keys_in_block);
          print_key(f->log_file,b->index_type,k," key=");
          fprintf(f->log_file,"\n    expected_lc=%d, actual=%d, new_prefix_lc=%d\n",
            expected_lc,actual_lc,prefix_lc);
	}
      }
    }
    ok = ok && simple_insert(f,b,ix,k,p);
    if ( !ok ) set_error(f,insert_err,"**insert failed in prefix_simple_insert\n");
  }
  return(fits);
}

/* replace_max_key is used to replace a max_key in a parent block at level. */
/*   The old max_key is used to search for the entry in the parent block.   */
/*   The pointer found must match the child.  The key value is replaced and */
/*   propagated upward if it replaces the max_key in this (parent) block.   */
/*   However, if the key replacement causes this block to split then it is  */
/*   not necessary to propagate further since split_block will update the   */
/*   parent max_key values.  replace_max_key is similar to         */ 
/*   replace_max_key_and_pntr except that it only replaces the key and can  */
/*   ignore the request if the old and new keys are identical.              */

static void replace_max_key(struct fcb *f, int index, struct key *old_key, struct key *new_key,
  struct leveln_pntr child, unsigned level) {
  int ix=0,bufix=0; 
  unsigned new_prefix_lc=0,new_lc=0; 
  char *name="rep_max_key";
  boolean found=false,propagate=false; 
  struct leveln_pntr p=nulln_ptr; 
  struct key k; 
  levelx_pntr px,childx;

  if ( level>f->primary_level[index] ){
    set_error1(f,repl_max_key_err,"**trying to replace_max_key in level above primary=",(int) level);
  }
  else if ( !eq_key(old_key,new_key) ) {
    p = search_index(f,index,level+1,old_key);
    bufix = get_index_update(f,p);
    if ( f->trace ) f->search_block_caller = name;
    ix = search_block(f,bufix,old_key,&found);
    propagate = (ix==f->buffer_pool.buffer[bufix].b.keys_in_block-1) && (!null_pntr(f->buffer_pool.buffer[bufix].b.next));
    get_nth_key_and_pntr(f,&(f->buffer_pool.buffer[bufix].b),&k,ix,&px);

    if ( f->trace ) {
      fprintf(f->log_file,"  replacing max_key\n");
      print_key(f->log_file,index,old_key,"    old=");
      fprintf(f->log_file,"\n");
      print_key(f->log_file,index,new_key,"    new=");
      fprintf(f->log_file,"\n    level=%u,",level);
      print_leveln_pntr(f->log_file,"child=",&child);
      fprintf(f->log_file," propagate=%d\n",propagate);
    }
    if ( !found || !eqn_pntr(child,px.pn) ) {
      if ( ix==f->buffer_pool.buffer[bufix].b.keys_in_block && null_pntr(f->buffer_pool.buffer[bufix].b.next) ) {/* ok */}
      else {
        set_error(f,ix_struct_err,"Couldn't find entry in replace_max_key");
        if ( log_errors ) {  
          fprintf(f->log_file,"  No entry in replace_max_key, index=%d, found=%d, level=%u\n",
            index,found,level);
          print_key(f->log_file,index,old_key,"  old key=");
          print_key(f->log_file,index,new_key,"\n  new key=");
          print_leveln_pntr(f->log_file,"\n  child=",&child);
          print_leveln_pntr(f->log_file," px=",&(px.pn));
          fprintf(f->log_file,"\n ix=%d, keys_in_block=%d, ",ix,f->buffer_pool.buffer[bufix].b.keys_in_block);
          print_leveln_pntr(f->log_file," next_ptr=",&(f->buffer_pool.buffer[bufix].b.next));
          fprintf(f->log_file,"\n");
	}
      }
    }
    else {
      childx.pn = child;
      new_lc = ix_pool_lc_after_replace(f,&(f->buffer_pool.buffer[bufix].b),new_key,&childx,ix,&new_prefix_lc);
      if ( new_lc <= keyspace_lc) {
        delete_keys(f,&(f->buffer_pool.buffer[bufix].b),ix,1);
        if ( !prefix_simple_insert(f,&(f->buffer_pool.buffer[bufix].b),ix,new_key,&childx) ) {
          set_error(f,repl_max_key_err,"**prefix_simple_insert failed in replace_max_key\n");
	}
      }
      else {
        split_block(f,new_key,&childx,bufix,ix,false);
	propagate = false;
      }
      if ( propagate && level<f->primary_level[index] )
        replace_max_key(f,index,old_key,new_key,p,level+1);
    }
  }
}

/* replace_max_key_and_pntr replaces old_key and old_child in an index block */
/*   at level level with new_key and new_child and propogates new_key upward */
/*   if necessary (new_key is a new max).  Note that the pointer replacement */
/*   occurs even if old_key==new_key.  If propagation is necessary then only */
/*   the key is propagated upward. */

static boolean replace_max_key_and_pntr(struct fcb *f, int index, struct key *old_key, struct key *new_key,
  struct leveln_pntr old_child, struct leveln_pntr new_child, unsigned level)
{int ix,bufix; unsigned new_prefix_lc,new_lc; char *name="rep_maxkey&pntr";
boolean found=false,propagate,split=false; struct leveln_pntr p; struct key k; levelx_pntr px,childx;

  if ( level>f->primary_level[index] ){
    set_error1(f,repl_max_key_err,"Replace_max_key_and_pntr in level above primary=",(int)level);
  }
  else {
    p = search_index(f,index,level+1,old_key);
    bufix = get_index_update(f,p);
    if ( f->trace ) f->search_block_caller = name;
    ix = search_block(f,bufix,old_key,&found);
    propagate = (ix==f->buffer_pool.buffer[bufix].b.keys_in_block-1) && (!null_pntr(f->buffer_pool.buffer[bufix].b.next));
    get_nth_key_and_pntr(f,&(f->buffer_pool.buffer[bufix].b),&k,ix,&px);

    if ( f->trace ) {
      fprintf(f->log_file,"  replacing max_key_and_pntr\n");
      print_key(f->log_file,index,old_key,"    old=");
      fprintf(f->log_file,"\n");
      print_key(f->log_file,index,new_key,"    new=");
      fprintf(f->log_file,"\n    level=%d, ",level);
      print_leveln_pntr(f->log_file,"old_child=",&old_child);
      print_leveln_pntr(f->log_file," new_child=",&new_child);
      fprintf(f->log_file,"propagate=%d\n",propagate);
    }
    if ( !found || !eqn_pntr(old_child,px.pn) ) {
      if ( ix==f->buffer_pool.buffer[bufix].b.keys_in_block && null_pntr(f->buffer_pool.buffer[bufix].b.next) ) {
        if ( !eqn_pntr(f->last_pntr[level][index],old_child) ) {
          set_error(f,ix_struct_err,"Replace_max_key_and_pntr pntr mismatch");
          if ( log_errors ) {
            fprintf(f->log_file,"**last_pntr[%d][%d]",level,index);
            print_leveln_pntr(f->log_file,"=",&(f->last_pntr[level][index]));
            print_leveln_pntr(f->log_file," doesn't match old_child=",&old_child);
            fprintf(f->log_file,"\n");
	  }
	}
        f->last_pntr[level][index] = new_child;
      }
      else {
        set_error(f,ix_struct_err,"No entry in replace_max_key_and_pntr");
        if ( log_errors ) {
          fprintf(f->log_file,"Couldn't find entry in replace_max_key_and_pntr, found=%d, level=%d\n",
            found,level);
          print_key(f->log_file,index,old_key,"  old key=");
          print_key(f->log_file,index,new_key,"\n  new key=");
          print_leveln_pntr(f->log_file,"\n  old_child=",&old_child);
          print_leveln_pntr(f->log_file," px=",&(px.pn));
          fprintf(f->log_file,"\n");
	}
      }
    }
    else {
      childx.pn = new_child;
      new_lc = ix_pool_lc_after_replace(f,&(f->buffer_pool.buffer[bufix].b),new_key,&childx,ix,&new_prefix_lc);
      if ( new_lc<=keyspace_lc) {
        delete_keys(f,&(f->buffer_pool.buffer[bufix].b),ix,1);
        if ( !prefix_simple_insert(f,&(f->buffer_pool.buffer[bufix].b),ix,new_key,&childx) ) {
          set_error(f,repl_max_key_err,"Prefix_simple_insert failed in replace_max_key_and_pntr");
	}
      }
      else {
        split = true;
        split_block(f,new_key,&childx,bufix,ix,false);
	propagate = false;
      }
      if ( propagate && level<f->primary_level[index] ) replace_max_key(f,index,old_key,new_key,p,level+1);
    }
  }
  return(split);
}

/* moving keys -- on input we are given two blocks; a mid block from */
/*   which keys will be moved and either a rt or lt block into which */
/*   they will be moved.  We are also given a new key and pointer that */
/*   will either be inserted (insert=true) in position ix or will    */
/*   replace (insert=false) the entry in position ix.  We always do  */
/*   cnt inserts to the new block; one of these inserts will be the  */
/*   new key if ix is in the move range.  When insert=true the total */
/*   number of keys to be distributed between blocks is one greater  */
/*   than the original content.  If ix is in the move range we will  */
/*   do cnt-1 deletes to mid otherwise we will do cnt deletes.  When */
/*   insert is false the end number of keys is the same as the       */
/*   original and we will always do cnt deletes to mid.  Note that   */
/*   if ix is not in the move range then the caller must decide what */
/*   to do with the new key.                                         */
/* move_keys_to_right sets the prefix and compresses the right       */
/*   block, move_keys_to_left sets the prefix and compresses the     */
/*   left_block.  The caller must compress the original block.       */


/* set_rt_prefix_lc computes the prefix_lc for block rt that would   */
/*   result if move_cnt keys are moved to it from lt (plus the new   */
/*   key k).  It also returns the max_key from rt.                   */

static unsigned set_rt_prefix_lc(struct fcb *f, struct ix_block *lt, struct ix_block *rt, int move_cnt,
  struct key *k, int ix, boolean insert, struct key *rt_max)
{int first_ix; unsigned prefix_lc; boolean move_new_key=false; struct key rt_min;


  if ( move_cnt==0 ) {
    prefix_lc = rt->prefix_lc;
    if ( rt->keys_in_block>0 ) get_max_key(rt,rt_max);
    else rt_max->lc = 0;
  }
  else {
    if (ix<(lt->keys_in_block+insert) && ix>=lt->keys_in_block-move_cnt+insert ) move_new_key = true;

    if ( rt->keys_in_block==0 && ix==lt->keys_in_block+insert-1 ) copy_key(k,rt_max);
    else if ( rt->keys_in_block>0 ) get_max_key(rt,rt_max);
    else get_max_key(lt,rt_max);

    first_ix = lt->keys_in_block - move_cnt;
    if ( (ix-insert)==first_ix ) copy_key(k,&rt_min);
    else if ( insert && ((ix-insert)>first_ix) ) get_nth_key(lt,&rt_min,first_ix+1);
    else get_nth_key(lt,&rt_min,first_ix);

    if ( (rt->keys_in_block+move_cnt)>1 ) prefix_lc = find_prefix_lc(&rt_min,rt_max);
    else prefix_lc = 0;

  }
  return(prefix_lc);
}

/* set_lt_prefix_lc computes the prefix_lc for block lt that would   */
/*   result if move_cnt keys are moved to it from rt (plus the new   */
/*   key k).  The number of keys in rt is assumed to be rt_cnt       */
/*   rather than rt->keys_in_block.  Unlike set_rt_prefix_lc it does */
/*   not return the max_key.                                         */

static unsigned set_lt_prefix_lc(struct fcb *f, struct ix_block *lt, struct ix_block *rt, int move_cnt,
  int rt_cnt, struct key *k, int ix, boolean insert)
{unsigned prefix_lc; boolean move_new_key=false; struct key lt_max,lt_min;


  if ( move_cnt==0 ) {
    prefix_lc = lt->prefix_lc;
  }
  else {
    if (ix<(rt_cnt+insert) && ix<=(move_cnt-1) ) move_new_key = true;

    if ( ix==move_cnt-1 ) copy_key(k,&lt_max);
    else if ( insert && move_new_key ) get_nth_key(rt,&lt_max,move_cnt-2);
    else get_nth_key(rt,&lt_max,move_cnt-1);

    if ( lt->keys_in_block==0 && ix==0 ) copy_key(k,&lt_min);
    else if ( lt->keys_in_block==0 ) get_nth_key(rt,&lt_min,0);
    else get_nth_key(lt,&lt_min,0);
    if ( (lt->keys_in_block+move_cnt)>1 ) prefix_lc = find_prefix_lc(&lt_min,&lt_max);
    else prefix_lc = 0;

  }
  return(prefix_lc);
}

/* test_set_prefix_lc is used to test the set_xx_prefix_lc functions.   */
/*   These functions are hard to get right so generating a useful range */
/*   of test cases is helpful.  If changes are made to either of these  */
/*   functions it's a good idea to test using this.  Uncomment the      */
/*   the print statements in the fn under test and call this fn from    */
/*   create_key_ld.                                                     */

#if 0
static void test_set_prefix_lc(struct fcb *f)
{int i,seed,lt_seed=100,mid_seed=200,rt_seed=300,ix,move_cnt,mid_cnt=10; struct ix_block lt,mid,rt;
struct key k,max; levelx_pntr p;
 boolean insert=true; unsigned prefix_lc;

  p.p0 = dummy_ptr;
  initialize_index_block(&lt, user_ix,0,&k,0);
  initialize_index_block(&mid,user_ix,0,&k,0);
  initialize_index_block(&rt, user_ix,0,&k,0);

  for (i=0; i<10; i++) {
    seed = mid_seed + i*10 + 1;
    sprintf(k.text,"%d",seed);
    k.lc = strlen(k.text);
    simple_insert(f,&mid,i,&k,&p);
  }

  sprintf(k.text,"%d",lt_seed + 0*10 + 1);
  k.lc = strlen(k.text);
  simple_insert(f,&lt,0,&k,&p);
  sprintf(k.text,"%d",rt_seed + 0*10 + 1);
  k.lc = strlen(k.text);
  simple_insert(f,&rt,0,&k,&p);

  fprintf(f->log_file,"Beginning set_prefix_lc test, insert=%d\n",insert);
  fprintf(f->log_file,"lt:");
  print_index_block(f->log_file,,f,&lt);
  fprintf(f->log_file,"mid:");
  print_index_block(f->log_file,f,&mid);
  fprintf(f->log_file,"rt:");
  print_index_block(f->log_file,f,&rt);

  strcpy(k.text,"new_key");
  k.lc = strlen(k.text);
  for (ix=0; ix<=10; ix++) {
    for (move_cnt=0; move_cnt<=10; move_cnt++) {
      /*      prefix_lc = set_rt_prefix_lc(f,&mid,&rt,move_cnt,&k,ix,insert,&max);*/
      prefix_lc = set_lt_prefix_lc(f,&lt,&mid,move_cnt,mid_cnt,&k,ix,insert);
    }
  }
}
#endif

static boolean move_keys_to_right(struct fcb *f, struct ix_block *mid, struct ix_block *rt, int cnt,
  struct key *new_key, levelx_pntr *new_p, int ix, boolean insert)
{int first,above_ix_cnt,below_ix_cnt; unsigned prefix_lc,orig_rt_prefix_lc;
boolean ok=true,move_new_key=false; struct key max_key;

 orig_rt_prefix_lc = rt->prefix_lc;

  if ( cnt>0 ) {
    prefix_lc = set_rt_prefix_lc(f,mid,rt,cnt,new_key,ix,insert,&max_key);
    if ( rt->keys_in_block==0 ) set_empty_block_prefix(rt,&max_key,prefix_lc);
    else if ( rt->prefix_lc!=prefix_lc ) {
      compress_ix_block(f,rt,prefix_lc);
    }


    if (ix<(mid->keys_in_block+insert) && ix>=mid->keys_in_block-cnt+insert ) move_new_key = true;
    if ( move_new_key && insert ) first = mid->keys_in_block - cnt + 1;
    else first = mid->keys_in_block - cnt;

    if ( cnt==0 ) { /* do nothing */ }
    else if ( move_new_key ) {
      above_ix_cnt = mid->keys_in_block - (ix+1-insert);
      below_ix_cnt = cnt - above_ix_cnt - 1;
      ok = move_keys(f,mid,ix+1-insert,rt,0,above_ix_cnt);
      ok = ok && simple_insert(f,rt,0,new_key,new_p);
      ok = ok && move_keys(f,mid,ix-below_ix_cnt,rt,0,below_ix_cnt);
      delete_keys(f,mid,ix-below_ix_cnt,cnt-insert);
    }
    else {
      ok = move_keys(f,mid,first,rt,0,cnt);
      delete_keys(f,mid,first,cnt);
    }

    if ( !ok ) {
      set_error(f,move_keys_err,"Move_keys failed in move_keys_to_right");
      fprintf(f->log_file,"  move_cnt=%d, mid_prefix=%d\n",cnt,mid->prefix_lc);
      fprintf(f->log_file,"    rt_prefix=%d(%d on entry)set_prefix=%d  ix=%d, insert=%d\n",
        rt->prefix_lc,orig_rt_prefix_lc,prefix_lc,ix,insert);
    }
  }
  return(move_new_key);
}

/* chars_after_move computes the size and prefix_lc of mid after lt_cnt     */
/*   keys are moved */
/*   to the left and rt_cnt keys are moved to the right.  It is generally   */
/*   called before any keys have actually been moved in which case          */
/*   key_in_mid should be passed in as true.  It can also be called after   */
/*   keys have allready been moved to the right.  In this case rt_cnt will  */
/*   be 0 and key_in_mid should be set to false if the new key was moved to */
/*   the right, true if not.
 */

static int chars_after_move(struct fcb *f, struct ix_block *mid, int lt_cnt, int rt_cnt,
struct key *k, levelx_pntr *p, int ix_in, boolean insert, boolean key_in_mid, unsigned *prefix_lc)
{int first,last=0,moved_cnt,mid_cnt,mid_lc,ix; boolean new_key_in_mid,moved_right=false;
struct key min_key,max_key;


  new_key_in_mid = key_in_mid;
  ix = ix_in;

  moved_cnt = lt_cnt + rt_cnt;
  if ( new_key_in_mid ) mid_cnt = mid->keys_in_block - moved_cnt + insert;
  else {
    mid_cnt = mid->keys_in_block - moved_cnt;
    moved_right = true;
    ix = mid->keys_in_block + 1;
  }

  if ( mid_cnt<=0 ) {
    mid_lc = 0;
    *prefix_lc = 0;
  }
  else {
    if ( ix<lt_cnt ) {
      if ( insert ) first = lt_cnt - 1;
      else first = lt_cnt;
      get_nth_key(mid,&min_key,first);
    }
    else if ( ix==lt_cnt ) {
      copy_key(k,&min_key);
      if ( insert ) first = lt_cnt;
      else first = lt_cnt + 1;
    }
    else /* ix>lt_cnt */ {
      first = lt_cnt;
      get_nth_key(mid,&min_key,first);
    }

    if ( new_key_in_mid ) {
      last = mid->keys_in_block - rt_cnt;
      if ( ix>=last+insert ) moved_right = true;

      if      ( ix< (last-1) ) get_nth_key(mid,&max_key,last-1);
      else if ( ix==(last-1) ) {
	if ( insert ) get_nth_key(mid,&max_key,last-1);
	else copy_key(k,&max_key);
      }
      else if ( ix==last ) {
	if ( insert ) copy_key(k,&max_key);
	else get_nth_key(mid,&max_key,last-1);
      }
      else /* ix>last */ {
	if ( insert ) get_nth_key(mid,&max_key,last);
	else get_nth_key(mid,&max_key,last-1);
      }

    }
    else {
      get_nth_key(mid,&max_key,mid->keys_in_block-rt_cnt-1);
    }

    if ( mid_cnt>1 ) *prefix_lc = find_prefix_lc(&min_key,&max_key);
    else *prefix_lc = 0;

    if ( ix<lt_cnt ) {
      mid_lc = ix_entries_lc(f,mid,first,mid_cnt,*prefix_lc);
    }
    else if ( ix==lt_cnt ) {
      mid_lc = ix_entry_lc(f,k,p,*prefix_lc,mid->level)
          + ix_entries_lc(f,mid,first,mid_cnt-1,*prefix_lc);
    }
    else /* ix>lt_cnt */ {
      if ( moved_right ) {
        mid_lc =  ix_entries_lc(f,mid,first,mid_cnt,*prefix_lc);
      }
      else if ( insert ) {
        mid_lc = ix_entry_lc(f,k,p,*prefix_lc,mid->level)
          + ix_entries_lc(f,mid,first,mid_cnt-1,*prefix_lc);
      }
      else {
        mid_lc = ix_entry_lc(f,k,p,*prefix_lc,mid->level)
          + ix_entries_lc(f,mid,first,mid_cnt,*prefix_lc)
          - ix_entries_lc(f,mid,ix,1,*prefix_lc);
      }
    }

    mid_lc = mid_lc + *prefix_lc + mid_cnt * sizeof(UINT16);

  }
  return(mid_lc);
}


static boolean move_keys_to_left(struct fcb *f, struct ix_block *lt, struct ix_block *mid, int cnt,
  struct key *new_key, levelx_pntr *new_p, int ix, boolean insert, boolean new_key_in_mid)
{int i,err=0,next=0,delete_cnt=0,new_ix; unsigned mid_lc,lt_prefix_lc,mid_prefix_lc;
boolean ok=true,moved_new_key=false; struct key k; levelx_pntr p; 

/*struct ix_block temp_lt,temp_mid;
  temp_lt = *lt;
  temp_mid = *mid;*/

    lt_prefix_lc = set_lt_prefix_lc(f,lt,mid,cnt,mid->keys_in_block,new_key,ix,insert);
    if ( f->trace ) fprintf(f->log_file,"  moving %d keys_to_left, orig key_cnts=%d/%d, lt_prefix_lc=%d\n",cnt,
      lt->keys_in_block,mid->keys_in_block,lt_prefix_lc); 
    if ( lt->keys_in_block==0 ) {
      if ( ix==0 ) set_empty_block_prefix(lt,new_key,lt_prefix_lc);
      else {
        get_nth_key(mid,&k,0);
        set_empty_block_prefix(lt,&k,lt_prefix_lc);
      }
    }
    else if ( lt->prefix_lc!=lt_prefix_lc ) {
      err = compress_ix_block(f,lt,lt_prefix_lc);
      if ( err!=0  ) {
        set_error(f,ix_compress_err,"compress_ix(lt) failed in move_keys_to_left");
      }
    }

    mid_lc = chars_after_move(f,mid,cnt,0,new_key,new_p,ix,insert,new_key_in_mid,&mid_prefix_lc);

    for (i=0; i<cnt; i++)
      if ( next<ix ) {
        get_nth_key_and_pntr(f,mid,&k,next,&p);

        delete_cnt++;
        ok = ok && simple_insert(f,lt,lt->keys_in_block,&k,&p);

        next++;
      }
      else if ( next==ix ) {
        moved_new_key = true;
        if ( !insert ) delete_cnt++;
        ok = ok && simple_insert(f,lt,lt->keys_in_block,new_key,new_p);
        next++;
      }
      else { 
        get_nth_key_and_pntr(f,mid,&k,next-insert,&p);
        delete_cnt++;
        ok = ok && simple_insert(f,lt,lt->keys_in_block,&k,&p);
        next++;
      }
    delete_keys(f,mid,0,delete_cnt);

    if ( new_key_in_mid && !moved_new_key ) {
      new_ix = ix - cnt;
      if ( !insert ) delete_keys(f,mid,new_ix,1);
      compress_ix_block(f,mid,mid_prefix_lc);
      if ( err!=0 && log_errors ) {
        set_error(f,ix_compress_err,"compress_ix(mid) failed in move_keys_to_left");
      }
      ok = ok && simple_insert(f,mid,new_ix,new_key,new_p);
    }
    else compress_ix_block(f,mid,mid_prefix_lc);

    if ( ix_pool_lc(mid)!=mid_lc ) {
      set_error2(f,move_keys_err,"Move_keys_to_left lc mismatch, actual/expected_lc=",
	(int)ix_pool_lc(mid),(int)mid_lc);
      print_key(f->log_file,lt->index_type,new_key,"  insert key=");
      fprintf(f->log_file,", ix=%d, insert=%d, move_cnt=%d\n",ix,insert,cnt);
      fprintf(f->log_file,"  expected mid_prefix_lc=%d, actual=%d, new_key_in_mid=%d\n",
        mid_prefix_lc,mid->prefix_lc,new_key_in_mid);
      /*      fprintf(f->log_file,"  lt before move is:\n");
      print_index_block(f->log_file,f,&temp_lt);
      fprintf(f->log_file,"  mid before move is:\n");
      print_index_block(f->log_file,f,&temp_mid);
      fprintf(f->log_file,"  mid after move is:\n");
      print_index_block(f->log_file,f,mid);*/
    }

    if ( !ok && log_errors ) {
      set_error2(f,move_keys_err,"Simple insert failed in move_keys_to_left, keys/lt_prefix=",
        cnt,(int)lt_prefix_lc);
      print_key(f->log_file,lt->index_type,new_key,"    insert key=");
      fprintf(f->log_file,", ix=%d, insert=%d\n",ix,insert);
      /*      fprintf(f->log_file,"  lt before move is:\n");
      print_index_block(f->log_file,f,&temp_lt);
      fprintf(f->log_file,"  mid before move is:\n");
      print_index_block(f->log_file,f,&temp_mid);*/
    }

  return(moved_new_key);
}

#if 0
static void test_chars_after_move(struct fcb *f)
{int i,seed,mid_seed=200,ix,lt_cnt,rt_cnt;
 unsigned mid_lc;
struct ix_block mid,temp,saved_mid;
struct key k; levelx_pntr p;
 boolean insert=false,moved; unsigned prefix_lc;

  p.p0 = dummy_ptr;
  initialize_index_block(&saved_mid,user_ix,0,&k,0);

  for (i=0; i<10; i++) {
    seed = mid_seed + i*10 + 1;
    sprintf(k.text,"%d",seed);
    k.lc = strlen(k.text);
    simple_insert(f,&saved_mid,i,&k,&p);
  }
  check_ix_block_compression(f,&saved_mid);
  mid = saved_mid;
  printf("Beginning chars_after_move test, insert=%d\n",insert);
  printf("mid:");
  print_index_block(stdout,f,&mid);

  strcpy(k.text,"new_key");
  k.lc = strlen(k.text);
  for (ix=0; ix<10; ix++) {
    seed = mid_seed + ix*10 + 0;
    sprintf(k.text,"%d",seed);
    strcat(k.text,"*");
    k.lc = strlen(k.text);
    for (lt_cnt=0; lt_cnt<=10; lt_cnt++) {
      for ( rt_cnt=0; rt_cnt<=(10-lt_cnt); rt_cnt++) {
	mid = saved_mid;
        mid_lc = chars_after_move(f,&mid,lt_cnt,rt_cnt,&k,&p,ix,insert,true,&prefix_lc);
        initialize_index_block(&temp,user_ix,0,&k,0);
        moved = move_keys_to_right(f,&mid,&temp,rt_cnt,&k,&p,ix,insert);
        initialize_index_block(&temp,user_ix,0,&k,0);
        move_keys_to_left(f,&temp,&mid,lt_cnt,&k,&p,ix,insert,!moved);
	if ( mid_lc!=ix_pool_lc(&mid) ) {
          printf("**mid_lc=%d doesn't match actual=%d, expected prefix=%d\n",mid_lc,
            ix_pool_lc(&mid),prefix_lc);
          print_index_block(stdout,f,&mid);
	}
	if ( prefix_lc!=mid.prefix_lc ) {
          printf("**prefix_lc=%d doesn't match actual=%d\n",prefix_lc,mid.prefix_lc);
          print_index_block(stdout,f,&mid);
	}
      }
    }
  }
}
#endif

/* lc_if_move_right returns the key and prefix lengths of rt that  */
/*   would result if move_cnt keys were moved from lt to rt.  It    */
/*   also returns the max_key in rt to be used to set the prefix.   */ 

static int lc_if_move_right(struct fcb *f, struct ix_block *lt, struct ix_block *rt, int cnt,
  struct key *k, levelx_pntr *p, int ix, boolean insert,
  boolean *move_new_key, unsigned *rt_prefix_lc, struct key *rt_max)
{int first_ix,move_cnt; unsigned rt_lc; struct key temp;

  *move_new_key = false;
  move_cnt = cnt;
  if (ix<(lt->keys_in_block+insert) && ix>=lt->keys_in_block-move_cnt+insert ) *move_new_key = true;

  if (move_cnt==0 || move_cnt>lt->keys_in_block+insert) {
    rt_lc = ix_pool_lc(rt);
    *rt_prefix_lc = rt->prefix_lc;
    get_max_key(rt,rt_max);
  }
  else {

    /*    if ( rt->keys_in_block==0 && ix==lt->keys_in_block+insert-1 ) copy_key(k,rt_max);
    else if ( rt->keys_in_block>0 ) get_max_key(rt,rt_max);
    else get_max_key(lt,rt_max);*/

    first_ix = lt->keys_in_block - move_cnt;
    if ( insert && *move_new_key ) {
      first_ix++;
      move_cnt--;
    }

    *rt_prefix_lc = set_rt_prefix_lc(f,lt,rt,cnt,k,ix,insert,&temp);


    rt_lc = new_chars_in_use(rt,(int)*rt_prefix_lc)
      + ix_entries_lc(f,lt,first_ix,move_cnt,*rt_prefix_lc);
    if ( *move_new_key && insert ) rt_lc = rt_lc + ix_entry_lc(f,k,p,*rt_prefix_lc,rt->level);
    else if ( *move_new_key ) {
      rt_lc = rt_lc + ix_entry_lc(f,k,p,*rt_prefix_lc,rt->level);
      rt_lc = rt_lc - ix_entries_lc(f,lt,ix,1,*rt_prefix_lc);
    }
    rt_lc = rt_lc + (rt->keys_in_block+cnt) * sizeof(UINT16);
  }
  return(rt_lc);
}

/* lc_if_move_left returns the keys and prefix lengths that would */
/*   result if move_cnt keys were moved from rt to lt.  It is      */
/*   similar to lc_if_move_right except that it assumes that rt   */
/*   contains rt_cnt<=rt->keys_in_block keys.                      */ 

static int lc_if_move_left(struct fcb *f, struct ix_block *lt, struct ix_block *rt,
int move_cnt, int rt_cnt, struct key *k, levelx_pntr *p, int ix, boolean insert,
boolean *move_new_key, unsigned *lt_prefix_lc, struct key *lt_max)
{int move_lc,lt_lc;

  *move_new_key = false;
  if (ix<(rt_cnt+insert) && ix<=(move_cnt-1) ) *move_new_key = true;

  if ( move_cnt==0 || move_cnt>rt_cnt+(*move_new_key) ) {
    lt_lc = ix_pool_lc(lt);
    *lt_prefix_lc = lt->prefix_lc;
    get_max_key(lt,lt_max);
  }
  else {

    *lt_prefix_lc = set_lt_prefix_lc(f,lt,rt,move_cnt,rt_cnt,k,ix,insert);


    if ( (*move_new_key) && insert ) {
      move_lc = ix_entries_lc(f,rt,0,move_cnt-1,*lt_prefix_lc)
         + ix_entry_lc(f,k,p,*lt_prefix_lc,lt->level);
    }
    else if ( *move_new_key ) {
      move_lc = ix_entries_lc(f,rt,0,move_cnt,*lt_prefix_lc)
        + ix_entry_lc(f,k,p,*lt_prefix_lc,lt->level)
        - ix_entries_lc(f,rt,ix,1,*lt_prefix_lc);
    }
    else move_lc = ix_entries_lc(f,rt,0,move_cnt,*lt_prefix_lc);
    lt_lc = new_chars_in_use(lt,(int)*lt_prefix_lc) + move_lc;

    lt_lc = lt_lc + (lt->keys_in_block+move_cnt) * sizeof(UINT16);

  }
  return(lt_lc);
}

static boolean check_ix_block_after_move(struct fcb *f, struct ix_block *b, char caption[],
int expected_cnt, unsigned expected_lc, unsigned expected_prefix_lc)
{boolean ok=true;

  if ( b->keys_in_block!=expected_cnt ) {
    ok = false;
    set_error2(f,move_keys_err,"key count wrong after move actual/expected keys_in_block=",
      expected_cnt,b->keys_in_block);
    fprintf(f->log_file,"  caller=%s\n",caption);
  }
  if ( ix_pool_lc(b)!=expected_lc ) {
    ok = false;
    set_error2(f,move_keys_err,"pool_lc wrong after move actual/expected pool_lc=",
      (int)expected_lc,(int)ix_pool_lc(b));
    fprintf(f->log_file,"  caller=%s\n",caption);
  }
  if ( b->prefix_lc!=expected_prefix_lc ) {
    ok = false;
    set_error2(f,move_keys_err,"prefix_lc!=expected after move actual/expected prefix_lc=",
      (int)expected_prefix_lc,b->prefix_lc);
    fprintf(f->log_file,"  caller=%s\n",caption);
  }
  if ( b->prefix_lc!=block_prefix_lc(b) ) {
    ok = false;
    set_error2(f,move_keys_err,"prefix_lc wrong after move is/should be=",
      b->prefix_lc,block_prefix_lc(b));
    fprintf(f->log_file,"  caller=%s\n",caption);
  }
  return(ok);
}

#ifdef VERIFY_SHUFFLES
static void check_shuffle(struct fcb *f, struct ix_block *lt, struct ix_block *new_lt,
  struct ix_block *mid, struct ix_block *new_mid, struct ix_block *rt, struct ix_block *new_rt,
  struct key *k, levelx_pntr *p, int ix, boolean insert)
{int i,lt_i,mid_i,lt_j,mid_j,index,original_key_cnt,new_key_cnt; boolean same_pntr;
struct key orig_key,new_key; levelx_pntr orig_p,new_p;

  original_key_cnt = lt->keys_in_block + mid->keys_in_block + rt->keys_in_block + insert;
  new_key_cnt = new_lt->keys_in_block + new_mid->keys_in_block + new_rt->keys_in_block;
  if ( original_key_cnt!=new_key_cnt )
    printf("** key_cnt mismatch after shuffle, orig_cnt=%d, new_cnt=%d\n",original_key_cnt,new_key_cnt);
  lt_i =  lt->keys_in_block;
  mid_i = mid->keys_in_block;
  lt_j =  new_lt->keys_in_block;
  mid_j = new_mid->keys_in_block;
  index = lt->index_type;
  for (i=0; i<new_key_cnt; i++) {
    if   ( i<lt_i )       get_nth_key_and_pntr(f,lt,&orig_key,i,&orig_p);
    else if ( i<lt_i+mid_i+insert ) {
      if ( (i-lt_i) < ix ) get_nth_key_and_pntr(f,mid,&orig_key,i-lt_i,&orig_p);
      else if ( i-lt_i==ix ) {orig_key = *k; orig_p = *p; }
      else get_nth_key_and_pntr(f,mid,&orig_key,i-lt_i-insert,&orig_p);
    }
    else                  get_nth_key_and_pntr(f,rt,&orig_key,i-lt_i-mid_i-insert,&orig_p);
    if   ( i<lt_j )       get_nth_key_and_pntr(f,new_lt,&new_key,i,&new_p);
    else if ( i<lt_j+mid_j ) get_nth_key_and_pntr(f,new_mid,&new_key,i-lt_j,&new_p);
    else                  get_nth_key_and_pntr(f,new_rt,&new_key,i-lt_j-mid_j,&new_p);
    if ( !eq_key(&orig_key,&new_key) ) {
      print_key(stdout,index,&orig_key,"**Key mismatch, orig_key=");
      print_key(stdout,index,&new_key,", new_key=");
      printf(", i=%d\n",i);
    }
    if ( lt->level>0 ) same_pntr = eqn_pntr(orig_p.pn,new_p.pn);
    else same_pntr = eq0_pntr(f,&(orig_p.p0),&(new_p.p0));
    if ( !same_pntr ) {
      printf("**Pointer mismatch, i=%d\n",i);
    }
  }
}
#endif

#define feasible_move(a,b,c) ( (a<=keyspace_lc) && (b<=keyspace_lc) && (c<=keyspace_lc) )

/*static unsigned distance_from_mean(unsigned lt_lc, unsigned mid_lc, unsigned rt_lc)
{int mean,distance;

  mean =  (int) (((lt_lc + mid_lc + rt_lc) / 3.0 ) + 0.5);
  distance = abs((int)lt_lc-mean) + abs((int)mid_lc-mean) + abs((int)rt_lc-mean);
  return(distance);
} */

static unsigned distance_from_min(struct shuffle_candidate *c)
{unsigned smallest,distance;

  smallest = c->lt_lc;
  if ( c->mid_lc<smallest ) smallest = c->mid_lc;
  if ( c->rt_lc<smallest ) smallest = c->rt_lc;

  distance = (c->lt_lc-smallest) + (c->mid_lc-smallest) + (c->rt_lc-smallest);
  return(distance);
}

static void print_shuffle_candidate(FILE *list,char caption[], struct shuffle_candidate *c)
{
  fprintf(list,"%s=%d/%d (%d/%d)(%d/%d)(%d/%d) (tot=%d, dist=%d)\n",caption,
    c->lt_move_cnt,c->rt_move_cnt,
    c->lt_lc,c->lt_prefix_lc,c->mid_lc,c->mid_prefix_lc,c->rt_lc,c->rt_prefix_lc,
    c->lt_lc+c->mid_lc+c->rt_lc,distance_from_min(c));
} 

/* find_shuffle_candidates checks to see if it is possible to move */
/*   keys out of mid into rt or lt and returns a list of plausible */
/*   moves. It tries moving keys out of mid to the smaller of the  */
/*   two blocks until either both lt and rt are full or they are   */
/*   the same size or larger than mid.  For each plausible move,   */
/*   the list of current candidates is searched.  If the current   */
/*   move is better than one in the list it replaces the first     */
/*   move such move otherwise it is added to the list.             */

static boolean better_shuffle_move(struct shuffle_candidate *c1, struct shuffle_candidate *c2)
{int size1,size2; unsigned distance1,distance2;

  size1 = c1->lt_lc + c1->mid_lc + c1->rt_lc;
  size2 = c2->lt_lc + c2->mid_lc + c2->rt_lc;
  /*  distance1 = distance_from_mean(c1->lt_lc,c1->mid_lc,c1->rt_lc);
      distance2 = distance_from_mean(c2->lt_lc,c2->mid_lc,c2->rt_lc);*/
  distance1 = distance_from_min(c1);
  distance2 = distance_from_min(c2);
  return( (size1<size2) || ( (size1==size2) && (distance1<distance2) ) );

}

static void add_candidate_move(struct shuffle_candidate candidate[], int *candidate_cnt,
int lt_move_cnt, int rt_move_cnt,
unsigned lt_lc, unsigned lt_prefix_lc, unsigned mid_lc, unsigned mid_prefix_lc,
unsigned rt_lc, unsigned rt_prefix_lc)
{int i; boolean replaced=false; struct shuffle_candidate temp;

  temp.lt_move_cnt    = lt_move_cnt;
  temp.rt_move_cnt    = rt_move_cnt;
  temp.lt_lc          = lt_lc;
  temp.lt_prefix_lc   = lt_prefix_lc;
  temp.mid_lc         = mid_lc;
  temp.mid_prefix_lc  = mid_prefix_lc;
  temp.rt_lc          = rt_lc;
  temp.rt_prefix_lc   = rt_prefix_lc;
  if ( *candidate_cnt==0 ) {
    candidate[0] = temp;
    (*candidate_cnt)++;
  }
  else {
    for ( i=0; i<(*candidate_cnt) && !replaced; i++) {
      if ( better_shuffle_move(&temp,&(candidate[i])) ) {
        candidate[i] = temp;
	replaced = true;
      }
    }
    if ( !replaced ) {
      candidate[*candidate_cnt] = temp;
      if ( (unsigned)(*candidate_cnt)<key_ptrs_per_block ) (*candidate_cnt)++;
    }
  }
}

static int pick_shuffle_candidate(struct fcb *f,struct shuffle_candidate candidate[], int candidate_cnt)
{int i,selection=-1; unsigned distance,min_distance=keyspace_lc*3; boolean show_candidates=false;

  if ( candidate_cnt==0 ) {
    if ( show_candidates && f->trace ) fprintf(f->log_file,"  no feasible candidates\n");
    selection = -1;
  }
  else {
    for (i=0; i<candidate_cnt; i++) {
      distance = distance_from_min(&(candidate[i]));
      if ( distance<min_distance ) {
        min_distance = distance;
	selection = i;
      }
      if ( show_candidates && f->trace ) {
	fprintf(f->log_file,"  candidate[%d]",i);
        print_shuffle_candidate(f->log_file,"",&candidate[i]);
      }
    }
    if ( show_candidates && f->trace )
      fprintf(f->log_file,"  min_distance=%d, selection=%d\n",min_distance,selection);
  }
  return(selection);

}

static boolean find_shuffle_candidate(struct fcb *f, struct ix_block *lt, struct ix_block *mid,
struct ix_block *rt, struct key *k, levelx_pntr *new_p, int ix, boolean insert,
struct shuffle_candidate *chosen)
{int mid_keys,lt_cnt=0,rt_cnt=0,candidate_cnt=0,selection,expected_mid_entry_lc,move_cnt;
unsigned lt_lc=0,mid_lc=0,rt_lc=0,lt_prefix_lc,mid_prefix_lc,rt_prefix_lc;
 boolean moved_key_left,moved_key_right,done=false; struct key lt_max,rt_max;
 struct shuffle_candidate candidates[key_ptrs_per_block];

  mid_keys = mid->keys_in_block;
  lt_lc = ix_pool_lc(lt);
  lt_prefix_lc = lt->prefix_lc;
  mid_lc = chars_after_move(f,mid,0,0,k,new_p,ix,insert,true,&mid_prefix_lc);
  expected_mid_entry_lc = mid_lc / mid_keys;
  rt_lc = ix_pool_lc(rt);
  rt_prefix_lc = rt->prefix_lc;

  do {
    if ( (lt_lc>=keyspace_lc) && (rt_lc>=keyspace_lc) ) done = true;
    else if ( (lt_lc>=mid_lc) && (rt_lc>=mid_lc) )      done = true;
    else {
      if ( lt_lc<rt_lc ) {
	move_cnt = ((mid_lc - lt_lc) / expected_mid_entry_lc) / 2;
	if ( move_cnt<1 ) move_cnt = 1;
        lt_cnt = lt_cnt + move_cnt;
        lt_lc = lc_if_move_left(f,lt,mid,lt_cnt,mid_keys,k,new_p,ix,insert,
          &moved_key_left,&lt_prefix_lc,&lt_max);
        mid_lc = chars_after_move(f,mid,lt_cnt,rt_cnt,k,new_p,ix,insert,true,&mid_prefix_lc);
        if ( feasible_move(lt_lc,mid_lc,rt_lc) ) { 
	  add_candidate_move(candidates,&candidate_cnt,lt_cnt,rt_cnt,lt_lc,lt_prefix_lc,
            mid_lc,mid_prefix_lc,rt_lc,rt_prefix_lc);
        }
      }
      else {
	move_cnt = ((mid_lc - rt_lc) / expected_mid_entry_lc) / 2;
	if ( move_cnt<1 ) move_cnt = 1;
        rt_cnt = rt_cnt + move_cnt;
        rt_lc = lc_if_move_right(f,mid,rt,rt_cnt,k,new_p,ix,insert,&moved_key_right,&rt_prefix_lc,
          &rt_max);
        mid_lc = chars_after_move(f,mid,lt_cnt,rt_cnt,k,new_p,ix,insert,true,&mid_prefix_lc);
        if ( feasible_move(lt_lc,mid_lc,rt_lc) ) {
	  add_candidate_move(candidates,&candidate_cnt,lt_cnt,rt_cnt,lt_lc,lt_prefix_lc,
            mid_lc,mid_prefix_lc,rt_lc,rt_prefix_lc);
        }
      }
    }
  } while (!done);
  selection = pick_shuffle_candidate(f,candidates,candidate_cnt);
  if ( selection>=0 ) *chosen = candidates[selection];
  return(selection>=0);
}


/* choose_shuffle_points is called when an insert or replace of key k in   */
/*   block mid and will cause an overflow.  It attempts to move keys to  */
/*   neighboring blocks to make room for the new key. */

static boolean choose_shuffle_points(struct fcb *f, struct ix_block *lt, struct ix_block *mid,
struct ix_block *rt, struct key *k, levelx_pntr *new_p, int ix, boolean insert, int *lt_cnt, int *rt_cnt)
{int expected_lt_cnt,expected_mid_cnt,expected_rt_cnt;
boolean moved_key_left,moved_key_right=false,can_shuffle;
 struct shuffle_candidate shuf;

#ifdef VERIFY_SHUFFLES
 struct ix_block temp_lt,temp_mid,temp_rt;
  temp_lt = *lt;
  temp_mid = *mid;
  temp_rt = *rt;
#endif

  if ( f->trace ) {
    print_key(f->log_file,mid->index_type,k,"checking nbrs, key=");
    fprintf(f->log_file,", lc=%d, pntr_lc=%d, ix=%d, level=%d, insert=%d\n",
      k->lc,levelx_pntr_lc(f,new_p,mid->level),ix,mid->level,insert);
  }

  can_shuffle = find_shuffle_candidate(f,lt,mid,rt,k,new_p,ix,insert,&shuf);

  expected_rt_cnt = rt->keys_in_block + shuf.rt_move_cnt;
  expected_mid_cnt = mid->keys_in_block - shuf.rt_move_cnt - shuf.lt_move_cnt + insert;
  expected_lt_cnt = lt->keys_in_block + shuf.lt_move_cnt;

  if ( f->trace ) {
    print_shuffle_candidate(f->log_file,"  shuffle candidate",&shuf);
  }

  if ( can_shuffle ) {
    moved_key_right = move_keys_to_right(f,mid,rt,shuf.rt_move_cnt,k,new_p,ix,insert);
    check_ix_block_after_move(f,rt,"shuffle rt",expected_rt_cnt,shuf.rt_lc,shuf.rt_prefix_lc);
    moved_key_left = move_keys_to_left(f,lt,mid,shuf.lt_move_cnt,k,new_p,ix,insert,!moved_key_right);
    check_ix_block_after_move(f,lt,"shuffle lt",expected_lt_cnt,shuf.lt_lc,shuf.lt_prefix_lc);

    check_ix_block_after_move(f,mid,"shuffle mid",expected_mid_cnt,shuf.mid_lc,shuf.mid_prefix_lc);
    *lt_cnt = shuf.lt_move_cnt;
    *rt_cnt = shuf.rt_move_cnt;

#ifdef VERIFY_SHUFFLES
    check_shuffle(f,&temp_lt,lt,&temp_mid,mid,&temp_rt,rt,k,new_p,ix,insert);
#endif
    }
  else {
    *lt_cnt = 0;
    *rt_cnt = 0;
  }
  return(can_shuffle);
}

/* shuffle_keys is called when an insert to buffer[mid_ix] won't fit.  */
/*   It checks to see if it is possible to move keys to the right and  */
/*   left to make room.  If the block is at the beginning or end of    */
/*   the block chain at this level then the shuffle fails.  Otherwise, */
/*   choose_shuffle_points moves the keys if the shuffle is possible     */
/*   otherwise it does nothing.  If the shuffle succeeded then the     */
/*   parent pointers are adjusted and the blocks are marked modified.  */

#if 0
static boolean shuffle_keys(struct fcb *f, int mid_ix, struct key *k, levelx_pntr *p, int ix,
  boolean insert)
{int lt_ix=-1,rt_ix=-1,index_type,lt_cnt,rt_cnt; unsigned level;
 boolean shuffled=false; struct key lt_sep,old_lt_sep,mid_sep,old_mid_sep;
 struct ix_block full_block,*lt_block,*rt_block;

  lock_buffer(f,mid_ix);
  get_max_key(&(f->buffer_pool.buffer[mid_ix].b),&old_mid_sep);
  level = f->buffer_pool.buffer[mid_ix].b.level;
  index_type = f->buffer_pool.buffer[mid_ix].b.index_type;
  initialize_index_block(&full_block,index_type,level,&lt_sep,0);
  full_block.chars_in_use = keyspace_lc;

  if ( null_pntr(f->buffer_pool.buffer[mid_ix].b.prev) ) lt_block = &full_block;
  else {
    lt_ix = get_index(f,f->buffer_pool.buffer[mid_ix].b.prev);
    lock_buffer(f,lt_ix);
    lt_block = &(f->buffer_pool.buffer[lt_ix].b);
  }
  if ( null_pntr(f->buffer_pool.buffer[mid_ix].b.next) ) rt_block = &full_block;
  else {
    rt_ix = get_index(f,f->buffer_pool.buffer[mid_ix].b.next);
    lock_buffer(f,rt_ix);
    rt_block = &(f->buffer_pool.buffer[rt_ix].b);
  }
  get_max_key(&(f->buffer_pool.buffer[lt_ix].b),&old_lt_sep);

  if ( f->trace ) {
    fprintf(f->log_file,"In shuffle_keys, ");
#if defined(TRACE_SHUFFLE_BLOCKS)
    fprintf(f->log_file,"blocks before shuffle are:\n");
    print_index_block(f->log_file,f,&(f->buffer_pool.buffer[lt_ix].b));
    print_index_block(f->log_file,f,&(f->buffer_pool.buffer[mid_ix].b));
    print_index_block(f->log_file,f,&(f->buffer_pool.buffer[rt_ix].b));
#endif
  }
  if ( choose_shuffle_points(f,lt_block,&(f->buffer_pool.buffer[mid_ix].b),rt_block,k,p,ix,insert,&lt_cnt,&rt_cnt) ) {
    shuffled = true;
    if ( lt_cnt==0 ) printf("  lt_cnt=0\n");
    if ( rt_cnt==0 ) printf("  rt_cnt=0\n");

    if ( level<f->primary_level[index_type] ) {
      if ( lt_cnt>0 ) {
        get_max_key(&(f->buffer_pool.buffer[lt_ix].b),&lt_sep);
        replace_max_key(f,index_type,&old_lt_sep,&lt_sep,f->buffer_pool.buffer[lt_ix].contents,
          (unsigned)f->buffer_pool.buffer[lt_ix].b.level+1);
      }
      if ( rt_cnt>0 ) {
        get_max_key(&(f->buffer_pool.buffer[mid_ix].b),&mid_sep);
        replace_max_key(f,index_type,&old_mid_sep,&mid_sep,f->buffer_pool.buffer[mid_ix].contents,
          (unsigned)f->buffer_pool.buffer[mid_ix].b.level+1);
      }
    }
    if ( lt_cnt>0 ) mark_modified(f,lt_ix);
    if ( rt_cnt>0 ) mark_modified(f,rt_ix);
  }
  if ( f->trace ) {
    if ( shuffled ) {
      fprintf(f->log_file,"reshuffle succeeded\n");
#if defined(TRACE_SHUFFLE_BLOCKS)
      fprintf(f->log_file,"  shuffled blocks are:\n");
      print_index_block(f->log_file,f,&(f->buffer_pool.buffer[lt_ix].b));
      print_index_block(f->log_file,f,&(f->buffer_pool.buffer[mid_ix].b));
      print_index_block(f->log_file,f,&(f->buffer_pool.buffer[rt_ix].b));
#endif
    }
    else fprintf(f->log_file,"reshuffle failed\n");
  }
  if ( rt_ix>=0 ) unlock_buffer(f,rt_ix);
  if ( lt_ix>=0 ) unlock_buffer(f,lt_ix);
  unlock_buffer(f,mid_ix);

  return(shuffled);
}
#endif

#if 1
static boolean shuffle_keys(struct fcb *f, int mid_ix, struct key *k, levelx_pntr *p, int ix,
  boolean insert)
{int lt_cnt,rt_cnt,lt_ix=-1,rt_ix=-1,index_type; unsigned level;
 boolean shuffled=false; struct key lt_sep,old_lt_sep,mid_sep,old_mid_sep;

  lock_buffer(f,mid_ix);
  get_max_key(&(f->buffer_pool.buffer[mid_ix].b),&old_mid_sep);
  level = f->buffer_pool.buffer[mid_ix].b.level;
  index_type = f->buffer_pool.buffer[mid_ix].b.index_type;
  if ( null_pntr(f->buffer_pool.buffer[mid_ix].b.prev) ) shuffled = false;
  else if ( null_pntr(f->buffer_pool.buffer[mid_ix].b.next) ) shuffled = false;
  else {
    lt_ix = get_index(f,f->buffer_pool.buffer[mid_ix].b.prev);
    lock_buffer(f,lt_ix);
    get_max_key(&(f->buffer_pool.buffer[lt_ix].b),&old_lt_sep);
    rt_ix = get_index(f,f->buffer_pool.buffer[mid_ix].b.next);
    lock_buffer(f,rt_ix);

    if ( f->trace ) {
      fprintf(f->log_file,"In shuffle_keys, ");
#if defined(TRACE_SHUFFLE_BLOCKS)
      fprintf(f->log_file,"blocks before shuffle are:\n");
      print_index_block(f->log_file,f,&(f->buffer_pool.buffer[lt_ix].b));
      print_index_block(f->log_file,f,&(f->buffer_pool.buffer[mid_ix].b));
      print_index_block(f->log_file,f,&(f->buffer_pool.buffer[rt_ix].b));
#endif
    }
    if ( choose_shuffle_points(f,&(f->buffer_pool.buffer[lt_ix].b),&(f->buffer_pool.buffer[mid_ix].b),&(f->buffer_pool.buffer[rt_ix].b),k,p,ix,insert,&lt_cnt,&rt_cnt) ) {
      shuffled = true;
      shuffle_cnt++;
      if ( lt_cnt==0 ) shuffle_lt_zero_cnt++;
      if ( rt_cnt==0 ) shuffle_rt_zero_cnt++;

      if ( lt_cnt>0 ) {
        if ( level<f->primary_level[index_type] ) {
          get_max_key(&(f->buffer_pool.buffer[lt_ix].b),&lt_sep);
          replace_max_key(f,index_type,&old_lt_sep,&lt_sep,f->buffer_pool.buffer[lt_ix].contents,
            (unsigned)f->buffer_pool.buffer[lt_ix].b.level+1);
	}
        mark_modified(f,lt_ix);
      }
      if ( rt_cnt>0 ) {
        if ( level<f->primary_level[index_type] ) {
          get_max_key(&(f->buffer_pool.buffer[mid_ix].b),&mid_sep);
          replace_max_key(f,index_type,&old_mid_sep,&mid_sep,f->buffer_pool.buffer[mid_ix].contents,
            (unsigned)f->buffer_pool.buffer[mid_ix].b.level+1);
	}
        mark_modified(f,rt_ix);
      }
    }
    if ( f->trace ) {
      if ( shuffled ) {
        fprintf(f->log_file,"reshuffle succeeded\n");
#if defined(TRACE_SHUFFLE_BLOCKS)
        fprintf(f->log_file,"  shuffled blocks are:\n");
        print_index_block(f->log_file,f,&(f->buffer_pool.buffer[lt_ix].b));
        print_index_block(f->log_file,f,&(f->buffer_pool.buffer[mid_ix].b));
        print_index_block(f->log_file,f,&(f->buffer_pool.buffer[rt_ix].b));
#endif
      }
      else fprintf(f->log_file,"reshuffle failed\n");
    }
    unlock_buffer(f,rt_ix);
    unlock_buffer(f,lt_ix);
  }
  unlock_buffer(f,mid_ix);

  return(shuffled);
}
#endif

#if 0
/* original version */
static boolean shuffle_keys(struct fcb *f, int mid_ix, struct key *k, levelx_pntr *p, int ix, boolean insert)
{int lt_cnt,rt_cnt,lt_ix,rt_ix,index_type; unsigned level;
 boolean shuffled=false; struct key lt_sep,old_lt_sep,mid_sep,old_mid_sep;

  lock_buffer(f,mid_ix);
  get_max_key(&(f->buffer_pool.buffer[mid_ix].b),&old_mid_sep);
  level = f->buffer_pool.buffer[mid_ix].b.level;
  index_type = f->buffer_pool.buffer[mid_ix].b.index_type;
  if ( null_pntr(f->buffer_pool.buffer[mid_ix].b.prev) ) shuffled = false;
  else if ( null_pntr(f->buffer_pool.buffer[mid_ix].b.next) ) shuffled = false;
  else {
    lt_ix = get_index(f,f->buffer_pool.buffer[mid_ix].b.prev);
    lock_buffer(f,lt_ix);
    get_max_key(&(f->buffer_pool.buffer[lt_ix].b),&old_lt_sep);
    rt_ix = get_index(f,f->buffer_pool.buffer[mid_ix].b.next);
    lock_buffer(f,rt_ix);

    if ( f->trace ) {
      fprintf(f->log_file,"In shuffle_keys, blocks before shuffle are:\n");
      print_index_block(f->log_file,f,&(f->buffer_pool.buffer[lt_ix].b));
      print_index_block(f->log_file,f,&(f->buffer_pool.buffer[mid_ix].b));
      print_index_block(f->log_file,f,&(f->buffer_pool.buffer[rt_ix].b));
    }
    if ( choose_shuffle_points(f,&(f->buffer_pool.buffer[lt_ix].b),&(f->buffer_pool.buffer[mid_ix].b),&(f->buffer_pool.buffer[rt_ix].b),k,p,ix,insert,&lt_cnt,&rt_cnt) ) {
      shuffled = true;
      if ( level<f->primary_level[index_type] ) {
        get_max_key(&(f->buffer_pool.buffer[lt_ix].b),&lt_sep);
        replace_max_key(f,index_type,&old_lt_sep,&lt_sep,f->buffer_pool.buffer[lt_ix].contents,
          (unsigned)f->buffer_pool.buffer[lt_ix].b.level+1);
        get_max_key(&(f->buffer_pool.buffer[mid_ix].b),&mid_sep);
        replace_max_key(f,index_type,&old_mid_sep,&mid_sep,f->buffer_pool.buffer[mid_ix].contents,
          (unsigned)f->buffer_pool.buffer[mid_ix].b.level+1);
      }
      mark_modified(f,lt_ix);
      mark_modified(f,rt_ix);
    }
    if ( f->trace ) {
      if ( shuffled ) {
        fprintf(f->log_file,"reshuffle succeeded, shuffled blocks are:\n");
        print_index_block(f->log_file,f,&(f->buffer_pool.buffer[lt_ix].b));
        print_index_block(f->log_file,f,&(f->buffer_pool.buffer[mid_ix].b));
        print_index_block(f->log_file,f,&(f->buffer_pool.buffer[rt_ix].b));
      }
      else fprintf(f->log_file,"reshuffle failed\n");
    }
    unlock_buffer(f,rt_ix);
    unlock_buffer(f,lt_ix);
  }
  unlock_buffer(f,mid_ix);

  return(shuffled);
}
#endif

/* create_new_primary creates a new primary block containing the key max  */
/*   and sets last_pntr */

static void create_new_primary(struct fcb *f, int index, struct leveln_pntr b, struct key *max,
  struct leveln_pntr newb)
{int ix; levelx_pntr bx; struct key dummy; /* block_type_t block;*/


  if ( f->primary_level[index]>=max_level-1 ) set_error(f,maxlevel_err,"");
  else {

    f->primary_level[index]++;
    ix = allocate_index_block(f,index,&(f->first_at_level[f->primary_level[index]][index]),
      f->primary_level[index],&dummy,0);
    bx.pn = b;
    simple_insert(f,&(f->buffer_pool.buffer[ix].b),0,max,&bx);
    f->last_pntr[f->primary_level[index]][index] = newb;

    if ( f->trace ) {
      fprintf(f->log_file,"  creating new ");
      print_index_type(f->log_file,index);
      fprintf(f->log_file," primary, levels are ");
      print_index_type(f->log_file,user_ix);
      fprintf(f->log_file,"=%u, ",f->primary_level[user_ix]);
      print_index_type(f->log_file,free_rec_ix);
      fprintf(f->log_file,"=%u, ",f->primary_level[free_rec_ix]);
      print_index_type(f->log_file,free_lc_ix);
      fprintf(f->log_file,"=%u\n",f->primary_level[free_lc_ix]);
    }
  }
}

/* choose_split_point figures out how to allocate keys when a block */
/*   split is required.  It tries moving half the keys to the right */
/*   and compares the sizes of the left and right blocks that would */
/*   result.  If they are equal then we are done.  If one is larger */
/*   then we move more keys to the smaller block until it is no     */
/*   longer smaller.  At that point we compare the block sizes on   */
/*   either side of the boundary and a) if one results in a smaller */
/*   combined block size we pick that one, or b) we pick the one    */
/*   that splits the blocks more evenly.                            */

static void choose_split_point(struct fcb *f, struct ix_block *b, struct key *k, levelx_pntr *p, int ix,
boolean insert, int *lt_cnt, unsigned *lt_lc, unsigned *lt_prefix_lc,
int *rt_cnt, unsigned *rt_lc, unsigned *rt_prefix_lc)
{int index_type,move_rt_cnt,left_lc,right_lc,
last_left_lc=0,last_left_prefix_lc=0,last_right_lc=0,last_right_prefix_lc=0,up_or_down=1;
  unsigned left_prefix_lc,right_prefix_lc;
struct ix_block temp;
struct key rt_max; boolean moved_new_key,done=false,use_last=false;

  index_type = b->index_type;
  initialize_index_block(&temp,index_type,0,&rt_max,0);

  if ( f->trace ) {
    fprintf(f->log_file,"Choosing split_point for ");
    print_index_type(f->log_file,index_type);
    fprintf(f->log_file," block, keys=%d, chars_in_use=%d, prefix_lc=%d\n",
      b->keys_in_block,b->chars_in_use,b->prefix_lc);
    print_key(f->log_file,index_type,k,"  new key=");
    fprintf(f->log_file,", ix=%d, insert=%d\n",ix,insert);
  }

  move_rt_cnt = (b->keys_in_block + insert) / 2;
  right_lc = lc_if_move_right(f,b,&temp,move_rt_cnt,k,p,ix,insert,&moved_new_key,
    &right_prefix_lc,&rt_max);
  left_lc = chars_after_move(f,b,0,move_rt_cnt,k,p,ix,insert,true,&left_prefix_lc);

  if ( left_lc > right_lc ) up_or_down = 1;
  else if ( left_lc < right_lc ) up_or_down = -1;

  if ( left_lc!=right_lc ) {
    do {
      last_left_lc = left_lc;
      last_left_prefix_lc = left_prefix_lc;
      last_right_lc = right_lc;
      last_right_prefix_lc = right_prefix_lc;
      move_rt_cnt = move_rt_cnt + up_or_down;
      right_lc = lc_if_move_right(f,b,&temp,move_rt_cnt,k,p,ix,insert,
        &moved_new_key,&right_prefix_lc,&rt_max);
      left_lc = chars_after_move(f,b,0,move_rt_cnt,k,p,ix,insert,true,&left_prefix_lc);

      if ( up_or_down>0 ) done = left_lc<=right_lc;
      else done = left_lc>=right_lc;

    } while(!done);

    if ( (last_left_lc+last_right_lc)==(left_lc+right_lc) ) {
      if ( abs(last_right_lc-last_left_lc)<abs(right_lc-left_lc) ) use_last = true;
    } 
    else if ( (last_left_lc+last_right_lc)<(left_lc+right_lc) ) use_last = true; 
    if ( use_last ) {
      move_rt_cnt = move_rt_cnt - up_or_down;
      left_lc = last_left_lc;
      left_prefix_lc = last_left_prefix_lc;
      right_lc = last_right_lc;
      right_prefix_lc = last_right_prefix_lc;
    } 
  }

  if ( f->trace ) {
    fprintf(f->log_file,"Chose split_point for ");
    print_index_type(f->log_file,index_type);
    fprintf(f->log_file," block, keys=%d, chars_in_use=%d, prefix_lc=%d\n",
      b->keys_in_block,b->chars_in_use,b->prefix_lc);
    fprintf(f->log_file,"  move_rt_cnt=%d, left=%d/%d, right=%d/%d, use_last=%d\n",move_rt_cnt,
      left_lc,left_prefix_lc,right_lc,right_prefix_lc,use_last);
  }

  *rt_cnt = move_rt_cnt;
  *rt_lc = right_lc;
  *rt_prefix_lc = right_prefix_lc;
  *lt_cnt = b->keys_in_block + insert - move_rt_cnt;
  *lt_lc = left_lc;
  *lt_prefix_lc = left_prefix_lc;

}

/* split_block splits the block in buffer[bufix]. On entry key k    */
/*   belongs in entry ix and must be <= the */
/*   max_key in the parent block that points to this block.  It     */
/*   creates a new block, moves keys to it, and adjusts prev and    */
/*   next pointers. It then updates the parent block with a new     */
/*   pointer for the right block and a new max_key for the left or  */
/*   creates a new primary, as appropriate.                         */
/*   If we are inserting at eof (next=null, ix=keys_in_block)       */
/*   then sequential inserts are assumed, otherwise random.         */
/*   Note, however, that we do not use sequential mode for free     */
/*   space blocks.  This is because freespace management repeatedly */
/*   deletes the last key in the index then reinserts the last key  */
/*   (with a new lc) which leads to degenerate behavior when block  */
/*   splits result in a new primary.                                */

static void split_block(struct fcb *f, struct key *k, levelx_pntr *p, int bufix, int ix, boolean insert)
{int err,new_ix,i,index_type,lt_cnt,rt_cnt;
 unsigned parent_level,old_block_prefix_lc,lt_lc,lt_prefix_lc,rt_lc,rt_prefix_lc;
 boolean split,seq,new_on_right,moved_new_key;
 struct leveln_pntr parent,oldb,newb,save_next,leftb,rightb;
 struct key left_max,right_max,original_max_key; levelx_pntr leftbx;
 struct ix_block *old_block,*new_block;

  lock_buffer(f,bufix);
  oldb = f->buffer_pool.buffer[bufix].contents;
  old_block = &(f->buffer_pool.buffer[bufix].b);
  index_type = old_block->index_type;
  get_max_key(old_block,&original_max_key);

  seq = insert && null_pntr(old_block->next) && ix==old_block->keys_in_block;
  seq = seq && (index_type==user_ix);

  parent_level = f->buffer_pool.buffer[bufix].b.level + 1;
  parent = parent_block(f,bufix,k);
  new_ix = allocate_index_block(f,index_type,&newb,f->buffer_pool.buffer[bufix].b.level,k,0);
  lock_buffer(f,new_ix);
  new_block = &(f->buffer_pool.buffer[new_ix].b);

  if ( f->trace ) {
    print_leveln_pntr(f->log_file,"splitting block ",&oldb);
    print_leveln_pntr(f->log_file," new block is ",&newb);
    fprintf(f->log_file,"insert=%d, seq=%d, ix=%d\n",insert,seq,ix);
    print_key(f->log_file,index_type,k,"  ins/rep key=");
    print_key(f->log_file,index_type,&original_max_key,"\n  orig_max_key=");
    fprintf(f->log_file,"\n");
    print_index_block(f->log_file,f,old_block);
  }
  if ( ix<(old_block->keys_in_block/2) ) { /* add block to left */
    new_on_right = false;
    new_block->next = oldb;
    new_block->prev = old_block->prev;
    if ( null_pntr(old_block->prev) ) {
      f->first_at_level[old_block->level][index_type] = newb;
    }
    else {
      i = get_index_update(f,old_block->prev);
      f->buffer_pool.buffer[i].b.next = newb;
    }
    old_block->prev = newb;

    choose_split_point(f,old_block,k,p,ix,insert,&lt_cnt,&lt_lc,&lt_prefix_lc,
      &rt_cnt,&rt_lc,&rt_prefix_lc);
    move_keys_to_left(f,new_block,old_block,lt_cnt,k,p,ix,insert,true);
    check_ix_block_after_move(f,new_block,"split block lt",lt_cnt,lt_lc,lt_prefix_lc);
  }
  else { /* add new block to right */
    new_on_right = true;
    new_block->next = old_block->next;
    new_block->prev = oldb;
    if ( !null_pntr(new_block->next) ) {
      i = get_index_update(f,new_block->next);
      f->buffer_pool.buffer[i].b.prev = newb;
    }
    old_block->next = newb;
    if ( seq ) simple_insert(f,new_block,0,k,p);
    else {
      choose_split_point(f,old_block,k,p,ix,insert,&lt_cnt,&lt_lc,&lt_prefix_lc,
        &rt_cnt,&rt_lc,&rt_prefix_lc);
      moved_new_key = move_keys_to_right(f,old_block,new_block,rt_cnt,k,p,ix,insert);
      if ( moved_new_key ) {
	/*        err = compress_ix_block(f,old_block,(unsigned)block_prefix_lc(old_block));*/
        err = compress_ix_block(f,old_block,lt_prefix_lc);
	if ( err!=0 )
          set_error1(f,ix_compress_err,"Split_block, compress failed, moved_new_key=",
            moved_new_key);
      }
      else {
	/*	old_block_prefix_lc = block_prefix_lc_update(old_block,k,ix,insert);*/
	old_block_prefix_lc = lt_prefix_lc;
        if ( !insert ) delete_keys(f,old_block,ix,1);
	err = compress_ix_block(f,old_block,old_block_prefix_lc);
	if ( err!=0 ) {
	  set_error1(f,ix_compress_err,"Compress_failed in split_block, moved_new_key=",moved_new_key);
          fprintf(f->log_file,"  ix=%d, insert=%d",ix,insert);
	  print_key(f->log_file,index_type,k,", insert_key=");
	  fprintf(f->log_file,"\n");
	}
        if ( !simple_insert(f,old_block,ix,k,p) )
          set_error(f,insert_err,"Insert failed in split_block, new is on rt");
      }
    }
  }
  
  /*now propogate upward*/
  if ( new_on_right ) {
    leftb = oldb;
    rightb = newb;
    get_max_key(old_block,&left_max);
    get_max_key(new_block,&right_max);
    save_next = new_block->next;
  }
  else {
    leftb = newb;
    rightb = oldb;
    get_max_key(old_block,&right_max);
    get_max_key(new_block,&left_max);
    save_next = old_block->next;
  }
  if ( f->trace ) {
    fprintf(f->log_file,"after split, new_on_right=%d, seq=%d, old block is\n",new_on_right,seq);
    print_index_block(f->log_file,f,old_block);
    fprintf(f->log_file,"after split, new block is\n");
    print_index_block(f->log_file,f,new_block);
    print_key(f->log_file,index_type,&original_max_key,"  replacing orig_max_key ");
    print_leveln_pntr(f->log_file," ptr= ",&oldb);
    print_key(f->log_file,index_type,&right_max,"\n  with ");
    print_leveln_pntr(f->log_file," ptr= ",&rightb);
    print_key(f->log_file,index_type,&left_max,"\n  inserting new max_key ");
    print_leveln_pntr(f->log_file," ptr= ",&leftb);
    fprintf(f->log_file,"\n");
  }

  unlock_buffer(f,bufix);
  unlock_buffer(f,new_ix);

  if ( parent_level>f->primary_level[index_type] )
    create_new_primary(f,index_type,leftb,&left_max,rightb);
  else {
    leftbx.pn = leftb;
    if ( null_pntr(save_next) ) {
      f->last_pntr[parent_level][index_type] = rightb;
      update_index(f,&left_max,parent,&leftbx);
    }
    else {
      split = replace_max_key_and_pntr(f,index_type,&original_max_key,&right_max,oldb,rightb,parent_level);
      if ( split ) {
        parent = search_index(f,index_type,parent_level+1,&left_max);
        if ( f->trace ) {
          print_leveln_pntr(f->log_file,"  parent after replace_max split is ",&parent);
          fprintf(f->log_file,"\n");
	}
      }
      update_index(f,&left_max,parent,&leftbx);
    }
  }
}

/* update_index1 inserts key k and pointer p into entry ix in the  */
/*   index block in buffer[bufix].  It assumes that the buffer has */
/*   been marked modified, locked and will be unlocked upon        */
/*   return              */  

static void update_index1(struct fcb *f, struct key *k, levelx_pntr *p, int bufix, int ix, boolean insert)
{int index_type,update_type;
  unsigned level,new_lc,new_prefix_lc;
 boolean at_end,update_parent=false;
 struct key old_max_key; struct leveln_pntr b;

  level = f->buffer_pool.buffer[bufix].b.level;
  index_type = f->buffer_pool.buffer[bufix].b.index_type;
  at_end = ix==f->buffer_pool.buffer[bufix].b.keys_in_block;
  if ( at_end && f->primary_level[index_type]>level && !null_pntr(f->buffer_pool.buffer[bufix].b.next) && insert) {
    update_parent = true;
    get_max_key(&(f->buffer_pool.buffer[bufix].b),&old_max_key);
  }
  if ( f->trace ) {
    b = f->buffer_pool.buffer[bufix].contents;
    print_leveln_pntr(f->log_file,"  updating block ",&b);
    print_key(f->log_file,index_type,k," with key=");
    if ( level==0 ) print_level0_pntr(f->log_file,f,", ptr=",&(p->p0) );
    else print_leveln_pntr(f->log_file,", ptr=",&(p->pn) );
    fprintf(f->log_file,"\n");
  }
  new_lc = ix_pool_lc_after_change(f,&(f->buffer_pool.buffer[bufix].b),k,p,ix,&new_prefix_lc,insert);
  if ( new_lc<=keyspace_lc ) {
    if ( !insert ) delete_keys(f,&(f->buffer_pool.buffer[bufix].b),ix,1);
    prefix_simple_insert(f,&(f->buffer_pool.buffer[bufix].b),ix,k,p);
    update_type = 0;
    if ( f->trace ) fprintf(f->log_file,"    simple insert\n");
    if ( update_parent )
      replace_max_key(f,index_type,&old_max_key,k,f->buffer_pool.buffer[bufix].contents,(unsigned)level+1);
  }
  else if ( shuffle_keys(f,bufix,k,p,ix,insert) ) {
    update_type = 1;
    if ( f->trace ) fprintf(f->log_file,"    shuffled keys to insert\n");
  }
  else {
    update_type = 2;
    if ( f->trace ) {
      print_key(f->log_file,index_type,k,"    block split, k=");
      fprintf(f->log_file,", keys_in_block=%d, chars_in_use=%d\n",
        f->buffer_pool.buffer[bufix].b.keys_in_block,f->buffer_pool.buffer[bufix].b.chars_in_use);
    }
    split_block(f,k,p,bufix,ix,insert);
  }
}

/* update_index inserts key k and pointer p into index block b. */

static void update_index(struct fcb *f, struct key *k, struct leveln_pntr b, levelx_pntr *p)
{int bufix,ix; boolean found; char *name="update_index";

  bufix = get_index_update(f,b);
  lock_buffer(f,bufix);
  if ( f->trace ) f->search_block_caller = name;
  ix = search_block(f,bufix,k,&found);
  update_index1(f,k,p,bufix,ix,!found);
  unlock_buffer(f,bufix);
}

/* intermediate calls */

/* extract_next extracts the key and pointer identified by the current file */
/*   position and advances the file position.  If max_key_lc==0 the key is  */
/*   not extracted.  Note that if the pointer contains the data_rec         */
/*   (lc<=f->max_data_in_index) then the data will extracted into rec.  The */
/*   caller should either point to the end destination for the data or      */
/*   point to the data_rec in p.  If max_rec_lc==0 no data will be          */
/*   extracted.                                                             */

static void extract_next(struct fcb *f, int index, int bufix, unsigned char t[], unsigned *key_lc,
 int max_key_lc, level0_pntr *p, unsigned char rec[], unsigned *rec_lc, unsigned max_rec_lc)
{struct key k;

  if ( f->position_ix[index]>=f->buffer_pool.buffer[bufix].b.keys_in_block ) {
    t[0] = '\0'; *key_lc = 0; *p = null0_ptr;
    if ( null_pntr(f->buffer_pool.buffer[bufix].b.next) ) f->error_code = ateof_err;
    else {
      set_error(f,ix_struct_err,"Error in extract_next");
      print_leveln_pntr(f->log_file,"  block=",&(f->buffer_pool.buffer[bufix].contents));
      fprintf(f->log_file," index=%d, position=%d, keys=%d, ",index,f->position_ix[index],
        f->buffer_pool.buffer[bufix].b.keys_in_block);
      print_leveln_pntr(f->log_file,"next",&(f->buffer_pool.buffer[bufix].b.next));
      fprintf(f->log_file,"\n");
    }
  }
  else {
    if ( max_key_lc>0 ) {
      get_nth_key(&(f->buffer_pool.buffer[bufix].b),&k,f->position_ix[index]);
      if ( k.lc<=max_key_lc ) *key_lc = k.lc;
      else {
        f->error_code = longkey_err; *key_lc = max_key_lc;
      }
      mvc(k.text,0,t,0,(unsigned)*key_lc);
    }
    unpack0_ptr_and_rec(f,&(f->buffer_pool.buffer[bufix]),f->position_ix[index],p,rec,rec_lc,(unsigned)max_rec_lc);
    if ( max_rec_lc==0 || max_rec_lc==f->data_in_index_lc ) /* not an error */;
    else if ( p->lc>(unsigned)*rec_lc ) f->error_code = longrec_err;
    f->position_ix[index]++;
    if ( f->position_ix[index]>=f->buffer_pool.buffer[bufix].b.keys_in_block && !null_pntr(f->buffer_pool.buffer[bufix].b.next) )
      set_position(f,index,f->buffer_pool.buffer[bufix].b.next,0);
  }
}

void kf_set_bof(struct fcb *f, int index)
{
  f->position[index] = f->first_at_level[0][index];
  f->position_ix[index] = 0;
  f->seq_cnt[index] = 0;
}

int kf_next_rec(struct fcb *f, int index, unsigned char key[], unsigned *key_lc, int max_key_lc,
  level0_pntr *p, unsigned char rec[], unsigned *rec_lc, unsigned max_rec_lc)
{int ix; struct leveln_pntr next;

  if ( check_fcb(f) ) {
    ix = get_index(f,f->position[index]);
    next = f->buffer_pool.buffer[ix].b.next;
    while ( f->position_ix[index]>=f->buffer_pool.buffer[ix].b.keys_in_block && !null_pntr(next) ) {
      ix = get_index(f,next);
      set_position(f,index,next,0);
      next = f->buffer_pool.buffer[ix].b.next;
    }
    extract_next(f,index,ix,key,key_lc,max_key_lc,p,rec,rec_lc,max_rec_lc);
  }
  return(f->error_code);
}

static int kf_next_ptr(struct fcb *f, int index, unsigned char t[], unsigned *key_lc, int max_key_lc,
   level0_pntr *p)
{int err; unsigned rec_lc;

  err = kf_next_rec(f,index,t,key_lc,max_key_lc,p,p->data_rec,&rec_lc,f->data_in_index_lc);
  return(err);
}

static int kf_prev_rec(struct fcb *f, int index, unsigned char t[], unsigned *key_lc, int max_key_lc,
  level0_pntr *p, unsigned char rec[], unsigned *rec_lc, unsigned max_rec_lc)
{int bufix; boolean done=false; struct leveln_pntr prev; struct key k;

  if ( check_fcb(f) ) {
    bufix = get_index(f,f->position[index]);
    if ( f->position_ix[index]==0 ) {
      prev = f->buffer_pool.buffer[bufix].b.prev;
      do {
        if ( null_pntr(prev) ) {
          done = true; f->error_code = atbof_err;
          *key_lc = 0; t[0] = '\0'; *p = null0_ptr;
        }
        else {
          bufix = get_index(f,prev);
          set_position(f,index,prev,f->buffer_pool.buffer[bufix].b.keys_in_block);
          prev = f->buffer_pool.buffer[bufix].b.prev;
          done = f->buffer_pool.buffer[bufix].b.keys_in_block > 0;
        }
      } while ( !done );
    }
    if ( f->error_code==no_err && f->position_ix[index]>0 ) {
      f->position_ix[index]--;
      get_nth_key(&(f->buffer_pool.buffer[bufix].b),&k,f->position_ix[index]);
      if ( k.lc<=max_key_lc ) *key_lc = k.lc;
      else {
        *key_lc = max_key_lc; f->error_code = longkey_err;
      }
      mvc(k.text,0,t,0,(unsigned)*key_lc);
      unpack0_ptr_and_rec(f,&(f->buffer_pool.buffer[bufix]),f->position_ix[index],p,rec,rec_lc,(unsigned)max_rec_lc);
      if ( max_rec_lc==0 || max_rec_lc==f->data_in_index_lc ) /* not an error */;
      else if ( p->lc>*rec_lc ) f->error_code = longrec_err;
    }
  }
  return(f->error_code);
}

static int kf_prev_ptr(struct fcb *f, int index, unsigned char t[], unsigned *key_lc, int max_key_lc,
  level0_pntr *p)
{int err; unsigned rec_lc;

  err = kf_prev_rec(f,index,t,key_lc,max_key_lc,p,p->data_rec,&rec_lc,f->data_in_index_lc);
  return(err);
}

/* kf_get_ptr gets the pointer associated with key t. We've tried   */
/*   strategies for sequential access that look first in the        */
/*   current position block but they seem to work well only when    */
/*   we are accessing nearly every key.                             */

static int kf_get_rec(struct fcb *f, int index, unsigned char t[], unsigned key_lc, level0_pntr *p,
  unsigned char rec[], unsigned *rec_lc, unsigned max_rec_lc)
{struct leveln_pntr b,last_position; int ix=0,bufix=0,last_ix; unsigned lc; struct key k;
boolean found=false,seq=false;
 unsigned char t1[maxkey_lc]; char *name="kf_get_ptr";

  set_up(f,t,key_lc,&k);
  if ( f->error_code==no_err ) {
    last_position = f->position[index];
    last_ix = f->position_ix[index];
    b = search_index(f,index,level_one,&k);
    bufix = get_index(f,b);
    if ( f->trace ) f->search_block_caller = name;
    ix = search_block(f,bufix,&k,&found);
    set_position(f,index,b,ix);

    if ( (eqn_pntr(b,last_position) && ix>=last_ix) ) seq = true;
    else if ( eqn_pntr(f->buffer_pool.buffer[bufix].b.prev,last_position) ) seq = true;
    if ( seq ) {
      if ( f->seq_cnt[index]<INT_MAX ) (f->seq_cnt[index])++;
    }
    else f->seq_cnt[index] = 0;

    if ( found ) {
      extract_next(f,index,bufix,t1,&lc,0,p,rec,rec_lc,max_rec_lc);
    }
    else if ( f->error_code==no_err ) {
      f->error_code = getnokey_err; *p = null0_ptr;
    }
  }
  return(f->error_code);
}

static int kf_get_ptr(struct fcb *f,int index, unsigned char t[], unsigned key_lc,
  level0_pntr *p)
{int err; unsigned rec_lc;

  err = kf_get_rec(f,index,t,key_lc,p,p->data_rec,&rec_lc,f->data_in_index_lc);
  return(err);
}

static int kf_put_ptr(struct fcb *f, int index, unsigned char t[], unsigned key_lc, level0_pntr p)
{struct leveln_pntr b; levelx_pntr px; struct key k;

  if ( f->read_only ) f->error_code = read_only_err;
  else {
    set_up(f,t,key_lc,&k);
    if ( f->error_code==no_err ) {
      b = search_index(f,index,level_one,&k);
      px.p0 = p;
      update_index(f,&k,b,&px);
      kf_set_bof(f,index);
    }
  }
  return(f->error_code);
}

static int kf_delete_ptr(struct fcb *f, int index, unsigned char t[], unsigned key_lc)
{struct key k; level0_pntr p;

  if ( f->read_only ) f->error_code = read_only_err;
  else {
    set_up(f,t,key_lc,&k);
    if (f->error_code==no_err ) {
      index_delete(f,index,k,&p,level_zero);
      kf_set_bof(f,index);
    }
  }
  return(f->error_code);
}

static int kf_put_rec(struct fcb *f,int index, unsigned char t[], unsigned key_lc, char r[], unsigned rlc)
{int ix,bufix; unsigned lc,rec_lc; boolean have_space=false,found;
 unsigned char t1[maxkey_lc]; char *name="kf_put_rec";
 struct key k; struct leveln_pntr b; level0_pntr p; levelx_pntr px; unsigned char dummy[2];

  if ( f->read_only ) f->error_code = read_only_err;
  else {
    set_up(f,t,key_lc,&k);
    if ( f->error_code==no_err ) {
      b = search_index(f,index,level_one,&k);
      bufix = get_index_update(f,b);
      lock_buffer(f,bufix);
      if ( f->trace ) f->search_block_caller = name;
      ix = search_block(f,bufix,&k,&found);
      set_position(f,index,b,ix);
      if ( found  ) {
        extract_next(f,index,bufix,t1,&lc,0,&p,dummy,&rec_lc,0);

        if ( rlc<=f->data_in_index_lc ) {
          if ( p.lc>f->data_in_index_lc ) deallocate_rec(f,&p);
          have_space = true; p = dummy_ptr; p.lc = rlc;
        }
        else { /* new rec goes on disk */
          if ( p.lc>f->data_in_index_lc && (rec_allocation_lc(rlc)==rec_allocation_lc(p.lc)) ) {
            have_space = true; p.lc = rlc;
          }
          else {
            if ( p.lc>f->data_in_index_lc ) deallocate_rec(f,&p);
            have_space =  allocate_rec(f,rlc,&p);
          }
        }
      }
      else {
        have_space = allocate_rec(f,rlc,&p);
      }
      if ( have_space ) {
        insert_rec(f,r,&p); 
        px.p0 = p;
	update_index1(f,&k,&px,bufix,ix,!found);
        kf_set_bof(f,index);
      }
      unlock_buffer(f,bufix);
    }
  }
  return(f->error_code);
}

static int kf_delete_rec(struct fcb *f, int index, unsigned char t[], unsigned key_lc)
{level0_pntr p; struct key k;

  if ( f->read_only ) f->error_code = read_only_err;
  else {
    set_up(f,t,key_lc,&k);
    if (f->error_code==no_err ) {
      index_delete(f,index,k,&p,level_zero);
      if (f->error_code==no_err ) deallocate_rec(f,&p);
      kf_set_bof(f,index);
    }
  }
  return(f->error_code);
}


/* Freespace management */

static boolean contiguous(level0_pntr *p1, level0_pntr *p2)
{
  if ( p1->segment!=p2->segment ) return(false);
  if ( (p1->sc+p1->lc)==p2->sc ) return(true);
  else return(false);
}

static int unpack_16bit(unsigned char key[], UINT16 *n)
{unsigned i;

  *n = 0;
  for (i=0; i<sizeof(UINT16); i++) {
    *n = (*n << 8) + key[i];
  }
  return(sizeof(UINT16));
}

static int unpack_32bit(unsigned char key[], UINT32 *n)
{unsigned i;

  *n = 0;
  for (i=0; i<sizeof(UINT32); i++) {
    *n = (*n << 8) + key[i];
  }
  return(sizeof(UINT32));
}

static int unpack_64bit(unsigned char key[], UINT64 *n)
{unsigned i;

  *n = 0;
  for (i=0; i<sizeof(UINT64); i++) {
    *n = (*n << 8) + key[i];
  }
  return(sizeof(UINT64));
}

static int pack_16bit(unsigned char key[], UINT32 n)
{int i;

  for (i=1; i>=0 ; i--) {
    key[i] = n & 255;
    n = n >> 8;
  }
  return((int)sizeof(UINT16));
}

static int pack_32bit(unsigned char key[], UINT32 n)
{int i;

  for (i=sizeof(UINT32)-1; i>=0 ; i--) {
    key[i] = n & 255;
    n = n >> 8;
  }
  return((int)sizeof(UINT32));
}

static int pack_64bit(unsigned char key[], UINT64 n)
{int i;

  for (i=sizeof(UINT64)-1; i>=0 ; i--) {
    key[i] = n & 255;
    n = n >> 8;
  }
  return((int)sizeof(UINT64));
}

/* unpack_lc_key extracts the lc, segment, and sc portions of key     */
/*   into p.                                                          */

int unpack_lc_key(unsigned char key[], level0_pntr *p)
{int lc; UINT16 segment; UINT32 plc; UINT64 sc;

  lc = unpack_32bit(key,&plc);
  p->lc = plc;
  lc = lc + unpack_16bit(key+lc,&segment);
  p->segment = segment;
  lc = lc + unpack_64bit(key+lc,&sc);
  p->sc = sc;
  return(lc);
}

/* pack_lc_key forms a key string from the lc, segment, and sc fields */
/*   of p and returns the length of the key.                          */

static int pack_lc_key(unsigned char key[], level0_pntr *p)
{int lc;

  lc = pack_32bit(key,p->lc);
  lc = lc + pack_16bit(key+lc,p->segment);
  lc = lc + pack_64bit(key+lc,p->sc);
  return(lc);
}

int unpack_rec_key(unsigned char key[], level0_pntr *p)
{int lc; UINT16 segment; UINT64 sc;

  lc = unpack_16bit(key,&segment);
  p->segment = segment;
  lc = lc + unpack_64bit(key+lc,&sc);
  p->sc = sc;
  return(lc);
}

static int pack_rec_key(unsigned char key[], UINT32 segment, UINT64 sc)
{int lc;

  lc = pack_16bit(key,segment);
  lc = lc + pack_64bit(key+lc,sc);
  return(lc);
}

static void insert_freespace_entry(struct fcb *f, level0_pntr *p0)
{int err; unsigned key_lc,rec_lc; unsigned char key[maxkey_lc],lc_rec[sizeof(UINT32)]; level0_pntr p;

  if ( f->trace_freespace ) {
    print_level0_pntr(f->log_file,f,"inserting freespace entry ",p0);
    fprintf(f->log_file,"\n");
  }
  p.segment = p0->segment;
  p.sc      = p0->sc;
  p.lc      = rec_allocation_lc(p0->lc);
  key_lc = pack_lc_key(key,&p);
  err = kf_put_rec(f,free_lc_ix,key,key_lc,(char *)lc_rec,0);
  if ( err!=no_err ) {
    set_error1(f,free_insrt_err,"**Couldn't insert free_lc entry, err=",err);
  }
  else {
    key_lc = pack_rec_key(key,p0->segment,p0->sc);
    rec_lc = pack_32bit(lc_rec,p.lc);
    err = kf_put_rec(f,free_rec_ix,key,key_lc,(char *)lc_rec,rec_lc);
    if ( err!=no_err ) {
      set_error1(f,free_insrt_err,"**Couldn't insert free_rec entry, err=",err);
    }
  }
}

static void delete_freespace_entry(struct fcb *f, level0_pntr *p0)
{int err; unsigned key_lc; unsigned char key[maxkey_lc]; level0_pntr p;

  if ( f->trace_freespace ) {
    print_level0_pntr(f->log_file,f,"deleting freespace entry ",p0);
    fprintf(f->log_file,"\n");
  }
  p.segment = p0->segment;
  p.sc      = p0->sc;
  p.lc      = rec_allocation_lc(p0->lc);
  key_lc = pack_lc_key(key,&p);
  err = kf_delete_rec(f,free_lc_ix,key,key_lc);
  if ( err!=no_err ) {
    set_error1(f,free_dlt_err,"Couldn't delete free_lc entry, err=",err);
    if ( log_errors ) {
      print_level0_pntr(f->log_file,f,"   ",&p);
      fprintf(f->log_file,"\n");
    }
  }
  else {
    key_lc = pack_rec_key(key,p0->segment,p0->sc);
    err = kf_delete_rec(f,free_rec_ix,key,key_lc);
    if ( err!=no_err ) {
      set_error1(f,free_dlt_err,"Couldn't delete free_rec entry, err=",err);
      if ( log_errors ) {
        print_level0_pntr(f->log_file,f,"**Couldn't delete free_rec entry ",&p);
        fprintf(f->log_file," free_rec_ix=%d\n",free_rec_ix);
      }
    }
  }
}

/* allocate_rec allocates space for a record of size lc bytes.  If */
/*   lc<=data_in_index_lc it does nothing since the rec will be    */
/*   packed in the index block will not be a separate data record. */
/*   It rounds the lc up to an allocation unit and forms a free_lc */
/*   key with segment=0 and sc=0 and looks for the free_lc record. */
/*   If the lookup fails (it will unless the free space is exactly */
/*   the right size and lies at sc=0 in a segment beyond the first)*/
/*   we get the next free_lc key.  If the original key matches or  */
/*   the next key exists then we have a space large enough for the */
/*   record.  We delete the current freespace entry and insert an  */
/*   entry for any residual space.                                 */
/* If we don't have an existing freespace entry of sufficient size */
/*   we allocate a new block of space that is at least large       */
/*   enough and insert a freespace entry for any residual space.   */

static boolean allocate_rec(struct fcb *f, unsigned lc, level0_pntr *p)
{int err; unsigned key_lc,dlc,block_allocate_lc,rec_allocate_lc; boolean have_space=false;
  unsigned char key[maxkey_lc],dummy[2];
 level0_pntr p0,dummy_p0; struct leveln_pntr pn;


 /*  if ( lc>f->data_in_index_lc ) fprintf(f->log_file,"allocating rec lc=%6u, ",lc);*/

  if ( f->trace_freespace ) fprintf(f->log_file,"allocating rec lc=%u\n",lc);
  p->segment = 0; p->sc = 0; p->lc = lc;
  if ( lc<=f->data_in_index_lc ) have_space = true;
  else {
    rec_allocate_lc = rec_allocation_lc(lc);
    p0.segment = 0; p0.sc = 0; p0.lc = rec_allocate_lc;
    key_lc = pack_lc_key(key,&p0);
    err = kf_get_rec(f,free_lc_ix,key,key_lc,&dummy_p0,dummy,&dlc,0);
    if ( err==getnokey_err ) {
      err = kf_next_rec(f,free_lc_ix,key,&key_lc,maxkey_lc,&dummy_p0,dummy,&dlc,0);
    }

    if ( err==ateof_err ) { /* no existing freespace entry big enough */
      f->error_code = no_err;
      block_allocate_lc = allocation_lc(lc,(unsigned)(block_allocation_unit*block_lc));
      have_space = extend_file(f,block_allocate_lc,&pn);
      p->segment = pn.segment;
      p->sc = pn.block << f->block_shift;
      p0.segment = pn.segment;
      p0.sc = p->sc + rec_allocate_lc;
      p0.lc = block_allocate_lc - rec_allocate_lc;
      if ( f->trace_freespace ) fprintf(f->log_file,"extending file by %d bytes, residual=%d\n",
        block_allocate_lc,block_allocate_lc-rec_allocate_lc);
      if ( p0.lc>0 ) insert_freespace_entry(f,&p0);
    }
    else if ( err==no_err) { /* have an entry of sufficient size */
      if ( key_lc!=freespace_lc_key_lc )
        set_error2(f,alloc_rec_err,"**Uh Oh. free_lc_key lc wrong, should be/is",
		   freespace_lc_key_lc,(int)key_lc);
      unpack_lc_key(key,&p0);
      if ( f->trace_freespace )
        fprintf(f->log_file,"using entry=%u/%lu/%u, residual=%d\n",
          p0.segment,p0.sc,p0.lc,p0.lc-rec_allocate_lc);

      if ( p0.lc>=rec_allocate_lc ) {
        have_space = true;
        p->segment = p0.segment;
        p->sc = p0.sc;
        delete_freespace_entry(f,&p0);
        p0.sc = p0.sc + rec_allocate_lc;
        p0.lc = p0.lc - rec_allocate_lc;
        if ( p0.lc>0 ) insert_freespace_entry(f,&p0);
      }
      else set_error(f,alloc_rec_err,"**Uh Oh. Existing freespace entry too small");

    }
    else {
      set_error1(f,alloc_rec_err,"**Couldn't get free_lc entry, err=",err);
    }
  }
  return(have_space);
}

#if 0
static boolean allocate_rec(struct fcb *f, unsigned lc, level0_pntr *p)
{int err; unsigned key_lc,dlc,block_allocate_lc,rec_allocate_lc; boolean have_space=false;
 unsigned char key[maxkey_lc],dummy[2];
 level0_pntr p0,dummy_p0; struct leveln_pntr pn;

  if ( f->trace_freespace ) fprintf(f->log_file,"allocating rec lc=%u\n",lc);
  p->segment = 0; p->sc = 0; p->lc = lc;
  if ( lc<=f->data_in_index_lc ) have_space = true;
  else {
    rec_allocate_lc = rec_allocation_lc(lc);
    p0.segment = 0; p0.sc = 0; p0.lc = rec_allocate_lc;
    key_lc = pack_lc_key(key,&p0);
    err = kf_get_rec(f,free_lc_ix,key,key_lc,&dummy_p0,dummy,&dlc,0);
    if ( err==getnokey_err ) {
      err = kf_next_rec(f,free_lc_ix,key,&key_lc,maxkey_lc,&dummy_p0,dummy,&dlc,0);
    }
    if ( err==ateof_err ) f->error_code = no_err;
    else {
      if ( err!=no_err ) {
        set_error1(f,alloc_rec_err,"**Couldn't get free_lc entry, err=",err);
      }
      if ( key_lc!=freespace_lc_key_lc )
        set_error1(f,alloc_rec_err,"**Uh Oh. Expected free_lc_key to be 14, is=",key_lc);
      unpack_lc_key(key,&p0);
      if ( p0.lc>=rec_allocate_lc ) {
        have_space = true;
        p->segment = p0.segment;
        p->sc = p0.sc;
        delete_freespace_entry(f,&p0);
        p0.sc = p0.sc + rec_allocate_lc;
        p0.lc = p0.lc - rec_allocate_lc;
        if ( p0.lc>0 ) insert_freespace_entry(f,&p0);
      }
    }
    if ( !have_space ) {
      block_allocate_lc = allocation_lc(lc,(unsigned)(block_allocation_unit*block_lc));
      have_space = extend_file(f,block_allocate_lc,&pn);
      p->segment = pn.segment;
      p->sc = pn.block << f->block_shift;
      p0.segment = pn.segment;
      p0.sc = p->sc + rec_allocate_lc;
      p0.lc = block_allocate_lc - rec_allocate_lc;
      if ( f->trace_freespace ) fprintf(f->log_file,"new space, block_lc=%d, residual=%d\n",
        block_allocate_lc,block_allocate_lc-rec_allocate_lc);
      if ( p0.lc>0 ) insert_freespace_entry(f,&p0);
    }
  }
  return(have_space);
}
#endif

/* deallocate_rec deallocates any space allocated with pointer p. */
/*   If p.lc<data_in_index_lc then p doesn't point to a disk record, the  */
/*   data has been stored in p.sc.                                */


static void deallocate_rec(struct fcb *f, level0_pntr *p)
{int err; unsigned key_lc,start_key_lc,rec_lc,dlc;
level0_pntr p0,p1,dummy_p0;
 unsigned char start_key[maxkey_lc],key[maxkey_lc],lc_rec[UINT32_lc],dummy[2];

  if ( f->trace_freespace ) {
    print_level0_pntr(f->log_file,f,"deallocating rec ",p);
    fprintf(f->log_file,"\n");
  }
  if ( p->lc > f->data_in_index_lc ) {
    p0.segment = p->segment;
    p0.sc      = p->sc;
    p0.lc      = rec_allocation_lc(p->lc);
    start_key_lc = pack_rec_key(start_key,p->segment,p->sc);
    err = kf_get_rec(f,free_rec_ix,start_key,start_key_lc,&dummy_p0,lc_rec,&rec_lc,UINT32_lc);
    if ( err==no_err ) {
      set_error1(f,dealloc_rec_err,"Trying to deallocate entry allready in free list",err);
      print_level0_pntr(f->log_file,f,"  entry=",&p0);
      fprintf(f->log_file,"\n");
    }
    err = kf_prev_rec(f,free_rec_ix,key,&key_lc,maxkey_lc,&dummy_p0,lc_rec,&rec_lc,UINT32_lc);
    if ( err==atbof_err ) /* nothing to merge */ {
      if ( f->trace_freespace ) fprintf(f->log_file,"prev is bof\n");
    }
    else if ( err!=no_err ) {
      set_error1(f,dealloc_rec_err,"**Couldn't get prev rec in deallocate_rec, err=",err);
    }
    else {
      key_lc = unpack_rec_key(key,&p1);
      unpack_32bit(lc_rec,&(p1.lc));
      if ( f->trace_freespace ) print_level0_pntr(f->log_file,f,"prev rec is ",&p1);
      if ( contiguous(&p1,&p0) ) {
        delete_freespace_entry(f,&p1);
        p0.sc = p1.sc;
        p0.lc = p0.lc + p1.lc;
        if ( f->trace_freespace ) print_level0_pntr(f->log_file,f,"contiguous, merged entry is ",&p0);
        err = kf_get_rec(f,free_rec_ix,start_key,start_key_lc,&dummy_p0,lc_rec,&rec_lc,UINT32_lc);
      }
      else err = kf_next_rec(f,free_rec_ix,key,&key_lc,maxkey_lc,&dummy_p0,dummy,&dlc,0/*skip current entry*/);
      if ( f->trace_freespace ) fprintf(f->log_file,"\n");
    }
    err = kf_next_rec(f,free_rec_ix,key,&key_lc,maxkey_lc,&dummy_p0,lc_rec,&rec_lc,(int)UINT32_lc);
    if ( err==ateof_err ) /* nothing to merge */ {
      if ( f->trace_freespace ) fprintf(f->log_file,"next is eof\n");
    }
    else if ( err!=no_err ) {
      set_error1(f,dealloc_rec_err,"**Couldn't get next rec in deallocate_rec, err=",err);
    }
    else {
      key_lc = unpack_rec_key(key,&p1);
      unpack_32bit(lc_rec,&(p1.lc));
      if ( f->trace_freespace ) print_level0_pntr(f->log_file,f,"next rec is ",&p1);
      if ( contiguous(&p0,&p1) ) {
        delete_freespace_entry(f,&p1);
        p0.lc = p0.lc + p1.lc;
        if ( f->trace_freespace )
          print_level0_pntr(f->log_file,f," contiguous, merged entry is ",&p0);
      }
      if ( f->trace_freespace ) fprintf(f->log_file,"\n");
    }
    insert_freespace_entry(f,&p0);
  }
}

static void init_free_space(struct fcb *f)
{int bufix; struct leveln_pntr b; struct key dummy;

  bufix = allocate_index_block(f,free_rec_ix,&b,level_zero,&dummy,0);
  f->first_at_level[level_zero][free_rec_ix] = f->buffer_pool.buffer[bufix].contents;
  kf_set_bof(f,free_rec_ix);
  bufix = allocate_index_block(f,free_lc_ix,&b,level_zero,&dummy,0);
  f->first_at_level[level_zero][free_lc_ix] = f->buffer_pool.buffer[bufix].contents;
  kf_set_bof(f,free_lc_ix);
}


/* user callable entries */

int kf7_set_bof(struct fcb *f)
{
  if ( check_fcb(f) ) {
    kf_set_bof(f,user_ix);
  }
  return(f->error_code);
}

int kf7_set_eof(struct fcb *f)
{int ix;

  if ( check_fcb(f) ) {
    if ( f->primary_level[user_ix]==0 ) f->position[user_ix] = f->first_at_level[0][user_ix];
    else f->position[user_ix] = f->last_pntr[1][user_ix];
    ix = get_index(f,f->position[user_ix]);
    f->position_ix[user_ix] = f->buffer_pool.buffer[ix].b.keys_in_block;
  }
  return(f->error_code);
}

int kf7_open_key(struct fcb *f, char id[], int lc, int read_only)
{  
  /*  read_fib(f,id,false,read_only);*/
  read_fib(f,id,machine_is_little_endian(),read_only);
  if ( f->error_code!=no_err ) set_error(f,badopen_err,"");
  else {
    init_key(f,id,lc);
    kf_set_bof(f,user_ix);
    kf_set_bof(f,free_rec_ix);
    kf_set_bof(f,free_lc_ix);
  }
  return(f->error_code);
}

int kf7_close_key(struct fcb *f)
{int i; FILE *temp;

#ifdef log_buffers
  fclose(buffer_log);
#endif
  if ( f->trace ) {
    fprintf(f->log_file,"  read_cnt =%d\n",read_cnt);
    fprintf(f->log_file,"  write_cnt=%d\n",write_cnt);
  }

  /*  printf("  shuffle_cnt=%d\n",shuffle_cnt);
  printf("  shuffle_lt_zero_cnt=%d\n",shuffle_lt_zero_cnt);
  printf("  shuffle_rt_zero_cnt=%d\n",shuffle_rt_zero_cnt);*/
  if ( f->marker!=keyf ) f->error_code = notkeyfil_err;
  else {
    f->error_code = no_err;
    for (i=0; i<f->buffer_pool.buffers_in_use; i++) {
      if (f->buffer_pool.buffer[i].modified){
        write_page(f,f->buffer_pool.buffer[i].contents,&(f->buffer_pool.buffer[i].b) );
        if ( f->trace ) {
          print_leveln_pntr(f->log_file,"  wrote block ",&(f->buffer_pool.buffer[i].contents));
          print_buffer_caption(f->log_file,f,i);
          fprintf(f->log_file," from buffer %d\n",i);
        }
      }
      if (f->buffer_pool.buffer[i].lock_cnt>0) {
        set_error(f,bad_close_err,"**Buffer locked at close\n");
      }
    }
    write_fib(f);
    for (i=0; i<f->open_file_cnt; i++) {
      if (f->trace) fprintf(f->log_file,"  closing segment %d\n",f->open_segment[i]);
      temp = f->open_file[i];
      fclose(temp);
    }
    f->marker = 0;
  }
  return(f->error_code);
}

int kf7_create_key_ld(struct fcb *f, char id[], int fcb_lc, int data_in_index_lc)
{int i,j,bufix; struct leveln_pntr b; struct key dummy;

  f->error_code = no_err;
  f->version = current_version;
  f->sub_version = current_sub_version;
  f->segment_cnt = 0;
  f->marker = keyf; f->file_ok = true;
  f->read_only = false;
  for (i=0; i<max_index; i++) {
    f->primary_level[i] = level_zero;
    for (j=0; j<max_level; j++) {
      f->first_free_block[j][i] = nulln_ptr;
      f->first_at_level[j][i] = nulln_ptr;
      f->last_pntr[j][i] = nulln_ptr;
    }
  }
  f->max_file_lc = 1;
  for (i=0; i<file_lc_bits; i++) f->max_file_lc = f->max_file_lc * 2;
  f->max_file_lc = f->max_file_lc - 1;
  for (i=0; i<max_segment; i++) f->segment_length[i] = 0;
  f->data_in_index_lc = min_data_in_index_lc;
  if ( data_in_index_lc>max_data_in_index_lc ) set_error(f,data_lc_err,"New data_lc too big");
  else if ( data_in_index_lc>min_data_in_index_lc ) f->data_in_index_lc = data_in_index_lc;

  if ( f->error_code==no_err ) {
    f->byte_swapping_required = machine_is_little_endian();
    /*    f->byte_swapping_required = false; */

    init_key(f,id,fcb_lc);
    if ( f->error_code==no_err ) {
      write_fib(f);
      f->segment_cnt = 1; f->segment_length[0] = ((fib_lc_on_disk-1)/block_lc + 1) * block_lc;
      init_free_space(f);
      bufix = allocate_index_block(f,user_ix,&b,level_zero,&dummy,0);
      f->first_at_level[level_zero][user_ix] = f->buffer_pool.buffer[bufix].contents;
      kf_set_bof(f,user_ix);
    }
  }

  return(f->error_code);
}

int kf7_create_key(struct fcb *f, char id[], int fcb_lc)
{

  kf7_create_key_ld(f,id,fcb_lc,(int) min_data_in_index_lc);
  return(f->error_code);
}

int kf7_keyrec_lc(level0_pntr *p)
{
  if ( p->segment==max_segment && p->sc==0 ) return(p->lc);
  else if ( p->segment>=max_segment ) return(-1);
  else return(p->lc);
}

int kf7_next_ptr(struct fcb *f, unsigned char key[], unsigned *key_lc, int max_key_lc,
  keyfile_pointer *pntr)
{int err;

  err = kf_next_ptr(f,user_ix,key,key_lc,max_key_lc,pntr);
  return(err);
}

int kf7_prev_ptr(struct fcb *f, unsigned char key[], unsigned *key_lc, int max_key_lc,
  keyfile_pointer *pntr)
{int err;

  err = kf_prev_ptr(f,user_ix,key,key_lc,max_key_lc,pntr);
  return(err);
}


int kf7_get_ptr(struct fcb *f, unsigned char t[], unsigned key_lc, keyfile_pointer *pntr)
{int err;

  err = kf_get_ptr(f,user_ix,t,key_lc,pntr);
  return(err);
}


int kf7_get_rec(struct fcb *f, unsigned char t[], unsigned key_lc, unsigned char r[],
  int *rlc,int max_lc)
{level0_pntr dummy_p0;

  return( kf_get_rec(f,user_ix,t,key_lc,&dummy_p0,r,(unsigned *)rlc,(unsigned)max_lc) );
}

int kf7_next_rec(struct fcb *f, unsigned char t[], unsigned *key_lc, int max_key_lc,
   unsigned char r[],int *rlc,int max_lc)
{level0_pntr dummy_p0;

  return( kf_next_rec(f,user_ix,t,key_lc,max_key_lc,&dummy_p0,r,(unsigned *)rlc,(unsigned)max_lc) );
}

int kf7_prev_rec(struct fcb *f, unsigned char t[], unsigned *key_lc, int max_key_lc,
   unsigned char r[],int *rlc,int max_lc)
{level0_pntr dummy_p0;

  return( kf_prev_rec(f,user_ix,t,key_lc,max_key_lc,&dummy_p0,r,(unsigned *)rlc,(unsigned)max_lc) );
}

int kf7_put_ptr(struct fcb *f, unsigned char t[], unsigned key_lc, level0_pntr *p)
{
  return( kf_put_ptr(f,user_ix,t,key_lc,*p) );
}

int kf7_delete_ptr(struct fcb *f, unsigned char t[], unsigned key_lc)
{
  return( kf_delete_ptr(f,user_ix,t,key_lc) );
}

int kf7_put_rec(struct fcb *f, unsigned char t[], unsigned key_lc, char r[], int rlc)
{
  return( kf_put_rec(f,user_ix,t,key_lc,r,(unsigned)rlc) );
}

int kf7_delete_rec(struct fcb *f, unsigned char key[], unsigned key_lc)
{
  return( kf_delete_rec(f,user_ix,key,key_lc) );
}

/* Functions to support subrecords: */

/* Function: get_subrec
   Retrieve a part of a record.  This is provided for those cases where the 
   records are so large they must be read into memory from the disk in smaller
   pieces.  Use get_ptr to get the keyfile_pointer, then call get_subrec 
   repeatedly with bytes_to_read set to some manageable size and incrementing 
   offset by bytes_actually_read each time until p->lc bytes have been read.  
   (You must keep track of offset and bytes_to_read so that you don't ask for 
   too much.)

   Returns no_err if all went well, else a non-zero err code.
*/

int kf7_get_subrec(
   struct fcb *f, 
   level0_pntr *p,      // IN - the original pointer from get_ptr
   int offset,                 // IN - offset of data to get within the rec 
   int bytes_to_read,          // IN - how many bytes of the rec to get
   unsigned char *rec,         // IN-OUT - where to put the bytes from the rec
   int *bytes_actually_read,   // OUT - how many bytes were actually transferred
   int max_lc)                 // IN - max bytes to transfer
{size_t size; FILE *file;

  if ( check_fcb(f) ) {
    *bytes_actually_read = bytes_to_read;
    if ( *bytes_actually_read>max_lc ) {
      f->error_code = longrec_err; *bytes_actually_read = max_lc;
    }
    if ( p->lc<=f->data_in_index_lc ) {
      if ( (unsigned)offset+bytes_to_read>=f->data_in_index_lc ) {
        f->error_code = longrec_err; *bytes_actually_read = f->data_in_index_lc - offset - 1;
      }
      memcpy(rec,(unsigned char *)(&p->data_rec)+offset,(size_t) *bytes_actually_read);
    }
    else {
      file = file_index(f,p->segment);
      if ( f->error_code!=no_err ) return(f->error_code);
      if ( fseeko(file,(FILE_OFFSET)p->sc+offset,0)!=0 ) {
        f->error_code = seek_err; return(f->error_code);
      }
      size = fread(rec,(size_t) 1,(size_t) *bytes_actually_read,file);
      if ( size!=(size_t)*bytes_actually_read ) f->error_code = read_err;
    }
  }
  return(f->error_code);
}


/* Multi version shell.  These calls are an interface to multiple    */
/*   keyed file versions.  Creates always create a keyed file in the */
/*   current version.  Opens read enough to the file to determine    */
/*   which open_key version to use.  Thereafter, the version in the  */
/*   fcb is used to determine which version of the API function to   */
/*   call.  If the version is not in the range of supported versions */
/*   then version_err is returned.  If the function called is not    */
/*   supported by the version opened then not_supported_err is       */
/*   returned.                                                       */

/* get_kf_version reads the version from the fcb.  This is not an    */
/*   exhaustive test (doesn't check marker or fcb_ok) since the      */
/*   appropriate open call will do more complete testing.            */

UINT32 get_kf_version(struct fcb *f, char id[])
{boolean err=false; UINT32 version=0; FILE *file;

  file = fopen(id,"rb");
  if ( file==NULL ) err = true;
  else {
    if ( fseeko(file,(FILE_OFFSET) 0,0)!=0 ) err = true;
    else {
      if ( fread(&version,sizeof(UINT32),(size_t)1,file)!=1 ) err = true;
      if ( fread(&version,sizeof(UINT32),(size_t)1,file)!=1 ) err = true;
      if ( machine_is_little_endian() ) byte_swap_UINT32((unsigned char *) &version);
    }
    fclose(file);
  }
  if ( err ) return(0);
  else return(version);
}


#ifndef COMPILE_MULTI_VERSION

#define kf6_keyrec_lc(a)            version_err
#define kf6_set_bof(a)              version_err
#define kf6_set_eof(a)              version_err
#define kf6_get_ptr(a,b,c,d)        version_err
#define kf6_put_ptr(a,b,c,d)        version_err
#define kf6_delete_ptr(a,b,c)       version_err
#define kf6_next_ptr(a,b,c,d,e)     version_err
#define kf6_prev_ptr(a,b,c,d,e)     version_err
#define kf6_get_rec(a,b,c,d,e,f)    version_err
#define kf6_put_rec(a,b,c,d,e)      version_err
#define kf6_delete_rec(a,b,c)       version_err
#define kf6_next_rec(a,b,c,d,e,f,g) version_err
#define kf6_prev_rec(a,b,c,d,e,f,g) version_err
#define kf6_get_subrec(a,b,c,d,e,f,g) version_err
#define kf6_open_key(a,b,c,d)       version_err
#define kf6_create_key(a,b,c)       version_err
#define kf6_create_key_ld(a,b,c,d)  version_err
#define kf6_close_key(a)            version_err
#else
#include "kf6ref.h"
#endif

int set_bof(struct fcb *f)
{int err;

  if ( f->version==7 )      err = kf7_set_bof(f);
  else if ( f->version<=6 ) err = kf6_set_bof(f);
  else                      err = version_err;
  return(err);
}

int set_eof(struct fcb *f)
{int err;

  if ( f->version==7 )      err = kf7_set_eof(f);
  else if ( f->version<=6 ) err = kf6_set_eof(f);
  else                      err = version_err;
  return(err);
}

int open_key(struct fcb *f, char id[], int lc, int read_only)
{int err,version;

  version = get_kf_version(f,id);
  if ( version==7 )      err = kf7_open_key(f,id,lc,read_only);
  else if ( version<=6 ) err = kf6_open_key(f,id,lc,read_only);
  else                   err = version_err;
  return(err);
  
}

int close_key(struct fcb *f)
{int err;

  if ( f->version==7 )      err = kf7_close_key(f);
  else if ( f->version<=6 ) err = kf6_close_key(f);
  else                      err = version_err;
  return(err);
}

int create_key_ld(struct fcb *f, char id[], int fcb_lc, int data_in_index_lc)
{int err;

  err = kf7_create_key_ld(f,id,fcb_lc,data_in_index_lc);
  return(err);
}

int create_key(struct fcb *f, char id[], int fcb_lc)
{int err;

  err = kf7_create_key(f,id,fcb_lc);
  return(err);
}

/* keyrec_lc to be removed when UMass can fix their code base */

int keyrec_lc(level0_pntr *p)
{int lc;

  lc = kf7_keyrec_lc(p);
  return(lc);
}

int next_ptr(struct fcb *f, char key[], int *key_lc, int max_key_lc, keyfile_pointer *pntr)
{int err;

  if ( f->version==7 )      err = kf7_next_ptr(f,(unsigned char *)key,(unsigned *)key_lc,max_key_lc,pntr);
  else if ( f->version<=6 ) err = kf6_next_ptr(f,key,key_lc,max_key_lc,pntr);
  else                      err = version_err;
  return(err);
}

int prev_ptr(struct fcb *f, char key[], int *key_lc, int max_key_lc, keyfile_pointer *pntr)
{int err;

  if ( f->version==7 )      err = kf7_prev_ptr(f,(unsigned char *)key,(unsigned *)key_lc,max_key_lc,pntr);
  else if ( f->version<=6 ) err = kf6_prev_ptr(f,key,key_lc,max_key_lc,pntr);
  else                      err = version_err;
  return(err);
}


int get_ptr(struct fcb *f, char key[], int key_lc, keyfile_pointer *pntr)
{int err;

  if ( f->version==7 )      err = kf7_get_ptr(f,(unsigned char *)key,(unsigned)key_lc,pntr);
  else if ( f->version<=6 ) err = kf6_get_ptr(f,key,key_lc,pntr);
  else                      err = version_err;
  return(err);
}


int get_rec(struct fcb *f,char key[],int key_lc, char r[],int *rlc,int max_lc)
{int err;

  if ( f->version==7 )      err = kf7_get_rec(f,(unsigned char *)key,(unsigned)key_lc,
                                    (unsigned char *)r,rlc,max_lc);
  else if ( f->version<=6 ) err = kf6_get_rec(f,key,key_lc,r,rlc,max_lc);
  else                      err = version_err;
  return(err);
}

int next_rec(struct fcb *f, char key[], int *key_lc, int max_key_lc,
   char r[],int *rlc,int max_lc)
{int err;

  if ( f->version==7 )      err = kf7_next_rec(f,(unsigned char *)key,(unsigned *)key_lc,
                                    max_key_lc,(unsigned char *)r,rlc,max_lc);
  else if ( f->version<=6 ) err = kf6_next_rec(f,key,key_lc,max_key_lc,r,rlc,max_lc);
  else                      err = version_err;
  return(err);
}

int prev_rec(struct fcb *f, char key[], int *key_lc, int max_key_lc,
   char r[],int *rlc,int max_lc)
{int err;

  if ( f->version==7 )      err = kf7_prev_rec(f,(unsigned char *)key,(unsigned *)key_lc,
                                    max_key_lc,(unsigned char *)r,rlc,max_lc);
  else if ( f->version<=6 ) err = kf6_prev_rec(f,key,key_lc,max_key_lc,r,rlc,max_lc);
  else                      err = version_err;
  return(err);
}

int put_ptr(struct fcb *f, char key[], int key_lc, level0_pntr *p)
{int err;

  if ( f->version==7 )      err = kf7_put_ptr(f,(unsigned char *)key,(unsigned)key_lc,p);
  else if ( f->version<=6 ) err = kf6_put_ptr(f,key,key_lc,p);
  else                      err = version_err;
  return(err);
}

int delete_ptr(struct fcb *f, char key[], int key_lc)
{int err;

  if ( f->version==7 )      err = kf7_delete_ptr(f,(unsigned char *)key,(unsigned)key_lc);
  else if ( f->version<=6 ) err = kf6_delete_ptr(f,key,key_lc);
  else                      err = version_err;
  return( err );
}

int put_rec(struct fcb *f, char key[], int key_lc, char r[], int rlc)
{int err=0;

  if ( f->version==7 )      err = kf7_put_rec(f,(unsigned char *)key,(unsigned)key_lc,r,rlc);
  else if ( f->version<=6 ) err = kf6_put_rec(f,key,key_lc,r,rlc);
  else                      err = version_err;
  return(err);
}

int delete_rec(struct fcb *f, char key[], int key_lc)
{int err;

  if ( f->version==7 )      err = kf7_delete_rec(f,(unsigned char *)key,(unsigned)key_lc);
  else if ( f->version<=6 ) err = kf6_delete_rec(f,key,key_lc);
  else                      err = version_err;
  return(err);
}

int get_subrec(
   struct fcb *f, 
   level0_pntr *p,      // IN - the original pointer from get_ptr
   int offset,                 // IN - offset of data to get within the rec 
   int bytes_to_read,          // IN - how many bytes of the rec to get
   unsigned char *rec,         // IN-OUT - where to put the bytes from the rec
   int *bytes_actually_read,   // OUT - how many bytes were actually transferred
   int max_lc)                 // IN - max bytes to transfer
{int err;

  if ( f->version==7 )     err = kf7_get_subrec(f,p,offset,bytes_to_read,rec,bytes_actually_read,max_lc);
  else if ( f->version<=6 )err = kf6_get_subrec(f,p,offset,bytes_to_read,rec,bytes_actually_read,max_lc);
  else                     err = version_err;
  return(err);
}


