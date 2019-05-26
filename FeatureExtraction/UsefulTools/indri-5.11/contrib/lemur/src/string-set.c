/*==========================================================================
 * Copyright (c) 2001 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "string-set.h"
#include <string.h>

/* Suppose you have a program that generates strings and keeps pointers to them. 
   The program never needs to change these strings once they're generated.
   If it generates the same string again, then it can reuse the one it
   generated before.  This is what this package supports.

   String_set is the object.  The functions are:

   char * string_set_add(char * source_string, String_set * ss);
   This function returns a pointer to a string with the same 
   contents as the source_string.  If that string is already
   in the table, then it uses that copy, otherwise it generates
   and inserts a new one.

   char * string_set_lookup(char * source_string, String_set * ss);
   This function returns a pointer to a string with the same 
   contents as the source_string.  If that string is not already
   in the table, returns NULL;

   String_set * string_set_create(void);
   Create a new empty String_set.
     
   string_set_delete(String_set *ss);
   Free all the space associated with this string set.

   The implementation uses probed hashing (i.e. not bucket).

*/

int hash_string(const unsigned char *str, String_set *ss) {
  unsigned int accum = 0;
  for (;*str != '\0'; str++) accum = ((256*accum) + (*str)) % (ss->size);
  return accum;
}

int stride_hash_string(const unsigned char *str, String_set *ss) {
  /* This is the stride used, so we have to make sure that its value is not 0 */
  unsigned int accum = 0;
  for (;*str != '\0'; str++) accum = ((17*accum) + (*str)) % (ss->size);
  if (accum == 0) accum = 1;
  return accum;
}

int next_prime_up(int start) {
  /* return the next prime up from start */
  int i;
  start = start | 1; /* make it odd */
  for (;;) {
    for (i=3; (i <= (start/i)); i += 2) {
      if (start % i == 0) break;
    }
    if (start % i == 0) {
      start += 2;
    } else {
      return start;
    }
  }
}

String_set * string_set_create(void) {
  String_set *ss;
  int i;
  ss = (String_set *) malloc(sizeof(String_set));
  ss->size = next_prime_up(100);
  ss->table = (char **) malloc(ss->size * sizeof(char *));
  ss->count = 0;
  for (i=0; i<ss->size; i++) ss->table[i] = NULL;
  return ss;
}

int find_place(const char * str, String_set *ss) {
  /* lookup the given string in the table.  Return a pointer
     to the place it is, or the place where it should be. */
  int h, s, i;
  h = hash_string(str, ss);
  s = stride_hash_string(str, ss);
  for (i=h; 1; i = (i + s)%(ss->size)) {
    if ((ss->table[i] == NULL) || (strcmp(ss->table[i], str) == 0)) return i;
  }
}

void grow_table(String_set *ss) {
  String_set old;
  int i, p;
    
  old = *ss;
  ss->size = next_prime_up(2 * old.size);  /* at least double the size */
  ss->table = (char **) malloc(ss->size * sizeof(char *));
  ss->count = 0;
  for (i=0; i<ss->size; i++) ss->table[i] = NULL;
  for (i=0; i<old.size; i++) {
    if (old.table[i] != NULL) {
      p = find_place(old.table[i], ss);
      ss->table[p] = old.table[i];
      ss->count++;
    }
  }
  /*printf("growing from %d to %d\n", old.size, ss->size);*/
  fflush(stdout);
  free((char *) old.table);
}

char * string_set_add(const char * source_string, String_set * ss) {
  char * str;
  int len, p;
    
  assert(source_string != NULL);

  p = find_place(source_string, ss);
  if (ss->table[p] != NULL) return ss->table[p];
    
  len = strlen(source_string);
  str = (char *) malloc(len+1);
  strcpy(str, source_string);
  ss->table[p] = str;
  ss->count++;
    
  /* We just added it to the table.
     If the table got too big, we grow it.
     Too big is defined as being more than 3/4 full */
  if ((4 * ss->count) > (3 * ss->size)) grow_table(ss);
    
  return str;
}

char * string_set_lookup(const char * source_string, String_set * ss) {
  int p;
    
  p = find_place(source_string, ss);
  return ss->table[p];
}

void string_set_delete(String_set *ss) {
  int i;
    
  if (ss == NULL) return;
  for (i=0; i<ss->size; i++) {
    if (ss->table[i] != NULL) free(ss->table[i]);
  }
  free((char *) ss->table);
  free((char *) ss);
}

void string_set_display(FILE * fp, String_set *ss) {
  int i;
  if (ss == NULL) return;
  for (i=0; i<ss->size; i++) {
    if (ss->table[i] != NULL) {
      fprintf(fp, "  %s\n", ss->table[i]);
    }
  }
}
