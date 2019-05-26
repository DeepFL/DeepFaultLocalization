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


//
// MetadataPair
//
// 12 May 2004 -- tds
//

#ifndef INDRI_METADATAPAIR_HPP
#define INDRI_METADATAPAIR_HPP

#include <string.h>
#include <ctype.h>
namespace indri
{
  namespace parse
  {
    
    struct MetadataPair {
      const char* key;
      const void* value;
      int valueLength;

      class key_equal {
        const char* k;

      public:
        key_equal( const char* key ) {
          k = key;
        }

        bool operator() ( const MetadataPair& pair ) {
          return strcmp( k, pair.key ) == 0;
        }
      };

      void stripValue() {
        while( isspace( *(char*)value ) ) {
          value = (char*)value + 1;
          valueLength -= 1;
        }

        while( isspace( ((char*)value)[valueLength-2] ) ) {
          valueLength -= 1;
          ((char*)value)[valueLength-1] = 0;
        }
      }
    };
  }
}

#endif // INDRI_METADATAPAIR_HPP
