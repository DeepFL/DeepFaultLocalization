/*==========================================================================
 * Copyright (c) 2002-2008 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */
//
// PageRank
//
// 17 July 2005 -- dam
//

#ifndef INDRI_PAGERANK_HPP
#define INDRI_PAGERANK_HPP

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "zlib.h"
#include "indri/Buffer.hpp"
#include "indri/UnparsedDocument.hpp"
#include "indri/Parameters.hpp"
#include "indri/FileTreeIterator.hpp"
#include "indri/TaggedDocumentIterator.hpp"
#include "indri/TaggedTextParser.hpp"
#include "indri/Path.hpp"

#include "indri/Repository.hpp"

namespace indri
{
  namespace parse
  {
    class pagerank 
    {
    public:
      std::string doc;
      float val;
      int int_val;
      struct pagerank_greater 
      {
        bool operator() (const pagerank &one, const pagerank &two) 
        {
          if (one.val == two.val) // match previous perl script secondary sort
            return one.doc > two.doc;
          return one.val > two.val;
        }
      };
    };
    class prEntry {
    public:
      lemur::api::DOCID_T doc;
      float val;
      int int_val;
      struct prEntry_greater {
        bool operator() (const prEntry &one, const prEntry &two) {
          if (one.val == two.val)
            return one.doc > two.doc;
          return one.val > two.val;
        }
      };
    };
    
    class PageRank {
    private:
      indri::utility::Buffer _gzbuffer;
      gzFile _in;
      // from Metztler's generate_priors.pl
      static const double _intToProb[11];
      float *prTable;
      
      const std::string _corpusPath;
      const std::string _linkPath;

      double _c; // dampening parameter
      UINT64 _colLen; // collection length

      indri::collection::Repository _repository;
      typedef std::map< std::string, float > PageRankVector;
      typedef std::map< std::string, std::pair< int, std::vector< std::string > > > Links;

      inline void _swap( std::string& a, std::string& b ) {
	std::string tmp = b;
	b = a;
	a = tmp;
      }

      bool _readLine( char* beginLine );
      
      void _computeColLen();
      
      float _readPageRankFromFile( std::ifstream& src, const std::string& sourceDoc );
      void _writePageRankToFile( std::ofstream& src, const std::string& destDoc, const float pr );

      void _computeOutDegrees( Links& links );
      void _doPageRankIter( const int docsPerIter, const std::string& srcFile, const std::string& destFile );
      void _updatePageRank( std::ifstream& src, std::ofstream& dest, Links& links );

      void _raw2int(std::vector<pagerank> &);
      void _ranks2int(std::vector<prEntry> &ranks);
      
      void _loadRanks( const std::string& dest, 
                       std::vector<pagerank> &pageranks);
      
    public:
      PageRank( const std::string& corpusPath, const std::string& linkPath, 
                UINT64 colLen = 0 ) : _corpusPath( corpusPath ), 
                                      _linkPath( linkPath ),
                                      _colLen( colLen ), prTable(0) {
	if (_colLen == 0 ) _computeColLen();
      }
      PageRank( const std::string& corpusPath, const std::string& linkPath, 
                const std::string& indexPath ) : _corpusPath( corpusPath ), 
                                                 _linkPath( linkPath ), prTable(0) {

        _repository.openRead(indexPath);
        indri::collection::Repository::index_state indexes = _repository.indexes();
        _colLen = 0;
        for( int i=0; i<indexes->size(); i++ ) {
          _colLen += (*indexes)[i]->documentCount();
        }
      }
      ~PageRank( ) {
        if( _in )
          gzclose( _in );
        delete(prTable);
      }
      
      void computePageRank( const std::string& outputFile, const int maxIters = 10, const int docsPerIter = 1000, const double c = 0.7 );
      void writeRaw( const std::string& dest, const std::string &fawFile  );
      void writePriors( const std::string& dest, const std::string &priorFile  );
      void writeRanks( const std::string& dest, const std::string &ranksFile  );

      void indexPageRank(const std::string& outputFile, const int maxIters = 100, const double c = 0.85 );
    };
  }
}

#endif
