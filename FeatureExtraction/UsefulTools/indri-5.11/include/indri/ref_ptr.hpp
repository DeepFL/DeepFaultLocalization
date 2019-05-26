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
// ref_ptr
//
// 15 December 2004 -- tds
//

#ifndef INDRI_REFPTR_HPP
#define INDRI_REFPTR_HPP

#include "indri/atomic.hpp"

namespace indri {
  namespace atomic
  {
    
    template<class T>
    class ref_ptr {
    private:
      template<class TT>
      struct object_ref {
        TT* object;
        value_type counter;
      };

      typedef ref_ptr<T> my_type;
      mutable object_ref<T>* _ref;

      void _removeRef() {
        if( _ref ) {
          decrement( _ref->counter );
          if( _ref->counter == 0 ) {
            delete _ref->object;
            delete _ref;
          }
        }
      }

      void _addRef() {
        if( _ref )
          increment( _ref->counter );
      }

      my_type& _refAssign( const my_type& other ) {
        _removeRef();
        _ref = other._ref;
        _addRef();
        return *this;
      }

    public:
      ref_ptr() :
        _ref(0)
      {
      }

      ~ref_ptr() {
        _removeRef();
      }

      ref_ptr( const my_type& other ) :
        _ref(0)
      {
        _refAssign( other );
      }

      ref_ptr( T* object ) {
        _ref = new object_ref<T>;
        _ref->object = object;
        _ref->counter = 1;
      }

      my_type& operator= ( T* object ) {
        _removeRef();

        if( object != 0 ) {
          _ref = new object_ref<T>;
          _ref->object = object;
          _ref->counter = 1;
        } else {
          _ref = 0;
        }

        return *this;
      }

      my_type& operator= ( const ref_ptr& other ) {
        _refAssign( other );
        return *this;
      }

      bool operator== ( T* other ) {
        if( _ref == 0 )
          return other == 0;
      
        return _ref->object == other;
      }

      bool operator== ( ref_ptr& other ) {
        return _ref == other._ref;
      }

      atomic::value_type references() {
        if( _ref )
          return _ref->counter;
        return 0;
      }

      T& operator* () {
        return *_ref->object;
      }

      T* operator-> () {
        return _ref->object;
      }

      T* get() {
        if( _ref )
          return _ref->object;
        return 0;
      }
    };
  }
}

#endif // INDRI_REFPTR_HPP
