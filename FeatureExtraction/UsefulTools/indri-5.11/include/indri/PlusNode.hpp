/*==========================================================================
 * Copyright (c) 2009 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// PlusNode
//
// 6 August 2009 - dmf
//

#ifndef INDRI_PLUSNODE_HPP
#define INDRI_PLUSNODE_HPP

#include <vector>
#include "indri/BeliefNode.hpp"
#include "indri/ScoredExtentResult.hpp"
#include <math.h>
namespace indri
{
  namespace infnet
  {
    
    class PlusNode : public BeliefNode {
    private:
      std::vector<BeliefNode*> _children;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _scores;
      indri::utility::greedy_vector<bool> _matches;
      std::string _name;

    public:
      PlusNode( const std::string& name, const std::vector<BeliefNode*>& children  ) : _name(name), _children(children) {}

      // InferenceNetworkNode interface
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
      double maximumScore();
      double maximumBackgroundScore();
      indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      const std::string& getName() const;
    };
  }
}

#endif // INDRI_PLUSNODE_HPP

