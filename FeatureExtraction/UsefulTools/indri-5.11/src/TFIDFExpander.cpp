/*==========================================================================
 * Copyright (c) 2009 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

//
// TFIDFExpander
// 
// 10 Aug 2009 -- dmf
//

#include "indri/TFIDFExpander.hpp"
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include "indri/count_iterator"
// Cloned from TermScoreFunctionFactory, it should be exported from there.
static void _parseSmoothing( indri::api::Parameters& converted, 
                             const std::string& spec ) {
  int nextComma = 0;
  int nextColon = 0;
  int  location = 0;

  for( location = 0; location < spec.length(); ) {
    nextComma = spec.find( ',', location );
    nextColon = spec.find( ':', location );

    std::string key = spec.substr( location, nextColon-location );
    std::string value = spec.substr( nextColon+1, nextComma-nextColon-1 );

    converted.set( key, value );

    if( nextComma > 0 )
      location = nextComma+1;
    else
      location = spec.size();
  }
}


struct TFIDFTerm {
  struct relevance_greater {
    bool operator () ( const TFIDFTerm& one, const TFIDFTerm& two ) const {
      return one.relevance > two.relevance;
    }
  };


  struct relevance_projection {
    std::pair< std::string, double > operator () ( const TFIDFTerm& pterm ) {
      return std::make_pair( pterm.stem, pterm.relevance );
    }
  };

  struct pair_projection {
    TFIDFTerm operator () ( const std::pair<std::string, TFIDFTerm>& p ) const {
      return p.second;
    }
  };

  std::string stem;
  double relevance;
};

indri::query::TFIDFExpander::TFIDFExpander( indri::api::QueryEnvironment * env , indri::api::Parameters& param ) : indri::query::QueryExpander( env, param ) { }


static double _BM25TF(const double rawTF, const double k1, const double b, 
                      const double docLen, const  double avgDocLen) {
  double x= rawTF+k1*(1-b+b*docLen/avgDocLen);
  return (k1*rawTF/x);
}


std::string indri::query::TFIDFExpander::expand( std::string originalQuery , std::vector<indri::api::ScoredExtentResult>& results ) {
  int fbDocs = _param.get( "fbDocs" , 10 );
  int fbTerms = _param.get( "fbTerms" , 10 );
  double fbOrigWt = _param.get( "fbOrigWeight", 0.5 );

  double k1;
  double b;
  double k3 = 0;

  std::string smoothing = _param.get("rule"); // need the tfidf parameters.
  indri::api::Parameters spec;
  _parseSmoothing( spec, smoothing );
  std::string method = spec.get( "method", "tfidf" );
  k1 = spec.get( "k1", 1.2 );
  b = spec.get( "b", 0.75 );
  if (method == "okapi") {
    k3 = spec.get( "k3", 7 );
  }

  std::vector<indri::api::DocumentVector*> docVectors = getDocumentVectors( results, fbDocs );
  std::vector<std::string> * rm_vocab = getVocabulary( docVectors );
  INT64 documentCount = _env->documentCount();
  UINT64 colLen = _env->termCount();
  double  avgDocLength = colLen / double(documentCount);

  std::map<std::string, TFIDFTerm> query_model;
  std::map<std::string, TFIDFTerm> orig_model;
  // initialize all TFIDFTerm structures
  for( size_t i = 0; i < rm_vocab->size(); i++ ) {
    TFIDFTerm pterm;
    pterm.stem = (*rm_vocab)[i];
    pterm.relevance = 0;
    query_model[ pterm.stem ] = pterm;
  }
  delete rm_vocab;
  // need original query term counts to initialize the relevance
  // for the existing terms
  const std::vector<indri::server::QueryServer*>& servers = _env->getServers();
  lemur::api::TERMID_T id = 0;
  std::string qTerm;
  istringstream qTerms(originalQuery);
  
  while (qTerms >> qTerm) {
        // take the first id returned
    id = 0;
    for (int i = 0; (id == 0) && (i < servers.size()); i++) {
          id = servers[i]->termID(qTerm);
          if (id) qTerm = servers[i]->termName(id);
    }
    if (id == 0) continue;
    if (orig_model.find(qTerm) == orig_model.end() ) {
      TFIDFTerm pterm;
      pterm.stem = qTerm;
      pterm.relevance = 0;
      orig_model[ pterm.stem ] = pterm;
    }
    TFIDFTerm& term = orig_model[ qTerm ];
    term.relevance++; // count occurrences
  }
  for (  std::map<std::string, TFIDFTerm>::iterator iter = orig_model.begin();
         iter != orig_model.end(); iter++ ) {
      // update the query term weight
       TFIDFTerm& term = iter->second;
       if (term.relevance != 0 ) {
           double queryK1 = 1000; // fixed constant in lemur
           INT64 documentOccurrences = _env->documentStemCount(term.stem);
           double idf = log( ( documentCount + 1 ) / ( documentOccurrences + 0.5 ) );
           // need to test for okapi here...
           term.relevance = ( idf * queryK1 * term.relevance ) / ( term.relevance + queryK1 );
         }
    }
  
  // gather document vectors / statistics for top fbDocs ranked documents
  if (fbDocs > results.size()) fbDocs = results.size();
  for( size_t doc = 0; (int)doc < fbDocs; doc++ ) {
    indri::api::DocumentVector * docVec = docVectors[ doc ];
    indri::utility::greedy_vector<int> positions = docVec->positions();
    const std::vector<std::string>& stems = docVec->stems();
    indri::utility::count_iterator<int> iter( positions.begin(), positions.end() );
    int docLen = int(positions.size());
    // accumulate the term scores
    for( ; iter != positions.end(); ++iter ) {
      const std::string& stem = stems[ (*iter).object ];
      // update the TFIDFTerm structure with computed probabilities
      TFIDFTerm& term = query_model[ stem ];
      int occurrences = (*iter).count;
      INT64 documentOccurrences = _env->documentStemCount(term.stem);
      double idf = log( ( documentCount + 1 ) / ( documentOccurrences + 0.5 ) );
      double score = _BM25TF(occurrences, k1, b, docLen, avgDocLength) * idf;
      //double score = _BM25TF(occurrences, k1, b, docLen, avgDocLength) ;
      term.relevance += score;
    }
    delete docVec;
  }

  // shove into a vector and sort terms by TFIDF metric
  std::vector<TFIDFTerm> sortedModel;
  std::transform( query_model.begin(),
                  query_model.end(),
                  std::back_inserter( sortedModel ),
                  TFIDFTerm::pair_projection() );
  // weight[t] /= fbDocs
  // weight[t] *= fbPosCoeff (default 0.5)
  for (int i = 0; i < sortedModel.size(); i++) {
    sortedModel[i].relevance /= fbDocs;
    sortedModel[i].relevance *= fbOrigWt;
  }
  
  std::sort( sortedModel.begin(), sortedModel.end(), TFIDFTerm::relevance_greater() );

  //update scores for top k term
  int numAdded = 0;
  for (int i = 0; numAdded < fbTerms && i < sortedModel.size(); i++) {
      TFIDFTerm& term = sortedModel[ i ];
      if (term.stem == "[OOV]") continue;
      if (orig_model.find(term.stem) != orig_model.end() ) {
        orig_model[term.stem].relevance += term.relevance ;
      } else {
        orig_model[term.stem] = term;
      }
      numAdded++;
  }
  sortedModel.clear();
  std::transform( orig_model.begin(),
                  orig_model.end(),
                  std::back_inserter( sortedModel ),
                  TFIDFTerm::pair_projection() );
  
  // copy into a vector with only the relevance weights */
  std::vector< std::pair<std::string, double> > probabilities;
                     
  std::transform( sortedModel.begin(),
                  sortedModel.end(),
                  std::back_inserter( probabilities ),
                  TFIDFTerm::relevance_projection() );
  // need to add K terms, with some of the original query possibly
  // remaining in addition.
  return _buildQuery( fbOrigWt, probabilities );
}

std::string indri::query::TFIDFExpander::_buildQuery( double originalWeight,
                                                      const std::vector< std::pair<std::string, double> >& expandedTerms ) {
  std::stringstream ret;
  ret << "#weight( " ;
  // extract top fbTerms and construct a new query
  std::vector< std::pair<std::string, double> >::const_iterator iter;
  for( iter = expandedTerms.begin(); iter != expandedTerms.end(); ++iter ) {
    std::string term = iter->first;
    // skip out of vocabulary term and those terms assigned 0 probability in the query model
    if( term != "[OOV]" && _stopwords.find( term ) == _stopwords.end() && iter->second != 0.0 ) {
      ret << " " 
          << iter->second
          << " \""
          << term
          << "\" ";
    }
  }
  ret << " ) ";
  return ret.str();
}
