/*==========================================================================
 * Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// TagExtent
//
// 4 February 2004 -- tds
//

#ifndef INDRI_TAGEXTENT_HPP
#define INDRI_TAGEXTENT_HPP

#include "indri/AttributeValuePair.hpp"
#include <string.h>

namespace indri
{
  namespace parse
  {
    
    struct TagExtent {

      // A comparator that sorts by end value, lowest first
      struct lowest_end_first {

        bool operator() ( const indri::parse::TagExtent* x,
                          const indri::parse::TagExtent* y ) const {

          // returns true if x < y; false otherwise

          if ( x->end > y->end ) return true;
          else if ( x->end == y->end ) return ( x < y ); 
          else return false;
        }
      };

      const char* name;
      int begin;
      int end;
      INT64 number;
      TagExtent *parent;
      // explicit initial count of two elements.
      indri::utility::greedy_vector<AttributeValuePair, 2> attributes;
    };
  

    class LessTagExtent {
    public:
      bool operator()(indri::parse::TagExtent * extent1, indri::parse::TagExtent * extent2 ) {
        if ( extent1->begin < extent2->begin )
          return true;
        if ( extent1->begin == extent2->begin  
             && extent1->end > extent2->end )
          return true;
        if ( extent1->begin == extent2->begin 
             &&  extent1->end == extent2->end ) {
          return (extent1 < extent2);
        }
                                                  
        return false;   
      }
    };

  }
}

#include <functional>
namespace std {

  // An STL comparator that implements first-and-longest ordering
  template<>
  struct less<indri::parse::TagExtent*> {

    bool operator() ( const indri::parse::TagExtent* x,
                      const indri::parse::TagExtent* y ) const {

      // returns true if x < y; false otherwise

      if ( x->begin < y->begin ) return true;
      else if ( x->begin > y->begin ) return false;
      else {

        if ( ( x->end - x->begin ) > ( y->end - y->begin ) ) return true;
        else if ( ( x->end - x->begin ) < ( y->end - y->begin ) ) return false;
        else {
          // We might have two extents with the same names at the same locations
          // as a result of offset annotations that actually have different children etc.
          return (x < y);

          // Two TagExtents must have same begin and end and name to be
          // considered equal.

          if ( strcmp( x->name, y->name ) < 0 ) return true;
          else return false;
        }       
      }
    }
  };
}

#endif // INDRI_TAGEXTENT_HPP
