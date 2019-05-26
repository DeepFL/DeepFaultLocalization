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

#ifndef INDRI_REFORMULATEQUERY_HPP
#define INDRI_REFORMULATEQUERY_HPP
#include <vector>
#include <string>
#include "indri/Parameters.hpp"
#include "indri/StopStructureRemover.hpp"
#include "indri/QueryStopper.hpp"

namespace indri
{
  namespace query
  {  
    class ReformulateQuery {
    private:
      indri::api::Parameters *params;
    public:
    struct weighted_field 
    {
      std::string field;
      std::string weight;
      weighted_field(std::string f, std::string w) {
        field = f;
        weight = w;
      }
    };
      ReformulateQuery(indri::api::Parameters &p) { 
        params = new indri::api::Parameters(p);
      };
      ReformulateQuery( ) { 
        params = NULL;
      };
      ~ReformulateQuery() {
        //        delete(params);
      }
      void setParameters(indri::api::Parameters &p){
        //        delete(params);
        params = new indri::api::Parameters(p);        
      }
      
      std::string transform(std::string queryTerms);
      // utilities
      std::string downcase_string( const std::string& str );
      std::vector<std::string> split(std::string q, char d);
      std::string replaceAll(std::string result, 
					const std::string& replaceWhat, const std::string& replaceWithWhat);
      std::string trim(std::string text);
      std::string makeIndriFriendly(std::string query);

      // the rest of these could be private...
      std::string generateSDMQuery(std::vector<std::string> strs,
                                   std::vector<indri::query::ReformulateQuery::weighted_field> fields);
      std::string generateSDMQuery(std::vector<std::string> strs);
      std::string generateSDMQuery(std::string q);
      std::string generateFDMQuery(std::vector<std::string> strs,
                                   std::vector<indri::query::ReformulateQuery::weighted_field> fields);
      std::string generateFDMQuery(std::vector<std::string> strs);
      std::string generateFDMQuery(std::string q);
      std::string generateCombineQuery(std::vector<std::string> strs);
      std::string generateCombineQuery(std::string q);
      std::string generateCMUFDMQuery(std::vector<std::string> strs);
      std::string generateCMUFDMQuery(std::string q);
    };
  }
}

#endif
