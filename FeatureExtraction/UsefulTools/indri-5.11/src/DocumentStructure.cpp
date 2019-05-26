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

#include "indri/DocumentStructure.hpp"
#include "indri/Index.hpp"

void
indri::index::DocumentStructure::setIndex( indri::index::Index & index ) {
  _index = &index;
}

void
indri::index::DocumentStructure::loadStructure(const indri::utility::greedy_vector<indri::index::FieldExtent> & fields) {

  _numNodes = fields.size();

  _nodes.resize( _numNodes + 1 );
  _childrenBuff.resize( _numNodes );

  // set up special place holder for root list
  _nodes[0].begin = 0;
  _nodes[0].end = 0;
  _nodes[0].type = 0;
  _nodes[0].length = 0;
  _nodes[0].totalLength = 0;
  _nodes[0].numChildren = 0;
  _nodes[0].parent = -1;
  _nodes[0].children = 0;
  _nodes[0].number=0;

  // no fields? no data.
  if (_numNodes==0) return;


  int i;

  // copy values from field list,
  for (i = 0; i < _numNodes; i++) {

    int type = fields[i].id;
    int begin = fields[i].begin;
    int end = fields[i].end;
    int length =  end - begin;

    // Node ids are from 1 to _numNodes
    int id = fields[i].ordinal;   
    int parent = fields[i].parentOrdinal;

    UINT64 number=fields[i].number;

    //    assert( id == i + 1 );

    _nodes[id].begin = begin;
    _nodes[id].end = end;
    _nodes[id].id = id;
    _nodes[id].type = type;
    _nodes[id].length = length;
    _nodes[id].totalLength = length;
    _nodes[id].numChildren = 0;
    _nodes[id].number=number;
    _nodes[id].parent = parent;
    _nodes[id].children = 0;

  }
  // count the children for each node
  for (i = 1; i <= _numNodes; i++) {
    int parent = _nodes[i].parent;
    // count number of children (and number of tree roots)
    if (parent >= 0) {
      _nodes[parent].numChildren++;
    }
  }

  // Divy up the children buffer array, and set the children.
  // This also assumes that the parent ids are always smaller than
  // their childrens' ids.
  int cbuffLoc = 0;
  for (i = 0; i <= _numNodes; i++ ) {
    if (_nodes[i].numChildren == 0) {
      _nodes[i].children = -1;
    } else {
      _nodes[i].children = cbuffLoc;
      cbuffLoc += _nodes[i].numChildren;
      // reset this counter as we will use it when building the children array
      _nodes[i].numChildren = 0;
    }
  }
  for (i = 1; i <= _numNodes; i++ ) {
    if ( _nodes[i].parent >= 0 ) {
      int p = _nodes[i].parent;
      _childrenBuff[ _nodes[p].children + _nodes[p].numChildren ] = _nodes[i].id;
      _nodes[p].numChildren++;
    }
  }

  // subtract away the children total lengths to get the node length
  for (i = 1; i <= _numNodes; i++ ) {
    // int * child = node->children;
    // int * childrenEnd = child + node->numChildren;
    child_iterator child = childrenBegin( i );
    child_iterator childEnd = childrenEnd( i );
    while (child < childEnd) {
      _nodes[i].length -= _nodes[(*child)].totalLength;
      child++;
    }
  }

  _topDownOrder.clear();
}

indri::index::DocumentStructure::~DocumentStructure() {
}

int 
indri::index::DocumentStructure::parent(int node) {
  if (node > 0 && node <= _numNodes) {
    return _nodes[node].parent;
  }
  return 0;
}


int 
indri::index::DocumentStructure::length(int node) {
  if (node > 0 && node <= _numNodes) {
    return _nodes[node].length;
  }
  return 0;
}


int 
indri::index::DocumentStructure::begin(int node) {
  if (node > 0 && node <= _numNodes) {
    return _nodes[node].begin;
  }
  return 0;
}

int 
indri::index::DocumentStructure::end(int node) {
  if (node > 0 && node <= _numNodes) {
    return _nodes[node].end;
  }
  return 0;
}


int 
indri::index::DocumentStructure::accumulatedLength(int node) {
  if (node > 0 && node <= _numNodes) {
    return _nodes[node].totalLength;
  }
  return 0;
}


int
indri::index::DocumentStructure::type(int node) {
  if (node > 0 && node <= _numNodes) {
    return _nodes[node].type;
  }
  return 0;
}

UINT64 
indri::index::DocumentStructure::number(int node) {
  if (node > 0 && node <= _numNodes) {
    return _nodes[node].number;
  }
  return 0;
}

indri::index::DocumentStructure::child_iterator
indri::index::DocumentStructure::childrenBegin( int node ) {
  if ( node >= 0 && node <= _numNodes ) {
    return _childrenBuff.begin() + _nodes[node].children;
  } 
  return _childrenBuff.end();
}

indri::index::DocumentStructure::child_iterator
indri::index::DocumentStructure::childrenEnd( int node ) {
  if ( node >= 0 && node <= _numNodes ) {
    return _childrenBuff.begin() + _nodes[node].children + _nodes[node].numChildren;
  } 
  return _childrenBuff.end();
}

bool 
indri::index::DocumentStructure::ancestor(int node, int possibleAncestor) {
  bool anc = false;
  if (node == possibleAncestor) {
    anc = true;
  } else if (node == 0) {
    anc = false;
  } else {    
    anc = ancestor(parent(node), possibleAncestor);
  }
  return anc;
}

bool 
indri::index::DocumentStructure::_findLeafs(int node, int b, int e, bool exact, std::set<int> * leafs ) {
  bool foundDescendant = false;
  child_iterator child = childrenBegin( node );
  child_iterator childEnd = childrenEnd( node );
  while ( child != childEnd ) {
    if ( _findLeafs( *child, b, e, exact, leafs ) ) {
      foundDescendant = true;
    }
    child++;
  }
//   if ( foundDescendant == false ) {    
    if ( (exact && begin( node ) == b && end( node ) == e ) ||
         (!exact && begin( node ) >= b && end( node ) <= e )) {
      leafs->insert( node );
      foundDescendant = true;
    }
//   }
  return foundDescendant;
}

std::set<int> *
indri::index::DocumentStructure::findLeafs(int b, int e, bool exact ) {
  std::set<int> * leafs = new std::set<int>;
  findLeafs(leafs, b, e, exact);
  return leafs;
}

void 
indri::index::DocumentStructure::findLeafs(std::set<int> * leafs, int b, int e, bool exact ) {
//   child_iterator root = childrenBegin( 0 );
//   child_iterator rootsEnd = childrenEnd( 0 );
//   while ( root != rootsEnd ) {
//     _findLeafs( *root, b, e, exact, leafs );
//     root++;
//   }
  for (int node = 1; node <= _numNodes; node++) {
    int nodeBegin = begin( node );
    int nodeEnd = end( node );
    if ( (exact && nodeBegin == b && nodeEnd == e ) ||
         (!exact && nodeBegin >= b && nodeEnd <= e ) ) {
      leafs->insert( node );
    }
    if ( nodeBegin > e ) {
      break;
    }
  }
}

int
indri::index::DocumentStructure::findLeaf(int b, int e, int root) {

  if ( root == -1 ) {
    root = *(childrenBegin( 0 ));
  }

  int leaf = 0;
  // start at root of tree and work way down to leaf
  if ( begin(root) <= b && end(root) >= e ) {
    leaf = root;
  } 
  child_iterator child = childrenBegin( root );
  child_iterator childEnd = childrenEnd( root );
  while (child < childEnd) {

    if ( begin(*child) <= b && end(*child) >= e ) {
      leaf = *child;
      child = childrenBegin( leaf );
      childEnd = childrenEnd( leaf );
      
    } else {
      child++;
    }
  }
  
  return leaf;
}

std::string
indri::index::DocumentStructure::path(int node) {
  std::stringstream path;
  
  _constructNodePath(path, node);

  return path.str();
}

void
indri::index::DocumentStructure::_constructNodePath(std::stringstream & path, int node) {

  int pid = parent(node);
  if ( pid > 0) {
    _constructNodePath(path, pid);
  }
  
  child_iterator kids = childrenBegin( pid );
  child_iterator kidsEnd = childrenEnd( pid );

  int nodeType = type(node);    
  int sameType = 0;
  int sameTypeLoc = 0;
  
  while ( kids < kidsEnd ) {
    int sid = *kids;
    if (type(sid) == nodeType) {
      sameType++;
      if (sid == node) {
        sameTypeLoc = sameType;
      }
    }
    kids++;
  }

  path << "/" << _index->field( nodeType );
  
  path << "[" << sameTypeLoc << "]";
  
  
}

int
indri::index::DocumentStructure::fieldId( const std::string path ) {
  size_t loc = 0;
  int node = 0;

  child_iterator kids = childrenBegin( node );
  child_iterator kidsEnd = childrenEnd( node );

  int seen = 0;
  
  while ( loc != std::string::npos && loc < path.size() ) {
  
    // seek to first path name
    int typeBegin = path.find_first_not_of( "/", loc );
    // find end of path name
    int typeEnd = path.find_first_of( "[" , typeBegin );
    // extract out the path name
    int typeLength = typeEnd - typeBegin;
    const std::string typeString = path.substr( typeBegin, typeLength );
    int typeId = _index->field( typeString );
    
    // extract count
    int countBegin = path.find_first_not_of( "[", typeEnd );
    int countEnd = path.find_first_of( "]" , typeBegin );
    int countLength = countEnd - countBegin;
    const std::string countString = path.substr( countBegin, countLength );
    int count = 0;
    sscanf( countString.c_str(), "%d", &count );
  
    // find the node
    while( kids < kidsEnd ) {
      if( type( *kids ) == typeId ) {
        seen++;
      }
      if ( count == seen ) {
        node = *kids;
        break;
      }
      kids++;
    }
    
  
    if( count != seen ) {
      return 0;
    } 

    seen = 0;    
    
    // set the children
    kids = childrenBegin( node );
    kidsEnd = childrenEnd( node );
    
    loc = path.find_first_of( "/", countEnd );
  }
  return node;

}

indri::utility::greedy_vector<int> &
indri::index::DocumentStructure::topDownOrder( ) {
  if ( _topDownOrder.size() == 0 ) {
    _computeTopDownOrder();
  }
  return  _topDownOrder;
}

void
indri::index::DocumentStructure::_computeTopDownOrder() {
  _topDownOrder.clear();

  //  std::stringstream order;
  //  order << "ORDER:";
  // seed with roots
  child_iterator begin = childrenBegin(0);
  child_iterator end = childrenEnd(0);
  while ( begin != end ) {
    _topDownOrder.push_back( *begin );
    //    order << " " << *begin;
    begin++;
  }
  
  for (int i = 0; i < _numNodes; i++) {
    // take each node and add its children
    begin = childrenBegin( _topDownOrder[i] );
    end  = childrenEnd( _topDownOrder[i] ); 
    while ( begin != end ) {
      _topDownOrder.push_back( *begin );
      //      order << " " << *begin;
      begin++;
    }   
  }
  //  std::cout << order.str() << std::endl;

  assert(_topDownOrder.size() == _numNodes);
}


void
indri::index::DocumentStructure::topDownOrder(std::set<int> & roots, indri::utility::greedy_vector<int> & order) {
  order.clear();
  
//   std::stringstream orderStr;
//   orderStr << "ORDER:";
  // seed with roots
  std::set<int>::iterator r = roots.begin();
  while ( r != roots.end() ) {
    order.push_back( *r );
//     orderStr << " " << *r;
    r++;
  }
  
  for (size_t i = 0; i < order.size() && int(i) < _numNodes; i++) {
    // take each node and add its children
    child_iterator begin = childrenBegin( order[i] );
    child_iterator end  = childrenEnd( order[i] ); 
    while ( begin != end ) {
      order.push_back( *begin );
//       orderStr << " " << *begin;
      begin++;
    }   
  }
//   std::cout << orderStr.str() << std::endl;
}
