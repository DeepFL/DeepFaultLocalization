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
// ListBeliefNode
//
// 26 January 2004 - tds
//

#ifndef INDRI_LISTBELIEFNODE_HPP
#define INDRI_LISTBELIEFNODE_HPP

#include "indri/BeliefNode.hpp"
#include "indri/TermScoreFunction.hpp"
#include "indri/greedy_vector"
#include "indri/ListIteratorNode.hpp"
#include "indri/ScoredExtentResult.hpp"
namespace indri
{
  namespace infnet
  {
    
    class ListBeliefNode : public BeliefNode {
    private:
      ListIteratorNode& _list;
      ListIteratorNode* _context;
      ListIteratorNode* _raw;
      double _maximumScore;
      double _maximumBackgroundScore;
      indri::query::TermScoreFunction& _scoreFunction;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _scores;
      indri::utility::greedy_vector<bool> _matches;
      std::string _name;
      bool _documentSmoothing;

    private:
      // computes the length of the scored context
      inline int _contextLength( indri::index::Extent &extent );
      inline double _contextOccurrences( indri::index::Extent &extent );
      inline double _documentOccurrences();

    public:
      ListBeliefNode( const std::string& name,
                      ListIteratorNode& child,
                      ListIteratorNode* context,
                      ListIteratorNode* raw,
                      indri::query::TermScoreFunction& scoreFunction,
                      double maximumBackgroundScore,
                      double maximumScore );

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

#endif // INDRI_TERMNODE_HPP
