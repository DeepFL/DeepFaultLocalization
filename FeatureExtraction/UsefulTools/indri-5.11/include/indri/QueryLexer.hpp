#ifndef INC_QueryLexer_hpp_
#define INC_QueryLexer_hpp_

#line 18 "indrilang.g"

  #include "indri/QuerySpec.hpp"
  #include "indri/DateParse.hpp"
  #include "indri/delete_range.hpp"
  #include "indri/QueryLexer.hpp"

#line 12 "QueryLexer.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.7 (2006-11-01): "indrilang.g" -> "QueryLexer.hpp"$ */
#include <antlr/CommonToken.hpp>
#include <antlr/InputBuffer.hpp>
#include <antlr/BitSet.hpp>
#include "indri/QueryLexerTokenTypes.hpp"
#include <antlr/CharScanner.hpp>
ANTLR_BEGIN_NAMESPACE(indri)
ANTLR_BEGIN_NAMESPACE(lang)
class CUSTOM_API QueryLexer : public ANTLR_USE_NAMESPACE(antlr)CharScanner, public QueryLexerTokenTypes
{
#line 77 "indrilang.g"

private:
  bool _numbers;

public:
  void init() {
    _numbers = false;
  }

  void setNumbers(bool f) {
    _numbers = f;
  } 
#line 23 "QueryLexer.hpp"
private:
	void initLiterals();
public:
	bool getCaseSensitiveLiterals() const
	{
		return true;
	}
public:
	QueryLexer(ANTLR_USE_NAMESPACE(std)istream& in);
	QueryLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib);
	QueryLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state);
	ANTLR_USE_NAMESPACE(antlr)RefToken nextToken();
	public: void mSTAR(bool _createToken);
	public: void mO_PAREN(bool _createToken);
	public: void mC_PAREN(bool _createToken);
	public: void mO_ANGLE(bool _createToken);
	public: void mC_ANGLE(bool _createToken);
	public: void mO_SQUARE(bool _createToken);
	public: void mC_SQUARE(bool _createToken);
	public: void mO_BRACE(bool _createToken);
	public: void mC_BRACE(bool _createToken);
	public: void mDBL_QUOTE(bool _createToken);
	public: void mQUOTE(bool _createToken);
	public: void mDOT(bool _createToken);
	public: void mCOMMA(bool _createToken);
	public: void mSLASH(bool _createToken);
	public: void mB_SLASH(bool _createToken);
	public: void mDASH(bool _createToken);
	public: void mSPACE_DASH(bool _createToken);
	public: void mCOLON(bool _createToken);
	protected: void mTAB(bool _createToken);
	protected: void mCR(bool _createToken);
	protected: void mLF(bool _createToken);
	protected: void mSPACE(bool _createToken);
	protected: void mHIGH_CHAR(bool _createToken);
	protected: void mDIGIT(bool _createToken);
	protected: void mASCII_LETTER(bool _createToken);
	protected: void mSAFE_LETTER(bool _createToken);
	protected: void mSAFE_CHAR(bool _createToken);
	protected: void mBASESIXFOUR_CHAR(bool _createToken);
	protected: void mTEXT_TERM(bool _createToken);
	protected: void mNUMBER(bool _createToken);
	protected: void mFLOAT(bool _createToken);
	public: void mTERM(bool _createToken);
	protected: void mENCODED_QUOTED_TERM(bool _createToken);
	protected: void mENCODED_TERM(bool _createToken);
	public: void mOPERATOR(bool _createToken);
	public: void mJUNK(bool _createToken);
private:
	
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
};

ANTLR_END_NAMESPACE
ANTLR_END_NAMESPACE
#endif /*INC_QueryLexer_hpp_*/
