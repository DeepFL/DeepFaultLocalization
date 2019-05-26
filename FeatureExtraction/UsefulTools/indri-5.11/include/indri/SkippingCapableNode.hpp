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
// SkippingCapableNode
//
// 27 January 2004 - tds
//

#ifndef INDRI_SKIPPINGCAPABLENODE_HPP
#define INDRI_SKIPPINGCAPABLENODE_HPP

#include "indri/BeliefNode.hpp"
namespace indri
{
  namespace infnet
  {
    
    class SkippingCapableNode : public BeliefNode {
    public:
      virtual void setThreshold( double threshold ) = 0;
    };
  }
}

#endif // INDRI_SKIPPINGCAPABLENODE_HPP

