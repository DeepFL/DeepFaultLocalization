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
// BeliefNode
// 
// 26 January 2004 -- tds
//

#ifndef INDRI_BELIEFNODE_HPP
#define INDRI_BELIEFNODE_HPP

#include "indri/InferenceNetworkNode.hpp"
#include "indri/greedy_vector"
#include "indri/ScoredExtentResult.hpp"
#include <float.h>

#define INDRI_HUGE_SCORE  ( DBL_MAX )
#define INDRI_TINY_SCORE  ( -DBL_MAX )
namespace indri
{
  namespace infnet
  {
    
    class BeliefNode : public InferenceNetworkNode {
    protected:
      /// flag (and potential counter) for if belief node has siblings
      int bSiblings;

    public:
      virtual double maximumBackgroundScore() = 0;
      virtual double maximumScore() = 0;
      virtual const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) = 0;
      virtual bool hasMatch( lemur::api::DOCID_T documentID ) = 0;
      virtual const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) = 0;
      virtual void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) = 0;

      /// sets the siblings flag (and counter) if the belief node
      /// has siblings
      virtual void setSiblingsFlag(int f) { bSiblings=f; }
    };
  }
}

#endif // INDRI_BELIEFNODE_HPP

