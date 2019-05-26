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

#include <sstream>

#include "indri/QueryParserFactory.hpp"
// additional parser/lexer types here
#include "indri/QueryLexer.hpp"
#include "indri/QueryParser.hpp"
#include "indri/NexiLexer.hpp"
#include "indri/NexiParser.hpp"

#include "lemur/Exception.hpp"

namespace indri 
{
  namespace api 
  {
    
    /* template wrapper class for antlr generated parser/lexer combination.
     */
    template<typename _lexType, typename _parseType>

    class _Wrapper : public QueryParserWrapper {
    public:
      _Wrapper(_lexType *lexer,
               _parseType *parser,
               std::istringstream* queryStream) :
        _lexer(lexer),_parser(parser), _queryStream(queryStream) {
      }

      virtual indri::lang::ScoredExtentNode *query() {
        return _parser->query();
      }

      virtual ~_Wrapper() {
        delete(_parser);
        delete(_lexer);
        delete(_queryStream);
      }
    private:
      _lexType *_lexer;
      _parseType *_parser;
      std::istringstream* _queryStream;      

    };

    QueryParserWrapper *QueryParserFactory::get(const std::string &query, 
                                                const std::string &parserType) {
      QueryParserWrapper *retval = 0;
      // need scope 
      std::istringstream* queryStream = new std::istringstream(query);
      if (parserType == "indri") {  
        indri::lang::QueryLexer *lexer = new indri::lang::QueryLexer ( *queryStream );
        indri::lang::QueryParser *parser = new indri::lang::QueryParser ( *lexer );
        // this step is required to initialize some internal
        // parser variables, since ANTLR grammars can't add things
        // to the constructor
        parser->init( lexer );
        lexer->init();
        QueryParserWrapper *retval = new _Wrapper<indri::lang::QueryLexer, indri::lang::QueryParser>(lexer, parser, queryStream);
        return retval;
      } 
      else if (parserType == "nexi") {
        indri::lang::NexiLexer *lexer = new indri::lang::NexiLexer ( *queryStream );
        indri::lang::NexiParser *parser = new indri::lang::NexiParser ( *lexer );
        // this step is required to initialize some internal
        // parser variables, since ANTLR grammars can't add things
        // to the constructor
        parser->init( lexer );
        lexer->init();
        QueryParserWrapper *retval = new _Wrapper<indri::lang::NexiLexer, indri::lang::NexiParser>(lexer, parser, queryStream);
        return retval;
      }
      else {
        LEMUR_THROW(LEMUR_MISSING_PARAMETER_ERROR, "could not query parser for " + parserType);
      }
    }
  }
}

