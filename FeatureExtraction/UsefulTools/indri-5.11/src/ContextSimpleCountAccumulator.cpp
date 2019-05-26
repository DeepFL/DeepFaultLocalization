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
// ContextSimpleCountAccumulator
//
// 14 December 2004 -- tds
//
// Unlike the ContextCountAccumulator, which counts occurrences of
// terms in a very general way, this node uses knowledge about the 
// query tree to extract counts directly from the index.
//
// This node is placed into the query tree by the
// ContextSimpleCountCollectorCopier.
//

#include "indri/ContextSimpleCountAccumulator.hpp"
#include "indri/ScoredExtentResult.hpp"

void indri::infnet::ContextSimpleCountAccumulator::_computeCounts( indri::index::Index& index ) {
  assert( _terms.size() );
  assert( _context.size() == 0 || _field.size() == 0 );

  if( _context.size() ) {
    _size += index.fieldTermCount( _context );
    _documentCount += index.fieldDocumentCount( _context );
  } else {
    _size += index.termCount();
    _documentCount += index.documentCount();
  }

  for( size_t i=0; i<_terms.size(); i++ ) {
    if( _terms.size() == 1 )
      _documentOccurrences += index.documentCount( _terms[0] );
    else {
      // TODO: implement this case!
    }

    if( _terms[i].length() != 0 ) {
      if( _field.size() ) {
        _occurrences += index.fieldTermCount( _field, _terms[i] );
      } else if( _context.size() ) {
        _occurrences += index.fieldTermCount( _context, _terms[i] );
      } else {
        _occurrences += index.termCount( _terms[i] );
      }
    }
  }
}

indri::infnet::ContextSimpleCountAccumulator::ContextSimpleCountAccumulator( const std::string& nodeName,
                                                                             const std::vector<std::string>& terms,
                                                                             const std::string& field,
                                                                             const std::string& context ) :
  _name(nodeName),
  _terms(terms),
  _field(field),
  _context(context),
  _occurrences(0),
  _size(0),
  _documentOccurrences(0),
  _documentCount(0)
{
}

const std::string& indri::infnet::ContextSimpleCountAccumulator::getName() const {
  return _name;
}

const indri::infnet::EvaluatorNode::MResults& indri::infnet::ContextSimpleCountAccumulator::getResults() {
  _results.clear();

  _results[ "occurrences" ].push_back( indri::api::ScoredExtentResult( _occurrences, 0 ) );
  _results[ "contextSize" ].push_back( indri::api::ScoredExtentResult( _size, 0 ) );
  _results[ "documentOccurrences" ].push_back( indri::api::ScoredExtentResult(UINT64(_documentOccurrences), 0 ) );
  _results[ "documentCount" ].push_back( indri::api::ScoredExtentResult( UINT64(_documentCount), 0 ) );

  return _results;
}

void indri::infnet::ContextSimpleCountAccumulator::indexChanged( indri::index::Index& index ) {
  _computeCounts( index );
  _maximumDocument = index.documentMaximum();
}

void indri::infnet::ContextSimpleCountAccumulator::evaluate( lemur::api::DOCID_T documentID, int documentLength ) {
  // do nothing
}

lemur::api::DOCID_T indri::infnet::ContextSimpleCountAccumulator::nextCandidateDocument() {
  return MAX_INT32;
}
