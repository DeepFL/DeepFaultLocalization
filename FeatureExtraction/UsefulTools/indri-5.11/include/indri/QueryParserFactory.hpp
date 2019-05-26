/*==========================================================================
 * Copyright (c) 2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// QueryParserFactory
//
// 30 Aug 2005 -- dmf
//

#ifndef INDRI_QUERYPARSERFACTORY_HPP
#define INDRI_QUERYPARSERFACTORY_HPP
#include <string>
#include "antlr/ANTLRException.hpp"
namespace indri
{
  namespace lang 
  {
    class ScoredExtentNode;
  }
  
  namespace api
  {
    /*! wrapper class for antlr generated parser/lexer combination.
     */
    class QueryParserWrapper {
    public:
      virtual ~QueryParserWrapper() {}
      virtual indri::lang::ScoredExtentNode *query() = 0;
    };


    /*! Create instances of appropriate query lexer/parsers
     */
    class QueryParserFactory {
    private:

    
    public:
      /// \brief Create an instance of the appropriate query parser.
      /// @param query the query string to give to the lexer.
      /// @param parserType the type of query parser to create. Default is indri.
      static QueryParserWrapper *get(const std::string &query, 
                                     const std::string &parserType = "indri");
    };
  }
}

#endif // INDRI_QUERYPARSERFACTORY_HPP
