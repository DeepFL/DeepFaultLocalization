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


#ifndef _STRINGSETH_
#define _STRINGSETH_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

typedef struct {
    int size;       /* the current size of the table */
    int count;      /* number of things currently in the table */
    char ** table;  /* the table itself */
} String_set;

String_set * string_set_create(void);
char *       string_set_add(const char * source_string, String_set * ss);
char *       string_set_lookup(const char * source_string, String_set * ss);
void         string_set_delete(String_set *ss);
void         string_set_display(FILE * fp, String_set *ss);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
