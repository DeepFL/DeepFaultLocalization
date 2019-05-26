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
// TreePrinterWalker
//
// 9 March 2004 -- tds
//

#ifndef INDRI_TREEPRINTERWALKER_HPP
#define INDRI_TREEPRINTERWALKER_HPP

#include "indri/QuerySpec.hpp"
namespace indri
{
  namespace lang
  {

    class TreePrinterWalker : public indri::lang::Walker {
    private:
      unsigned int _tabs;
    public:
      TreePrinterWalker() : _tabs(0) {}

      void before( indri::lang::IndexTerm* node ) {
        for( unsigned int i=0; i<_tabs; i++ )
          std::cout << "\t";

        std::cout << "IndexTerm "
                  << node->getText().c_str()
                  << " "
                  << node->nodeName().c_str()
                  << std::endl;

        _tabs++;
      }

      void before( indri::lang::Field* node ) {
        for( unsigned int i=0; i<_tabs; i++ )
          std::cout << "\t";

        std::cout << "Field "
                  << node->getFieldName().c_str()
                  << " "
                  << node->nodeName().c_str()
                  << std::endl;

        _tabs++;
      }

      void defaultBefore( indri::lang::Node* node ) {
        for( unsigned int i=0; i<_tabs; i++ )
          std::cout << "\t";

        std::string type = node->typeName();
        std::string query = node->queryText();

        std::cout << type
                  << " "
                  << query
                  << std::endl;

        _tabs++;
      }

      void defaultAfter( indri::lang::Node* node ) {
        _tabs--;
      }
    };
  }
}

#endif // INDRI_TREEPRINTERWALKER_HPP


