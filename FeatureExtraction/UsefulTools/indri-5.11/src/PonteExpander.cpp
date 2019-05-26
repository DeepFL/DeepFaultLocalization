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
// PonteExpander
// 
// 18 Aug 2004 -- dam
//

#include "indri/PonteExpander.hpp"
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include "indri/count_iterator"
#include "indri/DirichletTermScoreFunction.hpp"

struct PonteTerm {
  struct relevance_greater {
    bool operator () ( const PonteTerm& one, const PonteTerm& two ) const {
      return one.relevance > two.relevance;
    }
  };

  struct collection_greater {
    bool operator () ( const PonteTerm& one, const PonteTerm& two ) const {
      return one.collection > two.collection;
    }
  };

  struct relevance_projection {
    std::pair< std::string, double > operator () ( const PonteTerm& pterm ) {
      return std::make_pair( pterm.stem, pterm.relevance );
    }
  };

  struct pair_projection {
    PonteTerm operator () ( const std::pair<std::string, PonteTerm>& p ) const {
      return p.second;
    }
  };

  std::string stem;
  double relevance;
  double collection;
};

indri::query::PonteExpander::PonteExpander( indri::api::QueryEnvironment * env , indri::api::Parameters& param ) : indri::query::QueryExpander( env, param ) { }

//
// I'm trying to do something like Ponte expansion here, which ranks terms based on:
//    s(w) = \sum_{D \in R} [P(w|D) / P(w)]
// However, we just end up with terms with no way to pick the "good" ones.
// Therefore, we use weights on the terms based on RM.
//

std::string indri::query::PonteExpander::expand( std::string originalQuery , std::vector<indri::api::ScoredExtentResult>& results ) {
  int fbDocs = _param.get( "fbDocs" , 10 );
  int fbTerms = _param.get( "fbTerms" , 10 );
  double fbOrigWt = _param.get( "fbOrigWeight", 0.5 );
  double mu = _param.get( "fbMu", 0 );

  std::vector<indri::api::DocumentVector*> docVectors = getDocumentVectors( results, fbDocs );
  std::vector<std::string> * rm_vocab = getVocabulary( docVectors );
  size_t vocabSize = rm_vocab->size();
  UINT64 colLen = _env->termCount();

  std::map<std::string, PonteTerm> query_model;

  // initialize all PonteTerm structures
  for( size_t i = 0; i < rm_vocab->size(); i++ ) {
    PonteTerm pterm;

    pterm.stem = (*rm_vocab)[i];
    pterm.relevance = 0;
    pterm.collection = 0;

    query_model[ pterm.stem ] = pterm;
  }
  delete rm_vocab;

  // gather document vectors / statistics for top fbDocs ranked documents
  for( size_t doc = 0; (int)doc < fbDocs && doc < results.size(); doc++ ) {
    indri::api::DocumentVector * docVec = docVectors[ doc ];
    indri::utility::greedy_vector<int> positions = docVec->positions();
    const std::vector<std::string>& stems = docVec->stems();
    indri::utility::count_iterator<int> iter( positions.begin(), positions.end() );
    int docLen = int(positions.size());

    // find probabiliy of each term in the document
    for( ; iter != positions.end(); ++iter ) {
      const std::string& stem = stems[ (*iter).object ];
      UINT64 cf = getCF( stem );

      if( (*iter).count < 2 )
        continue;

      // P(w|D) / P(w) [Ponte]
      double documentProbability = double( (*iter).count ) / double(docLen);
      double collectionProbability = double(cf) / double(colLen);
      double logOdds = log( documentProbability / collectionProbability );

      // s(D) * P(w|D) [Lavrenko]
      indri::query::DirichletTermScoreFunction f( mu, collectionProbability );
      double relevance = exp( results[doc].score ) * exp( f.scoreOccurrence( (*iter).count, docLen ) );

      // update the PonteTerm structure with computed probabilities
      PonteTerm& term = query_model[ stem ];
      term.collection += logOdds;
      term.relevance += relevance;
    }

    delete docVec;
  }

  // shove into a vector and sort terms by Ponte metric
  std::vector<PonteTerm> sortedModel;
  std::transform( query_model.begin(),
                  query_model.end(),
                  std::back_inserter( sortedModel ),
                  PonteTerm::pair_projection() );
  std::sort( sortedModel.begin(), sortedModel.end(), PonteTerm::collection_greater() );

  // copy into a vector with only the relevance weights */
  std::vector< std::pair<std::string, double> > probabilities;
  std::transform( sortedModel.begin(),
                  sortedModel.end(),
                  std::back_inserter( probabilities ),
                  PonteTerm::relevance_projection() );
  

  return buildQuery( originalQuery, fbOrigWt, probabilities, fbTerms );
}

