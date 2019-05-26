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
// NullListNode.hpp
//
// 11 August 2004 -- tds
//

#ifndef INDRI_NULLLISTNODE_HPP
#define INDRI_NULLLISTNODE_HPP

#include "indri/ListIteratorNode.hpp"
namespace indri
{
  namespace infnet
  {
    
    class NullListNode : public ListIteratorNode {
    private:
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      std::string _name;
      bool _stopword;

    public:
      NullListNode( const std::string& name, bool stopword );

      bool isStopword() const;
      const std::string& getName() const;

      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      void prepare( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<indri::index::Extent>& extents();
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
    };
  }
}

#endif // INDRI_NULLLISTNODE_HPP

