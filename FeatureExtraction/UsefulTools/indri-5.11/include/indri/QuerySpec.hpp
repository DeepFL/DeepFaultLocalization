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


#ifndef INDRI_QUERYSPEC_HPP
#define INDRI_QUERYSPEC_HPP

#include <vector>
#include <string>
#include <sstream>
#include <indri/greedy_vector>
#include <algorithm>
#include "lemur/lemur-platform.h"

#include "indri/Walker.hpp"
#include "indri/Copier.hpp"
#include "indri/Packer.hpp"
#include "indri/Unpacker.hpp"

#include "lemur/Exception.hpp"
#include "indri/HashTable.hpp"
#include "indri/ref_ptr.hpp"

template<class T>
bool equal( const std::vector<T>& one, const std::vector<T>& two ) {
  if( one.size() != two.size() )
    return false;

  for( size_t i=0; i<one.size(); i++ ) {
    if( *one[i] == *two[i] )
      continue;

    return false;
  }

  return true;
}

template<class T>
bool unordered_equal( std::vector<T>& one, std::vector<T>& two ) {
  if( one.size() != two.size() )
    return false;

  std::vector<T> one_copy;
  for( size_t i=0; i<one.size(); i++ ) {
    one_copy.push_back( one[i] );
  }

  // this algorithm is n^2 as opposed to n log n if
  // we sorted things, but windows tend to be short
  for( size_t i=0; i<two.size(); i++ ) {
    for( size_t j=0; j<one_copy.size(); j++ ) {
      if( *one_copy[j] == *two[i] ) {
        // we remove each match--if they all match, the array will be empty
        one_copy.erase( one_copy.begin() + j );
        break;
      }
    }
  }

  return one_copy.size() == 0;
}

namespace indri {
  namespace lang {
    /* abstract */ class Node {
    protected:
      std::string _name;

    public:
      Node() {
        std::stringstream s;
        s << PTR_TO_INT(this);
        _name = s.str();
      }

      virtual ~Node() {
      }
      
      void setNodeName( const std::string& name ) {
        _name = name;
      }

      const std::string& nodeName() const {
        return _name;
      }

      virtual std::string typeName() const {
        return "Node";
      }

      virtual std::string queryText() const = 0;

      virtual bool operator < ( Node& other ) {
        // TODO: make this faster
        if( typeName() != other.typeName() )
          return typeName() < other.typeName();

        return queryText() < other.queryText();
      }
     
      virtual bool operator== ( Node& other ) {
        return &other == this; 
      }

      virtual UINT64 hashCode() const = 0;
      virtual void pack( Packer& packer ) = 0;
      virtual void walk( Walker& walker ) = 0;
      virtual Node* copy( Copier& copier ) = 0;
    };

    /* abstract */ class RawExtentNode : public Node {};
    /* abstract */ class ScoredExtentNode : public Node {};
    /* abstract */ class AccumulatorNode : public Node {};
    
    class IndexTerm : public RawExtentNode {
    private:
      std::string _text;
      bool _stemmed;

    public:
      IndexTerm( const std::string& text, bool stemmed = false ) : _text(text), _stemmed(stemmed)
      {
      }

      IndexTerm( Unpacker& unpacker ) {
        _text = unpacker.getString( "termName" );
        _stemmed = unpacker.getBoolean( "stemmed" );
      }

      const std::string& getText() { return _text; }

      bool operator==( Node& node ) {
        IndexTerm* other = dynamic_cast<IndexTerm*>(&node);

        if( !other )
          return false;

        if( other == this )
          return true;
        
        return other->_text == _text;
      }

      std::string typeName() const {
        return "IndexTerm";
      }

      std::string queryText() const {
        std::stringstream qtext;

        if( _stemmed ) {
          qtext << '"' << _text << '"';
        } else {
          qtext << _text;
        }

        return qtext.str();
      }

      void setStemmed(bool stemmed) {
        _stemmed = stemmed;
      }

      bool getStemmed() const {
        return _stemmed;
      }

      UINT64 hashCode() const {
        int accumulator = 1;

        if( _stemmed )
          accumulator += 3;

        indri::utility::GenericHash<const char*> hash;
        return accumulator + hash( _text.c_str() );
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "termName", _text );
        packer.put( "stemmed", _stemmed );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        IndexTerm* termCopy = new IndexTerm(*this);
        return copier.after(this, termCopy);
      }
    };

    class Field : public RawExtentNode {
    private:
      std::string _fieldName;

    public:
      Field( const std::string& name ) : _fieldName(name)
      {
      }

      Field( Unpacker& unpacker ) {
        _fieldName = unpacker.getString( "fieldName" );
      }

      const std::string& getFieldName() const { return _fieldName; }

      std::string typeName() const {
        return "Field";
      }

      std::string queryText() const {
        return _fieldName;
      }

      UINT64 hashCode() const {
        indri::utility::GenericHash<const char*> hash;
        return 5 + hash( _fieldName.c_str() );
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "fieldName", _fieldName );
        packer.after(this);
      }
      
      void walk( Walker& walker ) {
        walker.before(this);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        Field* newField = new Field(*this);
        return copier.after(this, newField);
      }

      bool operator== ( Node& other ) {
        Field* otherField = dynamic_cast<Field*>(&other);

        if( !otherField )
          return false;

        return otherField->getFieldName() == getFieldName();
      }
    };

    class ExtentInside : public RawExtentNode {
    protected:
      RawExtentNode* _inner;
      RawExtentNode* _outer;

    public:
      ExtentInside( RawExtentNode* inner, RawExtentNode* outer ) :
        _inner(inner),
        _outer(outer)
      {
      }

      ExtentInside( Unpacker& unpacker ) {
        _inner = unpacker.getRawExtentNode( "inner" );
        _outer = unpacker.getRawExtentNode( "outer" );
      }

      virtual bool operator== ( Node& o ) {
        ExtentInside* other = dynamic_cast<ExtentInside*>(&o);
  
        return other &&
          *_inner == *other->_inner &&
          *_outer == *other->_outer;
      }
      
      virtual std::string typeName() const {
        return "ExtentInside";
      }

      virtual UINT64 hashCode() const {
        return 7 + _inner->hashCode() + (_inner->hashCode() * 7);
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << _inner->queryText()
              << "."
              << _outer->queryText();

        return qtext.str();
      }

      void setInner( RawExtentNode * inner ) {
        _inner = inner;
      }

      void setOuter( RawExtentNode * outer ) {
        _outer = outer;
      }

      RawExtentNode* getInner() {
        return _inner;
      }

      RawExtentNode* getOuter() {
        return _outer;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "inner", _inner );
        packer.put( "outer", _outer );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _inner->walk(walker);
        _outer->walk(walker);
        walker.after(this);
      }

      virtual Node* copy( Copier& copier ) {
        copier.before(this);
        
        RawExtentNode* newInner = dynamic_cast<RawExtentNode*>(_inner->copy(copier));
        RawExtentNode* newOuter = dynamic_cast<RawExtentNode*>(_outer->copy(copier));
        ExtentInside* extentInsideCopy = new ExtentInside( newInner, newOuter );
        extentInsideCopy->setNodeName( nodeName() );

        return copier.after(this, extentInsideCopy);
      }
    };

    class WeightedExtentOr : public RawExtentNode {
    private:
      std::vector<RawExtentNode*> _children;
      std::vector<double> _weights;

    public:
      WeightedExtentOr() {}
      WeightedExtentOr( const std::vector<double>& weights, const std::vector<RawExtentNode*>& children ) :
        _children(children),
        _weights(weights)
      {
      }

      WeightedExtentOr( Unpacker& unpacker ) {
        _children = unpacker.getRawExtentVector( "children" );
        _weights = unpacker.getDoubleVector( "weights" );
      }

      std::string typeName() const {
        return "WeightedExtentOr";
      }

      std::string queryText() const {
        std::stringstream qtext;

        qtext << "#wsyn(";

        for( size_t i=0; i<_children.size(); i++ ) {
          qtext << " " << _children[i]->queryText();
        }

        qtext << " )";
        return qtext.str();
      }

      UINT64 hashCode() const {
        UINT64 hash = 11;

        for( size_t i=0; i<_children.size(); i++ ) {
          hash += (UINT64) (_weights[i] * 1000) + _children[i]->hashCode();
        }
        
        return hash;
      }

      void addChild( double weight, RawExtentNode* child ) {
        _children.push_back( child );
        _weights.push_back( weight );
      }

      std::vector<RawExtentNode*>& getChildren() {
        return _children;
      }

      std::vector<double>& getWeights() {
        return _weights;
      }

      bool operator == ( Node& node ) {
        WeightedExtentOr* other = dynamic_cast<WeightedExtentOr*>(&node);

        if( other == this )
          return true;

        // TODO: use better checking here to eliminate duplicate nodes
        return false;
      }

      void pack( Packer& packer ) {
        packer.before( this );
        packer.put( "weights", _weights );
        packer.put( "children", _children );
        packer.after( this );
      }

      void walk( Walker& walker ) {
        walker.before( this );
        for( size_t i=0; i<_children.size(); i++ ) {
          _children[i]->walk( walker );
        }
        walker.after( this );
      }

      Node* copy( Copier& copier ) {
        copier.before( this );

        WeightedExtentOr* duplicate = new WeightedExtentOr();
        for( size_t i=0; i<_children.size(); i++ ) {
          RawExtentNode* child = dynamic_cast<RawExtentNode*>(_children[i]->copy( copier ));
          duplicate->addChild( _weights[i], child );
        }

        return copier.after( this, duplicate );
      }
    };

    class ExtentOr : public RawExtentNode {
    private:
      std::vector<RawExtentNode*> _children;

    public:
      ExtentOr() {}
      ExtentOr( const std::vector<RawExtentNode*>& children ) :
        _children(children)
      {
      }

      ExtentOr( Unpacker& unpacker ) {
        _children = unpacker.getRawExtentVector( "children" );
      } 

      std::string typeName() const {
        return "ExtentOr";
      }

      std::string queryText() const {
        std::stringstream qtext;

        for( size_t i=0; i<_children.size(); i++ ) {
          if(i>0) qtext << " ";
          qtext << _children[i]->queryText();
        }

        return qtext.str();
      }

      UINT64 hashCode() const {
        UINT64 hash = 13;

        for( size_t i=0; i<_children.size(); i++ ) {
          hash += _children[i]->hashCode();
        }
        
        return hash;
      }

      void addChild( RawExtentNode* node ) {
        _children.push_back(node);
      }

      std::vector<RawExtentNode*>& getChildren() {
        return _children;
      }

      bool operator== ( Node& node ) {
        ExtentOr* other = dynamic_cast<ExtentOr*>(&node);

        if( other == this )
          return true;

        if( !other )
          return false;

        return unordered_equal( other->_children, _children );
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "children", _children );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        for( size_t i=0; i<_children.size(); i++ ) {
          _children[i]->walk(walker);
        }
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        
        ExtentOr* duplicate = new ExtentOr();
        duplicate->setNodeName( nodeName() );
        for( size_t i=0; i<_children.size(); i++ ) {
          indri::lang::Node* childNode = _children[i]->copy(copier);
          duplicate->addChild( dynamic_cast<RawExtentNode*>(childNode) );
        }

        return copier.after(this, duplicate);
      }
    };

    class ExtentAnd : public RawExtentNode {
    private:
      std::vector<RawExtentNode*> _children;

    public:
      ExtentAnd() {}
      ExtentAnd( const std::vector<RawExtentNode*>& children ) :
        _children(children)
      {
      }

      ExtentAnd( Unpacker& unpacker ) {
        _children = unpacker.getRawExtentVector( "children" );
      }

      std::string typeName() const {
        return "ExtentAnd";
      }

      std::string queryText() const {
        std::stringstream qtext;

        for( size_t i=0; i<_children.size(); i++ ) {
          if(i>0) qtext << ",";
          qtext << _children[i]->queryText();
        }

        return qtext.str();
      }

      UINT64 hashCode() const {
        UINT64 hash = 15;

        for( size_t i=0; i<_children.size(); i++ ) {
          hash += _children[i]->hashCode();
        }
        
        return hash;
      }

      void addChild( RawExtentNode* node ) {
        _children.push_back(node);
      }

      std::vector<RawExtentNode*>& getChildren() {
        return _children;
      }

      bool operator== ( Node& node ) {
        ExtentAnd* other = dynamic_cast<ExtentAnd*>(&node);

        if( other == this )
          return true;

        if( !other )
          return false;

        return unordered_equal( other->_children, _children );
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "children", _children );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        for( size_t i=0; i<_children.size(); i++ ) {
          _children[i]->walk(walker);
        }
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);

        ExtentAnd* duplicate = new ExtentAnd();
        duplicate->setNodeName( nodeName() );
        for( size_t i=0; i<_children.size(); i++ ) {
          Node* child = _children[i]->copy(copier);
          duplicate->addChild( dynamic_cast<RawExtentNode*>(child) );
        }

        return copier.after(this, duplicate);
      }
    };

    class BAndNode : public RawExtentNode {
    private:
      std::vector<RawExtentNode*> _children;

    public:
      BAndNode() {}

      BAndNode( Unpacker& unpacker ) {
        _children = unpacker.getRawExtentVector( "children" );
      }

      std::string typeName() const {
        return "BAndNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#band(";
        for( size_t i=0; i<_children.size(); i++ ) {
          qtext << _children[i]->queryText() << " ";
        }
        qtext << ")";

        return qtext.str();
      } 

      UINT64 hashCode() const {
        UINT64 hash = 17;

        for( size_t i=0; i<_children.size(); i++ ) {
          hash += _children[i]->hashCode();
        }
        
        return hash;
      }

      const std::vector<RawExtentNode*>& getChildren() const {
        return _children;
      }

      void addChild( RawExtentNode* node ) {
        _children.push_back( node );
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "children", _children );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        for( size_t i=0; i<_children.size(); i++ ) {
          _children[i]->walk(walker);
        }
        walker.after(this);
      }
      
      Node* copy( Copier& copier ) {
        copier.before(this);
        BAndNode* duplicate = new BAndNode();

        duplicate->setNodeName( nodeName() );
        for(size_t i=0; i<_children.size(); i++) {
          Node* child = _children[i]->copy(copier);
          duplicate->addChild( dynamic_cast<RawExtentNode*>(child) );
        }

        return copier.after(this, duplicate);
      }
    };

    class UWNode : public RawExtentNode {
    private:
      std::vector<RawExtentNode*> _children;
      int _windowSize;

    public:
      UWNode() :
        _windowSize(-1) // default is unlimited window size
      {
      }

      UWNode( int windowSize, std::vector<RawExtentNode*>& children ) :
        _windowSize(windowSize),
        _children(children)
      {
      }

      UWNode( Unpacker& unpacker ) {
        _windowSize = (int) unpacker.getInteger( "windowSize" );
        _children = unpacker.getRawExtentVector( "children" );
      }

      std::string typeName() const {
        return "UWNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        
        if( _windowSize >= 0 )
          qtext << "#uw" << _windowSize << "( ";
        else
          qtext << "#uw( ";
          
        for( size_t i=0; i<_children.size(); i++ ) {
          qtext << _children[i]->queryText() << " ";
        }
        qtext << ")";
        return qtext.str();
      }

      UINT64 hashCode() const {
        UINT64 hash = 19;
        hash += _windowSize;

        for( size_t i=0; i<_children.size(); i++ ) {
          hash += _children[i]->hashCode();
        }
        
        return hash;
      }

      void setWindowSize( int windowSize ) {
        _windowSize = windowSize;
      }

      void setWindowSize( const std::string& windowSize ) {
        setWindowSize( atoi( windowSize.c_str() ) );
      }

      int getWindowSize() const {
        return _windowSize;
      }

      const std::vector<RawExtentNode*>& getChildren() const {
        return _children;
      }

      void addChild( RawExtentNode* node ) {
        _children.push_back( node );
      }

      bool operator== ( Node& node ) {
        UWNode* other = dynamic_cast<UWNode*>(&node);

        if( !other )
          return false;

        if( other == this )
          return true;

        if( other->_windowSize != _windowSize ) {
          return false;
        }

        return unordered_equal( _children, other->_children );
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "windowSize", _windowSize );
        packer.put( "children", _children );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        for(size_t i=0; i<_children.size(); i++) {
          _children[i]->walk(walker);
        }
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        
        UWNode* duplicate = new UWNode();
        duplicate->setNodeName( nodeName() );
        duplicate->setWindowSize( _windowSize );
        for(size_t i=0; i<_children.size(); i++) {
          Node* child = _children[i]->copy(copier);
          duplicate->addChild( dynamic_cast<RawExtentNode*>(child) );
        }

        return copier.after(this, duplicate);
      }
    };

    class ODNode : public RawExtentNode {
    private:
      int _windowSize;
      std::vector<RawExtentNode*> _children;

    public:
      ODNode( int windowSize, std::vector<RawExtentNode*>& children ) :
        _windowSize(windowSize),
        _children(children)
      {
      }

      ODNode() :
        _windowSize(-1) // default is unlimited window size
      {
      }

      ODNode( Unpacker& unpacker ) {
        _windowSize = (int) unpacker.getInteger( "windowSize" );
        _children = unpacker.getRawExtentVector( "children" );
      }

      std::string typeName() const {
        return "ODNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        if( _windowSize >= 0 )
          qtext << "#" << _windowSize << "( ";
        else
          qtext << "#od( ";

        for( size_t i=0; i<_children.size(); i++ ) {
          qtext << _children[i]->queryText() << " ";
        }
        qtext << ")";
        return qtext.str();
      }

      UINT64 hashCode() const {
        UINT64 hash = 23;
        hash += _windowSize;

        for( size_t i=0; i<_children.size(); i++ ) {
          hash *= 7;
          hash += _children[i]->hashCode();
        }
        
        return hash;
      }

      const std::vector<RawExtentNode*>& getChildren() const {
        return _children;
      }

      void setWindowSize( int windowSize ) {
        _windowSize = windowSize;
      }

      void setWindowSize( const std::string& windowSize ) {
        setWindowSize( atoi( windowSize.c_str() ) );
      }

      int getWindowSize() const {
        return _windowSize;
      }

      void addChild( RawExtentNode* node ) {
        _children.push_back( node );
      }

      bool operator== ( Node& node ) {
        ODNode* other = dynamic_cast<ODNode*>(&node);

        if( ! other )
          return false;

        if( other == this )
          return true;

        if( other->_windowSize != _windowSize )
          return false;

        if( _children.size() != other->_children.size() )
          return false;

        return equal( _children, other->_children );
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "windowSize", _windowSize );
        packer.put( "children", _children );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        for(size_t i=0; i<_children.size(); i++) {
          _children[i]->walk(walker);
        }
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        
        ODNode* duplicate = new ODNode();
        duplicate->setNodeName( nodeName() );
        duplicate->setWindowSize( _windowSize );
        for(size_t i=0; i<_children.size(); i++) {
          Node* child = _children[i]->copy(copier);
          duplicate->addChild( dynamic_cast<RawExtentNode*>(child) );
        }

        return copier.after(this, duplicate);
      }
    };

    class FilReqNode : public ScoredExtentNode {
    private:
      RawExtentNode* _filter;
      ScoredExtentNode* _required;

    public:
      FilReqNode( RawExtentNode* filter, ScoredExtentNode* required ) {
        _filter = filter;
        _required = required;
      }

      FilReqNode( Unpacker& unpacker ) {
        _filter = unpacker.getRawExtentNode( "filter" );
        _required = unpacker.getScoredExtentNode( "required" );
      }

      std::string typeName() const {
        return "FilReqNode";
      }

      UINT64 hashCode() const {
        return 27 +
          _filter->hashCode() * 3 +
          _required->hashCode();
      }

      std::string queryText() const {
        std::stringstream qtext;

        qtext << "#filreq("
              << _filter->queryText()
              << " "
              << _required->queryText()
              << ")";
        return qtext.str();
      }

      RawExtentNode* getFilter() {
        return _filter;
      }

      ScoredExtentNode* getRequired() {
        return _required;
      }

      bool operator== ( Node& node ) {
        FilReqNode* other = dynamic_cast<FilReqNode*>(&node);

        if( !other )
          return false;

        return (*_filter) == (*other->getFilter()) &&
          (*_required) == (*other->getRequired());
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put("filter", _filter);
        packer.put("required", _required);
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _filter->walk(walker);
        _required->walk(walker);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        RawExtentNode* filterDuplicate = dynamic_cast<RawExtentNode*>(_filter->copy(copier));
        ScoredExtentNode* requiredDuplicate = dynamic_cast<ScoredExtentNode*>(_required->copy(copier));
        FilReqNode* duplicate = new FilReqNode( filterDuplicate, requiredDuplicate );
        return copier.after(this, duplicate);
      }
    };

    class FilRejNode : public ScoredExtentNode {
    private:
      RawExtentNode* _filter;
      ScoredExtentNode* _disallowed;

    public:
      FilRejNode( RawExtentNode* filter, ScoredExtentNode* disallowed ) {
        _filter = filter;
        _disallowed = disallowed;
      }

      FilRejNode( Unpacker& unpacker ) {
        _filter = unpacker.getRawExtentNode( "filter" );
        _disallowed = unpacker.getScoredExtentNode( "disallowed" );
      }

      std::string typeName() const {
        return "FilRejNode";
      }

      std::string queryText() const {
        std::stringstream qtext;

        qtext << "#filrej("
              << _filter->queryText()
              << " "
              << _disallowed->queryText()
              << ")";

        return qtext.str();
      }

      UINT64 hashCode() const {
        return 29 +
          _filter->hashCode() * 3 +
          _disallowed->hashCode();
      }

      RawExtentNode* getFilter() {
        return _filter;
      }

      ScoredExtentNode* getDisallowed() {
        return _disallowed;
      }

      bool operator== ( Node& node ) {
        FilRejNode* other = dynamic_cast<FilRejNode*>(&node);

        if( !other )
          return false;

        return (*_filter) == (*other->getFilter()) &&
          (*_disallowed) == (*other->getDisallowed());
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put("filter", _filter);
        packer.put("disallowed", _disallowed);
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _filter->walk(walker);
        _disallowed->walk(walker);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        RawExtentNode* filterDuplicate = dynamic_cast<RawExtentNode*>(_filter->copy(copier));
        ScoredExtentNode* disallowedDuplicate = dynamic_cast<ScoredExtentNode*>(_disallowed->copy(copier));
        FilRejNode* duplicate = new FilRejNode( filterDuplicate, disallowedDuplicate );
        return copier.after(this, duplicate);
      }
    };

    class FieldLessNode : public RawExtentNode {
    private:
      RawExtentNode* _field;
      INT64 _constant;

    public:
      FieldLessNode( RawExtentNode* field, INT64 constant ) :
        _field(field),
        _constant(constant) {
      }
      
      FieldLessNode( Unpacker& unpacker ) {
        _field = unpacker.getRawExtentNode( "field" );
        _constant = unpacker.getInteger("constant");
      }

      std::string typeName() const {
        return "FieldLessNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#less(" << _field->queryText() << " " << _constant << ")";
        return qtext.str();
      }

      UINT64 hashCode() const {
        return 31 +
          _constant +
          _field->hashCode();
      }

      INT64 getConstant() const {
        return _constant;
      }

      RawExtentNode* getField() {
        return _field;
      }

      bool operator== ( Node& node ) {
        FieldLessNode* other = dynamic_cast<FieldLessNode*>(&node);

        return other &&
          other->getConstant() == _constant &&
          *other->getField() == *_field;
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        RawExtentNode* fieldDuplicate = dynamic_cast<RawExtentNode*>(_field->copy(copier));
        FieldLessNode* duplicate = new FieldLessNode( fieldDuplicate, _constant );
        return copier.after(this, duplicate);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _field->walk(walker);
        walker.after(this);
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put("field", _field);
        packer.put("constant", _constant);
        packer.after(this);
      }
    };

    class FieldGreaterNode : public RawExtentNode {
    private:
      RawExtentNode* _field;
      INT64 _constant;

    public:
      FieldGreaterNode( RawExtentNode* field, INT64 constant ) :
        _field(field),
        _constant(constant) {
      }
      
      FieldGreaterNode( Unpacker& unpacker ) {
        _field = unpacker.getRawExtentNode( "field" );
        _constant = unpacker.getInteger("constant");
      }

      std::string typeName() const {
        return "FieldGreaterNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#greater(" << _field->queryText() << " " << _constant << ")";
        return qtext.str();
      }

      UINT64 hashCode() const {
        return 33 +
          _constant +
          _field->hashCode();
      }

      INT64 getConstant() const {
        return _constant;
      }

      RawExtentNode* getField() {
        return _field;
      }

      bool operator== ( Node& node ) {
        FieldGreaterNode* other = dynamic_cast<FieldGreaterNode*>(&node);

        return other &&
          other->getConstant() == _constant &&
          *other->getField() == *_field;
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        RawExtentNode* fieldDuplicate = dynamic_cast<RawExtentNode*>(_field->copy(copier));
        FieldGreaterNode* duplicate = new FieldGreaterNode( fieldDuplicate, _constant );
        return copier.after(this, duplicate);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _field->walk(walker);
        walker.after(this);
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put("field", _field);
        packer.put("constant", _constant);
        packer.after(this);
      }
    };

    class FieldBetweenNode : public RawExtentNode {
    private:
      RawExtentNode* _field;
      INT64 _low;
      INT64 _high;

    public:
      FieldBetweenNode( RawExtentNode* field, INT64 low, INT64 high ) :
        _field(field),
        _low(low),
        _high(high) {
      }
      
      FieldBetweenNode( Unpacker& unpacker ) {
        _field = unpacker.getRawExtentNode( "field" );
        _low = unpacker.getInteger("low");
        _high = unpacker.getInteger("high");
      }

      std::string typeName() const {
        return "FieldBetweenNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#between(" << _field->queryText() << " " << _low << " " << _high << ")";
        return qtext.str();
      }

      UINT64 hashCode() const {
        return 37 +
          _low * 3 +
          _high +
          _field->hashCode();
      }

      INT64 getLow() const {
        return _low;
      }

      INT64 getHigh() const {
        return _high;
      }

      RawExtentNode* getField() {
        return _field;
      }

      bool operator== ( Node& node ) {
        FieldBetweenNode* other = dynamic_cast<FieldBetweenNode*>(&node);

        return other &&
          other->getLow() == _low &&
          other->getHigh() == _high &&
          *other->getField() == *_field;
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        RawExtentNode* fieldDuplicate = dynamic_cast<RawExtentNode*>(_field->copy(copier));
        FieldBetweenNode* duplicate = new FieldBetweenNode( fieldDuplicate, _low, _high );
        return copier.after(this, duplicate);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _field->walk(walker);
        walker.after(this);
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put("field", _field);
        packer.put("low", _low);
        packer.put("high", _high);
        packer.after(this);
      }
    };

    class FieldEqualsNode : public RawExtentNode {
    private:
      RawExtentNode* _field;
      INT64 _constant;

    public:
      FieldEqualsNode( RawExtentNode* field, INT64 constant ) :
        _field(field),
        _constant(constant) {
      }
      
      FieldEqualsNode( Unpacker& unpacker ) {
        _field = unpacker.getRawExtentNode("field");
        _constant = unpacker.getInteger("constant");
      }

      std::string typeName() const {
        return "FieldEqualsNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#equals(" << _field->queryText() << " " << _constant << ")";
        return qtext.str();
      }

      UINT64 hashCode() const {
        return 41 +
          _constant +
          _field->hashCode();
      }

      INT64 getConstant() const {
        return _constant;
      }

      RawExtentNode* getField() {
        return _field;
      }

      bool operator== ( Node& node ) {
        FieldEqualsNode* other = dynamic_cast<FieldEqualsNode*>(&node);

        return other &&
          other->getConstant() == _constant &&
          *other->getField() == *_field;
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        RawExtentNode* fieldDuplicate = dynamic_cast<RawExtentNode*>(_field->copy(copier));
        FieldEqualsNode* duplicate = new FieldEqualsNode( fieldDuplicate, _constant );
        return copier.after(this, duplicate);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _field->walk(walker);
        walker.after(this);
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put("field", _field);
        packer.put("constant", _constant);
        packer.after(this);
      }
    };

    class RawScorerNode : public ScoredExtentNode {
    protected:
      double _occurrences; // number of occurrences within this context
      double _contextSize; // number of terms that occur within this context
      double _maximumContextFraction;
      int _documentOccurrences; // number of documents we occur in
      int _documentCount; // total number of documents

      RawExtentNode* _raw;
      RawExtentNode* _context;
      std::string _smoothing;

    public:
      RawScorerNode( RawExtentNode* raw, RawExtentNode* context, std::string smoothing = "method:dirichlet,mu:2500" ) {
        _raw = raw;
        _context = context;

        _occurrences = 0;
        _contextSize = 0;
        _documentOccurrences = 0;
        _documentCount = 0;
        _smoothing = smoothing;
      }

      RawScorerNode( Unpacker& unpacker ) {
        _raw = unpacker.getRawExtentNode( "raw" );
        _context = unpacker.getRawExtentNode( "context" );

        _occurrences = unpacker.getDouble( "occurrences" );
        _contextSize = unpacker.getDouble( "contextSize" );
        _documentOccurrences = unpacker.getInteger( "documentOccurrences" );
        _documentCount = unpacker.getInteger( "documentCount" );
        _smoothing = unpacker.getString( "smoothing" );
      }

      virtual std::string typeName() const {
        return "RawScorerNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        
        qtext << _raw->queryText();
        if( _context ) {
          // if we haven't added a period yet, put one in
          int dot = (int)qtext.str().find('.');
          if( dot < 0 )
            qtext << '.';

          qtext << "(" << _context->queryText() << ")";
        }

        return qtext.str();
      }

      virtual UINT64 hashCode() const {
        UINT64 hash = 0;

        hash += 43;
        hash += _raw->hashCode();

        if( _context ) {
          hash += _context->hashCode();
        }

        indri::utility::GenericHash<const char*> gh;
        hash += gh( _smoothing.c_str() );

        return hash;
      }

      double getOccurrences() const {
        return _occurrences;
      }

      double getContextSize() const {
        return _contextSize;
      }

      int getDocumentOccurrences() const {
        return _documentOccurrences;
      }

      int getDocumentCount() const {
        return _documentCount;
      }
      
      const std::string& getSmoothing() const {
        return _smoothing;
      }

      void setStatistics( double occurrences, double contextSize, int documentOccurrences, int documentCount ) {
        _occurrences = occurrences;
        _contextSize = contextSize;
        _documentOccurrences = documentOccurrences;
        _documentCount = documentCount;
      }

      void setContext( RawExtentNode* context ) {
        _context = context;
      }

      void setRawExtent( RawExtentNode* rawExtent ) {
        _raw = rawExtent;
      }

      void setSmoothing( const std::string& smoothing ) {
        _smoothing = smoothing;
      }

      RawExtentNode* getContext() {
        return _context;
      }

      RawExtentNode* getRawExtent() {
        return _raw;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "raw", _raw );
        packer.put( "context", _context );

        packer.put( "occurrences", _occurrences );
        packer.put( "contextSize", _contextSize );
        packer.put( "documentOccurrences", _documentOccurrences );
        packer.put( "documentCount", _documentCount );
        packer.put( "smoothing", _smoothing );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        if( _raw )
          _raw->walk(walker);
        if( _context )
          _context->walk(walker);
        walker.after(this);
      }

      virtual Node* copy( Copier& copier ) {
        copier.before(this);

        RawExtentNode* duplicateContext = _context ? dynamic_cast<RawExtentNode*>(_context->copy(copier)) : 0;
        RawExtentNode* duplicateRaw = _raw ? dynamic_cast<RawExtentNode*>(_raw->copy(copier)) : 0;
        RawScorerNode* duplicate = new RawScorerNode(*this);
        duplicate->setRawExtent( duplicateRaw );
        duplicate->setContext( duplicateContext );

        return copier.after(this, duplicate);
      }
    };

    class TermFrequencyScorerNode : public ScoredExtentNode {
    private:
      double _occurrences; // number of occurrences within this context
      double _contextSize; // number of terms that occur within this context
      int _documentOccurrences; // number of documents we occur in
      int _documentCount; // total number of documents

      std::string _text;
      std::string _smoothing;
      bool _stemmed;

    public:
      TermFrequencyScorerNode( const std::string& text, bool stemmed ) {
        _occurrences = 0;
        _contextSize = 0;
        _documentOccurrences = 0;
        _documentCount = 0;
        _smoothing = "";
        _text = text;
        _stemmed = stemmed;
      }

      TermFrequencyScorerNode( Unpacker& unpacker ) {
        _occurrences = unpacker.getDouble( "occurrences" );
        _contextSize = unpacker.getDouble( "contextSize" );
        _documentOccurrences = unpacker.getInteger( "documentOccurrences" );
        _documentCount = unpacker.getInteger( "documentCount" );
        _smoothing = unpacker.getString( "smoothing" );
        _text = unpacker.getString( "text" );
        _stemmed = unpacker.getBoolean( "stemmed" );
      }
      
      const std::string& getText() const {
        return _text;
      }

      bool getStemmed() const {
        return _stemmed;
      }

      std::string typeName() const {
        return "TermFrequencyScorerNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        
        if( !_stemmed )
          qtext << _text;
        else
          qtext << "\"" << _text << "\"";

        return qtext.str();
      }

      UINT64 hashCode() const {
        int accumulator = 47;

        if( _stemmed )
          accumulator += 3;

        indri::utility::GenericHash<const char*> hash;
        return accumulator + hash( _text.c_str() ) * 7 + hash( _smoothing.c_str() );
      }

      double getOccurrences() const {
        return _occurrences;
      }

      double getContextSize() const {
        return _contextSize;
      }

      int getDocumentOccurrences() const {
        return _documentOccurrences;
      }

      int getDocumentCount() const {
        return _documentCount;
      }

      const std::string& getSmoothing() const {
        return _smoothing;
      }

      void setStatistics( double occurrences, double contextSize, int documentOccurrences, int documentCount ) {
        _occurrences = occurrences;
        _contextSize = contextSize;
        _documentOccurrences = documentOccurrences;
        _documentCount = documentCount;
      }

      void setSmoothing( const std::string& smoothing ) {
        _smoothing = smoothing;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "occurrences", _occurrences );
        packer.put( "contextSize", _contextSize );
        packer.put( "documentOccurrences", _documentOccurrences );
        packer.put( "documentCount", _documentCount );
        packer.put( "text", _text );
        packer.put( "stemmed", _stemmed );
        packer.put( "smoothing", _smoothing );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        TermFrequencyScorerNode* duplicate = new TermFrequencyScorerNode(*this);
        return copier.after(this, duplicate);
      }
    };

    // The CachedFrequencyScorerNode should only be used on a local machine;
    // it should not be transferred across the network
    class CachedFrequencyScorerNode : public indri::lang::ScoredExtentNode {
    private:
      indri::lang::Node* _raw;
      indri::lang::Node* _context;
      std::string _smoothing;
      void* _list;
    
    public:
      CachedFrequencyScorerNode( indri::lang::Node* raw, indri::lang::Node* context )
        :
        _raw(raw),
        _context(context),
        _list(0)
      {
      }

      CachedFrequencyScorerNode( Unpacker& unpacker ) {
        LEMUR_THROW( LEMUR_RUNTIME_ERROR, "CachedFrequencyScorerNode should not be used on the network" );

        _raw = unpacker.getRawExtentNode( "raw" );
        _context = unpacker.getRawExtentNode( "context" );
        _smoothing = unpacker.getString( "smoothing" );
      }

      void setList( void* list ) {
        _list = list;
      }

      void* getList() {
        return _list;
      }

      std::string typeName() const {
        return "CachedFrequencyScorerNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        
        qtext << _raw->queryText();
        if( _context ) {
          // if we haven't added a period yet, put one in
          int dot = (int)qtext.str().find('.');
          if( dot < 0 )
            qtext << '.';

          qtext << "(" << _context->queryText() << ")";
        }

        return qtext.str();
      }

      UINT64 hashCode() const {
        UINT64 accumulator = 53;

        indri::utility::GenericHash<const char*> hash;
        return _raw->hashCode() * 7 + 
          _context->hashCode() + 
          hash( _smoothing.c_str() );
      }

      void setSmoothing( const std::string& smoothing ) {
        _smoothing = smoothing;
      }

      const std::string& getSmoothing() const {
        return _smoothing;
      }

      indri::lang::Node* getRaw() {
        return _raw;
      }

      indri::lang::Node* getContext() {
        return _context;
      }
      
      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "raw", _raw );
        packer.put( "context", _context );
        packer.put( "smoothing", _smoothing );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _raw->walk( walker );
        if( _context )
          _context->walk( walker );
        walker.after(this);
      }

      indri::lang::Node* copy( Copier& copier ) {
        copier.before(this);

        indri::lang::RawExtentNode* duplicateRaw = dynamic_cast<indri::lang::RawExtentNode*>(_raw->copy(copier));
        indri::lang::RawExtentNode* duplicateContext = 0;

        if( _context ) 
          duplicateContext = dynamic_cast<indri::lang::RawExtentNode*>(_context->copy(copier));

        CachedFrequencyScorerNode* duplicate = new CachedFrequencyScorerNode( duplicateRaw,
                                                                              duplicateContext );
        duplicate->setNodeName( nodeName() );
        duplicate->setSmoothing( _smoothing );
        duplicate->setList( getList() );

        return copier.after( this, duplicate );
      }
    };

    class PriorNode : public ScoredExtentNode {
    private:
      std::string _priorName;

    public:
      PriorNode( const std::string& priorName ) :
        _priorName( priorName )
      {
      } 
      
      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#prior(" << _priorName << ")";
        return qtext.str();
      }

      PriorNode( Unpacker& unpacker ) {
        _priorName = unpacker.getString( "priorName" );
      }

      std::string typeName() const {
        return "PriorNode";
      }
      
      UINT64 hashCode() const {
        indri::utility::GenericHash<const char*> hash;
        return hash( _priorName.c_str() ) + 9;
      }
      
      const std::string& getPriorName() const {
        return _priorName;
      }

      void walk( Walker& walker ) {
        walker.before(this);
        walker.after(this);
      }

      indri::lang::Node* copy( Copier& copier ) {
        copier.before(this);
        PriorNode* duplicate = new PriorNode( this->_priorName );
        duplicate->setNodeName( nodeName() );
        return copier.after(this, duplicate);
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "priorName", _priorName );
        packer.after(this);
      }
    };

    /* abstract */ class UnweightedCombinationNode : public ScoredExtentNode {
    protected:
      std::vector<ScoredExtentNode*> _children;

      void _unpack( Unpacker& unpacker ) {
        _children = unpacker.getScoredExtentVector( "children" );
      }

      UINT64 _hashCode() const {
        UINT64 accumulator = 0;

        for( size_t i=0; i<_children.size(); i++ ) {
          accumulator += _children[i]->hashCode();
        }

        return accumulator;
      }

      template<class _ThisType>
      void _walk( _ThisType* ptr, Walker& walker ) {
        walker.before(ptr);

        for( size_t i=0; i<_children.size(); i++ ) {
          _children[i]->walk(walker);
        }
        
        walker.after(ptr);
      }

      template<class _ThisType>
      Node* _copy( _ThisType* ptr, Copier& copier ) {
        copier.before(ptr);
        
        _ThisType* duplicate = new _ThisType();
        duplicate->setNodeName( nodeName() );
        for( size_t i=0; i<_children.size(); i++ ) {
          duplicate->addChild( dynamic_cast<ScoredExtentNode*>(_children[i]->copy(copier)) );
        } 

        return copier.after(ptr, duplicate);
      }

      void _childText( std::stringstream& qtext ) const {
        for( size_t i=0; i<_children.size(); i++ ) {
          if(i>0) qtext << " ";
          qtext << _children[i]->queryText();
        }
      }

    public:
      const std::vector<ScoredExtentNode*>& getChildren() {
        return _children;
      }

      void addChild( ScoredExtentNode* scoredNode ) {
        _children.push_back( scoredNode );
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "children", _children );
        packer.after(this);
      }
    };

    /* abstract */ class WeightedCombinationNode : public ScoredExtentNode {
    protected:
      std::vector< std::pair<double, ScoredExtentNode*> > _children;

      void _unpack( Unpacker& unpacker ) {
        std::vector<double> weights = unpacker.getDoubleVector( "weights" );
        std::vector<ScoredExtentNode*> nodes = unpacker.getScoredExtentVector( "children" );

        for( size_t i=0; i<weights.size(); i++ ) {
          _children.push_back( std::make_pair( weights[i], nodes[i] ) );
        }
      }

      UINT64 _hashCode() const {
        UINT64 accumulator = 0;

        for( size_t i=0; i<_children.size(); i++ ) {
          accumulator += (UINT64) (_children[i].first * 1000) + _children[i].second->hashCode();
        }

        return accumulator;
      }

      template<class _ThisType>
      void _walk( _ThisType* ptr, Walker& walker ) {
        walker.before(ptr);
        for( size_t i=0; i<_children.size(); i++ ) {
          _children[i].second->walk(walker);
        }
        walker.after(ptr);
      }

      template<class _ThisType>
      Node* _copy( _ThisType* ptr, Copier& copier ) {
        copier.before(ptr);

        _ThisType* duplicate = new _ThisType;
        duplicate->setNodeName( nodeName() );
        for( size_t i=0; i<_children.size(); i++ ) {
          double childWeight = _children[i].first;
          Node* childCopy = _children[i].second->copy( copier );

          duplicate->addChild( childWeight, dynamic_cast<ScoredExtentNode*>(childCopy) );
        }
        return copier.after(ptr, duplicate);
      }

      void _childText( std::stringstream& qtext ) const {
        for( size_t i=0; i<_children.size(); i++ ) {
          if(i>0) qtext << " ";
          qtext << _children[i].first
                << " "
                << _children[i].second->queryText();
        }
      }

    public:
      const std::vector< std::pair<double, ScoredExtentNode*> >& getChildren() {
        return _children;
      }

      void addChild( double weight, ScoredExtentNode* scoredNode ) {
        _children.push_back( std::make_pair( weight, scoredNode) );
      }

      void addChild( const std::string& weight, ScoredExtentNode* scoredNode ) {
        addChild( atof( weight.c_str() ), scoredNode );
      }

      void pack( Packer& packer ) {
        packer.before(this);
        
        std::vector<double> weights;
        std::vector<ScoredExtentNode*> nodes;

        for( size_t i=0; i<_children.size(); i++ ) {
          weights.push_back( _children[i].first );
          nodes.push_back( _children[i].second );
        }

        packer.put( "weights", weights );
        packer.put( "children", nodes );
        packer.after(this);
      }
    };

    class OrNode : public UnweightedCombinationNode {
    public:
      OrNode() {}
      OrNode( Unpacker& unpacker ) {
        _unpack( unpacker );
      }

      std::string typeName() const {
        return "OrNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#or(";
        _childText(qtext);
        qtext << ")";

        return qtext.str();
      } 

      UINT64 hashCode() const {
        return 55 + _hashCode();
      }

      void walk( Walker& walker ) {
        _walk( this, walker );
      }
      
      Node* copy( Copier& copier ) {
        return _copy( this, copier );
      }
    };

    class NotNode : public ScoredExtentNode {
    private:
      ScoredExtentNode* _child;

    public:
      NotNode() {
        _child = 0;
      }

      NotNode( ScoredExtentNode* child ) {
        _child = child;
      }

      NotNode( Unpacker& unpacker ) {
        _child = unpacker.getScoredExtentNode( "child" );
      }

      std::string typeName() const {
        return "NotNode";
      }

      ScoredExtentNode* getChild() {
        return _child;
      }

      void setChild( ScoredExtentNode* child ) {
        _child = child;
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#not(";
        qtext << _child->queryText();
        qtext << ")";

        return qtext.str();
      } 

      UINT64 hashCode() const {
        return 101 + _child->hashCode();
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _child->walk(walker);
        walker.after(this);
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "child", _child );
        packer.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before( this );
        ScoredExtentNode* childDuplicate = dynamic_cast<ScoredExtentNode*>(_child->copy(copier));
        NotNode* duplicate = new NotNode( childDuplicate );
        duplicate->setNodeName( nodeName() );
        return copier.after( this, duplicate );
      }
    };

    class MaxNode : public UnweightedCombinationNode {
    public:
      MaxNode() {}
      MaxNode( Unpacker& unpacker ) {
        _unpack( unpacker );
      }

      std::string typeName() const {
        return "MaxNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#max(";
        _childText(qtext);
        qtext << ")";

        return qtext.str();
      } 

      UINT64 hashCode() const {
        return 57 + _hashCode();
      }

      void walk( Walker& walker ) {
        _walk(this, walker);
      }

      Node* copy( Copier& copier ) {
        return _copy( this, copier );
      }
    };

    class CombineNode : public UnweightedCombinationNode {
    public:
      CombineNode() {}
      CombineNode( Unpacker& unpacker ) {
        _unpack( unpacker );
      }

      std::string typeName() const {
        return "CombineNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#combine(";
        _childText(qtext);
        qtext << ")";

        return qtext.str();
      } 

      UINT64 hashCode() const {
        return 59 + _hashCode();
      }

      void walk( Walker& walker ) {
        _walk( this, walker );
      }
      
      Node* copy( Copier& copier ) {
        return _copy( this, copier );
      }
    };

    class WAndNode : public WeightedCombinationNode {
    public:
      WAndNode() {}
      WAndNode( Unpacker& unpacker ) {
        _unpack( unpacker );
      }

      std::string typeName() const {
        return "WAndNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#wand(";
        _childText(qtext);
        qtext << ")";

        return qtext.str();
      } 

      UINT64 hashCode() const {
        return 61 + _hashCode();
      }

      void walk( Walker& walker ) {
        _walk( this, walker );
      }

      Node* copy( Copier& copier ) {
        return _copy( this, copier );
      }
    };

    class WSumNode : public WeightedCombinationNode {
    public:
      WSumNode() {}
      WSumNode( Unpacker& unpacker ) {
        _unpack( unpacker );
      }

      std::string typeName() const {
        return "WSumNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#wsum(";
        _childText(qtext);
        qtext << ")";

        return qtext.str();
      } 

      UINT64 hashCode() const {
        return 67 + _hashCode();
      }

      void walk( Walker& walker ) {
        _walk( this, walker );
      }

      Node* copy( Copier& copier ) {
        return _copy( this, copier );
      }
    };

    class WeightNode : public WeightedCombinationNode {
    public:
      WeightNode() {}
      WeightNode( Unpacker& unpacker ) {
        _unpack( unpacker );
      }

      std::string typeName() const {
        return "WeightNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#weight(";
        _childText(qtext);
        qtext << ")";

        return qtext.str();
      }

      UINT64 hashCode() const {
        return 71 + _hashCode();
      }

      void walk( Walker& walker ) {
        _walk( this, walker );
      }

      Node* copy( Copier& copier ) {
        return _copy( this, copier );
      }
    };

    class ExtentRestriction : public ScoredExtentNode {
    protected:
      ScoredExtentNode* _child;
      RawExtentNode* _field;

    public:
      ExtentRestriction( Unpacker& unpacker ) {
        _child = unpacker.getScoredExtentNode("child");
        _field = unpacker.getRawExtentNode("field");
      }

      ExtentRestriction( ScoredExtentNode* child, RawExtentNode* field ) :
        _child(child),
        _field(field)
      {
      }

      virtual std::string typeName() const {
        return "ExtentRestriction";
      }

      std::string queryText() const {
        std::stringstream qtext;
        // this extent restriction is almost certainly because of some #combine or #max operator
        // in the _child position.  We look for the first parenthesis (e.g. #combine(dog cat)) and
        // insert the brackets in.
        
        std::string childText = _child->queryText();
        std::string::size_type pos = childText.find( '(' );

        if( pos != std::string::npos ) {
          qtext << childText.substr(0,pos) 
                << "["
                << _field->queryText()
                << "]"
                << childText.substr(pos);
        } else {
          // couldn't find a parenthesis, so we'll tack the [field] on the front
          qtext << "["
                << _field->queryText()
                << "]"
                << childText;
        }

        return qtext.str();
      }

      virtual UINT64 hashCode() const {
        return 79 + _child->hashCode() * 7 + _field->hashCode();
      }

      ScoredExtentNode* getChild() {
        return _child;
      }

      RawExtentNode* getField() {
        return _field;
      }

      void setChild( ScoredExtentNode* child ) {
        _child = child;
      }

      void setField( RawExtentNode* field ) {
        _field = field;
      }
      
      void pack( Packer& packer ) {
        packer.before(this);
        packer.put("child", _child);
        packer.put("field", _field);
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _child->walk(walker);
        _field->walk(walker);
        walker.after(this);
      }

      virtual Node* copy( Copier& copier ) {
        copier.before(this);

        ScoredExtentNode* duplicateChild = dynamic_cast<indri::lang::ScoredExtentNode*>(_child->copy(copier));
        RawExtentNode* duplicateField = dynamic_cast<indri::lang::RawExtentNode*>(_field->copy(copier));
        ExtentRestriction* duplicate = new ExtentRestriction( duplicateChild, duplicateField );
        duplicate->setNodeName( nodeName() );
        
        return copier.after(this, duplicate);
      }
    };

    class FixedPassage : public ScoredExtentNode {
    private:
      ScoredExtentNode* _child;
      int _windowSize;
      int _increment;

    public:
      FixedPassage( Unpacker& unpacker ) {
        _child = unpacker.getScoredExtentNode("child");
        _windowSize = (int)unpacker.getInteger("windowSize");
        _increment = (int)unpacker.getInteger("increment");
      }

      FixedPassage( ScoredExtentNode* child, int windowSize, int increment ) :
        _child(child),
        _windowSize(windowSize),
        _increment(increment)
      {
      }

      std::string typeName() const {
        return "FixedPassage";
      }

      std::string queryText() const {
        std::stringstream qtext;
        // this extent restriction is almost certainly because of some #combine or #max operator
        // in the _child position.  We look for the first parenthesis (e.g. #combine(dog cat)) and
        // insert the brackets in.
        
        std::string childText = _child->queryText();
        std::string::size_type pos = childText.find( '(' );

        if( pos != std::string::npos ) {
          qtext << childText.substr(0,pos) 
                << "[passage"
                << _windowSize
                << ":"
                << _increment
                << "]"
                << childText.substr(pos);
        } else {
          // couldn't find a parenthesis, so we'll tack the [field] on the front
          qtext << "[passage"
                << _windowSize
                << ":"
                << _increment
                << "]"
                << childText;
        }

        return qtext.str();
      } 

      UINT64 hashCode() const {
        return 83 + _child->hashCode() + _windowSize * 3 + _increment;
      }

      ScoredExtentNode* getChild() {
        return _child;
      }

      int getWindowSize() {
        return _windowSize;
      }

      int getIncrement() {
        return _increment;
      }

      void setChild( ScoredExtentNode* child ) {
        _child = child;
      }

      void setWindowSize( int windowSize ) {
        _windowSize = windowSize;
      }

      void setIncrement( int increment ) {
        _increment = increment;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put("child", _child);
        packer.put("increment", _increment);
        packer.put("windowSize", _windowSize);
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _child->walk(walker);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);

        ScoredExtentNode* duplicateChild = dynamic_cast<indri::lang::ScoredExtentNode*>(_child->copy(copier));
        FixedPassage* duplicate = new FixedPassage( duplicateChild,
                                                    _windowSize,
                                                    _increment );
        duplicate->setNodeName( nodeName() );
        
        return copier.after(this, duplicate);
      }
    };

    class FilterNode : public ScoredExtentNode {
    private:
      ScoredExtentNode* _child;
      std::vector<lemur::api::DOCID_T> _documents;

    public:
      FilterNode( ScoredExtentNode* child, std::vector<lemur::api::DOCID_T>& documents ) : 
        _child(child),
        _documents(documents)
      {
      }

      FilterNode( Unpacker& unpacker ) {
        _child = unpacker.getScoredExtentNode( "child" );
        _documents = unpacker.getDocIdVector( "documents" );
      }

      std::string typeName() const {
        return "FilterNode";
      }

      ScoredExtentNode* getChild() {
        return _child;
      }

      const std::vector<lemur::api::DOCID_T>& getDocuments() const {
        return _documents;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put("child", _child);
        packer.put("documents", _documents);
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _child->walk(walker);
        walker.after(this);
      }

      std::string queryText() const {
        // for now, we'll let the filter be anonymous, since it can never
        // be typed by the user
        return _child->queryText();
      }

      UINT64 hashCode() const {
        UINT64 documentSum = 0;

        for( size_t i=0; i<_documents.size(); i++ ) {
          documentSum += _documents[i];
        }

        return 87 + _child->hashCode() + documentSum;
      }

      Node* copy( Copier& copier ) {
        copier.before(this);

        ScoredExtentNode* duplicateChild = dynamic_cast<ScoredExtentNode*>(_child->copy(copier));
        FilterNode* duplicate = new FilterNode( duplicateChild, _documents );
        duplicate->setNodeName( nodeName() );

        return copier.after(this, duplicate);
      }
    };

    class ListAccumulator : public AccumulatorNode {
    private:
      RawExtentNode* _raw;
     
    public:
      ListAccumulator( RawExtentNode* raw ) :
        _raw(raw)
      {
      }

      ListAccumulator( Unpacker& unpacker ) {
        _raw = unpacker.getRawExtentNode( "raw" );
      }

      std::string typeName() const {
        return "ListAccumulator";
      }

      std::string queryText() const {
        return _raw->queryText();
      }

      UINT64 hashCode() const {
        // we don't use hashCodes for accumulatorNodes
        return 0;
      }

      RawExtentNode* getRawExtent() {
        return _raw;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "raw", _raw );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _raw->walk( walker );
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        RawExtentNode* rawCopy = dynamic_cast<RawExtentNode*>(_raw->copy( copier ));
        ListAccumulator* duplicate = new ListAccumulator( rawCopy );
        duplicate->setNodeName( nodeName() );
        return copier.after(this, duplicate);
      }
    };

    class ContextCounterNode : public AccumulatorNode {
    private:
      RawExtentNode* _raw;
      RawExtentNode* _context;
      bool _hasCounts;
      bool _hasContextSize;
      double _occurrences;
      double _contextSize;
      int _documentOccurrences; // number of documents we occur in
      int _documentCount; // total number of documents

    public:
      ContextCounterNode( RawExtentNode* raw, RawExtentNode* context ) :
        _hasCounts(false),
        _hasContextSize(false),
        _occurrences(0),
        _contextSize(0),
        _documentOccurrences(0),
        _documentCount(0)
      {
        _raw = raw;
        _context = context;
      }

      ContextCounterNode( Unpacker& unpacker ) {
        _raw = unpacker.getRawExtentNode( "raw" );
        _context = unpacker.getRawExtentNode( "context" );
        _occurrences = unpacker.getDouble( "occurrences" );
        _contextSize = unpacker.getDouble( "contextSize" );
        _documentOccurrences = unpacker.getInteger( "documentOccurrences" );
        _documentCount = unpacker.getInteger( "documentCount" );

        _hasCounts = unpacker.getBoolean( "hasCounts" );
        _hasContextSize = unpacker.getBoolean( "hasContextSize" );
      }

      std::string typeName() const {
        return "ContextCounterNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        
        if( _raw )
          qtext << _raw->queryText();

        if( _context ) {
          // if we haven't added a period yet, put one in
          int dot = (int)qtext.str().find('.');
          if( dot < 0 )
            qtext << '.';

          qtext << "(" << _context->queryText() << ")";
        }

        return qtext.str();
      }

      UINT64 hashCode() const {
        // we don't use hashCodes for accumulatorNodes
        return 0;
      }

      RawExtentNode* getContext() {
        return _context;
      }

      RawExtentNode* getRawExtent() {
        return _raw;
      }

      void setRawExtent( RawExtentNode* rawExtent ) {
        _raw = rawExtent;
      }

      void setContext( RawExtentNode* context ) {
        _context = context;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "raw", _raw );
        packer.put( "context", _context );
        packer.put( "occurrences", _occurrences );
        packer.put( "contextSize", _contextSize );
        packer.put( "documentOccurrences", _documentOccurrences );
        packer.put( "documentCount", _documentCount );

        packer.put( "hasCounts", _hasCounts );
        packer.put( "hasContextSize", _hasContextSize );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        if( _raw ) _raw->walk(walker);
        if( _context ) _context->walk(walker);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        RawExtentNode* duplicateRaw = _raw ? dynamic_cast<RawExtentNode*>(_raw->copy(copier)) : 0;
        RawExtentNode* duplicateContext = _context ? dynamic_cast<RawExtentNode*>(_context->copy(copier)) : 0;
        ContextCounterNode* duplicate = new ContextCounterNode(*this);
        duplicate->setContext(duplicateContext);
        duplicate->setRawExtent(duplicateRaw);
        return copier.after(this, duplicate);
      }

      bool hasCounts() const {
        return _hasCounts;
      }

      bool hasContextSize() const {
        return _hasContextSize;
      }

      double getOccurrences() const {
        return _occurrences;
      }

      double getContextSize() const {
        return _contextSize;
      }

      int getDocumentOccurrences() const {
        return _documentOccurrences;
      }

      int getDocumentCount() const {
        return _documentCount;
      }

      void setContextSize( double contextSize ) {
        _contextSize = contextSize;
        _hasContextSize = true;
      }

      void setCounts( double occurrences,
                      double contextSize, int documentOccurrences, 
                      int documentCount ) {
        _hasCounts = true;
        _occurrences = occurrences;
        setContextSize( contextSize );
        _documentOccurrences = documentOccurrences;
        _documentCount = documentCount;
      }
    };

    class ContextSimpleCounterNode : public AccumulatorNode {
    private:
      std::vector<std::string> _terms;
      std::string _field;
      std::string _context;

      bool _hasCounts;
      bool _hasContextSize;
      double _occurrences;
      double _contextSize;
      int _documentOccurrences; // number of documents we occur in
      int _documentCount; // total number of documents

    public:
      ContextSimpleCounterNode( const std::vector<std::string>& terms, const std::string& field, const std::string& context ) :
        _hasCounts(false),
        _hasContextSize(false),
        _occurrences(0),
        _contextSize(0),
        _terms(terms),
        _field(field),
        _context(context),
        _documentOccurrences(0),
        _documentCount(0)
      {
      }

      ContextSimpleCounterNode( Unpacker& unpacker ) {
        _occurrences = unpacker.getDouble( "occurrences" );
        _contextSize = unpacker.getDouble( "contextSize" );

        _terms = unpacker.getStringVector( "terms" );
        _field = unpacker.getString( "field" );
        _context = unpacker.getString( "context" );
        _documentOccurrences = unpacker.getInteger( "documentOccurrences" );
        _documentCount = unpacker.getInteger( "documentCount" );

        _hasCounts = unpacker.getBoolean( "hasCounts" );
        _hasContextSize = unpacker.getBoolean( "hasContextSize" );
      }

      std::string typeName() const {
        return "ContextSimpleCounterNode";
      }

      std::string queryText() const {
        // nothing to see here -- this is an optimization node
        return std::string();
      }

      UINT64 hashCode() const {
        // we don't use hashCodes for accumulatorNodes
        return 0;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "occurrences", _occurrences );
        packer.put( "contextSize", _contextSize );
        packer.put( "documentOccurrences", _documentOccurrences );
        packer.put( "documentCount", _documentCount );

        packer.put( "terms", _terms );
        packer.put( "field", _field );
        packer.put( "context", _context );

        packer.put( "hasCounts", _hasCounts );
        packer.put( "hasContextSize", _hasContextSize );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        ContextSimpleCounterNode* duplicate = new ContextSimpleCounterNode(*this);
        return copier.after(this, duplicate);
      }

      bool hasCounts() const {
        return _hasCounts;
      }

      bool hasContextSize() const {
        return _hasContextSize;
      }

      double getOccurrences() const {
        return _occurrences;
      }

      double getContextSize() const {
        return _contextSize;
      }

      int getDocumentOccurrences() const {
        return _documentOccurrences;
      }

      int getDocumentCount() const {
        return _documentCount;
      }

      const std::vector<std::string>& terms() const {
        return _terms;
      }

      const std::string& field() const {
        return _field;
      }

      const std::string& context() const {
        return _context;
      }

      void setContextSize( double contextSize ) {
        _contextSize = contextSize;
        _hasContextSize = true;
      }

      void setCounts( double occurrences,
                      double contextSize, int documentOccurrences, 
                      int documentCount ) {
        _hasCounts = true;
        _occurrences = occurrences;
        setContextSize( contextSize );
        _documentOccurrences = documentOccurrences;
        _documentCount = documentCount;
      }
    };

    class ScoreAccumulatorNode : public AccumulatorNode {
    private:
      ScoredExtentNode* _scoredNode;

    public:
      ScoreAccumulatorNode( ScoredExtentNode* scoredNode ) :
        _scoredNode(scoredNode)
      {
      }

      ScoreAccumulatorNode( Unpacker& unpacker ) {
        _scoredNode = unpacker.getScoredExtentNode( "scoredNode" );
      }

      std::string typeName() const {
        return "ScoreAccumulatorNode";
      }

      std::string queryText() const {
        // anonymous
        return _scoredNode->queryText();
      }

      UINT64 hashCode() const {
        // we don't use hashCodes for accumulatorNodes
        return 0;
      }

      ScoredExtentNode* getChild() {
        return _scoredNode;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "scoredNode", _scoredNode );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _scoredNode->walk(walker);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        ScoredExtentNode* duplicateChild = dynamic_cast<ScoredExtentNode*>(_scoredNode->copy(copier));
        ScoreAccumulatorNode* duplicate = new ScoreAccumulatorNode(duplicateChild);
        duplicate->setNodeName( nodeName() );
        return copier.after(this, duplicate);
      }
    };

    class AnnotatorNode : public AccumulatorNode {
    private:
      ScoredExtentNode* _scoredNode;

    public:
      AnnotatorNode( ScoredExtentNode* scoredNode ) :
        _scoredNode(scoredNode)
      {
      }

      AnnotatorNode( Unpacker& unpacker ) {
        _scoredNode = unpacker.getScoredExtentNode( "scoredNode" );
      }

      std::string typeName() const {
        return "AnnotatorNode";
      }

      std::string queryText() const {
        return _scoredNode->queryText();
      }

      UINT64 hashCode() const {
        // we don't use hashCodes for accumulatorNodes
        return 0;
      }

      ScoredExtentNode* getChild() {
        return _scoredNode;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "scoredNode", _scoredNode );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _scoredNode->walk(walker);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        ScoredExtentNode* duplicateChild = dynamic_cast<ScoredExtentNode*>(_scoredNode->copy(copier));
        AnnotatorNode* duplicate = new AnnotatorNode(duplicateChild);
        duplicate->setNodeName( nodeName() );
        return copier.after(this, duplicate);
      }
    };

    
    class FieldWildcard : public RawExtentNode {
    private:

    public:
      FieldWildcard(  )
      {
      }

      FieldWildcard( Unpacker& unpacker ) {
      }

      bool operator== ( Node& o ) {
        FieldWildcard* other = dynamic_cast<FieldWildcard*>(&o);
        return other != NULL; // all instances are ==
      }

      std::string typeName() const {
        return "FieldWildcard";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "*";

        return qtext.str();
      }


      void pack( Packer& packer ) {
        packer.before(this);
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        walker.after(this);
      }

      UINT64 hashCode() const {
        return 103;//???????????????
      }

      Node* copy( Copier& copier ) {
        copier.before(this);

        FieldWildcard* fieldWildcardCopy = new FieldWildcard;
        fieldWildcardCopy->setNodeName( nodeName() );

        return copier.after(this, fieldWildcardCopy);
      }
    };

    class NestedExtentInside : public ExtentInside {

    public:
      NestedExtentInside( RawExtentNode* inner, RawExtentNode* outer ) :
        ExtentInside( inner, outer )
      {
      }

      NestedExtentInside( Unpacker& unpacker ):
        ExtentInside( unpacker ) 
      { 
      }

      bool operator== ( Node& o ) {
        NestedExtentInside* other = dynamic_cast<NestedExtentInside*>(&o);
  
        return other &&
          *_inner == *other->_inner &&
          *_outer == *other->_outer;
      }
      
      std::string typeName() const {
        return "NestedExtentInside";
      }

      UINT64 hashCode() const {
        return 107 + _inner->hashCode() + (_inner->hashCode() * 7);//???????????????
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        
        RawExtentNode* newInner = dynamic_cast<RawExtentNode*>(_inner->copy(copier));
        RawExtentNode* newOuter = dynamic_cast<RawExtentNode*>(_outer->copy(copier));
        NestedExtentInside* extentInsideCopy = new NestedExtentInside( newInner, newOuter );
        extentInsideCopy->setNodeName( nodeName() );

        return copier.after(this, extentInsideCopy);
      }
    };

    class NestedRawScorerNode : public RawScorerNode {

    public:
      NestedRawScorerNode( RawExtentNode* raw, RawExtentNode* context, std::string smoothing = "method:dirichlet,mu:2500" ) :
        RawScorerNode( raw, context, smoothing )
      {
      }

      NestedRawScorerNode( Unpacker& unpacker ) :
        RawScorerNode( unpacker )
      {
      }

      std::string typeName() const {
        return "NestedRawScorerNode";
      }

      UINT64 hashCode() const {
        UINT64 hash = 0;

        hash += 105;
        hash += _raw->hashCode();

        if( _context ) {
          hash += _context->hashCode();
        }

        indri::utility::GenericHash<const char*> gh;
        hash += gh( _smoothing.c_str() );

        return hash;
      }

      Node* copy( Copier& copier ) {
        copier.before(this);

        RawExtentNode* duplicateContext = _context ? dynamic_cast<RawExtentNode*>(_context->copy(copier)) : 0;
        RawExtentNode* duplicateRaw = _raw ? dynamic_cast<RawExtentNode*>(_raw->copy(copier)) : 0;
        NestedRawScorerNode* duplicate = new NestedRawScorerNode(*this);
        duplicate->setRawExtent( duplicateRaw );
        duplicate->setContext( duplicateContext );

        return copier.after(this, duplicate);
      }
    };
   

    class ExtentEnforcement : public ExtentRestriction {

    public:
      ExtentEnforcement( Unpacker& unpacker ) :
        ExtentRestriction( unpacker ) {
      }

      ExtentEnforcement( ScoredExtentNode* child, RawExtentNode* field ) :
        ExtentRestriction( child, field )
      {
      }

      std::string typeName() const {
        return "ExtentEnforcement";
      }

      
      UINT64 hashCode() const {
        return 109 + _child->hashCode() * 7 + _field->hashCode();//??????????????
      }


      Node* copy( Copier& copier ) {
        copier.before(this);

        ScoredExtentNode* duplicateChild = dynamic_cast<indri::lang::ScoredExtentNode*>(_child->copy(copier));
        RawExtentNode* duplicateField = dynamic_cast<indri::lang::RawExtentNode*>(_field->copy(copier));
        ExtentEnforcement* duplicate = new ExtentEnforcement( duplicateChild, duplicateField );
        duplicate->setNodeName( nodeName() );
        
        return copier.after(this, duplicate);
      }
    };

    class ContextInclusionNode : public ScoredExtentNode {
    protected:
      std::vector<ScoredExtentNode*> _children;
      ScoredExtentNode* _preserveExtentsChild;

      void _unpack( Unpacker& unpacker ) {
        _children = unpacker.getScoredExtentVector( "children" );
        _preserveExtentsChild = unpacker.getScoredExtentNode( "preserveExtentsChild" );
      }

      UINT64 _hashCode() const {
        UINT64 accumulator = 0;

        for( size_t i=0; i<_children.size(); i++ ) {
          accumulator += _children[i]->hashCode();
        }

        return accumulator;
      }

      template<class _ThisType>
      void _walk( _ThisType* ptr, Walker& walker ) {
        walker.before(ptr);

        for( size_t i=0; i<_children.size(); i++ ) {
          _children[i]->walk(walker);
        }
        
        walker.after(ptr);
      }

      template<class _ThisType>
      Node* _copy( _ThisType* ptr, Copier& copier ) {
        copier.before(ptr);
        
        _ThisType* duplicate = new _ThisType();
        duplicate->setNodeName( nodeName() );
        for( size_t i=0; i<_children.size(); i++ ) {
          bool preserveExtents = false;
          if ( _preserveExtentsChild == _children[i] ) {
            preserveExtents = true;
          }
          duplicate->addChild( dynamic_cast<ScoredExtentNode*>(_children[i]->copy(copier)), preserveExtents );
        } 

        return copier.after(ptr, duplicate);
      }

      void _childText( std::stringstream& qtext ) const {
        if ( _preserveExtentsChild != 0 ) {
          qtext << _preserveExtentsChild->queryText() << " ";
        }
        for( size_t i=0; i<_children.size(); i++ ) {
          if ( _children[i] != _preserveExtentsChild ) {
            if(i>0) qtext << " ";
            qtext << _children[i]->queryText();
          }
        }
      }

    public:
      ContextInclusionNode( ) { }
      ContextInclusionNode( Unpacker & unpacker ) {
        _unpack( unpacker );
      }

      const std::vector<ScoredExtentNode*>& getChildren() {
        return _children;
      }
      
      ScoredExtentNode * getPreserveExtentsChild() {
        return _preserveExtentsChild;
      }

      void addChild( ScoredExtentNode* scoredNode, bool preserveExtents = false ) {
        if (preserveExtents == true) {
          _preserveExtentsChild = scoredNode;
        }       
        _children.push_back( scoredNode );
      }

      std::string typeName() const {
        return "ContextInclusionNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#context(";
        _childText(qtext);
        qtext << ")";

        return qtext.str();
      } 

      virtual UINT64 hashCode() const {
        return 111 + _hashCode();//?????????????
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "children", _children );
        packer.put( "preserveExtentsChild", _preserveExtentsChild);
        packer.after(this);
      }

      void walk( Walker& walker ) {
        _walk( this, walker );
      }
      
      Node* copy( Copier& copier ) {
        return _copy( this, copier );
      }
    };

    class LengthPrior : public ScoredExtentNode {
    private:
      double _exponent;
      ScoredExtentNode * _child;

    public:
      LengthPrior(ScoredExtentNode * child, double exponent) :
        _child(child), 
        _exponent(exponent)
      {
        
      }

      LengthPrior( Unpacker& unpacker ) {
        _exponent = unpacker.getDouble( "exponent" );
        _child = unpacker.getScoredExtentNode( "child" );
      }

      std::string queryText() const {
        std::stringstream qtext;
        // with the definition of priors somewhat in flux, it's
        // hard to know what would be good to put here.  It's also
        // a little hard when there realy isn't a way to 
        // specify this in either of the indri/nexi query languages.
        qtext <<  "#lengthprior(" << _exponent << ")";
        return qtext.str();
      }

      std::string nodeType() {
        return "LengthPrior";
      }

      void setExponent( double exponent ) {
        _exponent = exponent;
      }

      double getExponent() {
        return _exponent;
      }

      ScoredExtentNode* getChild() {
        return _child;
      }
      
      virtual UINT64 hashCode() const {
        return 115; //?????????
      }

      void walk( Walker& walker ) {
        walker.before(this);
        _child->walk(walker);
        walker.after(this);
      }

      indri::lang::Node* copy( Copier& copier ) {
        copier.before(this);
        ScoredExtentNode * childCopy = dynamic_cast<ScoredExtentNode*> (_child->copy( copier ) );
        LengthPrior* duplicate = new LengthPrior( childCopy, _exponent );
        return copier.after(this, duplicate);
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "exponent", _exponent );
        packer.put( "child", _child );
        packer.after(this);
      }
    };

    class DocumentStructureNode : public Node {
    public:
      
      DocumentStructureNode( ) {        
      }

      DocumentStructureNode( Unpacker& unpacker ) {     
      }

      UINT64 hashCode() const {
        return 117; //?????????
      }

      bool operator== ( Node& o ) {
        DocumentStructureNode* other = dynamic_cast<DocumentStructureNode*>(&o);
        return other != NULL; // all instances are ==
      }

      std::string typeName() const {
        return "DocumentStructure";
      }
      
      void pack( Packer& packer ) {
        packer.before(this);
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        walker.after(this);
      }

      std::string queryText() const {
        return "";
      }

      Node* copy( Copier& copier ) {
        copier.before(this);

        DocumentStructureNode* documentStructureCopy = new DocumentStructureNode;
        documentStructureCopy->setNodeName( nodeName() );

        return copier.after(this, documentStructureCopy);
      }
    };

    class ShrinkageScorerNode : public RawScorerNode {
    private:
      DocumentStructureNode* _documentStructure;
      
      std::vector<std::string> _shrinkageRules;

    public:
      ShrinkageScorerNode( RawExtentNode* raw, 
                           DocumentStructureNode* documentStructure, 
                           std::string smoothing = "method:dirichlet,mu:2500" )
        : RawScorerNode( raw, 0, smoothing ),
          _documentStructure( documentStructure ), 
          _shrinkageRules( 0 )
      {
      }

      ShrinkageScorerNode( Unpacker& unpacker ) : RawScorerNode( 0, 0, "" ) {
        _raw = unpacker.getRawExtentNode( "raw" );
        _documentStructure = unpacker.getDocumentStructureNode( "documentStructureNode" );
        _occurrences = unpacker.getDouble( "occurrences" );
        _contextSize = unpacker.getDouble( "contextSize" );
        _smoothing = unpacker.getString( "smoothing" );
        _shrinkageRules = unpacker.getStringVector( "shrinkageRules" );
      }

      std::string typeName() const {
        return "ShrinkageScorerNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        
        qtext << _raw->queryText();

        return qtext.str();
      }

      void addShrinkageRule( std::string rule ) {
        _shrinkageRules.push_back( rule );
      }
      
      std::vector<std::string> getShrinkageRules() {
        return _shrinkageRules;
      }

      void setDocumentStructure( DocumentStructureNode* docStruct ) {
        _documentStructure = docStruct;
      }

      DocumentStructureNode* getDocumentStructure() {
        return _documentStructure;
      }


      UINT64 hashCode() const {
        UINT64 hash = 0;

        hash += 119;/////////////////????
        hash += _raw->hashCode();

        if( _context ) {
          hash += _context->hashCode();
        }

        indri::utility::GenericHash<const char*> gh;
        hash += gh( _smoothing.c_str() );

        return hash;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "raw", _raw );
        packer.put( "documentStructure", _documentStructure );
        packer.put( "occurrences", _occurrences );
        packer.put( "contextSize", _contextSize );
        packer.put( "smoothing", _smoothing );
        packer.put( "shrinkageRules", _shrinkageRules );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        if( _raw )
          _raw->walk(walker);
        if( _documentStructure ) 
          _documentStructure->walk(walker);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);

        RawExtentNode* duplicateRaw = _raw ? dynamic_cast<RawExtentNode*>(_raw->copy(copier)) : 0;
        DocumentStructureNode* duplicateDocStruct = _documentStructure ? dynamic_cast<DocumentStructureNode*>(_documentStructure->copy(copier)) : 0;
        ShrinkageScorerNode* duplicate = new ShrinkageScorerNode(*this);
        duplicate->setRawExtent( duplicateRaw );
        duplicate->setDocumentStructure( duplicateDocStruct );

        std::vector<std::string>::iterator ruleIter = _shrinkageRules.begin();
        while( ruleIter != _shrinkageRules.end() ) {
          duplicate->addShrinkageRule( *ruleIter );
          ruleIter++;
        }

        return copier.after(this, duplicate);
      }
    };



    class ExtentDescendant : public ExtentInside {
    protected:

      DocumentStructureNode* _documentStructure;
      
    public:
      ExtentDescendant( RawExtentNode* inner, RawExtentNode* outer, DocumentStructureNode * docStruct ) :
        ExtentInside( inner, outer ),
        _documentStructure( docStruct)
      {
      }

      ExtentDescendant( Unpacker& unpacker ):
        ExtentInside( unpacker ) 
      { 
        _documentStructure = unpacker.getDocumentStructureNode( "documentStructureNode" );
      }

      bool operator== ( Node& o ) {
        ExtentDescendant* other = dynamic_cast<ExtentDescendant*>(&o);
  
        return other &&
          *_inner == *other->_inner &&
          *_outer == *other->_outer &&
          *_documentStructure == *other->_documentStructure;
      }
      
      void setDocumentStructure( DocumentStructureNode* docStruct ) {
        _documentStructure = docStruct;
      }

      DocumentStructureNode* getDocumentStructure() {
        return _documentStructure;
      }

      std::string typeName() const {
        return "ExtentDescendant";
      }

      UINT64 hashCode() const {
        return 125 + _inner->hashCode() + (_inner->hashCode() * 7);//???????????????
      }


      void walk( Walker& walker ) {
        walker.before(this);
        _inner->walk(walker);
        _outer->walk(walker);
        _documentStructure->walk(walker);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        
        RawExtentNode* newInner = dynamic_cast<RawExtentNode*>(_inner->copy(copier));
        RawExtentNode* newOuter = dynamic_cast<RawExtentNode*>(_outer->copy(copier));
        DocumentStructureNode * newDocStruct = dynamic_cast<DocumentStructureNode*>(_documentStructure->copy(copier));
        ExtentDescendant* extentInsideCopy = new ExtentDescendant( newInner, newOuter, newDocStruct );
        extentInsideCopy->setNodeName( nodeName() );

        return copier.after(this, extentInsideCopy);
      }
    };

    class ExtentChild : public ExtentInside {
    protected:

      DocumentStructureNode* _documentStructure;
      
    public:
      ExtentChild( RawExtentNode* inner, RawExtentNode* outer, DocumentStructureNode * docStruct ) :
        ExtentInside( inner, outer ),
        _documentStructure( docStruct)
      {
      }

      ExtentChild( Unpacker& unpacker ):
        ExtentInside( unpacker ) 
      { 
        _documentStructure = unpacker.getDocumentStructureNode( "documentStructureNode" );
      }

      bool operator== ( Node& o ) {
        ExtentChild* other = dynamic_cast<ExtentChild*>(&o);
  
        return other &&
          *_inner == *other->_inner &&
          *_outer == *other->_outer &&
          *_documentStructure == *other->_documentStructure;
      }

      void setDocumentStructure( DocumentStructureNode* docStruct ) {
        _documentStructure = docStruct;
      }

      DocumentStructureNode* getDocumentStructure() {
        return _documentStructure;
      }
     
      std::string typeName() const {
        return "ExtentChild";
      }

      UINT64 hashCode() const {
        return 129 + _inner->hashCode() + (_inner->hashCode() * 7);//???????????????
      }


      void walk( Walker& walker ) {
        walker.before(this);
        _inner->walk(walker);
        _outer->walk(walker);
        _documentStructure->walk(walker);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        
        RawExtentNode* newInner = dynamic_cast<RawExtentNode*>(_inner->copy(copier));
        RawExtentNode* newOuter = dynamic_cast<RawExtentNode*>(_outer->copy(copier));
        DocumentStructureNode * newDocStruct = dynamic_cast<DocumentStructureNode*>(_documentStructure->copy(copier));
        ExtentChild* extentInsideCopy = new ExtentChild( newInner, newOuter, newDocStruct );
        extentInsideCopy->setNodeName( nodeName() );

        return copier.after(this, extentInsideCopy);
      }
    };

    class ExtentParent : public ExtentInside {
    protected:

      DocumentStructureNode* _documentStructure;
      
    public:
      ExtentParent( RawExtentNode* inner, RawExtentNode* outer, DocumentStructureNode * docStruct ) :
        ExtentInside( inner, outer ),
        _documentStructure( docStruct)
      {
      }

      ExtentParent( Unpacker& unpacker ):
        ExtentInside( unpacker ) 
      { 
        _documentStructure = unpacker.getDocumentStructureNode( "documentStructureNode" );
      }

      bool operator== ( Node& o ) {
        ExtentParent* other = dynamic_cast<ExtentParent*>(&o);
  
        return other &&
          *_inner == *other->_inner &&
          *_outer == *other->_outer &&
          *_documentStructure == *other->_documentStructure;
      }

      void setDocumentStructure( DocumentStructureNode* docStruct ) {
        _documentStructure = docStruct;
      }

      DocumentStructureNode* getDocumentStructure() {
        return _documentStructure;
      }
     
      std::string typeName() const {
        return "ExtentParent";
      }

      UINT64 hashCode() const {
        return 129 + _inner->hashCode() + (_inner->hashCode() * 7);//???????????????
      }


      void walk( Walker& walker ) {
        walker.before(this);
        _inner->walk(walker);
        _outer->walk(walker);
        _documentStructure->walk(walker);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        
        RawExtentNode* newInner = dynamic_cast<RawExtentNode*>(_inner->copy(copier));
        RawExtentNode* newOuter = dynamic_cast<RawExtentNode*>(_outer->copy(copier));
        DocumentStructureNode * newDocStruct = dynamic_cast<DocumentStructureNode*>(_documentStructure->copy(copier));
        ExtentParent* extentInsideCopy = new ExtentParent( newInner, newOuter, newDocStruct );
        extentInsideCopy->setNodeName( nodeName() );

        return copier.after(this, extentInsideCopy);
      }
    }; // end ExtentParent

		//
		// WildcardTerm
		//
    class WildcardTerm : public RawExtentNode {
    private:
			std::string _normalizedTerm;

			void normalizeTerm() {
				// remove the wildcard character
				std::string::size_type wpos=_normalizedTerm.rfind("*");
				if (wpos!=std::string::npos) {
          _normalizedTerm=_normalizedTerm.substr(0, wpos);
				}

				// lowercase the term
				for (size_t i=0; i < _normalizedTerm.size(); ++i) {
					_normalizedTerm[i]=tolower(_normalizedTerm[i]);
				}
			}

    public:
      WildcardTerm() {}
			WildcardTerm( std::string text) :
			_normalizedTerm(text)
      {
        normalizeTerm();
      }

      WildcardTerm( Unpacker& unpacker ) {
        _normalizedTerm = unpacker.getString( "normalizedTerm" );
      } 

      std::string typeName() const {
        return "WildcardTerm";
      }

      std::string queryText() const {
				return (_normalizedTerm + "*");
      }

      UINT64 hashCode() const {
        int accumulator = 7;

        indri::utility::GenericHash<const char*> hash;
        return accumulator + hash( _normalizedTerm.c_str() );
      }

			void setTerm( std::string term ) {
				_normalizedTerm=term;
				normalizeTerm();
      }

			std::string getTerm() {
        return _normalizedTerm;
      }

      bool operator== ( Node& node ) {
        WildcardTerm* other = dynamic_cast<WildcardTerm*>(&node);

        if( other == this )
          return true;

        if( !other )
          return false;

        if (other->getTerm()==_normalizedTerm) 
					return true;

				return false;
      }

      void pack( Packer& packer ) {
        packer.before(this);
        packer.put( "normalizedTerm", _normalizedTerm );
        packer.after(this);
      }

      void walk( Walker& walker ) {
        walker.before(this);
        walker.after(this);
      }

      Node* copy( Copier& copier ) {
        copier.before(this);
        
        WildcardTerm* duplicate = new WildcardTerm(_normalizedTerm);
        duplicate->setNodeName( nodeName() );

        return copier.after(this, duplicate);
      }
    };

    class PlusNode : public UnweightedCombinationNode {
    public:
      PlusNode() {}
      PlusNode( Unpacker& unpacker ) {
        _unpack( unpacker );
      }

      std::string typeName() const {
        return "PlusNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#plus(";
        _childText(qtext);
        qtext << ")";

        return qtext.str();
      } 

      UINT64 hashCode() const {
        return 259 + _hashCode();
      }

      void walk( Walker& walker ) {
        _walk( this, walker );
      }
      
      Node* copy( Copier& copier ) {
        return _copy( this, copier );
      }
    };

    class WPlusNode : public WeightedCombinationNode {
    public:
      WPlusNode() {}
      WPlusNode( Unpacker& unpacker ) {
        _unpack( unpacker );
      }

      std::string typeName() const {
        return "WPlusNode";
      }

      std::string queryText() const {
        std::stringstream qtext;
        qtext << "#wplus(";
        _childText(qtext);
        qtext << ")";

        return qtext.str();
      } 

      UINT64 hashCode() const {
        return 261 + _hashCode();
      }

      void walk( Walker& walker ) {
        _walk( this, walker );
      }
      
      Node* copy( Copier& copier ) {
        return _copy( this, copier );
      }
    };


  }
}

#endif // INDRI_QUERYSPEC_HPP
