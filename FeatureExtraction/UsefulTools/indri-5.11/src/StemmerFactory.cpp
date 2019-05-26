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
// StemmerFactory
//
// 5 August 2004 -- tds
//

#include "indri/StemmerFactory.hpp"
#include "indri/PorterStemmerTransformation.hpp"
#include "indri/KrovetzStemmerTransformation.hpp"
#include "indri/ArabicStemmerTransformation.hpp"
#include "lemur/Exception.hpp"

#define STEMMER_PORTER ( "Porter" )
#define STEMMER_KROVETZ ( "Krovetz" )
#define STEMMER_ARABIC ( "Arabic" )

//
// Directions for adding your own stemmer:
//   1. Modify preferredName() to return the normalized name for your stemmer.
///       The idea here is to admit as many names as possible (including possibly misspellings)
//        in parameter files, but still keeping a nice name around in case someone wants to
//        print something to the screen.
//   2. Modify get() to return a copy of your stemmer.
//        Don't keep a pointer to your stemmer around; it is the caller's job to delete the object.
//        You may use the stemmerParams object if you like for additional stemmer options.
//

indri::parse::Transformation* indri::parse::StemmerFactory::get( const std::string& stemmerName, indri::api::Parameters& stemmerParams ) {
  std::string name = preferredName( stemmerName );

  if( name == STEMMER_PORTER ) {
    return new indri::parse::PorterStemmerTransformation();
  } else if( name == STEMMER_KROVETZ ) {
    return new indri::parse::KrovetzStemmerTransformation( stemmerParams );
  } else if( name == STEMMER_ARABIC ) {
    return new indri::parse::ArabicStemmerTransformation( stemmerParams );
  }
  
  LEMUR_THROW( LEMUR_RUNTIME_ERROR, stemmerName + " is not a known stemmer." );
  return 0;
}

std::string indri::parse::StemmerFactory::preferredName( const std::string& name ) {
  if( name[0] == 'k' || name[0] == 'K' ) {
    return STEMMER_KROVETZ;
  } else if( name[0] == 'p' || name[0] == 'P' ) {
    return STEMMER_PORTER;
  } else if( name[0] == 'a' || name[0] == 'A' ) {
    return STEMMER_ARABIC;
  }
  // no match, return whatever was passed in.
  return name;
}
