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
// FilterRequireNode
//
// 13 April 2004 -- tds
//

#ifndef INDRI_FILTERREQUIRENODE_HPP
#define INDRI_FILTERREQUIRENODE_HPP

#include "indri/BeliefNode.hpp"
#include "indri/ListIteratorNode.hpp"
#include "indri/Extent.hpp"
namespace indri
{
  namespace infnet
  {
    
    class FilterRequireNode : public BeliefNode {
    private:
      ListIteratorNode* _filter;
      BeliefNode* _required;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _extents;
      indri::utility::greedy_vector<bool> _matches;
      std::string _name;

    public:
      FilterRequireNode( const std::string& name, ListIteratorNode* filter, 
                         BeliefNode* required );

      double maximumBackgroundScore();
      double maximumScore();
      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );

      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      const std::string& getName() const;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );

      virtual void setSiblingsFlag(int f){
        bSiblings=f; // need to set the flag for the current node itself.
        if (_required) {  _required->setSiblingsFlag(f); }
      }

    };
  }
}

#endif // INDRI_FILTERREQUIRENODE_HPP
