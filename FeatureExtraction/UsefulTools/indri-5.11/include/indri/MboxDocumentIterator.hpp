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
// MboxDocumentIterator
//
// 20 May 2005 -- tds
//

#ifndef INDRI_MBOXDOCUMENTITERATOR_HPP
#define INDRI_MBOXDOCUMENTITERATOR_HPP

#include "indri/DocumentIterator.hpp"
#include "indri/UnparsedDocument.hpp"
#include "indri/Buffer.hpp"
#include <fstream>
namespace indri
{
  namespace parse
  {
    class MboxDocumentIterator : public DocumentIterator {
    private:
      std::string _filename;
      UnparsedDocument _document;
      indri::utility::Buffer _buffer;
      indri::utility::Buffer _metaBuffer;
      std::ifstream _in;

      void _copyMetadata( const char* headerLine, int ignoreBytes, const char* tagName );

    public:
      void open( const std::string& filename );
      UnparsedDocument* nextDocument();
      void close();
    };
  }
}

#endif // INDRI_MBOXDOCUMENTITERATOR_HPP

