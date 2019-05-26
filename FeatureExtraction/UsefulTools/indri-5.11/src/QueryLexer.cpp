/* $ANTLR 2.7.7 (2006-11-01): "indrilang.g" -> "QueryLexer.cpp"$ */
#include "indri/QueryLexer.hpp"
#include <antlr/CharBuffer.hpp>
#include <antlr/TokenStreamException.hpp>
#include <antlr/TokenStreamIOException.hpp>
#include <antlr/TokenStreamRecognitionException.hpp>
#include <antlr/CharStreamException.hpp>
#include <antlr/CharStreamIOException.hpp>
#include <antlr/NoViableAltForCharException.hpp>

ANTLR_BEGIN_NAMESPACE(indri)
ANTLR_BEGIN_NAMESPACE(lang)
#line 1 "indrilang.g"
#line 13 "QueryLexer.cpp"
QueryLexer::QueryLexer(ANTLR_USE_NAMESPACE(std)istream& in)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(new ANTLR_USE_NAMESPACE(antlr)CharBuffer(in),true)
{
	initLiterals();
}

QueryLexer::QueryLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(ib,true)
{
	initLiterals();
}

QueryLexer::QueryLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(state,true)
{
	initLiterals();
}

void QueryLexer::initLiterals()
{
	literals["#any"] = 18;
	literals["#equals"] = 30;
	literals["#not"] = 9;
	literals["#datebetween"] = 25;
	literals["#between"] = 29;
	literals["#wsyn"] = 20;
	literals["#prior"] = 22;
	literals["#sum"] = 4;
	literals["#less"] = 27;
	literals["#or"] = 8;
	literals["#uw"] = 10;
	literals["#filreq"] = 14;
	literals["#weight"] = 12;
	literals["#dateafter"] = 23;
	literals["#wsum"] = 5;
	literals["#datebefore"] = 24;
	literals["#filrej"] = 15;
	literals["#dateequals"] = 26;
	literals["#od"] = 7;
	literals["#band"] = 19;
	literals["#wand"] = 6;
	literals["#greater"] = 28;
	literals["#scoreif"] = 16;
	literals["#max"] = 13;
	literals["#syn"] = 21;
	literals["#wildcard"] = 31;
	literals["#scoreifnot"] = 17;
	literals["#combine"] = 11;
}

ANTLR_USE_NAMESPACE(antlr)RefToken QueryLexer::nextToken()
{
	ANTLR_USE_NAMESPACE(antlr)RefToken theRetToken;
	for (;;) {
		ANTLR_USE_NAMESPACE(antlr)RefToken theRetToken;
		int _ttype = ANTLR_USE_NAMESPACE(antlr)Token::INVALID_TYPE;
		resetText();
		try {   // for lexical and char stream error handling
			switch ( LA(1)) {
			case 0x2a /* '*' */ :
			{
				mSTAR(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x28 /* '(' */ :
			{
				mO_PAREN(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x29 /* ')' */ :
			{
				mC_PAREN(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x3c /* '<' */ :
			{
				mO_ANGLE(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x3e /* '>' */ :
			{
				mC_ANGLE(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x5b /* '[' */ :
			{
				mO_SQUARE(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x5d /* ']' */ :
			{
				mC_SQUARE(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x7b /* '{' */ :
			{
				mO_BRACE(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x7d /* '}' */ :
			{
				mC_BRACE(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x22 /* '\"' */ :
			{
				mDBL_QUOTE(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x27 /* '\'' */ :
			{
				mQUOTE(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x2e /* '.' */ :
			{
				mDOT(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x2c /* ',' */ :
			{
				mCOMMA(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x2f /* '/' */ :
			{
				mSLASH(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x5c /* '\\' */ :
			{
				mB_SLASH(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x2d /* '-' */ :
			{
				mDASH(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x3a /* ':' */ :
			{
				mCOLON(true);
				theRetToken=_returnToken;
				break;
			}
			case 0x23 /* '#' */ :
			{
				mOPERATOR(true);
				theRetToken=_returnToken;
				break;
			}
			default:
				if ((LA(1) == 0x20 /* ' ' */ ) && (LA(2) == 0x2d /* '-' */ )) {
					mSPACE_DASH(true);
					theRetToken=_returnToken;
				}
				else if ((_tokenSet_0.member(LA(1)))) {
					mTERM(true);
					theRetToken=_returnToken;
				}
				else if ((_tokenSet_1.member(LA(1))) && (true)) {
					mJUNK(true);
					theRetToken=_returnToken;
				}
			else {
				if (LA(1)==EOF_CHAR)
				{
					uponEOF();
					_returnToken = makeToken(ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE);
				}
				else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
			}
			}
			if ( !_returnToken )
				goto tryAgain; // found SKIP token

			_ttype = _returnToken->getType();
			_returnToken->setType(_ttype);
			return _returnToken;
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& e) {
				throw ANTLR_USE_NAMESPACE(antlr)TokenStreamRecognitionException(e);
		}
		catch (ANTLR_USE_NAMESPACE(antlr)CharStreamIOException& csie) {
			throw ANTLR_USE_NAMESPACE(antlr)TokenStreamIOException(csie.io);
		}
		catch (ANTLR_USE_NAMESPACE(antlr)CharStreamException& cse) {
			throw ANTLR_USE_NAMESPACE(antlr)TokenStreamException(cse.getMessage());
		}
tryAgain:;
	}
}

void QueryLexer::mSTAR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = STAR;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('*' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mO_PAREN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = O_PAREN;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('(' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mC_PAREN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = C_PAREN;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match(')' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mO_ANGLE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = O_ANGLE;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('<' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mC_ANGLE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = C_ANGLE;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('>' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mO_SQUARE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = O_SQUARE;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('[' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mC_SQUARE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = C_SQUARE;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match(']' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mO_BRACE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = O_BRACE;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('{' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mC_BRACE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = C_BRACE;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('}' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mDBL_QUOTE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = DBL_QUOTE;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('\"' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mQUOTE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = QUOTE;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('\'' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mDOT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = DOT;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('.' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mCOMMA(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = COMMA;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match(',' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mSLASH(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = SLASH;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('/' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mB_SLASH(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = B_SLASH;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('\\' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mDASH(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = DASH;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('-' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mSPACE_DASH(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = SPACE_DASH;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match(" -");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mCOLON(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = COLON;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match(':' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mTAB(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = TAB;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('\t' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mCR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = CR;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('\n' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mLF(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = LF;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match('\r' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mSPACE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = SPACE;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	match(' ' /* charlit */ );
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mHIGH_CHAR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = HIGH_CHAR;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	matchRange(static_cast<unsigned char>('\200'),static_cast<unsigned char>('\377'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mDIGIT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = DIGIT;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	{
	matchRange('0','9');
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mASCII_LETTER(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = ASCII_LETTER;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	{
	switch ( LA(1)) {
	case 0x61 /* 'a' */ :
	case 0x62 /* 'b' */ :
	case 0x63 /* 'c' */ :
	case 0x64 /* 'd' */ :
	case 0x65 /* 'e' */ :
	case 0x66 /* 'f' */ :
	case 0x67 /* 'g' */ :
	case 0x68 /* 'h' */ :
	case 0x69 /* 'i' */ :
	case 0x6a /* 'j' */ :
	case 0x6b /* 'k' */ :
	case 0x6c /* 'l' */ :
	case 0x6d /* 'm' */ :
	case 0x6e /* 'n' */ :
	case 0x6f /* 'o' */ :
	case 0x70 /* 'p' */ :
	case 0x71 /* 'q' */ :
	case 0x72 /* 'r' */ :
	case 0x73 /* 's' */ :
	case 0x74 /* 't' */ :
	case 0x75 /* 'u' */ :
	case 0x76 /* 'v' */ :
	case 0x77 /* 'w' */ :
	case 0x78 /* 'x' */ :
	case 0x79 /* 'y' */ :
	case 0x7a /* 'z' */ :
	{
		matchRange('a','z');
		break;
	}
	case 0x41 /* 'A' */ :
	case 0x42 /* 'B' */ :
	case 0x43 /* 'C' */ :
	case 0x44 /* 'D' */ :
	case 0x45 /* 'E' */ :
	case 0x46 /* 'F' */ :
	case 0x47 /* 'G' */ :
	case 0x48 /* 'H' */ :
	case 0x49 /* 'I' */ :
	case 0x4a /* 'J' */ :
	case 0x4b /* 'K' */ :
	case 0x4c /* 'L' */ :
	case 0x4d /* 'M' */ :
	case 0x4e /* 'N' */ :
	case 0x4f /* 'O' */ :
	case 0x50 /* 'P' */ :
	case 0x51 /* 'Q' */ :
	case 0x52 /* 'R' */ :
	case 0x53 /* 'S' */ :
	case 0x54 /* 'T' */ :
	case 0x55 /* 'U' */ :
	case 0x56 /* 'V' */ :
	case 0x57 /* 'W' */ :
	case 0x58 /* 'X' */ :
	case 0x59 /* 'Y' */ :
	case 0x5a /* 'Z' */ :
	{
		matchRange('A','Z');
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mSAFE_LETTER(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = SAFE_LETTER;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	{
	switch ( LA(1)) {
	case 0x61 /* 'a' */ :
	case 0x62 /* 'b' */ :
	case 0x63 /* 'c' */ :
	case 0x64 /* 'd' */ :
	case 0x65 /* 'e' */ :
	case 0x66 /* 'f' */ :
	case 0x67 /* 'g' */ :
	case 0x68 /* 'h' */ :
	case 0x69 /* 'i' */ :
	case 0x6a /* 'j' */ :
	case 0x6b /* 'k' */ :
	case 0x6c /* 'l' */ :
	case 0x6d /* 'm' */ :
	case 0x6e /* 'n' */ :
	case 0x6f /* 'o' */ :
	case 0x70 /* 'p' */ :
	case 0x71 /* 'q' */ :
	case 0x72 /* 'r' */ :
	case 0x73 /* 's' */ :
	case 0x74 /* 't' */ :
	case 0x75 /* 'u' */ :
	case 0x76 /* 'v' */ :
	case 0x77 /* 'w' */ :
	case 0x78 /* 'x' */ :
	case 0x79 /* 'y' */ :
	case 0x7a /* 'z' */ :
	{
		matchRange('a','z');
		break;
	}
	case 0x41 /* 'A' */ :
	case 0x42 /* 'B' */ :
	case 0x43 /* 'C' */ :
	case 0x44 /* 'D' */ :
	case 0x45 /* 'E' */ :
	case 0x46 /* 'F' */ :
	case 0x47 /* 'G' */ :
	case 0x48 /* 'H' */ :
	case 0x49 /* 'I' */ :
	case 0x4a /* 'J' */ :
	case 0x4b /* 'K' */ :
	case 0x4c /* 'L' */ :
	case 0x4d /* 'M' */ :
	case 0x4e /* 'N' */ :
	case 0x4f /* 'O' */ :
	case 0x50 /* 'P' */ :
	case 0x51 /* 'Q' */ :
	case 0x52 /* 'R' */ :
	case 0x53 /* 'S' */ :
	case 0x54 /* 'T' */ :
	case 0x55 /* 'U' */ :
	case 0x56 /* 'V' */ :
	case 0x57 /* 'W' */ :
	case 0x58 /* 'X' */ :
	case 0x59 /* 'Y' */ :
	case 0x5a /* 'Z' */ :
	{
		matchRange('A','Z');
		break;
	}
	case 0x5f /* '_' */ :
	{
		match('_' /* charlit */ );
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mSAFE_CHAR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = SAFE_CHAR;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	{
	switch ( LA(1)) {
	case 0x61 /* 'a' */ :
	case 0x62 /* 'b' */ :
	case 0x63 /* 'c' */ :
	case 0x64 /* 'd' */ :
	case 0x65 /* 'e' */ :
	case 0x66 /* 'f' */ :
	case 0x67 /* 'g' */ :
	case 0x68 /* 'h' */ :
	case 0x69 /* 'i' */ :
	case 0x6a /* 'j' */ :
	case 0x6b /* 'k' */ :
	case 0x6c /* 'l' */ :
	case 0x6d /* 'm' */ :
	case 0x6e /* 'n' */ :
	case 0x6f /* 'o' */ :
	case 0x70 /* 'p' */ :
	case 0x71 /* 'q' */ :
	case 0x72 /* 'r' */ :
	case 0x73 /* 's' */ :
	case 0x74 /* 't' */ :
	case 0x75 /* 'u' */ :
	case 0x76 /* 'v' */ :
	case 0x77 /* 'w' */ :
	case 0x78 /* 'x' */ :
	case 0x79 /* 'y' */ :
	case 0x7a /* 'z' */ :
	{
		matchRange('a','z');
		break;
	}
	case 0x41 /* 'A' */ :
	case 0x42 /* 'B' */ :
	case 0x43 /* 'C' */ :
	case 0x44 /* 'D' */ :
	case 0x45 /* 'E' */ :
	case 0x46 /* 'F' */ :
	case 0x47 /* 'G' */ :
	case 0x48 /* 'H' */ :
	case 0x49 /* 'I' */ :
	case 0x4a /* 'J' */ :
	case 0x4b /* 'K' */ :
	case 0x4c /* 'L' */ :
	case 0x4d /* 'M' */ :
	case 0x4e /* 'N' */ :
	case 0x4f /* 'O' */ :
	case 0x50 /* 'P' */ :
	case 0x51 /* 'Q' */ :
	case 0x52 /* 'R' */ :
	case 0x53 /* 'S' */ :
	case 0x54 /* 'T' */ :
	case 0x55 /* 'U' */ :
	case 0x56 /* 'V' */ :
	case 0x57 /* 'W' */ :
	case 0x58 /* 'X' */ :
	case 0x59 /* 'Y' */ :
	case 0x5a /* 'Z' */ :
	{
		matchRange('A','Z');
		break;
	}
	case 0x30 /* '0' */ :
	case 0x31 /* '1' */ :
	case 0x32 /* '2' */ :
	case 0x33 /* '3' */ :
	case 0x34 /* '4' */ :
	case 0x35 /* '5' */ :
	case 0x36 /* '6' */ :
	case 0x37 /* '7' */ :
	case 0x38 /* '8' */ :
	case 0x39 /* '9' */ :
	{
		matchRange('0','9');
		break;
	}
	case 0x5f /* '_' */ :
	{
		match('_' /* charlit */ );
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mBASESIXFOUR_CHAR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = BASESIXFOUR_CHAR;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	{
	switch ( LA(1)) {
	case 0x61 /* 'a' */ :
	case 0x62 /* 'b' */ :
	case 0x63 /* 'c' */ :
	case 0x64 /* 'd' */ :
	case 0x65 /* 'e' */ :
	case 0x66 /* 'f' */ :
	case 0x67 /* 'g' */ :
	case 0x68 /* 'h' */ :
	case 0x69 /* 'i' */ :
	case 0x6a /* 'j' */ :
	case 0x6b /* 'k' */ :
	case 0x6c /* 'l' */ :
	case 0x6d /* 'm' */ :
	case 0x6e /* 'n' */ :
	case 0x6f /* 'o' */ :
	case 0x70 /* 'p' */ :
	case 0x71 /* 'q' */ :
	case 0x72 /* 'r' */ :
	case 0x73 /* 's' */ :
	case 0x74 /* 't' */ :
	case 0x75 /* 'u' */ :
	case 0x76 /* 'v' */ :
	case 0x77 /* 'w' */ :
	case 0x78 /* 'x' */ :
	case 0x79 /* 'y' */ :
	case 0x7a /* 'z' */ :
	{
		matchRange('a','z');
		break;
	}
	case 0x41 /* 'A' */ :
	case 0x42 /* 'B' */ :
	case 0x43 /* 'C' */ :
	case 0x44 /* 'D' */ :
	case 0x45 /* 'E' */ :
	case 0x46 /* 'F' */ :
	case 0x47 /* 'G' */ :
	case 0x48 /* 'H' */ :
	case 0x49 /* 'I' */ :
	case 0x4a /* 'J' */ :
	case 0x4b /* 'K' */ :
	case 0x4c /* 'L' */ :
	case 0x4d /* 'M' */ :
	case 0x4e /* 'N' */ :
	case 0x4f /* 'O' */ :
	case 0x50 /* 'P' */ :
	case 0x51 /* 'Q' */ :
	case 0x52 /* 'R' */ :
	case 0x53 /* 'S' */ :
	case 0x54 /* 'T' */ :
	case 0x55 /* 'U' */ :
	case 0x56 /* 'V' */ :
	case 0x57 /* 'W' */ :
	case 0x58 /* 'X' */ :
	case 0x59 /* 'Y' */ :
	case 0x5a /* 'Z' */ :
	{
		matchRange('A','Z');
		break;
	}
	case 0x30 /* '0' */ :
	case 0x31 /* '1' */ :
	case 0x32 /* '2' */ :
	case 0x33 /* '3' */ :
	case 0x34 /* '4' */ :
	case 0x35 /* '5' */ :
	case 0x36 /* '6' */ :
	case 0x37 /* '7' */ :
	case 0x38 /* '8' */ :
	case 0x39 /* '9' */ :
	{
		matchRange('0','9');
		break;
	}
	case 0x2b /* '+' */ :
	{
		match('+' /* charlit */ );
		break;
	}
	case 0x2f /* '/' */ :
	{
		match('/' /* charlit */ );
		break;
	}
	case 0x3d /* '=' */ :
	{
		match('=' /* charlit */ );
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mTEXT_TERM(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = TEXT_TERM;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	{ // ( ... )+
	int _cnt36=0;
	for (;;) {
		if (((LA(1) >= 0x80 && LA(1) <= 0xff))) {
			mHIGH_CHAR(false);
		}
		else if ((_tokenSet_2.member(LA(1)))) {
			mSAFE_CHAR(false);
		}
		else {
			if ( _cnt36>=1 ) { goto _loop36; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt36++;
	}
	_loop36:;
	}  // ( ... )+
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mNUMBER(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = NUMBER;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	{ // ( ... )+
	int _cnt39=0;
	for (;;) {
		if (((LA(1) >= 0x30 /* '0' */  && LA(1) <= 0x39 /* '9' */ ))) {
			matchRange('0','9');
		}
		else {
			if ( _cnt39>=1 ) { goto _loop39; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt39++;
	}
	_loop39:;
	}  // ( ... )+
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mFLOAT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = FLOAT;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	{ // ( ... )+
	int _cnt42=0;
	for (;;) {
		if (((LA(1) >= 0x30 /* '0' */  && LA(1) <= 0x39 /* '9' */ ))) {
			matchRange('0','9');
		}
		else {
			if ( _cnt42>=1 ) { goto _loop42; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt42++;
	}
	_loop42:;
	}  // ( ... )+
	mDOT(false);
	{ // ( ... )+
	int _cnt44=0;
	for (;;) {
		if (((LA(1) >= 0x30 /* '0' */  && LA(1) <= 0x39 /* '9' */ ))) {
			matchRange('0','9');
		}
		else {
			if ( _cnt44>=1 ) { goto _loop44; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt44++;
	}
	_loop44:;
	}  // ( ... )+
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mTERM(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = TERM;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	bool synPredMatched52 = false;
	if ((((LA(1) >= 0x30 /* '0' */  && LA(1) <= 0x39 /* '9' */ )) && (_tokenSet_3.member(LA(2))))) {
		int _m52 = mark();
		synPredMatched52 = true;
		inputState->guessing++;
		try {
			{
			mFLOAT(false);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched52 = false;
		}
		rewind(_m52);
		inputState->guessing--;
	}
	if ( synPredMatched52 ) {
		mFLOAT(false);
		if ( inputState->guessing==0 ) {
#line 135 "indrilang.g"
			_ttype = FLOAT;
#line 1063 "QueryLexer.cpp"
		}
	}
	else {
		bool synPredMatched50 = false;
		if (((_tokenSet_0.member(LA(1))) && (true))) {
			int _m50 = mark();
			synPredMatched50 = true;
			inputState->guessing++;
			try {
				{
				{ // ( ... )+
				int _cnt48=0;
				for (;;) {
					if (((LA(1) >= 0x30 /* '0' */  && LA(1) <= 0x39 /* '9' */ ))) {
						mDIGIT(false);
					}
					else {
						if ( _cnt48>=1 ) { goto _loop48; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
					}
					
					_cnt48++;
				}
				_loop48:;
				}  // ( ... )+
				{
				if ((_tokenSet_4.member(LA(1)))) {
					mSAFE_LETTER(false);
				}
				else if (((LA(1) >= 0x80 && LA(1) <= 0xff))) {
					mHIGH_CHAR(false);
				}
				else {
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
				}
				
				}
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched50 = false;
			}
			rewind(_m50);
			inputState->guessing--;
		}
		if ( synPredMatched50 ) {
			mTEXT_TERM(false);
		}
		else {
			bool synPredMatched54 = false;
			if ((((LA(1) >= 0x30 /* '0' */  && LA(1) <= 0x39 /* '9' */ )) && (true))) {
				int _m54 = mark();
				synPredMatched54 = true;
				inputState->guessing++;
				try {
					{
					mNUMBER(false);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched54 = false;
				}
				rewind(_m54);
				inputState->guessing--;
			}
			if ( synPredMatched54 ) {
				mNUMBER(false);
				if ( inputState->guessing==0 ) {
#line 136 "indrilang.g"
					_ttype = NUMBER;
#line 1133 "QueryLexer.cpp"
				}
			}
			else if ((_tokenSet_0.member(LA(1))) && (true)) {
				mTEXT_TERM(false);
			}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mENCODED_QUOTED_TERM(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = ENCODED_QUOTED_TERM;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	_saveIndex = text.length();
	match("#base64quote");
	text.erase(_saveIndex);
	_saveIndex = text.length();
	mO_PAREN(false);
	text.erase(_saveIndex);
	{ // ( ... )*
	for (;;) {
		switch ( LA(1)) {
		case 0x9 /* '\t' */ :
		{
			_saveIndex = text.length();
			mTAB(false);
			text.erase(_saveIndex);
			break;
		}
		case 0x20 /* ' ' */ :
		{
			_saveIndex = text.length();
			mSPACE(false);
			text.erase(_saveIndex);
			break;
		}
		default:
		{
			goto _loop57;
		}
		}
	}
	_loop57:;
	} // ( ... )*
	{ // ( ... )+
	int _cnt59=0;
	for (;;) {
		if ((_tokenSet_5.member(LA(1)))) {
			mBASESIXFOUR_CHAR(false);
		}
		else {
			if ( _cnt59>=1 ) { goto _loop59; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt59++;
	}
	_loop59:;
	}  // ( ... )+
	{ // ( ... )*
	for (;;) {
		switch ( LA(1)) {
		case 0x9 /* '\t' */ :
		{
			_saveIndex = text.length();
			mTAB(false);
			text.erase(_saveIndex);
			break;
		}
		case 0x20 /* ' ' */ :
		{
			_saveIndex = text.length();
			mSPACE(false);
			text.erase(_saveIndex);
			break;
		}
		default:
		{
			goto _loop61;
		}
		}
	}
	_loop61:;
	} // ( ... )*
	_saveIndex = text.length();
	mC_PAREN(false);
	text.erase(_saveIndex);
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mENCODED_TERM(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = ENCODED_TERM;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	_saveIndex = text.length();
	match("#base64");
	text.erase(_saveIndex);
	_saveIndex = text.length();
	mO_PAREN(false);
	text.erase(_saveIndex);
	{ // ( ... )*
	for (;;) {
		switch ( LA(1)) {
		case 0x9 /* '\t' */ :
		{
			_saveIndex = text.length();
			mTAB(false);
			text.erase(_saveIndex);
			break;
		}
		case 0x20 /* ' ' */ :
		{
			_saveIndex = text.length();
			mSPACE(false);
			text.erase(_saveIndex);
			break;
		}
		default:
		{
			goto _loop64;
		}
		}
	}
	_loop64:;
	} // ( ... )*
	{ // ( ... )+
	int _cnt66=0;
	for (;;) {
		if ((_tokenSet_5.member(LA(1)))) {
			mBASESIXFOUR_CHAR(false);
		}
		else {
			if ( _cnt66>=1 ) { goto _loop66; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt66++;
	}
	_loop66:;
	}  // ( ... )+
	{ // ( ... )*
	for (;;) {
		switch ( LA(1)) {
		case 0x9 /* '\t' */ :
		{
			_saveIndex = text.length();
			mTAB(false);
			text.erase(_saveIndex);
			break;
		}
		case 0x20 /* ' ' */ :
		{
			_saveIndex = text.length();
			mSPACE(false);
			text.erase(_saveIndex);
			break;
		}
		default:
		{
			goto _loop68;
		}
		}
	}
	_loop68:;
	} // ( ... )*
	_saveIndex = text.length();
	mC_PAREN(false);
	text.erase(_saveIndex);
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mOPERATOR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = OPERATOR;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	bool synPredMatched71 = false;
	if (((LA(1) == 0x23 /* '#' */ ) && (LA(2) == 0x62 /* 'b' */ ))) {
		int _m71 = mark();
		synPredMatched71 = true;
		inputState->guessing++;
		try {
			{
			match("#base64quote");
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched71 = false;
		}
		rewind(_m71);
		inputState->guessing--;
	}
	if ( synPredMatched71 ) {
		mENCODED_QUOTED_TERM(false);
		if ( inputState->guessing==0 ) {
#line 144 "indrilang.g"
			_ttype = ENCODED_QUOTED_TERM;
#line 1349 "QueryLexer.cpp"
		}
	}
	else {
		bool synPredMatched73 = false;
		if (((LA(1) == 0x23 /* '#' */ ) && (LA(2) == 0x62 /* 'b' */ ))) {
			int _m73 = mark();
			synPredMatched73 = true;
			inputState->guessing++;
			try {
				{
				match("#base64");
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched73 = false;
			}
			rewind(_m73);
			inputState->guessing--;
		}
		if ( synPredMatched73 ) {
			mENCODED_TERM(false);
			if ( inputState->guessing==0 ) {
#line 145 "indrilang.g"
				_ttype = ENCODED_TERM;
#line 1374 "QueryLexer.cpp"
			}
		}
		else {
			bool synPredMatched75 = false;
			if (((LA(1) == 0x23 /* '#' */ ) && (_tokenSet_6.member(LA(2))))) {
				int _m75 = mark();
				synPredMatched75 = true;
				inputState->guessing++;
				try {
					{
					match('#' /* charlit */ );
					mASCII_LETTER(false);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched75 = false;
				}
				rewind(_m75);
				inputState->guessing--;
			}
			if ( synPredMatched75 ) {
				match('#' /* charlit */ );
				{ // ( ... )+
				int _cnt77=0;
				for (;;) {
					if ((_tokenSet_6.member(LA(1)))) {
						mASCII_LETTER(false);
					}
					else {
						if ( _cnt77>=1 ) { goto _loop77; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
					}
					
					_cnt77++;
				}
				_loop77:;
				}  // ( ... )+
			}
			else if ((LA(1) == 0x23 /* '#' */ ) && (true)) {
				match('#' /* charlit */ );
			}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}}
	_ttype = testLiteralsTable(_ttype);
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void QueryLexer::mJUNK(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; ANTLR_USE_NAMESPACE(std)string::size_type _begin = text.length();
	_ttype = JUNK;
	ANTLR_USE_NAMESPACE(std)string::size_type _saveIndex;
	
	{
	switch ( LA(1)) {
	case 0x9 /* '\t' */ :
	{
		mTAB(false);
		break;
	}
	case 0xa /* '\n' */ :
	{
		mCR(false);
		break;
	}
	case 0xd /* '\r' */ :
	{
		mLF(false);
		break;
	}
	case 0x20 /* ' ' */ :
	{
		mSPACE(false);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}
	}
	if ( inputState->guessing==0 ) {
#line 150 "indrilang.g"
		_ttype = antlr::Token::SKIP;
#line 1464 "QueryLexer.cpp"
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}


const unsigned long QueryLexer::_tokenSet_0_data_[] = { 0UL, 67043328UL, 2281701374UL, 134217726UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// 0 1 2 3 4 5 6 7 8 9 A B C D 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryLexer::_tokenSet_0(_tokenSet_0_data_,24);
const unsigned long QueryLexer::_tokenSet_1_data_[] = { 9728UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// 0x9 0xa 0xd   
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryLexer::_tokenSet_1(_tokenSet_1_data_,10);
const unsigned long QueryLexer::_tokenSet_2_data_[] = { 0UL, 67043328UL, 2281701374UL, 134217726UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// 0 1 2 3 4 5 6 7 8 9 A B C D 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryLexer::_tokenSet_2(_tokenSet_2_data_,10);
const unsigned long QueryLexer::_tokenSet_3_data_[] = { 0UL, 67059712UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// . 0 1 2 3 4 5 6 7 8 9 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryLexer::_tokenSet_3(_tokenSet_3_data_,10);
const unsigned long QueryLexer::_tokenSet_4_data_[] = { 0UL, 0UL, 2281701374UL, 134217726UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// A B C D 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryLexer::_tokenSet_4(_tokenSet_4_data_,10);
const unsigned long QueryLexer::_tokenSet_5_data_[] = { 0UL, 603949056UL, 134217726UL, 134217726UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// + / 0 1 2 3 4 5 6 7 8 9 = A B C D 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryLexer::_tokenSet_5(_tokenSet_5_data_,10);
const unsigned long QueryLexer::_tokenSet_6_data_[] = { 0UL, 0UL, 134217726UL, 134217726UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// A B C D 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryLexer::_tokenSet_6(_tokenSet_6_data_,10);

ANTLR_END_NAMESPACE
ANTLR_END_NAMESPACE
