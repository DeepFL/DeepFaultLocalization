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
// FilterRejectNode
//
// 6 July 2004 -- tds
//

#include "lemur/lemur-compat.hpp"
#include "indri/FilterRejectNode.hpp"
#include "indri/ListIteratorNode.hpp"
#include "indri/greedy_vector"
#include "indri/Extent.hpp"
#include "indri/Annotator.hpp"

indri::infnet::FilterRejectNode::FilterRejectNode( const std::string& name, 
                                                   indri::infnet::ListIteratorNode* filter, 
                                                   indri::infnet::BeliefNode* disallowed ) {
  _name = name;
  _filter = filter;
  _disallowed = disallowed;
}


lemur::api::DOCID_T indri::infnet::FilterRejectNode::nextCandidateDocument() {
  // it'd be nice to use the information from _filter to 
  // skip documents in the case when _filter->nextCandidate..() == _disallowed->nextCandidate...()
  // but we don't know for sure that _filter will match: we only know that it might match.
  // therefore we have to just go with the next _disallowed document.
  return _disallowed->nextCandidateDocument();
}

double indri::infnet::FilterRejectNode::maximumBackgroundScore() {
  // delegate to the query as if the filter were true
  return _disallowed->maximumBackgroundScore();
}

double indri::infnet::FilterRejectNode::maximumScore() {
  return _disallowed->maximumScore();
}

bool indri::infnet::FilterRejectNode::hasMatch( lemur::api::DOCID_T documentID ) {
  // delegate to the children.
  return ( _disallowed->hasMatch( documentID ));
}

const indri::utility::greedy_vector<bool>& indri::infnet::FilterRejectNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  _matches.clear();
  _matches.resize( extents.size(), true ); // all match unless disallowed
  
  const indri::utility::greedy_vector<indri::index::Extent>& filtExtents = _filter->extents();

  indri::utility::greedy_vector<indri::index::Extent>::const_iterator innerIter = filtExtents.begin();
  indri::utility::greedy_vector<indri::index::Extent>::const_iterator outerIter = extents.begin();

  int i = 0;
  
  while( innerIter != filtExtents.end() && outerIter != extents.end() ) {
    if( outerIter->contains( *innerIter ) ) {
      _matches[i] = false;
      outerIter++;
      i++;
    } else if( outerIter->begin <= innerIter->begin ) {
      outerIter++;
      i++;
    } else { 
      innerIter++;
    }
  }
  return _matches;
}

const std::string& indri::infnet::FilterRejectNode::getName() const {
  return _name;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::FilterRejectNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  _extents.clear();
  // if the filter doesn't apply, return the child score.
  if ( _filter->extents().size() == 0 )
    return _disallowed->score( documentID, extent, documentLength );
  else {
    const indri::utility::greedy_vector<indri::index::Extent>& filtExtents = _filter->extents();
    indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
    iter = std::lower_bound( filtExtents.begin(), filtExtents.end(), extent, indri::index::Extent::begins_before_less() );
    bool found = false;
    while( !found && iter != filtExtents.end() ) {
      found = extent.contains( *iter ) ;
      iter++;
    }
    if (!found)
      return _disallowed->score( documentID, extent, documentLength );
    else
      return _extents;
  }
}

void indri::infnet::FilterRejectNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  _filter->annotate( annotator, documentID, extent);
  if( _filter->extents().size() == 0 ) {
    _disallowed->annotate( annotator, documentID, extent );
  } else {
    const indri::utility::greedy_vector<indri::index::Extent>& filtExtents = _filter->extents();
    indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
    iter = std::lower_bound( filtExtents.begin(), filtExtents.end(), extent, indri::index::Extent::begins_before_less() );
    bool found = false;
    while( !found && iter != filtExtents.end() ) {
      found = extent.contains( *iter ) ;
      iter++;
    }
    if (!found) {
      _disallowed->annotate( annotator, documentID, extent );
      return;
    }
  }
}

void indri::infnet::FilterRejectNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}

