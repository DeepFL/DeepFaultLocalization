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
// atomic
//
// 15 December 2004 -- tds
//

#ifndef INDRI_ATOMIC_HPP
#define INDRI_ATOMIC_HPP

#ifndef WIN32
#if HAVE_BITS_ATOMICITY_H
#include <bits/atomicity.h>
#endif
#if HAVE_EXT_ATOMICITY_H
#include <ext/atomicity.h>
#endif
#endif

namespace indri {
  /*! \brief Atomic actions for thread support */
  namespace atomic {
#ifdef WIN32
    typedef volatile LONG value_type;

    inline void increment( value_type& variable ) {
      ::InterlockedIncrement( &variable );
    }

    inline void decrement( value_type& variable ) {
      ::InterlockedDecrement( &variable );
    }
#else
    // GCC 3.4+ declares these in the __gnu_cxx namespace, 3.3- does not.
    #if P_NEEDS_GNU_CXX_NAMESPACE
    #define __atomic_add __gnu_cxx::__atomic_add
    #endif
    typedef _Atomic_word value_type;

    inline void increment( value_type& variable ) {
     __atomic_add( &variable, 1 );
    }

    inline void decrement( value_type& variable ) {
      __atomic_add( &variable, -1 );
    }
#endif
  }
}

#endif // INDRI_ATOMIC_HPP

