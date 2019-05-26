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
// FilterRejectNode
//
// 13 April 2004 -- tds
//

#ifndef INDRI_FILTERREJECTNODE_HPP
#define INDRI_FILTERREJECTNODE_HPP

#include "indri/greedy_vector"
#include "indri/Extent.hpp"
#include "indri/BeliefNode.hpp"
#include "indri/ListIteratorNode.hpp"
namespace indri 
{
  namespace infnet
  {
    
    class FilterRejectNode : public BeliefNode {
    private:
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _extents;
      indri::utility::greedy_vector<bool> _matches;
      ListIteratorNode* _filter;
      BeliefNode* _disallowed;
      std::string _name;

    public:
      FilterRejectNode( const std::string& name, ListIteratorNode* filter, 
                        BeliefNode* disallowed );

      double maximumBackgroundScore();
      double maximumScore();
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      const std::string& getName() const;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
    };
  }
}

#endif // INDRI_FILTERREJECTNODE_HPP

