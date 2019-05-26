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
// Appliers
//
// 16 September 2004 
//

#ifndef INDRI_APPLIERS_HPP
#define INDRI_APPLIERS_HPP

#include "indri/delete_range.hpp"
namespace indri
{
  /*! \brief Indri query language nodes and support classes. */
  namespace lang
  {
    
    template<class T, class NodeType = indri::lang::Node>
    class ApplySingleCopier {
    private:
      std::vector<indri::lang::Node*> _roots;
      T* _copier;

    public:
      ApplySingleCopier( std::vector<NodeType*>& previous, class indri::collection::Repository& repository ) {
        _copier = new T;

        for( size_t i=0; i<previous.size(); i++ ) {
          indri::lang::Node* root = previous[i];
          indri::lang::Node* newRoot = root->copy( *_copier );

          _roots.push_back(newRoot);
        }
      }

      ~ApplySingleCopier() {
        delete _copier;
      }

      std::vector<indri::lang::Node*>& roots() {
        return _roots;
      }
    };

    template<class T, class NodeType = indri::lang::Node>
    class ApplyCopiers {
    private:
      std::vector<indri::lang::Node*> _roots;
      std::vector<T*> _copiers;

    public:
      ApplyCopiers( std::vector<NodeType*>& previous ) {
        for( size_t i=0; i<previous.size(); i++ ) {
          indri::lang::Node* root = previous[i];
          T* copier = new T;
          indri::lang::Node* newRoot = root->copy( *copier );

          _roots.push_back(newRoot);
          _copiers.push_back(copier);
        }
      }

      ApplyCopiers( std::vector<NodeType*>& previous, class indri::collection::Repository& repository ) {
        for( size_t i=0; i<previous.size(); i++ ) {
          indri::lang::Node* root = previous[i];
          T* copier = new T( repository );
          indri::lang::Node* newRoot = root->copy( *copier );

          _roots.push_back(newRoot);
          _copiers.push_back(copier);
        }
      }

      ApplyCopiers( std::vector<NodeType*>& previous, class ListCache& listCache ) {
        for( size_t i=0; i<previous.size(); i++ ) {
          indri::lang::Node* root = previous[i];
          T* copier = new T( &listCache );
          indri::lang::Node* newRoot = root->copy( *copier );

          _roots.push_back(newRoot);
          _copiers.push_back(copier);
        }
      }

      ApplyCopiers( std::vector<NodeType*>& previous, class indri::collection::Repository& repository, class ListCache& listCache ) {
        for( size_t i=0; i<previous.size(); i++ ) {
          indri::lang::Node* root = previous[i];
          T* copier = new T( repository, listCache );
          indri::lang::Node* newRoot = root->copy( *copier );

          _roots.push_back(newRoot);
          _copiers.push_back(copier);
        }
      }

      ~ApplyCopiers() {
        indri::utility::delete_vector_contents( _copiers );
      }

      std::vector<indri::lang::Node*>& roots() {
        return _roots;
      }
    };

    template<class T, class NodeType=indri::lang::Node>
    class ApplyWalker {
    private:
      T* _walker;

    public:
      ApplyWalker( std::vector<NodeType*>& roots, T* walker ) {
        _walker = walker;
        for( size_t i=0; i<roots.size(); i++ )
          roots[i]->walk(*_walker);
      }

      T& get() {
        return _walker;
      }
    };
  }
}

#endif // INDRI_APPLIERS_HPP

