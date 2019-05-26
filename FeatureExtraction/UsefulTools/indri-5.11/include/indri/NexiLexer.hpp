#ifndef INC_NexiLexer_hpp_
#define INC_NexiLexer_hpp_

#line 18 "nexilang.g"

  #include "indri/QuerySpec.hpp"
  #include "indri/DateParse.hpp"
  #include "indri/delete_range.hpp"
  #include "indri/NexiLexer.hpp"
  #include <algorithm>
  #include <cctype>

#line 14 "NexiLexer.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.3 (20060307-1): "nexilang.g" -> "NexiLexer.hpp"$ */
#include <antlr/CommonToken.hpp>
#include <antlr/InputBuffer.hpp>
#include <antlr/BitSet.hpp>
#include "NexiLexerTokenTypes.hpp"
#include <antlr/CharScanner.hpp>
ANTLR_BEGIN_NAMESPACE(indri)
ANTLR_BEGIN_NAMESPACE(lang)
class CUSTOM_API NexiLexer : public ANTLR_USE_NAMESPACE(antlr)CharScanner, public NexiLexerTokenTypes
{
#line 58 "nexilang.g"

private:
  bool _numbers;

public:
  void init() {
    _numbers = false;
  }

  void setNumbers(bool f) {
    _numbers = f;
  } 
#line 25 "NexiLexer.hpp"
private:
	void initLiterals();
public:
	bool getCaseSensitiveLiterals() const
	{
		return true;
	}
public:
	NexiLexer(ANTLR_USE_NAMESPACE(std)istream& in);
	NexiLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib);
	NexiLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state);
	ANTLR_USE_NAMESPACE(antlr)RefToken nextToken();
	public: void mSTAR(bool _createToken);
	public: void mO_PAREN(bool _createToken);
	public: void mC_PAREN(bool _createToken);
	public: void mO_SQUARE(bool _createToken);
	public: void mC_SQUARE(bool _createToken);
	public: void mDBL_QUOTE(bool _createToken);
	public: void mQUOTE(bool _createToken);
	public: void mDOT(bool _createToken);
	public: void mCOMMA(bool _createToken);
	public: void mSLASH(bool _createToken);
	public: void mMINUS(bool _createToken);
	public: void mPLUS(bool _createToken);
	protected: void mTAB(bool _createToken);
	protected: void mCR(bool _createToken);
	protected: void mLF(bool _createToken);
	protected: void mSPACE(bool _createToken);
	protected: void mHIGH_CHAR(bool _createToken);
	protected: void mDIGIT(bool _createToken);
	protected: void mASCII_LETTER(bool _createToken);
	protected: void mSAFE_LETTER(bool _createToken);
	protected: void mSAFE_CHAR(bool _createToken);
	protected: void mTEXT_TERM(bool _createToken);
	protected: void mNUMBER(bool _createToken);
	protected: void mFLOAT(bool _createToken);
	public: void mTERM(bool _createToken);
	public: void mOPERATORS(bool _createToken);
	public: void mLESS(bool _createToken);
	public: void mGREATER(bool _createToken);
	public: void mLESSEQ(bool _createToken);
	public: void mGREATEREQ(bool _createToken);
	public: void mEQUALS(bool _createToken);
	public: void mJUNK(bool _createToken);
private:
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
	static const unsigned long _tokenSet_2_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_2;
};

ANTLR_END_NAMESPACE
ANTLR_END_NAMESPACE
#endif /*INC_NexiLexer_hpp_*/
