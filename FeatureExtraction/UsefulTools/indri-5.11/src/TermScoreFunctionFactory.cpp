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
// TermScoreFunctionFactory
//
// 19 August 2004 -- tds
//

#include "indri/TermScoreFunctionFactory.hpp"
#include "indri/BeliefNode.hpp"
#include "indri/TFIDFTermScoreFunction.hpp"
#include "indri/JelinekMercerTermScoreFunction.hpp"
#include "indri/DirichletTermScoreFunction.hpp"
#include "indri/TwoStageTermScoreFunction.hpp"
#include "indri/Parameters.hpp"

static void termscorefunctionfactory_parse( indri::api::Parameters& converted, const std::string& spec );

//
// Here's the place to add your own TermScoreFunction.
//
// Your job is to change the if( method == "..." ) statement in the get() method
// so that it can recognize your TermScoreFunction.  A spec line for Dirichlet might look like this:
//    dirichlet:1500
// That line represents Dirichlet smoothing with mu=1500.  The lines "d:1500" or "dir:1500" would also work.
//
// You'll want to pick a smoothing spec format with the name of your TermScoreFunction at the beginning,
// followed by a colon, followed by the parameters your smoothing method takes.  Return a pointer
// to a newly allocated TermScoreFunction.  It is the caller's job to delete the TermScoreFunction when
// it is finished with it.
//

indri::query::TermScoreFunction* indri::query::TermScoreFunctionFactory::get( const std::string& stringSpec, double occurrences, double contextSize, int documentOccurrences, int documentCount ) {
  indri::api::Parameters spec;
  termscorefunctionfactory_parse( spec, stringSpec );
  std::string method = spec.get( "method", "dirichlet" );

  // this is something that never happens in our collection, so we assume that it
  // happens somewhat less often than 1./collectionSize.  I picked 1/(2*collectionSize)
  // because it seemed most appropriate (from InferenceNetworkBuilder)
  if (contextSize == 0) contextSize = 1; // For non-existant fields.
  
  double collectionFrequency = occurrences ? (occurrences/contextSize) :
    (collectionFrequency = 1.0 / double(contextSize*2.));

  if( method == "dirichlet" || method == "d" || method == "dir" ) {
    // dirichlet -- takes parameter "mu"
    double mu = spec.get( "mu", 2500 );
    double docmu=spec.get("documentMu",-1.0); // default is no doc-level smoothing
    return new indri::query::DirichletTermScoreFunction( mu, collectionFrequency, docmu );
  } else if( method == "linear" || method == "jm" || method == "jelinek-mercer" ) {
    // jelinek-mercer -- can take parameters collectionLambda (or just lambda) and documentLambda
    double documentLambda = spec.get( "documentLambda", 0.0 );
    double collectionLambda;
    
    if( spec.exists( "collectionLambda" ) )
      collectionLambda = spec.get( "collectionLambda", 0.4 );
    else
      collectionLambda = spec.get( "lambda", 0.4 );

    return new indri::query::JelinekMercerTermScoreFunction( collectionFrequency, collectionLambda, documentLambda );
  } else if( method == "two" || method == "two-stage" || method == "twostage" ) {
    // twostage -- takes parameters mu and lambda
    double mu = spec.get( "mu", 2500 );
    double lambda = spec.get( "lambda", 0.4 );
    
    return new indri::query::TwoStageTermScoreFunction( mu, lambda, collectionFrequency );
  } else if ( method == "tfidf" ) {
    double k1 = spec.get( "k1", 1.2 );
    double b = spec.get( "b", 0.75 );
    int qtf = spec.get("qtf", 1);
    double idf = log( ( documentCount + 1 ) / ( documentOccurrences + 0.5 ) );
    double  avgDocLength = contextSize / double(documentCount);
    if (spec.exists("qtw")) {
      double weight = spec.get("qtw", 1.0);
      return new indri::query::TFIDFTermScoreFunction( idf, avgDocLength, weight, k1, b, false );
      } else {
      return new indri::query::TFIDFTermScoreFunction( idf, avgDocLength, qtf, k1, b, false );
    }
  } else if ( method == "okapi" ) {
    double k1 = spec.get( "k1", 1.2 );
    double b = spec.get( "b", 0.75 );
    double k3 = spec.get( "k3", 7 );
    int qtf = spec.get("qtf", 1);
    double idf = log( ( documentCount - documentOccurrences + 0.5 ) / ( documentOccurrences + 0.5 ) );
    double  avgDocLength = contextSize / double(documentCount);
    if (spec.exists("qtw")) {
      double weight = spec.get("qtw", 1.0);
      return new indri::query::TFIDFTermScoreFunction( idf, avgDocLength, weight, k1, b, true, k3 );
      } else {
      return new indri::query::TFIDFTermScoreFunction( idf, avgDocLength, qtf, k1, b, true, k3 );
    }
  }

  // if nothing else worked, we'll use dirichlet with mu=2500
  return new indri::query::DirichletTermScoreFunction( 2500, collectionFrequency );
}

static void termscorefunctionfactory_parse( indri::api::Parameters& converted, const std::string& spec ) {
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

