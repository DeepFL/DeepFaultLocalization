/*==========================================================================
 * Copyright (c) 2009 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// QueryTFWalker
//
// 07 August 2009 -- dmf
//

#ifndef INDRI_QUERYTFWALKER_HPP
#define INDRI_QUERYTFWALKER_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>

#include "indri/QuerySpec.hpp"
#include "indri/Walker.hpp"
#include "indri/QueryServer.hpp"

namespace indri
{
  namespace lang
  {
    
    class QueryTFWalker : public indri::lang::Walker {
    private:
      // map query term -> RawScorerNode list
      // qTF is the length of the list
      std::map< lemur::api::TERMID_T, std::vector<indri::lang::RawScorerNode*> > _nodeMap;
      std::vector<indri::server::QueryServer*> &_servers;      

    public:
      QueryTFWalker( std::vector<indri::server::QueryServer*> & servers ) : _servers(servers) {
      }

      ~QueryTFWalker( ) {
      }

      void after( indri::lang::RawScorerNode* scorer ) {
        // have to stem the term to get the counts right.
        lemur::api::TERMID_T id = 0;
        // take the first id returned
        for (int i = 0; (id == 0) && (i < _servers.size()); i++)
          id = _servers[i]->termID(scorer->queryText());
        _nodeMap[id].push_back(scorer);
      }
      void after( indri::lang::PlusNode *plus) {
        std::map< lemur::api::TERMID_T, std::vector<indri::lang::RawScorerNode*> >::iterator iter;
        for (iter = _nodeMap.begin(); iter != _nodeMap.end(); iter++) {
          std::vector<indri::lang::RawScorerNode *> & nodes = iter->second;
          for (int i = 0; i < nodes.size(); i++) {
            std::string smoothing = nodes[i]->getSmoothing();
            int qTF = nodes.size();
            std::stringstream smooth;
            smooth << smoothing << ",qtf:" << qTF;
            nodes[i]->setSmoothing( smooth.str() );
          }
        }
      }
      void after( indri::lang::WPlusNode *plus) {
        const std::vector< std::pair<double, ScoredExtentNode*> >& children = plus->getChildren();
        for( size_t i=0; i<children.size(); i++ ) {
          double weight = children[i].first;
          indri::lang::RawScorerNode * raw = dynamic_cast<indri::lang::RawScorerNode *>( children[i].second );
          std::string smoothing = raw->getSmoothing();
          std::stringstream smooth;
          smooth << smoothing << ",qtw:" << weight;
          raw->setSmoothing( smooth.str() );
        }
      }
    };
  }
}

#endif // INDRI_QUERYTFWALKER_HPP

