#ifndef INC_QueryLexerTokenTypes_hpp_
#define INC_QueryLexerTokenTypes_hpp_

ANTLR_BEGIN_NAMESPACE(indri)
ANTLR_BEGIN_NAMESPACE(lang)
/* $ANTLR 2.7.7 (2006-11-01): "indrilang.g" -> "QueryLexerTokenTypes.hpp"$ */

#ifndef CUSTOM_API
# define CUSTOM_API
#endif

#ifdef __cplusplus
struct CUSTOM_API QueryLexerTokenTypes {
#endif
	enum {
		EOF_ = 1,
		SUM = 4,
		WSUM = 5,
		WAND = 6,
		OD = 7,
		OR = 8,
		NOT = 9,
		UW = 10,
		COMBINE = 11,
		WEIGHT = 12,
		MAX = 13,
		FILREQ = 14,
		FILREJ = 15,
		SCOREIF = 16,
		SCOREIFNOT = 17,
		ANY = 18,
		BAND = 19,
		WSYN = 20,
		SYN = 21,
		PRIOR = 22,
		DATEAFTER = 23,
		DATEBEFORE = 24,
		DATEBETWEEN = 25,
		DATEEQUALS = 26,
		LESS = 27,
		GREATER = 28,
		BETWEEN = 29,
		EQUALS = 30,
		WCARD = 31,
		NUMBER = 32,
		NEGATIVE_NUMBER = 33,
		FLOAT = 34,
		STAR = 35,
		O_PAREN = 36,
		C_PAREN = 37,
		O_ANGLE = 38,
		C_ANGLE = 39,
		O_SQUARE = 40,
		C_SQUARE = 41,
		O_BRACE = 42,
		C_BRACE = 43,
		DBL_QUOTE = 44,
		QUOTE = 45,
		DOT = 46,
		COMMA = 47,
		SLASH = 48,
		B_SLASH = 49,
		DASH = 50,
		SPACE_DASH = 51,
		COLON = 52,
		TAB = 53,
		CR = 54,
		LF = 55,
		SPACE = 56,
		HIGH_CHAR = 57,
		DIGIT = 58,
		ASCII_LETTER = 59,
		SAFE_LETTER = 60,
		SAFE_CHAR = 61,
		BASESIXFOUR_CHAR = 62,
		TEXT_TERM = 63,
		TERM = 64,
		ENCODED_QUOTED_TERM = 65,
		ENCODED_TERM = 66,
		OPERATOR = 67,
		JUNK = 68,
		NULL_TREE_LOOKAHEAD = 3
	};
#ifdef __cplusplus
};
#endif
ANTLR_END_NAMESPACE
ANTLR_END_NAMESPACE
#endif /*INC_QueryLexerTokenTypes_hpp_*/
