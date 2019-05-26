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
// ExtentRestrictionNode
//
// 6 July 2004 -- tds
//

#include "indri/ExtentRestrictionNode.hpp"
#include "indri/Annotator.hpp"
#include "lemur/lemur-compat.hpp"

indri::infnet::ExtentRestrictionNode::ExtentRestrictionNode( const std::string& name, BeliefNode* child, ListIteratorNode* field ) :
  _name(name),
  _child(child),
  _field(field),
 _matches()
{
  bSiblings = 1;
}

void indri::infnet::ExtentRestrictionNode::setSiblingsFlag(int f){
  bSiblings=f; // need to set the flag for the current node itself.
  if (_child) {  _child->setSiblingsFlag(f); }
}

lemur::api::DOCID_T indri::infnet::ExtentRestrictionNode::nextCandidateDocument() {
  return _child->nextCandidateDocument();
}

double indri::infnet::ExtentRestrictionNode::maximumBackgroundScore() {
  return INDRI_TINY_SCORE;
}

double indri::infnet::ExtentRestrictionNode::maximumScore() {
  return INDRI_HUGE_SCORE;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::ExtentRestrictionNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  // we're going to run through the field list, etc.
  if ( _field != NULL ) {

    indri::index::Extent tExtent(extent);
    const indri::utility::greedy_vector<indri::index::Extent>& fieldExtentsTmp = _field->matches( tExtent );
    indri::utility::greedy_vector<indri::index::Extent> fieldExtents;
    fieldExtents.append(fieldExtentsTmp.begin(), fieldExtentsTmp.end());
    //open question whether to score all extents or not -- dmf    
       const indri::utility::greedy_vector<bool>& matches = _child->hasMatch( documentID, fieldExtents );
       assert( matches.size() == fieldExtents.size() );
    
    indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
    _scores.clear();
    
    for( size_t i = 0; i < fieldExtents.size(); i++ ) {    
    //open question whether to score all extents or not -- dmf    

      /// score all, but also all siblings if no matches...
      if( !matches[i] && !bSiblings )  // We actually want to score all, whether or not they have a query term 
        continue;        // match.  This will give us proper scores when ther eis not a match.
      
      iter = &(fieldExtents[i]);
      
      if( iter->end - iter->begin == 0 )
        continue; // this field has no text in it
      
      int scoreBegin = iter->begin;
      int scoreEnd = iter->end;
      
      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = _child->score( documentID, (indri::index::Extent&)(*iter), documentLength );
      
      double fieldWeight = iter->weight;
      
      for( size_t j=0; j<childResults.size(); j++ ) {
        // indri::api::ScoredExtentResult result( fieldWeight*childResults[j].score, documentID, scoreBegin, scoreEnd );
        indri::api::ScoredExtentResult result(childResults[j]);
        result.score*=fieldWeight;
        result.document=documentID;
        result.begin=scoreBegin;
        result.end=scoreEnd;
        result.ordinal=iter->ordinal;
        result.parentOrdinal=iter->parent;

        _scores.push_back( result );
      }
    } 

    // do a bad guess if there's no matching fields but there are siblings
    if ( _scores.size() == 0 && bSiblings) {
      indri::index::Extent e( extent );
      e.end = e.begin;
      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = _child->score( documentID, e, documentLength );
      for( int i=0; i<childResults.size(); i++ ) {
        // indri::api::ScoredExtentResult result( childResults[i].score, documentID, extent.begin, extent.end, 0 );
        indri::api::ScoredExtentResult result( childResults[i] );
        result.document=documentID;
        result.begin=extent.begin;
        result.end=extent.end;
        _scores.push_back( result );
      }
    }

  } else {
    return _child->score( documentID, extent, documentLength );    
  }

  return _scores;
}

void indri::infnet::ExtentRestrictionNode::annotate( indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {

  annotator.add(this, documentID, extent);
  // we're going to run through the field list, etc.

  if ( _field != NULL) {

    indri::index::Extent tExtent(extent);
    const indri::utility::greedy_vector<indri::index::Extent>& fieldExtentsTmp = _field->matches( tExtent );
    indri::utility::greedy_vector<indri::index::Extent> fieldExtents;
    fieldExtents.append(fieldExtentsTmp.begin(), fieldExtentsTmp.end());
    
    const indri::utility::greedy_vector<bool>& matches = _child->hasMatch( documentID, fieldExtents );
    assert( matches.size() == fieldExtents.size() );
    
    indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
    _scores.clear();
    
    for( size_t i = 0; i < fieldExtents.size(); i++ ) {    
      if( !matches[i] )
        continue;
      
      iter = &(fieldExtents[i]);
      
      if( iter->end - iter->begin == 0 )
        continue; // this field has no text in it
      
      _child->annotate( annotator, documentID, (indri::index::Extent&)(*iter));
      
    } 

  }
}

//
// hasMatch
//

bool indri::infnet::ExtentRestrictionNode::hasMatch( lemur::api::DOCID_T documentID ) {
  return _child->hasMatch( documentID );
}

//
// hasMatch
// 

const indri::utility::greedy_vector<bool>& indri::infnet::ExtentRestrictionNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
   _matches.clear();
  _matches.resize( extents.size(), false );

  if ( _field != NULL ) {

    for ( size_t i = 0; i < extents.size(); i++ ) {
      const indri::utility::greedy_vector<indri::index::Extent>& fieldExtents = _field->matches( (indri::index::Extent &)extents[i] );
      
      const indri::utility::greedy_vector<bool>& childMatches = _child->hasMatch( documentID, fieldExtents );
      bool match = false;
      for ( size_t j = 0; !match && j < childMatches.size(); j++ ) {
        
      if ( childMatches[ j ] ) {
        match = true;
      }
      }
      _matches[i] = match;
    }
  } else {
    return _child->hasMatch( documentID, extents );
  }
  

  return _matches;
}

//
// getName
//

const std::string& indri::infnet::ExtentRestrictionNode::getName() const {
  return _name;
}

//
// indexChanged
//

void indri::infnet::ExtentRestrictionNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}

