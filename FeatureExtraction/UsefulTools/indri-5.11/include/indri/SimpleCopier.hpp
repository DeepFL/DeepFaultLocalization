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
// SimpleCopier
//
// 24 September 2004 -- tds
//
// SimpleCopier just makes a copy of a query tree and stores it.
// This is useful for keeping a copy of a query tree around.
//

#ifndef INDRI_SIMPLECOPIER_HPP
#define INDRI_SIMPLECOPIER_HPP

#include "Copier.hpp"
namespace indri
{
  namespace lang
  {
    
    class SimpleCopier : public indri::lang::Copier {
    private:
      std::vector<indri::lang::Node*> _nodes;

    public:
      ~SimpleCopier() {
        indri::utility::delete_vector_contents( _nodes );
      }

      indri::lang::Node* defaultAfter( indri::lang::Node* oldNode, indri::lang::Node* newNode ) {
        _nodes.push_back(newNode);
        return newNode;
      }

      indri::lang::Node* root() {
        if( _nodes.size() )
          return _nodes.back();
        return 0;
      }
    };
  }
}

#endif // INDRI_SIMPLECOPIER_HPP
