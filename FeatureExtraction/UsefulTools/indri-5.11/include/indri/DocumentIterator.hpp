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
// DocumentIterator
//
// 11 May 2004 -- tds
//

#ifndef INDRI_DOCUMENTITERATOR_HPP
#define INDRI_DOCUMENTITERATOR_HPP

#include "indri/ObjectHandler.hpp"
#include "indri/UnparsedDocument.hpp"
#include <string>
#include <ctype.h>
namespace indri
{
  namespace parse
  {
    
    class DocumentIterator {
    protected:
      std::string _docnostring;
    public:
      virtual ~DocumentIterator() {};

      virtual UnparsedDocument* nextDocument() = 0;

      virtual void open( const std::string& filename ) = 0;
      virtual void close() = 0;
      // replace spaces with '_' in docno element value.
      void cleanDocno() {
        char *docno = (char *)_docnostring.c_str();
        while ( *docno ) {
          if ( isspace(*docno) )
            *docno = '_';
          docno++;
        }
      }
    };
  }
}

#endif // INDRI_DOCUMENTITERATOR_HPP

