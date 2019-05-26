/*==========================================================================
 * Copyright (c) 2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

//
// AttributeValuePair
//
// 15 September 2005 -- mwb
//

#ifndef INDRI_ATTRIBUTEVALUEPAIR_HPP
#define INDRI_ATTRIBUTEVALUEPAIR_HPP
namespace indri {
  namespace parse {
    
    struct AttributeValuePair {
      char* attribute;
      char* value;

      // byte extent of the value string w/r/t original text; values
      // set within TextTokenizer.
      unsigned int begin;
      unsigned int end;
    };

  }
}

#endif // INDRI_ATTRIBUTEVALUEPAIR_HPP
