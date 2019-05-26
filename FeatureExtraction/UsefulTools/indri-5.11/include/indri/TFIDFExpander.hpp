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

#ifndef INDRI_TFIDFEXPANDER_HPP
#define INDRI_TFIDFEXPANDER_HPP

#include <string>
#include <vector>
#include <map>

#include "indri/QueryExpander.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/Parameters.hpp"
namespace indri
{
  namespace query
  {
    
    class TFIDFExpander : public QueryExpander  {
    private: 
      std::string _buildQuery(double originalWeight,
                              const std::vector< std::pair<std::string, double> >& expansionTerms );
    public:
      TFIDFExpander( indri::api::QueryEnvironment * env , indri::api::Parameters& param );

      virtual std::string expand( std::string originalQuery, std::vector<indri::api::ScoredExtentResult>& results );
    };
  }
}

#endif // INDRI_TFIDFEXPANDER_HPP

