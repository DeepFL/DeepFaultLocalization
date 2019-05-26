/*==========================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software (and below), and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// ExtentEnforcementNode
//
// 31 Aug 2005 -- pto
//

// Behaves like extent restriction, but preserves the child's extents
// rather than using the field extents.  It merely enforces that
// the child extents be contained within the field extents.

#ifndef INDRI_EXTENTENFORCEMENTNODE_HPP
#define INDRI_EXTENTENFORCEMENTNODE_HPP

#include "indri/ExtentRestrictionNode.hpp"

namespace indri
{
  namespace infnet
  {
    
    class ExtentEnforcementNode : public BeliefNode {
    private:
      BeliefNode* _child;
      ListIteratorNode* _field;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _scores;
      indri::utility::greedy_vector<bool> _matches;
      std::string _name;

    public:
      ExtentEnforcementNode( const std::string& name, BeliefNode* child, ListIteratorNode* field );

      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      double maximumScore();
      double maximumBackgroundScore();

      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      const std::string& getName() const;

      /// sets the siblings flag (and counter) if the belief node
      /// has siblings
      void setSiblingsFlag(int f);
    };
  }
}

#endif // INDRI_EXTENTENFORCEMENTNODE_HPP
