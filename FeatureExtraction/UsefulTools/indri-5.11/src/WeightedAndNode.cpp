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
// WeightedAndNode
//
// 26 January 2004 - tds
//

#include "indri/WeightedAndNode.hpp"
#include <algorithm>
#include "lemur/lemur-platform.h"
#include <iostream>
#include "indri/Annotator.hpp"
#include "indri/TermFrequencyBeliefNode.hpp"
#include "indri/greedy_vector"
#include "indri/delete_range.hpp"
#include "indri/Parameters.hpp"
#include "indri/ExtentRestrictionNode.hpp"
#include <cmath>

double indri::infnet::WeightedAndNode::_computeMaxScore( unsigned int start ) {
  // first, find the maximum score of the first few columns
  double maxScoreSum = 0;

  for( unsigned int i=0; i<start+1; i++ ) {
    maxScoreSum += _children[i].maximumWeightedScore;
  }

  // then add in the background score of the last columns
  double minScoreSum = 0;

  for( unsigned int i=start+1; i<_children.size(); i++ ) {
    minScoreSum += _children[i].backgroundWeightedScore;
  }

  return maxScoreSum + minScoreSum;
}

void indri::infnet::WeightedAndNode::setSiblingsFlag(int f){
  // set flag for child nodes
  for(int i=0;i<_children.size();i++) {
    _children[i].node->setSiblingsFlag(f);
  }
}

void indri::infnet::WeightedAndNode::_computeQuorum() {
  double maximumScore = -DBL_MAX;
  unsigned int i;

  // keep going until we find a necessary term
  for( i=0; i<_children.size() && maximumScore < _threshold; i++ ) {
    maximumScore = _computeMaxScore(i);
  }

  _quorumIndex = i-1;
  if( _quorumIndex < 0 )
    _quorumIndex = 0;

  if( _quorumIndex > (int)_children.size()-1 )
    _recomputeThreshold = DBL_MAX;
  else
    _recomputeThreshold = maximumScore;
}

void indri::infnet::WeightedAndNode::addChild( double weight, BeliefNode* node ) {
  child_type child;

  child.node = node;
  child.weight = weight;
  child.backgroundWeightedScore = node->maximumBackgroundScore() * weight;
  child.maximumWeightedScore = node->maximumScore() * weight;

  // set sibling flag is more than 1 child to speedup
  ExtentRestrictionNode* ernode=dynamic_cast<indri::infnet::ExtentRestrictionNode *>(node); //lezhao
  if (ernode) {
    //lezhao: ExtentRestrictionNode inside a #combine node, need to score unmatched extents!
    node->setSiblingsFlag(1); 
  }

  if (_children.size() > 1) {
    child.node->setSiblingsFlag(1);
  }

  _children.push_back( child );
  std::sort( _children.begin(), _children.end(), child_type::maxscore_less() );
  _computeQuorum();

  // if this is the second child, ensure we have set the sibling flag
  // for the first and second ones (it will skip without this!)
  if (_children.size()==2) {
    for (int i=0; i < _children.size(); i++) {
      _children[i].node->setSiblingsFlag(1);
    }
  }

}

struct double_greater {
  bool operator() ( double one, double two ) const {
    return one > two;
  }
};

void indri::infnet::WeightedAndNode::doneAddingChildren() {
  // should be removed
}

void indri::infnet::WeightedAndNode::indexChanged( indri::index::Index& index ) {
  _candidates.clear();
  _candidatesIndex = 0;

  indri::utility::greedy_vector< indri::utility::greedy_vector<indri::index::DocListIterator::TopDocument>* > lists;

  // get all the relevant topdocs lists
  for( size_t i=0; i<_children.size(); i++ ) {
    indri::infnet::TermFrequencyBeliefNode* node = dynamic_cast<indri::infnet::TermFrequencyBeliefNode*>(_children[i].node);

    if( node && indri::api::Parameters::instance().get( "topdocs", true ) ) {
      indri::utility::greedy_vector<indri::index::DocListIterator::TopDocument>* copy = new indri::utility::greedy_vector<indri::index::DocListIterator::TopDocument>( node->topdocs() );
      lists.push_back( copy );
      std::sort( copy->begin(), copy->end(), indri::index::DocListIterator::TopDocument::docid_less() );

      _children[i].maximumWeightedScore = node->maximumScore() * _children[i].weight;
      _children[i].backgroundWeightedScore = node->maximumBackgroundScore() * _children[i].weight;
    }
  }

  std::sort( _children.begin(), _children.end(), child_type::maxscore_less() );

  // TODO: could compute an initial threshold here, but that may not be necessary
  indri::utility::greedy_vector<int> indexes;

  for( size_t i=0; i<lists.size(); i++ ) {
    if( lists[i]->size() )
      indexes.push_back( 0 );
    else
      indexes.push_back( -1 );
  }

  while( true ) {
    // find the smallest document
    lemur::api::DOCID_T smallestDocument = MAX_INT32;

    for( size_t i=0; i<lists.size(); i++ ) {
      indri::utility::greedy_vector<indri::index::DocListIterator::TopDocument>& currentList = *lists[i];      

      if( indexes[i] >= 0 )
        smallestDocument = lemur_compat::min( smallestDocument, currentList[indexes[i]].document );
    }

    if( smallestDocument == MAX_INT32 )
      break;

    _candidates.push_back( smallestDocument );

    // increment indexes
    for( size_t i=0; i<lists.size(); i++ ) {
      indri::utility::greedy_vector<indri::index::DocListIterator::TopDocument>& currentList = *lists[i];      

      if( indexes[i] >= 0 && currentList[indexes[i]].document == smallestDocument ) {
        indexes[i]++;
        
        if( indexes[i] == currentList.size() ) {
          indexes[i] = -1;
        }
      }
    }
  }

  for( size_t i=0; i<lists.size(); i++ )
    delete lists[i];

  // compute quorum
  _computeQuorum();
}

void indri::infnet::WeightedAndNode::setThreshold( double threshold ) {
  _threshold = threshold;

  if( _threshold >= _recomputeThreshold ) {
   // std::cout << "threshold hit recompute " << _recomputeThreshold << std::endl;
    _computeQuorum();
  }
}
  
lemur::api::DOCID_T indri::infnet::WeightedAndNode::nextCandidateDocument() {
  std::vector<child_type>::iterator iter;
  lemur::api::DOCID_T minDocument = MAX_INT32;
  lemur::api::DOCID_T currentCandidate;

  if( _candidatesIndex < _candidates.size() ) {
    minDocument = _candidates[_candidatesIndex];
  }

  for( iter = _children.begin() + _quorumIndex; iter != _children.end(); iter++ ) {
    currentCandidate = (*iter).node->nextCandidateDocument();

    if( currentCandidate < minDocument ) {
      minDocument = currentCandidate;
    }
  }

  return minDocument;
}

void indri::infnet::WeightedAndNode::annotate( indri::infnet::Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  std::vector<child_type>::iterator iter;
  annotator.add( this, documentID, extent );

  for( iter = _children.begin(); iter != _children.end(); iter++ ) {
    (*iter).node->annotate( annotator, documentID, extent );
  }
}

double indri::infnet::WeightedAndNode::maximumBackgroundScore() {
  std::vector<child_type>::iterator iter;
  double minimum = 0.0;

  for( iter = _children.begin(); iter != _children.end(); iter++ ) {
    minimum += (*iter).weight * (*iter).node->maximumBackgroundScore();
  }

  return minimum;
}

double indri::infnet::WeightedAndNode::maximumScore() {
  std::vector<child_type>::iterator iter;
  double maximum = 0.0;

  for( iter = _children.begin(); iter != _children.end(); iter++ ) {
    maximum += (*iter).weight * (*iter).node->maximumScore();
  }

  return maximum;
}

indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::WeightedAndNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  std::vector<child_type>::iterator iter;
  double score = 0;
  double sumWeight = 0;
  bool scored = false;
  indri::utility::greedy_vector< indri::utility::greedy_vector<indri::api::ScoredExtentResult> > scores;
  for( iter = _children.begin(); iter != _children.end(); iter++ ) {
    const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = (*iter).node->score( documentID, extent, documentLength );
    scores.push_back(childResults);
    // normalize over absolute values
    sumWeight += fabs((*iter).weight) * childResults.size();
  }
  int i = 0;
  for( iter = _children.begin(); iter != _children.end(); iter++ ) {
    double childScore = 0;
    indri::utility::greedy_vector<indri::api::ScoredExtentResult>& childResults = scores[i];
    for( size_t j=0; j<childResults.size(); j++ ) {
      scored = true;
      childScore += (*iter).weight * childResults[j].score/sumWeight;
    }
    i++;
    score += childScore;
  }

  _scores.clear();
  if (scored) {
    // dmf 12/03 if no child returned an extent, don't push one
    indri::api::ScoredExtentResult result(extent);
    result.score=score;
    result.document=documentID;
    _scores.push_back( result );
  }

  // advance candidates
  while( _candidatesIndex < _candidates.size() && _candidates[_candidatesIndex] <= documentID )
    _candidatesIndex++;

  return _scores;
}

//
// hasMatch
//

bool indri::infnet::WeightedAndNode::hasMatch( lemur::api::DOCID_T documentID ) {
  // advance candidates
  while( _candidatesIndex < _candidates.size() && _candidates[_candidatesIndex] <= documentID )
    _candidatesIndex++;

  for( size_t i=0; i<_children.size(); i++ ) {
    if( _children[i].node->hasMatch( documentID ) )
      return true;
  }

  return false;
}

//
// hasMatch
//

const indri::utility::greedy_vector<bool>& indri::infnet::WeightedAndNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
  // advance candidates
  while( _candidatesIndex < _candidates.size() && _candidates[_candidatesIndex] <= documentID )
    _candidatesIndex++;

  _matches.clear();
  _matches.resize( extents.size(), false );

  for( size_t i=0; i<_children.size(); i++ ) {
    const indri::utility::greedy_vector<bool>& kidMatches = _children[i].node->hasMatch( documentID, extents );

    for( size_t j=0; j<kidMatches.size(); j++ ) {
      if( kidMatches[j] ) {
        _matches[j] = true;
      }
    }
  }

  return _matches;
}

//
// getName
//

const std::string& indri::infnet::WeightedAndNode::getName() const {
  return _name;
}

