
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
// RelevanceModel
//
// 23 June 2005 -- tds
//


#include <string>
#include <vector>
#include "indri/TermFieldStatistics.hpp"
#include "indri/TermScoreFunction.hpp"
#include "indri/TermScoreFunctionFactory.hpp"
#include "indri/HashTable.hpp"
#include "indri/greedy_vector"
#include "indri/QueryEnvironment.hpp"

namespace indri {
  namespace query {
    class RelevanceModel {
    public:
      struct Gram {
        std::vector<std::string> terms;
        double weight;

        struct hash {
          int operator() ( const Gram* one ) const {
            indri::utility::GenericHash<const char*> h;
            int accumulator = 0;

            for( size_t i=0; i<one->terms.size(); i++ ) {
              accumulator *= 7;
              accumulator += h( one->terms[i].c_str() );
            }

            return accumulator;
          }
        };

        struct weight_greater {
          bool operator() ( const Gram* o, const Gram* t ) const {
            return t->weight < o->weight;
          }
        };

        struct string_comparator {
          int operator() ( const Gram* o, const Gram* t ) const {
            const Gram& one = *o;
            const Gram& two = *t;

            if( one.terms.size() != two.terms.size() ) {
              if( one.terms.size() < two.terms.size() ) {
                return 1;
              } else {
                return -1;
              }
            }

            for( size_t i=0; i<one.terms.size(); i++ ) {
              const std::string& oneString = one.terms[i];
              const std::string& twoString = two.terms[i];

              if( oneString != twoString ) {
                if( oneString < twoString )
                  return -1;
                else
                  return 1;
              }
            }

            return 0;
          }
        };
      };

    private:
      struct GramCounts {
        Gram gram;
        indri::utility::greedy_vector< std::pair< int, int > > counts;
      };

      indri::api::QueryEnvironment& _environment;
      int _maxGrams;
      std::string _smoothing;
      int _documents;

      typedef indri::utility::HashTable< Gram*, GramCounts*, Gram::hash, Gram::string_comparator > HGram;
      HGram _gramTable;

      std::vector<indri::api::ScoredExtentResult> _results;
      std::vector<lemur::api::DOCID_T> _documentIDs;
      std::vector<Gram*> _grams;
      std::vector<indri::api::DocumentVector*> _vectors;

      void _countGrams();
      void _scoreGrams();
      void _sortGrams();
      void _extractDocuments();

    public:
      RelevanceModel( indri::api::QueryEnvironment& environment,
                      const std::string& smoothing,\
                      int maxGrams,
                      int documents );
      ~RelevanceModel();

      void generate( const std::string& query );
      // generate from an existing result set
      void generate( const std::string &query , const std::vector<indri::api::ScoredExtentResult>& results );
      const std::vector<indri::api::ScoredExtentResult>& getQueryResults() const;
      const std::vector<Gram*>& getGrams() const;
    };
  }
}
