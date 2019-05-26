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
// TextDocumentExtractor
//
// 16 August 2004 -- tds
//

#ifndef INDRI_TEXTDOCUMENTEXTRACTOR_HPP
#define INDRI_TEXTDOCUMENTEXTRACTOR_HPP
#include "zlib.h"
#include "indri/DocumentIterator.hpp"
#include "indri/UnparsedDocument.hpp"
#include "indri/Buffer.hpp"
#include <fstream>
namespace indri
{
  namespace parse
  {
    
    class TextDocumentExtractor : public DocumentIterator {
    private:
      std::string _filename;
      UnparsedDocument _document;
      indri::utility::Buffer _buffer;
      gzFile _in;

    public:
      void open( const std::string& filename );
      UnparsedDocument* nextDocument();
      void close();
    };
  }
}

#endif // INDRI_TEXTDOCUMENTEXTRACTOR_HPP
