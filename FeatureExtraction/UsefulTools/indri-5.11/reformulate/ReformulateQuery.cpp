/*==========================================================================
 * Copyright (c) 2012 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */
#include <iostream>
#include <sstream>
#include "indri/QueryEnvironment.hpp"

int main (int argc, char *argv[]) {

  try {
    indri::api::Parameters& param = indri::api::Parameters::instance();
    param.loadCommandLine( argc, argv );
    indri::api::QueryEnvironment env;
    env.setFormulationParameters(param);
    // get processing step parameters here (indexes, etc).
    indri::api::Parameters queries = param[ "query" ];
    int queryOffset = 1;
    // create output parameters file
    std::stringstream qStream;
    qStream << "<parameters>" << std::endl;
    for( size_t i = 0; i < queries.size(); i++ ) {
      std::string queryNumber;
      std::string queryText;
      if (queries[i].exists("text"))
        queryText = (std::string) queries[i]["text"];
      if( queries[i].exists( "number" ) ) {
        queryNumber = (std::string) queries[i]["number"];
      } else {
        int thisQuery=queryOffset + int(i);
        std::stringstream s;
        s << thisQuery;
        queryNumber = s.str();
      }
      if (queryText.size() == 0)
        queryText = (std::string) queries[i];
      // get transform
      std::string reform = env.reformulateQuery(queryText);
      qStream <<"<query><number>" << queryNumber << "</number>\n<text>" 
              << reform << "</text>\n</query>" << std::endl;
    }
    qStream << "</parameters>" << std::endl;
    std::string ofile = param.get("outfile", "queries.param");
    ofstream output(ofile.c_str(), std::ios::out);
    output << qStream.str();
    output.close();
    return 0;
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  } catch( ... ) {
    std::cout << "Caught unhandled exception" << std::endl;
    return -1;
  }
}
