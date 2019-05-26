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
#include "indri/uint64comp.hpp"
#include <cstdlib>

// All "string" functions adapted from:
// http://en.wikibooks.org/wiki/C_Programming/Strings

namespace indri
{
    namespace parse
    {

        Uint64Comp::Uint64Comp()
        {

        }

        /* u_strlen */
        size_t Uint64Comp::u_strlen(const UINT64 *s)
        {
            const UINT64 *p = s;
            /* Loop over the data in s.  */
            while (*p != '\0')
                p++;
            return (size_t)(p - s);
        }

        /* u_strcpy */
        UINT64 *Uint64Comp::u_strcpy(UINT64 *s1, const UINT64 *s2)
        {
            UINT64 *dst = s1;
            const UINT64 *src = s2;
            /* Do the copying in a loop.  */
            while ((*dst++ = *src++) != '\0')
                ;               /* The body of this loop is left empty. */
            /* Return the destination string.  */
            return s1;
        }

        UINT64 *Uint64Comp::u_strdup(UINT64 *s) {
            int length = u_strlen(s) + 1;
            UINT64 *result = (UINT64*)malloc(length*sizeof(UINT64));
            if (result == (UINT64*)0){return (UINT64*)0;}
            u_strcpy(result, s);
            return result;
        }

        /* u_strcmp */   // Note the unsigned conversion doesn't actually do anything here - UINT64 is unsigned already
        int Uint64Comp::u_strcmp(const UINT64 *s1, const UINT64 *s2)
        {
            UINT64 uc1, uc2;
            /* Move s1 and s2 to the first differing characters
               in each string, or the ends of the strings if they
               are identical.  */
            while (*s1 != '\0' && *s1 == *s2) {
                s1++;
                s2++;
            }
            /* Compare the characters as unsigned char and
               return the difference.  */
            uc1 = (*s1);
            uc2 = (*s2);
            return ((uc1 < uc2) ? -1 : (uc1 > uc2));
        }

        /* u_strncmp */   // Note the unsigned conversion doesn't actually do anything here - UINT64 is unsigned already
        int Uint64Comp::u_strncmp(const UINT64 *s1, const UINT64 *s2, size_t n)
        {
            UINT64 uc1, uc2;
            /* Nothing to compare?  Return zero.  */
            if (n == 0)
                return 0;
            /* Loop, comparing bytes.  */
            while (n-- > 0 && *s1 == *s2) {
                /* If we've run out of bytes or hit a null, return zero
                   since we already know *s1 == *s2.  */
                if (n == 0 || *s1 == '\0')
                    return 0;
                s1++;
                s2++;
            }
            uc1 = (*s1);
            uc2 = (*s2);
            return ((uc1 < uc2) ? -1 : (uc1 > uc2));
        }
    }
}
