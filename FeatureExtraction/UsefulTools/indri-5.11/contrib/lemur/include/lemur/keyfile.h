#if !defined(KEYFILE_H)
#define KEYFILE_H

int  unpackn_ptr  (struct ix_block *b, int ix, struct leveln_pntr *p);
int  unpack0_ptr  (struct fcb *f, struct ix_block *b, int ix, level0_pntr *p);
int  get_nth_key  (struct ix_block *b, struct key *k, int n);
int  get_nth_key_and_pntr(struct fcb *f, struct ix_block *b, struct key *k, int n,
                   levelx_pntr *p);
void get_page     (struct fcb *f, struct leveln_pntr blk, block_type_t *buf);
void kf_set_bof   (struct fcb *f, int index);
int  kf_next_rec  (struct fcb *f, int index, unsigned char t[], unsigned *key_lc,
                   int max_key_lc, level0_pntr *p, unsigned char r[],
                   unsigned *rec_lc, unsigned max_rec_lc);
int unpack_lc_key (unsigned char key[], level0_pntr *p);
int unpack_rec_key(unsigned char key[], level0_pntr *p);

#endif
