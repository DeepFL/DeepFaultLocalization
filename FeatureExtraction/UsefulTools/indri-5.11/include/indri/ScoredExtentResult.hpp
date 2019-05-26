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
// ScoredExtentResult
//
// 8 March 2004 -- tds
//

#ifndef INDRI_SCOREDEXTENTRESULT_HPP
#define INDRI_SCOREDEXTENTRESULT_HPP

#include "lemur/lemur-platform.h"
#include "indri/Extent.hpp"
#include "lemur/IndexTypes.hpp"

namespace indri 
{
  namespace api 
  {
    
    struct ScoredExtentResult {
      struct score_greater {
      public:
        bool operator() ( const ScoredExtentResult& one, const ScoredExtentResult& two ) const {
          if( one.score != two.score )
            return one.score > two.score;

          if( one.document != two.document )
            return one.document > two.document;

          if ( one.begin != two.begin )
            return one.begin > two.begin;

          return one.end > two.end;

        }
      };

      ScoredExtentResult() :
        score(0),
        document(0),
        begin(0),
        end(0),
        number(0),
        ordinal(0),
        parentOrdinal(0)
      {
      }

      ScoredExtentResult( UINT64 s, lemur::api::DOCID_T d )
        :
        score( double(s) ),
        document(d),
        begin(0),
        end(0),
        number(0),
        ordinal(0),
        parentOrdinal(0)
      {
      }

      ScoredExtentResult( double s, lemur::api::DOCID_T d )
        :
        score(s),
        document(d),
        begin(0),
        end(0),
        number(0),
        ordinal(0),
        parentOrdinal(0)
      {
      }

      ScoredExtentResult( double s, lemur::api::DOCID_T d, int b, int e )
        :
        score(s),
        document(d),
        begin(b),
        end(e),
        number(0),
        ordinal(0),
        parentOrdinal(0)
      {
      }

      ScoredExtentResult( double s, lemur::api::DOCID_T d, int b, int e, UINT64 n)
        :
        score(s),
        document(d),
        begin(b),
        end(e),
        number(n),
        ordinal(0),
        parentOrdinal(0)
      {
      }

      ScoredExtentResult( double s, lemur::api::DOCID_T d, int b, int e, UINT64 n, int o)
        :
        score(s),
        document(d),
        begin(b),
        end(e),
        number(n),
        ordinal(o),
        parentOrdinal(0)
      {
      }

      ScoredExtentResult( double s, lemur::api::DOCID_T d, int b, int e, UINT64 n, int o, int p)
        :
        score(s),
        document(d),
        begin(b),
        end(e),
        number(n),
        ordinal(o),
        parentOrdinal(p)
      {
      }

      ScoredExtentResult( const ScoredExtentResult& other ) {
        score = other.score;
        document = other.document;
        begin = other.begin;
        end = other.end;
        number = other.number;
        ordinal = other.ordinal;
        parentOrdinal = other.parentOrdinal;
      }

      ScoredExtentResult( const indri::index::Extent &extent) {
        score = extent.weight;
        document = 0;
        begin = extent.begin;
        end = extent.end;
        number = extent.number;
        ordinal = extent.ordinal;
        parentOrdinal = extent.parent;
      }

      bool operator< ( const ScoredExtentResult& other ) const {
        return score > other.score;
      }

      bool operator== ( const ScoredExtentResult& other ) const {
        return ( document == other.document && score == other.score
                 && begin == other.begin && end == other.end 
                 && number == other.number
                 && ordinal==other.ordinal && parentOrdinal==other.parentOrdinal );
      }

      const ScoredExtentResult &operator=(const ScoredExtentResult& other) {
        score = other.score;
        document = other.document;
        begin = other.begin;
        end = other.end;
        number = other.number;
        ordinal = other.ordinal;
        parentOrdinal = other.parentOrdinal;
        return *this;
      }

      double score;
      lemur::api::DOCID_T document;
      int begin;
      int end;
      UINT64 number; /// future annotation "pointer"
      int ordinal;
      int parentOrdinal;
    };
  }
}

#endif // INDRI_SCOREDEXTENTRESULT_HPP

