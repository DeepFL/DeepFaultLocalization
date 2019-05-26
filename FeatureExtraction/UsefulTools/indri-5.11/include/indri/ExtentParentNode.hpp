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

//
// ExtentParentNode
//
// 31 Jan 2006 - pto
//
//
// Checks to make sure the inner node has an extent that matches the extent
// of a parent of nodes whose extents matching the outer extent
//

#ifndef INDRI_EXTENTPARENTNODE_HPP
#define INDRI_EXTENTPARENTNODE_HPP

#include "indri/ListIteratorNode.hpp"
#include "indri/DocumentStructureHolderNode.hpp"
#include "indri/greedy_vector"
#include "indri/Extent.hpp"
namespace indri
{
  namespace infnet
  {
    
    class ExtentParentNode : public ListIteratorNode {
      ListIteratorNode* _inner;
      ListIteratorNode* _outer;
      DocumentStructureHolderNode & _docStructHolder;
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      std::string _name;


      std::set<int> _ancestors;
      std::set<int> _leafs;


    public:
      ExtentParentNode( const std::string& name, 
                        ListIteratorNode* inner, 
                        ListIteratorNode* outer,
                        DocumentStructureHolderNode & documentStructureHolderNode  );

      void prepare( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<indri::index::Extent>& extents();
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      const std::string& getName() const;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );

      const indri::utility::greedy_vector<indri::index::Extent>& matches( indri::index::Extent &extent );
    };
  }
}

#endif // INDRI_EXTENTPARENTNODE_HPP

