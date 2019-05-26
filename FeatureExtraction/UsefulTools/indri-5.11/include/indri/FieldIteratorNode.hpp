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
// FieldIteratorNode
//
// 23 February 2004 -- tds
//

#ifndef INDRI_FIELDITERATORNODE_HPP
#define INDRI_FIELDITERATORNODE_HPP

#include "indri/ListIteratorNode.hpp"
#include "indri/DocExtentListIterator.hpp"
namespace indri
{
  namespace infnet
  {
    
    class FieldIteratorNode : public ListIteratorNode {
    private:
      class indri::index::DocExtentListIterator* _list;
      int _listID;
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      indri::utility::greedy_vector<INT64> _numbers;
      class InferenceNetwork& _network;
      std::string _name;

    public:
      FieldIteratorNode( const std::string& name, class InferenceNetwork& network, int fieldID );
      void prepare( lemur::api::DOCID_T documentID );
      /// returns a list of intervals describing positions of children
      const indri::utility::greedy_vector<indri::index::Extent>& extents(); 
      const indri::utility::greedy_vector<INT64>& numbers();
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
      const std::string& getName() const;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
    };
  }
}

#endif // INDRI_FIELDITERATORNODE_HPP

