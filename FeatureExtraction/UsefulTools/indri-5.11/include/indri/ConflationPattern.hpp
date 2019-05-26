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
// ConflationPattern
//
// 15 September 2005 -- mwb
//

// Data structure to support tag-attribute-value conflations at
// parsing time.  To illustrate by example, consider these three
// tags encountered in the source document text:
//
// <TAG ... ATT1="VAL1" ... />
// <TAG ... ATT1="VAL2" ... />
// <TAG ... ATT2="VAL1" ... />
//
// The pattern { "tag", null, null } matches all three.
// The pattern { "tag", "att1", null } matches only the top two.
// The pattern { "tag", "att1", "VAL2" } matches only the middle one.
//
// Note that the pattern { "tag", null, "VAL2" } is not valid.
//
// These patterns are defined in FileClassEnvironmentFactory, and are
// passed to the ParserFactory when the FileClassEnvironment is
// constructed.  The Parser will replace any tags that match the
// pattern with a tag of a specified name that has no attributes.

#ifndef INDRI_CONFLATIONPATTERN_HPP
#define INDRI_CONFLATIONPATTERN_HPP

#include <string.h>
#include <functional>

namespace indri {
  namespace parse {

    // The tag_name and attribute_name strings in the
    // ConflationPattern should always be downcased, but value should
    // appear as it does in the source document.
    
    struct ConflationPattern {
      const char* tag_name;
      const char* attribute_name;
      const char* value;
    };

  }
}

namespace std {

        template <>
        struct less<indri::parse::ConflationPattern *> {
    
    bool operator() ( const indri::parse::ConflationPattern* one, 
                      const indri::parse::ConflationPattern* two ) const {

      // First compare tag_name, then attribute_name, then value.
      // Comparison is lexical ordering according to strcmp.  Recall
      // that tag_name and attribute_name should always be downcased
      // in a ConflationPattern, so this leads to a case-insensitive
      // match.  A value NULL for any entry in the pattern ( which is
      // interpreted as a wildcard ), always comes first.
    
      // { NULL, NULL, NULL } always comes first.
      // { x, NULL, NULL } always comes before { x, y, NULL }
      // { x, y, NULL } always comes before { x, y, z }
    
      // Return true if ConflationPattern one precedes
      // ConflationPattern two; false otherwise.
      
      int r = 0;

      // tag_name
    
      if ( one->tag_name && two->tag_name )
        r = strcmp( one->tag_name, two->tag_name );
      else if ( ! one->tag_name ) return true;
      else if ( ! two->tag_name ) return false;

      if ( r != 0 ) return ( r < 0 );
        
      // attribute_name
    
      if ( one->attribute_name && two->attribute_name )
        r = strcmp( one->attribute_name, two->attribute_name );
      else if ( ! one->attribute_name ) return true;
      else if ( ! two->attribute_name ) return false;
    
      if ( r != 0 ) return ( r < 0 );

      // value
    
      if ( one->value && two->value )
        r = strcmp( one->value, two->value );
      else if ( ! one->value ) return true;
      else if ( ! two->value ) return false;
    
      if ( r != 0 ) return ( r < 0 );

      // If both ConflationPatterns are equal, neither precedes the
      // other.

      return false;

    }
  };
}

#endif // INDRI_CONFLATIONPATTERN_HPP
