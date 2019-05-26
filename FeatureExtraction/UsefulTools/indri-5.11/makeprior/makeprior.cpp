/*==========================================================================
 * Copyright (c) 2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/

//
// makeprior
//
// 22 July 2005 -- tds
//
/*! \page makeprior Indri Named Prior Table Builder
<P>
 This application builds a named prior table for a collection of documents 
and installs it within an Indri Repository.
 Parameter formats for all Indri applications are also described in
<a href="IndriParameters.html">IndriParameters.html</a>
<H3> Prior table construction parameters</h3>
<dl>
  <dt>index</dt>
  <dd> path to an Indri Repository. Specified as
  &lt;index&gt;/path/to/repository&lt;/index&gt; in the parameter file and
  as <tt>-index=/path/to/repository</tt> on the command line.
  </dd>

  <dt>name</dt>
  <dd> The name for this prior. Specified as
  &lt;name&gt;PriorName&lt;/name&gt; in the parameter file and
  as <tt>-name=PriorName</tt> on the command line.
  </dd>

  <dt>input</dt>
  <dd> path to the input file for this prior. The input file should contain
  a list of entries containing first an external document id and second a 
  log probability value. There should be one entry per line of the file.
  Specified as
  &lt;input&gt;/path/to/inputfile&lt;/input&gt; in the parameter file and
  as <tt>-input=/path/to/inputfile</tt> on the command line.
  </dd>
  <dt>memory</dt>
  <dd> an integer value specifying the number of bytes to use for the
  table building process. The value can include a scaling factor by
  adding a suffix. Valid values are (case insensitive) K = 1000, M =
  1000000, G = 1000000000. So 100M would be equivalent to 100000000. The
  value should contain only decimal digits and the optional
  suffix. Specified as &lt;memory&gt;100M&lt;/memory&gt; in the parameter
  file and as <tt>-memory=100M</tt> on the command line. </dd> 

</dl>
*/

#include "indri/QueryEnvironment.hpp"
#include "indri/Parameters.hpp"
#include "indri/File.hpp"
#include "indri/SequentialReadBuffer.hpp"
#include "indri/SequentialWriteBuffer.hpp"
#include "indri/Path.hpp"
#include "indri/ScopedLock.hpp"
#include <queue>
#include <fstream>

void copy_region( indri::file::File& out, indri::file::File& in, UINT64 position, UINT64 length ) {
  char* buffer = new char[1024*1024];
  UINT64 bufLength = 1024*1024;
  UINT64 total = 0;
  
  while( length > total ) {
    UINT64 chunk = lemur_compat::min<UINT64>( bufLength, length - total );

    in.read( buffer, position + total, chunk );
    out.write( buffer, total, chunk );
    
    total += chunk;
  }
  
  delete[] buffer;
}

struct MergeFile {
  indri::file::File* file;
  indri::file::SequentialReadBuffer* buffer;
  UINT64 length;
  UINT32 document;
  double score;
  
  void readScore() {
    buffer->read( &score, sizeof(double) );
  }
  
  void read() {
    buffer->read( &document, sizeof(UINT32) );
    buffer->read( &score, sizeof(double) );
  }
  
  bool finished() const {
    return length == buffer->position();
  }
  
  bool operator< ( const MergeFile& other ) const {
    return document > other.document;
  }
};

void merge_sorted_runs( indri::file::File& out, std::vector<std::string>& inputs, int totalDocuments ) {
  std::priority_queue< MergeFile > files;

  // open files
  for( size_t i=0; i<inputs.size(); i++ ) {
    indri::file::File* file = new indri::file::File;
    file->openRead( inputs[i] );
    indri::file::SequentialReadBuffer* buffer = new indri::file::SequentialReadBuffer( *file, 512*1024 );
    
    MergeFile mf;
    mf.file = file;
    mf.buffer = buffer;
    mf.length = file->size();
    
    mf.read();
    files.push( mf );
  }
  
  indri::file::SequentialWriteBuffer* outb = new indri::file::SequentialWriteBuffer( out, 512*1024 );
  
  UINT32 lastDocument = 0;
  double lowProbability = -1e10;
  
  UINT32 itemCount = totalDocuments;
  UINT32 tableCount = 0;
  
  outb->write( &itemCount, sizeof(UINT32) );
  outb->write( &tableCount, sizeof(UINT32) );
  
  // merge
  while( files.size() ) {
    MergeFile file = files.top();
    
    while( file.document > lastDocument+1 ) {
      outb->write( &lowProbability, sizeof(double) );
      lastDocument++;
    }

    outb->write( &file.score, sizeof(double) );
    lastDocument++;
  
    if( file.finished() ) {
      delete file.buffer;
      file.file->close();
      delete file.file;
      files.pop();
    } else {
      file.read();
      files.pop();
      files.push( file );
    }
    
    itemCount++;
  }

  while( (unsigned int)totalDocuments > lastDocument ) {
    outb->write( &lowProbability, sizeof(double) );
    lastDocument++;
  }
  
  outb->flush();
  delete outb;
}

int sort_comparator( const void* one, const void* two ) {
  UINT32 oneKey = *(UINT32*) one;
  UINT32 twoKey = *(UINT32*) two;

  return oneKey - twoKey;
}

void sort_run( indri::file::File& out, indri::file::File& in, size_t memory ) {
  // read the data in
  UINT64 length = in.size();
  char* data = new char[length];
  in.read( data, 0, length );
  in.close();

  qsort( data, length / 12, 12, sort_comparator );

  out.write( data, 0, length );
  delete[] data;
}

void sort_file( indri::file::File& out, indri::file::File& in, size_t memory, int totalDocuments ) {
  UINT64 length = in.size();
  size_t rounded = (memory / 12) * 12;
    
  UINT64 total = 0;
  std::vector<std::string> temporaries;

  while( length > total ) {
    UINT64 chunk = lemur_compat::min<UINT64>( rounded, length - total );
    indri::file::File tempIn;
    indri::file::File tempOut;
    std::string nameIn;
    std::string nameOut;
  
    tempIn.openTemporary( nameIn );
    tempOut.openTemporary( nameOut );

    // make a sorted run
    copy_region( tempIn, in, total, chunk );
    sort_run( tempOut, tempIn, memory );
    
    tempIn.close();
    tempOut.close();
    lemur_compat::remove( nameIn.c_str() );
    temporaries.push_back( nameOut );
    
    total += chunk;
  }

  in.close();
  merge_sorted_runs( out, temporaries, totalDocuments );
  
  for( size_t i=0; i<temporaries.size(); i++ ) {
    lemur_compat::remove( temporaries[i].c_str() );
  }
}

void convert_intscore_to_long_binary( indri::file::File& outfile, const char* infile ) {
  std::ifstream in;
  in.open( infile );
  
  indri::file::SequentialWriteBuffer* outb = new indri::file::SequentialWriteBuffer( outfile, 1024*1024 );

  while( !in.eof() ) {
    int document;
    double score;

    in >> document
       >> score;
  
    outb->write( (const void*) &document, sizeof(UINT32) );
    outb->write( (const void*) &score, sizeof(double) );
  }
  
  outb->flush();
  delete outb;
  
  outfile.close();
  in.close();
}

void convert_docnoscore_to_binary( indri::file::File& outfile, const std::string& infile, indri::api::QueryEnvironment& env ) {
  std::ifstream in;
  std::string docnoName = "docno";
  
  indri::file::SequentialWriteBuffer* outb = new indri::file::SequentialWriteBuffer( outfile, 1024*1024 );
  in.open( infile.c_str(), std::ifstream::in );
  
  while( !in.eof() ) {
    std::string docno;
    double score;
    
    in >> docno
       >> score;

    if( in.eof() )
      break;
#ifdef VERBOSE_MAKEPRIOR       
    std::cout << "looking up: " << docno << " " << score << std::endl;
#endif       
    std::vector<std::string> docnoValues;
    docnoValues.push_back( docno );
       
    std::vector<lemur::api::DOCID_T> result = env.documentIDsFromMetadata( docnoName, docnoValues );
    
    if( result.size() == 0 ) {
      continue; // allow entries that don't exist and ignore silently.
    }
    
    int document = result[0];
#ifdef VERBOSE_MAKEPRIOR
    std::cout << document << std::endl;
#endif
    outb->write( (const void*) &document, sizeof(UINT32) );
    outb->write( (const void*) &score, sizeof(double) );
  }
  
  outb->flush();
  delete outb;
  in.close();
}

void invert_map( std::map<int, double>& out, const std::map<double, int>& in ) {
  std::map<double, int>::const_iterator iter;
  
  for( iter = in.begin(); iter != in.end(); iter++ ) {
    out[iter->second] = iter->first;
  }
}

void compress_file( indri::file::File& out, indri::file::File& in, const std::map<double, int>& values ) {
  indri::file::SequentialReadBuffer* inb = new indri::file::SequentialReadBuffer( in, 512*1024 );
  indri::file::SequentialWriteBuffer* outb = new indri::file::SequentialWriteBuffer( out, 512*1024 );
  
  UINT32 itemCount = 0;
  UINT32 tableCount = (UINT32)values.size();
  
  inb->read( &itemCount, sizeof(UINT32) );
  inb->read( &tableCount, sizeof(UINT32) );
  
  std::map<int, double> inverted;
  invert_map( inverted, values );
  tableCount = (UINT32)inverted.size();
  
  outb->write( &itemCount, sizeof(UINT32) );
  outb->write( &tableCount, sizeof(UINT32) );
  
  // write table
  for( UINT32 i=0; i<tableCount; i++ ) {
    double value = (double) inverted[i];
    outb->write( &value, sizeof(double) );
  }
  
  // write indexes  
  for( UINT32 i=0; i<itemCount; i++ ) {
    double value;
    inb->read( &value, sizeof(double) );
  
    UINT8 index = (UINT8) values.find(value)->second;
    outb->write( &index, sizeof(UINT8) );
  }
  
  outb->flush();
  delete inb;
  delete outb;
}

bool extract_compression_table( std::map<double, int>& values, indri::file::File& in ) {
  indri::file::SequentialReadBuffer* inb = new indri::file::SequentialReadBuffer( in, 512*1024 );
    
  MergeFile mf;
  mf.file = &in;
  mf.buffer = inb;
  mf.length = in.size();
  mf.score = 0;
  inb->seek( sizeof(UINT32)*2 );

  while( !mf.finished() && values.size() <= 256 ) {
    mf.readScore();
    std::map<double,int>::iterator iter = values.find( mf.score );
    
    if( iter == values.end() ) {
      values.insert( std::make_pair( mf.score, values.size() ) );
    }
  }
  
  delete mf.buffer;

  return values.size() <= 256;
}

void install_prior( const std::string& indexPath, const std::string& priorName, indri::file::File& priorFile ) {
  std::string priorDirectory = indri::file::Path::combine( indexPath, "prior" );
  std::string priorPath = indri::file::Path::combine( priorDirectory, priorName );

  // make sure there's a prior directory in the index
  if( indri::file::Path::exists( priorDirectory ) == false ) {
    indri::file::Path::make( priorDirectory );
  }

  // if there's a old prior there with this name, remove it
  if( indri::file::Path::exists( priorPath ) ) { 
    lemur_compat::remove( priorPath.c_str() );
  }

  // copy the file
  indri::file::File output;
  output.create( priorPath );
  size_t length = priorFile.size();
  
  copy_region( output, priorFile, 0, length );
  output.close(); 
}

int main( int argc, char** argv ) {
  try {
    indri::api::Parameters& param = indri::api::Parameters::instance();
    param.loadCommandLine( argc, argv );

    if( !param.exists("index") || !param.exists("input") || !param.exists("name") ) {
      std::cerr << "makeprior usage: " << std::endl
                << "    makeprior -index=myindex -input=myinputfile -name=priorname" << std::endl
                << "      myindex: a valid Indri index " << std::endl
                << "      myinputfile: a two column text file, where the first column contains docno values" << std::endl
                << "         and the second column contains log probabilities (should be between -infinity and zero)" << std::endl
                << "      name: the name of this prior (as you will reference it in queries, using the #prior(name) syntax)" << std::endl;
      exit(-1);
    }
  
    std::string index = param["index"];

    // get the total document count, including deleted documents.
    indri::collection::Repository* _repository = new indri::collection::Repository();
    _repository->openRead(index);    
    indri::collection::Repository::index_state indexes = _repository->indexes();
    int documentCount = 0;
  
    for( size_t i=0; i<indexes->size(); i++ ) {
      indri::thread::ScopedLock lock( (*indexes)[i]->statisticsLock() );
      documentCount += (int)(*indexes)[i]->documentCount();
    }
    delete _repository;
    
    indri::api::QueryEnvironment env;
    std::cout << "opening index: " << index << std::endl;
    env.addIndex( index );
    
    std::string input = param["input"];
    std::string priorName = param["name"];
    size_t memory = param.get( "memory", 50*1024*1024 );
    
    // step one - convert file from docno/score format to binary format
    indri::file::File unsortedBinary;
    std::string unsortedName;
    
    unsortedBinary.openTemporary( unsortedName );
    std::cout << "converting to binary..." << std::endl;
    convert_docnoscore_to_binary( unsortedBinary, input, env );
    std::cout << "finished" << std::endl;
    
    // step two -- sort the binary version
    indri::file::File uncompressedPrior;
    std::string uncompressedPriorName;
    uncompressedPrior.openTemporary( uncompressedPriorName );
    
    std::cout << "sorting..."<< std::endl;
    sort_file( uncompressedPrior, unsortedBinary, memory, documentCount );
    std::cout << "finished" << std::endl;
    
    unsortedBinary.close();
    lemur_compat::remove( unsortedName.c_str() );
    
    // step three -- check to see if it's compressable, if so, compress it
    std::map<double, int> table;
    indri::file::File compressedPrior;
    
    std::string compressedPriorName;
    compressedPrior.openTemporary( compressedPriorName );
    
    indri::file::File& finalPrior = uncompressedPrior;
    std::cout << "checking for compressability..." << std::endl;
    bool result = extract_compression_table( table, uncompressedPrior );
    
    if( result ) {
      std::cout << "yep" << std::endl;
      // compress the file by using a lookup table
      std::cout << "compressing..." << std::endl;
      compress_file( compressedPrior, uncompressedPrior, table );
      std::cout << std::endl;
      finalPrior = compressedPrior;
    } else {
      std::cout << "nope" << std::endl;
    }
    
    // step four -- install the prior in the index
    std::cout << "installing..." << std::endl;
    install_prior( index, priorName, finalPrior );
    std::cout << "finished" << std::endl;
    
    // clean up
    uncompressedPrior.close();
    compressedPrior.close();
    lemur_compat::remove( uncompressedPriorName.c_str() );
    lemur_compat::remove( compressedPriorName.c_str() );
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  }
  
  return 0;  
}
