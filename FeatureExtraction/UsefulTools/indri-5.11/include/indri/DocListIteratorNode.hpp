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
// DocListIteratorNode
//
// 26 January 2004 -- tds
//

#ifndef INDRI_DOCLISTITERATORNODE_HPP
#define INDRI_DOCLISTITERATORNODE_HPP

#include "indri/DocListIterator.hpp"
#include "indri/ListIteratorNode.hpp"
#include "indri/Extent.hpp"
#include "indri/greedy_vector"
#include <string>
namespace indri
{
  namespace infnet
  {
    
    class DocListIteratorNode : public ListIteratorNode {
    private:
      indri::index::DocListIterator* _list;
      class InferenceNetwork& _network;
      int _listID;
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      std::string _name;

    public:
      DocListIteratorNode( const std::string& name, class InferenceNetwork& network, int listID );

      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
      void indexChanged( indri::index::Index& index, class InferenceNetwork *network );

      void prepare( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<indri::index::Extent>& extents();
      const std::string& getName() const;  void annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
    };
  }
}

#endif // INDRI_DOCLISTITERATORNODE_HPP
