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
// NestedListBeliefNode
//
// 30 Aug 2005
//

#include "indri/NestedListBeliefNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"

// computes the length of the scored context
int indri::infnet::NestedListBeliefNode::_contextLength( int begin, int end ) {
  //
  // There are two possible contexts at work here.  Consider the query
  // #combine[sentence]( dog.(paragraph) )
  //
  // In this case, the context for scoring is text in a paragraph field, but
  // this text will be scored for every sentence.  The paragraph field will
  // be represented by <_context>, and the sentence to be scored
  // will be represented by the term offsets <begin> and <end>.
  //

  if( !_context )
    return end - begin;

  int contextLength = 0;
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _context->extents();

  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin > end )
      break;

    if( extents[i].end < begin )
      continue; 

    // make sure to restrict the extents here to match the scored region
    int extentBegin = lemur_compat::max( extents[i].begin, begin );
    int extentEnd = lemur_compat::min( extents[i].end, end );

    contextLength += extentEnd - extentBegin;
  }

  return contextLength;
}

double indri::infnet::NestedListBeliefNode::_contextOccurrences( int begin, int end ) {
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _list.extents();
  double count = 0;
  int lastEnd = 0;

  // Ideally this chunk of code would find the highest sum of weights from 
  // non-overlapping extents that are
  // contained in the context.  I haven't spent the time to find
  // an obvious simple solution to this  
  // problem, so we will do an approximation where we take the extent that ends
  // first in a sequence and work greedily from the beginning of the extent list

  // look for all occurrences within bounds and that don't overlap
  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin >= begin &&
        extents[i].end <= end &&
        extents[i].begin >= lastEnd ) {

      count += extents[i].weight;
      lastEnd = extents[i].end;
    }
  }

  return count;
}

double indri::infnet::NestedListBeliefNode::_documentOccurrences() {
  assert( _raw ); // score() maintains this invariant
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _raw->extents();
  double count = 0;
  int lastEnd = 0;

  // look for all occurrences within bounds and that don't overlap
  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin >= lastEnd ) {
      count += extents[i].weight;
      lastEnd = extents[i].end;
    }
  }

  return count;
}

indri::infnet::NestedListBeliefNode::NestedListBeliefNode( const std::string& name, ListIteratorNode& child, ListIteratorNode* context, ListIteratorNode* raw, indri::query::TermScoreFunction& scoreFunction, double maximumBackgroundScore, double maximumScore )
  :
  _name(name),
  _scoreFunction(scoreFunction),
  _maximumScore(maximumScore),
  _maximumBackgroundScore(maximumBackgroundScore),
  _documentSmoothing(false),
  _context(context),
  _raw(raw),
  _list(child)
{
  _maximumScore = INDRI_HUGE_SCORE;
}

lemur::api::DOCID_T indri::infnet::NestedListBeliefNode::nextCandidateDocument() {
  return _list.nextCandidateDocument();
}

double indri::infnet::NestedListBeliefNode::maximumBackgroundScore() {
  return _maximumBackgroundScore;
}

double indri::infnet::NestedListBeliefNode::maximumScore() {
  return _maximumScore;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::NestedListBeliefNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  int contextSize = _contextLength( extent.begin, extent.end );
  double occurrences = _contextOccurrences( extent.begin, extent.end );
  double documentOccurrences = _raw ? _documentOccurrences() : occurrences;
  double score = 0;
  
  score = _scoreFunction.scoreOccurrence( occurrences, contextSize, documentOccurrences, documentLength );

  _scores.clear();
  indri::api::ScoredExtentResult result(extent);
  result.score=score;
  result.document=documentID;
  _scores.push_back( result );

  return _scores;
}

void indri::infnet::NestedListBeliefNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _list.extents();

  indri::index::Extent range( extent.begin, extent.end );
  indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
  iter = std::lower_bound( extents.begin(), extents.end(), range, indri::index::Extent::begins_before_less() );

  // mark the begin and end points for this list
  for( size_t i = iter - extents.begin(); i < extents.size() && extents[i].begin <= extent.end; i++ ) {
    if( extents[i].begin >= extent.begin &&
        extents[i].end <= extent.end ) {

      annotator.add( this, documentID, (indri::index::Extent &)extents[i]);
      _list.annotate( annotator, documentID, (indri::index::Extent &)extents[i]);
    }
  }
}

bool indri::infnet::NestedListBeliefNode::hasMatch( lemur::api::DOCID_T documentID ) {
  return _list.extents().size() > 0;
}

const indri::utility::greedy_vector<bool>& indri::infnet::NestedListBeliefNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& matchExtents ) {
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _list.extents();
  _matches.clear();
  _matches.resize( matchExtents.size(), false );

  // if the extent list is empty, there are no matches
  if ( extents.size() == 0 ) {
    return _matches;
  }

  size_t j = matchExtents.size(); 


  // Walk through the match list from the end.
  // As we encounter a new node in the match list:
  // - check all inner extents for a new smaller end that has the same begin or greater
  // - keep track of that smallest end
  // - we know the begin is at least as big as the match begin
  //   so we only have to check to make sure that the end is smaller than the match's end
  int matchBegin = -1;
  int smallestEnd = -1;
  indri::utility::greedy_vector<const indri::index::Extent>::iterator innerIter = extents.end(); 

  // move the pointer to the last item in the list
  innerIter--;
  bool doneInnerList = false;
  while( j > 0 ) {
    j--;
    // check to see whether we need to look for a new smallest end of an inner extent
    if ( matchBegin != matchExtents[j].begin ) {
      // new begin for the match extents
      matchBegin = matchExtents[j].begin;      
      while ( ! doneInnerList ) {
        // if the inner iter's begin is at least as large as the matchBegin
        if ( innerIter->begin >= matchBegin ) {
          // set the new smallest end if it hasn't been set yet or 
          // if it is smaller than the current smallest seen end for an inner extent
          if ( smallestEnd == -1 || 
               innerIter->end < smallestEnd ) {
            smallestEnd = innerIter->end;
          }
          if ( innerIter == extents.begin() ) {
            // mark that we've finished processing the inner extent list
            doneInnerList = true;
          } else {
            // move back to the previous inner extent
            innerIter--;
          }
        } else {
          // the inner iter's extent begin is smaller than the match extent's, so we can't consider
          // it or any others closer to the beginning of the inner extent list
          break;
        }
      }      
    }
    // check whether our extent has a match
    if ( matchExtents[j].end >= smallestEnd && smallestEnd != -1) {
      _matches[j] = true;
    }
  }

  return _matches;
}

const std::string& indri::infnet::NestedListBeliefNode::getName() const {
  return _name;
}

void indri::infnet::NestedListBeliefNode::indexChanged( indri::index::Index& index ) {
  // do nothing
}


