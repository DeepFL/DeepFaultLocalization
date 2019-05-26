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
// QueryAnnotation
// 
// 21 July 2004 -- tds
//

#include "indri/QueryAnnotation.hpp"
#include "indri/Walker.hpp"
#include <map>
#include <set>
#include <stack>
#include "indri/QuerySpec.hpp"
namespace indri
{
  namespace lang
  {
    class QueryNodeBuilder : public indri::lang::Walker {
    private:
      indri::api::QueryAnnotationNode* _root;
      std::map< indri::lang::Node*, indri::api::QueryAnnotationNode* > _nodes;
      std::stack< indri::api::QueryAnnotationNode* > _stack;

    public:
      QueryNodeBuilder() : _root(0) {}

      void defaultBefore( indri::lang::Node* n ) {
        std::map< indri::lang::Node*, indri::api::QueryAnnotationNode* >::iterator iter = _nodes.find(n);
        indri::api::QueryAnnotationNode* next = 0;

        if( iter != _nodes.end() ) {
          next = iter->second;
        } else {
          next = new indri::api::QueryAnnotationNode;
          next->name = n->nodeName();
          next->queryText = n->queryText();
          next->type = n->typeName();
        }

        if( _stack.size() ) {
          _stack.top()->children.push_back( next );
        }

        _stack.push( next );

        if( !_root ) {
          _root = next;
        }
      }

      void defaultAfter( indri::lang::Node* n ) {
        _stack.pop();
      }

      indri::api::QueryAnnotationNode* getRoot() {
        return _root;
      }
    };
  }
}

  
void delete_query_node( indri::api::QueryAnnotationNode* node, std::set<indri::api::QueryAnnotationNode*>& deleted ) {
  // query tree may be a dag, so we have to be careful
  deleted.insert(node);
  
  for( size_t i=0; i<node->children.size(); i++ ) {
    indri::api::QueryAnnotationNode* child = node->children[i];
    if( deleted.find(child) == deleted.end() )
      delete_query_node(node->children[i], deleted);
  }
}


indri::api::QueryAnnotation::QueryAnnotation() :
  _queryTree(0) {
}

indri::api::QueryAnnotation::~QueryAnnotation() {
  std::set<indri::api::QueryAnnotationNode*> deleted;
  delete_query_node(_queryTree, deleted);
  std::set<indri::api::QueryAnnotationNode*>::iterator item;
  for(item = deleted.begin(); item != deleted.end(); item++) {
    delete *item;
  }
}


indri::api::QueryAnnotation::QueryAnnotation( indri::lang::Node* queryRoot, indri::infnet::EvaluatorNode::MResults& annotations, std::vector<indri::api::ScoredExtentResult>& results ) {
  indri::lang::QueryNodeBuilder builder;
  queryRoot->walk(builder);
  _queryTree = builder.getRoot();
  _annotations = annotations;
  _results = results;
}

const indri::api::QueryAnnotationNode* indri::api::QueryAnnotation::getQueryTree() const {
  return _queryTree;
}

const std::vector<indri::api::ScoredExtentResult>& indri::api::QueryAnnotation::getResults() const {
  return _results;
}

const indri::infnet::EvaluatorNode::MResults& indri::api::QueryAnnotation::getAnnotations() const {
  return _annotations;
}
