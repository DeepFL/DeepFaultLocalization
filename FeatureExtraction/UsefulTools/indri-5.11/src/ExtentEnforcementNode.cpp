/*==========================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software (and below), and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/


//
// ExtentEnforcementNode
//
// 31 August 2005 -- ptox
//

#include "indri/ExtentEnforcementNode.hpp"
#include "indri/Annotator.hpp"
#include "lemur/lemur-compat.hpp"

indri::infnet::ExtentEnforcementNode::ExtentEnforcementNode( const std::string& name, BeliefNode* child, ListIteratorNode* field ) :
  _name(name),
  _child(child),
  _field(field)
{
}


void indri::infnet::ExtentEnforcementNode::setSiblingsFlag(int f){
  bSiblings=f; // need to set the flag for the current node itself.
  if (_child) {  _child->setSiblingsFlag(f); }
}

lemur::api::DOCID_T indri::infnet::ExtentEnforcementNode::nextCandidateDocument() {
  return _child->nextCandidateDocument();
}

double indri::infnet::ExtentEnforcementNode::maximumBackgroundScore() {
  return INDRI_TINY_SCORE;
}

double indri::infnet::ExtentEnforcementNode::maximumScore() {
  return INDRI_HUGE_SCORE;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::ExtentEnforcementNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  // we're going to run through the field list, etc.
  indri::index::Extent tExtent( extent.begin, extent.end );
  const indri::utility::greedy_vector<indri::index::Extent>& fieldExtentsTmp = _field->matches( tExtent );
  indri::utility::greedy_vector<indri::index::Extent> fieldExtents;
  fieldExtents.append(fieldExtentsTmp.begin(), fieldExtentsTmp.end());

  indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
  _scores.clear();

  for( size_t i = 0; i < fieldExtents.size(); i++ ) {
    iter = &(fieldExtents[i]);

    if( iter->end - iter->begin == 0 )
      continue; // this field has no text in it

    int scoreBegin = iter->begin;
    int scoreEnd = iter->end;
    const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = _child->score( documentID, (indri::index::Extent&)(*iter), documentLength );

    double fieldWeight = iter->weight;

    for( size_t j=0; j<childResults.size(); j++ ) {
      // indri::api::ScoredExtentResult result( fieldWeight*childResults[j].score, documentID, childResults[j].begin, childResults[j].end );
      indri::api::ScoredExtentResult result(childResults[j]);
      result.score*=fieldWeight;
      result.document=documentID;
      _scores.push_back( result );
    }
  }

  return _scores;
}

void indri::infnet::ExtentEnforcementNode::annotate( indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {

  annotator.add(this, documentID, extent);
  // we're going to run through the field list, etc.
  
  if ( _field != NULL) {

    indri::index::Extent tExtent(extent.begin, extent.end);
    const indri::utility::greedy_vector<indri::index::Extent>& fieldExtentsTmp = _field->matches( tExtent );
    indri::utility::greedy_vector<indri::index::Extent> fieldExtents;
    fieldExtents.append(fieldExtentsTmp.begin(), fieldExtentsTmp.end());
    
    indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
    _scores.clear();
    
    for( size_t i = 0; i < fieldExtents.size(); i++ ) {    
      iter = &(fieldExtents[i]);
      
      if( iter->end - iter->begin == 0 )
        continue; // this field has no text in it
      
      int scoreBegin = iter->begin;
      int scoreEnd = iter->end;
      
      _child->annotate( annotator, documentID, (indri::index::Extent&)(*iter));
      
    } 
  }
}

//
// hasMatch
//

bool indri::infnet::ExtentEnforcementNode::hasMatch( lemur::api::DOCID_T documentID ) {
  return _child->hasMatch( documentID );
}

//
// hasMatch
// 

const indri::utility::greedy_vector<bool>& indri::infnet::ExtentEnforcementNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  // just delegate -- not perfect, but close
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

const std::string& indri::infnet::ExtentEnforcementNode::getName() const {
  return _name;
}

//
// indexChanged
//

void indri::infnet::ExtentEnforcementNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}

