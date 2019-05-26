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
// FieldWildcardNode
//
// 18 June 2005 -- pto
//
// This node poplulates the field list with all fields in the document contained
// within the "outer" field as candidate fields.  Slow as it requires 
// scanning the field list for the doc or scanning a document structure object.
//

#ifndef INDRI_FIELDWILDCARDNODE_HPP
#define INDRI_FIELDWILDCARDNODE_HPP

#include "indri/ListIteratorNode.hpp"
#include "indri/greedy_vector"
#include "indri/Index.hpp"

namespace indri {
  namespace infnet {

    class FieldWildcardNode : public ListIteratorNode {
      indri::utility::greedy_vector<indri::index::Extent> _extents;
      std::string _name;
      indri::index::Index * _index;
      lemur::api::DOCID_T _nextDocument;
      indri::index::TermListFileIterator * _docIter;
      int _docIterID;
      
    public:
      FieldWildcardNode( const std::string& name );
      ~FieldWildcardNode( );
      
      void prepare( lemur::api::DOCID_T documentID );
      const indri::utility::greedy_vector<indri::index::Extent>& extents();
      lemur::api::DOCID_T nextCandidateDocument();
      const std::string& getName() const;
      void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent );

      void indexChanged( indri::index::Index& index);
    };
  }
}
#endif // INDRI_FIELDWILDCARDNODE_HPP

