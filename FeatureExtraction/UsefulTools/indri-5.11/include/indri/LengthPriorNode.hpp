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
// LengthPriorNode
// 
// 9 July 2005 -- pto
//

// Implements an extent length based prior of the form  P(extent) proportional to (end - begin) ^ exponent.
// In terms of actual ranking, exponent * log ( end - begin ) is computed, where INDRI_TINY_SCORE is 
// used for extents of length 0.

#ifndef INDRI_LENGTHPRIORNODE_HPP
#define INDRI_LENGTHPRIORNODE_HPP

#include "indri/BeliefNode.hpp"

namespace indri
{
  namespace infnet
  {
    
    class LengthPriorNode : public BeliefNode {
    private:
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _scores;
      BeliefNode * _child;
      std::string _name;
      double _exponent;

    public:
      LengthPriorNode( const std::string& name, BeliefNode * child, double exponent );
      ~LengthPriorNode();

      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      double maximumScore();
      double maximumBackgroundScore();
      const std::string& getName() const;

      virtual void setSiblingsFlag(int f){
        bSiblings=f; // need to set the flag for the current node itself.
        if (_child) {  _child->setSiblingsFlag(f); }
      }

    };
  }
}

#endif // INDRI_LENGTHPRIORNODE_HPP

