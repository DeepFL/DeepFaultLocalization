/*==========================================================================
 * Copyright (c) 2012 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/
#ifndef UINT64COMP_HPP
#define UINT64COMP_HPP

#include "indri/indri-platform.h"

namespace indri {
  namespace parse {

    class Uint64Comp {
        public:
            Uint64Comp();
            static size_t u_strlen(const UINT64 *s);
            static int u_strncmp(const UINT64 *s1, const UINT64 *s2, size_t n);
            static int u_strcmp(const UINT64 *s1, const UINT64 *s2);
            static UINT64 *u_strcpy(UINT64 *s1, const UINT64 *s2);
            static UINT64 *u_strdup(UINT64 *s);
    };
  }
}
#endif // UINT64COMP_H
