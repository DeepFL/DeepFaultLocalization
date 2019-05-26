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
// QueryExpander
//
// 18 Aug 2004 -- dam
//

#ifndef INDRI_QUERYEXPANDER_HPP
#define INDRI_QUERYEXPANDER_HPP

#include <string>
#include <vector>
#include <map>

#include "indri/QueryEnvironment.hpp"
#include "indri/Parameters.hpp"
namespace indri
{
  namespace query
  {
    
    struct QueryExpanderSort {
    public:
      bool operator() ( const std::pair<std::string, double>& one, const std::pair<std::string, double>& two ) const {
        return one.second > two.second;
      }
    };

    class QueryExpander {
    private:
      std::map<std::string, UINT64> _cf_cache;
  
    protected:
      indri::api::QueryEnvironment * _env;
      indri::api::Parameters _param;

      std::map< std::string, bool > _stopwords;

      std::vector<indri::api::DocumentVector*> getDocumentVectors( std::vector<indri::api::ScoredExtentResult>& results, int rmDocs );
      std::vector<std::string> * getVocabulary( std::vector<indri::api::ScoredExtentResult>& results, int rmDocs );
      std::vector<std::string> * getVocabulary( std::vector<indri::api::DocumentVector*>& docVectors );
      std::string buildQuery( const std::string& originalQuery, double originalWeight,
                              const std::vector< std::pair<std::string, double> >& expansionTerms,
                              int termCount );
      UINT64 getCF( const std::string& term );
  
    public:
      QueryExpander( indri::api::QueryEnvironment * env , indri::api::Parameters& param );
      virtual ~QueryExpander() {};

      // runs original query, expands query based on results ( via expand( .. ) ), then runs expanded query
      std::vector<indri::api::ScoredExtentResult> runExpandedQuery( std::string originalQuery , int resultsRequested , bool verbose = false );
  
      // creates expanded query from an original query and a ranked list of documents
      virtual std::string expand( std::string originalQuery , std::vector<indri::api::ScoredExtentResult>& results ) = 0;
    };
  }
}


#endif
