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
// UnnecessaryNodeRemoverCopier
//
// 17 March 2004 -- tds
//

#ifndef INDRI_UNNECESSARYNODEREMOVERCOPIER_HPP
#define INDRI_UNNECESSARYNODEREMOVERCOPIER_HPP

#include "indri/QuerySpec.hpp"
#include "indri/delete_range.hpp"
namespace indri
{
  namespace lang
  {
    
    class UnnecessaryNodeRemoverCopier : public indri::lang::Copier {
    protected:
      std::vector<indri::lang::Node*> _nodes;

      class SingleChildWalker : public indri::lang::Walker { 
      private:
        indri::lang::Node* _child;
        int _children;
        bool _seenRoot;

      public:
        SingleChildWalker() : _seenRoot(false), _children(0), _child(0) {}

        void defaultBefore( indri::lang::Node* n ) {
          if( !_seenRoot ) {
            _seenRoot = true;
          } else {
            _children++;
            _child = n;
          }
        }

        bool onlyOneChild() {
          return _children == 1;
        }

        indri::lang::Node* getChild() {
          return _child;
        }
      };

    public:
      ~UnnecessaryNodeRemoverCopier() {
        indri::utility::delete_vector_contents( _nodes );
      }

      indri::lang::Node* defaultAfter( indri::lang::Node* old, indri::lang::Node* newNode ) {
        _nodes.push_back( newNode );
        return newNode;
      }

      indri::lang::Node* after( indri::lang::ExtentAnd* oldAnd, indri::lang::ExtentAnd* newAnd ) {
        SingleChildWalker walker;
        newAnd->walk(walker);

        if( walker.onlyOneChild() ) {
          delete newAnd;
          return walker.getChild();
        } else {
          _nodes.push_back( newAnd );
          return newAnd;
        }
      }

      indri::lang::Node* after( indri::lang::ExtentOr* oldOr, indri::lang::ExtentOr* newOr ) {
        SingleChildWalker walker;
        newOr->walk(walker);

        if( walker.onlyOneChild() ) {
          delete newOr;
          return walker.getChild();
        } else {
          _nodes.push_back( newOr );
          return newOr;
        }
      }

      indri::lang::Node* after( indri::lang::ODNode* oldOD, indri::lang::ODNode* newOD ) {
        SingleChildWalker walker;
        newOD->walk(walker);

        if( walker.onlyOneChild() ) {
          delete newOD;
          return walker.getChild();
        } else {
          _nodes.push_back( newOD );
          return newOD;
        }
      }

      indri::lang::Node* after( indri::lang::UWNode* oldUW, indri::lang::UWNode* newUW ) {
        SingleChildWalker walker;
        newUW->walk(walker);

        if( walker.onlyOneChild() ) {
          delete newUW;
          return walker.getChild();
        } else {
          _nodes.push_back( newUW );
          return newUW;
        }
      }

      indri::lang::Node* after( indri::lang::LengthPrior * oldLP, indri::lang::LengthPrior * newLP ) {
        if ( oldLP->getExponent() == 0 ) {
          indri::lang::Node* child = newLP->getChild();
          delete newLP;  
          return child;
        } else {
          _nodes.push_back( newLP );
          return newLP;
        }
      }

    };
  }
}

#endif // INDRI_UNNECESSARYNODEREMOVERCOPIER_HPP

