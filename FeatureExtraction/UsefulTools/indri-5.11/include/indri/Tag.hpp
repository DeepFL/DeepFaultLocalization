/*==========================================================================
 * Copyright (c) 2003-2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

#include <stdlib.h>
#include <string.h>

#ifndef _TAG_HPP
#define _TAG_HPP

#define MAX_TAG_LENGTH 1024
namespace indri
{
  namespace parse
  {
    
    class Tag {
    public:
      Tag() {
        next = NULL;
        prev = NULL;
        begin = -1;
        end = -1;
      }

      Tag(char *n, int b) {
        next = NULL;
        prev = NULL;
        strncpy(name, n, MAX_TAG_LENGTH);
        begin = b;
        end = -1;
      }

      ~Tag() { }  
  
      void set_next(Tag *t) { next = t; t->set_prev(this);}
      void set_prev(Tag *t) { prev = t; }
      void set_end(int e) { end = e; }
      Tag * get_next() { return next; }
      Tag * get_prev() { return prev; }
      char * get_name() { return name; }
      int get_begin() { return begin; }
      int get_end() { return end; }
    private:
      char name[MAX_TAG_LENGTH];
      Tag *next;
      Tag *prev;
      int begin;
      int end;
    };
  }
}

#endif
