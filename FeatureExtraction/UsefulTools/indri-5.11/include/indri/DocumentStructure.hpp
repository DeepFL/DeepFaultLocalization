/*==========================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software (and below), and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/

#ifndef INDRI_DOCUMENTSTRUCTURE_HPP
#define INDRI_DOCUMENTSTRUCTURE_HPP

#include "indri/FieldExtent.hpp"
#include "indri/greedy_vector"
#include <sstream>
#include <vector>
#include <set>

typedef struct ninf {
  int id;
  int parent;
  int type;

  int begin;
  int end;

  UINT64 number;
  
  int length;
  int totalLength;

  int numChildren;
  int children;

} nodeinfo_t;


namespace indri {
  namespace index {
    
    class Index;

class DocumentStructure {

public:

  DocumentStructure(Index & index, const indri::utility::greedy_vector<indri::index::FieldExtent> & fields) 
    : _index( &index ) ,
      _numNodes( 0 ),
      _nodes( ),
      _childrenBuff( )
  {     
    loadStructure(fields);
  }

  DocumentStructure(Index & index) :
    _index( &index ) ,
    _numNodes( 0 ),
    _nodes( ),
    _childrenBuff( )
  { 
  }

  DocumentStructure() :
    _index( 0 ) ,
    _numNodes( 0 ),
    _nodes( ),
    _childrenBuff( )
  { 
  }

  void loadStructure(const indri::utility::greedy_vector<indri::index::FieldExtent> & fields);
  void setIndex( indri::index::Index & index );
  indri::index::Index * getIndex() { return _index; }

  ~DocumentStructure();

  int parent(int node);
  int length(int node);
  int begin(int node);
  int end(int node);
  int accumulatedLength(int node);
  int type(int node);
  UINT64 number(int node);

  typedef indri::utility::greedy_vector<int>::iterator child_iterator;
  // Gets the start of a list containing children of a node.  childrenBegin( 0 )
  // returns the start of a list containing the roots of trees in the document.
  child_iterator childrenBegin( int node );
  // Gets the end of a list containing children of a node. childrenEnd( 0 )
  // returns the end of the list containing the roots of trees in the document.
  child_iterator childrenEnd( int node );
  

  int nodeCount() { return _numNodes; }

  // Finds a the smallest leaf in a subtree that contains the extent.
  // By default searches the tree rooted by node 1 (BROKEN)
  int findLeaf(int begin, int end, int root = -1);

  // Finds the deepest nodess that contain the extent.
  // If exact is true, then the leaf must exactly match (rather than contain) the extent.
  // Caller is responsible for deleting the returned set.
  std::set<int> * findLeafs(int begin, int end, bool exact = false);
  void findLeafs(std::set<int> * leafs, int begin, int end, bool exact = false);

  // returns true if node == possibleAncestor or
  // if possibleAncestor is a parent or parent's parent ... of node
  bool ancestor(int node, int possibleAncestor);

  std::string path(int node);
  // returns the ordinal matching a string, 0 if unable to find the ordinal matching the xpath
  int fieldId( const std::string path );

  // builds an ordering of nodes that allows walking the trees in a breadth first fashion
  indri::utility::greedy_vector<int> & topDownOrder();
  void topDownOrder(std::set<int> & roots, indri::utility::greedy_vector<int> & order);
  
protected:

  int _numNodes;

  indri::utility::greedy_vector<nodeinfo_t> _nodes;
  indri::utility::greedy_vector<int> _childrenBuff;
  
  Index * _index;
  void _constructNodePath(std::stringstream & path, int node);
  
  indri::utility::greedy_vector<int> _topDownOrder;
  // builds a breadth first walk and stores it in  _topDownOrder
  void _computeTopDownOrder();

  // push leafs matching the extent rooted at node into the set. 
  // will not insert a node if a descedant matches
  bool _findLeafs(int node, int begin, int end, bool exact, std::set<int> * leafs);

};

  }
}
#endif

