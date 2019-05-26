#if !defined(KEYPRINT_H)
  #define KEYPRINT_H

void print_level0_pntr(FILE *list, struct fcb *f, char caption[], level0_pntr *p0);
void print_leveln_pntr(FILE *list, char caption[], struct leveln_pntr *pn);
void print_key(FILE *list, int index_type, struct key *k, char caption[]);
void print_index_block(FILE *list, struct fcb *f, struct ix_block *b);

int  print_hash_chain(FILE *list,struct fcb *f, int ix);
void print_buffer_caption(FILE *list, struct fcb *f, int index_type);
void print_buffer_MRU_chain(FILE *list, struct fcb *f);

#endif
