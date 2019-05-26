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
// ListAccumulator
//
// 11 April 2006 -- tds
//

#ifndef INDRI_LISTACCUMULATOR_HPP
#define INDRI_LISTACCUMULATOR_HPP

#include "indri/ListIteratorNode.hpp"
#include "lemur/lemur-platform.h"
#include "lemur/lemur-compat.hpp"
#include <vector>
#include "indri/EvaluatorNode.hpp"
#include "indri/QuerySpec.hpp"
#include "indri/DocumentCount.hpp"
#include "indri/ScoredExtentResult.hpp"

namespace indri
{
  namespace infnet
  {
    class ListAccumulator : public EvaluatorNode {
    private:
      // The accumulator records all extents from the node _counted.
      ListIteratorNode& _counted;
      std::string _name;

      EvaluatorNode::MResults _results;
      std::vector<indri::api::ScoredExtentResult>* _resultVector;

    public:
      ListAccumulator( const std::string& name, ListIteratorNode& counted );
      ~ListAccumulator();

      const ListIteratorNode* getCountedNode() const;
      const EvaluatorNode::MResults& getResults();

      const std::string& getName() const;
      void evaluate( lemur::api::DOCID_T documentID, int documentLength );
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
    };
  }
}

#endif // INDRI_LISTACCUMULATOR_HPP

