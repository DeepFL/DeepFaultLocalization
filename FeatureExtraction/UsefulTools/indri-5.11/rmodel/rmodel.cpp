

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
// relevancemodel
//
// 23 June 2005 -- tds
//
// Options are:
//    index
//    server
//    query
//    ngram -- default is '1' (unigram)

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

static void open_indexes( indri::api::QueryEnvironment& environment, indri::api::Parameters& param ) {
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

static void printGrams( const std::string& query, const std::vector<indri::query::RelevanceModel::Gram*>& grams ) {
  std::cout << "# query: " << query << std::endl;
  for( size_t j=0; j<grams.size(); j++ ) {
    std::cout << std::setw(15)
              << std::setprecision(15)
              << grams[j]->weight << " ";
    std::cout << grams[j]->terms.size() << " ";

    for( size_t k=0; k<grams[j]->terms.size(); k++ ) {
      std::cout << grams[j]->terms[k] << " ";
    }

    std::cout << std::endl;
  }
}

static void usage( indri::api::Parameters param ) {
  if( !param.exists( "query" ) || !( param.exists( "index" ) || param.exists( "server" ) ) || !param.exists( "documents" ) ) {
   std::cerr << "rmodel usage: " << std::endl
             << "   rmodel -query=myquery -index=myindex -documents=10 -maxGrams=2" << std::endl
             << "     myquery: a valid Indri query (be sure to use quotes around it if there are spaces in it)" << std::endl
             << "     myindex: a valid Indri index" << std::endl
             << "     documents: the number of documents to use to build the relevance model" << std::endl
             << "     maxGrams (optional): maximum length (in words) of phrases to be added to the model, default is 1 (unigram)" << std::endl;
   exit(-1);
  }
}

// open repository
// for each query
//    run query
//    get document vectors from results, save weights from retrieval
//    extract 1-grams, 2-grams, 3-grams etc. as appropriate
//    run background statistics on the n-grams
//    print result
// close repository

int main( int argc, char** argv ) {
  try {
    indri::api::Parameters& param = indri::api::Parameters::instance();
    param.loadCommandLine( argc, argv );
    usage( param );

    indri::api::QueryEnvironment environment;
    open_indexes( environment, param );
              
    indri::api::Parameters parameterQueries = param[ "query" ];
    std::string rmSmoothing = ""; // eventually, we should offer relevance model smoothing
    int documents = (int) param[ "documents" ];
    int maxGrams = (int) param.get( "maxGrams", 1 ); // unigram is default

    for( size_t i=0; i<parameterQueries.size(); i++ ) {
      std::string query = parameterQueries[i];
      indri::query::RelevanceModel model( environment, rmSmoothing, maxGrams, documents );
      model.generate( query );

      const std::vector<indri::query::RelevanceModel::Gram*>& grams = model.getGrams();
      printGrams( query, grams );
    }
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  } catch( ... ) {
    std::cout << "Caught an unhandled exception" << std::endl;
  }

  return 0;
}

