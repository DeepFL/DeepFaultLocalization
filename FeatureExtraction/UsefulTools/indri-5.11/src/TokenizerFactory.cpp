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
// TokenizerFactory
//
// 5 August 2005 -- mwb
//

#include "indri/TokenizerFactory.hpp"
#include "lemur/Exception.hpp"


#include "indri/TextTokenizer.hpp"
// Add an #include for your Tokenizer here.


#define TOKENIZER_WORD ("Word")
#define TOKENIZER_WORD_NO_MARKUP ("Word without Markup")
#define TOKENIZER_CHAR ("Char")
#define TOKENIZER_CHAR_NO_MARKUP ("Char without Markup")
// Add a #define for your Tokenizer here.


//
// Directions for adding your own Tokenizer:
//   1. Add a #define at the top of the file (see TOKENIZER_WORD as an example) that gives a normalized
//        name to your Tokenizer.  This is the name that you'd want to show up in a user interface.
//   2. Modify preferredName() to return the normalized name for your Tokenizer.
///       The idea here is to admit as many names as possible (including possibly misspellings)
//        in parameter files, but still keeping a nice name around in case someone wants to
//        print something to the screen.
//   3. Modify get() to return a copy of your Tokenizer.
//        Use the tokenizers map to keep a copy of your Tokenizer around.  The TokenizerFactory destructor
//        will delete any tokenizers you create.
//

std::string indri::parse::TokenizerFactory::preferredName( const std::string& name ) {

  if ( ( name[0] == 'w' || name[0] == 'W' ) &&
       ( name[1] == 'o' || name[1] == 'O' ) &&
       ( name[2] == 'r' || name[2] == 'R' ) &&
       ( name[3] == 'd' || name[3] == 'D' ) ) {

    if ( name[4] == '-' && 
         ( name[5] == 'n' || name[5] == 'N' ) && 
         ( name[5] == 'o' || name[5] == 'O' ) ) {

      // got "word-nomarkup"
      return TOKENIZER_WORD_NO_MARKUP;
    }

    // got "word"
    return TOKENIZER_WORD;

  } else if ( ( name[0] == 'c' || name[0] == 'C' ) &&
              ( name[1] == 'h' || name[1] == 'H' ) &&
              ( name[2] == 'a' || name[2] == 'A' ) &&
              ( name[3] == 'r' || name[3] == 'R' ) ) {

    if ( name[4] == '-' && 
         ( name[5] == 'n' || name[5] == 'N' ) && 
         ( name[5] == 'o' || name[5] == 'O' ) ) {

      // got "char-nomarkup"
      return TOKENIZER_CHAR_NO_MARKUP;
    }

    // got "char"
    return TOKENIZER_CHAR;

  }

  return "";
}


indri::parse::Tokenizer* indri::parse::TokenizerFactory::get( const std::string& name ) {

  indri::parse::Tokenizer* tokenizer;
  std::string preferred = preferredName( name );

  if ( preferred == TOKENIZER_WORD ) {

    tokenizer = new indri::parse::TextTokenizer();

  } else if ( preferred == TOKENIZER_WORD_NO_MARKUP ) {

    tokenizer = new indri::parse::TextTokenizer( false );

  } else if ( preferred == TOKENIZER_CHAR ) {

    tokenizer = new indri::parse::TextTokenizer( true, false );

  } else if ( preferred == TOKENIZER_CHAR_NO_MARKUP ) {

    tokenizer = new indri::parse::TextTokenizer( false, false );

  } else {

    LEMUR_THROW( LEMUR_RUNTIME_ERROR, name + " is not a known tokenizer." );
  }

  return tokenizer;
}
