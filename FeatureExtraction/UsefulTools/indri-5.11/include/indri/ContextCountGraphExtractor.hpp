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
// ContextCountGraphExtractor
//
// 5 March 2004 -- tds
//

#ifndef INDRI_CONTEXTCOUNTGRAPHEXTRACTOR_HPP
#define INDRI_CONTEXTCOUNTGRAPHEXTRACTOR_HPP

#include <indri/delete_range.hpp>
namespace indri
{
  namespace lang
  {
    
    class ContextCountGraphExtractor : public indri::lang::Copier {
    private:
      std::vector<indri::lang::Node*> _nodes;

    public:
      ~ContextCountGraphExtractor() {
        delete_range( _nodes.begin(), _nodes.end() );
      }

      Node* defaultAfter( indri::lang::Node* node ) {

      }

      Node* after( indri::lang::RawScorerNode* oldNode, indri::lang::RawScorerNode* oldNode ) {



      }
    };
 
  }
}

#endif // INDRI_CONTEXTCOUNTGRAPHEXTRACTOR_HPP


