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
// ExtentOrNode
//
// 23 February 2004 -- tds
//

#ifndef INDRI_EXTENTORNODE_HPP
#define INDRI_EXTENTORNODE_HPP

#include <vector>
#include <string>
#include "indri/greedy_vector"
#include "indri/ListIteratorNode.hpp"
namespace indri
{
  namespace infnet
  {
    
    class ExtentOrNode : public ListIteratorNode {
    private:
      std::vector<ListIteratorNode*> _children;
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      std::string _name;

    public:
      ExtentOrNode( const std::string& name, std::vector<ListIteratorNode*>& children );
      void prepare( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<indri::index::Extent>& extents();
  
      void indexChanged( indri::index::Index& index );
      lemur::api::DOCID_T nextCandidateDocument();

      const std::string& getName() const;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
    };
  }
}

#endif // INDRI_EXTENTORNODE_HPP

