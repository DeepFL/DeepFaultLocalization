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
// WeightFoldingCopier
//
// 17 September 2004 -- tds
//

#ifndef INDRI_WEIGHTFOLDINGCOPIER_HPP
#define INDRI_WEIGHTFOLDINGCOPIER_HPP
namespace indri
{
  namespace lang 
  {
    
    class WeightFoldingCopier : public indri::lang::Copier {
    private:
      std::vector<indri::lang::Node*> _nodes;

    public:
      ~WeightFoldingCopier() {
        indri::utility::delete_vector_contents( _nodes );
      }

      indri::lang::Node* defaultAfter( indri::lang::Node* old, indri::lang::Node* newNode ) {
        _nodes.push_back( newNode );
        return newNode;
      }

      indri::lang::Node* after( indri::lang::WeightNode* oldWeightNode, indri::lang::WeightNode* newWeightNode ) {
        indri::lang::WeightNode* newerWeightNode = new indri::lang::WeightNode();
        const std::vector< std::pair<double, indri::lang::ScoredExtentNode*> >& children = newWeightNode->getChildren();
    
        for( size_t i=0; i<children.size(); i++ ) {
          // is this a weight node?
          indri::lang::WeightNode* childWeightNode = dynamic_cast<indri::lang::WeightNode*>( children[i].second );

          // is this a combine node?
          indri::lang::CombineNode* childCombineNode = dynamic_cast<indri::lang::CombineNode*>( children[i].second );

          if( !childWeightNode && !childCombineNode ) {
            // child is not a weight node, so just add it directly
            newerWeightNode->addChild( children[i].first, children[i].second );
          } else if( childCombineNode ) {
            const std::vector< indri::lang::ScoredExtentNode* >& grandkids = childCombineNode->getChildren();
            double kidWeight = children[i].first / double(grandkids.size());
        
            for( size_t j=0; j<grandkids.size(); j++ ) {
              newerWeightNode->addChild( kidWeight, grandkids[j] );
            }
          } else {
            // child _is_ a weight node, so we're going to fold all its children up to this level
            const std::vector< std::pair<double, indri::lang::ScoredExtentNode*> >& grandkids = childWeightNode->getChildren();
            double parentWeight = children[i].first;
            double normalizer = 0.0;

            // need to normalize all weights to sum to 1
            for( size_t j=0; j<grandkids.size(); j++ ) {
              normalizer += grandkids[j].first;
            }
    
            for( size_t j=0; j<grandkids.size(); j++ ) {
              // have to normalize the weight by including the parent weight as well
              newerWeightNode->addChild( parentWeight * grandkids[j].first / normalizer,
                                         grandkids[j].second );
            }
          }
        }

        newerWeightNode->setNodeName( newWeightNode->nodeName() );
        delete newWeightNode;
        _nodes.push_back( newerWeightNode );

        return newerWeightNode;
      }
    };
  }
}

#endif // INDRI_WEIGHTFOLDINGCOPIER_HPP

