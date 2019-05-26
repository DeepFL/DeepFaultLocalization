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
// Extent
//
// 11 February 2004 -- tds
//

#ifndef INDRI_EXTENT_HPP
#define INDRI_EXTENT_HPP
#include "lemur/lemur-platform.h"
namespace indri
{
  namespace index
  {
    
    struct Extent {
      double weight;
      int begin;
      int end;
      int ordinal;
      int parent;
      UINT64 number;

      Extent() : weight(1), begin(-1), end(-1), ordinal(0), parent(-1), number(0) {}
      Extent( int b, int e ) : weight(1), begin(b), end(e), ordinal(0), parent(-1), number(0) {}
      Extent( int b, int e, int o ) : weight(1), begin(b), end(e), ordinal(o), parent(-1), number(0) {}
      Extent( int b, int e, int o, int p ) : weight(1), begin(b), end(e), ordinal(o), parent(p), number(0) {}
      Extent( int b, int e, int o, int p, UINT64 n ) : weight(1), begin(b), end(e), ordinal(o), parent(p), number(n) {}
      Extent( double w, int b, int e ) : weight(w), begin(b), end(e), ordinal(0), parent(-1), number(0) {}
      Extent( double w, int b, int e, int o) : weight(w), begin(b), end(e), ordinal(o), parent(-1), number(0) {}
      Extent( double w, int b, int e, int o, int p) : weight(w), begin(b), end(e), ordinal(o), parent(p), number(0) {}
      Extent( double w, int b, int e, int o, int p, UINT64 n) : weight(w), begin(b), end(e), ordinal(o), parent(p), number(n) {}

      Extent(const Extent &e) {
        weight = e.weight;
        begin = e.begin;
        end = e.end;
        ordinal = e.ordinal;
        parent = e.parent;
        number = e.number;
      }
    
      bool contains( const Extent& other ) const {
        return begin <= other.begin && end >= other.end;
      }

      bool before( const Extent& other ) const {
        return end <= other.begin;
      }

      bool beginsBefore( const Extent& other ) const {
        return begin < other.begin;
      }

      struct begins_before_less {
        bool operator() ( const Extent& one, const Extent& two ) const {
          return one.beginsBefore( two );
        }
      };
      
      struct ends_before_less {
        bool operator() ( const Extent& one, const Extent& two ) const {
          if ( one.end < two.end ) {
            return true;
          } else if ( one.end == two.end ) {
            if ( one.begin < two.begin ) {
              return true;
            } else if ( one.begin == two.begin ) {
              if (one.ordinal < two.ordinal ) {
                return true;
              } else if ( one.ordinal == two.ordinal ) {
                if ( one.weight < two.weight ) {
                  return true;
                }
              } 
            }
          }
          return false;
        }       
      };

      struct ends_before_greater {
        bool operator() ( const Extent& one, const Extent& two ) const {
          if ( one.end > two.end ) {
            return true;
          } else if ( one.end == two.end ) {
            if ( one.begin < two.begin ) {
              return true;
            } else if ( one.begin == two.begin ) {
              if (one.ordinal < two.ordinal ) {
                return true;
              } else if ( one.ordinal == two.ordinal ) {
                if ( one.weight < two.weight ) {
                  return true;
                }
              } 
            }
          }
          return false;
        }       
      };

      struct begins_before_ends_before_less {
        bool operator() ( const Extent& one, const Extent& two ) const {
          if ( one.beginsBefore( two ) )
            return true;
          else if ( one.begin == two.begin && one.end < two.end ) 
            return true;
          else if ( one.begin == two.begin && one.end == two.end && one.ordinal < two.ordinal )
            return true;
          
          return false;
        }
      };
    };
  }
}

#endif // INDRI_EXTENT_HPP
