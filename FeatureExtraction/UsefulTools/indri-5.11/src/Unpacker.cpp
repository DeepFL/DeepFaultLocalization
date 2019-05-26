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
// Unpacker
//
// 22 March 2004 -- tds
//

#include "indri/Unpacker.hpp"
#include "indri/QuerySpec.hpp"
#include "indri/XMLNode.hpp"
#include "lemur/Exception.hpp"
#include "indri/delete_range.hpp"
namespace indri {
  namespace lang {
    // Note: _unpack is automatically generated in UnpackerInternal.cpp

    Unpacker::Unpacker( indri::xml::XMLNode* root ) :
      _root(root)
    {
    }

    Unpacker::~Unpacker( ) {
      indri::utility::delete_map_contents<std::string, indri::lang::Node>(_nodes);
    }

    std::vector<indri::lang::Node*> Unpacker::unpack() {
      std::vector<indri::lang::Node*> result;

      for( size_t i=0; i<_root->getChildren().size(); i++ ) {
        indri::xml::XMLNode* child = _root->getChildren()[i];
        _current = child;
        Node* node = _unpack(child);
        _nodes[node->nodeName()] = node;

        if( child->getAttribute( "root" ) == "true" ) {
          result.push_back(node);
        } 
      }

      return result;
    }

    ::std::string Unpacker::getString( const char* stringName ) {
      assert( _current );
      return _current->getAttribute( stringName );
    }

    double Unpacker::getDouble( const char* name ) {
      std::string value = _current->getAttribute( name );
      std::stringstream s(value);
      double v;
      s >> v;
      return v;
    }

    UINT64 Unpacker::getInteger( const char* name ) {
      std::string value = _current->getAttribute( name );
      return string_to_i64( value );
    }

    RawExtentNode* Unpacker::getRawExtentNode( const char* name ) {
      std::string internalName = _current->getChildValue(name);
      return (RawExtentNode*) _nodes[internalName];
    }

    ScoredExtentNode* Unpacker::getScoredExtentNode( const char* name ) {
      std::string internalName = _current->getChildValue(name);
      return (ScoredExtentNode*) _nodes[internalName];
    } 

    DocumentStructureNode* Unpacker::getDocumentStructureNode( const char* name ) {
      std::string internalName = _current->getChildValue(name);
      return (DocumentStructureNode*) _nodes[internalName];
    }

    ::std::vector<RawExtentNode*> Unpacker::getRawExtentVector( const char* name ) {
      return getNodeVector<RawExtentNode>(name);
    }

    ::std::vector<ScoredExtentNode*> Unpacker::getScoredExtentVector( const char* name ) {
      return getNodeVector<ScoredExtentNode>(name);
    }

    ::std::vector<int> Unpacker::getIntVector( const char* name ) {
      std::vector<int> result;
      const indri::xml::XMLNode* vector = _current->getChild(name);

      for( size_t i=0; i<vector->getChildren().size(); i++ ) {
        indri::xml::XMLNode* ref = vector->getChildren()[i];
        std::stringstream s( ref->getValue() );
        int value;
        s >> value;
        result.push_back( value );
      }

      return result;
    }

    ::std::vector<lemur::api::DOCID_T> Unpacker::getDocIdVector( const char* name ) {
      std::vector<lemur::api::DOCID_T> result;
      const indri::xml::XMLNode* vector = _current->getChild(name);

      for( size_t i=0; i<vector->getChildren().size(); i++ ) {
        indri::xml::XMLNode* ref = vector->getChildren()[i];
        std::stringstream s( ref->getValue() );
        lemur::api::DOCID_T value;
        s >> value;
        result.push_back( value );
      }

      return result;
    }

    ::std::vector<double> Unpacker::getDoubleVector( const char* name ) {
      std::vector<double> result;
      const indri::xml::XMLNode* vector = _current->getChild(name);

      for( size_t i=0; i<vector->getChildren().size(); i++ ) {
        indri::xml::XMLNode* ref = vector->getChildren()[i];
        std::stringstream s( ref->getValue() );
        double value;
        s >> value;
        result.push_back( value );
      }

      return result;
    }

    std::vector<std::string> Unpacker::getStringVector( const char* name ) {
      std::vector<std::string> result;
      const indri::xml::XMLNode* vector = _current->getChild(name);

      for( size_t i=0; i<vector->getChildren().size(); i++ ) {
        indri::xml::XMLNode* ref = vector->getChildren()[i];
        result.push_back( ref->getValue() );
      }

      return result;
    }

    bool Unpacker::getBoolean( const char* name ) {
      return (getInteger(name) ? true : false );
    }  
  }
}


