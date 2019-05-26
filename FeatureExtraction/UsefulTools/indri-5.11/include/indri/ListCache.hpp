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
// ListCache
//
// 24 September 2004 -- tds
//
// Stores previously used precomputed lists.
//

#ifndef INDRI_LISTCACHE_HPP
#define INDRI_LISTCACHE_HPP

#include <vector>
#include "indri/delete_range.hpp"
#include "indri/SimpleCopier.hpp"
#include "indri/DocumentCount.hpp"
#include "indri/QuerySpec.hpp"
#include "indri/TreePrinterWalker.hpp"
#include "indri/Parameters.hpp"
namespace indri
{
  namespace lang
  {
    
    class ListCache {
    public:
      struct CachedList {
        // query structure
        indri::lang::SimpleCopier raw;
        indri::lang::SimpleCopier context;

        // postings
        indri::utility::greedy_vector<indri::index::DocumentContextCount> entries;

        // statistics about the entries
        INT64 occurrences;
        INT64 contextSize;
        INT64 minimumContextSize;
        INT64 maximumContextSize;
        INT64 maximumOccurrences;
        float maximumContextFraction;
      };

    private:
      std::vector<struct CachedList*> _lists;

    public:
      ~ListCache() {
        indri::utility::delete_vector_contents( _lists );
      }

      void add( CachedList* list ) {
        if( _lists.size() > 100 ) {
          delete _lists[0];
          _lists.erase( _lists.begin() );
        }

        _lists.push_back( list );
      }

      CachedList* find( indri::lang::Node* raw, indri::lang::Node* context ) {
        ListCache::CachedList* list = 0;
        size_t i = 0;

        // TODO: use a hash function to make this faster
        for( i=0; i<_lists.size(); i++ ) {
          indri::lang::Node* cachedRaw = _lists[i]->raw.root();
          indri::lang::Node* cachedContext = _lists[i]->context.root();

          if( *cachedRaw == *raw ) {
            if( ( !cachedContext && !context ) ||
                ( cachedContext && context && (*context == *cachedContext)) ) {
              list = _lists[i];
              break;
            } 
          }
        }

        return list;
      }
    };
  }
}

#endif // INDRI_LISTCACHE_HPP


