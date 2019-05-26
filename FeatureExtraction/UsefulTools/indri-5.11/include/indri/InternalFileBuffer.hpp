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
// InternalFileBuffer
//
// 2004 November 24 -- tds
//

#ifndef INDRI_INTERNALFILEBUFFER_HPP
#define INDRI_INTERNALFILEBUFFER_HPP

#include "indri/Buffer.hpp"
namespace indri
{
  namespace file
  {
    
    struct InternalFileBuffer {
      InternalFileBuffer( size_t length ) {
        buffer.grow(length);
        filePosition = 0;
      }

      indri::utility::Buffer buffer;
      UINT64 filePosition;
    };
  }
}

#endif // INDRI_INTERNALFILEBUFFER_HPP
