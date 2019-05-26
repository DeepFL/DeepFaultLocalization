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
// Packer
//
// 22 March 2004 -- tds
//

#include "indri/Packer.hpp"
#include "indri/QuerySpec.hpp"
#include <sstream>
#include "indri/XMLWriter.hpp"
#include "indri/delete_range.hpp"

namespace indri {
  namespace lang {
    Packer::Packer() {
      _packedNodes = new indri::xml::XMLNode( "query" );
    }

    Packer::~Packer() {
      delete _packedNodes;
      indri::utility::delete_map_contents( _elements );
    }

    Packer::node_element* Packer::_getElement( class indri::lang::Node* node ) {
      if( _elements.find( node ) == _elements.end() ) {
        _elements[node] = new node_element( node );
      }

      return _elements[node];
    }

    indri::xml::XMLNode* Packer::_getNodeReference( class indri::lang::Node* node, const ::std::string& name ) {
      if( !node )
        return 0;

      indri::xml::XMLNode* reference = new indri::xml::XMLNode( name, node->nodeName() );

      if( _getElement(node)->flushed == false ) {
        node->pack(*this);
      }

      return reference;
    }

    void Packer::pack( class indri::lang::Node* root ) {
      node_element* element = _getElement( root );
      element->xmlNode->addAttribute( "root", "true" );
      root->pack(*this);
    }

    void Packer::pack( std::vector<indri::lang::Node*>& roots ) {
      for( size_t i=0; i<roots.size(); i++ ) {
        pack( roots[i] );
      }
    }

    void Packer::before( class indri::lang::Node* someNode ) {
      node_element* element = _getElement(someNode);
      _stack.push( element );
    }

    void Packer::after( class indri::lang::Node* someNode ) {
      node_element* element = _stack.top();

      assert( element->languageNode == someNode );
      if( !element->flushed ) {
        element->xmlNode->addAttribute( "name", someNode->nodeName() );
        element->xmlNode->addAttribute( "type", someNode->typeName() );
        // we don't send the query text across the wire, because it might have
        // some characters in it that would mess up the XML serialization
        _packedNodes->addChild( element->xmlNode );
        element->flushed = true;
      }

      _stack.pop();
    }

    void Packer::put( const char* name, double value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      std::stringstream valueToString;
      valueToString << value;

      if( !element->flushed )
        element->xmlNode->addAttribute( name, valueToString.str() );
    }

    void Packer::put( const char* name, bool value ) {
      put( name, UINT64(value) );
    }

    void Packer::put( const char* name, int value ) {
      put( name, UINT64(value) );
    }

    void Packer::put( const char* name, unsigned int value ) {
      put( name, UINT64(value) );
    }

    void Packer::put( const char* name, UINT64 value ) {
      put( name, INT64(value) );
    }

    void Packer::put( const char* name, INT64 value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      std::stringstream valueToString;
      valueToString << value;

      if( !element->flushed )
        element->xmlNode->addAttribute( name, valueToString.str() );
    }

    void Packer::put( const char* name, const ::std::string& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed )
        element->xmlNode->addAttribute( name, value );
    }

    void Packer::put( const char* name, const ::std::vector<lemur::api::DOCID_T>& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) { 
        indri::xml::XMLNode* node = new indri::xml::XMLNode( name );

        for( size_t i=0; i<value.size(); i++ ) {
          ::std::stringstream intToString;
          intToString << value[i];

          node->addChild( new indri::xml::XMLNode( "DOCID_T", intToString.str() ) );
        }

        element->xmlNode->addChild( node );
      }
    }

    void Packer::put( const char* name, const ::std::vector<double>& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) { 
        indri::xml::XMLNode* node = new indri::xml::XMLNode( name );

        for( size_t i=0; i<value.size(); i++ ) {
          ::std::stringstream doubleToString;
          doubleToString << value[i];

          node->addChild( new indri::xml::XMLNode( "double", doubleToString.str() ) );
        }

        element->xmlNode->addChild( node );
      }
    }

    void Packer::put( const char* name, const ::std::vector<std::string>& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) { 
        indri::xml::XMLNode* node = new indri::xml::XMLNode( name );

        for( size_t i=0; i<value.size(); i++ ) {
          node->addChild( new indri::xml::XMLNode( "string", value[i] ) );
        }

        element->xmlNode->addChild( node );
      }
    }

    void Packer::put( const char* name, const ::std::vector<RawExtentNode*>& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) {
        indri::xml::XMLNode* node = new indri::xml::XMLNode( name );

        for( size_t i=0; i<value.size(); i++ ) {
          indri::xml::XMLNode* child = _getNodeReference( value[i], "noderef" );
          node->addChild(child);
        }

        element->xmlNode->addChild( node );
      }
    }

    void Packer::put( const char* name, const ::std::vector<ScoredExtentNode*>& value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) {
        indri::xml::XMLNode* node = new indri::xml::XMLNode( name );

        for( size_t i=0; i<value.size(); i++ ) {
          indri::xml::XMLNode* child = _getNodeReference( value[i], "noderef" );
          node->addChild(child);
        }

        element->xmlNode->addChild( node );
      }
    }

    void Packer::put( const char* name, Node* value ) {
      assert( _stack.size() );
      node_element* element = _stack.top();

      if( !element->flushed ) {
        indri::xml::XMLNode* node = _getNodeReference( value, name );
        if( node )
          element->xmlNode->addChild( node );
      }
    }

    std::string Packer::toString() {
      std::string output;
      indri::xml::XMLWriter writer( _packedNodes );
      writer.write( output );
      return output;
    }

    indri::xml::XMLNode* Packer::xml() {
      return _packedNodes;
    }
  }
}
