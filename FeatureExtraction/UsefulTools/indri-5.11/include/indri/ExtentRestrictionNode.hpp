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
// ExtentRestrictionNode
//
// 6 July 2004 -- tds
//

#ifndef INDRI_EXTENTRESTRICTIONNODE_HPP
#define INDRI_EXTENTRESTRICTIONNODE_HPP

#include "indri/BeliefNode.hpp"
#include "indri/FieldIteratorNode.hpp"
#include "indri/greedy_vector"
#include "indri/Extent.hpp"
namespace indri
{
  namespace infnet
  {
    
    class ExtentRestrictionNode : public BeliefNode {
    private:
      BeliefNode* _child;
      ListIteratorNode* _field;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _scores;
      indri::utility::greedy_vector<bool> _matches;
      std::string _name;

    public:
      ExtentRestrictionNode( const std::string& name, BeliefNode* child, ListIteratorNode* field );

      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      double maximumScore();
      double maximumBackgroundScore();

      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent);
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      const std::string& getName() const;

      /// over-ridden to set child nodes too
      void setSiblingsFlag(int f);
    };
  }
}

#endif // INDRI_EXTENTRESTRICTIONNODE_HPP
