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
// UnorderedWindowNode
//
// 26 January 2004 -- tds
// 11 December 2007 -- mjh - added child sort
//

#ifndef INDRI_UNORDEREDWINDOWNODE_HPP
#define INDRI_UNORDEREDWINDOWNODE_HPP

#include "indri/ListIteratorNode.hpp"
#include <vector>
#include <indri/greedy_vector>
namespace indri
{
  namespace infnet
  {

    // comparison operation used in child sorting
    // for ascending order by # of extents in each child
    inline bool UWNodeChildLess(indri::infnet::ListIteratorNode* _child1, indri::infnet::ListIteratorNode* _child2) {
      const indri::utility::greedy_vector<indri::index::Extent>& childPositions1 = _child1->extents();
      const indri::utility::greedy_vector<indri::index::Extent>& childPositions2 = _child2->extents();
      return (childPositions1.size() < childPositions2.size());
    }

    class UnorderedWindowNode : public ListIteratorNode {
    private:
      struct term_position {
        bool operator< ( const term_position& other ) const {
          return begin < other.begin;
        }

        double weight;
        int type;
        int begin;
        int end;
        int last; // index of previous entry of this type
      };

      int _windowSize;
      std::vector<ListIteratorNode*> _children;
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      std::string _name;
      bool _childrenAlreadySorted;

    public:
      UnorderedWindowNode( const std::string& name, std::vector<ListIteratorNode*>& children );
      UnorderedWindowNode( const std::string& name, std::vector<ListIteratorNode*>& children, int windowSize );
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
      void prepare( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<indri::index::Extent>& extents();
      const std::string& getName() const;
      void annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
    };
  }
}

#endif // INDRI_UNORDEREDWINDOWNODE_HPP
