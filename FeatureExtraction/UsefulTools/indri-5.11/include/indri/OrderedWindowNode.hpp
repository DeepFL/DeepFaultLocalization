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
// OrderedWindowNode
//
// 26 January 2004 -- tds
// 

#ifndef INDRI_ORDEREDWINDOWNODE_HPP
#define INDRI_ORDEREDWINDOWNODE_HPP

#include "indri/ListIteratorNode.hpp"
#include <vector>
#include <indri/greedy_vector>
#include <assert.h>

//
// Expression:
//     #od<number>( <word>+ ) | #<number>( <word>+ )
// Examples:
//     #2( word1 word2 )
//     #od2( word1 word2 )
//
// Semantics:
//    Means "find these words, in order, in the text, where no two
//    adjacent words in the query are further apart than <number>-1".
//
// Examples:
//    #2(george bush) matches "george w. bush" but not "george herbert walker bush"
//    #2(george w bush) matches "george this w. that bush" but not "george w. this that bush"
//
namespace indri
{
  namespace infnet
  {
    
    class OrderedWindowNode : public ListIteratorNode {
    private:
      struct extents_pointer {
        indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
        indri::utility::greedy_vector<indri::index::Extent>::const_iterator end;
        indri::utility::greedy_vector<indri::index::Extent>::const_iterator start;
      };
      int _windowSize;
      std::vector<ListIteratorNode*> _children;
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      std::vector<extents_pointer> _pointers;
      std::string _name;

      bool findNextPossibleOccurrence(int i, bool& fail);
      
    public:
      OrderedWindowNode( const std::string& name, const std::vector<ListIteratorNode*>& children );
      OrderedWindowNode( const std::string& name, const std::vector<ListIteratorNode*>& children, int windowSize );

      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      void prepare( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<indri::index::Extent>& extents();
      const std::string& getName() const ;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
    };
  }
}

#endif // INDRI_ORDEREDWINDOWNODE_HPP

