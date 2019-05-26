#if !defined(KEYDEF_H)
#define KEYDEF_H

/*                                                               */
/* Copyright 1984,1985,1986,1988,1989,1990,2003,2004,2005,       */
/*   2006, 2007 by Howard Turtle                                 */
/*                                                               */

#include <math.h>
#include "integer_types.h"

/*#ifdef WIN32
typedef unsigned short     UINT16;
typedef unsigned int       UINT32;
typedef unsigned __int64   UINT64;
#define UINT64_format "%llu"
#define UINT64_C(c)   c ## ULL
#define PATH_SEPARATOR '\\'
typedef struct F_HANDLE F_HANDLE;
#else
#include <inttypes.h>
typedef uint16_t           UINT16;
typedef uint32_t           UINT32;
typedef uint64_t           UINT64;
#define UINT64_format "%" PRIu64
#define PATH_SEPARATOR '/'
#define F_HANDLE FILE
#endif
*/

#define bits(i) ( (int) (log((double)i-1)/log(2.0)) + 1 )
#define compressed_bits_per_byte 7
#define compressed_lc(bits) ( (bits-1) / compressed_bits_per_byte + 1 )

#define eqn_pntr(p1,p2) ((p1.block==p2.block) && (p1.segment==p2.segment))
#define lt_pntr(p1,p2) ((p1.segment<p2.segment) || ((p1.segment==p2.segment) && (p1.block<p2.block)))
#define null_pntr(p1) ((p1.segment==max_segment) && (p1.block==0))
#define mvc(t1,sc1,t2,sc2,lc) memmove((unsigned char *)t2+sc2,(unsigned char *)t1+sc1,(size_t)lc)

#define keyf                   32472 /* marker for fcb */
#define current_version            7 /* version of keyed file software */
#define current_sub_version        0
#define maxkey_lc                512 /* maximum key length */
#define max_prefix_lc            127 /* max index block prefix */
#define level_zero                 0 /* level of index leaves */
#define level_one                  1 /* level immediately above leaves */
#define min_data_in_index_lc       6 /* records <= this go in index block */
#define max_data_in_index_lc     128 /* longest record that can go in index block */
#define seq_threshold             20
#define min_buffer_cnt             8 /* default number of buffers allocated */
/*#define max_buffer_cnt         32768*/ /* max buffers allowed */
#define buf_hash_load_factor       3 /* hash table is>=this times buffers alloc,*/
#define max_level                 32 /* number of index block levels */
#define file_lc_bits              31 /* <64, max_file_lc = 2**file_lc_bits - 1 */
#define max_segment              127 /* max number of file segments */
#define max_files                 10 /* max number of open files */
#define max_filename_lc          128 /* max length of a file name */
#define max_extension_lc          40 /* max length of file name extension */
/*#define v6_block_lc             4096*/
#define block_lc                4096
#define rec_allocation_unit        8 /* data recs allocated as multiple of this */
#define block_allocation_unit     16 /* # index (or freespace) blocks to allocate */
#define max_allocation_depth       4
#define user_ix                    0
#define free_rec_ix                1
#define free_lc_ix                 2
#define max_index                  3
#define cmp_less                   0
#define cmp_equal                  1
#define cmp_greater                2

#define UINT32_lc      (unsigned)sizeof(UINT32)
#define freespace_lc_key_lc 14       /* length of a freespace lc key */
#define freespace_rec_key_lc 10      /* length of a freespace rec key */

enum comparison {less,equal,greater};

struct key {
  unsigned char
    text[maxkey_lc];
  UINT16
    lc;
};

/* leveln_pntrs point to index blocks and are the pointers stored  */
/*   in index blocks above level0.  They are always compressed     */
/*   when stored in index blocks; segment is usually small (less   */
/*   that max_segment), block is a block number (not a file       */
/*   offset).  leveln_lc is the size of the pointer on disk.       */

struct leveln_pntr{
  UINT16
    segment;                    /* in range 0..max_segment  */
  UINT64
    block;
};
#define leveln_lc (sizeof(UINT16)+sizeof(UINT64))
#define compressed_block_lc ( compressed_lc( file_lc_bits - bits(block_lc) ) )
#define compressed_leveln_lc ( compressed_lc(bits(max_segment)) + compressed_block_lc )

/* level0_pntrs point to (or contain) data records and are only   */
/*   found in level0 blocks.  Segment is the file segment in      */
/*   which the record lies (short).  sc is the byte offset in the */
/*   file at which the record starts, lc is the length of the     */
/*   record.  If the record will fit in sizeof(UINT64) then it is */
/*   stored in sc rather than on disk.                            */

/* Starting with version 7, level0 pntrs are stored differently.  */
/*   lc is compressed first.  If lc<=f->data_in_index_lc then lc  */
/*   is followed by the actual data associated with the key       */
/*   (uncompressed).  If lc>f->data_in_index_lc then lc is        */
/*   followed by segment and sc that define the disk offset to    */
/*   the data stored on disk.                                     */

struct disk_offset {
  UINT16
    segment;
  UINT64
    sc;
};

typedef union data_or_disk_offset_t {
  struct disk_offset    offset;
  unsigned char        data[max_data_in_index_lc];
} data_or_disk_offset;

struct internal_level0_pntr {
  UINT16
    segment;
  UINT32
    lc;
  UINT64
    sc;
  unsigned char
    data_rec[max_data_in_index_lc];
};
typedef struct internal_level0_pntr level0_pntr;

struct external_level0_pntr {
  UINT16
    segment;
  UINT32
    lc;
  UINT64
    sc;
};
typedef struct internal_level0_pntr keyfile_pointer; /* external name for Chiliad use */


#define ix_block_header_lc (2*sizeof(UINT16)+ 4 +2*leveln_lc)
#define key_ptrs_per_block ((block_lc - ix_block_header_lc) / sizeof(UINT16))
#define keyspace_lc (sizeof(UINT16)*key_ptrs_per_block)

struct ix_block {                  /* block is the disk resident image of */
  UINT16                         /*   an index block */
    keys_in_block,
    chars_in_use;               /* chars in key/pointer pool, does not */
                                /*   include length of key_ptr_t entries */
  unsigned char
    index_type,                 /* user, free_lc, or free_rec */
    prefix_lc,                  /* lc of prefix removed from all keys in block */
    unused,
    level;                      /* level of block */
  struct leveln_pntr
    next,prev;
  UINT16                        /* key_ptrs are inserted from 0, keys and */
    keys[key_ptrs_per_block];   /*  file pointers overlay the top end */
};
typedef struct ix_block block_type_t;
#define ix_block_lc sizeof(block_type_t)


/* Free space management.  Available space is recorded in two separate */
/*   indexes.  The first (free_rec_ix) records each space in address   */
/*   order using a binary key of segment/sc and lc as the record,  The */
/*   second (free_lc_ix) records each space in length order using a    */
/*   key of lc/segment/sc.  To allocate a record the lc list is        */
/*   searched with a key of lc/0/0 then next_rec is used to find a     */
/*   space of lc or longer (if it exists).  To deallocate, the rec     */
/*   list searched and any contiguous entries are combined.            */

typedef union level0orn_pntr {
  level0_pntr            p0;
  struct leveln_pntr     pn;
} levelx_pntr;

/* Buffer handling.  Buffers contain the disk image of an index or    */
/*   freespace block */
/*   together with additional information.  A hashing technique is    */
/*   used to find a buffer that holds a given block.  A hash table is */
/*   allocated as the last buffers in the fcb of roughly three times  */
/*   the number of buffers allocated.  buf_hash_table[k] contains the */
/*   index of the first buffer containing a block whose hash value is */
/*   k.  If there are multiple buffers containing blocks with hash    */
/*   value k then they are linked using hash_next.                    */

struct buffer_struct {            /* buffer is the memory resident image of */
                                /* a disk block */
  unsigned char
    lock_cnt,
    modified,
    notused;
  int
    older,                      /* index to prev element in LRU list */
    younger,                    /* index to next element in LRU list */
    hash_next,
    search_cnt;
  struct leveln_pntr
    contents;                   /* block in buffer, nulln_ptr if empty */
  block_type_t
    b;
};
typedef struct buffer_struct buffer_t;
#define buffer_lc sizeof(buffer_t)
#define hash_entries_per_buf (buffer_lc / sizeof(int))

struct buffer_pool_struct {
  int
    buffers_allocated,          /* number of buffers allocated */
    buffers_in_use,             /* buffers actually used */
    *buf_hash_table,            /* pointer to base of buffer hash table */
    buf_hash_entries;           /* size of buf_hash_table              */
  buffer_t
    buffer[min_buffer_cnt];    /* should be at end of fcb so we can extend */
};
typedef struct buffer_pool_struct buffer_pool_t;
#define min_buffer_pool_lc (sizeof(buffer_pool_t) + min_buffer_cnt*sizeof(buffer_t))


/* File information block.  The following information is stored at the */
/*   start of segment 0 for all keyed files.  The sizes are            */
/*   fixed for all keyed files but the information is kept in the fcb  */
/*   in whatever form fits the architecture at hand.  Note that this   */
/*   struct is never actually used -- it's here to document the        */
/*   structure of the fib.  fib_lc_on_disk is the length of the fib on disk -- */
/*   it doesn't include alignment padding inserted by a compiler.      */

struct file_information_block {
  UINT32
    error_code,
    version,                    /* version of keyed file manager */
    sub_version,
    segment_cnt,                /* number of segments in use     */
    primary_level[max_index],              /* level of primary index block */
    marker,
    file_ok;
  struct leveln_pntr
    first_free_block[max_level][max_index],/* points to start of empty block chain */
    first_at_level[max_level][max_index],  /* block containing lowest key at level */
    last_pntr[max_level][max_index];       /* last pointer at each level */
  UINT64
    max_file_lc,
    segment_length[max_segment];/* length in bytes of each segment     */
  UINT32
    data_in_index_lc;

};
#define fib_lc_on_disk ((7+max_index)*sizeof(UINT32)+(3*max_level*max_index)*leveln_lc+(max_segment+1)*sizeof(UINT64))


/* Segment handling.  A keyed file consist of one or more component files */
/*    called segments.  Segment 0 contains the file information block and */
/*    is alway present.  Additional segments are created as needed with   */
/*    a suffix of "$n" appended to the base file name where n is the      */
/*    segment number.  The file information block contains a segment_cnt  */
/*    and a list of each segment_length.  After open the fcb contains a   */
/*    list of the file number on which each segment is open (max_files    */
/*    implies not open) in segment_ix                                     */

/* File handling.  Up to max_files files may be open at one time.         */
/*   open_file_cnt is the number of files actually open, open_file[] is   */
/*   a list of file_indexes in use, file_age[] is the age of each open    */
/*   file, open_segment[] is the segment to which the file is open        */

struct fcb {

  UINT32
    error_code,
    version,                    /* version of keyed file manager */
    sub_version,
    segment_cnt,                /* number of segments in use     */
    primary_level[max_index],              /* level of primary index block */
    marker,
    file_ok;
  struct leveln_pntr
    first_free_block[max_level][max_index],/* points to start of empty block chain */
    first_at_level[max_level][max_index],  /* block containing lowest key at level */
    last_pntr[max_level][max_index];       /* last pointer at each level */
  UINT64
    max_file_lc,                 /* max file lc for file system (2**file_lc_bits - 1)*/
    segment_length[max_segment];/* length in bytes of each segment     */
  UINT32
    data_in_index_lc;            /* data recs <= this go in index */

    /* end of fib information, temporary information follows */

  char
    file_name[max_filename_lc],
    file_extension[max_extension_lc],
    *search_block_caller;       /* pointer to name of caller for debug */
  unsigned char
    byte_swapping_required,     /* true means swap bytes on I/O */
    trace,                      /* true means trace execution */
    trace_freespace,            /* true means trace space management */
    read_only;                  /* true means file is read only */
  int
    open_file_cnt,              /* number of files actually open */
    open_segment[max_files],    /* segment to which each file is open */
    file_age[max_files],        /* age of each open file  */
    oldest_buffer,              /* first buffer in LRU buffer list */
    youngest_buffer,            /* last buffer in LRU buffer list */
    block_shift;                /* log2(block_lc) */
  F_HANDLE
    *log_file,
    *open_file[max_files];      /* pointers to open files */

  int
    segment_ix[max_segment],   /* index into open_file[] if segment open */
    position_ix[max_index],                /* posn. in level0 blk of last retrieval */
    seq_cnt[max_index],
    current_age;                /* age of file pool (0..maxint)*/
  struct leveln_pntr
    mru_at_level[max_level][max_index],    /* most recently used block at each level*/
    position[max_index];                   /* level0 block of last retrieval */

  buffer_pool_t
    buffer_pool;     /* should be at end of fcb so we can extend */
  /*  buffer_t
      buffer[min_buffer_cnt];*/
};
#define min_fcb_lc sizeof(struct fcb)

#endif
