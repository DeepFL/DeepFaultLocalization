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
// Combiner
//
// 2 June 2004 -- tds
//
// Based on Don Metzler's Combiner class
//

#include "indri/Combiner.hpp"
#include <iostream>
#include <fstream>
#include "indri/HashTable.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Path.hpp"
#include "indri/XMLNode.hpp"
#include "indri/FileTreeIterator.hpp"
#include "indri/IndriTimer.hpp"

#define COMBINER_WRITE_BUFFER_SIZE (1*1024*1024)

indri::parse::Combiner::url_entry* indri::parse::Combiner::_newUrlEntry( const char* url, const char* corpusPath, const char* docNo ) {
  int urlLen = (int)strlen(url) + 1;
  int corpusLen = (int)strlen(corpusPath) + 1;
  int docLen = (int)strlen(docNo) + 1;
  int total = urlLen + corpusLen + docLen;

  char* buffer = (char*) malloc(total + sizeof(url_entry));
  new(buffer) url_entry;
  url_entry* e = (url_entry*) buffer;

  strcpy( buffer + sizeof(url_entry), url );
  strcpy( buffer + sizeof(url_entry) + urlLen, corpusPath );
  strcpy( buffer + sizeof(url_entry) + urlLen + corpusLen, docNo );

  e->url = buffer + sizeof(url_entry);
  e->corpusPath = buffer + sizeof(url_entry) + urlLen;
  e->docNo = buffer + sizeof(url_entry) + urlLen + corpusLen;
  e->linkCount = 0;

  return e;
}

void indri::parse::Combiner::_deleteUrlEntry( void* buffer ) {
  url_entry* e = (url_entry*) buffer;
  e->~url_entry();
  free(buffer);
}

void indri::parse::Combiner::_openWriteBuckets( std::vector<std::stringstream*>& buffers, std::vector<std::ofstream*>& buckets, const std::string& path, int bins ) {
  for( int i=0; i<_bins; i++ ) {
    std::string number = i64_to_string(i);

    std::string binpath = indri::file::Path::combine( path, number );
    std::ofstream* bucket = new std::ofstream;
    bucket->open( binpath.c_str(), std::ios::out | std::ios::app | std::ios::binary);
    buffers.push_back( new std::stringstream() );
    buckets.push_back( bucket );
  }
}

void indri::parse::Combiner::_closeWriteBuckets( std::vector<std::stringstream*>& buffers, std::vector<std::ofstream*>& buckets ) {
  for( size_t i=0; i<buckets.size(); i++ ) {
    _flushWriteBuffer( buffers, buckets, true, (int)i );
    delete buffers[i];
    buckets[i]->close();
    delete buckets[i];
  }
}

void indri::parse::Combiner::_flushWriteBuffers( std::vector<std::stringstream*>& buffers, std::vector<std::ofstream*>& buckets, bool force ) {
  for( size_t i=0; i<buckets.size(); i++ ) {
    _flushWriteBuffer( buffers, buckets, force, (int)i );
  }
}

void indri::parse::Combiner::_flushWriteBuffer( std::vector<std::stringstream*>& buffers, std::vector<std::ofstream*>& buckets, bool force, int i ) {
  std::stringstream* buffer = buffers[i];
  if( buffer->tellp() > COMBINER_WRITE_BUFFER_SIZE || force ) {
    (*buckets[i]) << buffer->str();
    delete buffer;
    buffers[i] = new std::stringstream();
  }
}

void indri::parse::Combiner::_openReadBuckets( std::vector<std::ifstream*>& buckets, const std::string& path, int bins ) {
  for( int i=0; i<_bins; i++ ) {
    std::string number = i64_to_string(i);

    std::string binpath = indri::file::Path::combine( path, number );
    std::ifstream* bucket = new std::ifstream;
    bucket->open( binpath.c_str(), std::ios::in | std::ios::binary);
    buckets.push_back( bucket );
  }
}

void indri::parse::Combiner::_readDocBucket( UrlEntryTable& urlTable, std::ifstream& docIn ) {
  char docno[8192];
  char docurl[65536];
  char corpusPath[8192];

  while( !docIn.eof() ) {
    docIn.getline( docno, sizeof docno-1 );
    if (docIn.eof()) break;
    docIn.getline( docurl, sizeof docurl-1 );
    docIn.getline( corpusPath, sizeof corpusPath-1 );
    
    url_entry* e = _newUrlEntry( docurl + sizeof "DOCURL="-1,
                                 corpusPath + sizeof "CORPUSPATH="-1,
                                 docno + sizeof "DOCNO="-1 );
    urlTable.insert( e->url, e );
  }
  docIn.close();
}

int indri::parse::Combiner::hashString( const char* str ) {
  unsigned int hash = 0;

  for( unsigned int i=0; str[i]; i++ ) {
    hash *= 31;
    hash += str[i];
  }

  return hash % _bins;
}

void indri::parse::Combiner::hashRedirectTargets( const std::string& bucketPath, const std::string& redirectsPath ) {
  std::cout << "hashing redirects" << std::endl;

  // read redirects file, and hash destinations into buckets
  std::vector<std::stringstream*> outputs;
  std::vector<std::ofstream*> outputFiles;
  std::string targetPath = indri::file::Path::combine( bucketPath, "target" );
  lemur_compat::mkdir( targetPath.c_str(), 0755 );
  _openWriteBuckets( outputs, outputFiles, targetPath, _bins );

  // open the redirects file
  std::ifstream in;
  in.open( redirectsPath.c_str(), std::ios::in | std::ios::binary);

  char redirectsLine[65536];

  while( !in.eof() ) {
    in.getline( redirectsLine, sizeof redirectsLine );
    if (in.eof()) break;
    char* space = strchr( redirectsLine, ' ' );

    if( !space )
      continue;

    *space = 0;
    char* source = redirectsLine;
    char* target = space+1;

    int bucket = hashString(target);

    (*outputs[bucket]) << "LINKTO=" << target << std::endl
                       << "ALIAS=" << source << std::endl;

    _flushWriteBuffer( outputs, outputFiles, false, bucket );
  }

  _closeWriteBuckets( outputs, outputFiles );
  in.close();
}

void indri::parse::Combiner::combineRedirectDestinationBucket( const std::string& tmpPath, int i, std::vector<std::stringstream*>& outputs, std::vector<std::ofstream*>& outputFiles ) {
  std::cout << "combining dest bucket " << i << std::endl;

  std::string number = i64_to_string(i);

  // open docs file
  std::ifstream doc;
  std::string docPath = indri::file::Path::combine( tmpPath, "doc" );
  std::string docFilePath = indri::file::Path::combine( docPath, number );
  doc.open( docFilePath.c_str(), std::ios::in | std::ios::binary);

  // read docs file
  UrlEntryTable table;
  _readDocBucket( table, doc );
  doc.close();

  // open targets file
  std::ifstream target;
  std::string targetPath = indri::file::Path::combine( tmpPath, "target" );
  std::string targetFilePath = indri::file::Path::combine( targetPath, number );
  target.open( targetFilePath.c_str(), std::ios::in | std::ios::binary);

  char aliasLine[65536];
  char linktoLine[65536];

  while( !target.eof() ) {
    target.getline( linktoLine, sizeof linktoLine );
    if (target.eof()) break;
    target.getline( aliasLine, sizeof aliasLine );
    char* linkUrl = linktoLine + sizeof "LINKTO=" - 1;
    char* aliasUrl = aliasLine + sizeof "ALIAS=" - 1;

    url_entry** entry = table.find( linkUrl );

    if( entry ) {
      // the target matches something in this bucket
      int bucket = hashString( aliasUrl );
      (*outputs[bucket]) << linktoLine << std::endl
                         << aliasLine << std::endl
                         << "PATH=" << (*entry)->corpusPath << std::endl
                         << "DOCNO=" << (*entry)->docNo << std::endl;
      _flushWriteBuffer( outputs, outputFiles, false, i );
    }
  }
  target.close();

  UrlEntryTable::iterator iter;
  for( iter = table.begin(); iter != table.end(); iter++ ) {
    _deleteUrlEntry( *(iter->second) );
  }
}

void indri::parse::Combiner::combineRedirectDestinationBuckets( const std::string& tmpPath ) {
  std::cout << "combining redirect destination buckets" << std::endl;

  std::vector<std::stringstream*> outputs;
  std::vector<std::ofstream*> outputFiles;
  std::string redirectPath = indri::file::Path::combine( tmpPath, "redirect" );
  lemur_compat::mkdir( redirectPath.c_str(), 0755 );
  _openWriteBuckets( outputs, outputFiles, redirectPath, _bins );

  for( int i=0; i<_bins; i++ ) {
    combineRedirectDestinationBucket( tmpPath, i, outputs, outputFiles );
  }
}

//
// hashToBuckets
//

void indri::parse::Combiner::hashToBuckets( std::ifstream& in, const std::string& path ) {
  char docno[8192];
  char docUrl[4096];
  char linkCountText[256];
  char linkUrl[4096];
  char text[65536];

  while( !in.eof() ) {
    in.getline( docno, sizeof docno );
    if (in.eof()) break;
    in.getline( docUrl, sizeof docUrl );
    in.getline( linkCountText, sizeof linkCountText );

    if( strncmp( docno, "DOC", 3 ) != 0 )
      break;

    int docBucket = hashString( docUrl + sizeof "DOCURL=" - 1 );
    std::stringstream* docFile = _docBuckets[ docBucket ];

    (*docFile) << docno << std::endl
               << docUrl << std::endl
               << "CORPUSPATH=" << path.c_str() << std::endl;

    int linkCount = atoi( linkCountText + sizeof "LINKS=" - 1 );

    for( int i=0; i<linkCount; i++ ) {
      in.getline( linkUrl, sizeof linkUrl );
      in.getline( text, sizeof text );

      int linkBucket = hashString( linkUrl + sizeof "LINKURL=" - 1 );
      std::stringstream* linkFile = _linkBuckets[ linkBucket ];

      (*linkFile) << docno << std::endl
                  << docUrl << std::endl
                  << linkUrl << std::endl
                  << text << std::endl;
    }
  }

  _flushWriteBuffers( _docBuckets, _docBucketFiles, false );
  _flushWriteBuffers( _linkBuckets, _linkBucketFiles, false );

  if( !in.eof() ) {
    std::cerr << "closing file " << path << " but not done reading it" << std::endl;
    std::cerr << "last docno was: " << docno << std::endl;
  }

  in.close();
}

//
// createBuckets
//

void indri::parse::Combiner::createBuckets( const std::string& tmpPath ) {
  std::string docPath = indri::file::Path::combine( tmpPath, "doc" );
  std::string linkPath = indri::file::Path::combine( tmpPath, "link" );

  lemur_compat::mkdir( docPath.c_str(), 0755 );
  lemur_compat::mkdir( linkPath.c_str(), 0755 );

  for( int i=0; i<_bins; i++ ) {
    std::string number = i64_to_string(i);

    std::string docBinPath = indri::file::Path::combine( docPath, number );
    std::ofstream* docBucket = new std::ofstream;
    docBucket->open( docBinPath.c_str(), std::ios::out| std::ios::binary );
    _docBucketFiles.push_back( docBucket );
    _docBuckets.push_back( new std::stringstream() );

    std::string linkBinPath = indri::file::Path::combine( linkPath, number );
    std::ofstream* linkBucket = new std::ofstream;
    linkBucket->open( linkBinPath.c_str(), std::ios::out| std::ios::binary);
    _linkBucketFiles.push_back( linkBucket );
    _linkBuckets.push_back( new std::stringstream() );
  }
}

std::stringstream* combiner_flush_stream( std::stringstream* buffer, std::ofstream* out, bool force ) {
  if( buffer->tellp() > COMBINER_WRITE_BUFFER_SIZE || force ) {
    (*out) << buffer->str();
    delete buffer;
    return new std::stringstream();
  } else {
    return buffer;
  }
}

void indri::parse::Combiner::closeBuckets() {
  for( size_t i=0; i<_docBuckets.size(); i++ ) {
    _docBucketFiles[i]->close();
    delete _docBuckets[i];
    delete _docBucketFiles[i];
  }
  _docBuckets.clear();
  _docBucketFiles.clear();

  for( size_t i=0; i<_linkBuckets.size(); i++ ) {
    _linkBucketFiles[i]->close();
    delete _linkBuckets[i];
    delete _linkBucketFiles[i];
  }
  _linkBuckets.clear();
  _linkBucketFiles.clear();
}

//
// _hashToCorpusTable
//

void indri::parse::Combiner::_hashToCorpusTable( UrlEntryVectorTable& corpusTable, UrlEntryTable& urlTable ) {
  UrlEntryTable::iterator iter;

  for( iter = urlTable.begin(); iter != urlTable.end(); iter++ ) {
    url_entry* entry = (*iter->second);

    if( !entry->linkCount ) {
      _deleteUrlEntry(entry);
      continue;
    }

    std::vector<url_entry*>* entryVec = corpusTable.find( entry->corpusPath );
    if( !entryVec ) {
      entryVec = corpusTable.insert( entry->corpusPath );
    }

    entryVec->push_back(entry);
  }
}

//
// _writeCorpusTable
//

void indri::parse::Combiner::_writeCorpusTable( UrlEntryVectorTable& corpusTable,
                                                const std::string& outputPath )
{
  UrlEntryVectorTable::iterator citer;

  for( citer = corpusTable.begin(); citer != corpusTable.end(); citer++ ) {
    std::vector<url_entry*>* entryVec = citer->second;

    // open the appropriate file
    std::string corpusPath = (*citer->first);
    std::string anchorPath = indri::file::Path::combine( outputPath, corpusPath );
    std::ofstream out;
    out.open( anchorPath.c_str(), std::ios::out | std::ios::app | std::ios::binary);
    out.seekp( 0, std::ios::end );

    if( !out.good() ) {
      // wasn't able to create this file, so maybe we need to build
      // the directory structure
      out.clear();
      std::string parent = indri::file::Path::directory( anchorPath );
      indri::file::Path::make( parent );
      out.open( anchorPath.c_str(), std::ios::out | std::ios::app | std::ios::binary);
      out.seekp( 0, std::ios::end );
    }

    // write to it
    // TODO: note that this may cause disk fragmentation
    for( size_t i=0; i<entryVec->size(); i++ ) {
      url_entry* entry = (*entryVec)[i];
      std::string number = i64_to_string( entry->linkCount );

      out << "DOCURL=" << entry->url << std::endl
          << "DOCNO=" << entry->docNo << std::endl
          << "LINKS=" << number << std::endl
          << entry->linkinfo.front();

      _deleteUrlEntry( entry );
    }

    out.close();
  }
}

//
// _readRedirects
//

void indri::parse::Combiner::_readRedirects( UrlEntryTable& urlTable, const std::string& redirectPath, int number ) {
  std::string redirectBucketPath = indri::file::Path::combine( redirectPath, i64_to_string(number) );
  std::ifstream redirectIn;
  redirectIn.open( redirectBucketPath.c_str(), std::ios::in | std::ios::binary);

  char docurl[65536];
  char aliasurl[65536];
  char pathline[4096];
  char docnoline[256];

  while( !redirectIn.eof() && redirectIn.good() ) {
    redirectIn.getline( docurl, sizeof docurl );
    if (redirectIn.eof()) break;
    redirectIn.getline( aliasurl, sizeof aliasurl );
    redirectIn.getline( pathline, sizeof pathline );
    redirectIn.getline( docnoline, sizeof docnoline );

    if( strcmp( "ALIAS=", aliasurl ) )
      break;

    // look for the aliasurl in the hash table
    url_entry** entry = urlTable.find( aliasurl + sizeof "ALIAS=" - 1 );
    url_entry* new_entry = _newUrlEntry( aliasurl + sizeof "ALIAS=" - 1,
                                         docnoline + sizeof "DOCNO=" - 1,
                                         pathline + sizeof "PATH=" - 1);

    if( entry ) {
      _deleteUrlEntry( *entry );
      urlTable.remove( aliasurl + sizeof "ALIAS=" - 1 );
    }

    urlTable.insert( aliasurl + sizeof "ALIAS=" - 1, new_entry );
  }
  redirectIn.close();
}

//
// _readLinks
//

void indri::parse::Combiner::_readLinks( UrlEntryTable& urlTable, std::ifstream& linkIn ) {
  char docno[65536];
  char linkurl[65536];
  char docurl[65536];
  char linktext[65536];
  int linkCount = 0;

  // read the incoming link information and match it with document information
  while( !linkIn.eof() && linkIn.good() && linkCount < 250000 ) {

    linkIn.getline( docno, sizeof docno-1 );
    if (linkIn.eof() ) break;
    linkIn.getline( docurl, sizeof docurl-1 );
    linkIn.getline( linkurl, sizeof linkurl-1 );
    linkIn.getline( linktext, sizeof linktext-1 );
    
    char* url = linkurl + sizeof "LINKURL=" - 1;
    url_entry** entry = urlTable.find( url );

    // entry exists, so this is a link match
    if( entry ) {
      (*entry)->addLink( docno + sizeof "DOCNO=" - 1,
                         docurl + sizeof "DOCURL=" - 1,
                         linktext + sizeof "TEXT=" - 1 );
    }

    linkCount++;
  }

  if( !linkIn.good() && !linkIn.eof() ) {
    std::cout << "WARNING: link file error: " << docurl << std::endl;
  }
}

//
// combineBucket
//

void indri::parse::Combiner::combineBucket( const std::string& outputPath, const std::string& tmpPath, int bucket ) {
  std::string number = i64_to_string(bucket);

  // open link file
  std::string linkPath = indri::file::Path::combine( tmpPath, "link" );
  std::string linkBucketPath = indri::file::Path::combine( linkPath, number );
  std::ifstream linkIn;

  linkIn.open( linkBucketPath.c_str(), std::ios::in | std::ios::binary);

  do {
    UrlEntryTable urlTable( 5*1024*1024 );

    // read document information into the hash table
    std::string docPath = indri::file::Path::combine( tmpPath, "doc" );
    std::string docBucketPath = indri::file::Path::combine( docPath, number );
    std::ifstream docIn;

    std::cout << "  reading documents" << std::endl;
    docIn.open( docBucketPath.c_str(), std::ios::in | std::ios::binary);
    _readDocBucket( urlTable, docIn );
    docIn.close();

    // update the hash table based on redirect information
    std::string redirectPath = indri::file::Path::combine( tmpPath, "redirect" );

    std::cout << "  reading redirects" << std::endl;
    _readRedirects( urlTable, redirectPath, bucket );

    // read some of the links
    std::cout << "  reading links" << std::endl;
    _readLinks( urlTable, linkIn );

    std::cout << "  hashing to file buckets" << std::endl;
    // hash all the data into file buckets
    UrlEntryVectorTable corpusTable;
    _hashToCorpusTable( corpusTable, urlTable );

    // open each file in turn and write this additional data, then close
    std::cout << "  writing data out to files" << std::endl;
    _writeCorpusTable( corpusTable, outputPath );
  }
  while( !linkIn.eof() );

  linkIn.close();
}

void indri::parse::Combiner::combineBuckets( const std::string& outputPath, const std::string& tmpPath ) {
  indri::utility::IndriTimer t;
  t.start();

  for( int i=0; i<_bins; i++ ) {
    t.printElapsedSeconds(std::cout);
    std::cout << ": combining bucket " << i << std::endl;
    combineBucket(outputPath, tmpPath, i);
  }
}

void indri::parse::Combiner::hashToBuckets( const std::string& bucketPath, const std::string& inputPath ) {
  createBuckets( bucketPath );

  indri::file::FileTreeIterator input( inputPath );

  for( ; input != indri::file::FileTreeIterator::end(); input++ ) {
    std::string path = *input;
    std::ifstream in;

    in.open( path.c_str() );

    // make path relative
    std::string relative = indri::file::Path::relative( inputPath, path );
    std::cout << "hashing " << relative << std::endl;

    hashToBuckets( in, relative );
    in.close();
  }

  _flushWriteBuffers( _docBuckets, _docBucketFiles, true );
  _flushWriteBuffers( _linkBuckets, _linkBucketFiles, true );
  closeBuckets();
}

void indri::parse::Combiner::sortCorpusFiles( const std::string& outputPath, const std::string& preSortPath, const std::string& inputPath ) {
  indri::file::FileTreeIterator files( preSortPath );

  for( ; files != indri::file::FileTreeIterator::end(); files++ ) {
    std::string unsortedPath = *files;
    std::string relativePath = indri::file::Path::relative( preSortPath, unsortedPath );
    std::ifstream in;

    UrlEntryTable urlTable;

    // open an unsorted file
    in.open( unsortedPath.c_str() );
    std::cout << "sorting " << unsortedPath << std::endl;

    int insertCount = 0;
    // read in the sorted file one entry at a time, and
    // store the data in a hash table, keyed on docno
    while( !in.eof() ) {
      char docUrl[4096];
      char docno[8192];
      char linkCountText[8192];

      char linkFrom[4096];
      char linkDocno[4096];
      char linkText[65536];

      in.getline( docUrl, sizeof docUrl );
      if (in.eof()) break;
      in.getline( docno, sizeof docno );
      in.getline( linkCountText, sizeof linkCountText );

      int linkCount = atoi( linkCountText + sizeof "LINKS=" - 1 );
      assert( linkCount > 0 );

      url_entry* e = 0;
      url_entry** spot = 0;

      spot = urlTable.find( docno + sizeof "DOCNO=" - 1 );

      if( spot ) {
        e = *spot;
        if( e->linkCount )
          e->linkinfo.unwrite(1); // remove trailing 0
        e->linkCount += linkCount;
      } else {
        e = _newUrlEntry( docUrl + sizeof "DOCURL=" - 1, "", docno + sizeof "DOCNO=" - 1 );
        e->linkCount = linkCount;
        urlTable.insert( e->docNo, e );
        insertCount++;
      }

      for( int i=0; i<linkCount; i++ ) {
        in.getline( linkDocno, sizeof linkDocno );
        int linkDocnoLen = (int)strlen(linkDocno);

        in.getline( linkFrom, sizeof linkFrom );
        int linkFromLen = (int)strlen(linkFrom);

        in.getline( linkText, sizeof linkText );
        int linkTextLen = (int)strlen(linkText);

        sprintf( e->linkinfo.write(linkDocnoLen+2), "%s\n", linkDocno );
        e->linkinfo.unwrite(1);
        sprintf( e->linkinfo.write(linkFromLen+2), "%s\n", linkFrom );
        e->linkinfo.unwrite(1);
        sprintf( e->linkinfo.write(linkTextLen+2), "%s\n", linkText );
        e->linkinfo.unwrite(1);
      }

      *(e->linkinfo.write(1)) = 0;
    }
    in.close();
    in.clear();

    // Now that the contents of the file are hashed, we
    // open the original anchor text file (which is sorted in the
    // same order as the corpus file it came from).
    // Every time we see a new DOCNO in the file, we'll look for that
    // DOCNO in the hash table, and write it out if we find it.

    std::string inputFilePath;
    inputFilePath = indri::file::Path::combine( inputPath, relativePath );
    in.open( inputFilePath.c_str(), std::ios::in | std::ios::binary);

    std::ofstream out;
    std::string outFilePath;
    outFilePath = indri::file::Path::combine( outputPath, relativePath );

    out.open( outFilePath.c_str(), std::ios::out | std::ios::binary);

    if( !out.good() ) {
      out.clear();
      std::string parent = indri::file::Path::directory( outFilePath );
      indri::file::Path::make( parent );
      out.open( outFilePath.c_str(), std::ios::out | std::ios::binary);
    }

    int totalDocuments = 0;
    int writtenDocuments = 0;

    while( !in.eof() && in.good() ) {
      char docno[65536];
      in.getline( docno, sizeof docno );
      if (in.eof()) break;
      // only interested in DOCNO lines
      if( strncmp( docno, "DOCNO=", sizeof "DOCNO="-1 ) != 0 )
        continue;

      totalDocuments++;
      url_entry** e = urlTable.find( docno + sizeof "DOCNO=" - 1 );

      if( !e )
        continue;

      // found a match, so write out anchor text
      out << docno << std::endl
          << (*e)->url << std::endl
          << "LINKS=" << (*e)->linkCount << std::endl
          << (*e)->linkinfo.front();
      writtenDocuments++;
    }
    out.close();

    UrlEntryTable::iterator iter;

    for( iter = urlTable.begin(); iter != urlTable.end(); iter++ ) {
      url_entry** e = iter->second;
      _deleteUrlEntry(*e);
    }
    urlTable.clear();
  }
}
