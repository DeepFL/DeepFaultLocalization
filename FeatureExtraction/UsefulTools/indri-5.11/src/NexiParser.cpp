/* $ANTLR 2.7.3 (20060307-1): "nexilang.g" -> "NexiParser.cpp"$ */
#include "indri/NexiParser.hpp"
#include <antlr/NoViableAltException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/ASTFactory.hpp>
ANTLR_BEGIN_NAMESPACE(indri)
ANTLR_BEGIN_NAMESPACE(lang)
#line 1 "nexilang.g"
#line 8 "NexiParser.cpp"
NexiParser::NexiParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,k)
{
}

NexiParser::NexiParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,1)
{
}

NexiParser::NexiParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,k)
{
}

NexiParser::NexiParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,1)
{
}

NexiParser::NexiParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(state,1)
{
}

 indri::lang::ScoredExtentNode*  NexiParser::query() {
#line 152 "nexilang.g"
	 indri::lang::ScoredExtentNode* q ;
#line 37 "NexiParser.cpp"
#line 152 "nexilang.g"
	
	indri::lang::ScoredExtentNode * c = 0;
	indri::lang::NestedExtentInside * p = 0;
	indri::lang::RawExtentNode * f = 0;
	indri::lang::ScoredExtentNode * c2 = 0;
	indri::lang::NestedExtentInside * p2 = 0;
	indri::lang::RawExtentNode * f2 = 0;
	
#line 47 "NexiParser.cpp"
	
	switch ( LA(1)) {
	case SLASH:
	{
		{
		p=path();
		f=field();
		match(O_SQUARE);
		c=clause();
		match(C_SQUARE);
		if ( inputState->guessing==0 ) {
#line 160 "nexilang.g"
			
			
			indri::lang::ExtentRestriction * r = 0;
			
			// finish the path with a field as the inner for the last extent inside
			if (p != 0) {
			indri::lang::NestedExtentInside * pt = p;
			while (pt->getInner() != NULL) {
			pt = (indri::lang::NestedExtentInside *) pt->getInner();
			}
			pt->setInner(f);
			
			r = new indri::lang::ExtentRestriction(c, p);
			} else {
			r = new indri::lang::ExtentRestriction(c, f);
			}
			_nodes.push_back(r);
			q=r;
			
#line 79 "NexiParser.cpp"
		}
		{
		switch ( LA(1)) {
		case SLASH:
		{
			p2=path();
			f2=field();
			match(O_SQUARE);
			c2=clause();
			match(C_SQUARE);
			if ( inputState->guessing==0 ) {
#line 179 "nexilang.g"
				
				
				// finish the path with a field as the inner for the last extent inside
				indri::lang::ExtentRestriction * r2;
				if (p2 != 0) {
				indri::lang::NestedExtentInside * pt2 = p2;
				while (pt2->getInner() != NULL) {
				pt2 = (indri::lang::NestedExtentInside *) pt2->getInner();
				}
				pt2->setInner(f2);
				
				r2 = new indri::lang::ExtentRestriction(c2, p2);
				} else {
				r2 = new indri::lang::ExtentRestriction(c2, f2);
				}
				_nodes.push_back(r2);
				
				indri::lang::ContextInclusionNode * combine = new indri::lang::ContextInclusionNode;
				_nodes.push_back(combine);
				
				combine->addChild(c);
				combine->addChild(r2, true);
				
				indri::lang::ExtentEnforcement * enf = new indri::lang::ExtentEnforcement(combine, f);
				_nodes.push_back(enf);
				q=enf;
				
				
#line 120 "NexiParser.cpp"
			}
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
		if ( inputState->guessing==0 ) {
#line 207 "nexilang.g"
			
			
			indri::lang::LengthPrior * prior = new indri::lang::LengthPrior(q, 0);
			_nodes.push_back(prior);
			q=prior;
			
			
#line 144 "NexiParser.cpp"
		}
		}
		break;
	}
	case ABOUT:
	case NUMBER:
	case FLOAT:
	case DBL_QUOTE:
	case MINUS:
	case PLUS:
	case TERM:
	{
		{
		c=termList();
		match(ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE);
		if ( inputState->guessing==0 ) {
#line 214 "nexilang.g"
			
			indri::lang::FieldWildcard * wild = new indri::lang::FieldWildcard;
			_nodes.push_back(wild);
			indri::lang::ExtentRestriction * r = new indri::lang::ExtentRestriction(c, wild);
			_nodes.push_back(r);
			
			
			indri::lang::LengthPrior * prior = new indri::lang::LengthPrior(r, 0);
			_nodes.push_back(prior);
			q=prior;
			
			
#line 174 "NexiParser.cpp"
		}
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return q ;
}

 indri::lang::NestedExtentInside*  NexiParser::path() {
#line 252 "nexilang.g"
	 indri::lang::NestedExtentInside* e ;
#line 190 "NexiParser.cpp"
#line 252 "nexilang.g"
	
	indri::lang::RawExtentNode * f = 0;
	indri::lang::RawExtentNode * f2 = 0;
	indri::lang::NestedExtentInside * c;
	e = 0;
	
#line 198 "NexiParser.cpp"
	
	match(SLASH);
	match(SLASH);
	{
	bool synPredMatched66 = false;
	if (((LA(1) == WILD || LA(1) == O_PAREN || LA(1) == TERM))) {
		int _m66 = mark();
		synPredMatched66 = true;
		inputState->guessing++;
		try {
			{
			field();
			match(SLASH);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched66 = false;
		}
		rewind(_m66);
		inputState->guessing--;
	}
	if ( synPredMatched66 ) {
		f=field();
		match(SLASH);
		match(SLASH);
		if ( inputState->guessing==0 ) {
#line 259 "nexilang.g"
			
			c = new indri::lang::NestedExtentInside(NULL, f);
			_nodes.push_back(c);
			e = c;
			
#line 231 "NexiParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			bool synPredMatched69 = false;
			if (((LA(1) == WILD || LA(1) == O_PAREN || LA(1) == TERM))) {
				int _m69 = mark();
				synPredMatched69 = true;
				inputState->guessing++;
				try {
					{
					field();
					match(SLASH);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched69 = false;
				}
				rewind(_m69);
				inputState->guessing--;
			}
			if ( synPredMatched69 ) {
				f2=field();
				match(SLASH);
				match(SLASH);
				if ( inputState->guessing==0 ) {
#line 267 "nexilang.g"
					
					indri::lang::NestedExtentInside * ct = new indri::lang::NestedExtentInside(NULL, f2);
					_nodes.push_back(ct);
					c->setInner(ct);
					c = ct;
					
#line 264 "NexiParser.cpp"
				}
			}
			else {
				goto _loop70;
			}
			
		}
		_loop70:;
		} // ( ... )*
	}
	else if ((LA(1) == WILD || LA(1) == O_PAREN || LA(1) == TERM)) {
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	}
	return e ;
}

 indri::lang::RawExtentNode *  NexiParser::field() {
#line 279 "nexilang.g"
	 indri::lang::RawExtentNode * e ;
#line 288 "NexiParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  fieldName = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  fieldName2 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  fieldName3 = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 279 "nexilang.g"
	
	indri::lang::Field * f = 0;
	indri::lang::ExtentOr * eo = 0;
	e = 0;
	
#line 298 "NexiParser.cpp"
	
	switch ( LA(1)) {
	case WILD:
	{
		match(WILD);
		if ( inputState->guessing==0 ) {
#line 285 "nexilang.g"
			
			e = new indri::lang::FieldWildcard;
			_nodes.push_back(e);
			
#line 310 "NexiParser.cpp"
		}
		break;
	}
	case TERM:
	{
		fieldName = LT(1);
		match(TERM);
		if ( inputState->guessing==0 ) {
#line 290 "nexilang.g"
			
			f = new indri::lang::Field(fieldName->getText());
			_nodes.push_back(f);
			e = f;
			
#line 325 "NexiParser.cpp"
		}
		break;
	}
	case O_PAREN:
	{
		{
		match(O_PAREN);
		fieldName2 = LT(1);
		match(TERM);
		if ( inputState->guessing==0 ) {
#line 295 "nexilang.g"
			
			f = new indri::lang::Field(fieldName2->getText());
			_nodes.push_back(f);
			eo = new indri::lang::ExtentOr;
			_nodes.push_back(eo);
			eo->addChild(f);
			e = eo;
			
#line 345 "NexiParser.cpp"
		}
		{ // ( ... )+
		int _cnt74=0;
		for (;;) {
			if ((LA(1) == 41)) {
				match(41);
				fieldName3 = LT(1);
				match(TERM);
				if ( inputState->guessing==0 ) {
#line 303 "nexilang.g"
					
					f = new indri::lang::Field(fieldName3->getText());
					_nodes.push_back(f);
					eo->addChild(f);
					
#line 361 "NexiParser.cpp"
				}
			}
			else {
				if ( _cnt74>=1 ) { goto _loop74; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt74++;
		}
		_loop74:;
		}  // ( ... )+
		match(C_PAREN);
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return e ;
}

 indri::lang::ScoredExtentNode*  NexiParser::clause() {
#line 313 "nexilang.g"
	 indri::lang::ScoredExtentNode* s ;
#line 387 "NexiParser.cpp"
#line 313 "nexilang.g"
	
	indri::lang::ScoredExtentNode * c = 0;
	indri::lang::UnweightedCombinationNode * l = 0;
	
#line 393 "NexiParser.cpp"
	
	s=filter();
	{
	switch ( LA(1)) {
	case AND:
	case OR:
	{
		l=logical();
		c=filter();
		if ( inputState->guessing==0 ) {
#line 318 "nexilang.g"
			
			l->addChild(s);
			l->addChild(c);
			s = l;
			
#line 410 "NexiParser.cpp"
		}
		break;
	}
	case C_PAREN:
	case C_SQUARE:
	{
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	return s ;
}

 indri::lang::ScoredExtentNode*  NexiParser::termList() {
#line 228 "nexilang.g"
	 indri::lang::ScoredExtentNode* q ;
#line 431 "NexiParser.cpp"
#line 228 "nexilang.g"
	
	indri::lang::CombineNode* c = 0; 
	indri::lang::ScoredExtentNode* s = 0;
	
	
#line 438 "NexiParser.cpp"
	
	q=term();
	{
	switch ( LA(1)) {
	case ABOUT:
	case NUMBER:
	case FLOAT:
	case DBL_QUOTE:
	case MINUS:
	case PLUS:
	case TERM:
	{
		s=term();
		if ( inputState->guessing==0 ) {
#line 235 "nexilang.g"
			
			c = new indri::lang::CombineNode;
			c->addChild(q);
			c->addChild(s);
			_nodes.push_back(c);
			q = c;
			
#line 461 "NexiParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_0.member(LA(1)))) {
				s=term();
				if ( inputState->guessing==0 ) {
#line 243 "nexilang.g"
					
					c->addChild(s);
					
#line 472 "NexiParser.cpp"
				}
			}
			else {
				goto _loop62;
			}
			
		}
		_loop62:;
		} // ( ... )*
		break;
	}
	case ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE:
	case C_PAREN:
	{
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	return q ;
}

 indri::lang::ScoredExtentNode*  NexiParser::term() {
#line 442 "nexilang.g"
	 indri::lang::ScoredExtentNode* t ;
#line 501 "NexiParser.cpp"
#line 442 "nexilang.g"
	
	indri::lang::NotNode * n = 0;
	
#line 506 "NexiParser.cpp"
	
	switch ( LA(1)) {
	case ABOUT:
	case NUMBER:
	case FLOAT:
	case DBL_QUOTE:
	case TERM:
	{
		t=unrestrictedTerm();
		break;
	}
	case PLUS:
	{
		match(PLUS);
		t=unrestrictedTerm();
		break;
	}
	case MINUS:
	{
		match(MINUS);
		t=unrestrictedTerm();
		if ( inputState->guessing==0 ) {
#line 447 "nexilang.g"
			
			n = new indri::lang::NotNode;
			_nodes.push_back(n);
			n->setChild(t);
			t = n;
			
#line 536 "NexiParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return t ;
}

 indri::lang::ScoredExtentNode*  NexiParser::filter() {
#line 336 "nexilang.g"
	 indri::lang::ScoredExtentNode* s;
#line 551 "NexiParser.cpp"
#line 336 "nexilang.g"
	
	indri::lang::RawExtentNode * a = 0;
	indri::lang::RawExtentNode * contexts = 0;  
	
#line 557 "NexiParser.cpp"
	
	switch ( LA(1)) {
	case ABOUT:
	{
		s=aboutClause();
		break;
	}
	case 42:
	{
		a=arithmeticClause();
		if ( inputState->guessing==0 ) {
#line 341 "nexilang.g"
			
			
			if ( _shrinkage == true ) {
			indri::lang::DocumentStructureNode * d = new indri::lang::DocumentStructureNode;
			_nodes.push_back( d );
			s = new indri::lang::ShrinkageScorerNode( a, d );
			_nodes.push_back( s );
			} else {
			s = new indri::lang::NestedRawScorerNode( a, contexts );
			_nodes.push_back( s );
			}
			
			indri::lang::MaxNode * m = new indri::lang::MaxNode;
			_nodes.push_back(m);
			m->addChild(s);
			s = m;        
			
			
#line 588 "NexiParser.cpp"
		}
		break;
	}
	case O_PAREN:
	{
		s=filterParens();
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return s;
}

 indri::lang::UnweightedCombinationNode*  NexiParser::logical() {
#line 325 "nexilang.g"
	 indri::lang::UnweightedCombinationNode* s;
#line 608 "NexiParser.cpp"
	
	switch ( LA(1)) {
	case AND:
	{
		match(AND);
		if ( inputState->guessing==0 ) {
#line 326 "nexilang.g"
			
			s = new indri::lang::CombineNode;
			_nodes.push_back(s);
			
#line 620 "NexiParser.cpp"
		}
		break;
	}
	case OR:
	{
		match(OR);
		if ( inputState->guessing==0 ) {
#line 330 "nexilang.g"
			
			s = new indri::lang::OrNode;
			_nodes.push_back(s);
			
#line 633 "NexiParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return s;
}

 indri::lang::ScoredExtentNode*  NexiParser::aboutClause() {
#line 367 "nexilang.g"
	 indri::lang::ScoredExtentNode* s ;
#line 648 "NexiParser.cpp"
#line 367 "nexilang.g"
	
	indri::lang::NestedExtentInside * p = 0;
	indri::lang::ScoredExtentNode * t = 0;
	indri::lang::RawExtentNode * f = 0;
	indri::lang::ExtentRestriction * r = 0;
	
#line 656 "NexiParser.cpp"
	
	match(ABOUT);
	match(O_PAREN);
	match(42);
	{
	switch ( LA(1)) {
	case SLASH:
	{
		{
		p=path();
		f=field();
		match(43);
		t=termList();
		}
		if ( inputState->guessing==0 ) {
#line 373 "nexilang.g"
			
			if (p != 0) { 
			indri::lang::NestedExtentInside * pt = p;
			while (pt->getInner() != NULL) {
			pt = (indri::lang::NestedExtentInside *) pt->getInner();    
			}    
			pt->setInner(f);                    
			r = new indri::lang::ExtentRestriction(t, p);
			} else {
			r = new indri::lang::ExtentRestriction(t, f);
			}
			_nodes.push_back(r);
			indri::lang::MaxNode * m = new indri::lang::MaxNode;
			_nodes.push_back(m);
			m->addChild(r);
			s = m;   
			
#line 690 "NexiParser.cpp"
		}
		break;
	}
	case 43:
	{
		{
		match(43);
		s=termList();
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	match(C_PAREN);
	return s ;
}

 indri::lang::RawExtentNode *  NexiParser::arithmeticClause() {
#line 395 "nexilang.g"
	 indri::lang::RawExtentNode * s;
#line 715 "NexiParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  field = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 395 "nexilang.g"
	
	indri::lang::Field* f = 0;
	indri::lang::RawExtentNode* c = 0;
	indri::lang::NestedExtentInside* p = 0;
	INT64 n = 0;
	s = 0;
	
#line 725 "NexiParser.cpp"
	
	match(42);
	p=path();
	field = LT(1);
	match(TERM);
	if ( inputState->guessing==0 ) {
#line 404 "nexilang.g"
		
		f = new indri::lang::Field(field->getText());
		_nodes.push_back(f);
		
#line 737 "NexiParser.cpp"
	}
	{
	switch ( LA(1)) {
	case LESS:
	{
		match(LESS);
		n=number();
		if ( inputState->guessing==0 ) {
#line 408 "nexilang.g"
			
			c = new indri::lang::FieldLessNode(f, n);
			_nodes.push_back(c);
			
#line 751 "NexiParser.cpp"
		}
		break;
	}
	case LESSEQ:
	{
		match(LESSEQ);
		n=number();
		if ( inputState->guessing==0 ) {
#line 412 "nexilang.g"
			
			c = new indri::lang::FieldLessNode(f, n + 1);
			_nodes.push_back(c);
			
#line 765 "NexiParser.cpp"
		}
		break;
	}
	case EQUALS:
	{
		match(EQUALS);
		n=number();
		if ( inputState->guessing==0 ) {
#line 416 "nexilang.g"
			
			c = new indri::lang::FieldEqualsNode(f, n);
			_nodes.push_back(c);
			
#line 779 "NexiParser.cpp"
		}
		break;
	}
	case GREATER:
	{
		match(GREATER);
		n=number();
		if ( inputState->guessing==0 ) {
#line 420 "nexilang.g"
			
			c = new indri::lang::FieldGreaterNode(f, n);
			_nodes.push_back(c);
			
#line 793 "NexiParser.cpp"
		}
		break;
	}
	case GREATEREQ:
	{
		match(GREATEREQ);
		n=number();
		if ( inputState->guessing==0 ) {
#line 424 "nexilang.g"
			
			c = new indri::lang::FieldGreaterNode(f, n - 1);
			_nodes.push_back(c);
			
#line 807 "NexiParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
	if ( inputState->guessing==0 ) {
#line 427 "nexilang.g"
		
		if (p != 0) {
		indri::lang::NestedExtentInside * pt = p;
		while (pt->getInner() != NULL) {
		pt = (indri::lang::NestedExtentInside *) pt->getInner();    
		}    
		pt->setInner(c);
		s = p;
		} else {
		s = c;
		}
		
#line 831 "NexiParser.cpp"
	}
	return s;
}

 indri::lang::ScoredExtentNode*  NexiParser::filterParens() {
#line 362 "nexilang.g"
	 indri::lang::ScoredExtentNode* s ;
#line 839 "NexiParser.cpp"
	
	match(O_PAREN);
	s=clause();
	match(C_PAREN);
	return s ;
}

 INT64  NexiParser::number() {
#line 528 "nexilang.g"
	 INT64 v ;
#line 850 "NexiParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 528 "nexilang.g"
	
	v = 0;
	
#line 856 "NexiParser.cpp"
	
	n = LT(1);
	match(NUMBER);
	if ( inputState->guessing==0 ) {
#line 531 "nexilang.g"
		
		v = string_to_i64(n->getText());
		
#line 865 "NexiParser.cpp"
	}
	return v ;
}

 indri::lang::ScoredExtentNode *  NexiParser::unrestrictedTerm() {
#line 455 "nexilang.g"
	 indri::lang::ScoredExtentNode * t ;
#line 873 "NexiParser.cpp"
#line 455 "nexilang.g"
	
	indri::lang::RawExtentNode * raw = 0;
	indri::lang::RawExtentNode * contexts = 0;
	t = 0;
	
#line 880 "NexiParser.cpp"
	
	switch ( LA(1)) {
	case ABOUT:
	case NUMBER:
	case FLOAT:
	case TERM:
	{
		raw=rawText();
		if ( inputState->guessing==0 ) {
#line 461 "nexilang.g"
			
			if ( _shrinkage == true ) {
			indri::lang::DocumentStructureNode * d = new indri::lang::DocumentStructureNode;
			_nodes.push_back( d );
			t = new indri::lang::ShrinkageScorerNode( raw, d );
			_nodes.push_back( t );
			} else {
			t = new indri::lang::NestedRawScorerNode( raw, contexts );
			_nodes.push_back( t );
			}
			
#line 902 "NexiParser.cpp"
		}
		break;
	}
	case DBL_QUOTE:
	{
		match(DBL_QUOTE);
		raw=odNode();
		match(DBL_QUOTE);
		if ( inputState->guessing==0 ) {
#line 472 "nexilang.g"
			
			
			if ( _shrinkage == true ) {
			indri::lang::DocumentStructureNode * d = new indri::lang::DocumentStructureNode;
			_nodes.push_back( d );
			t = new indri::lang::ShrinkageScorerNode( raw, d );
			_nodes.push_back( t );
			} else {
			t = new indri::lang::NestedRawScorerNode( raw, contexts );
			_nodes.push_back( t );
			}
			
#line 925 "NexiParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return t ;
}

 indri::lang::RawExtentNode*  NexiParser::rawText() {
#line 489 "nexilang.g"
	 indri::lang::RawExtentNode* t ;
#line 940 "NexiParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  ab = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 489 "nexilang.g"
	
	t = 0;
	
#line 949 "NexiParser.cpp"
	
	switch ( LA(1)) {
	case TERM:
	{
		id = LT(1);
		match(TERM);
		if ( inputState->guessing==0 ) {
#line 492 "nexilang.g"
			
			std::string text = id->getText();
			
			//     std::string::iterator c = text.begin();
			//     while (c != text.end()) { *c = tolower(*c); c++; }
			
			t = new indri::lang::IndexTerm(id->getText());
			_nodes.push_back(t);
			
#line 967 "NexiParser.cpp"
		}
		break;
	}
	case ABOUT:
	{
		ab = LT(1);
		match(ABOUT);
		if ( inputState->guessing==0 ) {
#line 501 "nexilang.g"
			
			std::string text = ab->getText();
			t = new indri::lang::IndexTerm(ab->getText());
			_nodes.push_back(t);
			
#line 982 "NexiParser.cpp"
		}
		break;
	}
	case NUMBER:
	{
		n = LT(1);
		match(NUMBER);
		if ( inputState->guessing==0 ) {
#line 506 "nexilang.g"
			
			t = new indri::lang::IndexTerm(n->getText());
			_nodes.push_back(t);
			
#line 996 "NexiParser.cpp"
		}
		break;
	}
	case FLOAT:
	{
		f = LT(1);
		match(FLOAT);
		if ( inputState->guessing==0 ) {
#line 510 "nexilang.g"
			
			t = new indri::lang::IndexTerm(f->getText());
			_nodes.push_back(t);
			
#line 1010 "NexiParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return t ;
}

 indri::lang::ODNode*  NexiParser::odNode() {
#line 516 "nexilang.g"
	 indri::lang::ODNode* od ;
#line 1025 "NexiParser.cpp"
#line 516 "nexilang.g"
	
	RawExtentNode* t = 0;
	od = new indri::lang::ODNode;
	_nodes.push_back(od);
	
#line 1032 "NexiParser.cpp"
	
	{ // ( ... )+
	int _cnt91=0;
	for (;;) {
		if ((_tokenSet_1.member(LA(1)))) {
			t=rawText();
			if ( inputState->guessing==0 ) {
#line 523 "nexilang.g"
				
				od->addChild(t);
				
#line 1044 "NexiParser.cpp"
			}
		}
		else {
			if ( _cnt91>=1 ) { goto _loop91; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt91++;
	}
	_loop91:;
	}  // ( ... )+
	return od ;
}

void NexiParser::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& )
{
}
const char* NexiParser::tokenNames[] = {
	"<0>",
	"EOF",
	"<2>",
	"NULL_TREE_LOOKAHEAD",
	"\"about\"",
	"\"AND\"",
	"\"OR\"",
	"\"*\"",
	"NUMBER",
	"NEGATIVE_NUMBER",
	"FLOAT",
	"STAR",
	"O_PAREN",
	"C_PAREN",
	"O_SQUARE",
	"C_SQUARE",
	"DBL_QUOTE",
	"QUOTE",
	"DOT",
	"COMMA",
	"SLASH",
	"MINUS",
	"PLUS",
	"TAB",
	"CR",
	"LF",
	"SPACE",
	"HIGH_CHAR",
	"DIGIT",
	"ASCII_LETTER",
	"SAFE_LETTER",
	"SAFE_CHAR",
	"TEXT_TERM",
	"TERM",
	"OPERATORS",
	"LESS",
	"GREATER",
	"LESSEQ",
	"GREATEREQ",
	"EQUALS",
	"JUNK",
	"\"|\"",
	"\".\"",
	"\",\"",
	0
};

const unsigned long NexiParser::_tokenSet_0_data_[] = { 6358288UL, 2UL, 0UL, 0UL };
// "about" NUMBER FLOAT DBL_QUOTE MINUS PLUS TERM 
const ANTLR_USE_NAMESPACE(antlr)BitSet NexiParser::_tokenSet_0(_tokenSet_0_data_,4);
const unsigned long NexiParser::_tokenSet_1_data_[] = { 1296UL, 2UL, 0UL, 0UL };
// "about" NUMBER FLOAT TERM 
const ANTLR_USE_NAMESPACE(antlr)BitSet NexiParser::_tokenSet_1(_tokenSet_1_data_,4);


ANTLR_END_NAMESPACE
ANTLR_END_NAMESPACE
