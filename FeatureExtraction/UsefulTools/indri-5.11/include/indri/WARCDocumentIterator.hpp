/*==========================================================================
 * Copyright (c) 2009 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// WARCDocumentIterator
//
// 03 Mar 2009 -- dmf
//

#ifndef INDRI_WARCDOCUMENTITERATOR_HPP
#define INDRI_WARCDOCUMENTITERATOR_HPP
#include <string>
#include <fstream>
#include "zlib.h"
#include "indri/DocumentIterator.hpp"
#include "indri/Buffer.hpp"
#include "indri/UnparsedDocument.hpp"
#include "indri/HashTable.hpp"

namespace indri
{
  namespace parse
  {
    class WARCRecord {
      private:
      // header fields
      //WARC-TYPE
      std::string warcType;
      //WARC-Record-ID
      std::string uuid;
      // WARC-TREC-ID // clueweb specific
      std::string trecID;
      // WARC-Target-URI
      std::string targetURI;
      //Content-Length
      int contentLength;
      // other metadata headers
      indri::utility::HashTable< std::string, std::string > metadata;
      // the header of the record
      std::string header;
      // the body of the record
      const char *content;
      bool _readLine( char*& beginLine, size_t& lineLength );
      bool readHeader();
      bool readContent();
      gzFile &_gzin;
      indri::utility::Buffer & _buffer;
      public:
      WARCRecord(gzFile &in, indri::utility::Buffer &buf) : _gzin(in), 
                                                            _buffer(buf) { }

      ~WARCRecord();

      std::string getWarcType() { return warcType ; }
      std::string getUUID() { return uuid; }
      std::string getTrecID() { return trecID; }
      std::string getTargetURI() { return targetURI; }
      const char *getHeader() { return header.c_str(); }
      const char *getContent(){ return content; }

      std::string getMetadata(const char *key);

      bool readRecord();
      // header string constants
      static const char * WARCTYPE;
      static const char * WARCRECORDID;
      static const char * CONTENTLENGTH;
      static const char * WARCTARGETURI;
      static const char * WARCTRECID;
    };

    class WARCDocumentIterator : public DocumentIterator {
    private:
      WARCRecord *_record;
      UnparsedDocument _document;
      gzFile _gzin;
      indri::utility::Buffer _buffer;
      indri::utility::Buffer _metaBuffer;
      std::string _warcUUID;
      const char * _warcMeta;
      const char * _dochdr;
      const char * _docnoString;
      char _docno[512];

    public:
      WARCDocumentIterator();
      ~WARCDocumentIterator();
      void open( const std::string& filename );
      void close();
      UnparsedDocument* nextDocument();
    };
  }
}

#endif // INDRI_WARCDOCUMENTITERATOR_HPP
