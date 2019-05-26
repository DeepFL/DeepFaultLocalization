/*==========================================================================
 * Copyright (c) 2010 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/

//
// clarity
//
// 15 July 2010 -- dmf
//
// Options are:
//    index
//    server
//    query
//    number of documents
//    number of terms
//
// computes query clarity and prints it out to std::out
#include <cmath>
#include "indri/Parameters.hpp"
#include "indri/RelevanceModel.hpp"

static bool copy_parameters_to_string_vector( std::vector<std::string>& vec, indri::api::Parameters p, const std::string& parameterName ) {
  if( !p.exists(parameterName) )
    return false;

  indri::api::Parameters slice = p[parameterName];
  
  for( size_t i=0; i<slice.size(); i++ ) {
    vec.push_back( slice[i] );
  }

  return true;
}

static void open_indexes( indri::api::QueryEnvironment& environment, 
                          indri::api::Parameters& param ) {
  if( param.exists( "index" ) ) {
    indri::api::Parameters indexes = param["index"];
    for( unsigned int i=0; i < indexes.size(); i++ ) {
      environment.addIndex( std::string(indexes[i]) );
    }
  }
  if( param.exists( "server" ) ) {
    indri::api::Parameters servers = param["server"];
    for( unsigned int i=0; i < servers.size(); i++ ) {
      environment.addServer( std::string(servers[i]) );
    }
  }
  std::vector<std::string> smoothingRules;
  if( copy_parameters_to_string_vector( smoothingRules, param, "rule" ) )
    environment.setScoringRules( smoothingRules );
}

// how to just compute the clarity score without printing out the terms.
static double clarity( const std::string& query, 
                       indri::api::QueryEnvironment & env, 
                       const std::vector<indri::query::RelevanceModel::Gram*>& grams, int numTerms ) {

  int count = 0;
  double sum=0, ln_Pr=0;
  for( size_t j=0; j< numTerms && j < grams.size(); j++ ) {
    std::string t = grams[j]->terms[0];
    count++;
    // query-clarity = SUM_w{P(w|Q)*log(P(w|Q)/P(w))}
    // P(w)=cf(w)/|C|
    // the relevance model uses stemmed terms, so use stemCount
    double pw = ((double)env.stemCount(t)/(double)env.termCount());
    // P(w|Q) is a prob computed by any model, e.g. relevance models
    double pwq = grams[j]->weight;
    sum += pwq;    
    ln_Pr += (pwq)*log(pwq/pw);
  }
  return (ln_Pr/(sum ? sum : 1.0)/log(2.0));
}

static void printClarity( const std::string& query, 
                          indri::api::QueryEnvironment & env, 
                          const std::vector<indri::query::RelevanceModel::Gram*>& grams, int numTerms ) {

  int count = 0;
  double sum=0, ln_Pr=0;
  for( size_t j=0; j< numTerms && j < grams.size(); j++ ) {
    std::string t = grams[j]->terms[0];
    count++;
    // query-clarity = SUM_w{P(w|Q)*log(P(w|Q)/P(w))}
    // P(w)=cf(w)/|C|
    // the relevance model uses stemmed terms, so use stemCount
    double pw = ((double)env.stemCount(t)/(double)env.termCount());
    // P(w|Q) is a prob computed by any model, e.g. relevance models
    double pwq = grams[j]->weight;
    sum += pwq;    
    ln_Pr += (pwq)*log(pwq/pw);
  }
  std::cout << "# query: " << query <<  " = " << count << " " 
            << (ln_Pr/(sum ? sum : 1.0)/log(2.0)) << std::endl;
  for( size_t j=0; j< numTerms && j < grams.size(); j++ ) {
    std::string t = grams[j]->terms[0];
    double pw = ((double)env.stemCount(t)/(double)env.termCount());
    std::cout << t << " "
              << (grams[j]->weight*log(grams[j]->weight/
    // the relevance model uses stemmed terms, so use stemCount
                            ((double)env.stemCount(t)/
                             (double)env.termCount())))/log(2.0) << std::endl;
  }
}

static void usage( indri::api::Parameters param ) {
  if( !param.exists( "query" ) || 
      !( param.exists( "index" ) || param.exists( "server" ) )) {
   std::cerr << "clarity usage: " << std::endl
             << "   clarity -query=myquery -index=myindex -documents=10 -terms=5 -smoothing=\"method:jm,lambda,0.5\"" << std::endl
             << "OR clarity -query=myquery -server=myserver -documents=10 -terms=5 -smoothing=\"method:jm,lambda,0.5\"" << std::endl
             << "     myquery: a valid Indri query (be sure to use quotes around it if there are spaces in it)" << std::endl
             << "     myindex: a valid Indri index" << std::endl
             << "     myserver: a valid IndriDaemon instance" << std::endl
             << "     documents: the number of documents to use to build the relevance model. Default is 5" << std::endl
             << "     terms: the number of terms to use to build the relevance model. Default is 10" 
             << "     smoothing: the smoothing rule to apply. Default is linear smoothing with lambda=0.5" << std::endl;
   exit(-1);
  }
}

int main( int argc, char** argv ) {
  try {
    indri::api::Parameters& param = indri::api::Parameters::instance();
    param.loadCommandLine( argc, argv );
    usage( param );

    indri::api::QueryEnvironment environment;
    open_indexes( environment, param );
              
    indri::api::Parameters parameterQueries = param[ "query" ];
    std::string rmSmoothing = param.get("smoothing", "method:jm,lambda,0.5");
    int documents = (int) param.get( "documents", 5 );
    int terms = (int) param.get( "terms", 10 );
    int maxGrams = 1; 

    for( size_t i=0; i<parameterQueries.size(); i++ ) {
      std::string query = parameterQueries[i];
      indri::query::RelevanceModel model( environment, rmSmoothing, maxGrams, documents );
      model.generate( query );

      const std::vector<indri::query::RelevanceModel::Gram*>& grams = model.getGrams();
      printClarity(query, environment, grams, terms);
    }
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  } catch( ... ) {
    std::cout << "Caught an unhandled exception" << std::endl;
  }
  return 0;
}

