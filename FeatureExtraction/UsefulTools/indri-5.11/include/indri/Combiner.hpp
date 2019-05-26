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
// 3 June 2004 -- tds
//


#ifndef INDRI_COMBINER_HPP
#define INDRI_COMBINER_HPP

#include <iostream>
#include <vector>
#include <string>
#include "indri/Buffer.hpp"
#include "indri/HashTable.hpp"
namespace indri
{
  namespace parse
  {
    
    class Combiner {
    private:
      std::vector< std::ofstream* > _docBucketFiles;
      std::vector< std::ofstream* > _linkBucketFiles;
      std::vector< std::stringstream* > _docBuckets;
      std::vector< std::stringstream* > _linkBuckets;
      int _bins;

      struct strhash {
      public:
        int operator() ( const char* k ) const {
          int hash = 0;
          for( ; *k; k++ ){
            hash *= 7;
            hash += *k;
          }
          return hash;
        }
      };

      struct strcompst {
      public:
        int operator () ( const char* o, const char* t ) const {
          return strcmp( o, t );
        }
      };

      struct url_entry {
        char* url;
        char* corpusPath;
        char* docNo;
        int linkCount;
        indri::utility::Buffer linkinfo;

        void addLink( const char* docno,
                      const char* linkDocUrl,
                      const char* linkText )
        {
          if( linkinfo.position() ) {
            // remove trailing 0
            linkinfo.unwrite(1);
          }

          int docnoLen = (int)strlen(docno);
          int docUrlLen = (int)strlen(linkDocUrl);
          int textLen = (int)strlen(linkText);

          int total = docnoLen + sizeof "LINKDOCNO=" + 
            docUrlLen + sizeof "LINKFROM=" +
            textLen + sizeof "TEXT=" + 1;

          sprintf( linkinfo.write(total),
                   "LINKDOCNO=%s\nLINKFROM=%s\nTEXT=%s\n",
                   docno,
                   linkDocUrl,
                   linkText );

          linkCount++;
        }
      };

      typedef indri::utility::HashTable<char*, url_entry*, strhash, strcompst> UrlEntryTable;
      typedef indri::utility::HashTable<char*, std::vector<url_entry*>, strhash, strcompst> UrlEntryVectorTable;

      url_entry* _newUrlEntry( const char* url, const char* corpusPath, const char* docNo );
      void _deleteUrlEntry( void* buffer );
  
      void _readLinks( UrlEntryTable& urlTable, std::ifstream& linkIn );
      void _readRedirects( UrlEntryTable& urlTable, const std::string& redirectPath, int number );
      void _writeCorpusTable( UrlEntryVectorTable& corpusTable, const std::string& outputPath );
      void _hashToCorpusTable( UrlEntryVectorTable& corpusTable, UrlEntryTable& urlTable );
      
      void _openWriteBuckets( std::vector<std::stringstream*>& buffers, std::vector<std::ofstream*>& buckets, const std::string& path, int bins );
      void _flushWriteBuffer( std::vector<std::stringstream*>& buffers, std::vector<std::ofstream*>& buckets, bool force, int i );
      void _flushWriteBuffers( std::vector<std::stringstream*>& buffers, std::vector<std::ofstream*>& buckets, bool force );
      void _closeWriteBuckets( std::vector<std::stringstream*>& buffers, std::vector<std::ofstream*>& buckets );
      void _openReadBuckets( std::vector<std::ifstream*>& buckets, const std::string& path, int bins );
      void _readDocBucket( UrlEntryTable& urlTable, std::ifstream& docIn );

      int hashString( const char* str );
      void hashToBuckets( std::ifstream& in, const std::string& path );
      void createBuckets( const std::string& tmpPath );
      void closeBuckets();
      void combineBucket( const std::string& outputPath, const std::string& tmpPath, int bucket );
      void hashToBuckets( const std::string& inputPath );
      void combineRedirectDestinationBucket( const std::string& tmpPath, int i, std::vector<std::stringstream*>& outBuffers, std::vector<std::ofstream*>& outputFiles );

    public:
      Combiner( int bins = 10 ) : _bins(bins) {}

      void combineRedirectDestinationBuckets( const std::string& tmpPath );
      void combineBuckets( const std::string& outputPath, const std::string& tmpPath );
      void hashRedirectTargets( const std::string& bucketPath, const std::string& redirectsPath );
      void hashToBuckets( const std::string& bucketPath, const std::string& inputPath );
      void sortCorpusFiles( const std::string& outputPath, const std::string& preSortPath, const std::string& inputPath );
    };
  }
}

#endif // INDRI_COMBINER_HPP

