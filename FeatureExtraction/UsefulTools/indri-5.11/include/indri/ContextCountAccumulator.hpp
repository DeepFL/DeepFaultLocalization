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
// ContextCountAccumulator
//
// 24 February 2004 -- tds
//

#ifndef INDRI_CONTEXTCOUNTACCUMULATOR_HPP
#define INDRI_CONTEXTCOUNTACCUMULATOR_HPP

#include "indri/ListIteratorNode.hpp"
#include "lemur/lemur-platform.h"
#include "lemur/lemur-compat.hpp"
#include <vector>
#include "indri/EvaluatorNode.hpp"
#include "indri/QuerySpec.hpp"
#include "indri/DocumentCount.hpp"
namespace indri
{
  namespace infnet
  {
    
    class ContextCountAccumulator : public EvaluatorNode {
    private:
      // this is a ListIteratorNode that contains extents
      // that compose the context of the operation. _contextSize
      // is the sum of the lengths of the extents in _context.
      ListIteratorNode* _context;

      // this is a ListIteratorNode that emits an extent only for a
      // true query match; therefore _occurrences is just a simple count
      // the number of extents in _matches.
      ListIteratorNode* _matches; 

      std::string _name;
  double _occurrences;
  double _contextSize;
      int _documentOccurrences;
      int _documentCount;
      
      EvaluatorNode::MResults _results;

public:
      ContextCountAccumulator( const std::string& name, ListIteratorNode* matches, ListIteratorNode* context );
      ~ContextCountAccumulator();

  double getOccurrences() const;
  double getContextSize() const;

      int getDocumentOccurrences() const ;
      int getDocumentCount() const ;

      const ListIteratorNode* getContextNode() const;
      const ListIteratorNode* getMatchesNode() const;
      const std::string& getName() const;

      const EvaluatorNode::MResults& getResults();
      void evaluate( lemur::api::DOCID_T documentID, int documentLength );
      lemur::api::DOCID_T nextCandidateDocument();
      void indexChanged( indri::index::Index& index );
    };
  }
}

#endif // INDRI_CONTEXTCOUNTACCUMULATOR_HPP


