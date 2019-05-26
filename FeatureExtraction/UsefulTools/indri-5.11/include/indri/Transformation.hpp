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
// Transformation
//
// 11 May 2004 -- tds
//

#ifndef INDRI_TRANSFORMATION_HPP
#define INDRI_TRANSFORMATION_HPP

#include "indri/ObjectHandler.hpp"
#include "indri/ParsedDocument.hpp"
namespace indri
{
  namespace parse
  {
    
    class Transformation : public ObjectHandler<indri::api::ParsedDocument> {
    public:
      virtual ~Transformation() {}
      virtual indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document ) = 0;

      virtual void handle( indri::api::ParsedDocument* document ) = 0;
      virtual void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler ) = 0;
    };
  }
}

#endif // INDRI_TRANSFORMATION_HPP


