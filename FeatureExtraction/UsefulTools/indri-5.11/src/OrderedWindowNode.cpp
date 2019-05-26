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
// OrderedWindowNode
//
// 28 July 2004 -- tds
//

#include "indri/OrderedWindowNode.hpp"
#include "indri/Annotator.hpp"

indri::infnet::OrderedWindowNode::OrderedWindowNode( const std::string& name, const std::vector<indri::infnet::ListIteratorNode*>& children ) :
  _children(children),
  _windowSize(-1), // unlimited window size
  _name(name)
{
  _pointers.resize(children.size());
}

indri::infnet::OrderedWindowNode::OrderedWindowNode( const std::string& name, const std::vector<indri::infnet::ListIteratorNode*>& children, int windowSize ) :
  _children(children),
  _windowSize(windowSize),
  _name(name)
{
  _pointers.resize(children.size());
}

lemur::api::DOCID_T indri::infnet::OrderedWindowNode::nextCandidateDocument() {
  lemur::api::DOCID_T maxDocument = 0;

  for( size_t i=0; i<_children.size(); i++ ) {
    lemur::api::DOCID_T current = _children[i]->nextCandidateDocument();
    if( current > maxDocument )
      maxDocument = current;
  }

  return maxDocument;
}

// tried to find the first occurrence of a term for the i-th iterator
// that might possible work (has to start after the last one ends;
// this breaks CMU's #0 property syntax)
// returns true if it's okay to proceed; false if there's no such term
// will set boolean argument "fail" to true if we should fail on this iteration
// but a nested extent could possibly save the match; false otherwise.
bool indri::infnet::OrderedWindowNode::findNextPossibleOccurrence(int i, 
                                                                  bool& fail) {
  fail=false;
  // Usually we have to look forward...
  if (_pointers[i].iter->begin < _pointers[i-1].iter->end) {
    bool behindBegin=(_pointers[i].iter->end<_pointers[i-1].iter->begin);
    while( _pointers[i].iter->begin < _pointers[i-1].iter->end ) {
      
      _pointers[i].iter++;
      // if we're out of word positions for any term, then we're done
      if( _pointers[i].iter == _pointers[i].end ) {          
        // used to always be "return" but we can't because if we
        // overshot on a nested extent (see below) we could bail
        // too early. However, if we're so far behind that we're
        // before the *beginning* of the current extent, then we
        // don't have to worry about that
        if (behindBegin) {
          return false;
        } else {
          fail=true;
          --_pointers[i].iter;
          break;
        }
      }
    }
  } else {
    // but when there are nested extents, we might have overshot and
    // need to go backwards.
    // Consider the query #1(#any:per married #any:per) matching
    // "<per>Harriet, <per>who</per> married <per>Peter</per></per>
    // and <per>Fred, <per>who</per> married <per>Sally</per></per>.
    // We'll first try #any:per = "Harriet, who married Peter", which
    // will fast-forward our "married" iterator to the second "married",
    // so if we don't back up, the first <per>who</per> will never get
    // the chance to be tried out with the first "married".
    while (_pointers[i].iter!=_pointers[i].start) {
      --_pointers[i].iter;
      if (_pointers[i].iter->begin < _pointers[i-1].iter->end) {
        ++_pointers[i].iter;
        break;
      }
    }
  }
  return true;
}

/*
  FIXME: A problem still remains if there is a wildcard in a middle term,
  rather than the first or the last, because when we do the break right
  after the "match=false" we go all the way back to looking for the next
  extent for the first term and not the immediately previous term. I'd fix
  that too, but unfortunately the current version fixes my project's
  immediate problem, so I'm not permitted to spend more time on it.  I
  don't think it's far from a complete solution, though.
*/

void indri::infnet::OrderedWindowNode::prepare( lemur::api::DOCID_T documentID ) {
  
  // initialize the child / sibling pointer
  initpointer();
  _extents.clear();
  _lastExtent.begin = -1;
  _lastExtent.end = -1;
  assert( _children.size() >= 2 );
  // TODO: could make this faster:
  //          sort incoming words by the number of times they occur
  //          do initial matching with the infrequent terms
  //          check the candidate matches with the more frequent ones.
  // initialize children indices
  for( size_t i=0; i<_children.size(); i++ ) {
    const indri::utility::greedy_vector<indri::index::Extent>& childPositions = _children[i]->extents();

    _pointers[i].iter = childPositions.begin();
    _pointers[i].start = childPositions.begin();
    _pointers[i].end = childPositions.end();
    if( _pointers[i].iter == _pointers[i].end ) {
      // if one word doesn't appear, then there's no use
      // looking for any occurrences
      return;
    }
  }
  // now, find every position possible
  // the outer loop iterates over the first word in the sequence,
  // while the inner loop iterates over the remaining words
  for( ; _pointers[0].iter != _pointers[0].end; (_pointers[0].iter)++ ) {
    bool match = true;
    double weight = 1.0;
    for( size_t i=1; i<_pointers.size(); i++ ) {
      bool dontMatch=false;
      if (!findNextPossibleOccurrence(i, dontMatch)) {
        return;
      }
      // now check the distance.  It's a match if there are fewer
      // than _windowSize-1 positions between the end of the last term
      // and the beginning of this one

      if(dontMatch || ((_pointers[i].iter->begin - _pointers[i-1].iter->end + 1 >
                        _windowSize) && (_windowSize >= 0) )) {
        // word <i> appears too far from the last word
        match = false;
        break;
      } else {
        weight *= _pointers[i].iter->weight;
      }
    }
    if( match ) {
      // the match extent spans the beginning of the first term and 
      // the end of the last term
      _extents.push_back( indri::index::Extent( weight, _pointers.front().iter->begin, _pointers.back().iter->end ) );
    }
  }
}

#if 0
void indri::infnet::OrderedWindowNode::prepare( lemur::api::DOCID_T documentID ) {
  // initialize the child / sibling pointer
  initpointer();
  _extents.clear();
  _lastExtent.begin = -1;
  _lastExtent.end = -1;

  assert( _children.size() >= 2 );

  // TODO: could make this faster:
  //          sort incoming words by the number of times they occur
  //          do initial matching with the infrequent terms
  //          check the candidate matches with the more frequent ones.

  // initialize children indices
  for( size_t i=0; i<_children.size(); i++ ) {
    const indri::utility::greedy_vector<indri::index::Extent>& childPositions = _children[i]->extents();
    _pointers[i].iter = childPositions.begin();
    _pointers[i].end = childPositions.end();

    if( _pointers[i].iter == _pointers[i].end ) {
      // if one word doesn't appear, then there's no use
      // looking for any occurrences
      return;
    }
  }

  // now, find every position possible
  // the outer loop iterates over the first word in the sequence,
  // while the inner loop iterates over the remaining words
  for( ; _pointers[0].iter != _pointers[0].end; (_pointers[0].iter)++ ) {
    bool match = true;
    double weight = 1.0;

    for( size_t i=1; i<_pointers.size(); i++ ) {
      // try to find the first occurrence of this term that might
      // possibly work (has to start after the last one ends; this breaks CMU's #0 property syntax)
      while( _pointers[i].iter->begin < _pointers[i-1].iter->end ) {
        _pointers[i].iter++;
        // if we're out of word positions for any term, then we're done
        if( _pointers[i].iter == _pointers[i].end )
          return;
      }

      // now check the distance.  It's a match if there are fewer
      // than _windowSize-1 positions between the end of the last term
      // and the beginning of this one
      if( (_pointers[i].iter->begin - _pointers[i-1].iter->end + 1 > _windowSize) && (_windowSize >= 0) ) {
        // word <i> appears too far from the last word
        match = false;
        break;
      } else {
        weight *= _pointers[i].iter->weight;
      }
    }

    if( match ) {
      // the match extent spans the beginning of the first term and the end of the
      // last term
      _extents.push_back( indri::index::Extent( weight, _pointers.front().iter->begin, _pointers.back().iter->end ) );
    }
  }
}
#endif

const indri::utility::greedy_vector<indri::index::Extent>& indri::infnet::OrderedWindowNode::extents() {
  return _extents;
}

const std::string& indri::infnet::OrderedWindowNode::getName() const {
  return _name;
}

void indri::infnet::OrderedWindowNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
  if (! _lastExtent.contains(extent)) {
    // if the last extent we annotated contains this one, there is no work
    // to do.
    _lastExtent = extent;
    annotator.addMatches( _extents, this, documentID, extent );
    indri::utility::greedy_vector<indri::index::Extent>::const_iterator iter;
    iter = std::lower_bound( _extents.begin(), _extents.end(), extent, indri::index::Extent::begins_before_less() );

    while( iter != _extents.end() ) {
      for( size_t j=0; j<_children.size(); j++ ) {
        indri::index::Extent e = (*iter);
        if (extent.contains(e)) {
          _children[j]->annotate( annotator, documentID, e );
        }
      }
      iter++;
    }
  }
}

void indri::infnet::OrderedWindowNode::indexChanged( indri::index::Index& index ) {
  _lastExtent.begin = -1;
  _lastExtent.end = -1;
}
