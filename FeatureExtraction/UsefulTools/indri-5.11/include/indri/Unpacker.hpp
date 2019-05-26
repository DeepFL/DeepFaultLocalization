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
// 17 March 2004 -- tds
//

#ifndef INDRI_UNPACKER_HPP
#define INDRI_UNPACKER_HPP

#include <string>
#include <map>
#include "indri/XMLNode.hpp"
#include "lemur/IndexTypes.hpp"

namespace indri {
  namespace lang {
    class RawExtentNode;
    class ScoredExtentNode;
    class DocumentStructureNode;

    class Unpacker {
    private:
      indri::xml::XMLNode* _root;
      std::map<std::string, class Node*> _nodes;
      indri::xml::XMLNode* _current;

      Node* _unpack( indri::xml::XMLNode* child );

    public:
      Unpacker( indri::xml::XMLNode* root );
      ~Unpacker( );
      std::vector<Node*> unpack();
      std::string getString( const char* stringName ) ;
      UINT64 getInteger( const char* name );
      double getDouble( const char* name );
      RawExtentNode* getRawExtentNode( const char* name );
      std::vector<RawExtentNode*> getRawExtentVector( const char* name );
      std::vector<ScoredExtentNode*> getScoredExtentVector( const char* name );
      std::vector<std::string> getStringVector( const char* name );
      std::vector<int> getIntVector( const char* name );
      std::vector<lemur::api::DOCID_T> getDocIdVector( const char* name );
      
      std::vector<double> getDoubleVector( const char* name ) ;
      ScoredExtentNode* getScoredExtentNode( const char* name );
      bool getBoolean( const char* name );
      DocumentStructureNode* getDocumentStructureNode( const char* name );

      template<class T>
      std::vector<T*> getNodeVector( const char* name ) {
        std::vector<T*> result;
        const indri::xml::XMLNode* vector = _current->getChild(name);

        for( size_t i=0; i<vector->getChildren().size(); i++ ) {
          indri::xml::XMLNode* ref = vector->getChildren()[i];
          T* node = dynamic_cast<T*>(_nodes[ref->getValue()]);
          result.push_back(node);
        }

        return result;
      }
    };
  }
}
#endif // INDRI_UNPACKER_HPP
