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
// TaggedDocumentIterator
//
// 14 May 2004 -- tds
//

#ifndef INDRI_TRECDOCUMENTITERATOR_HPP
#define INDRI_TRECDOCUMENTITERATOR_HPP
#include "zlib.h"
#include "indri/DocumentIterator.hpp"
#include "indri/Buffer.hpp"
#include "indri/UnparsedDocument.hpp"
#include <string>
#include <fstream>
namespace indri
{
  namespace parse
  {
    
    class TaggedDocumentIterator : public DocumentIterator {
    private:
      UnparsedDocument _document;
      gzFile _in;
      indri::utility::Buffer _buffer;
      indri::utility::Buffer _metaBuffer;
      std::string _lastMetadataTag;
      std::string _fileName;

      bool _readLine( char*& beginLine, size_t& lineLength );

      const char* _startDocTag;
      const char* _endDocTag;
      const char* _endMetadataTag;

      int _startDocTagLength;
      int _endDocTagLength;
      int _endMetadataTagLength;

    public:
      TaggedDocumentIterator();
      ~TaggedDocumentIterator();

      void setTags( const char* startDoc, const char* endDoc, const char* endMetadata );
  
      void open( const std::string& filename );
      void close();

      UnparsedDocument* nextDocument();
    };
  }
}

#endif // INDRI_TRECDOCUMENTITERATOR_HPP
