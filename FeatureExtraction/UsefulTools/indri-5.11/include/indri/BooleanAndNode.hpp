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
// BooleanAndNode
//
// 16 November 2004 -- tds
//

#ifndef INDRI_BOOLEANANDNODE_HPP
#define INDRI_BOOLEANANDNODE_HPP

#include "indri/ListIteratorNode.hpp"
#include "indri/greedy_vector"
#include "lemur/lemur-platform.h"
#include <vector>
#include <string>
namespace indri
{
  namespace infnet
  {
    
    class BooleanAndNode : public ListIteratorNode {
      std::vector<ListIteratorNode*> _lists;
      std::string _name;
      indri::utility::greedy_vector<indri::index::Extent> _extents;

    public:
      BooleanAndNode( const std::string& name, std::vector<ListIteratorNode*>& children );

      void prepare( lemur::api::DOCID_T documentID );
      indri::utility::greedy_vector<indri::index::Extent>& extents();
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
      const std::string& getName() const;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent);
    };
  }
}

#endif // INDRI_BOOLEANANDNODE_HPP

