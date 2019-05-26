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
// NestedExtentInsideNode
//
// 30 Aug 2005 - pto
//
// Behaves the same as a ExtentInsideNode, but has added logic that allows
// for nodes in the inner list to be nested, rather than assuming they will
// be non-overlapping.
//

#ifndef INDRI_NESTEDEXTENTINSIDENODE_HPP
#define INDRI_NESTEDEXTENTINSIDENODE_HPP

#include "indri/ListIteratorNode.hpp"
#include "indri/greedy_vector"
#include "indri/Extent.hpp"
namespace indri
{
  namespace infnet
  {
    
    class NestedExtentInsideNode : public ListIteratorNode {
      ListIteratorNode* _inner;
      ListIteratorNode* _outer;
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      std::string _name;

    public:
      NestedExtentInsideNode( const std::string& name, ListIteratorNode* inner, ListIteratorNode* outer );

      void prepare( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<indri::index::Extent>& extents();
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      const std::string& getName() const;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
    };
  }
}

#endif // INDRI_NESTEDEXTENTISIDENODE_HPP

