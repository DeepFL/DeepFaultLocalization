#ifndef _KF6REF_H_
#define _KF6REF_H_

int kf6_keyrec_lc    (void *pointer);
int kf6_set_bof      (void *f);
int kf6_set_eof      (void *f);
int kf6_get_ptr      (void *f, char *key, int key_lc, void *pointer);
int kf6_put_ptr      (void *f, char *key, int key_lc, void *pointer);
int kf6_delete_ptr   (void *f, char *key, int key_lc);
int kf6_next_ptr     (void *f, char *key, int *key_lc, int max_key_lc, void *pointer);
int kf6_prev_ptr     (void *f, char *key, int *key_lc, int max_key_lc, void *pointer);
int kf6_get_rec      (void *f, char *key, int key_lc, void *rec, int *lc, int max_rec_lc);
int kf6_put_rec      (void *f, char *key, int key_lc, void *rec, int lc);
int kf6_delete_rec   (void *f, char *key, int key_lc);
int kf6_next_rec     (void *f, char *key, int *key_lc, int max_key_lc,
                          void *rec, int *rec_lc, int max_rec_lc);
int kf6_prev_rec     (void *f, char *key, int *key_lc, int max_key_lc,
                          void *rec, int *rec_lc, int max_rec_lc);
int kf6_get_subrec   (void *f, void *pointer, int offset, int bytes_to_read,
                           void *rec, int *bytes_actually_read, int max_lc);
int kf6_open_key     (void *f, char *id, int fcb_lc, int read_only);
int kf6_create_key   (void *f, char *id, int fcb_lc);
int kf6_close_key    (void *f);

#endif
