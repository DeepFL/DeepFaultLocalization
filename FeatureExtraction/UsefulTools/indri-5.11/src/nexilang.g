/*==========================================================================
 * Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/

//
// IndriQueryLexer
//
// 17 February 2004 -- tds
//

header "pre_include_hpp" {
  #include "indri/QuerySpec.hpp"
  #include "indri/DateParse.hpp"
  #include "indri/delete_range.hpp"
  #include "indri/NexiLexer.hpp"
  #include <algorithm>
  #include <cctype>
}

options {
  language = "Cpp";
  namespace = "indri::lang";
}

class NexiLexer extends Lexer;

options {
  k=2;
  charVocabulary = '\u0001'..'\u00ff'; // UTF-8 format
//  testLiterals = false;
  defaultErrorHandler = false;

}

tokens {
  // NEXI operators

  ABOUT = "about";
  AND = "AND";
  OR = "OR";

  // wildcard 
  WILD = "*";
  
  // pseudo-tokens
  NUMBER;
  NEGATIVE_NUMBER;
  FLOAT;
}

{
private:
  bool _numbers;

public:
  void init() {
    _numbers = false;
  }

  void setNumbers(bool f) {
    _numbers = f;
  } 
} 

STAR:     '*';
O_PAREN:  '(';
C_PAREN:  ')';
O_SQUARE: '[';
C_SQUARE: ']';
DBL_QUOTE: '\"';
QUOTE:     '\'';
DOT:       '.';
COMMA:     ',';
SLASH:     '/';
MINUS:     '-';
PLUS:      '+';


protected TAB:       '\t';
protected CR:        '\n';
protected LF:        '\r';
protected SPACE:     ' ';

protected HIGH_CHAR:         '\u0080'..'\u00ff';
protected DIGIT:             ('0'..'9');
protected ASCII_LETTER:      ('a'..'z' | 'A'..'Z');
protected SAFE_LETTER:       ('a'..'z' | 'A'..'Z' | '-' | '_');
protected SAFE_CHAR:         ('a'..'z' | 'A'..'Z' | '0'..'9' | '-' | '_');

//
// Within the ASCII range, we only accept a restricted
// set of characters (letters and numbers).  However,
// we allow any Unicode character (composed of high
// chars) so that we can support UTF-8 input.
//

protected TEXT_TERM:        ( HIGH_CHAR | ASCII_LETTER | DIGIT ) ( HIGH_CHAR | SAFE_CHAR )*;
protected NUMBER:           ( '0'..'9' )+;
// protected NEGATIVE_NUMBER:  MINUS ( '0'..'9' )+;
protected FLOAT:            ( '0'..'9' )+ DOT ( '0'..'9' )*;

TERM:     ( (DIGIT)+ SAFE_LETTER ) => TEXT_TERM |
          ( NUMBER DOT ) => FLOAT { $setType(FLOAT); } |
          ( NUMBER ) => NUMBER { $setType(NUMBER); } |
          TEXT_TERM;

OPERATORS: '#' TERM;

//numeric operators
LESS:      "<";
GREATER:   ">";
LESSEQ:    "<=";
GREATEREQ: ">=";
EQUALS:    "=";

JUNK:      ( TAB | CR | LF | SPACE )
           { $setType(antlr::Token::SKIP); };
     
class NexiParser extends Parser;
options {
  defaultErrorHandler = false;
}

{
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
}


query returns [ indri::lang::ScoredExtentNode* q ] {
    indri::lang::ScoredExtentNode * c = 0;
    indri::lang::NestedExtentInside * p = 0;
    indri::lang::RawExtentNode * f = 0;
    indri::lang::ScoredExtentNode * c2 = 0;
    indri::lang::NestedExtentInside * p2 = 0;
    indri::lang::RawExtentNode * f2 = 0;
  } :
  (p=path f=field O_SQUARE c=clause C_SQUARE {

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
  } 
  ( p2=path f2=field O_SQUARE c2=clause C_SQUARE { 
    
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

  } 
)? EOF {

    indri::lang::LengthPrior * prior = new indri::lang::LengthPrior(q, 0);
    _nodes.push_back(prior);
    q=prior;

} ) 
| ( c=termList EOF {
    indri::lang::FieldWildcard * wild = new indri::lang::FieldWildcard;
    _nodes.push_back(wild);
    indri::lang::ExtentRestriction * r = new indri::lang::ExtentRestriction(c, wild);
    _nodes.push_back(r);


    indri::lang::LengthPrior * prior = new indri::lang::LengthPrior(r, 0);
    _nodes.push_back(prior);
    q=prior;

  } )
;

termList returns [ indri::lang::ScoredExtentNode* q ] {
    indri::lang::CombineNode* c = 0; 
    indri::lang::ScoredExtentNode* s = 0;
   
  } :
  q=term
  ( options { greedy=true; } : s=term
    {
      c = new indri::lang::CombineNode;
      c->addChild(q);
      c->addChild(s);
      _nodes.push_back(c);
      q = c;
    }
    ( options { greedy=true; } : s=term
      {
        c->addChild(s);
      }
    )*
  )?;



// consumes all but last field
path returns [ indri::lang::NestedExtentInside* e ] {
    indri::lang::RawExtentNode * f = 0;
    indri::lang::RawExtentNode * f2 = 0;
    indri::lang::NestedExtentInside * c;
    e = 0;
  } :
  SLASH SLASH ((field SLASH) => f=field SLASH SLASH
  { 
    c = new indri::lang::NestedExtentInside(NULL, f);
    _nodes.push_back(c);
    e = c;
  }
  (options { greedy=true; } :  
      (field SLASH) =>
      f2=field SLASH SLASH
      {
        indri::lang::NestedExtentInside * ct = new indri::lang::NestedExtentInside(NULL, f2);
        _nodes.push_back(ct);
        c->setInner(ct);
        c = ct;
      }
  )*
)?
;



field returns [ indri::lang::RawExtentNode * e ] 
  { 
    indri::lang::Field * f = 0;
    indri::lang::ExtentOr * eo = 0;
    e = 0;
  } :
  WILD {
    e = new indri::lang::FieldWildcard;
    _nodes.push_back(e);
  }
| fieldName:TERM
  {
    f = new indri::lang::Field(fieldName->getText());
    _nodes.push_back(f);
    e = f;
  } 
|( O_PAREN fieldName2:TERM {
    f = new indri::lang::Field(fieldName2->getText());
    _nodes.push_back(f);
    eo = new indri::lang::ExtentOr;
    _nodes.push_back(eo);
    eo->addChild(f);
    e = eo;
  }
  (options { greedy=true; } :  "|" fieldName3:TERM {
    f = new indri::lang::Field(fieldName3->getText());
    _nodes.push_back(f);
    eo->addChild(f);
  } )+
  C_PAREN )  
;



clause returns [ indri::lang::ScoredExtentNode* s ] 
  {
    indri::lang::ScoredExtentNode * c = 0;
    indri::lang::UnweightedCombinationNode * l = 0;
  } :
  s=filter (options { greedy=true; } : l=logical c=filter {
    l->addChild(s);
    l->addChild(c);
    s = l;
  })?
;

logical returns [ indri::lang::UnweightedCombinationNode* s] :
  AND {
    s = new indri::lang::CombineNode;
    _nodes.push_back(s);
  }
| OR {
    s = new indri::lang::OrNode;
    _nodes.push_back(s);
  }
;

filter returns [ indri::lang::ScoredExtentNode* s] {
  indri::lang::RawExtentNode * a = 0;
  indri::lang::RawExtentNode * contexts = 0;  
} :
  s=aboutClause
| a=arithmeticClause {

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

  }
| s=filterParens
;

filterParens returns [ indri::lang::ScoredExtentNode* s ] :
  O_PAREN s=clause C_PAREN
;


aboutClause returns [ indri::lang::ScoredExtentNode* s ] {
    indri::lang::NestedExtentInside * p = 0;
    indri::lang::ScoredExtentNode * t = 0;
    indri::lang::RawExtentNode * f = 0;
    indri::lang::ExtentRestriction * r = 0;
  } :
  ABOUT O_PAREN "." ((p=path f=field "," t=termList)  {
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
  }
  | ("," s=termList)) C_PAREN
;



arithmeticClause returns [ indri::lang::RawExtentNode * s] 
  {
    indri::lang::Field* f = 0;
    indri::lang::RawExtentNode* c = 0;
    indri::lang::NestedExtentInside* p = 0;
    INT64 n = 0;
    s = 0;
  } :
  "." p=path field:TERM 
  {
    f = new indri::lang::Field(field->getText());
    _nodes.push_back(f);
  }
  ( LESS n=number {
    c = new indri::lang::FieldLessNode(f, n);
    _nodes.push_back(c);
  }
  | LESSEQ n=number {
    c = new indri::lang::FieldLessNode(f, n + 1);
    _nodes.push_back(c);
  }
  | EQUALS n=number {
    c = new indri::lang::FieldEqualsNode(f, n);
    _nodes.push_back(c);
  }
  | GREATER n=number {
    c = new indri::lang::FieldGreaterNode(f, n);
    _nodes.push_back(c);
  }
  | GREATEREQ n=number {
    c = new indri::lang::FieldGreaterNode(f, n - 1);
    _nodes.push_back(c);
  }) {
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
  }
;


term returns [ indri::lang::ScoredExtentNode* t ] {
    indri::lang::NotNode * n = 0;
  } :
  t=unrestrictedTerm 
| PLUS t=unrestrictedTerm
| MINUS t=unrestrictedTerm {
    n = new indri::lang::NotNode;
    _nodes.push_back(n);
    n->setChild(t);
    t = n;
  }
;

unrestrictedTerm returns [ indri::lang::ScoredExtentNode * t ] 
{ 
  indri::lang::RawExtentNode * raw = 0;
  indri::lang::RawExtentNode * contexts = 0;
  t = 0;
}:
  raw=rawText {
    if ( _shrinkage == true ) {
      indri::lang::DocumentStructureNode * d = new indri::lang::DocumentStructureNode;
      _nodes.push_back( d );
      t = new indri::lang::ShrinkageScorerNode( raw, d );
      _nodes.push_back( t );
    } else {
      t = new indri::lang::NestedRawScorerNode( raw, contexts );
      _nodes.push_back( t );
    }
  }
| DBL_QUOTE raw=odNode DBL_QUOTE {

    if ( _shrinkage == true ) {
      indri::lang::DocumentStructureNode * d = new indri::lang::DocumentStructureNode;
      _nodes.push_back( d );
      t = new indri::lang::ShrinkageScorerNode( raw, d );
      _nodes.push_back( t );
    } else {
      t = new indri::lang::NestedRawScorerNode( raw, contexts );
      _nodes.push_back( t );
    }
  }
;
 
  

// rawText is something that can be considered a query term
rawText returns [ indri::lang::RawExtentNode* t ] {
    t = 0;
  } :
  id:TERM {
    std::string text = id->getText();
    t = new indri::lang::IndexTerm(id->getText());
    _nodes.push_back(t);
  } |
  ab:ABOUT {
    std::string text = ab->getText();
    t = new indri::lang::IndexTerm(ab->getText());
    _nodes.push_back(t);
  } |
  n:NUMBER {
    t = new indri::lang::IndexTerm(n->getText());
    _nodes.push_back(t);
  } |
  f:FLOAT {
    t = new indri::lang::IndexTerm(f->getText());
    _nodes.push_back(t);
  } 
;

odNode returns [ indri::lang::ODNode* od ] 
  {
    RawExtentNode* t = 0;
    od = new indri::lang::ODNode;
    _nodes.push_back(od);
  } :
  ( options { greedy=true; } : t=rawText
    {
      od->addChild(t);
    }    
  )+
;
number returns [ INT64 v ] {
    v = 0;
  } :
  n:NUMBER {
    v = string_to_i64(n->getText());
  }
;
