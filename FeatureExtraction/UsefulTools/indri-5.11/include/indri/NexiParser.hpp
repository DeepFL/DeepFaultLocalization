#ifndef INC_NexiParser_hpp_
#define INC_NexiParser_hpp_

#line 18 "nexilang.g"

  #include "indri/QuerySpec.hpp"
  #include "indri/DateParse.hpp"
  #include "indri/delete_range.hpp"
  #include "indri/NexiLexer.hpp"
  #include <algorithm>
  #include <cctype>

#line 14 "NexiParser.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.3 (20060307-1): "nexilang.g" -> "NexiParser.hpp"$ */
#include <antlr/TokenStream.hpp>
#include <antlr/TokenBuffer.hpp>
#include "NexiLexerTokenTypes.hpp"
#include <antlr/LLkParser.hpp>

ANTLR_BEGIN_NAMESPACE(indri)
ANTLR_BEGIN_NAMESPACE(lang)
class CUSTOM_API NexiParser : public ANTLR_USE_NAMESPACE(antlr)LLkParser, public NexiLexerTokenTypes
{
#line 132 "nexilang.g"

private:
  // storage for allocated nodes
  std::vector<indri::lang::Node*> _nodes;
  // makes sure nodes go away when parser goes away
  indri::utility::VectorDeleter<indri::lang::Node*> _deleter;
    bool _shrinkage;
  
public:
  void init( NexiLexer* lexer ) {
    _deleter.setVector( _nodes );
    _shrinkage = true;
  }

  void setShrinkage( bool shrink ) {
    _shrinkage = shrink;
  }
#line 25 "NexiParser.hpp"
public:
	void initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory );
protected:
	NexiParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k);
public:
	NexiParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf);
protected:
	NexiParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k);
public:
	NexiParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer);
	NexiParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state);
	int getNumTokens() const
	{
		return NexiParser::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return NexiParser::tokenNames[type];
	}
	const char* const* getTokenNames() const
	{
		return NexiParser::tokenNames;
	}
	public:  indri::lang::ScoredExtentNode*  query();
	public:  indri::lang::NestedExtentInside*  path();
	public:  indri::lang::RawExtentNode *  field();
	public:  indri::lang::ScoredExtentNode*  clause();
	public:  indri::lang::ScoredExtentNode*  termList();
	public:  indri::lang::ScoredExtentNode*  term();
	public:  indri::lang::ScoredExtentNode*  filter();
	public:  indri::lang::UnweightedCombinationNode*  logical();
	public:  indri::lang::ScoredExtentNode*  aboutClause();
	public:  indri::lang::RawExtentNode *  arithmeticClause();
	public:  indri::lang::ScoredExtentNode*  filterParens();
	public:  INT64  number();
	public:  indri::lang::ScoredExtentNode *  unrestrictedTerm();
	public:  indri::lang::RawExtentNode*  rawText();
	public:  indri::lang::ODNode*  odNode();
public:
	ANTLR_USE_NAMESPACE(antlr)RefAST getAST()
	{
		return returnAST;
	}
	
protected:
	ANTLR_USE_NAMESPACE(antlr)RefAST returnAST;
private:
	static const char* tokenNames[];
#ifndef NO_STATIC_CONSTS
	static const int NUM_TOKENS = 44;
#else
	enum {
		NUM_TOKENS = 44
	};
#endif
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
};

ANTLR_END_NAMESPACE
ANTLR_END_NAMESPACE
#endif /*INC_NexiParser_hpp_*/
