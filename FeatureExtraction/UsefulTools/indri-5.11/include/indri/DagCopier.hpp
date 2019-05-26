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
// DagCopier
//
// 5 March 2004 -- tds
//

#ifndef INDRI_DAGCOPIER_HPP
#define INDRI_DAGCOPIER_HPP

#include <vector>
#include "indri/delete_range.hpp"
namespace indri 
{
  namespace lang
  {
    
    class DagCopier : public indri::lang::Copier {
    private:
      std::vector<indri::lang::IndexTerm*> _terms;
      std::vector<indri::lang::Field*> _fields;
      std::vector<indri::lang::ExtentOr*> _extentOrs;
      std::vector<indri::lang::ExtentAnd*> _extentAnds;
      std::vector<indri::lang::ExtentInside*> _extentInsides;
      std::vector<indri::lang::ODNode*> _odNodes;
      std::vector<indri::lang::UWNode*> _uwNodes;
      std::vector<indri::lang::FieldWildcard*> _fieldWildcardNodes;
      std::vector<indri::lang::DocumentStructureNode*> _documentStructureNodes;

      std::vector<indri::lang::Node*> _newNodes;

      template<class T>
      T* _findReplacement( std::vector<T*>& replacements, T* candidate ) {
        T* replacement = 0;
    
        for( unsigned int i=0; i<replacements.size(); i++ ) {
          if( (*candidate) == (*replacements[i]) ) {
            replacement = replacements[i];
            break;
          }
        }

        if( replacement ) {
          delete candidate;
          candidate = replacement;
        } else {
          _newNodes.push_back( candidate );
          replacements.push_back( candidate );
        }

        return candidate;
      }

    public:
      ~DagCopier() {
        indri::utility::delete_vector_contents( _newNodes );
      }

      indri::lang::Node* defaultAfter( indri::lang::Node* oldNode, indri::lang::Node* newNode ) {
        _newNodes.push_back( newNode );
        return newNode;
      }

      indri::lang::Node* after( indri::lang::IndexTerm* indexTerm, indri::lang::IndexTerm* newIndexTerm ) {
        return _findReplacement<indri::lang::IndexTerm>( _terms, newIndexTerm );
      }

      indri::lang::Node* after( indri::lang::Field* field, indri::lang::Field* newField ) {
        return _findReplacement<indri::lang::Field>( _fields, newField );
      }

      indri::lang::Node* after( indri::lang::ExtentOr* oldExtentOr, indri::lang::ExtentOr* newExtentOr ) {
        return _findReplacement<indri::lang::ExtentOr>( _extentOrs, newExtentOr );
      }

      indri::lang::Node* after( indri::lang::ExtentAnd* oldExtentAnd, indri::lang::ExtentAnd* newExtentAnd ) {
        return _findReplacement<indri::lang::ExtentAnd>( _extentAnds, newExtentAnd );
      }

      indri::lang::Node* after( indri::lang::ExtentInside* oldExtentInside, indri::lang::ExtentInside* newExtentInside ) {      
        return _findReplacement<indri::lang::ExtentInside>( _extentInsides, newExtentInside );
      }

      indri::lang::Node* after( indri::lang::NestedExtentInside* oldExtentInside, indri::lang::NestedExtentInside* newExtentInside ) {  
        return after((indri::lang::ExtentInside*) oldExtentInside, (indri::lang::ExtentInside*) newExtentInside );
      }
 
      indri::lang::Node* after( indri::lang::ODNode* oldODNode, indri::lang::ODNode* newODNode ) {
        return _findReplacement<indri::lang::ODNode>( _odNodes, newODNode );
      }

      indri::lang::Node* after( indri::lang::UWNode* oldUWNode, indri::lang::UWNode* newUWNode ) {
        return _findReplacement<indri::lang::UWNode>( _uwNodes, newUWNode );
      }

      indri::lang::Node* after( indri::lang::FieldWildcard* fieldWildcard, indri::lang::FieldWildcard* newFieldWildcard ) {
        return _findReplacement<indri::lang::FieldWildcard>( _fieldWildcardNodes, newFieldWildcard );
      }

      indri::lang::Node* after( indri::lang::DocumentStructureNode* docStruct, indri::lang::DocumentStructureNode* newDocStruct ) {
        return _findReplacement<indri::lang::DocumentStructureNode>( _documentStructureNodes, newDocStruct );
      }
      
    };
  }
}

#endif // INDRI_DAGCOPIER_HPP

