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
  #include "indri/QueryLexer.hpp"
}

options {
  language = "Cpp";
  namespace = "indri::lang";
}

class QueryLexer extends Lexer;

options {
  k=2;
  charVocabulary = '\u0001'..'\u00ff'; // UTF-8 format
  testLiterals = false;
  defaultErrorHandler = false;
}

tokens {
  // core operators
  SUM = "#sum";
  WSUM = "#wsum";
  WAND = "#wand";
  OD = "#od";
  OR = "#or";
  NOT = "#not";
  UW = "#uw";
  COMBINE = "#combine";
  WEIGHT = "#weight";
  MAX = "#max";
  FILREQ = "#filreq";
  FILREJ = "#filrej";
  SCOREIF = "#scoreif";
  SCOREIFNOT = "#scoreifnot";
  ANY = "#any";
  BAND = "#band";
  WSYN = "#wsyn";
  SYN = "#syn";
  // numerics
  PRIOR = "#prior";
  DATEAFTER = "#dateafter";
  DATEBEFORE = "#datebefore";
  DATEBETWEEN = "#datebetween";
  DATEEQUALS = "#dateequals";
  LESS = "#less";
  GREATER = "#greater";
  BETWEEN = "#between";
  EQUALS = "#equals";
  WCARD = "#wildcard";
  
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
O_ANGLE:  '<';
C_ANGLE:  '>';
O_SQUARE: '[';
C_SQUARE: ']';
O_BRACE:  '{';
C_BRACE:   '}';
DBL_QUOTE: '\"';
QUOTE:     '\'';
DOT:       '.';
COMMA:     ',';
SLASH:     '/';
B_SLASH:   '\\';

DASH:      '-';
SPACE_DASH: " -";
COLON:     ':';

protected TAB:       '\t';
protected CR:        '\n';
protected LF:        '\r';
protected SPACE:     ' ';

protected HIGH_CHAR:         '\u0080'..'\u00ff';
protected DIGIT:             ('0'..'9');
protected ASCII_LETTER:      ('a'..'z' | 'A'..'Z');
protected SAFE_LETTER:       ('a'..'z' | 'A'..'Z' | '_');
protected SAFE_CHAR:         ('a'..'z' | 'A'..'Z' | '0'..'9' | '_');
protected BASESIXFOUR_CHAR:  ('a'..'z' | 'A'..'Z' | '0'..'9' | '+' | '/' | '=');

//
// Within the ASCII range, we only accept a restricted
// set of characters (letters and numbers).  However,
// we allow any Unicode character (composed of high
// chars) so that we can support UTF-8 input.
//

protected TEXT_TERM:        ( HIGH_CHAR | SAFE_CHAR )+;
protected NUMBER:           ( '0'..'9' )+;
protected FLOAT:            ( '0'..'9' )+ DOT ( '0'..'9' )+;

TERM:   ( (DIGIT)+ (SAFE_LETTER | HIGH_CHAR) ) => TEXT_TERM |
        ( FLOAT ) => FLOAT { $setType(FLOAT); } |
        ( NUMBER ) => NUMBER { $setType(NUMBER); } |
        TEXT_TERM;

protected ENCODED_QUOTED_TERM:    "#base64quote"! O_PAREN! (TAB! | SPACE!)* (BASESIXFOUR_CHAR)+ (TAB! | SPACE!)* C_PAREN!;
protected ENCODED_TERM:           "#base64"! O_PAREN! (TAB! | SPACE!)* (BASESIXFOUR_CHAR)+ (TAB! | SPACE!)* C_PAREN!;

OPERATOR
  options { testLiterals = true; }:
    ( "#base64quote" ) => ENCODED_QUOTED_TERM { $setType(ENCODED_QUOTED_TERM); } |
    ( "#base64" ) => ENCODED_TERM { $setType(ENCODED_TERM); } |
    ('#' ASCII_LETTER) => '#' (ASCII_LETTER)+ |
    '#' ;

JUNK:      ( TAB | CR | LF | SPACE )
           { $setType(antlr::Token::SKIP); };
     
class QueryParser extends Parser;
options {
  defaultErrorHandler = false;
  k=2;
}

{
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
}

query returns [ indri::lang::ScoredExtentNode* q ] {
    indri::lang::CombineNode* c = 0;
    indri::lang::ScoredExtentNode* s = 0;
    q = 0;
  } :
  q=scoredExtentNode[0]
  ( options { greedy=true; } : s=scoredExtentNode[0]
    {
      c = new CombineNode;
      c->addChild(q);
      c->addChild(s);
      _nodes.push_back(c);
      q = c;
    }
    ( options { greedy=true; } : s=scoredExtentNode[0]
      {
        c->addChild(s);
      }
    )*
  )? EOF;

scoredExtentNode [ indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* s ] :
    ( WEIGHT ) => s=weightNode[ou]
  | ( COMBINE ) => s=combineNode[ou]
  | ( OR ) => s=orNode[ou]
  | ( NOT ) => s=notNode[ou]
  | ( WAND ) => s=wandNode[ou]
  | ( WSUM ) => s=wsumNode[ou]
  | ( MAX ) => s=maxNode[ou]
  | ( PRIOR ) => s=priorNode
  | ( FILREJ ) => s=filrejNode[ou]
  | ( FILREQ ) => s=filreqNode[ou]
  | ( SCOREIF ) => s=scoreifNode[ou]
  | ( SCOREIFNOT ) => s=scoreifnotNode[ou]
  | s=scoredRaw[ou]
  ;

scoredRaw [ indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* sn ]
  {
    RawExtentNode* raw = 0;
    RawExtentNode* contexts = 0;
    sn = 0;
  } :
  ( qualifiedTerm DOT ) => raw=qualifiedTerm DOT contexts=context_list[ou]
  {
    sn = new indri::lang::RawScorerNode( raw, contexts );
    _nodes.push_back(sn);
  }
  | ( qualifiedTerm ) => raw=qualifiedTerm
  {
    sn = new indri::lang::RawScorerNode( raw, contexts );
    _nodes.push_back(sn);
  }
  | ( unqualifiedTerm DOT ) => raw=unqualifiedTerm DOT contexts=context_list[ou]
  {
    sn = new indri::lang::RawScorerNode( raw, contexts );
    _nodes.push_back(sn);
  }
  | raw=unqualifiedTerm
  {
    sn = new indri::lang::RawScorerNode( raw, contexts );
    _nodes.push_back(sn);
  };

//
// Score operators start here:
//    #weight = weightNode
//    #combine = combineNode
//    #max = maxNode
//    #wsum = wsumNode
//    #wand = wandNode
//    #or = orNode
//    #not = notNode
//

weightedList[ indri::lang::WeightedCombinationNode* wn, indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* sr ] 
  {
    double w = 0;
    ScoredExtentNode* n = 0;
    sr = wn;
  } :
  ( sr=extentRestriction[wn, ou] { ou = innerMost(sr); } )? 
  O_PAREN 
    (
      w=floating
      n=scoredExtentNode[ou]
      { wn->addChild( w, n ); }
    )+
  C_PAREN
  ;

sumList[ indri::lang::WSumNode* wn, indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* sr ] 
  {
    double w = 0;
    ScoredExtentNode* n = 0;
    sr = wn;
  } :
  ( sr=extentRestriction[wn, ou] { ou = innerMost(sr); } )?
  O_PAREN 
    ( options { greedy=true; } : n=scoredExtentNode[ou] { wn->addChild( 1.0, n ); } )+
  C_PAREN
  ;

unweightedList[ indri::lang::UnweightedCombinationNode* cn, indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* sr ]
  {
    ScoredExtentNode* n = 0;
    sr = cn;
  } :
  ( sr=extentRestriction[cn, ou] { ou = innerMost(sr); } )?
  O_PAREN
    ( options { greedy=true; } : n=scoredExtentNode[ou] { cn->addChild( n ); } )+
  C_PAREN
  ;

weightNode [indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* r ] 
  {
    indri::lang::WeightNode* wn = new indri::lang::WeightNode;
    _nodes.push_back(wn);
  } :
  WEIGHT r=weightedList[wn, ou];

combineNode [indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* r ]
  { 
    indri::lang::CombineNode* cn = new indri::lang::CombineNode;
    _nodes.push_back(cn);
  } :
  COMBINE r=unweightedList[cn, ou];

sumNode [indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* r ]
  {
    indri::lang::WSumNode* wn = new indri::lang::WSumNode;
    _nodes.push_back(wn);
  } :
  SUM r=sumList[wn, ou];

wsumNode [indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* r ] 
  {
    indri::lang::WSumNode* wn = new indri::lang::WSumNode;
    _nodes.push_back(wn);
  } :
  WSUM r=weightedList[wn, ou];

wandNode [indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* r ]
  {
    indri::lang::WAndNode* wn = new indri::lang::WAndNode;
    _nodes.push_back(wn);
  } :
  WAND r=weightedList[wn, ou];
  
orNode [indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* r ]
  {
    indri::lang::OrNode* on = new indri::lang::OrNode;
    _nodes.push_back(on);
  } :
  OR r=unweightedList[on, ou];

maxNode [indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* r ]
  {
    indri::lang::MaxNode* mn = new indri::lang::MaxNode;
    _nodes.push_back(mn);
  } :
  MAX r=unweightedList[mn, ou];
  
notNode [indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* r ]
  {
    indri::lang::NotNode* n = new indri::lang::NotNode;
    indri::lang::ScoredExtentNode* c = 0;
    _nodes.push_back(n);
    r = n;
  } :
  NOT (r=extentRestriction[r, ou])? O_PAREN c=scoredExtentNode[ou] C_PAREN
  {
    n->setChild(c);
  };
  
priorNode returns [ indri::lang::PriorNode* p ]
  {
    indri::lang::Field* field = 0;
    p = 0;
  } :
  PRIOR O_PAREN name:TERM C_PAREN {
    p = new indri::lang::PriorNode( name->getText() );
    _nodes.push_back(p);
  };
  
//
// Extent operators start here:
//    #wsyn = wsynNode
//    #odn = odNode
//    #uwn = uwNode
//    #band = bandNode
//    #filrej = filrejNode
//    #filreq = filreqNode
//
  
// wsynNode : WSYN O_PAREN ( weight unscoredTerm )+ C_PAREN
wsynNode returns [ indri::lang::WeightedExtentOr* ws ]
  {
    ws = new indri::lang::WeightedExtentOr;
    _nodes.push_back(ws);

    double w = 0;
    RawExtentNode* n = 0;
  } :
  WSYN O_PAREN
       ( options { greedy=true; } : w=floating n=unscoredTerm { ws->addChild( w, n ); } )+
       C_PAREN;
  
// odNode : OD DECIMAL O_PAREN ( qualifiedTerm )+ C_PAREN
odNode returns [ indri::lang::ODNode* od ] 
  {
    RawExtentNode* rn = 0;
    od = new indri::lang::ODNode;
    _nodes.push_back(od);
  } :
  // operator
  (
    // #od5 syntax
      (OD NUMBER) => (OD n1:NUMBER  { od->setWindowSize( n1->getText() ); } )
    // #od( term ) syntax
    | (OD) => (OD)
    // #5 syntax
    | (OPERATOR n2:NUMBER { od->setWindowSize( n2->getText() ); } )
  )
  // contents
  O_PAREN
    ( options { greedy=true; } : rn=unscoredTerm { od->addChild( rn ); } )+
  C_PAREN;

// uwNode : UW DECIMAL O_PAREN ( qualifiedTerm )+ C_PAREN
uwNode returns [ indri::lang::UWNode* uw ]
  {
    uw = new indri::lang::UWNode;
    RawExtentNode* rn = 0;
    _nodes.push_back(uw);
  } :
  (
      // operator (#uw2)
      (UW NUMBER) => (UW n:NUMBER { uw->setWindowSize( n->getText() ); } )
      // operator #uw( term )
    | (UW)
  )
  // contents
  O_PAREN
    ( options { greedy=true; } : rn=unscoredTerm { uw->addChild( rn ); } )+
  C_PAREN;

bandNode returns [ indri::lang::BAndNode* b ] 
  {
    b = new indri::lang::BAndNode;
    RawExtentNode* rn = 0;
    _nodes.push_back(b);
  } :
  BAND
  O_PAREN
    ( options { greedy=true; } : rn=unscoredTerm { b->addChild( rn ); } )+
  C_PAREN;
  
filrejNode [ indri::lang::RawExtentNode * ou ] returns [ indri::lang::FilRejNode* fj ]
  {
    RawExtentNode* filter = 0;
    ScoredExtentNode* disallowed = 0;
  } :
  FILREJ
  O_PAREN filter=unscoredTerm disallowed=scoredExtentNode[ou] C_PAREN {
    fj = new FilRejNode( filter, disallowed );
    _nodes.push_back(fj);
  }; 
  
filreqNode[ indri::lang::RawExtentNode * ou ] returns [ indri::lang::FilReqNode* fq ]
  {
    RawExtentNode* filter = 0;
    ScoredExtentNode* required = 0;
  } :
  FILREQ
  O_PAREN filter=unscoredTerm required=scoredExtentNode[ou] C_PAREN {
    fq = new FilReqNode( filter, required );
    _nodes.push_back(fq);
  }; 

scoreifnotNode [ indri::lang::RawExtentNode * ou ] returns [ indri::lang::FilRejNode* fj ]
  {
    RawExtentNode* filter = 0;
    ScoredExtentNode* disallowed = 0;
  } :
  SCOREIFNOT
  O_PAREN filter=unscoredTerm disallowed=scoredExtentNode[ou] C_PAREN {
    fj = new FilRejNode( filter, disallowed );
    _nodes.push_back(fj);
  }; 
  
scoreifNode[ indri::lang::RawExtentNode * ou ] returns [ indri::lang::FilReqNode* fq ]
  {
    RawExtentNode* filter = 0;
    ScoredExtentNode* required = 0;
  } :
  SCOREIF
  O_PAREN filter=unscoredTerm required=scoredExtentNode[ou] C_PAREN {
    fq = new FilReqNode( filter, required );
    _nodes.push_back(fq);
  }; 

anyField returns [ indri::lang::Field* f ]
  {
  std::string fName;
    f = 0;
  } :
  (ANY COLON) => ANY COLON fName=fieldNameString  {
    f = new Field(fName);
    _nodes.push_back(f);
  } |
  (ANY O_PAREN) => ANY O_PAREN fName=fieldNameString C_PAREN {
    f = new Field(fName);
    _nodes.push_back(f);
  };

unscoredTerm returns [ RawExtentNode* t ]
  {
    t = 0;
  } :
    ( qualifiedTerm ) => t=qualifiedTerm
  | t=unqualifiedTerm;
  
qualifiedTerm returns [ RawExtentNode* t ] 
  {
    RawExtentNode* synonyms = 0;
    RawExtentNode* fields = 0;
    t = 0;
  } :
  synonyms=unqualifiedTerm DOT fields=field_list
  {
    if( fields ) {
      t = new indri::lang::ExtentInside( synonyms, fields );
      _nodes.push_back(t);
      synonyms = t;
    } else {
      t = synonyms;
    }
  };

unqualifiedTerm returns [ indri::lang::RawExtentNode* re ] {
    re = 0;
    indri::lang::IndexTerm* t = 0;
  }:
    ( OD ) => re=odNode
  | ( UW ) => re=uwNode
  | ( BAND ) => re=bandNode
  | ( DATEBEFORE ) => re=dateBefore
  | ( DATEAFTER ) => re=dateAfter
  | ( DATEBETWEEN ) => re=dateBetween
  | ( DATEEQUALS ) => re=dateEquals
  | ( O_ANGLE ) => re=synonym_list
  | ( O_BRACE ) => re=synonym_list_brace
  | ( SYN ) => re=synonym_list_alt
  | ( WSYN ) => re=wsynNode
  | ( ANY ) => re=anyField
  | ( LESS ) => re=lessNode
  | ( GREATER ) => re=greaterNode
  | ( BETWEEN ) => re=betweenNode
  | ( EQUALS ) => re=equalsNode
  | ( TERM SPACE_DASH ) => re=rawText
  | ( TERM DASH ) => re=hyphenTerm
  | ( NUMBER DASH ) => re=hyphenTerm
  | ( SPACE_DASH ) => re=rawText
  | ( DASH ) => re=rawText
  | ( WCARD ) => re=wildcardOpNode
  | ( TERM STAR ) => t=rawText STAR {
      // wildcard support as an unqualified term
      // i.e. "term*"
      re=new indri::lang::WildcardTerm( t->getText() );
      _nodes.push_back(re);
  }
  | ( NUMBER STAR ) => t=rawText STAR {
      // wildcard support as an unqualified term
      // i.e. "term*"
      re=new indri::lang::WildcardTerm( t->getText() );
      _nodes.push_back(re);
  }
  | ( FLOAT STAR ) => t=rawText STAR {
      // wildcard support as an unqualified term
      // i.e. "term*"
      re=new indri::lang::WildcardTerm( t->getText() );
      _nodes.push_back(re);
  }
  | ( NEGATIVE_NUMBER STAR ) => t=rawText STAR {
      // wildcard support as an unqualified term
      // i.e. "term*"
      re=new indri::lang::WildcardTerm( t->getText() );
      _nodes.push_back(re);
  }
  | ( TEXT_TERM STAR ) => t=rawText STAR {
      // wildcard support as an unqualified term
      // i.e. "term*"
      re=new indri::lang::WildcardTerm( t->getText() );
      _nodes.push_back(re);
  }
  | re = rawText;

hyphenTerm returns [ indri::lang::ODNode* od ] {
    od = new indri::lang::ODNode;
    od->setWindowSize(1);
    _nodes.push_back(od);
    indri::lang::IndexTerm* t = 0;
  } : id:TERM {     
            t = new indri::lang::IndexTerm(id->getText());
            _nodes.push_back(t);
            od->addChild(t); }
    ( options { greedy=true; }: DASH t=hyphenate { 
            od->addChild(t); }
    )+ |
      n:NUMBER {     
            t = new indri::lang::IndexTerm(n->getText());
            _nodes.push_back(t);
            od->addChild(t); }
    ( options { greedy=true; }: DASH t=hyphenate { 
            od->addChild(t); }
    )+
    ;

hyphenate returns [indri::lang::IndexTerm * t] {
    t = 0;
  } :
   id:TERM {
   t = new indri::lang::IndexTerm(id->getText());
   _nodes.push_back(t);
  } |
  n:NUMBER {
    t = new indri::lang::IndexTerm(n->getText());
    _nodes.push_back(t);
  };

fieldNameString returns [std::string field] {
    std::string second;
} : (TERM DASH) => first:TERM {
            field = first->getText();
        } (options { greedy = true; } : DASH second=fstring {
            field += "-";
            field += second;
            } )+ |
        fname:TERM {
            field = fname->getText();
        } ;

fstring returns [ std::string f] :
  id:TERM {
    f = id->getText();
  } |
  n:NUMBER {
    f = n->getText();
  };

wildcardOpNode returns [ indri::lang::WildcardTerm* s ] {
    // wildcard operator "#wildcard( term )"
    indri::lang::IndexTerm* t = 0;
    s = new indri::lang::WildcardTerm;
    _nodes.push_back(s);
  } :
  WCARD
  O_PAREN
    ( options { greedy=true; }: t=rawText { s->setTerm(t->getText()); } )
  C_PAREN;
          
extentRestriction [ indri::lang::ScoredExtentNode* sn, indri::lang::RawExtentNode * ou ] returns [ indri::lang::ScoredExtentNode* er ] {
    indri::lang::Field* f = 0;
    std::string fName;
    er = 0;
    indri::lang::ExtentInside * po = 0;
  } :
  ( O_SQUARE TERM COLON ) => O_SQUARE passageWindowSize:TERM COLON inc:NUMBER C_SQUARE
  {
    int startWindow;

    for( startWindow = 0; startWindow < passageWindowSize->getText().size(); startWindow++ ) {
      if( isdigit((unsigned char) passageWindowSize->getText()[startWindow] ) )
        break;
    }
  
    int increment = atoi(inc->getText().c_str());
    int windowSize = atoi(passageWindowSize->getText().c_str() + startWindow );
    
    er = new indri::lang::FixedPassage(sn, windowSize, increment);
    _nodes.push_back(er);
  } |
  ( O_SQUARE TERM ) =>  O_SQUARE fName=fieldNameString C_SQUARE
  {
    f = new indri::lang::Field(fName);
    _nodes.push_back(f);
    er = new indri::lang::ExtentRestriction(sn, f);
    _nodes.push_back(er);
  } |
  O_SQUARE DOT po=path C_SQUARE 
  {

    if ( ou == 0 ) {
      throw new antlr::SemanticException("Use of a . in a extent restriction without a valid outer context.");
    }
    po->setOuter(ou);
    er = new indri::lang::ExtentRestriction(sn, po);
    _nodes.push_back(er);
  }
;

path returns [ indri::lang::ExtentInside* r ] {
    r = 0;
    indri::lang::Field * f = 0;
    indri::lang::ExtentInside * po = 0;
    indri::lang::ExtentInside * lastPo = 0;
    std::string fieldRestricted;
} :
  (options{greedy=true;} : po=pathOperator fieldRestricted=fieldNameString {
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
    }
  )+
;

pathOperator returns [ indri::lang::ExtentInside* r ] {
    r = 0;
    indri::lang::DocumentStructureNode * ds = 0;
  } :
  SLASH (SLASH {    
    ds = new indri::lang::DocumentStructureNode;
    _nodes.push_back(ds);
    r = new indri::lang::ExtentDescendant(NULL, NULL, ds);
    _nodes.push_back(r);
  })? { 
    if (r == 0) {
      ds = new indri::lang::DocumentStructureNode;
      _nodes.push_back(ds);
      r = new indri::lang::ExtentChild(NULL, NULL, ds);
      _nodes.push_back(r);
    }
  } | 
  B_SLASH {
   ds = new indri::lang::DocumentStructureNode;
   _nodes.push_back(ds);
   r = new indri::lang::ExtentParent(NULL, NULL, ds);
   _nodes.push_back(r);
  } |
  O_BRACE {
    r = new indri::lang::ExtentInside(NULL, NULL);   
    _nodes.push_back(r);
  }
;        


        
synonym_list returns [ indri::lang::ExtentOr* s ] {
    indri::lang::RawExtentNode* term = 0;
    s = new indri::lang::ExtentOr;
    _nodes.push_back(s);
  } :
  O_ANGLE
    ( options { greedy=true; }: term=unscoredTerm { s->addChild(term); } )+
  C_ANGLE;

synonym_list_brace returns [ indri::lang::ExtentOr* s ] {
    indri::lang::RawExtentNode* term = 0;
    s = new indri::lang::ExtentOr;
    _nodes.push_back(s);
  } :
  O_BRACE
    ( options { greedy=true; }: term=unscoredTerm { s->addChild(term); } )+
  C_BRACE;

synonym_list_alt returns [ indri::lang::ExtentOr* s ] {
    indri::lang::RawExtentNode* term = 0;
    // semantics of this node will change
    s = new indri::lang::ExtentOr;
    _nodes.push_back(s);
  } :
  SYN
  O_PAREN
    ( options { greedy=true; }: term=unscoredTerm { s->addChild(term); } )+
  C_PAREN;

field_list returns [ indri::lang::ExtentAnd* fields ]
  { 
    std::string first, additional;
    fields = new ExtentAnd;
    _nodes.push_back( fields );
  } :
  // first field
  first=fieldNameString {
    Field* firstField = new indri::lang::Field( first );
    _nodes.push_back( firstField );
    fields->addChild( firstField );
  }
  // additional fields
  ( options { greedy=true; } :
      COMMA additional=fieldNameString {
        Field* additionalField = new Field(additional);
        _nodes.push_back( additionalField );
        fields->addChild( additionalField );
      }
  )*;

context_list [ indri::lang::RawExtentNode * ou ] returns [ ExtentOr* contexts ] {
    contexts = new ExtentOr;
    _nodes.push_back( contexts );
    indri::lang::ExtentInside * p = 0;
    indri::lang::ExtentInside * pAdditional = 0;
    std::string first, additional;
  } :
  O_PAREN
  // first field
  (first=fieldNameString {
    Field* firstField = new indri::lang::Field( first );
    _nodes.push_back( firstField );
    contexts->addChild( firstField );
  }
  | 
  (DOT p=path {
    p->setOuter( ou );
    contexts->addChild( p );
  }))
  // additional fields
  ( options { greedy=true; } :
      COMMA ( additional=fieldNameString {
        Field* additionalField = new Field(additional);
        _nodes.push_back( additionalField );
        contexts->addChild( additionalField );
      } |
      (DOT pAdditional=path {
          pAdditional->setOuter( ou );
          contexts->addChild( pAdditional );
       }))
  )*
  C_PAREN;



field_restriction returns [ indri::lang::Field* extent ] {
    std::string fieldName;
  } :
  O_SQUARE
  fieldName=fieldNameString {
    extent = new Field( fieldName );
    _nodes.push_back( extent );
  }
  C_SQUARE;


dateBefore returns [ indri::lang::FieldLessNode* extent ] {
    UINT64 d = 0;
    Field* dateField = 0;
    extent = 0;
  } :
  DATEBEFORE O_PAREN d=date C_PAREN {
    dateField = new Field("date");
    extent = new FieldLessNode( dateField, d );
    _nodes.push_back( dateField );
    _nodes.push_back( extent );
  };
  
dateAfter returns [ indri::lang::FieldGreaterNode* extent ] {
    UINT64 d = 0;
    Field* dateField = 0;
    extent = 0;
  } :
  DATEAFTER O_PAREN d=date C_PAREN {
    dateField = new Field("date");
    extent = new FieldGreaterNode( dateField, d );
    _nodes.push_back( dateField );
    _nodes.push_back( extent );
  };
  
dateBetween returns [ indri::lang::FieldBetweenNode* extent ] {
    UINT64 low = 0;
    UINT64 high = 0;
    Field* dateField = 0;
    extent = 0;
  } :
  DATEBETWEEN O_PAREN low=date high=date C_PAREN {
    dateField = new Field("date");
    extent = new FieldBetweenNode( dateField, low, high );
    _nodes.push_back( dateField );
    _nodes.push_back( extent );
  };

dateEquals returns [ indri::lang::FieldEqualsNode* extent ] {
    UINT64 d = 0;
    Field* dateField = 0;
    extent = 0;
  } :
  DATEEQUALS O_PAREN d=date C_PAREN {
    dateField = new Field("date");
    extent = new FieldEqualsNode( dateField, d );
    _nodes.push_back( dateField );
    _nodes.push_back( extent );
  };

//
// want to handle the following date strings:
//  11 january 2004
//  11-JAN-04
//  11-JAN-2004
//  January 11 2004
//  01/11/04
//  01/11/2004
// also the following times:
//  4:25 pm
//  4:25p
//  16:25
//  16:25:00
//

date returns [ UINT64 d ] :
  ( NUMBER SLASH ) => d=slashDate |
  ( TERM NUMBER ) => d=spaceDate |
  ( NUMBER TERM ) => d=spaceDate |
  ( NUMBER DASH ) => d=dashDate 
  ;
  
dashDate returns [ UINT64 d ] {
    d = 0;
  } :
  day:NUMBER DASH month:TERM DASH year:NUMBER {
    d = indri::parse::DateParse::convertDate( year->getText(), month->getText(), day->getText() );             
  };
  
slashDate returns [ UINT64 d ] {
    d = 0;
  } :
  month:NUMBER SLASH day:NUMBER SLASH year:NUMBER {
    d = indri::parse::DateParse::convertDate( year->getText(), month->getText(), day->getText() ); 
  };
  
spaceDate returns [ UINT64 d ] {
    d = 0;
  } :
    (NUMBER) => day:NUMBER month:TERM year:NUMBER {
    d = indri::parse::DateParse::convertDate( year->getText(), month->getText(), day->getText() );
  } | 
    (TERM) =>  m:TERM dd:NUMBER y:NUMBER {
    d = indri::parse::DateParse::convertDate( y->getText(), m->getText(), dd->getText() );
};

// rawText is something that can be considered a query term
rawText returns [ indri::lang::IndexTerm* t ] {
    t = 0;
  } :
  id:TERM {
    t = new indri::lang::IndexTerm(id->getText());
    _nodes.push_back(t);
  } |
  n:NUMBER {
    t = new indri::lang::IndexTerm(n->getText());
    _nodes.push_back(t);
  } |
  SPACE_DASH nn:NUMBER {
    t = new indri::lang::IndexTerm(std::string("-") + nn->getText());
    _nodes.push_back(t);
  } |
  DASH nnn:NUMBER {
    t = new indri::lang::IndexTerm(std::string("-") + nnn->getText());
    _nodes.push_back(t);
  } |
  f:FLOAT {
    t = new indri::lang::IndexTerm(f->getText());
    _nodes.push_back(t);
  } |
  DASH ff:FLOAT {
    t = new indri::lang::IndexTerm(std::string("-") + ff->getText());
    _nodes.push_back(t);
  } |
  SPACE_DASH fff:FLOAT {
    t = new indri::lang::IndexTerm(std::string("-") + fff->getText());
    _nodes.push_back(t);
  } |
  DBL_QUOTE t=rawText DBL_QUOTE {
    // if a text term appears in quotes, consider it stemmed
    t->setStemmed(true);
  } |
  et:ENCODED_TERM {
    std::string decodedString; 
    base64_decode_string(decodedString, et->getText());
    t = new indri::lang::IndexTerm( decodedString );
    _nodes.push_back(t);
  } |
  qet:ENCODED_QUOTED_TERM {
    std::string decodedString; 
    base64_decode_string(decodedString, qet->getText());
    t = new indri::lang::IndexTerm( decodedString );
    t->setStemmed(true);
    _nodes.push_back(t);
  };

// A "floating" is either an integer or a number with a decimal point followed by a number.
// For instance, 1 and 1.0 are floating, but 1. is not.  
floating returns [ double d ] {
    d = 0;
  } :
  f:FLOAT {
    d = atof(f->getText().c_str());
  } |
  n:NUMBER {
    d = atof(n->getText().c_str());
  } |
  DASH ff:FLOAT {
    d = - atof(ff->getText().c_str());
  } |
  DASH nn:NUMBER {
    d = - atof(nn->getText().c_str());
  } |
  SPACE_DASH fff:FLOAT {
    d = - atof(fff->getText().c_str());
  } |
  SPACE_DASH nnn:NUMBER {
    d = - atof(nnn->getText().c_str());
  };

number returns [ INT64 v ] {
    v = 0;
  } :
  n:NUMBER {
    v = string_to_i64(n->getText());
  } |
  DASH nn:NUMBER {
    v = - string_to_i64(nn->getText());
  } |
  SPACE_DASH nnn:NUMBER {
    v = - string_to_i64(nnn->getText());
  };

greaterNode returns [ indri::lang::FieldGreaterNode* gn ] {
    gn = 0;
    Field* compareField = 0;
    INT64 low = 0;
    std::string field;
  } :
  GREATER O_PAREN field=fieldNameString low=number C_PAREN {
    compareField = new Field(field);
    gn = new FieldGreaterNode( compareField, low );
    _nodes.push_back( compareField );
    _nodes.push_back( gn );
  };
  
lessNode returns [ indri::lang::FieldLessNode* ln ] {
    ln = 0;
    Field* compareField = 0;
    INT64 high = 0;
    std::string field;
  } :
  LESS O_PAREN field=fieldNameString high=number C_PAREN {
    compareField = new Field(field);
    ln = new FieldLessNode( compareField, high );
    _nodes.push_back( compareField );
    _nodes.push_back( ln );
  };

betweenNode returns [ indri::lang::FieldBetweenNode* bn ] {
    bn = 0;
    Field* compareField = 0;
    INT64 low = 0;
    INT64 high = 0;
    std::string field;
  } :
  BETWEEN O_PAREN field=fieldNameString low=number high=number C_PAREN {
    compareField = new Field(field);
    bn = new FieldBetweenNode( compareField, low, high );
    _nodes.push_back( compareField );
    _nodes.push_back( bn );
  };

equalsNode returns [ indri::lang::FieldEqualsNode* en ] {
    en = 0;
    Field* compareField = 0;
    INT64 eq = 0;
    std::string field;
  } :
  EQUALS O_PAREN field=fieldNameString eq=number C_PAREN {
    compareField = new Field(field);
    en = new FieldEqualsNode( compareField, eq );
    _nodes.push_back( compareField );
    _nodes.push_back( en );
  };

