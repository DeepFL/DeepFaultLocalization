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
// PriorNode
//
// 27 April 2004 -- tds
//

#ifndef INDRI_PRIORNODE_HPP
#define INDRI_PRIORNODE_HPP

#include "indri/BeliefNode.hpp"
#include "indri/QuerySpec.hpp"
#include "indri/PriorListIterator.hpp"

namespace indri
{
  namespace infnet
  {
    class PriorNode : public BeliefNode {
    private:
      indri::utility::greedy_vector<bool> _matches;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _scores;
      std::string _name;

      class InferenceNetwork& _network;
      int _listID;
      
      indri::collection::PriorListIterator* _iterator;
      
    public:
      PriorNode( const std::string& name,
                 class InferenceNetwork& network,
                 int listID );
      ~PriorNode();

      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      double maximumScore();
      double maximumBackgroundScore();
      const std::string& getName() const;
    };
  }
}

#endif // INDRI_PRIORNODE_HPP

