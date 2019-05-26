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
// 17 March 2004 -- tds
//

#ifndef INDRI_PACKER_HPP
#define INDRI_PACKER_HPP

#include "indri/XMLNode.hpp"
#include "lemur/IndexTypes.hpp"
#include <stack>
#include <vector>
#include <string>

namespace indri {
  namespace lang {
    class RawExtentNode;
    class ScoredExtentNode;

    class Packer {
    private:
      struct node_element {
        // be warned--there is no defined copy constructor
        // here and no destructor; memory management must be
        // done by the user
        node_element() : languageNode(0), xmlNode(0), flushed(false) {}
        node_element( class Node* langNode ) :
          languageNode( langNode ),
          xmlNode( new indri::xml::XMLNode( "node" ) ),
          flushed(false)
        {
        }

        class Node* languageNode;
        indri::xml::XMLNode* xmlNode;
        bool flushed;
      };

      std::map<class Node*, node_element*> _elements;
      std::stack<node_element*> _stack;
      indri::xml::XMLNode* _packedNodes;

      node_element* _getElement( class Node* node );
      indri::xml::XMLNode* _getNodeReference( class Node* node, const std::string& name );

    public:
      Packer();
      ~Packer();

      void before( class Node* someNode );
      void after( class Node* someNode );
      void put( const char* name, bool value );
      void put( const char* name, int value );
      void put( const char* name, unsigned int value );
      void put( const char* name, UINT64 value );
      void put( const char* name, INT64 value );
      void put( const char* name, double value );
      void put( const char* name, const std::string& value );
      void put( const char* name, const std::vector<lemur::api::DOCID_T>& value );
      void put( const char* name, const std::vector<double>& value );
      void put( const char* name, const std::vector<std::string>& value );
      void put( const char* name, const std::vector<RawExtentNode*>& value );
      void put( const char* name, const std::vector<ScoredExtentNode*>& value );
      void put( const char* name, Node* value );
      void pack( class indri::lang::Node* root );
      void pack( std::vector<class indri::lang::Node*>& roots );
      std::string toString();
      indri::xml::XMLNode* xml();
    };
  }
}

#endif // INDRI_PACKER_HPP
