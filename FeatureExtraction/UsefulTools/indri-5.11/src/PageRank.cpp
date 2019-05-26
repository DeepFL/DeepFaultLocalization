/*==========================================================================
 * Copyright (c) 2002-2008 University of Massachusetts.  All Rights Reserved
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
// Jan 2008 -- dmf
//

#include "indri/PageRank.hpp"
#include "indri/CompressedCollection.hpp"
#include "indri/SequentialReadBuffer.hpp"
#include "indri/SequentialWriteBuffer.hpp"
#include "indri/IndriTimer.hpp"

#include <algorithm>
const double
indri::parse::PageRank::_intToProb[11] = {log(2.21916E-10), 
                                          log(2.21916E-10),
                                          log(5.61952E-10),
                                          log(1.94786E-09),
                                          log(3.62742E-09), 
                                          log(1.00745E-08),
                                          log(1.97616E-08),
                                          log(3.46944E-08),
                                          log(4.39146E-08),
                                          log(1.74226E-07),
                                          log(2.28983E-07) };

// this is somewhat slow, but it gets the job done
// of course, it'd be faster if we had an index built...
void indri::parse::PageRank::_computeColLen() {
  _colLen = 0;
  indri::file::FileTreeIterator files( _corpusPath );
  for( ; files != indri::file::FileTreeIterator::end(); files++ ) {
    std::string filePath = *files;
    indri::parse::TaggedDocumentIterator iterator;
    iterator.open( filePath );
    iterator.setTags( "<DOC>",             // startDocTag
                      "</DOC>",          // endDocTag
                      "</DOCHDR>" );       // endMetadataTag
    indri::parse::UnparsedDocument* unparsed;
    while( (unparsed = iterator.nextDocument()) != 0 )
      _colLen++;
    // close up everything
    iterator.close();
  }
}

float indri::parse::PageRank::_readPageRankFromFile( std::ifstream& src, const std::string& sourceDoc ) {
  int lengthBuf;
  char docBuf[4096];
  float prBuf;

  if( !src.good() )
    return 1.0 / (double)_colLen;

  while( !src.eof() ) {
    src.read( (char *)&lengthBuf, sizeof( int ) ); // get length of document name
    src.read( docBuf, lengthBuf ); // get document name
    src.read( (char *)&prBuf, sizeof( float ) ); // read page rank

    std::string docName = docBuf;

    if( docName > sourceDoc ) {
      int relPos = -( sizeof( int ) + lengthBuf + sizeof( float ) );
      src.seekg( relPos, std::ios::cur );
      return ( 1.0 - _c ) / (double)_colLen;
    }

    if( docName == sourceDoc )
      return prBuf;
  }

  return ( 1.0 - _c ) / (double)_colLen;
}

void indri::parse::PageRank::_writePageRankToFile( std::ofstream& src, const std::string& destDoc, const float pr ) {
  int lengthBuf;

  lengthBuf = destDoc.length();
  src.write( (char *)&lengthBuf, sizeof( int ) ); // document name length
  src.write( destDoc.c_str(), lengthBuf ); // document name
  src.write( (char *)&pr, sizeof( float ) ); // page rank
}

void indri::parse::PageRank::_updatePageRank( std::ifstream& src, std::ofstream& dest, Links& links ) {
  src.seekg( 0, std::ios::beg );

  PageRankVector destPR;

  // update PageRank for each destination in links
  Links::iterator iter;
  for( iter = links.begin(); iter != links.end(); ++iter ) {
    std::string sourceDoc = iter->first;
    int outDegree = iter->second.first;
    std::vector< std::string > destDocs = iter->second.second;

    float srcPR = _readPageRankFromFile( src, sourceDoc );

    std::vector< std::string >::iterator destIter;
    for( destIter = destDocs.begin(); destIter != destDocs.end(); ++destIter )
      destPR[ *destIter ] += srcPR / double( outDegree );
  }

  // dampen PageRanks
  PageRankVector::iterator PRiter;
  for( PRiter = destPR.begin(); PRiter != destPR.end(); ++PRiter ) {
    PRiter->second = _c * PRiter->second + ( 1.0 - _c ) / (double)_colLen;
    _writePageRankToFile( dest, PRiter->first, PRiter->second );
  }
}

void indri::parse::PageRank::_computeOutDegrees( Links& links ) {
  char docno[512];
  char docUrl[4096];
  char linkCountText[256];
  char text[65536];
  char linkDocno[512];
  
  indri::file::FileTreeIterator input( _linkPath );
  Links::iterator iter;

  for( ; input != indri::file::FileTreeIterator::end(); input++ ) {
    std::string path = *input;
    bool result = true;
    
    if( _in )
      gzclose( _in );
    _in = gzopen( path.c_str(), "rb" );

    // fill in the links structure for the documents currently under consideration
    while( result ) {
      result = _readLine( docno );
      result = _readLine( docUrl );
      result = _readLine( linkCountText);

      if( strncmp( docno, "DOC", 3 ) != 0 )
        break;

      int linkCount = atoi( linkCountText + sizeof "LINKS=" - 1 );

      for( int i=0; i<linkCount; i++ ) {
        result = _readLine( linkDocno );
        result = _readLine( text ); // ignore LINKFROM
        result = _readLine( text ); // ignore TEXT

        // see if this is one of our sources
        iter = links.find( linkDocno + 10 );

        // if so, update outdegree count
        if( iter != links.end() ) {
          iter->second.first++;
        }
      }
    }
    if( _in )
      gzclose( _in );
  }
}

bool indri::parse::PageRank::_readLine( char* beginLine ){
  size_t lineLength = 0;
  size_t actual;
  
  // make a buffer of a reasonable size so we're not always allocating
  if( _gzbuffer.size() < 1024*1024 )
    _gzbuffer.grow( 1024*1024 );
  // if we're running out of room, add 25MB
  if( (_gzbuffer.size() -  _gzbuffer.position()) < 512*1024 ) {
    _gzbuffer.grow( _gzbuffer.size() + 1024*1024*25 );
  }
  
  size_t readAmount = _gzbuffer.size() - _gzbuffer.position() - 2;
  
  // fetch next document line
  char* buffer = _gzbuffer.write( readAmount );
  char* result = gzgets( _in, buffer, (int)readAmount );
  
  if(!result) {
    return false;
  }
  
  actual = strlen(buffer);
  lineLength += actual; 
  _gzbuffer.unwrite( readAmount - actual );
  
  // all finished reading
  *_gzbuffer.write(1) = 0;
  strncpy(beginLine, (_gzbuffer.front() + _gzbuffer.position() - lineLength - 1), lineLength);
  
  return true;
  
}

void indri::parse::PageRank::_doPageRankIter( const int docsPerIter, const std::string& srcFile, const std::string& destFile ) {
  char docno[512];
  char docUrl[4096];
  char linkCountText[256];
  char text[65536];
  char linkDocno[512];
  
  int docsAdded = 0;
  Links links;

  std::ifstream src( srcFile.c_str(), std::ios::in | std::ios::binary );
  std::ofstream dest( destFile.c_str(), std::ios::out | std::ios::binary );

  // we need to iterate over the corpus path directories to ensure that we
  // keep things in the correct sorted order
  indri::file::FileTreeIterator input( _corpusPath );

  for( ; input != indri::file::FileTreeIterator::end(); input++ ) {
    std::string filePath = *input;

    std::string relative = indri::file::Path::relative( _corpusPath, filePath );
    std::string linkFile = indri::file::Path::combine( _linkPath, relative );

    if( _in )
      gzclose( _in );
    _in = gzopen( linkFile.c_str(), "rb" );
    bool result = true;
    // grab the doc ids of the source documents we're interested in
    while( result ) {
      result = _readLine( docno);
      result = _readLine( docUrl );
      result = _readLine( linkCountText );

      if( strncmp( docno, "DOC", 3 ) != 0 )
        break;

      docsAdded++;

      int linkCount = atoi( linkCountText + sizeof "LINKS=" - 1 );

      // skip this stuff
      for( int i = 0; i < linkCount; i++ ) {
        result = _readLine( linkDocno );
        result = _readLine( text ); // ignore LINKFROM
        result = _readLine( text ); // ignore TEXT

        links[ linkDocno + 10 ].first = 0;
        links[ linkDocno + 10 ].second.push_back( docno + 6 );
      }

      if( docsAdded == docsPerIter ) {
        _computeOutDegrees( links );
        _updatePageRank( src, dest, links );
        links.clear();
        docsAdded = 0;
      }
    }

    if( _in )
      gzclose( _in );
  }

  // do the remainder of the documents
  if( docsAdded > 0 ) {
    _computeOutDegrees( links );
    _updatePageRank( src, dest, links );
  }

  src.close();
  dest.close();
}

void indri::parse::PageRank::_raw2int(std::vector<pagerank> &ranks)
{
  // Metzler's rawToInt.pl
  // sort the raw scores
  std::sort(ranks.begin(), ranks.end(), pagerank::pagerank_greater());
  int numDocs = ranks.size();
  // max page rank, could be a parameter.
  // if changed, the _intToProb table will need to change.
  int max_pr = 10;
  int rank = max_pr;
  double B = pow((numDocs + 1.0),(1.0/max_pr));
  int baseNum = int(B - 1);
  int num = baseNum;
  for (int i = 0; i < ranks.size(); i++) {
    if (num > 0) {
      ranks[i].int_val = rank;
      num--;
    }
    if (num == 0) {
      baseNum = int(ceil(B*baseNum));
      num = baseNum;
      if (rank > 1) rank--; // don't allow rank 0
    }
  }
}

void indri::parse::PageRank::_loadRanks( const std::string& dest,
                                         std::vector<pagerank> &pageranks) {
  std::ifstream in( dest.c_str(), std::ios::in | std::ios::binary );

  int lengthBuf;
  char docBuf[4096];
  float prBuf;
  // read in the values
  while( in.good() && !in.eof() ) {
    in.read( (char *)&lengthBuf, sizeof( int ) ); // get length of document name
    in.read( docBuf, lengthBuf ); // get document name
    in.read( (char *)&prBuf, sizeof( float ) ); // read page rank
    docBuf[lengthBuf] = '\0';

    // insert into a vector
    indri::parse::pagerank p;
    p.doc = docBuf;
    p.val = prBuf;
    pageranks.push_back(p);
  }
  in.close();
}

void indri::parse::PageRank::writeRaw( const std::string& dest, const std::string &rawFile ) {
  std::ofstream out(rawFile.c_str(), std::ios::out | std::ios::binary);
  out.setf( std::ios_base::fixed );
  out.precision( 32 );
  if (prTable != 0) {
    // we had an index
    lemur::api::DOCID_T docid;
    std::vector<prEntry> pageranks;
    for (docid = 1; docid <= _colLen; docid++) {
      indri::parse::prEntry p;
      p.doc = docid;
      p.val = prTable[docid];
      pageranks.push_back(p);
    }
    _ranks2int(pageranks);
    for (int i = 0; i < pageranks.size(); i++) {
      std::string docno = _repository.collection()->retrieveMetadatum(pageranks[i].doc, "docno");
      out << docno << " " << pageranks[i].val<< std::endl;
    }
  }  else {
    std::vector<pagerank> pageranks;
    _loadRanks(dest, pageranks);
    for (int i = 0; i < pageranks.size(); i++) {
      out << pageranks[i].doc << " " << pageranks[i].val<< std::endl;
    }
  }
}

// merge these two
void indri::parse::PageRank::writeRanks( const std::string& dest, const std::string &ranksFile ) {
  std::ofstream out(ranksFile.c_str(), std::ios::out | std::ios::binary);
  out.setf( std::ios_base::fixed );
  out.precision( 13 );
  if (prTable != 0) {
    // we had an index
    lemur::api::DOCID_T docid;
    std::vector<prEntry> pageranks;
    for (docid = 1; docid <= _colLen; docid++) {
      indri::parse::prEntry p;
      p.doc = docid;
      p.val = prTable[docid];
      pageranks.push_back(p);
    }
    _ranks2int(pageranks);
    for (int i = 0; i < pageranks.size(); i++) {
      std::string docno = _repository.collection()->retrieveMetadatum(pageranks[i].doc, "docno");
      out << docno << " " << pageranks[i].int_val<< std::endl;
    }
  }  else {
    std::vector<pagerank> pageranks;
    _loadRanks(dest, pageranks);
    // bin the values to integer page ranks
    _raw2int(pageranks);
    for (int i = 0; i < pageranks.size(); i++) {
      out << pageranks[i].doc << " " << pageranks[i].int_val<< std::endl;
    }
    out.close();
  }
  
}

void indri::parse::PageRank::writePriors( const std::string& dest, const std::string &priorFile ) {
  std::ofstream out(priorFile.c_str(), std::ios::out | std::ios::binary);
  out.setf( std::ios_base::fixed );
  out.precision( 13 );
  if (prTable != 0) {
    // we had an index
    lemur::api::DOCID_T docid;
    std::vector<prEntry> pageranks;
    for (docid = 1; docid <= _colLen; docid++) {
      indri::parse::prEntry p;
      p.doc = docid;
      p.val = prTable[docid];
      pageranks.push_back(p);
    }
    _ranks2int(pageranks);
    for (int i = 0; i < pageranks.size(); i++) {
      std::string docno = _repository.collection()->retrieveMetadatum(pageranks[i].doc, "docno");
      out << docno << " " << _intToProb[pageranks[i].int_val]<< std::endl;
    }
  }  else {

    std::vector<pagerank> pageranks;
    _loadRanks(dest, pageranks);
    // sort the ranks
    std::sort(pageranks.begin(), pageranks.end(), pagerank::pagerank_greater());
    // bin the values to integer page ranks
    _raw2int(pageranks);
    // assign log probabilities to the bins.
    for (int i = 0; i < pageranks.size(); i++) {
      out << pageranks[i].doc << " " << _intToProb[pageranks[i].int_val]<< std::endl;
    }
  }
  
  out.close();
}

void indri::parse::PageRank::computePageRank( const std::string& outputFile, const int maxIters, const int docsPerIter, const double c ) {
  static const std::string _tmpFile = "PageRank.tmp";
  _c = c;

  std::string src = _tmpFile;
  std::string dest = outputFile;

  // ensure outputFile is always the final destination
  if( maxIters % 2 == 1 )
    _swap( src, dest );

  for( int iter = 0; iter < maxIters; iter++ ) {
    _swap( src, dest ); // alternate between source and destination files
    _doPageRankIter( docsPerIter, src, dest );
  }

  // delete the temp file
  ::remove( _tmpFile.c_str() );
}

void indri::parse::PageRank::_ranks2int(std::vector<prEntry> &ranks)
{
  // Metzler's rawToInt.pl
  // sort the raw scores
  std::sort(ranks.begin(), ranks.end(), prEntry::prEntry_greater());
  int numDocs = ranks.size();
  // max page rank, could be a parameter.
  // if changed, the _intToProb table will need to change.
  int max_pr = 10;
  int rank = max_pr;
  double B = pow((numDocs + 1.0),(1.0/max_pr));
  int baseNum = int(B - 1);
  int num = baseNum;
  for (int i = 0; i < ranks.size(); i++) {
    if (num > 0) {
      ranks[i].int_val = rank;
      num--;
    }
    if (num == 0) {
      baseNum = int(ceil(B*baseNum));
      num = baseNum;
      if (rank > 1) rank--; // don't allow rank 0
    }
  }
}

void indri::parse::PageRank::indexPageRank( const std::string& outputFile, 
                                            const int maxIters, 
                                            const double c ) {
  _c = c;
  double epsilon = 0.000001; // should be a parameter...
  
  // make sure we don't leak...
  delete prTable;
  // prTable |C| * sizeof(float)  
  prTable = new float[_colLen+1];
  float *tmp_prTable = new float[_colLen+1];;
  float *in_prTable_ptr, *out_prTable_ptr;
  // outlinksTable |C| * sizeof(unsigned int)
  unsigned int *outlinksTable = new unsigned int[_colLen+1];
  // ivlIndex |C| * sizeoff(UINT64)
  INT64 * ivlIndex = new INT64[_colLen+1];
  
  // create ivlFile (count, docno_1...docno_count)
  std::string ivlPath = outputFile + ".ivl";
  indri::file::File ivlFile;
  ivlFile.create(ivlPath);
  indri::file::SequentialWriteBuffer *ivlWriteBuffer;
  ivlWriteBuffer = new indri::file::SequentialWriteBuffer(ivlFile, 1024*1024);
  
  // initialize pr (( 1.0 ) / (double)_colLen;, default value)
  // initialize outlinksTable
  // initialize ivlIndex
  float defaultPR = ( 1.0 ) / (double)_colLen;
  for (int i = 1; i <= _colLen; i++) {
    prTable[i] = defaultPR;
    tmp_prTable[i] = defaultPR;
    outlinksTable[i] = 0;
    ivlIndex[i] = -1;
  }

  // read harvestlinks files
  // build ivl
  // update outcount

  char docno[512];
  char docUrl[4096];
  char linkCountText[256];
  char text[65536];
  char linkDocno[512];

  lemur::api::DOCID_T docid;
  std::vector<lemur::api::DOCID_T> docids;
  std::vector<lemur::api::DOCID_T> linkids;

  indri::file::FileTreeIterator input( _corpusPath );
  for( ; input != indri::file::FileTreeIterator::end(); input++ ) {
    std::string filePath = *input;
    std::ifstream in;
    std::string relative = indri::file::Path::relative( _corpusPath, filePath );
    std::string linkFile = indri::file::Path::combine( _linkPath, relative );
    bool result = true;
    if( _in )
      gzclose( _in );
    _in = gzopen( linkFile.c_str(), "rb" );

    // grab the doc ids of the source documents we're interested in
    while( result ) {
      result = _readLine( docno );
      result = _readLine( docUrl );
      result =_readLine( linkCountText );

      if( strncmp( docno, "DOC", 3 ) != 0 )
        break;

      int linkCount = atoi( linkCountText + sizeof "LINKS=" - 1 );
      docids = _repository.collection()->retrieveIDByMetadatum("docno", docno + 6);
      docid = docids[0];
      docids.clear();
      linkids.clear();
      
      for( int i = 0; i < linkCount; i++ ) {
        result = _readLine( linkDocno );
        result = _readLine( text ); // ignore LINKFROM
        result = _readLine( text ); // ignore TEXT
        docids = _repository.collection()->retrieveIDByMetadatum("docno", linkDocno + 10);
        linkids.push_back(docids[0]);
        docids.clear();
      }
      
      // get the ivlOffset
      ivlIndex[docid] = ivlWriteBuffer->tell();
      // stick them in the list and update outcount
      char * spot = ivlWriteBuffer->write(linkids.size() * sizeof(lemur::api::DOCID_T) + sizeof(int));
      memcpy(spot, &linkCount, sizeof(int));
      spot += sizeof(int);
      for (int i = 0; i < linkCount; i++) {
        lemur::api::DOCID_T doc = linkids[i];
        memcpy(spot, &doc, sizeof(lemur::api::DOCID_T));
        spot += sizeof(lemur::api::DOCID_T);
        outlinksTable[doc]++;
      }
    }
    if( _in )
      gzclose( _in );
  }
  ivlWriteBuffer->flush();
  delete ivlWriteBuffer;
  ivlFile.close();
  ivlFile.openRead(ivlPath);
  indri::file::SequentialReadBuffer *ivlReadBuffer;
  ivlReadBuffer = new indri::file::SequentialReadBuffer(ivlFile);
  
  // iterate on pr
  // make final iteration even, so output table is prTable
  int iters = maxIters;
    if ((maxIters % 2) != 0) iters++;
  for (int i = 1; i <= iters; i++) {
    if ((i % 2) == 0) {
      in_prTable_ptr = prTable ; 
      out_prTable_ptr = tmp_prTable ;
    } else {
      in_prTable_ptr = tmp_prTable ; 
      out_prTable_ptr = prTable ;
    }

    for (docid = 1; docid <= _colLen; docid++) {
      INT64 offset = ivlIndex[docid];
      float pr = 0;
      if (offset >= 0) {
        int count;
        ivlReadBuffer->read(&count, offset, sizeof(count));        
        lemur::api::DOCID_T *linkdocs = new lemur::api::DOCID_T[count];
        ivlReadBuffer->read(linkdocs, offset + sizeof(count), 
                            count * sizeof(lemur::api::DOCID_T));
        for (int j = 0; j < count; j++) {
          lemur::api::DOCID_T link = linkdocs[j];
          pr += out_prTable_ptr[link]/outlinksTable[link];
        }
          in_prTable_ptr[docid] = ((1.0 - _c)/(double)_colLen) + _c * pr;
        delete linkdocs;
      }
    }
    // test for convergence
    bool converged = true;
    for (docid = 1; converged && docid <= _colLen; docid++) {
      if (abs(out_prTable_ptr[docid] - in_prTable_ptr[docid]) > epsilon)
        converged = false;
    }
    if (converged) {
      break;
    }
  }

  // clean up
  delete[](tmp_prTable);
  delete outlinksTable;
  delete ivlIndex;
  delete ivlReadBuffer;
  ivlFile.close();
  lemur_compat::remove( ivlPath.c_str() );
}
