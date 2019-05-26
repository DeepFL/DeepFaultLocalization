#ifndef _KEYREF_H_
#define _KEYREF_H_

int keyrec_lc    (void *pointer);
int set_bof      (void *f);
int set_eof      (void *f);
int get_ptr      (void *f, char *key, int key_lc, void *pointer);
int put_ptr      (void *f, char *key, int key_lc, void *pointer);
int delete_ptr   (void *f, char *key, int key_lc);
int next_ptr     (void *f, char *key, int *key_lc, int max_key_lc, void *pointer);
int prev_ptr     (void *f, char *key, int *key_lc, int max_key_lc, void *pointer);
int get_rec      (void *f, char *key, int key_lc, void *rec, int *lc, int max_rec_lc);
int put_rec      (void *f, char *key, int key_lc, void *rec, int lc);
int delete_rec   (void *f, char *key, int key_lc);
int next_rec     (void *f, char *key, int *key_lc, int max_key_lc,
                          void *rec, int *rec_lc, int max_rec_lc);
int check_fcb(struct fcb *f);
int prev_rec     (void *f, char *key, int *key_lc, int max_key_lc,
                          void *rec, int *rec_lc, int max_rec_lc);
int get_subrec   (void *f, void *pointer, int offset, int bytes_to_read,
                           void *rec, int *bytes_actually_read, int max_lc);
int open_key     (void *f, char *id, int fcb_lc, int read_only);
int create_key   (void *f, char *id, int fcb_lc);
int create_key_ld(void *f, char *id, int fcb_lc, int data_in_index_lc);
int close_key    (void *f);

#endif
