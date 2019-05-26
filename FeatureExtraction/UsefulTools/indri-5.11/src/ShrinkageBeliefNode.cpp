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
// ShrinkageBeliefNode
//
// 29 July 2005 -- pto
//

#include "indri/ShrinkageBeliefNode.hpp"
#include "lemur/lemur-compat.hpp"
#include "indri/Annotator.hpp"
#include "indri/Parameters.hpp"
#include <cmath>
#ifdef ISNAN_IN_NAMESPACE_STD
using std::isnan;
#else
#ifdef ISNAN_IN_NAMESPACE_GNU_CXX
using __gnu_cxx::isnan;
#endif
#endif
#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif

indri::infnet::ShrinkageBeliefNode::ShrinkageBeliefNode( const std::string& name, ListIteratorNode& child, DocumentStructureHolderNode& documentStructureHolderNode, indri::query::TermScoreFunction& scoreFunction, double maximumBackgroundScore, double maximumScore )
  :
  _name(name),
  _scoreFunction(scoreFunction),
  _maximumScore(maximumScore),
  _maximumBackgroundScore(maximumBackgroundScore),
  _list(child),
  _docStructHolder(documentStructureHolderNode),
  _down(),
  _up(),
  _base(),
  _counts(),
  _roots(),
  _topDownOrder(),
  _documentID(0),
  _parentWeight(0),
  _docWeight(0),
  _otherWeight(0),
  _defaultScore(0),
  _recursive(false),
  _queryLevelCombine(false)
{

  _maximumScore = INDRI_HUGE_SCORE;
}

lemur::api::DOCID_T indri::infnet::ShrinkageBeliefNode::nextCandidateDocument() {
  return _list.nextCandidateDocument();
}

double indri::infnet::ShrinkageBeliefNode::maximumBackgroundScore() {
  return _maximumBackgroundScore;
}

double indri::infnet::ShrinkageBeliefNode::maximumScore() {
  return _maximumScore;
}

const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& indri::infnet::ShrinkageBeliefNode::score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
  indri::index::DocumentStructure * docStruct = _docStructHolder.getDocumentStructure();
  int numNodes = docStruct->nodeCount();

  _buildScoreCache( documentID );

  _results.clear();

  // Find the appropriate result.  We can't actually know which of the excatly annotations
  // were requested, so we are forced to average these probabilities for now.
  
  double score = 0;
  double matched = 0;

  std::set<int> leafs;
  if ( extent.ordinal == 0 ) {
    docStruct->findLeafs( &leafs, extent.begin, extent.end, true );
  } else {
    leafs.insert( extent.ordinal );
  }
  if ( leafs.size() == 0 ) {
    // can't find exact matches, so approximate instead
    docStruct->findLeafs( &leafs, extent.begin, extent.end, false );
  }
  std::set<int>::iterator leaf = leafs.begin();
  std::set<int>::iterator leafsEnd = leafs.end();
  while ( leaf != leafsEnd ) {
    // output for debugging
    //        std::cout << _list.getName() << "\t\t    leaf: " << docStruct->path(*leaf) << " " << extent.begin << ":" << extent.end
    //            << "    count: " << _counts[*leaf] 
    //                    << "    score: " << _down[*leaf] 
    //            << "    length: " << docStruct->accumulatedLength(*leaf)
    //            << "    coll: " << _scoreFunction.scoreOccurrence( 0, docStruct->accumulatedLength(*leaf) )
    //               << "    parent: " << _down[docStruct->parent(*leaf)] 
    //            << std::endl;    
    if ( _down[ *leaf ] != 0 ) {
      score += _down[ *leaf ];
      matched++;    
    }
    leaf++;
  }

  if ( matched > 0 ) {
    score /= matched;
    if ( !_queryLevelCombine ) {
      score = log( score );
    }
    //     std::cout << "\t" << getName() << " " << extent.begin << ":" << extent.end << " " << score << std::endl;

    indri::api::ScoredExtentResult result(extent);
    result.score=score;
    result.document=documentID;
    _results.push_back(result);
  } else {    
    score = _defaultScore;
    if ( !_queryLevelCombine ) {
      score = log( score );
    }
    //       std::cout << _list.getName() << "\t\t    leaf: ? " << extent.begin << ":" << extent.end 
    //            << "    score: " << _defaultScore
    //            << std::endl;    
    //     std::cout << "\t" << getName() << " " << extent.begin << ":" << extent.end << " " << score << " " << "Default!" << std::endl;

    indri::api::ScoredExtentResult result(extent);
    result.score=score;
    result.document=documentID;
    _results.push_back(result);
  }
  return _results;
}

void indri::infnet::ShrinkageBeliefNode::annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {

 
  const indri::utility::greedy_vector<indri::index::Extent>& extents = _list.extents();
  int count = 0;


  // mark the begin and end points for this list
  for( size_t i=0; i<extents.size(); i++ ) {
    if( extents[i].begin >= extent.begin &&
        extents[i].end <= extent.end ) {
      annotator.add( this, documentID, (indri::index::Extent &)extents[i] );
      _list.annotate( annotator, documentID, (indri::index::Extent &)extents[i] );
    }
  }
}

bool indri::infnet::ShrinkageBeliefNode::hasMatch( lemur::api::DOCID_T documentID ) {
  return _list.extents().size() > 0;
}


void indri::infnet::ShrinkageBeliefNode::_buildScoreCache( lemur::api::DOCID_T documentID ) {
  indri::index::DocumentStructure * docStruct = _docStructHolder.getDocumentStructure();
  int numNodes = docStruct->nodeCount();

  int documentLength = docStruct->getIndex()->documentLength( documentID );

  // build a new cache of scores if needed
  if ( documentID != _documentID ) {

    _counts.clear();
    _base.clear();
    _up.clear();
    _down.clear();

    _counts.resize( numNodes+1, 0 );
    _base.resize( numNodes+1, 0 );
    _up.resize( numNodes+1, 0 );
    _down.resize( numNodes+1, 0 );
  
    // Count up the occurrences terms in each tree.
    // We include all child components - this is a break from the model in my proposal, and is 
    // to allow occurrences of phrases that cross field boundaries.  Otherwise, it is
    // an equivalent reformulation of the original model, but the parameters learned/chosen
    // would be different.

    const indri::utility::greedy_vector<indri::index::Extent> & extents = _list.extents();
    // std::cout << "got list " << documentID << std::endl;

    int lastEnd = 0;
    

    // keep track of the roots we need to work with for walking    
    _roots.clear();


    // Walk through the extent list.
    // As we encounter a new term in the extent list:
    // - add new extents to an active node list from the document nodes that have the same begin or less
    // - remove extents from the active node list where the end is less then the begin of the term extent
    // Scan the active node list for extents that contains the term extent.

    // Sort the active node list by increasing end.
    // - When removing, the extents to remove will be at the beginning
    // - When scanning, check the from the end. All nodes that have a larger or equal end to the term
    //   extent's end should have their count increased

    // active document nodes
    std::set<indri::index::Extent, indri::index::Extent::ends_before_less> activeOuterExtents;
    
    int docNode = 1;

    indri::utility::greedy_vector<indri::index::Extent>::const_iterator innerIter = extents.begin();
    while ( innerIter != extents.end() ) {
      // remove outer extents we don't need anymore
      std::set<indri::index::Extent, indri::index::Extent::ends_before_less>::iterator activeIter = activeOuterExtents.begin();
      std::set<indri::index::Extent, indri::index::Extent::ends_before_less>::iterator activeEnd = activeOuterExtents.end();
      while ( activeIter != activeEnd ) {
        if ( activeIter->end >= innerIter->begin ) {
          break;
        } 
        activeIter++;
      }
      activeOuterExtents.erase( activeOuterExtents.begin(), activeIter );         

      // push new document node extents on that we may need
      while ( docNode <= numNodes && docStruct->begin( docNode ) <= innerIter->begin ) {      
        indri::index::Extent extent( 1.0, docStruct->begin( docNode ), docStruct->end( docNode ), docNode );
        // only insert if needed
        if ( extent.end >= innerIter->begin ) {
          activeOuterExtents.insert( extent );
        }
        docNode++;
      }

      if ( innerIter->begin >= lastEnd ) {
        _counts[0] += innerIter->weight;
        // scan the doc nodes for nodes that match
        activeIter = activeOuterExtents.end();
        if (!activeOuterExtents.empty()) {
          activeIter--;
          bool doneActiveList = false;
          while ( ! doneActiveList &&
                  activeIter->end >= innerIter->end ) {
            // Since we know that all active doc node extents have a begin that is at or before
            // the inner iter's begin, and from the if statement we know the end of one
            // of the active outer extents is at least 
            // as large as the inner end, we know the inner iter extent is contained
            // by this extent in the active list 
            _counts[ activeIter->ordinal ] += ( activeIter->weight * innerIter->weight );

            // keep track of roots we've seen
            int r = activeIter->ordinal;
            while ( docStruct->parent( r ) != 0 ) {
              r = docStruct->parent( r );
            }
            if ( _roots.find( r ) == _roots.end() ) {
              _roots.insert( r );
            }

            if ( activeIter == activeOuterExtents.begin() ) {
              doneActiveList = true;
            } else {
              activeIter--;
            }
          }
        }
        lastEnd = innerIter->end;
      }
      innerIter++;
    }

    _base[ 0 ] = _scoreFunction.scoreOccurrence( _counts[0], documentLength, _counts[0], documentLength );    

    // weight given to smoothing in linear interpolation
    double otherScore = 0;

    _defaultScore = _scoreFunction.scoreOccurrence( 0, 0, _counts[0], documentLength );
    if ( !_queryLevelCombine ) {
      _base [ 0 ] = exp( _base[0] );
      _defaultScore = exp( _defaultScore );
    }
    otherScore = _defaultScore;
    _defaultScore = _docWeight * _base[0] + (1 - _docWeight) * _defaultScore;

    docStruct->topDownOrder( _roots, _topDownOrder );
    indri::utility::greedy_vector<int>::iterator nodesBegin = _topDownOrder.begin();
    indri::utility::greedy_vector<int>::iterator nodesEnd = _topDownOrder.end();

    // estimate scores for each of the nodes ( \theta mixed with collection model )
    indri::utility::greedy_vector<int>::iterator node = nodesBegin;
    while (node < nodesEnd) {
      int i = *node;
      int contextSize = docStruct->accumulatedLength(i);
      double occurrences = _counts[i];
      double score = _scoreFunction.scoreOccurrence( occurrences, contextSize, _counts[0], documentLength );
      if ( !_queryLevelCombine ) {
        // must subtract weight given to collection/document so that we can do the shrinkage properly
        score = (exp( score ) - otherScore) / (1 - _otherWeight) ;
      } 
      _base[i] = score;
      node++;
    }

    // std::cout << "done base " << documentID << std::endl;

    // smooth up doc 
    node = nodesEnd;
    while (node != nodesBegin) {
      node--;
      int i = *node;

      // what's left for the original model and the length proportional models
      double remaining = 1;
      
      // for summing probabilities with absolute weights
      double absolute = 0;

      // for summing probabilites with length proportional weights
      int length = docStruct->accumulatedLength( i );
      double divisor = length;
      double relative = _base[ i ] * length;

      if ( !_ruleMap.empty() ) {

        indri::index::DocumentStructure::child_iterator kids = docStruct->childrenBegin( i );
        indri::index::DocumentStructure::child_iterator kidsEnd = docStruct->childrenEnd( i );      
        while( kids < kidsEnd ) {
          // check for a smoothing rule for the child
          int kidType = docStruct->type(*kids);
          std::map<int, smoothing_rule>::iterator ruleIter = _ruleMap.find(kidType);
          if( ruleIter != _ruleMap.end() ) {
            smoothing_rule rule = ruleIter->second;
            if( rule.lengthProportional ) {
              double lengthAlpha = rule.weight * docStruct->accumulatedLength( *kids );
              if( _recursive ) {
                relative += lengthAlpha * _up[ *kids ];
              } else {
                relative += lengthAlpha * _base[ *kids ];
              }
              divisor += lengthAlpha;
            } else {
              if( _recursive ) {
                absolute += rule.weight * _up[ *kids ];
              } else {
                absolute += rule.weight * _base[ *kids ];
              }
              remaining -= rule.weight;
            }
          }
          kids++;
        }
      }

      relative /= divisor;
      
      if ( ! isnan( relative ) ) {
        if ( remaining >= 0 ) {
          _up[ i ] = remaining * relative + absolute;
        } else {
          // the absolute weights sum to something larger than 1, so we will ignore these 
          _up[ i ] = relative;
        }
      } else {
        // the divisor was 0, so we can't use the relative weights
        if ( remaining >= 0 ) {
          _up[ i ] = remaining * _base[ i ] + absolute;
        } else {
          // the absolute weights sum to something larger than 1, so we will ignore these 
          _up[ i ] = _base[ i ];
        }
      }

    }
    // std::cout << "done up " << documentID << std::endl;

    // smooth down the each tree
    _up[0] = _base[0];
    _down[0] = _up[0];
    node = nodesBegin;    
    while ( node != nodesEnd ) {
      int i = *node;
      if ( docStruct->parent( i ) == 0 ) {
        if ( _recursive ) {
          _down[i] = (1.0 - _docWeight) * _up[i] 
            + _docWeight * _down[0];
        } else {
          _down[i] = (1.0 - _docWeight) * _up[i] 
            + _docWeight * _up[0];
        }         
      } else {      
        if( _recursive ) {
          _down[i] = (1.0 - _parentWeight - _docWeight) * _up[i] 
            + _parentWeight * _down[docStruct->parent(i)]
            + _docWeight * _down[0];
        } else {
          _down[i] = (1.0 - _parentWeight - _docWeight) * _up[i]
            + _parentWeight * _up[docStruct->parent(i)]
            + _docWeight * _up[0];
        }
      }
      node++;
    }

    // add back in the collection/doc weight
    if ( ! _queryLevelCombine ) {
      node = nodesBegin;
      while ( node != nodesEnd ) {
        size_t i = *node;
        _down[i] = (1 - _otherWeight) * _down[i] + otherScore;
        node++;
      }
    }
    // std::cout << "done down " << documentID << std::endl;
        
    _documentID = documentID;

    //     std::cout << "done smoothing " << documentID <<std::endl;

  }

}

const indri::utility::greedy_vector<bool>& indri::infnet::ShrinkageBeliefNode::hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& matchExtents ) {
  //  std::cout << this << " Matching " << documentID << std::endl;
  _matches.clear();
  // Allows matches elsewhere in the document as shrinkage may give a reasonably large belief
  _matches.resize( matchExtents.size(), false );

  indri::index::DocumentStructure * docStruct = _docStructHolder.getDocumentStructure();

  _buildScoreCache( documentID );

  for ( size_t i = 0 ; i < matchExtents.size() ; i++ ) {
    const indri::index::Extent * extent =  &(matchExtents[i]);
    std::set<int> leafs;
    if ( extent->ordinal == 0 ) {
      docStruct->findLeafs( &leafs, extent->begin, extent->end, true );
    } else {
      leafs.insert( extent->ordinal );
    }
    if ( leafs.size() == 0 ) {
      // can't find exact matches, so approximate instead
      docStruct->findLeafs( &leafs, extent->begin, extent->end, false );
    }
    std::set<int>::iterator leaf = leafs.begin();
    std::set<int>::iterator leafsEnd = leafs.end();
    while ( _matches[ i ] == false && leaf != leafsEnd ) {
      if ( _down[ *leaf ] != 0 ) {
        _matches[ i ] = true;
      }
      leaf++;
    }
  }
  return _matches;
}

const std::string& indri::infnet::ShrinkageBeliefNode::getName() const {
  return _name;
}

void indri::infnet::ShrinkageBeliefNode::indexChanged( indri::index::Index& index ) {

  _ruleMap.clear();
  
  std::set<smoothing_rule, lt_rule>::iterator ruleIter = _ruleSet.begin();
  while( ruleIter != _ruleSet.end() ) {
    int field = index.field( ruleIter->fieldName );
    _ruleMap[ field ] = *ruleIter;
    ruleIter++;
  }

}


void indri::infnet::ShrinkageBeliefNode::addShrinkageRule( std::string ruleText ) {


  int nextComma = 0;
  int nextColon = 0;
  int location = 0;
  
  smoothing_rule rule;
  rule.weight = 0;
  rule.lengthProportional = false;
  rule.fieldName = "";

  for( location = 0; location < ruleText.length(); ) {
    nextComma = ruleText.find( ',', location );
    nextColon = ruleText.find( ':', location );
    
    std::string key = ruleText.substr( location, nextColon-location );
    std::string value = ruleText.substr( nextColon+1, nextComma-nextColon-1 );

    if( key == "parentWeight" ) {
      _parentWeight = atof( value.c_str() );        
    } else if( key == "docWeight" ) {
      _docWeight = atof( value.c_str() );
    } else if( key == "recursive" ) {
      _recursive = (value == "true");
    } else if( key == "queryLevelCombine" ) {
      _queryLevelCombine = (value == "true");
    } else if( key == "field" ) {
      rule.fieldName = value;
    } else if( key == "weight" ) {
      rule.weight = atof( value.c_str() );
    } else if( key == "length") {
      rule.lengthProportional = (value == "true");
    }
    if( nextComma > 0 )
      location = nextComma+1;
    else
      location = ruleText.size();
  }
  if ( rule.fieldName != "" ) {
    _ruleSet.insert( rule );
  }

}


void indri::infnet::ShrinkageBeliefNode::setSmoothing( const std::string & stringSpec ) {
  indri::api::Parameters spec;

  int nextComma = 0;
  int nextColon = 0;
  int location = 0;

  for( location = 0; location < stringSpec.length(); ) {
    nextComma = stringSpec.find( ',', location );
    nextColon = stringSpec.find( ':', location );

    std::string key = stringSpec.substr( location, nextColon-location );
    std::string value = stringSpec.substr( nextColon+1, nextComma-nextColon-1 );

    spec.set( key, value );

    if( nextComma > 0 )
      location = nextComma+1;
    else
      location = stringSpec.size();
  }


  std::string method = spec.get( "method", "dirichlet" );

  if( method == "linear" || method == "jm" || method == "jelinek-mercer" ) {
    // jelinek-mercer -- can take parameters collectionLambda (or just lambda) and documentLambda
    double documentLambda = spec.get( "documentLambda", 0.0 );
    double collectionLambda;
    
    if( spec.exists( "collectionLambda" ) )
      collectionLambda = spec.get( "collectionLambda", 0.4 );
    else
      collectionLambda = spec.get( "lambda", 0.4 );

    _otherWeight = documentLambda + collectionLambda;
  }
}
