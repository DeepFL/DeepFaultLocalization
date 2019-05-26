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
// ExtentInsideNode
//
// 23 February 2004 -- tds
//
// The difference between an include node and an
// and node is that in the include node, the inner
// extent must be contained completely within an
// outer extent.
//

#ifndef INDRI_EXTENTINSIDENODE_HPP
#define INDRI_EXTENTINSIDENODE_HPP

#include "indri/ListIteratorNode.hpp"
#include "indri/greedy_vector"
#include "indri/Extent.hpp"
namespace indri
{
  namespace infnet
  {
    
    class ExtentInsideNode : public ListIteratorNode {
      ListIteratorNode* _inner;
      ListIteratorNode* _outer;
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      std::string _name;

    public:
      ExtentInsideNode( const std::string& name, ListIteratorNode* inner, ListIteratorNode* outer );

      void prepare( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<indri::index::Extent>& extents();
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      const std::string& getName() const;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
    };
  }
}

#endif // INDRI_EXTENTISIDENODE_HPP

