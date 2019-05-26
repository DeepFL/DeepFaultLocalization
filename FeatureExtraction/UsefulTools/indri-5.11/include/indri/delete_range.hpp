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
// delete_range
//
// 5 March 2004 -- tds
//

#ifndef LEMUR_DELETE_RANGE_HPP
#define LEMUR_DELETE_RANGE_HPP

#include <vector>
#include <stack>
#include <map>
namespace indri
{
  namespace utility
  {
    template<typename VectorType>
    void delete_vector_contents( std::vector<VectorType>& vector ) {
      typedef typename std::vector<VectorType>::iterator vec_iter;

      vec_iter begin = vector.begin();
      vec_iter end = vector.end();
      vec_iter iter;

      for( iter = begin; iter != end; iter++ ) {
        delete *iter;
      }

      vector.clear();
    }

    template<typename MapKey, typename MapValue>
    void delete_map_contents( std::map<MapKey, MapValue*>& mp ) {
      typedef typename std::map<MapKey, MapValue*>::iterator map_iter;

      map_iter begin = mp.begin();
      map_iter end = mp.end();
      map_iter iter;

      for( iter = begin; iter != end; iter++ )
        delete iter->second;

      mp.clear();
    }

    template<typename VectorType>
    class VectorDeleter {
    private:
      std::vector<VectorType>* _vec;

    public:
      VectorDeleter() : _vec(0) {}
      VectorDeleter( std::vector<VectorType>& vec ) : _vec(&vec) {}
      ~VectorDeleter() {
        if( _vec )
          delete_vector_contents<VectorType>( *_vec );
      }

      void setVector( std::vector<VectorType>& vec ) {
        _vec = &vec;
      }
    };

    template<typename StackType>
    class StackDeleter {
    private:
      std::stack<StackType*>* _stack;

    public:
      StackDeleter() : _stack(0) {}
      StackDeleter( std::stack<StackType*>& stack ) : _stack(&stack) {}
      ~StackDeleter() {
        if( _stack ) {
          while( _stack->size() ) {
            delete _stack->top();
            _stack->pop();
          }
        }
      }

      void setStack( std::stack<StackType*>& stack ) {
        _stack = &stack;
      }
    };
  }
}

#endif // LEMUR_DELETE_RANGE_HPP

