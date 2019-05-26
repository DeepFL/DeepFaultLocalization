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
// CachedFrequencyBeliefNode
//
// 24 September 2004 -- tds
//

#ifndef INDRI_CACHEDFREQUENCYBELIEFNODE_HPP
#define INDRI_CACHEDFREQUENCYBELIEFNODE_HPP

#include "indri/Annotator.hpp"
#include "indri/ScoredExtentResult.hpp"
#include <string>
#include "indri/TermScoreFunction.hpp"
#include "indri/ListBeliefNode.hpp"
#include "indri/ListCache.hpp"
namespace indri
{
  namespace infnet 
  {
    
    class CachedFrequencyBeliefNode : public BeliefNode {
    private:
      indri::query::TermScoreFunction& _function;
      indri::utility::greedy_vector<indri::api::ScoredExtentResult> _extents;
      indri::utility::greedy_vector<indri::index::DocumentContextCount>::iterator _iter;
      indri::utility::greedy_vector<bool> _matches;
      indri::lang::ListCache::CachedList* _list;
      double _maximumBackgroundScore;
      double _maximumScore;
      std::string _name;

    public:
      CachedFrequencyBeliefNode( const std::string& name,
                                 indri::lang::ListCache::CachedList* list,
                                 indri::query::TermScoreFunction& scoreFunction,
                                 double maximumBackgroundScore,
                                 double maximumScore );

      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );

      double maximumBackgroundScore();
      double maximumScore();
      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, int begin, int end, int documentLength );
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, int begin, int end );
      bool hasMatch( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents );
      
      const std::string& getName() const;
    };
  }
}

#endif // INDRI_CACHEDFREQUENCYBELIEFNODE_HPP

