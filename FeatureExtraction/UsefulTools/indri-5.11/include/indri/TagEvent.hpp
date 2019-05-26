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
// TagEvent
//
// 15 September 2005 -- mwb
//

#ifndef INDRI_TAGEVENT_HPP
#define INDRI_TAGEVENT_HPP
#include "indri/greedy_vector"
#include "indri/AttributeValuePair.hpp"
namespace indri {
  namespace parse {
    
    struct TagEvent {

      const char *name;

      // True if tag is an open tag, false if it is a close tag
      bool open_tag;

      // These fields refer to the tag's extent in the source text,
      // *not* the extent enclosed by an open tag and its matching
      // close tag.
      int begin;
      int end;

      // Token position of this tag event; for example, if the tag
      // event occured between the 23rd and the 24th tokens extracted
      // from the source document, pos would be 23.
      unsigned int pos;

      indri::utility::greedy_vector<AttributeValuePair,2> attributes;
    };
  }
}

#endif // INDRI_TAGEVENT_HPP
