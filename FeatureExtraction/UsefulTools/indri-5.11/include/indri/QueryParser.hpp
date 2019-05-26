#ifndef INC_QueryParser_hpp_
#define INC_QueryParser_hpp_

#line 18 "indrilang.g"

  #include "indri/QuerySpec.hpp"
  #include "indri/DateParse.hpp"
  #include "indri/delete_range.hpp"
  #include "indri/QueryLexer.hpp"

#line 12 "QueryParser.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.7 (2006-11-01): "indrilang.g" -> "QueryParser.hpp"$ */
#include <antlr/TokenStream.hpp>
#include <antlr/TokenBuffer.hpp>
#include "indri/QueryLexerTokenTypes.hpp"
#include <antlr/LLkParser.hpp>

ANTLR_BEGIN_NAMESPACE(indri)
ANTLR_BEGIN_NAMESPACE(lang)
class CUSTOM_API QueryParser : public ANTLR_USE_NAMESPACE(antlr)LLkParser, public QueryLexerTokenTypes
{
#line 158 "indrilang.g"

private:
  // storage for allocated nodes
  std::vector<indri::lang::Node*> _nodes;
  // makes sure nodes go away when parser goes away
  indri::utility::VectorDeleter<indri::lang::Node*> _deleter;
    
  indri::lang::RawExtentNode * innerMost( indri::lang::ScoredExtentNode* sr ) {
    indri::lang::RawExtentNode * ou = 0;
    // set the new outer node we need to pass down (the innermost of field or field list of 
    // of this restriction)
    indri::lang::ExtentRestriction * er = dynamic_cast<indri::lang::ExtentRestriction *>(sr);
    if (er != 0) {
      indri::lang::RawExtentNode * f = er->getField();
      indri::lang::ExtentInside * ei = dynamic_cast<indri::lang::ExtentInside *>(f);
      while (ei != 0) {
        f = ei->getInner();
        ei = dynamic_cast<indri::lang::ExtentInside *>(f);
      }       
      ou = f;
    }
    return ou;         
  }
  
public:
  void init( QueryLexer* lexer ) {
    _deleter.setVector( _nodes );
  }
#line 23 "QueryParser.hpp"
public:
	void initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory );
protected:
	QueryParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k);
public:
	QueryParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf);
protected:
	QueryParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k);
public:
	QueryParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer);
	QueryParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state);
	int getNumTokens() const
	{
		return QueryParser::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return QueryParser::tokenNames[type];
	}
	const char* const* getTokenNames() const
	{
		return QueryParser::tokenNames;
	}
	public:  indri::lang::ScoredExtentNode*  query();
	public:  indri::lang::ScoredExtentNode*  scoredExtentNode(
		 indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  weightNode(
		indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  combineNode(
		indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  orNode(
		indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  notNode(
		indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  wandNode(
		indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  wsumNode(
		indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  maxNode(
		indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::PriorNode*  priorNode();
	public:  indri::lang::FilRejNode*  filrejNode(
		 indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::FilReqNode*  filreqNode(
		 indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::FilReqNode*  scoreifNode(
		 indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::FilRejNode*  scoreifnotNode(
		 indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  scoredRaw(
		 indri::lang::RawExtentNode * ou 
	);
	public:  RawExtentNode*  qualifiedTerm();
	public:  ExtentOr*  context_list(
		 indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::RawExtentNode*  unqualifiedTerm();
	public:  indri::lang::ScoredExtentNode*  weightedList(
		 indri::lang::WeightedCombinationNode* wn, indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  extentRestriction(
		 indri::lang::ScoredExtentNode* sn, indri::lang::RawExtentNode * ou 
	);
	public:  double  floating();
	public:  indri::lang::ScoredExtentNode*  sumList(
		 indri::lang::WSumNode* wn, indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  unweightedList(
		 indri::lang::UnweightedCombinationNode* cn, indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::ScoredExtentNode*  sumNode(
		indri::lang::RawExtentNode * ou 
	);
	public:  indri::lang::WeightedExtentOr*  wsynNode();
	public:  RawExtentNode*  unscoredTerm();
	public:  indri::lang::ODNode*  odNode();
	public:  indri::lang::UWNode*  uwNode();
	public:  indri::lang::BAndNode*  bandNode();
	public:  indri::lang::Field*  anyField();
	public: std::string  fieldNameString();
	public:  indri::lang::ExtentAnd*  field_list();
	public:  indri::lang::FieldLessNode*  dateBefore();
	public:  indri::lang::FieldGreaterNode*  dateAfter();
	public:  indri::lang::FieldBetweenNode*  dateBetween();
	public:  indri::lang::FieldEqualsNode*  dateEquals();
	public:  indri::lang::ExtentOr*  synonym_list();
	public:  indri::lang::ExtentOr*  synonym_list_brace();
	public:  indri::lang::ExtentOr*  synonym_list_alt();
	public:  indri::lang::FieldLessNode*  lessNode();
	public:  indri::lang::FieldGreaterNode*  greaterNode();
	public:  indri::lang::FieldBetweenNode*  betweenNode();
	public:  indri::lang::FieldEqualsNode*  equalsNode();
	public:  indri::lang::IndexTerm*  rawText();
	public:  indri::lang::ODNode*  hyphenTerm();
	public:  indri::lang::WildcardTerm*  wildcardOpNode();
	public: indri::lang::IndexTerm *  hyphenate();
	public:  std::string  fstring();
	public:  indri::lang::ExtentInside*  path();
	public:  indri::lang::ExtentInside*  pathOperator();
	public:  indri::lang::Field*  field_restriction();
	public:  UINT64  date();
	public:  UINT64  slashDate();
	public:  UINT64  spaceDate();
	public:  UINT64  dashDate();
	public:  INT64  number();
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
	static const int NUM_TOKENS = 69;
#else
	enum {
		NUM_TOKENS = 69
	};
#endif
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
	static const unsigned long _tokenSet_2_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_2;
	static const unsigned long _tokenSet_3_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_3;
	static const unsigned long _tokenSet_4_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_4;
	static const unsigned long _tokenSet_5_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_5;
	static const unsigned long _tokenSet_6_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_6;
	static const unsigned long _tokenSet_7_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_7;
	static const unsigned long _tokenSet_8_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_8;
	static const unsigned long _tokenSet_9_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_9;
};

ANTLR_END_NAMESPACE
ANTLR_END_NAMESPACE
#endif /*INC_QueryParser_hpp_*/
