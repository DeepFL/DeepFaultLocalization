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
// FilterRequireNode
//
// 6 July 2004 -- tds
//

#include <algorithm>
#include "indri/FilterRequireNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Extent.hpp"
#include "indri/Annotator.hpp"

indri::infnet::FilterRequireNode::FilterRequireNode( const std::string& name, 
                                                     ListIteratorNode* filter, 
                                                     BeliefNode* required ) {
  _name = name;
  _filter = filter;
  _required = required;
}

lemur::api::DOCID_T indri::infnet::FilterRequireNode::nextCandidateDocument() {
  // both terms have to appear before this matches, so we take the max
  return lemur_compat::max( _filter->nextCandidateDocument(),
                            _required->nextCandidateDocument() );
}

double indri::infnet::FilterRequireNode::maximumBackgroundScore() {
  // delegate to the query as if the filter were true
  return _required->maximumBackgroundScore();
}

double indri::infnet::FilterRequireNode::maximumScore() {
  return _required->maximumScore();
}

bool indri::infnet::FilterRequireNode::hasMatch( lemur::api::DOCID_T documentID ) {
  // delegate to the children.
  return (_filter->extents().size() && _required->hasMatch( documentID ));
}

const indri::utility::greedy_vector<bool>& indri::infnet::FilterRequireNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  _matches.clear();
  _matches.resize( extents.size(), false );
  
  const indri::utility::greedy_vector<indri::index::Extent>& filtExtents = _filter->extents();

  indri::utility::greedy_vector<indri::index::Extent>::const_iterator innerIter = filtExtents.begin();
  indri::utility::greedy_vector<indri::index::Extent>::const_iterator outerIter = extents.begin();

  int i = 0;
  
  while( innerIter != filtExtents.end() && outerIter != extents.end() ) {
    if( outerIter->contains( *innerIter ) ) {
      _matches[i] = true;
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

const std::string& indri::infnet::FilterRequireNode::getName() const {
  return _name;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::FilterRequireNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  _extents.clear();
      
  const indri::utility::greedy_vector<indri::index::Extent>& filtExtents = _filter->extents();
  indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
  iter = std::lower_bound( filtExtents.begin(), filtExtents.end(), extent, indri::index::Extent::begins_before_less() );

  // if the filter applies, return the child score.
  while( iter != filtExtents.end() ) {
    if ( extent.contains( *iter ) )
      return _required->score( documentID, extent, documentLength );
    iter++;
  }
  return _extents;
}

void indri::infnet::FilterRequireNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  // mark up the filter
  _filter->annotate( annotator, documentID, extent);
  // if the filter applied, mark up the matches.
  const indri::utility::greedy_vector<indri::index::Extent>& filtExtents = _filter->extents();
  indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
  iter = std::lower_bound( filtExtents.begin(), filtExtents.end(), extent, indri::index::Extent::begins_before_less() );

  while( iter != filtExtents.end() ) {
    if ( extent.contains( *iter ) ) {
      _required->annotate( annotator, documentID, extent );
      return;
    }
    iter++;
  }
}

void indri::infnet::FilterRequireNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}
