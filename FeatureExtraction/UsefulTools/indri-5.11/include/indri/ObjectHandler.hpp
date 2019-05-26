/*==========================================================================
 * Copyright (c) 2003-2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// ObjectHandler
//
// 11 May 2004 -- tds
//

#ifndef INDRI_OBJECTHANDLER_HPP
#define INDRI_OBJECTHANDLER_HPP
namespace indri
{
  namespace parse
  {
    
    template<class _Type>
    class ObjectHandler {
    public:
      virtual ~ObjectHandler() {};
      virtual void handle( _Type* object ) = 0;
    };
  }
}

#endif // INDRI_OBJECTHANDLER_HPP
