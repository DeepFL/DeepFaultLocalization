/*==========================================================================
 * Copyright (c) 2003-2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// OffsetMetadataAnnotator
//
// 3 November 2005 -- jcb
//
// Reads supplied offset metadata file and adds the metadata to
// the parsed document.
//

// Format of the offset metadata file: 3-column, tab-delimited.
// From left-to-right, those columns are:
//
//   docno     : external doc id for document to annotate (string) (e.g. 10)
//
//   key       : the key/name of the metadata element (string) (e.g. origURL)
//
//   value     : the value of the metadata element (string) (e.g. http://bla)

// While the OffsetMetadataAnnotator is transforming the
// ParsedDocument, it will directly operate on the data structures
// just as if it were the Parser, except that it adds metadata from
// its file as opposed to from the original TokenizedDocument text.

#ifndef INDRI_OFFSETMETADATAANNOTATOR_CPP
#define INDRI_OFFSETMETADATAANNOTATOR_CPP

#include "indri/OffsetMetadataAnnotator.hpp"
#include "indri/MetadataPair.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/DirectoryIterator.hpp"
#include "indri/Path.hpp"
#include <fstream>
#include <vector>
#include <string.h>
#include <string>

void indri::parse::OffsetMetadataAnnotator::open( const std::string& offsetMetadataFile ) {
  //Only re-load this data if the new file is *different* from the old
  if ( _offsetMetadataFile.compare( offsetMetadataFile ) == 0 )
    return;

  _offsetMetadataFile = offsetMetadataFile;

  std::cerr << "Loading offset metadata file: " << _offsetMetadataFile << std::endl;

  //If this isn't the first time open was called, cleanup first
  if( !_first_open ) {
    _cleanup();
  }

  //Mark this as opened
  _first_open = false;

  //Check if the file exists
  if( ! indri::file::Path::isFile( _offsetMetadataFile ) ) {
    std::cerr << "No offset metadata file at: " << _offsetMetadataFile << std::endl; 
    return;
  }

  //Load file
  std::string thisFile = _offsetMetadataFile;

  char buf[65536];
  char field[256];

  std::ifstream in;
  in.open( thisFile.c_str() );

  int line = 1;

  //Primary parsing loop:
  while ( in.good() || ! in.eof() ) {

    in.getline( buf, sizeof(buf) - 1 );

    if ( buf[0] == '\0' ) break;

    int fieldStart = 0;
    int fieldCount = 0;
    int fieldOffset = 0;
        
    char* docno = NULL;
    char* key = NULL;
    char* value = NULL;

    int len = 0;

    //Read in the three params
    for( char *c = buf + fieldStart; fieldCount < 3 && fieldOffset < sizeof(field);
           c++, fieldOffset++ ) {
      if( *c == '\t' || *c == '\0' ) { 
        field[fieldOffset] = '\0';
              
        switch( fieldCount ) {
          case 0: //DOCNO (string)
            len = strlen( field );
            docno = new char[len + 1];
            strncpy( docno, field, len);
            docno[len] = '\0';
            _buffers_allocated.push_back( docno );
            break;
          case 1: //KEY (string)
            len = strlen( field );
            key = new char[len + 1];
            strncpy( key, field, len );
            key[len] = '\0';
            //key should be case normalized to lower case.
            for( char *c = key; *c; c++ ) *c = tolower( *c );
            _buffers_allocated.push_back( key );
            break;
          case 2: //VALUE (string)
            len = strlen( field );
            value = new char[len + 1];
            strncpy( value, field, len );
            value[len] = '\0';
            _buffers_allocated.push_back( value );
            break;
        }

        fieldCount++;
        fieldStart += ( fieldOffset + 1 );
        fieldOffset = -1;
      } 
      else {
        field[fieldOffset] = *c;
      }

      if( *c == '\0' ) break;
    }

    //Create the greedy_vector to insert the MetadataPair elements into
    //  unless we already have one for this docno
    indri::utility::greedy_vector<indri::parse::MetadataPair*>** p = 
      _annotations.find( docno );
    indri::utility::greedy_vector<indri::parse::MetadataPair*>* metadata = 
      p ? *p : NULL;
    if( !metadata ) {
      metadata = new indri::utility::greedy_vector<indri::parse::MetadataPair*>;
      _annotations.insert( docno, metadata );
    }

    //Create a MetadataPair for this line, 
    // to enter into the metadata vector
    MetadataPair* pair = new MetadataPair;
    pair->key = key;
    pair->value = value;
    pair->valueLength = strlen(value)+1;

    metadata->push_back( pair );

    line++;

  } //end while

  in.close();

} //end method

indri::api::ParsedDocument* indri::parse::OffsetMetadataAnnotator::transform( indri::api::ParsedDocument* document ) {
  const char *docno = _getDocno( document ); 

  indri::utility::greedy_vector<indri::parse::MetadataPair*>** p =
    _annotations.find( docno );

  indri::utility::greedy_vector<indri::parse::MetadataPair*>* metadata =
    p ? *p : NULL;

  //if we don't have any metadata for this docno, 
  // just return the ParsedDocument passed to us
  if( !metadata || metadata->size()==0 ) return document;

  //add any metadata elements to the document that aren't already there
  for( size_t i=0; i<metadata->size(); i++ ) {
    indri::parse::MetadataPair* newPair = (*metadata)[i];
    const char* newKey = (*metadata)[i]->key;

    bool keyAlreadyExists = false;
    for( size_t j=0; j<document->metadata.size(); j++ ) {
      //indri::parse::MetadataPair existingPair = document->metadata[j];
      const char* existingKey = document->metadata[j].key;
      if( strcmp( newKey, existingKey ) == 0 ) {
        keyAlreadyExists = true;
      }
    }

    if( keyAlreadyExists ) {
      std::cerr << "Metadata element '" << newKey << "' already exists for docno '"
                << docno << "'.  Skipping..." << std::endl;
    }
    else {
      document->metadata.push_back( *newPair );
    }
  }

  return document;
}

#endif // INDRI_OFFSETMETADATAANNOTATOR_CPP


