/*==========================================================================
 * Copyright (c) 2009  University of Massachusetts.  All Rights Reserved.
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

#include "indri/WARCDocumentIterator.hpp"
#include <iostream>
#include "lemur/Exception.hpp"
// helpers
#define SPACES " \t\r\n"
inline std::string trim_right (const string & s, const string & t = SPACES) {
  std::string d (s);
  std::string::size_type i (d.find_last_not_of (t));
  if (i == std::string::npos)
    return std::string("");
  else
    return d.erase (d.find_last_not_of (t) + 1) ;
}

inline std::string trim_left (const string & s, const string & t = SPACES) {
  string d (s);
  return d.erase (0, s.find_first_not_of (t)) ;
}

inline std::string trim (const string & s, const string & t = SPACES) {
  string d (s);
  return trim_left (trim_right (d, t), t) ;
}

inline void split(const std::string &line, std::string &key, 
                  std::string&value, char delim = ':' ) {
  std::string::size_type colon = line.find(delim);
  if (colon != std::string::npos) {
      key = line.substr(0, colon);
      value = line.substr(colon+1);
  }
}


//

const char * indri::parse::WARCRecord::WARCTYPE = "WARC-Type";
const char * indri::parse::WARCRecord::WARCRECORDID = "WARC-Record-ID";
const char * indri::parse::WARCRecord::CONTENTLENGTH = "Content-Length";
const char * indri::parse::WARCRecord::WARCTARGETURI = "WARC-Target-URI";
const char * indri::parse::WARCRecord::WARCTRECID = "WARC-TREC-ID";

std::string indri::parse::WARCRecord::getMetadata(const char *key) {
  std::string *result = metadata.find(key);
  return result ? *result : std::string("");
}

bool indri::parse::WARCRecord::_readLine( char*& beginLine, 
                                          size_t& lineLength ) {
  lineLength = 0;
  size_t actual;
  // make a buffer of a reasonable size so we're not always allocating
  if( _buffer.size() < 1024*1024 )
    _buffer.grow( 1024*1024 );
  // if we're running out of room, add 25MB
  if( (_buffer.size() -  _buffer.position()) < 512*1024 ) {
    _buffer.grow( _buffer.size() + 1024*1024*25 );
  }
  if( _buffer.position() ) {
    // get rid of null terminator from previous call
    _buffer.unwrite(1);
  }

  size_t readAmount = _buffer.size() - _buffer.position() - 2;

  // fetch next document line
  char* buffer = _buffer.write( readAmount );
  char* result;
  result = gzgets( _gzin , buffer, (int)readAmount);
  if(!result) {
    return false;
  }

  actual = strlen(buffer);
  lineLength += actual; 
  _buffer.unwrite( readAmount - actual );

  // all finished reading
  *_buffer.write(1) = 0;
  beginLine = _buffer.front() + _buffer.position() - lineLength - 1;

  return true;
}

bool indri::parse::WARCRecord::readHeader() {
  // skip over empty lines until content-length has been found
  // to accomodate the slightly malformed clueweb warcs.
  // Consume the first WARC record (type warcinfo)
  // should verify the WARC-Type is warcinfo
  // if not, it's a partial file or broken... bleah.
  // read until the WARC-Record-ID line.
  // parse the urn:uri value for the file unique identifier (ugly)
  // read until the Content-Length line.
  // parse the length (atoi)
  // read until an empty line
  // read length bytes into record

  char* beginLine;
  size_t lineLength;  
  std::string line;
  std::string key, value;
  bool result;
  bool empty = false;
  _buffer.clear();
  do {
    result = _readLine( beginLine, lineLength );
    if (! result) break;
    line.assign(beginLine, lineLength);
    // gather the unmolested header string
    header += line;
    // strip the newline (CRLF or LF)
    line = trim_right(line, "\r\n");
    empty = line.size() == 0;
    if (! empty) {
      split(line, key, value);
      value = trim_left(value);
      // classify and add to metadata
      if (key == WARCTYPE) {
        warcType.assign(value);
      } else if (key == WARCRECORDID) {
        // trim '<urn:uuid:' from front, '>' from back
        uuid.assign(value.substr(10, value.size() - 11));
      } else if (key == CONTENTLENGTH) {
        contentLength = atoi(value.c_str());
      } else if (key == WARCTARGETURI) {
        targetURI.assign(value);
      } else if (key == WARCTRECID) {
        trecID.assign(value);
      } else {
        // stuff it in the hash table
        metadata.insert(key, value);
      }
    }
  } while (result && (!contentLength || !empty)) ;
  return contentLength != 0;
}

bool indri::parse::WARCRecord::readContent() {
  _buffer.clear();
  _buffer.grow(contentLength+1);
  content = (char *)_buffer.front();
  int numRead;
  numRead = gzread(_gzin, _buffer.write(contentLength), contentLength);
  if (numRead == -1) 
    {
      int error;
      const char *err = gzerror(_gzin, &error);
      std::cerr << "gzread error: " << err << std::endl;
    }
  
  if (numRead != contentLength) {
    return false;
  }
  // terminate the string
  *_buffer.write(1) = 0;
  // walk the buffer and replace any embedded NUL characters with a space.
  char * tmpContent = (char *)_buffer.front();
  for (int i = 0; i < contentLength; i++)
    if (!tmpContent[i]) { 
      tmpContent[i] = ' '; 
    }
  return true;
}

bool indri::parse::WARCRecord::readRecord() {
  metadata.clear();
  contentLength = 0;
  header = "";
  warcType = "";
  uuid = "";
  targetURI  = "";
  trecID = "";
  bool res = readHeader();
  if (! res) {
    return false;
  }  
  res = readContent();
  return res;
}

indri::parse::WARCRecord::~WARCRecord() {
}

indri::parse::WARCDocumentIterator::WARCDocumentIterator() {
  _gzin = 0;
  _record = 0;
  _metaBuffer.grow (512 * 1024);
  _warcMeta = "warc";
  _dochdr = "dochdr";
  _docnoString = "docno";
}

indri::parse::WARCDocumentIterator::~WARCDocumentIterator() {
  close();
}

void indri::parse::WARCDocumentIterator::open( const std::string& filename ) {
    _gzin = gzopen(filename.c_str(), "rb");
  if( !_gzin)
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open file " + filename + "." );
  _record = new WARCRecord(_gzin, _buffer);
  // Consume the first WARC record (type warcinfo)
  // verify the WARC-Type is warcinfo
  // if not, it's a partial file or broken... bleah.
  bool read = _record->readRecord();
  if ( ! read ) {
    LEMUR_THROW(LEMUR_IO_ERROR, "Bad WARC file." );
  }
  std::string warcType = _record->getWarcType();
  if ( warcType != "warcinfo" ) {
    LEMUR_THROW(LEMUR_IO_ERROR, "Bad WARC file." );
  }
  _warcUUID = _record->getUUID();
  // file pointer is now positioned to read the first response record.
}

void indri::parse::WARCDocumentIterator::close() {
  if (_gzin)
    gzclose(_gzin);
  _gzin = 0;
  delete(_record);
  _record = 0;
}

indri::parse::UnparsedDocument* indri::parse::WARCDocumentIterator::nextDocument() {
  _document.metadata.clear();
  _metaBuffer.clear();

  char* beginLine;
  size_t lineLength;  
  bool result;
  // loop until a response record is found
  // Consume the WARC record, put into WARC metadata
  // verify the WARC-Type is response
  // if not, the record should be skipped
  // get urn:uri value for the record unique identifier
  // combine with the file uuid to make a DOCNO
  // get the WARC-Target-URI to get the url for the front of the DOCHDR.
  std::string warcType = "";
  do {
    result = _record->readRecord();
    if (! result) return 0;
    warcType = _record->getWarcType();
  } while (warcType != "response");
  // current record is a response

  std::string uuid;
  std::string trecDocno = "";
  std::string uri;
  const char *header;
  
  uuid = _record->getUUID();
  trecDocno = _record->getTrecID();
  uri = _record->getTargetURI();
  header = _record->getHeader();
  int headerLen = strlen(header);
  // copy whatever we've read so far into the _metaBuffer.
  memcpy( _metaBuffer.write(headerLen + 1), header, 
          (headerLen + 1) * sizeof(char));

  // make a WARC metadata tag
  indri::parse::MetadataPair warcMetadata;
  warcMetadata.key = _warcMeta;
  warcMetadata.value = _metaBuffer.front();
  warcMetadata.valueLength = _metaBuffer.position();
  _document.metadata.push_back(warcMetadata);

  // add a tag for DOCNO
  if (trecDocno.size() > 0) 
    sprintf(_docno, "%s", trecDocno.c_str());
  else
    sprintf(_docno, "%s-%s", _warcUUID.c_str(), uuid.c_str());
  indri::parse::MetadataPair docnoMetadata;
  docnoMetadata.key = _docnoString;
  docnoMetadata.value = _docno;
  docnoMetadata.valueLength = strlen(_docno) + 1;
  _document.metadata.push_back(docnoMetadata);
  
  // now read the dochdr metadata.
  // make a DOCHDR metadata tag
  indri::parse::MetadataPair dochdrMetadata;
  dochdrMetadata.key = _dochdr;
  dochdrMetadata.value = _metaBuffer.front() + _metaBuffer.position();
  // put in the uri
  // add back the newline that was stripped.
  uri += '\n';
  memcpy( _metaBuffer.write(uri.size()), uri.c_str(), 
          uri.size() * sizeof(char));
  dochdrMetadata.valueLength = uri.size();

  // read they bytes
  const char *startDocument = _record->getContent();
  int contentLen = strlen(startDocument);
  // copy until the two sequential empty lines into metabuffer
  int newlines = 0;
  int numRead = 0;
  header = startDocument;
  for ( numRead = 0; 
        newlines != 2 && (numRead < contentLen );
        numRead++)
    if (header[numRead] == '\n' ) {
      newlines++; 
    } else if (header[numRead] == '\r' && header[numRead+1] == '\n') {
      newlines++; 
      numRead++;
    } else {
      newlines = 0;
    }
  if (newlines != 2 && numRead < contentLen ) {
    // didn't find an http request header.
    // dns requests do not have one (numRead == contentLen)
    return 0;
  }
  if (numRead < contentLen) {
    // found a header
    memcpy( _metaBuffer.write(numRead), header, numRead * sizeof(char) );
    dochdrMetadata.valueLength += numRead;
  } else {
    // dns response has none
    //    numRead = 0;
  }
  
  *_metaBuffer.write(1) = 0; // terminate it.
  dochdrMetadata.valueLength += 1; // add the terminator
  _document.metadata.push_back(dochdrMetadata);

  _document.content =  startDocument + numRead;
  _document.contentLength = contentLen - numRead ;
  _document.text = startDocument;
  _document.textLength = contentLen + 1;
  return &_document;
}
