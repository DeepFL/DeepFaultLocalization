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
// InferenceNetworkBuilder
//
// 8 March 2004 -- tds
//

#include "indri/InferenceNetworkBuilder.hpp"
#include "indri/ScopedLock.hpp"

#include "indri/ContextCountAccumulator.hpp"
#include "indri/ContextSimpleCountAccumulator.hpp"
#include "indri/ListAccumulator.hpp"

#include "indri/DocListIteratorNode.hpp"
#include "indri/ExtentInsideNode.hpp"
#include "indri/ExtentAndNode.hpp"
#include "indri/ExtentOrNode.hpp"
#include "indri/WeightedExtentOrNode.hpp"
#include "indri/OrderedWindowNode.hpp"
#include "indri/UnorderedWindowNode.hpp"
#include "indri/FieldIteratorNode.hpp"
#include "indri/ListBeliefNode.hpp"
#include "indri/ScoredExtentAccumulator.hpp"
#include "indri/WeightedAndNode.hpp"
#include "indri/NullScorerNode.hpp"
#include "indri/OrNode.hpp"
#include "indri/NotNode.hpp"
#include "indri/WeightedAndNode.hpp"
#include "indri/WeightedSumNode.hpp"
#include "indri/FilterRejectNode.hpp"
#include "indri/FilterRequireNode.hpp"
#include "indri/FieldBetweenNode.hpp"
#include "indri/FieldEqualsNode.hpp"
#include "indri/FieldLessNode.hpp"
#include "indri/FieldGreaterNode.hpp"
#include "indri/MaxNode.hpp"
#include "indri/PriorNode.hpp"
#include "indri/ExtentRestrictionNode.hpp"
#include "indri/FixedPassageNode.hpp"
#include "indri/FilterNode.hpp"
#include "indri/NullListNode.hpp"
#include "indri/PlusNode.hpp"
#include "indri/WPlusNode.hpp"
#include "indri/TermScoreFunctionFactory.hpp"
#include "indri/TermFrequencyBeliefNode.hpp"
#include "indri/CachedFrequencyBeliefNode.hpp"
#include "indri/BooleanAndNode.hpp"
#include "indri/FieldWildcardNode.hpp"
#include "indri/NestedExtentInsideNode.hpp"
#include "indri/NestedListBeliefNode.hpp"
#include "indri/ExtentEnforcementNode.hpp"
#include "indri/ContextInclusionAndNode.hpp"
#include "indri/LengthPriorNode.hpp"
#include "indri/DocumentStructureHolderNode.hpp"
#include "indri/ShrinkageBeliefNode.hpp"
#include "indri/ExtentParentNode.hpp"
#include "indri/ExtentChildNode.hpp"
#include "indri/ExtentDescendantNode.hpp"

#include "indri/FieldBelowWalker.hpp"
#include <cmath>

#include <stdexcept>

indri::query::TermScoreFunction* indri::infnet::InferenceNetworkBuilder::_buildTermScoreFunction( const std::string& smoothing, double occurrences, double contextSize, int documentOccurrences, int documentCount ) const {
  double collectionFrequency;
  return indri::query::TermScoreFunctionFactory::get( smoothing, occurrences, contextSize, documentOccurrences, documentCount );
}

indri::infnet::InferenceNetworkBuilder::InferenceNetworkBuilder( indri::collection::Repository& repository, indri::lang::ListCache& cache, int resultsRequested, int maxWildcardTerms ) :
  _repository(repository),
  _cache(cache),
  _network( new indri::infnet::InferenceNetwork( repository ) ),
  _resultsRequested( resultsRequested ),
  _maxWildcardTerms( maxWildcardTerms )
{
}

indri::infnet::InferenceNetworkBuilder::~InferenceNetworkBuilder() {
  delete _network;
}

indri::infnet::InferenceNetwork* indri::infnet::InferenceNetworkBuilder::getNetwork() {
  return _network;
}

void indri::infnet::InferenceNetworkBuilder::defaultAfter( indri::lang::Node* node ) {
  LEMUR_THROW( LEMUR_INTERNAL_ERROR, "InferenceNetworkBuilder found a node type that it didn't know what to do with" );
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::IndexTerm* term ) {
  if( _nodeMap.find( term ) == _nodeMap.end() ) {
    bool stopword = false;
    std::string processed = term->getText();
    int termID = 0;
    ListIteratorNode* iteratorNode = 0;
    
    // stem and stop the word
    if( term->getStemmed() == false ) {
      processed = _repository.processTerm( term->getText() );
      stopword = processed.length() == 0;
    }

    // if it isn't a stopword, we can try to get it from the index
    if( !stopword ) {
      int listID = _network->addDocIterator( processed );
      iteratorNode = new DocListIteratorNode( term->nodeName(), *_network, listID );
    }
    
    // if we didn't find anything at the index, make a placeholder
    if( !iteratorNode ) {
      iteratorNode = new NullListNode( term->nodeName(), stopword );
    }

    _network->addListNode( iteratorNode );
    _nodeMap[term] = iteratorNode;
  }
}

//
// Field
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::Field* field ) {
  if( _nodeMap.find( field ) == _nodeMap.end() ) {
    indri::index::FieldListIterator* iterator = 0;
    indri::infnet::ListIteratorNode* iteratorNode = 0;

    int listID = _network->addFieldIterator( field->getFieldName() );
    iteratorNode = new indri::infnet::FieldIteratorNode( field->nodeName(), *_network, listID );

    _network->addListNode( iteratorNode );
    _nodeMap[field] = iteratorNode;
  }
}

//
// ExtentRestriction
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ExtentRestriction* erNode ) {
  indri::lang::ExtentEnforcement* eeNode = dynamic_cast<indri::lang::ExtentEnforcement*>(erNode);
  if( eeNode ) {
    _after( eeNode );
  } else if( _nodeMap.find( erNode ) == _nodeMap.end() ) {
    indri::infnet::BeliefNode* childNode = dynamic_cast<indri::infnet::BeliefNode*>(_nodeMap[erNode->getChild()]);
    indri::infnet::ListIteratorNode* fieldNode = dynamic_cast<indri::infnet::ListIteratorNode*>(_nodeMap[erNode->getField()]);
    indri::infnet::ExtentRestrictionNode* extentRestriction = new indri::infnet::ExtentRestrictionNode( erNode->nodeName(), childNode, fieldNode );

    _network->addBeliefNode( extentRestriction );
    _nodeMap[erNode] = extentRestriction;
  }
}

//
// FixedPassage
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::FixedPassage* fpNode ) {
  if( _nodeMap.find( fpNode ) == _nodeMap.end() ) {
    indri::infnet::BeliefNode* childNode = dynamic_cast<indri::infnet::BeliefNode*>(_nodeMap[fpNode->getChild()]);
    indri::infnet::FixedPassageNode* fixedPassage = new indri::infnet::FixedPassageNode( fpNode->nodeName(),
                                                                                         childNode,
                                                                                         fpNode->getWindowSize(),
                                                                                         fpNode->getIncrement() );

    _network->addBeliefNode( fixedPassage );
    _nodeMap[fpNode] = fixedPassage;
  }
}

//
// ExtentAnd
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ExtentAnd* extentAnd ) {
  if( _nodeMap.find( extentAnd ) == _nodeMap.end() ) {
    std::vector<indri::infnet::ListIteratorNode*> translation = _translate<indri::infnet::ListIteratorNode>( extentAnd->getChildren() );
    indri::infnet::ExtentAndNode* extentAndNode = new indri::infnet::ExtentAndNode( extentAnd->nodeName(), translation );

    _network->addListNode( extentAndNode );
    _nodeMap[extentAnd] = extentAndNode;
  }
}

//
// ExtentOr
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ExtentOr* extentOr ) {
  if( _nodeMap.find( extentOr ) == _nodeMap.end() ) {
    std::vector<indri::infnet::ListIteratorNode*> translation = _translate<indri::infnet::ListIteratorNode>( extentOr->getChildren() );
    indri::infnet::ExtentOrNode* extentOrNode = new indri::infnet::ExtentOrNode( extentOr->nodeName(), translation );

    _network->addListNode( extentOrNode );
    _nodeMap[extentOr] = extentOrNode;
  }
}

//
// WeightedExtentOr
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::WeightedExtentOr* weightedExtentOr ) {
  if( _nodeMap.find( weightedExtentOr ) == _nodeMap.end() ) {
    std::vector<ListIteratorNode*> translation = _translate<ListIteratorNode>( weightedExtentOr->getChildren() );
    std::vector<double>& weights = weightedExtentOr->getWeights();
    WeightedExtentOrNode* weightedExtentOrNode = new WeightedExtentOrNode( weightedExtentOr->nodeName(), translation, weights );

    _network->addListNode( weightedExtentOrNode );
    _nodeMap[weightedExtentOr] = weightedExtentOrNode;
  }
}

//
// ExtentInside
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ExtentInside* extentInside ) {
  indri::lang::NestedExtentInside* nestedExtentInside = dynamic_cast<indri::lang::NestedExtentInside*>(extentInside);
  if( nestedExtentInside ) {
    _after( nestedExtentInside );
  } else if( _nodeMap.find( extentInside ) == _nodeMap.end() ) {
    indri::lang::FieldBelowWalker fieldFinder;
    extentInside->walk( fieldFinder );
    // If we use a field in specifying the ExtentInside (is that always?), we could have nested fields (or overlapping) and need the
    // NestedExtentInside operator.  It would be nice later if Field lists know whether there's nesting or not (auto-detection at index time?)
    // and then the FieldBelowWalker would check for nested/overlapping fields below rather than all fields.
    if ( fieldFinder.fieldBelow() ) {    
      NestedExtentInsideNode* extentInsideNode = new NestedExtentInsideNode( 
                                                                            extentInside->nodeName(),
                                                                            dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getInner()]),
                                                                            dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getOuter()]) );

      _network->addListNode( extentInsideNode );
      _nodeMap[extentInside] = extentInsideNode;
    } else {
      ExtentInsideNode* extentInsideNode = new ExtentInsideNode( 
                                                                extentInside->nodeName(),
                                                                dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getInner()]),
                                                                dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getOuter()]) );

      _network->addListNode( extentInsideNode );
      _nodeMap[extentInside] = extentInsideNode;
    }
  }
}

static bool inferencenetworkbuilder_is_stopword( indri::infnet::ListIteratorNode* node ) {
  indri::infnet::NullListNode* nullNode = dynamic_cast<indri::infnet::NullListNode*>(node);
  return nullNode && nullNode->isStopword();
}

static bool inferencenetworkbuilder_contains_stopwords( const std::vector<indri::infnet::ListIteratorNode*>& nodes ) {
  // scan for stopwords
  int stopwords = 0;

  for( size_t i=0; i<nodes.size(); i++ ) {
    if( inferencenetworkbuilder_is_stopword(nodes[i]) )
      stopwords++;
  }

  return stopwords != 0;
}

static int inferencenetworkbuilder_find_stopwords_left( const std::vector<indri::infnet::ListIteratorNode*>& nodes ) {
  size_t begin;

  for( begin=0; begin<nodes.size(); begin++ ) {
    if( inferencenetworkbuilder_is_stopword(nodes[begin]) )
      continue;

    break;
  }

  return int(begin);
}

static int inferencenetworkbuilder_find_stopwords_right( const std::vector<indri::infnet::ListIteratorNode*>& nodes ) {
  int end;

  if( nodes.size() == 0 )
    return 0;

  for( end=(int)nodes.size()-1; end>=0; end-- ) {
    if( inferencenetworkbuilder_is_stopword(nodes[end]) )
      continue;

    break;
  }

  return end+1;
}

static void inferencenetworkbuilder_find_stopwords_stretch( const std::vector<indri::infnet::ListIteratorNode*>& nodes, int& begin, int& end ) {
  for( begin=0; begin<int(nodes.size()); begin++ ) {
    if( inferencenetworkbuilder_is_stopword(nodes[begin]) )
      break;
  }

  for( end=begin+1; end<int(nodes.size()); end++ ) {
    if( !inferencenetworkbuilder_is_stopword(nodes[end]) )
      break;
  }
}

static indri::infnet::ListIteratorNode* inferencenetworkbuilder_build_child_window( indri::infnet::InferenceNetwork* network, const std::vector<indri::infnet::ListIteratorNode*>& nodes, int windowSize ) {
  assert( nodes.size() ); // we should have nodes
  assert( !inferencenetworkbuilder_is_stopword(nodes.front()) ); // the front must not be a stopword
  assert( !inferencenetworkbuilder_is_stopword(nodes.back()) );  // the back must not be a stopword

  std::string nodeName = "unknown";

  if( ! inferencenetworkbuilder_contains_stopwords( nodes ) ) {
    indri::infnet::ListIteratorNode* total = 0;
    
    if( nodes.size() == 1 ) {
      total = nodes.front();
    } else {
      total = new indri::infnet::OrderedWindowNode( nodeName, nodes, windowSize );
      network->addListNode( total );
    }

    return total;
  } else {
    int begin;
    int end;

    inferencenetworkbuilder_find_stopwords_stretch( nodes, begin, end );

    std::vector<indri::infnet::ListIteratorNode*> leftNodes;
    std::vector<indri::infnet::ListIteratorNode*> rightNodes;
    leftNodes.assign( nodes.begin(), nodes.begin()+begin );
    rightNodes.assign( nodes.begin()+end, nodes.end() );

    indri::infnet::ListIteratorNode* left = 0; 
    indri::infnet::ListIteratorNode* right = 0;

    if( leftNodes.size() == 1 )
      left = leftNodes.front();
    else
      left = inferencenetworkbuilder_build_child_window( network, leftNodes, windowSize );
    
    if( rightNodes.size() == 1 )
      right = rightNodes.front();
    else if( rightNodes.size() > 1 )
      right = inferencenetworkbuilder_build_child_window( network, rightNodes, windowSize );

    std::vector<indri::infnet::ListIteratorNode*> totalNodes;

    if( left )
      totalNodes.push_back( left );
    if( right )
      totalNodes.push_back( right );

    assert( totalNodes.size() >= 1 );
    indri::infnet::ListIteratorNode* total = 0;

    if( totalNodes.size() > 1 ) {
      total = new indri::infnet::OrderedWindowNode( nodeName, totalNodes, windowSize*(end-begin+1) );
      network->addListNode( total );
    } else {
      total = totalNodes.front();
    }

    return total;
  }
}

static indri::infnet::ListIteratorNode* inferencenetworkbuilder_build_ordered_window( indri::infnet::InferenceNetwork* network, const std::string& nodeName, const std::vector<indri::infnet::ListIteratorNode*>& nodes, int windowSize ) {
  // trim stopwords from beginning and ending
  int begin = inferencenetworkbuilder_find_stopwords_left( nodes );
  int end = inferencenetworkbuilder_find_stopwords_right( nodes );
  indri::infnet::ListIteratorNode* result = 0;

  // We can't just ignore stopwords, because they'll cause this window to not match unnecessarily.
  // Here's the plan: if we find a stopword, we throw it out, but we build a two-level ordered window in its place,
  // like this: (assuming "the" is a stopword)
  // #1(jumped over the lazy dog) = #2( #1(jumped over) #1(lazy dog) )

  if( end <= begin ) {
    // window contains no non-stopwords
    indri::infnet::NullListNode* nullNode = new indri::infnet::NullListNode( nodeName, true );
    result = nullNode;
    network->addListNode( result );
  } else {
    // window contains some non-stopwords
    std::vector<indri::infnet::ListIteratorNode*> inner;
    inner.assign( nodes.begin()+begin, nodes.begin()+end );

    if( inferencenetworkbuilder_contains_stopwords( inner ) ) {
      // window contains some stopwords in the middle, sandwitched between non-stopwords
      inferencenetworkbuilder_find_stopwords_stretch( inner, begin, end );

      std::vector<indri::infnet::ListIteratorNode*> leftNodes;
      std::vector<indri::infnet::ListIteratorNode*> rightNodes;
      leftNodes.assign( inner.begin(), inner.begin()+begin );
      rightNodes.assign( inner.begin()+end, inner.end() );

      indri::infnet::ListIteratorNode* left = inferencenetworkbuilder_build_child_window( network, leftNodes, windowSize );
      indri::infnet::ListIteratorNode* right = inferencenetworkbuilder_build_child_window( network, rightNodes, windowSize );

      std::vector<indri::infnet::ListIteratorNode*> totalNodes;
      totalNodes.push_back( left );
      totalNodes.push_back( right );

      indri::infnet::OrderedWindowNode* total = new indri::infnet::OrderedWindowNode( nodeName, totalNodes, windowSize*(end-begin+1) );
      result = total;
      network->addListNode( result );
    } else if( inner.size() > 1 ) {
      // no stopwords left, more than 1 word
      indri::infnet::OrderedWindowNode* odNode = new indri::infnet::OrderedWindowNode( nodeName, inner, windowSize );
      result = odNode;
      network->addListNode( result );
    } else if( inner.size() == 1 ) {
      // no stopwords left, only one word
      result = inner.front();
    } else {
      // no stopwords left, no words
      assert( false && "shouldn't ever get here, handled by top case" );
    }
  }

  return result;
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ODNode* odNode ) {
  if( _nodeMap.find( odNode ) == _nodeMap.end() ) {
    std::vector<ListIteratorNode*> translation = _translate<ListIteratorNode>( odNode->getChildren() );
    ListIteratorNode* orderedNode = inferencenetworkbuilder_build_ordered_window( _network, odNode->nodeName(), translation, odNode->getWindowSize() ); 

    _nodeMap[odNode] = orderedNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::UWNode* uwNode ) {
  if( _nodeMap.find( uwNode ) == _nodeMap.end() ) {
    std::vector<ListIteratorNode*> translation = _translate<ListIteratorNode>( uwNode->getChildren() );
    std::vector<ListIteratorNode*> nullsRemoved;
    nullsRemoved.reserve( translation.size() );

    for( size_t i=0; i<translation.size(); i++ ) {
      NullListNode* nullNode = dynamic_cast<NullListNode*>(translation[i]);
      // we want to remove words that are stopwords (either query or index) because they'll cause this window not to match
      if( !(nullNode && nullNode->isStopword()) ) 
        nullsRemoved.push_back( translation[i] );
    }

    ListIteratorNode* unorderedNode = 0; 
    
    if( nullsRemoved.size() == 0 ) {
      unorderedNode = new NullListNode( uwNode->nodeName(), true );
      _network->addListNode( unorderedNode );
    } else if( nullsRemoved.size() == 1 ) {
      unorderedNode = nullsRemoved.front();
    } else {
      unorderedNode = new UnorderedWindowNode( uwNode->nodeName(), nullsRemoved, uwNode->getWindowSize() );
      _network->addListNode( unorderedNode );
    }

    _nodeMap[uwNode] = unorderedNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::BAndNode* bandNode ) {
  if( _nodeMap.find( bandNode ) == _nodeMap.end() ) {
    std::vector<ListIteratorNode*> translation = _translate<ListIteratorNode>( bandNode->getChildren() );
    std::vector<ListIteratorNode*> nullsRemoved;
    nullsRemoved.reserve( translation.size() );

    for( size_t i=0; i<translation.size(); i++ ) {
      NullListNode* nullNode = dynamic_cast<NullListNode*>(translation[i]);
      // we want to remove words that are stopwords (either query or index) because they'll cause this window not to match
      if( !(nullNode && nullNode->isStopword()) ) 
        nullsRemoved.push_back( translation[i] );
    }

    ListIteratorNode* unorderedNode = 0; 
    if( nullsRemoved.size() == 0 ) {
      unorderedNode = new NullListNode( bandNode->nodeName(), true );
      _network->addListNode( unorderedNode );
    } else if( nullsRemoved.size() == 1 ) {
      unorderedNode = nullsRemoved.front();
    } else {
      unorderedNode = new UnorderedWindowNode( bandNode->nodeName(), 
                                               nullsRemoved, -1 );
      _network->addListNode( unorderedNode );
    }
    _nodeMap[bandNode] = unorderedNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::FilRejNode* filRejNode ) {
  if( _nodeMap.find( filRejNode ) == _nodeMap.end() ) {
    InferenceNetworkNode* untypedFilter = _nodeMap[ filRejNode->getFilter() ];
    InferenceNetworkNode* untypedDisallowed = _nodeMap[ filRejNode->getDisallowed() ];
    BeliefNode* filterRejectNode = 0;
    
    if( untypedFilter ) {
      // normal case
      filterRejectNode = new FilterRejectNode( filRejNode->nodeName(),
                                               dynamic_cast<ListIteratorNode*>(untypedFilter),
                                               dynamic_cast<BeliefNode*>(untypedDisallowed) );
      _network->addBeliefNode( filterRejectNode );
    } else {
      // the filter term doesn't exist in the collection, so therefore, it never matches
      // therefore, we can promote the disallowed node up the chain
      filterRejectNode = dynamic_cast<BeliefNode*>(untypedDisallowed);
    }

    _nodeMap[filRejNode] = filterRejectNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::FilReqNode* filReqNode ) {
  if( _nodeMap.find( filReqNode ) == _nodeMap.end() ) {
    InferenceNetworkNode* untypedFilter = _nodeMap[ filReqNode->getFilter() ];
    InferenceNetworkNode* untypedRequired = _nodeMap[ filReqNode->getRequired() ];
    FilterRequireNode* filterRequireNode = 0;
    
    if( untypedFilter && untypedRequired ) {
      filterRequireNode = new FilterRequireNode( filReqNode->nodeName(),
                                                 dynamic_cast<ListIteratorNode*>(untypedFilter),
                                                 dynamic_cast<BeliefNode*>(untypedRequired) );
      _network->addBeliefNode( filterRequireNode );
    }

    _nodeMap[filReqNode] = filterRequireNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::FilterNode* fNode ) {
  if( _nodeMap.find( fNode ) == _nodeMap.end() ) {
    InferenceNetworkNode* untypedChild = _nodeMap[ fNode->getChild() ];
    FilterNode* filterNode = new FilterNode( fNode->nodeName(),
                                             dynamic_cast<BeliefNode*>(untypedChild),
                                             fNode->getDocuments() );

    _network->addBeliefNode( filterNode );
    _nodeMap[fNode] = filterNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::FieldLessNode* flNode ) {
  if( _nodeMap.find( flNode ) == _nodeMap.end() ) {
    InferenceNetworkNode* untypedFieldNode = _nodeMap[ flNode->getField() ];
    FieldLessNode* fieldLessNode = 0;
    
    if( untypedFieldNode ) {
      fieldLessNode = new FieldLessNode( flNode->nodeName(),
                                         dynamic_cast<FieldIteratorNode*>( untypedFieldNode ),
                                         flNode->getConstant() );
      _network->addListNode( fieldLessNode );
    }

    _nodeMap[flNode] = fieldLessNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::FieldGreaterNode* fgNode ) {
  if( _nodeMap.find( fgNode ) == _nodeMap.end() ) {
    InferenceNetworkNode* untypedFieldNode = _nodeMap[ fgNode->getField() ];
    FieldGreaterNode* fieldGreaterNode = 0;
    
    if( untypedFieldNode ) {
      fieldGreaterNode = new FieldGreaterNode( fgNode->nodeName(),
                                               dynamic_cast<FieldIteratorNode*>( untypedFieldNode ),
                                               fgNode->getConstant() );
      _network->addListNode( fieldGreaterNode );
    }

    _nodeMap[fgNode] = fieldGreaterNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::FieldBetweenNode* fbNode ) {
  if( _nodeMap.find( fbNode ) == _nodeMap.end() ) {
    InferenceNetworkNode* untypedFieldNode = _nodeMap[ fbNode->getField() ];
    FieldBetweenNode* fieldBetweenNode = 0;
      
    if( untypedFieldNode ) {
      fieldBetweenNode = new FieldBetweenNode( fbNode->nodeName(),
                                               dynamic_cast<FieldIteratorNode*>( untypedFieldNode ),
                                               fbNode->getLow(),
                                               fbNode->getHigh() );
      _network->addListNode( fieldBetweenNode );
    }

    _nodeMap[fbNode] = fieldBetweenNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::FieldEqualsNode* feNode ) {
  if( _nodeMap.find( feNode ) == _nodeMap.end() ) {
    InferenceNetworkNode* untypedFieldNode = _nodeMap[ feNode->getField() ];
    FieldEqualsNode* fieldEqualsNode = 0;
      
    if( untypedFieldNode ) {
      fieldEqualsNode = new FieldEqualsNode( feNode->nodeName(),
                                             dynamic_cast<FieldIteratorNode*>( untypedFieldNode ),
                                             feNode->getConstant() );
      _network->addListNode( fieldEqualsNode );
    }

    _nodeMap[feNode] = fieldEqualsNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ContextCounterNode* contextCounterNode ) {
  if( _nodeMap.find( contextCounterNode ) == _nodeMap.end() ) {
    InferenceNetworkNode* untypedRawExtent = _nodeMap[ contextCounterNode->getRawExtent() ];
    InferenceNetworkNode* untypedContext = _nodeMap[ contextCounterNode->getContext() ];
    ContextCountAccumulator* contextCount = 0;

    contextCount = new ContextCountAccumulator( contextCounterNode->nodeName(),
                                                dynamic_cast<ListIteratorNode*>(untypedRawExtent),
                                                dynamic_cast<ListIteratorNode*>(untypedContext) );

    _network->addEvaluatorNode( contextCount );
    _network->addComplexEvaluatorNode( contextCount );
    _nodeMap[ contextCounterNode ] = contextCount;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ContextSimpleCounterNode* contextSimpleCounterNode ) {
  if( _nodeMap.find( contextSimpleCounterNode ) == _nodeMap.end() ) {
    ContextSimpleCountAccumulator* contextCount = 0;

    contextCount = new ContextSimpleCountAccumulator( contextSimpleCounterNode->nodeName(),
                                                      contextSimpleCounterNode->terms(),
                                                      contextSimpleCounterNode->field(),
                                                      contextSimpleCounterNode->context() );

    _network->addEvaluatorNode( contextCount );
    _nodeMap[ contextSimpleCounterNode ] = contextCount;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ListAccumulator* accumulator ) {
  if( _nodeMap.find( accumulator ) == _nodeMap.end() ) {
    indri::infnet::ListAccumulator* accumulatorNode = 0;
    InferenceNetworkNode* untypedRawExtent = _nodeMap[ accumulator->getRawExtent() ];

    accumulatorNode = new indri::infnet::ListAccumulator( accumulator->nodeName(),
                                                          *dynamic_cast<ListIteratorNode*>(untypedRawExtent) );

    _network->addEvaluatorNode( accumulatorNode );
    _network->addComplexEvaluatorNode( accumulatorNode );
    _nodeMap[ accumulator ] = accumulatorNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ScoreAccumulatorNode* scoreAccumulatorNode ) {
  if( _nodeMap.find( scoreAccumulatorNode ) == _nodeMap.end() ) {
    indri::lang::Node* c = scoreAccumulatorNode->getChild();
    BeliefNode* child = dynamic_cast<BeliefNode*>(_nodeMap[c]);
    ScoredExtentAccumulator* accumulator = new ScoredExtentAccumulator( scoreAccumulatorNode->nodeName(), child, _resultsRequested );

    _network->addEvaluatorNode( accumulator );
    _network->addComplexEvaluatorNode( accumulator );
    _nodeMap[ scoreAccumulatorNode ] = accumulator;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::AnnotatorNode* annotatorNode ) {
  if( _nodeMap.find( annotatorNode ) == _nodeMap.end() ) {
    indri::lang::Node* c = annotatorNode->getChild();
    BeliefNode* child = dynamic_cast<BeliefNode*>(_nodeMap[c]);
    Annotator* annotator = new Annotator( annotatorNode->nodeName(), child );

    _network->addEvaluatorNode( annotator );
    _network->addComplexEvaluatorNode( annotator );
    _nodeMap[ annotatorNode ] = annotator;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::CachedFrequencyScorerNode* cachedScorerNode ) {
  // TODO: either remove this or fix it
  LEMUR_THROW( LEMUR_RUNTIME_ERROR, "For the time being, InferenceNetworkBuilder does not support CachedFrequencyScorerNodes" );
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::TermFrequencyScorerNode* termScorerNode ) {
  if( _nodeMap.find( termScorerNode ) == _nodeMap.end() ) {
    indri::infnet::BeliefNode* belief = 0;
    indri::query::TermScoreFunction* function = 0;

    function = _buildTermScoreFunction( termScorerNode->getSmoothing(),
                                        termScorerNode->getOccurrences(),
                                        termScorerNode->getContextSize(),
                                        termScorerNode->getDocumentOccurrences(),
                                        termScorerNode->getDocumentCount());

    if( termScorerNode->getOccurrences() > 0 ) {
      bool stopword = false;
      std::string processed = termScorerNode->getText();
      int termID = 0;
    
      // stem and stop the word
      if( termScorerNode->getStemmed() == false ) {
        processed = _repository.processTerm( termScorerNode->getText() );
        stopword = processed.length() == 0;
      }

      // if it isn't a stopword, we can try to get it from the index
      if( !stopword ) {
        int listID = _network->addDocIterator( processed );
        belief = new TermFrequencyBeliefNode( termScorerNode->nodeName(), *_network, listID, *function );
      }
    }

    // either there's no list here, or there aren't any occurrences
    // in the local collection, so just use a NullScorerNode in place
    if( !belief ) {
      belief = new NullScorerNode( termScorerNode->nodeName(), *function );
    }

    _network->addScoreFunction( function );
    _network->addBeliefNode( belief );
    _nodeMap[termScorerNode] = belief;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::RawScorerNode* rawScorerNode ) {
  indri::lang::NestedRawScorerNode * nested = dynamic_cast<indri::lang::NestedRawScorerNode*>(rawScorerNode);
  indri::lang::ShrinkageScorerNode * shrinkage = dynamic_cast<indri::lang::ShrinkageScorerNode*>(rawScorerNode);
  if ( nested ) {
    _after( nested );
  } else if ( shrinkage ) {
    _after( shrinkage );
  } else if( _nodeMap.find( rawScorerNode ) == _nodeMap.end() ) {
    BeliefNode* belief;
    InferenceNetworkNode* untypedRawExtentNode = _nodeMap[rawScorerNode->getRawExtent()];
    InferenceNetworkNode* untypedContextNode = _nodeMap[rawScorerNode->getContext()];
    ListIteratorNode* iterator = dynamic_cast<ListIteratorNode*>(untypedRawExtentNode);

    indri::query::TermScoreFunction* function = 0;

    function = _buildTermScoreFunction( rawScorerNode->getSmoothing(),
                                        rawScorerNode->getOccurrences(),
                                        rawScorerNode->getContextSize(),
                                        rawScorerNode->getDocumentOccurrences(),
                                        rawScorerNode->getDocumentCount() );
    
    if( rawScorerNode->getOccurrences() > 0 && iterator != 0 ) {
      ListIteratorNode* rawIterator = 0;
      ListIteratorNode* context = dynamic_cast<ListIteratorNode*>(untypedContextNode);

      if( context ) {
        rawIterator = iterator;
        iterator = new ExtentInsideNode( "", rawIterator, context );
        _network->addListNode( iterator );
      }
      
      // this is here to turn max-score off for this term
      // only frequency lists are "max-scored"
      double maximumScore = INDRI_HUGE_SCORE;
      double maximumBackgroundScore = INDRI_HUGE_SCORE;

      // We need to use the NestedListBeliefNode if there is a field in the rawIterator or below it
      indri::lang::FieldBelowWalker fieldFinder;
      rawScorerNode->getRawExtent()->walk( fieldFinder );
      if ( fieldFinder.fieldBelow() ) {      
        belief = new NestedListBeliefNode( rawScorerNode->nodeName(), *iterator, context, rawIterator, *function, maximumBackgroundScore, maximumScore );
      } else {
        belief = new ListBeliefNode( rawScorerNode->nodeName(), *iterator, context, rawIterator, *function, maximumBackgroundScore, maximumScore );
      }
    } else {
      belief = new NullScorerNode( rawScorerNode->nodeName(), *function );
    }

    _network->addScoreFunction( function );
    _network->addBeliefNode( belief );
    _nodeMap[rawScorerNode] = belief;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::PriorNode* pNode ) {
  if( _nodeMap.find( pNode ) == _nodeMap.end() ) {
    PriorNode* priorNode = new PriorNode( pNode->nodeName(), *_network, _network->addPriorIterator( pNode->getPriorName() ) );
    
    _network->addBeliefNode( priorNode );
    _nodeMap[pNode] = priorNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::WeightNode* weightNode ) {
  if( _nodeMap.find( weightNode ) == _nodeMap.end() ) {
    const std::vector< std::pair<double, indri::lang::ScoredExtentNode*> >& children = weightNode->getChildren();
    WeightedAndNode* wandNode = new WeightedAndNode( weightNode->nodeName() );
    // normalize over absolute values
    double totalWeights = 0;
    for( size_t i=0; i<children.size(); i++ ) {
      totalWeights += fabs(children[i].first);
    }

    for( size_t i=0; i<children.size(); i++ ) {
      wandNode->addChild( children[i].first / totalWeights,
                          dynamic_cast<BeliefNode*>( _nodeMap[children[i].second] ) );
    }

    wandNode->doneAddingChildren();

    _network->addBeliefNode( wandNode );
    _nodeMap[weightNode] = wandNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::WSumNode* wsumNode ) {
  if( _nodeMap.find( wsumNode ) == _nodeMap.end() ) {
    const std::vector< std::pair<double, indri::lang::ScoredExtentNode*> >& children = wsumNode->getChildren();
    WeightedSumNode* weightedSumNode = new WeightedSumNode( wsumNode->nodeName() );
    // normalize over absolute values.
    double totalWeights = 0;
    for( size_t i=0; i<children.size(); i++ ) {
      totalWeights += fabs(children[i].first);
    }

    for( size_t i=0; i<children.size(); i++ ) {
      weightedSumNode->addChild( children[i].first / totalWeights,
                                 dynamic_cast<BeliefNode*>( _nodeMap[children[i].second] ) );
    }

    _network->addBeliefNode( weightedSumNode );
    _nodeMap[wsumNode] = weightedSumNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::WAndNode* wandNode ) {
  if( _nodeMap.find( wandNode ) == _nodeMap.end() ) {
    const std::vector< std::pair<double, indri::lang::ScoredExtentNode*> >& children = wandNode->getChildren();
    WeightedAndNode* weightedAndNode = new WeightedAndNode( wandNode->nodeName() );
    // normalize over absolute values
    double totalWeights = 0;
    for( size_t i=0; i<children.size(); i++ ) {
      totalWeights += fabs(children[i].first);
    }

    for( size_t i=0; i<children.size(); i++ ) {
      weightedAndNode->addChild( children[i].first / totalWeights,
                                 dynamic_cast<BeliefNode*>( _nodeMap[children[i].second] ) );
    }

    weightedAndNode->doneAddingChildren();

    _network->addBeliefNode( weightedAndNode );
    _nodeMap[wandNode] = weightedAndNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::OrNode* orSpecNode ) {
  if( _nodeMap.find( orSpecNode ) == _nodeMap.end() ) {
    std::vector<BeliefNode*> translation = _translate<BeliefNode>( orSpecNode->getChildren() );
    OrNode* orNode = new OrNode( orSpecNode->nodeName(), translation );

    _network->addBeliefNode( orNode );
    _nodeMap[orSpecNode] = orNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::NotNode* notSpecNode ) {
  if( _nodeMap.find( notSpecNode ) == _nodeMap.end() ) {
    BeliefNode* child = dynamic_cast<BeliefNode*>( _nodeMap[notSpecNode->getChild()] );
    NotNode* notNode = new NotNode( notSpecNode->nodeName(), child );

    _network->addBeliefNode( notNode );
    _nodeMap[notSpecNode] = notNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::MaxNode* maxSpecNode ) {
  if( _nodeMap.find( maxSpecNode ) == _nodeMap.end() ) {
    std::vector<BeliefNode*> translation = _translate<BeliefNode>( maxSpecNode->getChildren() );
    MaxNode* maxNode = new MaxNode( maxSpecNode->nodeName(), translation );

    _network->addBeliefNode( maxNode );
    _nodeMap[maxSpecNode] = maxNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::CombineNode* combineNode ) {
  if( _nodeMap.find( combineNode ) == _nodeMap.end() ) {
    const std::vector<indri::lang::ScoredExtentNode*>& children = combineNode->getChildren();
    double weight = 1. / double(children.size());

    std::vector<BeliefNode*> translation = _translate<BeliefNode,indri::lang::ScoredExtentNode>( children );
    WeightedAndNode* wandNode = new WeightedAndNode( combineNode->nodeName() );

    for( size_t i=0; i<children.size(); i++ ) {
      wandNode->addChild( weight, translation[i] );
    }

    wandNode->doneAddingChildren();

    _network->addBeliefNode( wandNode );
    _nodeMap[combineNode] = wandNode;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::PlusNode* plusNode ) {
  if( _nodeMap.find( plusNode ) == _nodeMap.end() ) {
    const std::vector<indri::lang::ScoredExtentNode*>& children = plusNode->getChildren();
    std::vector<BeliefNode*> translation = _translate<BeliefNode,indri::lang::ScoredExtentNode>( children );
    PlusNode* plus = new PlusNode( plusNode->nodeName(), translation );
    _network->addBeliefNode( plus );
    _nodeMap[plusNode] = plus;
  }
}

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::WPlusNode* plusNode ) {
  if( _nodeMap.find( plusNode ) == _nodeMap.end() ) {
    const std::vector< std::pair<double, indri::lang::ScoredExtentNode*> >& children = plusNode->getChildren();
    WPlusNode* plus = new WPlusNode( plusNode->nodeName() );
    for( size_t i=0; i<children.size(); i++ ) {
    plus->addChild( children[i].first,
                    dynamic_cast<BeliefNode*>( _nodeMap[children[i].second] ));
    }
    _network->addBeliefNode( plus );
    _nodeMap[plusNode] = plus;
  }
}


//
// FieldWildcard
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::FieldWildcard* fieldWildcard ) {
  if( _nodeMap.find( fieldWildcard ) == _nodeMap.end() ) {
    indri::infnet::FieldWildcardNode* fieldWildcardNode = 
      new indri::infnet::FieldWildcardNode( fieldWildcard->nodeName() );

    _network->addListNode( fieldWildcardNode );
    _nodeMap[fieldWildcard] = fieldWildcardNode;
  }
}


//
// NestedExtentInside
//

void indri::infnet::InferenceNetworkBuilder::_after( indri::lang::NestedExtentInside* extentInside ) {
  if( _nodeMap.find( extentInside ) == _nodeMap.end() ) {
    NestedExtentInsideNode* extentInsideNode = new NestedExtentInsideNode( 
                                                                          extentInside->nodeName(),
                                                                          dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getInner()]),
                                                                          dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getOuter()]) );

    _network->addListNode( extentInsideNode );
    _nodeMap[extentInside] = extentInsideNode;
  }
}


//
// NestedRawScorer
//

void indri::infnet::InferenceNetworkBuilder::_after( indri::lang::NestedRawScorerNode* rawScorerNode ) {
  if( _nodeMap.find( rawScorerNode ) == _nodeMap.end() ) {
    BeliefNode* belief;
    InferenceNetworkNode* untypedRawExtentNode = _nodeMap[rawScorerNode->getRawExtent()];
    InferenceNetworkNode* untypedContextNode = _nodeMap[rawScorerNode->getContext()];
    ListIteratorNode* iterator = dynamic_cast<ListIteratorNode*>(untypedRawExtentNode);

    indri::query::TermScoreFunction* function = 0;

    function = _buildTermScoreFunction( rawScorerNode->getSmoothing(),
                                        rawScorerNode->getOccurrences(),
                                        rawScorerNode->getContextSize(),
                                        rawScorerNode->getDocumentOccurrences(),
                                        rawScorerNode->getDocumentCount() );

    if( rawScorerNode->getOccurrences() > 0 && iterator != 0 ) {
      ListIteratorNode* rawIterator = 0;
      ListIteratorNode* context = dynamic_cast<ListIteratorNode*>(untypedContextNode);

      if( context ) {
        rawIterator = iterator;
        iterator = new ExtentInsideNode( "", rawIterator, context );
        _network->addListNode( iterator );
      }
      
      // this is here to turn max-score off for this term
      // only frequency lists are "max-scored"
      double maximumScore = INDRI_HUGE_SCORE;
      double maximumBackgroundScore = INDRI_HUGE_SCORE;
      
      belief = new NestedListBeliefNode( rawScorerNode->nodeName(), *iterator, context, rawIterator, *function, maximumBackgroundScore, maximumScore );
    } else {
      belief = new NullScorerNode( rawScorerNode->nodeName(), *function );
    }

    _network->addScoreFunction( function );
    _network->addBeliefNode( belief );
    _nodeMap[rawScorerNode] = belief;
  }
}

//
// ExtentEnforcement
//

void indri::infnet::InferenceNetworkBuilder::_after( indri::lang::ExtentEnforcement* eeNode ) {
  if( _nodeMap.find( eeNode ) == _nodeMap.end() ) {
    indri::infnet::BeliefNode* childNode = dynamic_cast<indri::infnet::BeliefNode*>(_nodeMap[eeNode->getChild()]);
    indri::infnet::ListIteratorNode* fieldNode = dynamic_cast<indri::infnet::ListIteratorNode*>(_nodeMap[eeNode->getField()]);
    indri::infnet::ExtentEnforcementNode* extentEnforcement = new indri::infnet::ExtentEnforcementNode( eeNode->nodeName(), childNode, fieldNode );
    _network->addBeliefNode( extentEnforcement );    
    _nodeMap[eeNode] = extentEnforcement;
  }
}

//
// ContextInclusion
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ContextInclusionNode* contextInclusionNode ) {
  if( _nodeMap.find( contextInclusionNode ) == _nodeMap.end() ) {
    const std::vector<indri::lang::ScoredExtentNode*>& children = contextInclusionNode->getChildren();
    double weight = 1. / double(children.size());

    std::vector<BeliefNode*> translation = _translate<BeliefNode,indri::lang::ScoredExtentNode>( children );
    indri::infnet::ContextInclusionAndNode* ciaNode = new indri::infnet::ContextInclusionAndNode( contextInclusionNode->nodeName() );

    indri::lang::ScoredExtentNode* preserveExtentsChild = contextInclusionNode->getPreserveExtentsChild();
    for( size_t i=0; i<children.size(); i++ ) {
      bool preserveExtents = false;
      if (children[i] == preserveExtentsChild) {
        preserveExtents = true;
      }
      ciaNode->addChild( weight, translation[i], preserveExtents );
    }

    ciaNode->doneAddingChildren();

    _network->addBeliefNode( ciaNode );
    _nodeMap[contextInclusionNode] = ciaNode;
  }
}

//
// LengthPrior
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::LengthPrior* lengthPrior ) {
  if( _nodeMap.find( lengthPrior ) == _nodeMap.end() ) {

    BeliefNode * child = dynamic_cast<BeliefNode*>(_nodeMap[lengthPrior->getChild()]);

    indri::infnet::LengthPriorNode * lengthPriorNode = 
      new indri::infnet::LengthPriorNode( lengthPrior->nodeName(), child, lengthPrior->getExponent() );

    _network->addBeliefNode( lengthPriorNode );
    _nodeMap[lengthPrior] = lengthPriorNode;
  }
}


//
// DocumentStructureNode
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::DocumentStructureNode* docStructNode ) {
  if( _nodeMap.find( docStructNode ) == _nodeMap.end() ) {
    indri::infnet::DocumentStructureHolderNode* docStructHolderNode = 
      new indri::infnet::DocumentStructureHolderNode( docStructNode->nodeName() );

    _network->addDocumentStructureHolderNode( docStructHolderNode );
    _nodeMap[docStructNode] = docStructHolderNode;
  }
}

//
// ShrinkageScorer
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ShrinkageScorerNode* shrinkScorerNode ) {
  _after( shrinkScorerNode );
}

void indri::infnet::InferenceNetworkBuilder::_after( indri::lang::ShrinkageScorerNode* shrinkScorerNode ) {
  if( _nodeMap.find( shrinkScorerNode ) == _nodeMap.end() ) {
    BeliefNode* belief;
    InferenceNetworkNode* untypedShrinkExtentNode = _nodeMap[shrinkScorerNode->getRawExtent()];
    InferenceNetworkNode* untypedContextNode = _nodeMap[shrinkScorerNode->getContext()];
    ListIteratorNode* iterator = dynamic_cast<ListIteratorNode*>(untypedShrinkExtentNode);

    indri::query::TermScoreFunction* function = 0;

    function = _buildTermScoreFunction( shrinkScorerNode->getSmoothing(),
                                        shrinkScorerNode->getOccurrences(),
                                        shrinkScorerNode->getContextSize(),
                                        0, 0);

    if( shrinkScorerNode->getOccurrences() > 0 && iterator != 0 ) {
      
      DocumentStructureHolderNode* docStruct = dynamic_cast<DocumentStructureHolderNode*>(_nodeMap[shrinkScorerNode->getDocumentStructure()]);
      
      // this is here to turn max-score off for this term
      // only frequency lists are "max-scored"
      double maximumScore = INDRI_HUGE_SCORE;
      double maximumBackgroundScore = INDRI_HUGE_SCORE;
      
      ShrinkageBeliefNode * shrinkageBelief = new ShrinkageBeliefNode( shrinkScorerNode->nodeName(), *iterator, *docStruct, *function, maximumBackgroundScore, maximumScore );
      belief = shrinkageBelief;
      std::vector<std::string> shrinkageRules = shrinkScorerNode->getShrinkageRules();
      std::vector<std::string>::iterator ruleIter = shrinkageRules.begin();
      while( ruleIter != shrinkageRules.end() ) {
        shrinkageBelief->addShrinkageRule( *ruleIter );
        ruleIter++;
      }

    } else {
      belief = new NullScorerNode( shrinkScorerNode->nodeName(), *function );
    }

    _network->addScoreFunction( function );
    _network->addBeliefNode( belief );
    _nodeMap[shrinkScorerNode] = belief;
  }
}

//
// ExtentDescendant
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ExtentDescendant* extentInside ) {
  _after( extentInside );
}

void indri::infnet::InferenceNetworkBuilder::_after( indri::lang::ExtentDescendant* extentInside ) {
  if( _nodeMap.find( extentInside ) == _nodeMap.end() ) {
   
    DocumentStructureHolderNode* docStruct = dynamic_cast<DocumentStructureHolderNode*>(_nodeMap[extentInside->getDocumentStructure()]);

    ExtentDescendantNode* extentInsideNode = new ExtentDescendantNode( 
                                                                      extentInside->nodeName(),
                                                                      dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getInner()]),
                                                                      dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getOuter()]),
                                                                      *docStruct );

    _network->addListNode( extentInsideNode );
    _nodeMap[extentInside] = extentInsideNode;
  }
}


//
// ExtentChild
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ExtentChild* extentInside ) {
  _after( extentInside );
}

void indri::infnet::InferenceNetworkBuilder::_after( indri::lang::ExtentChild* extentInside ) {
  if( _nodeMap.find( extentInside ) == _nodeMap.end() ) {

    DocumentStructureHolderNode* docStruct = dynamic_cast<DocumentStructureHolderNode*>(_nodeMap[extentInside->getDocumentStructure()]);
   
    ExtentChildNode* extentInsideNode = new ExtentChildNode( 
                                                            extentInside->nodeName(),
                                                            dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getInner()]),
                                                            dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getOuter()]),
                                                            *docStruct );

    _network->addListNode( extentInsideNode );
    _nodeMap[extentInside] = extentInsideNode;
  }
}


//
// ExtentParent
//

void indri::infnet::InferenceNetworkBuilder::after( indri::lang::ExtentParent* extentInside ) {
  _after( extentInside );
}

void indri::infnet::InferenceNetworkBuilder::_after( indri::lang::ExtentParent* extentInside ) {
  if( _nodeMap.find( extentInside ) == _nodeMap.end() ) {

    DocumentStructureHolderNode* docStruct = dynamic_cast<DocumentStructureHolderNode*>(_nodeMap[extentInside->getDocumentStructure()]);
   
    ExtentParentNode* extentInsideNode = new ExtentParentNode( 
                                                              extentInside->nodeName(),
                                                              dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getInner()]),
                                                              dynamic_cast<ListIteratorNode*>(_nodeMap[extentInside->getOuter()]),
                                                              *docStruct );

    _network->addListNode( extentInsideNode );
    _nodeMap[extentInside] = extentInsideNode;
  }
}

//
// WildcardTerm
//
void indri::infnet::InferenceNetworkBuilder::after( indri::lang::WildcardTerm* wildcardTerm ) {
  if (_nodeMap.find(wildcardTerm)==_nodeMap.end()) {
    // we will need to create an ExtentOr for our wildcard terms...
    // (acts like a #syn operator)

    // but first! we need to get a vector
    // for the children...
    std::vector<ListIteratorNode*> wildcardChildren;

    // fill the extent or with IndexTerms from the repository
    // that match our wildcard spec...
    indri::collection::Repository::index_state theIndexes = _repository.indexes();
    size_t numIndexes=theIndexes->size();

    std::string normalizedTerm=wildcardTerm->getTerm();
                
    for (size_t i=0; i < numIndexes; i++) {
      // get the index.
      indri::index::Index* thisIndex = (*theIndexes)[i];
      indri::thread::ScopedLock iterators( thisIndex->iteratorLock() );

      // for each index in the repository...
      // get a the vocabulary iterator from the index...
      indri::index::VocabularyIterator *vIter=thisIndex->vocabularyIterator();

      if (vIter) {
        vIter->startIteration();

        // get the next entry that corresponds to our term
        while (vIter->nextEntry(normalizedTerm)) {

          // get the term
          indri::index::DiskTermData* entry = vIter->currentEntry();
          if (entry) {
            indri::index::TermData* termData = entry->termData;

            if (strstr(termData->term, normalizedTerm.c_str())==termData->term) {
              if (wildcardChildren.size()==_maxWildcardTerms) {

                // be sure to delete the vocabulary iterator when we're done!
                delete vIter; 
                //replace with stringstream.
                std::stringstream  maxTermExString;
                maxTermExString <<  "Error in parsing wildcard terms. Too many terms matched " << normalizedTerm << "*. Limit is " << _maxWildcardTerms << "." ;
                LEMUR_THROW( LEMUR_PARSE_ERROR, maxTermExString.str());

              } // end if (wildcardChildren.size()==_maxWildcardTerms)

              // OK - we have the term... create what amounts to an indexterm...
              // and add it...

              int listID = _network->addDocIterator(termData->term);
              indri::infnet::DocListIteratorNode *iteratorNode=new DocListIteratorNode("wildcard_child:"+std::string(termData->term), *_network, listID);
              wildcardChildren.push_back(iteratorNode);
              _network->addListNode(iteratorNode);

            } // end if (strstr(termData->term, _normalizedTerm.c_str())==termData->term)
          } // end if (entry)
        } // end while (vIter->nextEntry(normalizedTerm))

        // be sure to delete the vocabulary iterator when we're done!
        delete vIter;

      } // end if (vIter)
      iterators.unlock();

    } // end for (int i=0; i < numIndexes; i++)

    // ok - now we can create our ExtentOrNode...
    indri::infnet::ExtentOrNode* wildcardNode = new indri::infnet::ExtentOrNode( wildcardTerm->nodeName(), wildcardChildren );

    // and finally, add it to our network...
    _network->addListNode( wildcardNode );
    _nodeMap[wildcardTerm] = wildcardNode;

  }  // end if (_nodeMap.find(wildcardTerm)==_nodeMap.end())
}

