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
// DocumentStructureHolderNode
//
// 18 June 2005 -- pto
//
// This node stores the document structure for the current document
// to be shared among shrinkage belief nodes or other nodes that 
// may need to use the document structure for retrieval.
//

#ifndef INDRI_DOCUMENTSTRUCTUREHOLDERNODE_HPP
#define INDRI_DOCUMENTSTRUCTUREHOLDERNODE_HPP

#include "indri/ListIteratorNode.hpp"
#include "indri/greedy_vector"
#include "indri/Index.hpp"
#include "indri/DocumentStructure.hpp"

namespace indri {
  namespace infnet {

    class DocumentStructureHolderNode : public InferenceNetworkNode {
    private:
      indri::index::Index * _index;
      lemur::api::DOCID_T _nextDocument;
      indri::index::TermListFileIterator * _docIter;
      lemur::api::DOCID_T  _docIterID;
      std::string _name;
      indri::index::DocumentStructure * _documentStructure;

    public:
      DocumentStructureHolderNode( const std::string& name );
      ~DocumentStructureHolderNode();
      
      void prepare( lemur::api::DOCID_T documentID );
      lemur::api::DOCID_T nextCandidateDocument();
      const std::string& getName() const;
      indri::index::DocumentStructure * getDocumentStructure();
     
      void indexChanged( indri::index::Index& index);
    };
  }
}
#endif // INDRI_DOCUMENTSTRUCTUREHOLDERNODE_HPP

