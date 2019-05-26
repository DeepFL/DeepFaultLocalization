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
// ExtentAndNode
//
// 23 February 2004 -- tds
//

#ifndef INDRI_EXTENTANDNODE_HPP
#define INDRI_EXTENTANDNODE_HPP

#include "indri/ListIteratorNode.hpp"
#include <vector>
#include "indri/greedy_vector"
namespace indri 
{
  namespace infnet
  {
    
    class ExtentAndNode : public ListIteratorNode {
    private:
      std::vector<ListIteratorNode*> _children;
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      indri::utility::greedy_vector<indri::index::Extent> _partialExtents;
      std::string _name;

      void _and( indri::utility::greedy_vector<indri::index::Extent>& out, const indri::utility::greedy_vector<indri::index::Extent>& one, const indri::utility::greedy_vector<indri::index::Extent>& two );

    public:
      ExtentAndNode( const std::string& name, std::vector<ListIteratorNode*>& children );
      void prepare( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<indri::index::Extent>& extents();
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
      const std::string& getName() const;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
    };
  }
}

#endif //INDRI_EXTENTANDNODE_HPP

