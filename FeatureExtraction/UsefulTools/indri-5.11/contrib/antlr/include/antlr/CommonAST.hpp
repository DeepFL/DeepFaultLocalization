#ifndef INC_CommonAST_hpp__
#define INC_CommonAST_hpp__

/* ANTLR Translator Generator
 * Project led by Terence Parr at http://www.jGuru.com
 * Software rights: http://www.antlr.org/license.html
 *
 * $Id: //depot/code/org.antlr/release/antlr-2.7.4/lib/cpp/antlr/CommonAST.hpp#1 $
 */

#include <antlr/config.hpp>
#include <antlr/BaseAST.hpp>

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
namespace antlr {
#endif

class ANTLR_API CommonAST : public BaseAST {
public:
	CommonAST();
	CommonAST( RefToken t );
	CommonAST( const CommonAST& other );

	virtual ~CommonAST();

	virtual const char* typeName( void ) const;
	/// Clone this AST node.
	virtual RefAST clone( void ) const;

	virtual ANTLR_USE_NAMESPACE(std)string getText() const;
	virtual int getType() const;

	virtual void initialize( int t, const ANTLR_USE_NAMESPACE(std)string& txt );
	virtual void initialize( RefAST t );
	virtual void initialize( RefToken t );
#ifdef ANTLR_SUPPORT_XML
	virtual void initialize( ANTLR_USE_NAMESPACE(std)istream& in );
#endif

	virtual void setText( const ANTLR_USE_NAMESPACE(std)string& txt );
	virtual void setType( int type );

	static RefAST factory();

	static const char* const TYPE_NAME;
protected:
	int ttype;
	ANTLR_USE_NAMESPACE(std)string text;
};

typedef ASTRefCount<CommonAST> RefCommonAST;

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
}
#endif

#endif //INC_CommonAST_hpp__
