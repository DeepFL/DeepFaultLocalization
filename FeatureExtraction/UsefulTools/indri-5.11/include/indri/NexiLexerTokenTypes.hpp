#ifndef INC_NexiLexerTokenTypes_hpp_
#define INC_NexiLexerTokenTypes_hpp_

ANTLR_BEGIN_NAMESPACE(indri)
ANTLR_BEGIN_NAMESPACE(lang)
/* $ANTLR 2.7.3 (20060307-1): "nexilang.g" -> "NexiLexerTokenTypes.hpp"$ */

#ifndef CUSTOM_API
# define CUSTOM_API
#endif

#ifdef __cplusplus
struct CUSTOM_API NexiLexerTokenTypes {
#endif
	enum {
		EOF_ = 1,
		ABOUT = 4,
		AND = 5,
		OR = 6,
		WILD = 7,
		NUMBER = 8,
		NEGATIVE_NUMBER = 9,
		FLOAT = 10,
		STAR = 11,
		O_PAREN = 12,
		C_PAREN = 13,
		O_SQUARE = 14,
		C_SQUARE = 15,
		DBL_QUOTE = 16,
		QUOTE = 17,
		DOT = 18,
		COMMA = 19,
		SLASH = 20,
		MINUS = 21,
		PLUS = 22,
		TAB = 23,
		CR = 24,
		LF = 25,
		SPACE = 26,
		HIGH_CHAR = 27,
		DIGIT = 28,
		ASCII_LETTER = 29,
		SAFE_LETTER = 30,
		SAFE_CHAR = 31,
		TEXT_TERM = 32,
		TERM = 33,
		OPERATORS = 34,
		LESS = 35,
		GREATER = 36,
		LESSEQ = 37,
		GREATEREQ = 38,
		EQUALS = 39,
		JUNK = 40,
		// "|" = 41
		// "." = 42
		// "," = 43
		NULL_TREE_LOOKAHEAD = 3
	};
#ifdef __cplusplus
};
#endif
ANTLR_END_NAMESPACE
ANTLR_END_NAMESPACE
#endif /*INC_NexiLexerTokenTypes_hpp_*/
