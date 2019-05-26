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
// StopperTransformation
//
// 13 May 2004 -- tds
//


#include "indri/StopperTransformation.hpp"
#include "lemur/Exception.hpp"
#include <fstream>
#include <iostream>

indri::parse::StopperTransformation::StopperTransformation() :
  _handler(0)
{
}

indri::parse::StopperTransformation::StopperTransformation( const std::vector<char*>& stopwords ) :
  _handler(0)
{
  read( stopwords );
}

indri::parse::StopperTransformation::StopperTransformation( const std::vector<const char*>& stopwords ) :
  _handler(0)
{
  read( stopwords );
}

indri::parse::StopperTransformation::StopperTransformation( const std::vector<std::string>& stopwords ) :
  _handler(0)
{
  read( stopwords );
}

indri::parse::StopperTransformation::StopperTransformation( indri::api::Parameters& stopwords ) :
  _handler(0)
{
  read( stopwords );
}

indri::parse::StopperTransformation::~StopperTransformation() {
}

void indri::parse::StopperTransformation::read( const std::string& filename ) {
  std::ifstream in;

  in.open( filename.c_str(), std::ios::in | std::ios::binary);
  if( !in.good() )
    LEMUR_THROW( LEMUR_IO_ERROR, "Stopper was unable to open the stopwords file named: " + filename );

  while( !in.eof() ) {
    // a little buffer is okay; it doesn't really make sense to
    // have a long stopword anyway
    char buffer[512];
    in.getline( buffer, sizeof buffer );
    
    // skip blank lines
    if( strlen(buffer) )
      _table.insert(strdup(buffer));
  }

  in.close();
}

void indri::parse::StopperTransformation::read( const std::vector<std::string>& stopwords ) {
  for( size_t i=0; i<stopwords.size(); i++ ) {
    _table.insert(strdup(stopwords[i].c_str()));
  }
}

void indri::parse::StopperTransformation::read( const std::vector<const char*>& stopwords ) {
  for( size_t i=0; i<stopwords.size(); i++ ) {
    _table.insert(strdup(stopwords[i]));
  }
}

void indri::parse::StopperTransformation::read( const std::vector<char*>& stopwords ) {
  for( size_t i=0; i<stopwords.size(); i++ ) {
    _table.insert(strdup(stopwords[i]));
  }
}

void indri::parse::StopperTransformation::read( indri::api::Parameters& stopwords ) {
  for( unsigned int i=0; i < stopwords.size(); i++ ) {
    _table.insert(strdup(((std::string) stopwords[i] ).c_str()));
}
}

indri::api::ParsedDocument* indri::parse::StopperTransformation::transform( indri::api::ParsedDocument* document ) {
  indri::utility::greedy_vector<char*>& terms = document->terms;
  for( size_t i=0; i<terms.size(); i++ ) {
    if( terms[i] && (_table.find(terms[i]) != _table.end()) ) {
      terms[i] = 0;
    }
  }

  return document;
}

void indri::parse::StopperTransformation::handle( indri::api::ParsedDocument* document ) {
  _handler->handle( transform(document) );
}

void indri::parse::StopperTransformation::setHandler( ObjectHandler<indri::api::ParsedDocument>& handler ) {
  _handler = &handler;
}
