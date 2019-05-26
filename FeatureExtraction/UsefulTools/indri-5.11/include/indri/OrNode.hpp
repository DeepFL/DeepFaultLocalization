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
// OrNode
//
// 31 March 2004 -- tds
//
// Implements the InQuery #or node.
//
// Note that this class transforms the probabilities
// out of log space and back into log space, which
// could cause a (catastrophic) loss of precision.
//

#ifndef INDRI_ORNODE_HPP
#define INDRI_ORNODE_HPP

#include <math.h>
#include "indri/BeliefNode.hpp"
#include <vector>
#include "indri/greedy_vector"
namespace indri
{
  namespace infnet
  {
    
    class OrNode : public BeliefNode {
    private:
      std::vector<BeliefNode*> _children;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _scores;
      indri::utility::greedy_vector<bool> _matches;
      std::string _name;

    public:
      OrNode( const std::string& name );
      OrNode( const std::string& name, const std::vector<BeliefNode*>& children );

      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      double maximumScore();
      double maximumBackgroundScore();
  
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      virtual void setSiblingsFlag(int f);

      const std::string& getName() const;
    };
  }
}

#endif // INDRI_ORNODE_HPP

