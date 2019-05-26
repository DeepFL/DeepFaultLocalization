/* $ANTLR 2.7.7 (2006-11-01): "indrilang.g" -> "QueryParser.cpp"$ */
#include "indri/QueryParser.hpp"
#include <antlr/NoViableAltException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/ASTFactory.hpp>
ANTLR_BEGIN_NAMESPACE(indri)
ANTLR_BEGIN_NAMESPACE(lang)
#line 1 "indrilang.g"
#line 8 "QueryParser.cpp"
QueryParser::QueryParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,k)
{
}

QueryParser::QueryParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,2)
{
}

QueryParser::QueryParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,k)
{
}

QueryParser::QueryParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,2)
{
}

QueryParser::QueryParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(state,2)
{
}

 indri::lang::ScoredExtentNode*  QueryParser::query() {
#line 188 "indrilang.g"
	 indri::lang::ScoredExtentNode* q ;
#line 37 "QueryParser.cpp"
#line 188 "indrilang.g"
	
	indri::lang::CombineNode* c = 0;
	indri::lang::ScoredExtentNode* s = 0;
	q = 0;
	
#line 44 "QueryParser.cpp"
	
	q=scoredExtentNode(0);
	{
	switch ( LA(1)) {
	case WSUM:
	case WAND:
	case OD:
	case OR:
	case NOT:
	case UW:
	case COMBINE:
	case WEIGHT:
	case MAX:
	case FILREQ:
	case FILREJ:
	case SCOREIF:
	case SCOREIFNOT:
	case ANY:
	case BAND:
	case WSYN:
	case SYN:
	case PRIOR:
	case DATEAFTER:
	case DATEBEFORE:
	case DATEBETWEEN:
	case DATEEQUALS:
	case LESS:
	case GREATER:
	case BETWEEN:
	case EQUALS:
	case WCARD:
	case NUMBER:
	case FLOAT:
	case O_ANGLE:
	case O_BRACE:
	case DBL_QUOTE:
	case DASH:
	case SPACE_DASH:
	case TERM:
	case ENCODED_QUOTED_TERM:
	case ENCODED_TERM:
	case OPERATOR:
	{
		s=scoredExtentNode(0);
		if ( inputState->guessing==0 ) {
#line 195 "indrilang.g"
			
			c = new CombineNode;
			c->addChild(q);
			c->addChild(s);
			_nodes.push_back(c);
			q = c;
			
#line 98 "QueryParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_0.member(LA(1)))) {
				s=scoredExtentNode(0);
				if ( inputState->guessing==0 ) {
#line 203 "indrilang.g"
					
					c->addChild(s);
					
#line 109 "QueryParser.cpp"
				}
			}
			else {
				goto _loop83;
			}
			
		}
		_loop83:;
		} // ( ... )*
		break;
	}
	case ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE:
	{
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	match(ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE);
	return q ;
}

 indri::lang::ScoredExtentNode*  QueryParser::scoredExtentNode(
	 indri::lang::RawExtentNode * ou 
) {
#line 209 "indrilang.g"
	 indri::lang::ScoredExtentNode* s ;
#line 140 "QueryParser.cpp"
	
	switch ( LA(1)) {
	case WEIGHT:
	{
		s=weightNode(ou);
		break;
	}
	case COMBINE:
	{
		s=combineNode(ou);
		break;
	}
	case OR:
	{
		s=orNode(ou);
		break;
	}
	case NOT:
	{
		s=notNode(ou);
		break;
	}
	case WAND:
	{
		s=wandNode(ou);
		break;
	}
	case WSUM:
	{
		s=wsumNode(ou);
		break;
	}
	case MAX:
	{
		s=maxNode(ou);
		break;
	}
	case PRIOR:
	{
		s=priorNode();
		break;
	}
	case FILREJ:
	{
		s=filrejNode(ou);
		break;
	}
	case FILREQ:
	{
		s=filreqNode(ou);
		break;
	}
	case SCOREIF:
	{
		s=scoreifNode(ou);
		break;
	}
	case SCOREIFNOT:
	{
		s=scoreifnotNode(ou);
		break;
	}
	case OD:
	case UW:
	case ANY:
	case BAND:
	case WSYN:
	case SYN:
	case DATEAFTER:
	case DATEBEFORE:
	case DATEBETWEEN:
	case DATEEQUALS:
	case LESS:
	case GREATER:
	case BETWEEN:
	case EQUALS:
	case WCARD:
	case NUMBER:
	case FLOAT:
	case O_ANGLE:
	case O_BRACE:
	case DBL_QUOTE:
	case DASH:
	case SPACE_DASH:
	case TERM:
	case ENCODED_QUOTED_TERM:
	case ENCODED_TERM:
	case OPERATOR:
	{
		s=scoredRaw(ou);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return s ;
}

 indri::lang::ScoredExtentNode*  QueryParser::weightNode(
	indri::lang::RawExtentNode * ou 
) {
#line 302 "indrilang.g"
	 indri::lang::ScoredExtentNode* r ;
#line 246 "QueryParser.cpp"
#line 302 "indrilang.g"
	
	indri::lang::WeightNode* wn = new indri::lang::WeightNode;
	_nodes.push_back(wn);
	
#line 252 "QueryParser.cpp"
	
	match(WEIGHT);
	r=weightedList(wn, ou);
	return r ;
}

 indri::lang::ScoredExtentNode*  QueryParser::combineNode(
	indri::lang::RawExtentNode * ou 
) {
#line 309 "indrilang.g"
	 indri::lang::ScoredExtentNode* r ;
#line 264 "QueryParser.cpp"
#line 309 "indrilang.g"
	
	indri::lang::CombineNode* cn = new indri::lang::CombineNode;
	_nodes.push_back(cn);
	
#line 270 "QueryParser.cpp"
	
	match(COMBINE);
	r=unweightedList(cn, ou);
	return r ;
}

 indri::lang::ScoredExtentNode*  QueryParser::orNode(
	indri::lang::RawExtentNode * ou 
) {
#line 337 "indrilang.g"
	 indri::lang::ScoredExtentNode* r ;
#line 282 "QueryParser.cpp"
#line 337 "indrilang.g"
	
	indri::lang::OrNode* on = new indri::lang::OrNode;
	_nodes.push_back(on);
	
#line 288 "QueryParser.cpp"
	
	match(OR);
	r=unweightedList(on, ou);
	return r ;
}

 indri::lang::ScoredExtentNode*  QueryParser::notNode(
	indri::lang::RawExtentNode * ou 
) {
#line 351 "indrilang.g"
	 indri::lang::ScoredExtentNode* r ;
#line 300 "QueryParser.cpp"
#line 351 "indrilang.g"
	
	indri::lang::NotNode* n = new indri::lang::NotNode;
	indri::lang::ScoredExtentNode* c = 0;
	_nodes.push_back(n);
	r = n;
	
#line 308 "QueryParser.cpp"
	
	match(NOT);
	{
	switch ( LA(1)) {
	case O_SQUARE:
	{
		r=extentRestriction(r, ou);
		break;
	}
	case O_PAREN:
	{
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	match(O_PAREN);
	c=scoredExtentNode(ou);
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 359 "indrilang.g"
		
		n->setChild(c);
		
#line 336 "QueryParser.cpp"
	}
	return r ;
}

 indri::lang::ScoredExtentNode*  QueryParser::wandNode(
	indri::lang::RawExtentNode * ou 
) {
#line 330 "indrilang.g"
	 indri::lang::ScoredExtentNode* r ;
#line 346 "QueryParser.cpp"
#line 330 "indrilang.g"
	
	indri::lang::WAndNode* wn = new indri::lang::WAndNode;
	_nodes.push_back(wn);
	
#line 352 "QueryParser.cpp"
	
	match(WAND);
	r=weightedList(wn, ou);
	return r ;
}

 indri::lang::ScoredExtentNode*  QueryParser::wsumNode(
	indri::lang::RawExtentNode * ou 
) {
#line 323 "indrilang.g"
	 indri::lang::ScoredExtentNode* r ;
#line 364 "QueryParser.cpp"
#line 323 "indrilang.g"
	
	indri::lang::WSumNode* wn = new indri::lang::WSumNode;
	_nodes.push_back(wn);
	
#line 370 "QueryParser.cpp"
	
	match(WSUM);
	r=weightedList(wn, ou);
	return r ;
}

 indri::lang::ScoredExtentNode*  QueryParser::maxNode(
	indri::lang::RawExtentNode * ou 
) {
#line 344 "indrilang.g"
	 indri::lang::ScoredExtentNode* r ;
#line 382 "QueryParser.cpp"
#line 344 "indrilang.g"
	
	indri::lang::MaxNode* mn = new indri::lang::MaxNode;
	_nodes.push_back(mn);
	
#line 388 "QueryParser.cpp"
	
	match(MAX);
	r=unweightedList(mn, ou);
	return r ;
}

 indri::lang::PriorNode*  QueryParser::priorNode() {
#line 363 "indrilang.g"
	 indri::lang::PriorNode* p ;
#line 398 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  name = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 363 "indrilang.g"
	
	indri::lang::Field* field = 0;
	p = 0;
	
#line 405 "QueryParser.cpp"
	
	match(PRIOR);
	match(O_PAREN);
	name = LT(1);
	match(TERM);
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 368 "indrilang.g"
		
		p = new indri::lang::PriorNode( name->getText() );
		_nodes.push_back(p);
		
#line 418 "QueryParser.cpp"
	}
	return p ;
}

 indri::lang::FilRejNode*  QueryParser::filrejNode(
	 indri::lang::RawExtentNode * ou 
) {
#line 446 "indrilang.g"
	 indri::lang::FilRejNode* fj ;
#line 428 "QueryParser.cpp"
#line 446 "indrilang.g"
	
	RawExtentNode* filter = 0;
	ScoredExtentNode* disallowed = 0;
	
#line 434 "QueryParser.cpp"
	
	match(FILREJ);
	match(O_PAREN);
	filter=unscoredTerm();
	disallowed=scoredExtentNode(ou);
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 452 "indrilang.g"
		
		fj = new FilRejNode( filter, disallowed );
		_nodes.push_back(fj);
		
#line 447 "QueryParser.cpp"
	}
	return fj ;
}

 indri::lang::FilReqNode*  QueryParser::filreqNode(
	 indri::lang::RawExtentNode * ou 
) {
#line 457 "indrilang.g"
	 indri::lang::FilReqNode* fq ;
#line 457 "QueryParser.cpp"
#line 457 "indrilang.g"
	
	RawExtentNode* filter = 0;
	ScoredExtentNode* required = 0;
	
#line 463 "QueryParser.cpp"
	
	match(FILREQ);
	match(O_PAREN);
	filter=unscoredTerm();
	required=scoredExtentNode(ou);
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 463 "indrilang.g"
		
		fq = new FilReqNode( filter, required );
		_nodes.push_back(fq);
		
#line 476 "QueryParser.cpp"
	}
	return fq ;
}

 indri::lang::FilReqNode*  QueryParser::scoreifNode(
	 indri::lang::RawExtentNode * ou 
) {
#line 479 "indrilang.g"
	 indri::lang::FilReqNode* fq ;
#line 486 "QueryParser.cpp"
#line 479 "indrilang.g"
	
	RawExtentNode* filter = 0;
	ScoredExtentNode* required = 0;
	
#line 492 "QueryParser.cpp"
	
	match(SCOREIF);
	match(O_PAREN);
	filter=unscoredTerm();
	required=scoredExtentNode(ou);
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 485 "indrilang.g"
		
		fq = new FilReqNode( filter, required );
		_nodes.push_back(fq);
		
#line 505 "QueryParser.cpp"
	}
	return fq ;
}

 indri::lang::FilRejNode*  QueryParser::scoreifnotNode(
	 indri::lang::RawExtentNode * ou 
) {
#line 468 "indrilang.g"
	 indri::lang::FilRejNode* fj ;
#line 515 "QueryParser.cpp"
#line 468 "indrilang.g"
	
	RawExtentNode* filter = 0;
	ScoredExtentNode* disallowed = 0;
	
#line 521 "QueryParser.cpp"
	
	match(SCOREIFNOT);
	match(O_PAREN);
	filter=unscoredTerm();
	disallowed=scoredExtentNode(ou);
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 474 "indrilang.g"
		
		fj = new FilRejNode( filter, disallowed );
		_nodes.push_back(fj);
		
#line 534 "QueryParser.cpp"
	}
	return fj ;
}

 indri::lang::ScoredExtentNode*  QueryParser::scoredRaw(
	 indri::lang::RawExtentNode * ou 
) {
#line 225 "indrilang.g"
	 indri::lang::ScoredExtentNode* sn ;
#line 544 "QueryParser.cpp"
#line 225 "indrilang.g"
	
	RawExtentNode* raw = 0;
	RawExtentNode* contexts = 0;
	sn = 0;
	
#line 551 "QueryParser.cpp"
	
	bool synPredMatched111 = false;
	if (((_tokenSet_1.member(LA(1))) && (_tokenSet_2.member(LA(2))))) {
		int _m111 = mark();
		synPredMatched111 = true;
		inputState->guessing++;
		try {
			{
			qualifiedTerm();
			match(DOT);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched111 = false;
		}
		rewind(_m111);
		inputState->guessing--;
	}
	if ( synPredMatched111 ) {
		raw=qualifiedTerm();
		match(DOT);
		contexts=context_list(ou);
		if ( inputState->guessing==0 ) {
#line 232 "indrilang.g"
			
			sn = new indri::lang::RawScorerNode( raw, contexts );
			_nodes.push_back(sn);
			
#line 580 "QueryParser.cpp"
		}
	}
	else {
		bool synPredMatched113 = false;
		if (((_tokenSet_1.member(LA(1))) && (_tokenSet_2.member(LA(2))))) {
			int _m113 = mark();
			synPredMatched113 = true;
			inputState->guessing++;
			try {
				{
				qualifiedTerm();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched113 = false;
			}
			rewind(_m113);
			inputState->guessing--;
		}
		if ( synPredMatched113 ) {
			raw=qualifiedTerm();
			if ( inputState->guessing==0 ) {
#line 237 "indrilang.g"
				
				sn = new indri::lang::RawScorerNode( raw, contexts );
				_nodes.push_back(sn);
				
#line 608 "QueryParser.cpp"
			}
		}
		else {
			bool synPredMatched115 = false;
			if (((_tokenSet_1.member(LA(1))) && (_tokenSet_2.member(LA(2))))) {
				int _m115 = mark();
				synPredMatched115 = true;
				inputState->guessing++;
				try {
					{
					unqualifiedTerm();
					match(DOT);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched115 = false;
				}
				rewind(_m115);
				inputState->guessing--;
			}
			if ( synPredMatched115 ) {
				raw=unqualifiedTerm();
				match(DOT);
				contexts=context_list(ou);
				if ( inputState->guessing==0 ) {
#line 242 "indrilang.g"
					
					sn = new indri::lang::RawScorerNode( raw, contexts );
					_nodes.push_back(sn);
					
#line 639 "QueryParser.cpp"
				}
			}
			else if ((_tokenSet_1.member(LA(1))) && (_tokenSet_3.member(LA(2)))) {
				raw=unqualifiedTerm();
				if ( inputState->guessing==0 ) {
#line 247 "indrilang.g"
					
					sn = new indri::lang::RawScorerNode( raw, contexts );
					_nodes.push_back(sn);
					
#line 650 "QueryParser.cpp"
				}
			}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}}
	return sn ;
}

 RawExtentNode*  QueryParser::qualifiedTerm() {
#line 511 "indrilang.g"
	 RawExtentNode* t ;
#line 663 "QueryParser.cpp"
#line 511 "indrilang.g"
	
	RawExtentNode* synonyms = 0;
	RawExtentNode* fields = 0;
	t = 0;
	
#line 670 "QueryParser.cpp"
	
	synonyms=unqualifiedTerm();
	match(DOT);
	fields=field_list();
	if ( inputState->guessing==0 ) {
#line 518 "indrilang.g"
		
		if( fields ) {
		t = new indri::lang::ExtentInside( synonyms, fields );
		_nodes.push_back(t);
		synonyms = t;
		} else {
		t = synonyms;
		}
		
#line 686 "QueryParser.cpp"
	}
	return t ;
}

 ExtentOr*  QueryParser::context_list(
	 indri::lang::RawExtentNode * ou 
) {
#line 794 "indrilang.g"
	 ExtentOr* contexts ;
#line 696 "QueryParser.cpp"
#line 794 "indrilang.g"
	
	contexts = new ExtentOr;
	_nodes.push_back( contexts );
	indri::lang::ExtentInside * p = 0;
	indri::lang::ExtentInside * pAdditional = 0;
	std::string first, additional;
	
#line 705 "QueryParser.cpp"
	
	match(O_PAREN);
	{
	switch ( LA(1)) {
	case TERM:
	{
		first=fieldNameString();
		if ( inputState->guessing==0 ) {
#line 803 "indrilang.g"
			
			Field* firstField = new indri::lang::Field( first );
			_nodes.push_back( firstField );
			contexts->addChild( firstField );
			
#line 720 "QueryParser.cpp"
		}
		break;
	}
	case DOT:
	{
		{
		match(DOT);
		p=path();
		if ( inputState->guessing==0 ) {
#line 809 "indrilang.g"
			
			p->setOuter( ou );
			contexts->addChild( p );
			
#line 735 "QueryParser.cpp"
		}
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == COMMA)) {
			match(COMMA);
			{
			switch ( LA(1)) {
			case TERM:
			{
				additional=fieldNameString();
				if ( inputState->guessing==0 ) {
#line 815 "indrilang.g"
					
					Field* additionalField = new Field(additional);
					_nodes.push_back( additionalField );
					contexts->addChild( additionalField );
					
#line 762 "QueryParser.cpp"
				}
				break;
			}
			case DOT:
			{
				{
				match(DOT);
				pAdditional=path();
				if ( inputState->guessing==0 ) {
#line 820 "indrilang.g"
					
					pAdditional->setOuter( ou );
					contexts->addChild( pAdditional );
					
#line 777 "QueryParser.cpp"
				}
				}
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		else {
			goto _loop273;
		}
		
	}
	_loop273:;
	} // ( ... )*
	match(C_PAREN);
	return contexts ;
}

 indri::lang::RawExtentNode*  QueryParser::unqualifiedTerm() {
#line 528 "indrilang.g"
	 indri::lang::RawExtentNode* re ;
#line 803 "QueryParser.cpp"
#line 528 "indrilang.g"
	
	re = 0;
	indri::lang::IndexTerm* t = 0;
	
#line 809 "QueryParser.cpp"
	
	switch ( LA(1)) {
	case OD:
	case OPERATOR:
	{
		re=odNode();
		break;
	}
	case UW:
	{
		re=uwNode();
		break;
	}
	case BAND:
	{
		re=bandNode();
		break;
	}
	case DATEBEFORE:
	{
		re=dateBefore();
		break;
	}
	case DATEAFTER:
	{
		re=dateAfter();
		break;
	}
	case DATEBETWEEN:
	{
		re=dateBetween();
		break;
	}
	case DATEEQUALS:
	{
		re=dateEquals();
		break;
	}
	case O_ANGLE:
	{
		re=synonym_list();
		break;
	}
	case O_BRACE:
	{
		re=synonym_list_brace();
		break;
	}
	case SYN:
	{
		re=synonym_list_alt();
		break;
	}
	case WSYN:
	{
		re=wsynNode();
		break;
	}
	case ANY:
	{
		re=anyField();
		break;
	}
	case LESS:
	{
		re=lessNode();
		break;
	}
	case GREATER:
	{
		re=greaterNode();
		break;
	}
	case BETWEEN:
	{
		re=betweenNode();
		break;
	}
	case EQUALS:
	{
		re=equalsNode();
		break;
	}
	case WCARD:
	{
		re=wildcardOpNode();
		break;
	}
	default:
		bool synPredMatched210 = false;
		if (((_tokenSet_4.member(LA(1))) && (_tokenSet_5.member(LA(2))))) {
			int _m210 = mark();
			synPredMatched210 = true;
			inputState->guessing++;
			try {
				{
				match(TERM);
				match(SPACE_DASH);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched210 = false;
			}
			rewind(_m210);
			inputState->guessing--;
		}
		if ( synPredMatched210 ) {
			re=rawText();
		}
		else {
			bool synPredMatched212 = false;
			if (((LA(1) == NUMBER || LA(1) == TERM) && (LA(2) == DASH))) {
				int _m212 = mark();
				synPredMatched212 = true;
				inputState->guessing++;
				try {
					{
					match(TERM);
					match(DASH);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched212 = false;
				}
				rewind(_m212);
				inputState->guessing--;
			}
			if ( synPredMatched212 ) {
				re=hyphenTerm();
			}
			else {
				bool synPredMatched214 = false;
				if (((LA(1) == NUMBER || LA(1) == TERM) && (LA(2) == DASH))) {
					int _m214 = mark();
					synPredMatched214 = true;
					inputState->guessing++;
					try {
						{
						match(NUMBER);
						match(DASH);
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched214 = false;
					}
					rewind(_m214);
					inputState->guessing--;
				}
				if ( synPredMatched214 ) {
					re=hyphenTerm();
				}
				else {
					bool synPredMatched216 = false;
					if (((_tokenSet_4.member(LA(1))) && (_tokenSet_5.member(LA(2))))) {
						int _m216 = mark();
						synPredMatched216 = true;
						inputState->guessing++;
						try {
							{
							match(SPACE_DASH);
							}
						}
						catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
							synPredMatched216 = false;
						}
						rewind(_m216);
						inputState->guessing--;
					}
					if ( synPredMatched216 ) {
						re=rawText();
					}
					else {
						bool synPredMatched218 = false;
						if (((_tokenSet_4.member(LA(1))) && (_tokenSet_5.member(LA(2))))) {
							int _m218 = mark();
							synPredMatched218 = true;
							inputState->guessing++;
							try {
								{
								match(DASH);
								}
							}
							catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
								synPredMatched218 = false;
							}
							rewind(_m218);
							inputState->guessing--;
						}
						if ( synPredMatched218 ) {
							re=rawText();
						}
						else {
							bool synPredMatched222 = false;
							if (((_tokenSet_4.member(LA(1))) && (_tokenSet_6.member(LA(2))))) {
								int _m222 = mark();
								synPredMatched222 = true;
								inputState->guessing++;
								try {
									{
									match(TERM);
									match(STAR);
									}
								}
								catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
									synPredMatched222 = false;
								}
								rewind(_m222);
								inputState->guessing--;
							}
							if ( synPredMatched222 ) {
								t=rawText();
								match(STAR);
								if ( inputState->guessing==0 ) {
#line 554 "indrilang.g"
									
									// wildcard support as an unqualified term
									// i.e. "term*"
									re=new indri::lang::WildcardTerm( t->getText() );
									_nodes.push_back(re);
									
#line 1030 "QueryParser.cpp"
								}
							}
							else {
								bool synPredMatched224 = false;
								if (((_tokenSet_4.member(LA(1))) && (_tokenSet_6.member(LA(2))))) {
									int _m224 = mark();
									synPredMatched224 = true;
									inputState->guessing++;
									try {
										{
										match(NUMBER);
										match(STAR);
										}
									}
									catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
										synPredMatched224 = false;
									}
									rewind(_m224);
									inputState->guessing--;
								}
								if ( synPredMatched224 ) {
									t=rawText();
									match(STAR);
									if ( inputState->guessing==0 ) {
#line 560 "indrilang.g"
										
										// wildcard support as an unqualified term
										// i.e. "term*"
										re=new indri::lang::WildcardTerm( t->getText() );
										_nodes.push_back(re);
										
#line 1062 "QueryParser.cpp"
									}
								}
								else {
									bool synPredMatched226 = false;
									if (((_tokenSet_4.member(LA(1))) && (_tokenSet_6.member(LA(2))))) {
										int _m226 = mark();
										synPredMatched226 = true;
										inputState->guessing++;
										try {
											{
											match(FLOAT);
											match(STAR);
											}
										}
										catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
											synPredMatched226 = false;
										}
										rewind(_m226);
										inputState->guessing--;
									}
									if ( synPredMatched226 ) {
										t=rawText();
										match(STAR);
										if ( inputState->guessing==0 ) {
#line 566 "indrilang.g"
											
											// wildcard support as an unqualified term
											// i.e. "term*"
											re=new indri::lang::WildcardTerm( t->getText() );
											_nodes.push_back(re);
											
#line 1094 "QueryParser.cpp"
										}
									}
									else {
										bool synPredMatched228 = false;
										if (((_tokenSet_4.member(LA(1))) && (_tokenSet_6.member(LA(2))))) {
											int _m228 = mark();
											synPredMatched228 = true;
											inputState->guessing++;
											try {
												{
												match(NEGATIVE_NUMBER);
												match(STAR);
												}
											}
											catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
												synPredMatched228 = false;
											}
											rewind(_m228);
											inputState->guessing--;
										}
										if ( synPredMatched228 ) {
											t=rawText();
											match(STAR);
											if ( inputState->guessing==0 ) {
#line 572 "indrilang.g"
												
												// wildcard support as an unqualified term
												// i.e. "term*"
												re=new indri::lang::WildcardTerm( t->getText() );
												_nodes.push_back(re);
												
#line 1126 "QueryParser.cpp"
											}
										}
										else {
											bool synPredMatched230 = false;
											if (((_tokenSet_4.member(LA(1))) && (_tokenSet_6.member(LA(2))))) {
												int _m230 = mark();
												synPredMatched230 = true;
												inputState->guessing++;
												try {
													{
													match(TEXT_TERM);
													match(STAR);
													}
												}
												catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
													synPredMatched230 = false;
												}
												rewind(_m230);
												inputState->guessing--;
											}
											if ( synPredMatched230 ) {
												t=rawText();
												match(STAR);
												if ( inputState->guessing==0 ) {
#line 578 "indrilang.g"
													
													// wildcard support as an unqualified term
													// i.e. "term*"
													re=new indri::lang::WildcardTerm( t->getText() );
													_nodes.push_back(re);
													
#line 1158 "QueryParser.cpp"
												}
											}
											else if ((_tokenSet_4.member(LA(1))) && (_tokenSet_5.member(LA(2)))) {
												re=rawText();
											}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}}}}}}}}}}
	return re ;
}

 indri::lang::ScoredExtentNode*  QueryParser::weightedList(
	 indri::lang::WeightedCombinationNode* wn, indri::lang::RawExtentNode * ou 
) {
#line 263 "indrilang.g"
	 indri::lang::ScoredExtentNode* sr ;
#line 1176 "QueryParser.cpp"
#line 263 "indrilang.g"
	
	double w = 0;
	ScoredExtentNode* n = 0;
	sr = wn;
	
#line 1183 "QueryParser.cpp"
	
	{
	switch ( LA(1)) {
	case O_SQUARE:
	{
		sr=extentRestriction(wn, ou);
		if ( inputState->guessing==0 ) {
#line 269 "indrilang.g"
			ou = innerMost(sr);
#line 1193 "QueryParser.cpp"
		}
		break;
	}
	case O_PAREN:
	{
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	match(O_PAREN);
	{ // ( ... )+
	int _cnt119=0;
	for (;;) {
		if ((_tokenSet_7.member(LA(1)))) {
			w=floating();
			n=scoredExtentNode(ou);
			if ( inputState->guessing==0 ) {
#line 274 "indrilang.g"
				wn->addChild( w, n );
#line 1217 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt119>=1 ) { goto _loop119; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt119++;
	}
	_loop119:;
	}  // ( ... )+
	match(C_PAREN);
	return sr ;
}

 indri::lang::ScoredExtentNode*  QueryParser::extentRestriction(
	 indri::lang::ScoredExtentNode* sn, indri::lang::RawExtentNode * ou 
) {
#line 650 "indrilang.g"
	 indri::lang::ScoredExtentNode* er ;
#line 1237 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  passageWindowSize = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  inc = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 650 "indrilang.g"
	
	indri::lang::Field* f = 0;
	std::string fName;
	er = 0;
	indri::lang::ExtentInside * po = 0;
	
#line 1247 "QueryParser.cpp"
	
	bool synPredMatched247 = false;
	if (((LA(1) == O_SQUARE) && (LA(2) == TERM))) {
		int _m247 = mark();
		synPredMatched247 = true;
		inputState->guessing++;
		try {
			{
			match(O_SQUARE);
			match(TERM);
			match(COLON);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched247 = false;
		}
		rewind(_m247);
		inputState->guessing--;
	}
	if ( synPredMatched247 ) {
		match(O_SQUARE);
		passageWindowSize = LT(1);
		match(TERM);
		match(COLON);
		inc = LT(1);
		match(NUMBER);
		match(C_SQUARE);
		if ( inputState->guessing==0 ) {
#line 657 "indrilang.g"
			
			int startWindow;
			
			for( startWindow = 0; startWindow < passageWindowSize->getText().size(); startWindow++ ) {
			if( isdigit((unsigned char) passageWindowSize->getText()[startWindow] ) )
			break;
			}
			
			int increment = atoi(inc->getText().c_str());
			int windowSize = atoi(passageWindowSize->getText().c_str() + startWindow );
			
			er = new indri::lang::FixedPassage(sn, windowSize, increment);
			_nodes.push_back(er);
			
#line 1291 "QueryParser.cpp"
		}
	}
	else {
		bool synPredMatched249 = false;
		if (((LA(1) == O_SQUARE) && (LA(2) == TERM))) {
			int _m249 = mark();
			synPredMatched249 = true;
			inputState->guessing++;
			try {
				{
				match(O_SQUARE);
				match(TERM);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched249 = false;
			}
			rewind(_m249);
			inputState->guessing--;
		}
		if ( synPredMatched249 ) {
			match(O_SQUARE);
			fName=fieldNameString();
			match(C_SQUARE);
			if ( inputState->guessing==0 ) {
#line 672 "indrilang.g"
				
				f = new indri::lang::Field(fName);
				_nodes.push_back(f);
				er = new indri::lang::ExtentRestriction(sn, f);
				_nodes.push_back(er);
				
#line 1324 "QueryParser.cpp"
			}
		}
		else if ((LA(1) == O_SQUARE) && (LA(2) == DOT)) {
			match(O_SQUARE);
			match(DOT);
			po=path();
			match(C_SQUARE);
			if ( inputState->guessing==0 ) {
#line 679 "indrilang.g"
				
				
				if ( ou == 0 ) {
				throw new antlr::SemanticException("Use of a . in a extent restriction without a valid outer context.");
				}
				po->setOuter(ou);
				er = new indri::lang::ExtentRestriction(sn, po);
				_nodes.push_back(er);
				
#line 1343 "QueryParser.cpp"
			}
		}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return er ;
}

 double  QueryParser::floating() {
#line 987 "indrilang.g"
	 double d ;
#line 1356 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  ff = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  nn = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  fff = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  nnn = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 987 "indrilang.g"
	
	d = 0;
	
#line 1367 "QueryParser.cpp"
	
	switch ( LA(1)) {
	case FLOAT:
	{
		f = LT(1);
		match(FLOAT);
		if ( inputState->guessing==0 ) {
#line 990 "indrilang.g"
			
			d = atof(f->getText().c_str());
			
#line 1379 "QueryParser.cpp"
		}
		break;
	}
	case NUMBER:
	{
		n = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 993 "indrilang.g"
			
			d = atof(n->getText().c_str());
			
#line 1392 "QueryParser.cpp"
		}
		break;
	}
	default:
		if ((LA(1) == DASH) && (LA(2) == FLOAT)) {
			match(DASH);
			ff = LT(1);
			match(FLOAT);
			if ( inputState->guessing==0 ) {
#line 996 "indrilang.g"
				
				d = - atof(ff->getText().c_str());
				
#line 1406 "QueryParser.cpp"
			}
		}
		else if ((LA(1) == DASH) && (LA(2) == NUMBER)) {
			match(DASH);
			nn = LT(1);
			match(NUMBER);
			if ( inputState->guessing==0 ) {
#line 999 "indrilang.g"
				
				d = - atof(nn->getText().c_str());
				
#line 1418 "QueryParser.cpp"
			}
		}
		else if ((LA(1) == SPACE_DASH) && (LA(2) == FLOAT)) {
			match(SPACE_DASH);
			fff = LT(1);
			match(FLOAT);
			if ( inputState->guessing==0 ) {
#line 1002 "indrilang.g"
				
				d = - atof(fff->getText().c_str());
				
#line 1430 "QueryParser.cpp"
			}
		}
		else if ((LA(1) == SPACE_DASH) && (LA(2) == NUMBER)) {
			match(SPACE_DASH);
			nnn = LT(1);
			match(NUMBER);
			if ( inputState->guessing==0 ) {
#line 1005 "indrilang.g"
				
				d = - atof(nnn->getText().c_str());
				
#line 1442 "QueryParser.cpp"
			}
		}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return d ;
}

 indri::lang::ScoredExtentNode*  QueryParser::sumList(
	 indri::lang::WSumNode* wn, indri::lang::RawExtentNode * ou 
) {
#line 279 "indrilang.g"
	 indri::lang::ScoredExtentNode* sr ;
#line 1457 "QueryParser.cpp"
#line 279 "indrilang.g"
	
	double w = 0;
	ScoredExtentNode* n = 0;
	sr = wn;
	
#line 1464 "QueryParser.cpp"
	
	{
	switch ( LA(1)) {
	case O_SQUARE:
	{
		sr=extentRestriction(wn, ou);
		if ( inputState->guessing==0 ) {
#line 285 "indrilang.g"
			ou = innerMost(sr);
#line 1474 "QueryParser.cpp"
		}
		break;
	}
	case O_PAREN:
	{
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	match(O_PAREN);
	{ // ( ... )+
	int _cnt123=0;
	for (;;) {
		if ((_tokenSet_0.member(LA(1)))) {
			n=scoredExtentNode(ou);
			if ( inputState->guessing==0 ) {
#line 287 "indrilang.g"
				wn->addChild( 1.0, n );
#line 1497 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt123>=1 ) { goto _loop123; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt123++;
	}
	_loop123:;
	}  // ( ... )+
	match(C_PAREN);
	return sr ;
}

 indri::lang::ScoredExtentNode*  QueryParser::unweightedList(
	 indri::lang::UnweightedCombinationNode* cn, indri::lang::RawExtentNode * ou 
) {
#line 291 "indrilang.g"
	 indri::lang::ScoredExtentNode* sr ;
#line 1517 "QueryParser.cpp"
#line 291 "indrilang.g"
	
	ScoredExtentNode* n = 0;
	sr = cn;
	
#line 1523 "QueryParser.cpp"
	
	{
	switch ( LA(1)) {
	case O_SQUARE:
	{
		sr=extentRestriction(cn, ou);
		if ( inputState->guessing==0 ) {
#line 296 "indrilang.g"
			ou = innerMost(sr);
#line 1533 "QueryParser.cpp"
		}
		break;
	}
	case O_PAREN:
	{
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	match(O_PAREN);
	{ // ( ... )+
	int _cnt127=0;
	for (;;) {
		if ((_tokenSet_0.member(LA(1)))) {
			n=scoredExtentNode(ou);
			if ( inputState->guessing==0 ) {
#line 298 "indrilang.g"
				cn->addChild( n );
#line 1556 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt127>=1 ) { goto _loop127; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt127++;
	}
	_loop127:;
	}  // ( ... )+
	match(C_PAREN);
	return sr ;
}

 indri::lang::ScoredExtentNode*  QueryParser::sumNode(
	indri::lang::RawExtentNode * ou 
) {
#line 316 "indrilang.g"
	 indri::lang::ScoredExtentNode* r ;
#line 1576 "QueryParser.cpp"
#line 316 "indrilang.g"
	
	indri::lang::WSumNode* wn = new indri::lang::WSumNode;
	_nodes.push_back(wn);
	
#line 1582 "QueryParser.cpp"
	
	match(SUM);
	r=sumList(wn, ou);
	return r ;
}

 indri::lang::WeightedExtentOr*  QueryParser::wsynNode() {
#line 384 "indrilang.g"
	 indri::lang::WeightedExtentOr* ws ;
#line 1592 "QueryParser.cpp"
#line 384 "indrilang.g"
	
	ws = new indri::lang::WeightedExtentOr;
	_nodes.push_back(ws);
	
	double w = 0;
	RawExtentNode* n = 0;
	
#line 1601 "QueryParser.cpp"
	
	match(WSYN);
	match(O_PAREN);
	{ // ( ... )+
	int _cnt140=0;
	for (;;) {
		if ((_tokenSet_7.member(LA(1)))) {
			w=floating();
			n=unscoredTerm();
			if ( inputState->guessing==0 ) {
#line 393 "indrilang.g"
				ws->addChild( w, n );
#line 1614 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt140>=1 ) { goto _loop140; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt140++;
	}
	_loop140:;
	}  // ( ... )+
	match(C_PAREN);
	return ws ;
}

 RawExtentNode*  QueryParser::unscoredTerm() {
#line 504 "indrilang.g"
	 RawExtentNode* t ;
#line 1632 "QueryParser.cpp"
#line 504 "indrilang.g"
	
	t = 0;
	
#line 1637 "QueryParser.cpp"
	
	bool synPredMatched174 = false;
	if (((_tokenSet_1.member(LA(1))) && (_tokenSet_2.member(LA(2))))) {
		int _m174 = mark();
		synPredMatched174 = true;
		inputState->guessing++;
		try {
			{
			qualifiedTerm();
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched174 = false;
		}
		rewind(_m174);
		inputState->guessing--;
	}
	if ( synPredMatched174 ) {
		t=qualifiedTerm();
	}
	else if ((_tokenSet_1.member(LA(1))) && (_tokenSet_8.member(LA(2)))) {
		t=unqualifiedTerm();
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	return t ;
}

 indri::lang::ODNode*  QueryParser::odNode() {
#line 397 "indrilang.g"
	 indri::lang::ODNode* od ;
#line 1671 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  n1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  n2 = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 397 "indrilang.g"
	
	RawExtentNode* rn = 0;
	od = new indri::lang::ODNode;
	_nodes.push_back(od);
	
#line 1680 "QueryParser.cpp"
	
	{
	bool synPredMatched144 = false;
	if (((LA(1) == OD) && (LA(2) == NUMBER))) {
		int _m144 = mark();
		synPredMatched144 = true;
		inputState->guessing++;
		try {
			{
			match(OD);
			match(NUMBER);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched144 = false;
		}
		rewind(_m144);
		inputState->guessing--;
	}
	if ( synPredMatched144 ) {
		{
		match(OD);
		n1 = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 406 "indrilang.g"
			od->setWindowSize( n1->getText() );
#line 1708 "QueryParser.cpp"
		}
		}
	}
	else {
		bool synPredMatched147 = false;
		if (((LA(1) == OD) && (LA(2) == O_PAREN))) {
			int _m147 = mark();
			synPredMatched147 = true;
			inputState->guessing++;
			try {
				{
				match(OD);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched147 = false;
			}
			rewind(_m147);
			inputState->guessing--;
		}
		if ( synPredMatched147 ) {
			{
			match(OD);
			}
		}
		else if ((LA(1) == OPERATOR)) {
			{
			match(OPERATOR);
			n2 = LT(1);
			match(NUMBER);
			if ( inputState->guessing==0 ) {
#line 410 "indrilang.g"
				od->setWindowSize( n2->getText() );
#line 1742 "QueryParser.cpp"
			}
			}
		}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	match(O_PAREN);
	{ // ( ... )+
	int _cnt151=0;
	for (;;) {
		if ((_tokenSet_1.member(LA(1)))) {
			rn=unscoredTerm();
			if ( inputState->guessing==0 ) {
#line 414 "indrilang.g"
				od->addChild( rn );
#line 1760 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt151>=1 ) { goto _loop151; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt151++;
	}
	_loop151:;
	}  // ( ... )+
	match(C_PAREN);
	return od ;
}

 indri::lang::UWNode*  QueryParser::uwNode() {
#line 418 "indrilang.g"
	 indri::lang::UWNode* uw ;
#line 1778 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 418 "indrilang.g"
	
	uw = new indri::lang::UWNode;
	RawExtentNode* rn = 0;
	_nodes.push_back(uw);
	
#line 1786 "QueryParser.cpp"
	
	{
	bool synPredMatched155 = false;
	if (((LA(1) == UW) && (LA(2) == NUMBER))) {
		int _m155 = mark();
		synPredMatched155 = true;
		inputState->guessing++;
		try {
			{
			match(UW);
			match(NUMBER);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched155 = false;
		}
		rewind(_m155);
		inputState->guessing--;
	}
	if ( synPredMatched155 ) {
		{
		match(UW);
		n = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 426 "indrilang.g"
			uw->setWindowSize( n->getText() );
#line 1814 "QueryParser.cpp"
		}
		}
	}
	else if ((LA(1) == UW) && (LA(2) == O_PAREN)) {
		{
		match(UW);
		}
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	}
	match(O_PAREN);
	{ // ( ... )+
	int _cnt159=0;
	for (;;) {
		if ((_tokenSet_1.member(LA(1)))) {
			rn=unscoredTerm();
			if ( inputState->guessing==0 ) {
#line 432 "indrilang.g"
				uw->addChild( rn );
#line 1837 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt159>=1 ) { goto _loop159; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt159++;
	}
	_loop159:;
	}  // ( ... )+
	match(C_PAREN);
	return uw ;
}

 indri::lang::BAndNode*  QueryParser::bandNode() {
#line 435 "indrilang.g"
	 indri::lang::BAndNode* b ;
#line 1855 "QueryParser.cpp"
#line 435 "indrilang.g"
	
	b = new indri::lang::BAndNode;
	RawExtentNode* rn = 0;
	_nodes.push_back(b);
	
#line 1862 "QueryParser.cpp"
	
	match(BAND);
	match(O_PAREN);
	{ // ( ... )+
	int _cnt162=0;
	for (;;) {
		if ((_tokenSet_1.member(LA(1)))) {
			rn=unscoredTerm();
			if ( inputState->guessing==0 ) {
#line 443 "indrilang.g"
				b->addChild( rn );
#line 1874 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt162>=1 ) { goto _loop162; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt162++;
	}
	_loop162:;
	}  // ( ... )+
	match(C_PAREN);
	return b ;
}

 indri::lang::Field*  QueryParser::anyField() {
#line 490 "indrilang.g"
	 indri::lang::Field* f ;
#line 1892 "QueryParser.cpp"
#line 490 "indrilang.g"
	
	std::string fName;
	f = 0;
	
#line 1898 "QueryParser.cpp"
	
	bool synPredMatched169 = false;
	if (((LA(1) == ANY) && (LA(2) == COLON))) {
		int _m169 = mark();
		synPredMatched169 = true;
		inputState->guessing++;
		try {
			{
			match(ANY);
			match(COLON);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched169 = false;
		}
		rewind(_m169);
		inputState->guessing--;
	}
	if ( synPredMatched169 ) {
		match(ANY);
		match(COLON);
		fName=fieldNameString();
		if ( inputState->guessing==0 ) {
#line 495 "indrilang.g"
			
			f = new Field(fName);
			_nodes.push_back(f);
			
#line 1927 "QueryParser.cpp"
		}
	}
	else {
		bool synPredMatched171 = false;
		if (((LA(1) == ANY) && (LA(2) == O_PAREN))) {
			int _m171 = mark();
			synPredMatched171 = true;
			inputState->guessing++;
			try {
				{
				match(ANY);
				match(O_PAREN);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched171 = false;
			}
			rewind(_m171);
			inputState->guessing--;
		}
		if ( synPredMatched171 ) {
			match(ANY);
			match(O_PAREN);
			fName=fieldNameString();
			match(C_PAREN);
			if ( inputState->guessing==0 ) {
#line 499 "indrilang.g"
				
				f = new Field(fName);
				_nodes.push_back(f);
				
#line 1959 "QueryParser.cpp"
			}
		}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return f ;
}

std::string  QueryParser::fieldNameString() {
#line 619 "indrilang.g"
	std::string field;
#line 1972 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  first = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  fname = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 619 "indrilang.g"
	
	std::string second;
	
#line 1979 "QueryParser.cpp"
	
	bool synPredMatched239 = false;
	if (((LA(1) == TERM) && (LA(2) == DASH))) {
		int _m239 = mark();
		synPredMatched239 = true;
		inputState->guessing++;
		try {
			{
			match(TERM);
			match(DASH);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched239 = false;
		}
		rewind(_m239);
		inputState->guessing--;
	}
	if ( synPredMatched239 ) {
		first = LT(1);
		match(TERM);
		if ( inputState->guessing==0 ) {
#line 621 "indrilang.g"
			
			field = first->getText();
			
#line 2006 "QueryParser.cpp"
		}
		{ // ( ... )+
		int _cnt241=0;
		for (;;) {
			if ((LA(1) == DASH) && (LA(2) == NUMBER || LA(2) == TERM)) {
				match(DASH);
				second=fstring();
				if ( inputState->guessing==0 ) {
#line 623 "indrilang.g"
					
					field += "-";
					field += second;
					
#line 2020 "QueryParser.cpp"
				}
			}
			else {
				if ( _cnt241>=1 ) { goto _loop241; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt241++;
		}
		_loop241:;
		}  // ( ... )+
	}
	else if ((LA(1) == TERM) && (_tokenSet_9.member(LA(2)))) {
		fname = LT(1);
		match(TERM);
		if ( inputState->guessing==0 ) {
#line 627 "indrilang.g"
			
			field = fname->getText();
			
#line 2040 "QueryParser.cpp"
		}
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	return field;
}

 indri::lang::ExtentAnd*  QueryParser::field_list() {
#line 773 "indrilang.g"
	 indri::lang::ExtentAnd* fields ;
#line 2053 "QueryParser.cpp"
#line 773 "indrilang.g"
	
	std::string first, additional;
	fields = new ExtentAnd;
	_nodes.push_back( fields );
	
#line 2060 "QueryParser.cpp"
	
	first=fieldNameString();
	if ( inputState->guessing==0 ) {
#line 780 "indrilang.g"
		
		Field* firstField = new indri::lang::Field( first );
		_nodes.push_back( firstField );
		fields->addChild( firstField );
		
#line 2070 "QueryParser.cpp"
	}
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == COMMA)) {
			match(COMMA);
			additional=fieldNameString();
			if ( inputState->guessing==0 ) {
#line 787 "indrilang.g"
				
				Field* additionalField = new Field(additional);
				_nodes.push_back( additionalField );
				fields->addChild( additionalField );
				
#line 2084 "QueryParser.cpp"
			}
		}
		else {
			goto _loop266;
		}
		
	}
	_loop266:;
	} // ( ... )*
	return fields ;
}

 indri::lang::FieldLessNode*  QueryParser::dateBefore() {
#line 840 "indrilang.g"
	 indri::lang::FieldLessNode* extent ;
#line 2100 "QueryParser.cpp"
#line 840 "indrilang.g"
	
	UINT64 d = 0;
	Field* dateField = 0;
	extent = 0;
	
#line 2107 "QueryParser.cpp"
	
	match(DATEBEFORE);
	match(O_PAREN);
	d=date();
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 845 "indrilang.g"
		
		dateField = new Field("date");
		extent = new FieldLessNode( dateField, d );
		_nodes.push_back( dateField );
		_nodes.push_back( extent );
		
#line 2121 "QueryParser.cpp"
	}
	return extent ;
}

 indri::lang::FieldGreaterNode*  QueryParser::dateAfter() {
#line 852 "indrilang.g"
	 indri::lang::FieldGreaterNode* extent ;
#line 2129 "QueryParser.cpp"
#line 852 "indrilang.g"
	
	UINT64 d = 0;
	Field* dateField = 0;
	extent = 0;
	
#line 2136 "QueryParser.cpp"
	
	match(DATEAFTER);
	match(O_PAREN);
	d=date();
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 857 "indrilang.g"
		
		dateField = new Field("date");
		extent = new FieldGreaterNode( dateField, d );
		_nodes.push_back( dateField );
		_nodes.push_back( extent );
		
#line 2150 "QueryParser.cpp"
	}
	return extent ;
}

 indri::lang::FieldBetweenNode*  QueryParser::dateBetween() {
#line 864 "indrilang.g"
	 indri::lang::FieldBetweenNode* extent ;
#line 2158 "QueryParser.cpp"
#line 864 "indrilang.g"
	
	UINT64 low = 0;
	UINT64 high = 0;
	Field* dateField = 0;
	extent = 0;
	
#line 2166 "QueryParser.cpp"
	
	match(DATEBETWEEN);
	match(O_PAREN);
	low=date();
	high=date();
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 870 "indrilang.g"
		
		dateField = new Field("date");
		extent = new FieldBetweenNode( dateField, low, high );
		_nodes.push_back( dateField );
		_nodes.push_back( extent );
		
#line 2181 "QueryParser.cpp"
	}
	return extent ;
}

 indri::lang::FieldEqualsNode*  QueryParser::dateEquals() {
#line 877 "indrilang.g"
	 indri::lang::FieldEqualsNode* extent ;
#line 2189 "QueryParser.cpp"
#line 877 "indrilang.g"
	
	UINT64 d = 0;
	Field* dateField = 0;
	extent = 0;
	
#line 2196 "QueryParser.cpp"
	
	match(DATEEQUALS);
	match(O_PAREN);
	d=date();
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 882 "indrilang.g"
		
		dateField = new Field("date");
		extent = new FieldEqualsNode( dateField, d );
		_nodes.push_back( dateField );
		_nodes.push_back( extent );
		
#line 2210 "QueryParser.cpp"
	}
	return extent ;
}

 indri::lang::ExtentOr*  QueryParser::synonym_list() {
#line 744 "indrilang.g"
	 indri::lang::ExtentOr* s ;
#line 2218 "QueryParser.cpp"
#line 744 "indrilang.g"
	
	indri::lang::RawExtentNode* term = 0;
	s = new indri::lang::ExtentOr;
	_nodes.push_back(s);
	
#line 2225 "QueryParser.cpp"
	
	match(O_ANGLE);
	{ // ( ... )+
	int _cnt257=0;
	for (;;) {
		if ((_tokenSet_1.member(LA(1)))) {
			term=unscoredTerm();
			if ( inputState->guessing==0 ) {
#line 750 "indrilang.g"
				s->addChild(term);
#line 2236 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt257>=1 ) { goto _loop257; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt257++;
	}
	_loop257:;
	}  // ( ... )+
	match(C_ANGLE);
	return s ;
}

 indri::lang::ExtentOr*  QueryParser::synonym_list_brace() {
#line 753 "indrilang.g"
	 indri::lang::ExtentOr* s ;
#line 2254 "QueryParser.cpp"
#line 753 "indrilang.g"
	
	indri::lang::RawExtentNode* term = 0;
	s = new indri::lang::ExtentOr;
	_nodes.push_back(s);
	
#line 2261 "QueryParser.cpp"
	
	match(O_BRACE);
	{ // ( ... )+
	int _cnt260=0;
	for (;;) {
		if ((_tokenSet_1.member(LA(1)))) {
			term=unscoredTerm();
			if ( inputState->guessing==0 ) {
#line 759 "indrilang.g"
				s->addChild(term);
#line 2272 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt260>=1 ) { goto _loop260; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt260++;
	}
	_loop260:;
	}  // ( ... )+
	match(C_BRACE);
	return s ;
}

 indri::lang::ExtentOr*  QueryParser::synonym_list_alt() {
#line 762 "indrilang.g"
	 indri::lang::ExtentOr* s ;
#line 2290 "QueryParser.cpp"
#line 762 "indrilang.g"
	
	indri::lang::RawExtentNode* term = 0;
	// semantics of this node will change
	s = new indri::lang::ExtentOr;
	_nodes.push_back(s);
	
#line 2298 "QueryParser.cpp"
	
	match(SYN);
	match(O_PAREN);
	{ // ( ... )+
	int _cnt263=0;
	for (;;) {
		if ((_tokenSet_1.member(LA(1)))) {
			term=unscoredTerm();
			if ( inputState->guessing==0 ) {
#line 770 "indrilang.g"
				s->addChild(term);
#line 2310 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt263>=1 ) { goto _loop263; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt263++;
	}
	_loop263:;
	}  // ( ... )+
	match(C_PAREN);
	return s ;
}

 indri::lang::FieldLessNode*  QueryParser::lessNode() {
#line 1035 "indrilang.g"
	 indri::lang::FieldLessNode* ln ;
#line 2328 "QueryParser.cpp"
#line 1035 "indrilang.g"
	
	ln = 0;
	Field* compareField = 0;
	INT64 high = 0;
	std::string field;
	
#line 2336 "QueryParser.cpp"
	
	match(LESS);
	match(O_PAREN);
	field=fieldNameString();
	high=number();
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 1041 "indrilang.g"
		
		compareField = new Field(field);
		ln = new FieldLessNode( compareField, high );
		_nodes.push_back( compareField );
		_nodes.push_back( ln );
		
#line 2351 "QueryParser.cpp"
	}
	return ln ;
}

 indri::lang::FieldGreaterNode*  QueryParser::greaterNode() {
#line 1022 "indrilang.g"
	 indri::lang::FieldGreaterNode* gn ;
#line 2359 "QueryParser.cpp"
#line 1022 "indrilang.g"
	
	gn = 0;
	Field* compareField = 0;
	INT64 low = 0;
	std::string field;
	
#line 2367 "QueryParser.cpp"
	
	match(GREATER);
	match(O_PAREN);
	field=fieldNameString();
	low=number();
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 1028 "indrilang.g"
		
		compareField = new Field(field);
		gn = new FieldGreaterNode( compareField, low );
		_nodes.push_back( compareField );
		_nodes.push_back( gn );
		
#line 2382 "QueryParser.cpp"
	}
	return gn ;
}

 indri::lang::FieldBetweenNode*  QueryParser::betweenNode() {
#line 1048 "indrilang.g"
	 indri::lang::FieldBetweenNode* bn ;
#line 2390 "QueryParser.cpp"
#line 1048 "indrilang.g"
	
	bn = 0;
	Field* compareField = 0;
	INT64 low = 0;
	INT64 high = 0;
	std::string field;
	
#line 2399 "QueryParser.cpp"
	
	match(BETWEEN);
	match(O_PAREN);
	field=fieldNameString();
	low=number();
	high=number();
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 1055 "indrilang.g"
		
		compareField = new Field(field);
		bn = new FieldBetweenNode( compareField, low, high );
		_nodes.push_back( compareField );
		_nodes.push_back( bn );
		
#line 2415 "QueryParser.cpp"
	}
	return bn ;
}

 indri::lang::FieldEqualsNode*  QueryParser::equalsNode() {
#line 1062 "indrilang.g"
	 indri::lang::FieldEqualsNode* en ;
#line 2423 "QueryParser.cpp"
#line 1062 "indrilang.g"
	
	en = 0;
	Field* compareField = 0;
	INT64 eq = 0;
	std::string field;
	
#line 2431 "QueryParser.cpp"
	
	match(EQUALS);
	match(O_PAREN);
	field=fieldNameString();
	eq=number();
	match(C_PAREN);
	if ( inputState->guessing==0 ) {
#line 1068 "indrilang.g"
		
		compareField = new Field(field);
		en = new FieldEqualsNode( compareField, eq );
		_nodes.push_back( compareField );
		_nodes.push_back( en );
		
#line 2446 "QueryParser.cpp"
	}
	return en ;
}

 indri::lang::IndexTerm*  QueryParser::rawText() {
#line 936 "indrilang.g"
	 indri::lang::IndexTerm* t ;
#line 2454 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  nn = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  nnn = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  ff = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  fff = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  et = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  qet = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 936 "indrilang.g"
	
	t = 0;
	
#line 2468 "QueryParser.cpp"
	
	switch ( LA(1)) {
	case TERM:
	{
		id = LT(1);
		match(TERM);
		if ( inputState->guessing==0 ) {
#line 939 "indrilang.g"
			
			t = new indri::lang::IndexTerm(id->getText());
			_nodes.push_back(t);
			
#line 2481 "QueryParser.cpp"
		}
		break;
	}
	case NUMBER:
	{
		n = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 943 "indrilang.g"
			
			t = new indri::lang::IndexTerm(n->getText());
			_nodes.push_back(t);
			
#line 2495 "QueryParser.cpp"
		}
		break;
	}
	case FLOAT:
	{
		f = LT(1);
		match(FLOAT);
		if ( inputState->guessing==0 ) {
#line 955 "indrilang.g"
			
			t = new indri::lang::IndexTerm(f->getText());
			_nodes.push_back(t);
			
#line 2509 "QueryParser.cpp"
		}
		break;
	}
	case DBL_QUOTE:
	{
		match(DBL_QUOTE);
		t=rawText();
		match(DBL_QUOTE);
		if ( inputState->guessing==0 ) {
#line 967 "indrilang.g"
			
			// if a text term appears in quotes, consider it stemmed
			t->setStemmed(true);
			
#line 2524 "QueryParser.cpp"
		}
		break;
	}
	case ENCODED_TERM:
	{
		et = LT(1);
		match(ENCODED_TERM);
		if ( inputState->guessing==0 ) {
#line 971 "indrilang.g"
			
			std::string decodedString; 
			base64_decode_string(decodedString, et->getText());
			t = new indri::lang::IndexTerm( decodedString );
			_nodes.push_back(t);
			
#line 2540 "QueryParser.cpp"
		}
		break;
	}
	case ENCODED_QUOTED_TERM:
	{
		qet = LT(1);
		match(ENCODED_QUOTED_TERM);
		if ( inputState->guessing==0 ) {
#line 977 "indrilang.g"
			
			std::string decodedString; 
			base64_decode_string(decodedString, qet->getText());
			t = new indri::lang::IndexTerm( decodedString );
			t->setStemmed(true);
			_nodes.push_back(t);
			
#line 2557 "QueryParser.cpp"
		}
		break;
	}
	default:
		if ((LA(1) == SPACE_DASH) && (LA(2) == NUMBER)) {
			match(SPACE_DASH);
			nn = LT(1);
			match(NUMBER);
			if ( inputState->guessing==0 ) {
#line 947 "indrilang.g"
				
				t = new indri::lang::IndexTerm(std::string("-") + nn->getText());
				_nodes.push_back(t);
				
#line 2572 "QueryParser.cpp"
			}
		}
		else if ((LA(1) == DASH) && (LA(2) == NUMBER)) {
			match(DASH);
			nnn = LT(1);
			match(NUMBER);
			if ( inputState->guessing==0 ) {
#line 951 "indrilang.g"
				
				t = new indri::lang::IndexTerm(std::string("-") + nnn->getText());
				_nodes.push_back(t);
				
#line 2585 "QueryParser.cpp"
			}
		}
		else if ((LA(1) == DASH) && (LA(2) == FLOAT)) {
			match(DASH);
			ff = LT(1);
			match(FLOAT);
			if ( inputState->guessing==0 ) {
#line 959 "indrilang.g"
				
				t = new indri::lang::IndexTerm(std::string("-") + ff->getText());
				_nodes.push_back(t);
				
#line 2598 "QueryParser.cpp"
			}
		}
		else if ((LA(1) == SPACE_DASH) && (LA(2) == FLOAT)) {
			match(SPACE_DASH);
			fff = LT(1);
			match(FLOAT);
			if ( inputState->guessing==0 ) {
#line 963 "indrilang.g"
				
				t = new indri::lang::IndexTerm(std::string("-") + fff->getText());
				_nodes.push_back(t);
				
#line 2611 "QueryParser.cpp"
			}
		}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return t ;
}

 indri::lang::ODNode*  QueryParser::hyphenTerm() {
#line 586 "indrilang.g"
	 indri::lang::ODNode* od ;
#line 2624 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 586 "indrilang.g"
	
	od = new indri::lang::ODNode;
	od->setWindowSize(1);
	_nodes.push_back(od);
	indri::lang::IndexTerm* t = 0;
	
#line 2634 "QueryParser.cpp"
	
	switch ( LA(1)) {
	case TERM:
	{
		id = LT(1);
		match(TERM);
		if ( inputState->guessing==0 ) {
#line 591 "indrilang.g"
			
			t = new indri::lang::IndexTerm(id->getText());
			_nodes.push_back(t);
			od->addChild(t);
#line 2647 "QueryParser.cpp"
		}
		{ // ( ... )+
		int _cnt233=0;
		for (;;) {
			if ((LA(1) == DASH) && (LA(2) == NUMBER || LA(2) == TERM)) {
				match(DASH);
				t=hyphenate();
				if ( inputState->guessing==0 ) {
#line 595 "indrilang.g"
					
					od->addChild(t);
#line 2659 "QueryParser.cpp"
				}
			}
			else {
				if ( _cnt233>=1 ) { goto _loop233; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt233++;
		}
		_loop233:;
		}  // ( ... )+
		break;
	}
	case NUMBER:
	{
		n = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 598 "indrilang.g"
			
			t = new indri::lang::IndexTerm(n->getText());
			_nodes.push_back(t);
			od->addChild(t);
#line 2682 "QueryParser.cpp"
		}
		{ // ( ... )+
		int _cnt235=0;
		for (;;) {
			if ((LA(1) == DASH) && (LA(2) == NUMBER || LA(2) == TERM)) {
				match(DASH);
				t=hyphenate();
				if ( inputState->guessing==0 ) {
#line 602 "indrilang.g"
					
					od->addChild(t);
#line 2694 "QueryParser.cpp"
				}
			}
			else {
				if ( _cnt235>=1 ) { goto _loop235; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt235++;
		}
		_loop235:;
		}  // ( ... )+
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return od ;
}

 indri::lang::WildcardTerm*  QueryParser::wildcardOpNode() {
#line 639 "indrilang.g"
	 indri::lang::WildcardTerm* s ;
#line 2718 "QueryParser.cpp"
#line 639 "indrilang.g"
	
	// wildcard operator "#wildcard( term )"
	indri::lang::IndexTerm* t = 0;
	s = new indri::lang::WildcardTerm;
	_nodes.push_back(s);
	
#line 2726 "QueryParser.cpp"
	
	match(WCARD);
	match(O_PAREN);
	{
	t=rawText();
	if ( inputState->guessing==0 ) {
#line 647 "indrilang.g"
		s->setTerm(t->getText());
#line 2735 "QueryParser.cpp"
	}
	}
	match(C_PAREN);
	return s ;
}

indri::lang::IndexTerm *  QueryParser::hyphenate() {
#line 607 "indrilang.g"
	indri::lang::IndexTerm * t;
#line 2745 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 607 "indrilang.g"
	
	t = 0;
	
#line 2752 "QueryParser.cpp"
	
	switch ( LA(1)) {
	case TERM:
	{
		id = LT(1);
		match(TERM);
		if ( inputState->guessing==0 ) {
#line 610 "indrilang.g"
			
			t = new indri::lang::IndexTerm(id->getText());
			_nodes.push_back(t);
			
#line 2765 "QueryParser.cpp"
		}
		break;
	}
	case NUMBER:
	{
		n = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 614 "indrilang.g"
			
			t = new indri::lang::IndexTerm(n->getText());
			_nodes.push_back(t);
			
#line 2779 "QueryParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return t;
}

 std::string  QueryParser::fstring() {
#line 631 "indrilang.g"
	 std::string f;
#line 2794 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	switch ( LA(1)) {
	case TERM:
	{
		id = LT(1);
		match(TERM);
		if ( inputState->guessing==0 ) {
#line 632 "indrilang.g"
			
			f = id->getText();
			
#line 2808 "QueryParser.cpp"
		}
		break;
	}
	case NUMBER:
	{
		n = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 635 "indrilang.g"
			
			f = n->getText();
			
#line 2821 "QueryParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return f;
}

 indri::lang::ExtentInside*  QueryParser::path() {
#line 690 "indrilang.g"
	 indri::lang::ExtentInside* r ;
#line 2836 "QueryParser.cpp"
#line 690 "indrilang.g"
	
	r = 0;
	indri::lang::Field * f = 0;
	indri::lang::ExtentInside * po = 0;
	indri::lang::ExtentInside * lastPo = 0;
	std::string fieldRestricted;
	
#line 2845 "QueryParser.cpp"
	
	{ // ( ... )+
	int _cnt252=0;
	for (;;) {
		if ((LA(1) == O_BRACE || LA(1) == SLASH || LA(1) == B_SLASH)) {
			po=pathOperator();
			fieldRestricted=fieldNameString();
			if ( inputState->guessing==0 ) {
#line 697 "indrilang.g"
				
				if ( r == 0 ) {  // set the root
				r = po;
				}      
				f = new indri::lang::Field(fieldRestricted);
				_nodes.push_back(f);      
				po->setInner(f);  // set the leaf's inner
				if ( lastPo != 0 ) {  // add this operator into the chain
				po->setOuter( lastPo->getInner() );
				lastPo->setInner( po );
				}
				lastPo = po;
				
#line 2868 "QueryParser.cpp"
			}
		}
		else {
			if ( _cnt252>=1 ) { goto _loop252; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt252++;
	}
	_loop252:;
	}  // ( ... )+
	return r ;
}

 indri::lang::ExtentInside*  QueryParser::pathOperator() {
#line 713 "indrilang.g"
	 indri::lang::ExtentInside* r ;
#line 2885 "QueryParser.cpp"
#line 713 "indrilang.g"
	
	r = 0;
	indri::lang::DocumentStructureNode * ds = 0;
	
#line 2891 "QueryParser.cpp"
	
	switch ( LA(1)) {
	case SLASH:
	{
		match(SLASH);
		{
		switch ( LA(1)) {
		case SLASH:
		{
			match(SLASH);
			if ( inputState->guessing==0 ) {
#line 717 "indrilang.g"
				
				ds = new indri::lang::DocumentStructureNode;
				_nodes.push_back(ds);
				r = new indri::lang::ExtentDescendant(NULL, NULL, ds);
				_nodes.push_back(r);
				
#line 2910 "QueryParser.cpp"
			}
			break;
		}
		case TERM:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
#line 722 "indrilang.g"
			
			if (r == 0) {
			ds = new indri::lang::DocumentStructureNode;
			_nodes.push_back(ds);
			r = new indri::lang::ExtentChild(NULL, NULL, ds);
			_nodes.push_back(r);
			}
			
#line 2934 "QueryParser.cpp"
		}
		break;
	}
	case B_SLASH:
	{
		match(B_SLASH);
		if ( inputState->guessing==0 ) {
#line 730 "indrilang.g"
			
			ds = new indri::lang::DocumentStructureNode;
			_nodes.push_back(ds);
			r = new indri::lang::ExtentParent(NULL, NULL, ds);
			_nodes.push_back(r);
			
#line 2949 "QueryParser.cpp"
		}
		break;
	}
	case O_BRACE:
	{
		match(O_BRACE);
		if ( inputState->guessing==0 ) {
#line 736 "indrilang.g"
			
			r = new indri::lang::ExtentInside(NULL, NULL);   
			_nodes.push_back(r);
			
#line 2962 "QueryParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return r ;
}

 indri::lang::Field*  QueryParser::field_restriction() {
#line 829 "indrilang.g"
	 indri::lang::Field* extent ;
#line 2977 "QueryParser.cpp"
#line 829 "indrilang.g"
	
	std::string fieldName;
	
#line 2982 "QueryParser.cpp"
	
	match(O_SQUARE);
	fieldName=fieldNameString();
	if ( inputState->guessing==0 ) {
#line 833 "indrilang.g"
		
		extent = new Field( fieldName );
		_nodes.push_back( extent );
		
#line 2992 "QueryParser.cpp"
	}
	match(C_SQUARE);
	return extent ;
}

 UINT64  QueryParser::date() {
#line 904 "indrilang.g"
	 UINT64 d ;
#line 3001 "QueryParser.cpp"
	
	bool synPredMatched281 = false;
	if (((LA(1) == NUMBER) && (LA(2) == SLASH))) {
		int _m281 = mark();
		synPredMatched281 = true;
		inputState->guessing++;
		try {
			{
			match(NUMBER);
			match(SLASH);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched281 = false;
		}
		rewind(_m281);
		inputState->guessing--;
	}
	if ( synPredMatched281 ) {
		d=slashDate();
	}
	else {
		bool synPredMatched283 = false;
		if (((LA(1) == NUMBER || LA(1) == TERM) && (LA(2) == NUMBER || LA(2) == TERM))) {
			int _m283 = mark();
			synPredMatched283 = true;
			inputState->guessing++;
			try {
				{
				match(TERM);
				match(NUMBER);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched283 = false;
			}
			rewind(_m283);
			inputState->guessing--;
		}
		if ( synPredMatched283 ) {
			d=spaceDate();
		}
		else {
			bool synPredMatched285 = false;
			if (((LA(1) == NUMBER || LA(1) == TERM) && (LA(2) == NUMBER || LA(2) == TERM))) {
				int _m285 = mark();
				synPredMatched285 = true;
				inputState->guessing++;
				try {
					{
					match(NUMBER);
					match(TERM);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched285 = false;
				}
				rewind(_m285);
				inputState->guessing--;
			}
			if ( synPredMatched285 ) {
				d=spaceDate();
			}
			else {
				bool synPredMatched287 = false;
				if (((LA(1) == NUMBER) && (LA(2) == DASH))) {
					int _m287 = mark();
					synPredMatched287 = true;
					inputState->guessing++;
					try {
						{
						match(NUMBER);
						match(DASH);
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched287 = false;
					}
					rewind(_m287);
					inputState->guessing--;
				}
				if ( synPredMatched287 ) {
					d=dashDate();
				}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}}}
	return d ;
}

 UINT64  QueryParser::slashDate() {
#line 918 "indrilang.g"
	 UINT64 d ;
#line 3096 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  month = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  day = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  year = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 918 "indrilang.g"
	
	d = 0;
	
#line 3104 "QueryParser.cpp"
	
	month = LT(1);
	match(NUMBER);
	match(SLASH);
	day = LT(1);
	match(NUMBER);
	match(SLASH);
	year = LT(1);
	match(NUMBER);
	if ( inputState->guessing==0 ) {
#line 921 "indrilang.g"
		
		d = indri::parse::DateParse::convertDate( year->getText(), month->getText(), day->getText() ); 
		
#line 3119 "QueryParser.cpp"
	}
	return d ;
}

 UINT64  QueryParser::spaceDate() {
#line 925 "indrilang.g"
	 UINT64 d ;
#line 3127 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  day = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  month = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  year = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  m = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  dd = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  y = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 925 "indrilang.g"
	
	d = 0;
	
#line 3138 "QueryParser.cpp"
	
	switch ( LA(1)) {
	case NUMBER:
	{
		day = LT(1);
		match(NUMBER);
		month = LT(1);
		match(TERM);
		year = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 928 "indrilang.g"
			
			d = indri::parse::DateParse::convertDate( year->getText(), month->getText(), day->getText() );
			
#line 3154 "QueryParser.cpp"
		}
		break;
	}
	case TERM:
	{
		m = LT(1);
		match(TERM);
		dd = LT(1);
		match(NUMBER);
		y = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 931 "indrilang.g"
			
			d = indri::parse::DateParse::convertDate( y->getText(), m->getText(), dd->getText() );
			
#line 3171 "QueryParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return d ;
}

 UINT64  QueryParser::dashDate() {
#line 911 "indrilang.g"
	 UINT64 d ;
#line 3186 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  day = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  month = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  year = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 911 "indrilang.g"
	
	d = 0;
	
#line 3194 "QueryParser.cpp"
	
	day = LT(1);
	match(NUMBER);
	match(DASH);
	month = LT(1);
	match(TERM);
	match(DASH);
	year = LT(1);
	match(NUMBER);
	if ( inputState->guessing==0 ) {
#line 914 "indrilang.g"
		
		d = indri::parse::DateParse::convertDate( year->getText(), month->getText(), day->getText() );             
		
#line 3209 "QueryParser.cpp"
	}
	return d ;
}

 INT64  QueryParser::number() {
#line 1009 "indrilang.g"
	 INT64 v ;
#line 3217 "QueryParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  nn = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  nnn = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1009 "indrilang.g"
	
	v = 0;
	
#line 3225 "QueryParser.cpp"
	
	switch ( LA(1)) {
	case NUMBER:
	{
		n = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 1012 "indrilang.g"
			
			v = string_to_i64(n->getText());
			
#line 3237 "QueryParser.cpp"
		}
		break;
	}
	case DASH:
	{
		match(DASH);
		nn = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 1015 "indrilang.g"
			
			v = - string_to_i64(nn->getText());
			
#line 3251 "QueryParser.cpp"
		}
		break;
	}
	case SPACE_DASH:
	{
		match(SPACE_DASH);
		nnn = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 1018 "indrilang.g"
			
			v = - string_to_i64(nnn->getText());
			
#line 3265 "QueryParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return v ;
}

void QueryParser::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& )
{
}
const char* QueryParser::tokenNames[] = {
	"<0>",
	"EOF",
	"<2>",
	"NULL_TREE_LOOKAHEAD",
	"\"#sum\"",
	"\"#wsum\"",
	"\"#wand\"",
	"\"#od\"",
	"\"#or\"",
	"\"#not\"",
	"\"#uw\"",
	"\"#combine\"",
	"\"#weight\"",
	"\"#max\"",
	"\"#filreq\"",
	"\"#filrej\"",
	"\"#scoreif\"",
	"\"#scoreifnot\"",
	"\"#any\"",
	"\"#band\"",
	"\"#wsyn\"",
	"\"#syn\"",
	"\"#prior\"",
	"\"#dateafter\"",
	"\"#datebefore\"",
	"\"#datebetween\"",
	"\"#dateequals\"",
	"\"#less\"",
	"\"#greater\"",
	"\"#between\"",
	"\"#equals\"",
	"\"#wildcard\"",
	"NUMBER",
	"NEGATIVE_NUMBER",
	"FLOAT",
	"STAR",
	"O_PAREN",
	"C_PAREN",
	"O_ANGLE",
	"C_ANGLE",
	"O_SQUARE",
	"C_SQUARE",
	"O_BRACE",
	"C_BRACE",
	"DBL_QUOTE",
	"QUOTE",
	"DOT",
	"COMMA",
	"SLASH",
	"B_SLASH",
	"DASH",
	"SPACE_DASH",
	"COLON",
	"TAB",
	"CR",
	"LF",
	"SPACE",
	"HIGH_CHAR",
	"DIGIT",
	"ASCII_LETTER",
	"SAFE_LETTER",
	"SAFE_CHAR",
	"BASESIXFOUR_CHAR",
	"TEXT_TERM",
	"TERM",
	"ENCODED_QUOTED_TERM",
	"ENCODED_TERM",
	"OPERATOR",
	"JUNK",
	0
};

const unsigned long QueryParser::_tokenSet_0_data_[] = { 4294967264UL, 791621UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "#wsum" "#wand" "#od" "#or" "#not" "#uw" "#combine" "#weight" "#max" 
// "#filreq" "#filrej" "#scoreif" "#scoreifnot" "#any" "#band" "#wsyn" 
// "#syn" "#prior" "#dateafter" "#datebefore" "#datebetween" "#dateequals" 
// "#less" "#greater" "#between" "#equals" "#wildcard" NUMBER FLOAT O_ANGLE 
// O_BRACE DBL_QUOTE DASH SPACE_DASH TERM ENCODED_QUOTED_TERM ENCODED_TERM 
// OPERATOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryParser::_tokenSet_0(_tokenSet_0_data_,8);
const unsigned long QueryParser::_tokenSet_1_data_[] = { 4290512000UL, 791621UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "#od" "#uw" "#any" "#band" "#wsyn" "#syn" "#dateafter" "#datebefore" 
// "#datebetween" "#dateequals" "#less" "#greater" "#between" "#equals" 
// "#wildcard" NUMBER FLOAT O_ANGLE O_BRACE DBL_QUOTE DASH SPACE_DASH TERM 
// ENCODED_QUOTED_TERM ENCODED_TERM OPERATOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryParser::_tokenSet_1(_tokenSet_1_data_,8);
const unsigned long QueryParser::_tokenSet_2_data_[] = { 4290512000UL, 1856605UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "#od" "#uw" "#any" "#band" "#wsyn" "#syn" "#dateafter" "#datebefore" 
// "#datebetween" "#dateequals" "#less" "#greater" "#between" "#equals" 
// "#wildcard" NUMBER FLOAT STAR O_PAREN O_ANGLE O_BRACE DBL_QUOTE DOT 
// DASH SPACE_DASH COLON TERM ENCODED_QUOTED_TERM ENCODED_TERM OPERATOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryParser::_tokenSet_2(_tokenSet_2_data_,8);
const unsigned long QueryParser::_tokenSet_3_data_[] = { 4294967266UL, 1840253UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "#wsum" "#wand" "#od" "#or" "#not" "#uw" "#combine" "#weight" "#max" 
// "#filreq" "#filrej" "#scoreif" "#scoreifnot" "#any" "#band" "#wsyn" 
// "#syn" "#prior" "#dateafter" "#datebefore" "#datebetween" "#dateequals" 
// "#less" "#greater" "#between" "#equals" "#wildcard" NUMBER FLOAT STAR 
// O_PAREN C_PAREN O_ANGLE O_BRACE DBL_QUOTE DASH SPACE_DASH COLON TERM 
// ENCODED_QUOTED_TERM ENCODED_TERM OPERATOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryParser::_tokenSet_3(_tokenSet_3_data_,8);
const unsigned long QueryParser::_tokenSet_4_data_[] = { 0UL, 790533UL, 7UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NUMBER FLOAT DBL_QUOTE DASH SPACE_DASH TERM ENCODED_QUOTED_TERM ENCODED_TERM 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryParser::_tokenSet_4(_tokenSet_4_data_,8);
const unsigned long QueryParser::_tokenSet_5_data_[] = { 4294967266UL, 810213UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "#wsum" "#wand" "#od" "#or" "#not" "#uw" "#combine" "#weight" "#max" 
// "#filreq" "#filrej" "#scoreif" "#scoreifnot" "#any" "#band" "#wsyn" 
// "#syn" "#prior" "#dateafter" "#datebefore" "#datebetween" "#dateequals" 
// "#less" "#greater" "#between" "#equals" "#wildcard" NUMBER FLOAT C_PAREN 
// O_ANGLE C_ANGLE O_BRACE C_BRACE DBL_QUOTE DOT DASH SPACE_DASH TERM ENCODED_QUOTED_TERM 
// ENCODED_TERM OPERATOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryParser::_tokenSet_5(_tokenSet_5_data_,8);
const unsigned long QueryParser::_tokenSet_6_data_[] = { 0UL, 790541UL, 7UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NUMBER FLOAT STAR DBL_QUOTE DASH SPACE_DASH TERM ENCODED_QUOTED_TERM 
// ENCODED_TERM 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryParser::_tokenSet_6(_tokenSet_6_data_,8);
const unsigned long QueryParser::_tokenSet_7_data_[] = { 0UL, 786437UL, 0UL, 0UL };
// NUMBER FLOAT DASH SPACE_DASH 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryParser::_tokenSet_7(_tokenSet_7_data_,4);
const unsigned long QueryParser::_tokenSet_8_data_[] = { 4294967264UL, 1842429UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "#wsum" "#wand" "#od" "#or" "#not" "#uw" "#combine" "#weight" "#max" 
// "#filreq" "#filrej" "#scoreif" "#scoreifnot" "#any" "#band" "#wsyn" 
// "#syn" "#prior" "#dateafter" "#datebefore" "#datebetween" "#dateequals" 
// "#less" "#greater" "#between" "#equals" "#wildcard" NUMBER FLOAT STAR 
// O_PAREN C_PAREN O_ANGLE C_ANGLE O_BRACE C_BRACE DBL_QUOTE DASH SPACE_DASH 
// COLON TERM ENCODED_QUOTED_TERM ENCODED_TERM OPERATOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryParser::_tokenSet_8(_tokenSet_8_data_,8);
const unsigned long QueryParser::_tokenSet_9_data_[] = { 4294967266UL, 1040101UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "#wsum" "#wand" "#od" "#or" "#not" "#uw" "#combine" "#weight" "#max" 
// "#filreq" "#filrej" "#scoreif" "#scoreifnot" "#any" "#band" "#wsyn" 
// "#syn" "#prior" "#dateafter" "#datebefore" "#datebetween" "#dateequals" 
// "#less" "#greater" "#between" "#equals" "#wildcard" NUMBER FLOAT C_PAREN 
// O_ANGLE C_ANGLE C_SQUARE O_BRACE C_BRACE DBL_QUOTE DOT COMMA SLASH B_SLASH 
// DASH SPACE_DASH TERM ENCODED_QUOTED_TERM ENCODED_TERM OPERATOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet QueryParser::_tokenSet_9(_tokenSet_9_data_,8);


ANTLR_END_NAMESPACE
ANTLR_END_NAMESPACE
