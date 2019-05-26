/*                                                                    */
/* Copyright 1984,1985,1986,1988,1989,1990,2003,2004,2005,2006,2007   */
/*    by Howard Turtle                                                */
/*                                                                    */

#define boolean int
#define true 1
#define false 0

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "keydef.h"
#include "keyerr.h"
#include "keyprint.h"
#include "keyfile.h"

static void print_text_or_binary(FILE *list, unsigned char t[], unsigned int lc)
{unsigned int i; boolean is_text;

  is_text = true;
  for (i=0; i<lc; i++) if ( !isprint(t[i]) ) is_text = false;
  if ( lc==0 ) fprintf(list,"null");
  else if ( is_text ) for (i=0; i<lc; i++) fprintf(list,"%c",t[i]);
  else {
    fprintf(list,"0x");
    for (i=0; i<lc; i++) fprintf(list,"%02x",t[i]);
  }
}

void print_leveln_pntr(FILE *list, char caption[], struct leveln_pntr *pn)
{
  fprintf(list,"%s%4u/",caption,pn->segment);
  fprintf(list,UINT64_format,pn->block);
}

void print_level0_pntr(FILE *list, struct fcb *f, char caption[], level0_pntr *p0)
{
  if ( p0->lc <= f->data_in_index_lc ) {
    fprintf(list,"%s%u/",caption,p0->lc);
    print_text_or_binary(list,p0->data_rec,p0->lc);
  }
  else {
    fprintf(list,"%s%u/",caption,p0->lc);
    fprintf(list,"%u/",p0->segment);
    fprintf(list,UINT64_format,p0->sc);
  }

}

/* key printing */

static void print_lc_key(FILE *list, struct key *k, char caption[])
{level0_pntr p;

  unpack_lc_key(k->text,&p);
  fprintf(list,"%s(free_lc)%u, %u/",caption,p.lc,p.segment);
  fprintf(list,UINT64_format,p.sc);
}

static void print_rec_key(FILE *list, struct key *k, char caption[])
{level0_pntr p;

  unpack_rec_key(k->text,&p);
  fprintf(list,"%s(free_rec)%u/",caption,p.segment);
  fprintf(list,UINT64_format,p.sc);
}

#define print_abbreviated_keys true

static void print_key_struct(FILE *list, struct key *k, char caption[])
{int i,lc; boolean is_text;

  is_text = true;
  lc = k->lc;
  for (i=0; i<lc; i++) if ( !isprint(k->text[i]) ) is_text = false;
  fprintf(list,"%s",caption);
  if ( lc==0 ) fprintf(list,"null");
  else if ( is_text ) {
    if ( print_abbreviated_keys && lc>127 ) lc = 127; 
    for (i=0; i<lc; i++) fprintf(list,"%c",k->text[i]);
  }
  else {
    if ( print_abbreviated_keys && lc>63 ) lc = 63; 
    fprintf(list,"0x");
    for (i=0; i<lc; i++) fprintf(list,"%02x",k->text[i]);
  }
}

void print_key(FILE *list, int index, struct key *k, char caption[])
{
  if ( index==user_ix ) print_key_struct(list,k,caption);
  else if ( index==free_rec_ix ) print_rec_key(list,k,caption);
  else if ( index==free_lc_ix )  print_lc_key(list,k,caption);
}

/* index block printing */

static void print_index_type(FILE *list, int index_type)
{
  if (index_type==user_ix) fprintf(list,"user");
  else if (index_type==free_lc_ix) fprintf(list,"free_lc");
  else if (index_type==free_rec_ix) fprintf(list,"free_rec");
  else fprintf(list,"unknown");
}

static void print_ix_block_entry(FILE *list, struct fcb *f, struct ix_block *b, int i)
{int lc,temp; struct key k; level0_pntr p0; struct leveln_pntr pn;

  get_nth_key(b,&k,i);
  lc = k.lc;
  fprintf(list,"%4d %4d ",b->keys[i],lc);
  print_key(list,b->index_type,&k,"");
  if ( b->level>0 ) {
    temp = unpackn_ptr(b,i,&pn);
    print_leveln_pntr(list," - ",&pn);
    fprintf(list,"(lc=%d)\n",temp);
  }
  else {
    temp = unpack0_ptr(f,b,i,&p0);
    print_level0_pntr(list,f," - ",&p0);
    fprintf(list,"(ptr_lc=%d)\n",temp);
  }
}

void print_index_block(FILE *list, struct fcb *f, struct ix_block *b)
{int i,lc,load; unsigned char t[maxkey_lc];

  fprintf(list," keys=%d",b->keys_in_block);
  lc = b->chars_in_use + (b->keys_in_block * sizeof(UINT16));
  load = (lc*100) / (keyspace_lc);
  fprintf(list,", chars=%d(%d) %d%% loaded",b->chars_in_use,lc,load);
  fprintf(list,", level=%d",b->level);
  fprintf(list,", ix_type=");
  print_index_type(list,b->index_type);
  fprintf(list,", prefix_lc=%d",b->prefix_lc);
  if ( b->prefix_lc>0 ) {
    fprintf(list,", prefix=");
    mvc(b->keys,keyspace_lc-b->prefix_lc,t,0,b->prefix_lc);
    print_text_or_binary(list,t,b->prefix_lc);
    /*    for (i=0; i<b->prefix_lc; i++) 
      if ( b->index_type==user_ix ) fprintf(list,"%c",t[i]);
      else fprintf(list,"%2x ",t[i]);*/
    fprintf(list,"\n");
  }
  print_leveln_pntr(list,"   next=",&(b->next));
  print_leveln_pntr(list," prev=",&(b->prev));
  fprintf(list,"\n");
  if ( b->keys_in_block<50 ) {
    for (i=0; i<b->keys_in_block; i++) print_ix_block_entry(list,f,b,i);
  }
  else {
    for (i=0; i<20; i++) print_ix_block_entry(list,f,b,i);
    fprintf(list,"    ...\n");
    for (i=b->keys_in_block-20; i<b->keys_in_block; i++) print_ix_block_entry(list,f,b,i);
  }
}

/* buffer printing */

static boolean is_primary(struct fcb *f, int bufix)
{int index;

  index = f->buffer_pool.buffer[bufix].b.index_type;
  if ( f->buffer_pool.buffer[bufix].b.level==f->primary_level[index] ) return(true);
  else return(false);
}

int print_hash_chain(FILE *list,struct fcb *f, int ix)
{int next,cnt=0;

  next = f->buffer_pool.buf_hash_table[ix];
  while ( next>=0 ) {
    if ( is_primary(f,next) ) fprintf(list,"**");
    fprintf(list," %d(",next);
    print_leveln_pntr(list,"",&(f->buffer_pool.buffer[next].contents));
    fprintf(list,")(%d)",f->buffer_pool.buffer[next].b.level);
    if ( is_primary(f,next) ) fprintf(list,"**");
    cnt++;
    next = f->buffer_pool.buffer[next].hash_next;
  }
  fprintf(list,"\n");
  return(cnt); 
}

void print_buffer_caption(FILE *list, struct fcb *f, int ix)
{int index_type,level;

  index_type = f->buffer_pool.buffer[ix].b.index_type;
  level = f->buffer_pool.buffer[ix].b.level;
  fprintf(list,"%4d(%2d/" UINT64_formatf(-6),
    ix,f->buffer_pool.buffer[ix].contents.segment,f->buffer_pool.buffer[ix].contents.block);
  if (index_type==user_ix) {
    if ( is_primary(f,ix) ) fprintf(list,"X%2d",level);
    else fprintf(list,"x%2d",level);
  }
  else if (index_type==free_lc_ix) {
    if ( is_primary(f,ix) ) fprintf(list,"L%2d",level);
    else fprintf(list,"l%2d",level);
  }
  else if (index_type==free_rec_ix) {
    if ( is_primary(f,ix) ) fprintf(list,"R%2d",level);
    else fprintf(list,"r%2d",level);
  }
  if ( f->buffer_pool.buffer[ix].modified ) fprintf(list,"*");
  else fprintf(list," ");
  if ( f->buffer_pool.buffer[ix].lock_cnt>0 ) fprintf(list,"!)");
  else fprintf(list," )");
}

void print_buffer_MRU_chain(FILE *list,struct fcb *f)
{int ix,cnt=0;

  ix = f->youngest_buffer;
  while ( ix>=0 ) {
    if ( (cnt%5)==0) fprintf(list,"\n    ");
    cnt++;
    print_buffer_caption(list,f,ix);
    ix = f->buffer_pool.buffer[ix].older;
  }
  fprintf(list,"\n");
}



