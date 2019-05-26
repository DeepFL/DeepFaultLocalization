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
// TermFrequencyBeliefNode
//
// 25 August 2004 -- tds
//

#ifndef INDRI_TERMFREQUENCYBELIEFNODE_HPP
#define INDRI_TERMFREQUENCYBELIEFNODE_HPP

#include "indri/Annotator.hpp"
#include "indri/ScoredExtentResult.hpp"
#include <string>
#include "indri/TermScoreFunction.hpp"
#include "indri/ListBeliefNode.hpp"
#include "indri/DocListIterator.hpp"
namespace indri
{
  namespace infnet
  {
    
    class TermFrequencyBeliefNode : public BeliefNode {
    private:
      class InferenceNetwork& _network;
      indri::query::TermScoreFunction& _function;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _extents;
      indri::utility::greedy_vector<bool> _matches;
      indri::index::DocListIterator* _list;
      double _maximumBackgroundScore;
      double _maximumScore;
      std::string _name;
      int _listID;

      indri::utility::greedy_vector<indri::index::DocListIterator::TopDocument> _emptyTopdocs;

    public:
      TermFrequencyBeliefNode( const std::string& name,
                               class InferenceNetwork& network,
                               int listID,
                               indri::query::TermScoreFunction& scoreFunction );

      ~TermFrequencyBeliefNode();

      const indri::utility::greedy_vector<indri::index::DocListIterator::TopDocument>& topdocs() const;
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
      double maximumBackgroundScore();
      double maximumScore();
      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      const std::string& getName() const;
    };
  }
}

#endif // INDRI_TERMFREQUENCYBELIEFNODE_HPP


