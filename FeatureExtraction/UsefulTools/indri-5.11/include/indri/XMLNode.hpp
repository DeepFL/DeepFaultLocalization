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
// XMLNode
//
// 8 October 2003 - tds
//

#ifndef MONITOR_XMLNODE_H
#define MONITOR_XMLNODE_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <assert.h>
#include <iomanip>
#include <iostream>

#include "indri/indri-platform.h"
#include "lemur/lemur-compat.hpp"
namespace indri
{
  /*! \brief XML support classes. */
  namespace xml
  {
    
    /*! Internal Representation of an XML node hierarchy.
     */
    class XMLNode {
    public:
      typedef std::map<std::string,std::string> MAttributes;
  
    private:
      std::string _name;
      MAttributes _attributes;
      std::vector<XMLNode*> _children;
      std::string _value;

    public:
      /// Create a node with the given name
      /// @param name the name for the node.
      XMLNode( const std::string& name );
      /// Create a node with the given name and value
      /// @param name the name for the node.
      /// @param value the value for the node
      XMLNode( const std::string& name, const std::string& value );
      /// Create a node with the given name and value
      /// @param name the name for the node.
      /// @param attributes the map constituting the value for the node
      XMLNode( const std::string& name, const MAttributes& attributes );
      /// Create a node with the given name and value
      /// @param name the name for the node.
      /// @param attributes the map constituting the value for the node
      /// @param value the string value for the node.
      XMLNode( const std::string& name, const MAttributes& attributes, const std::string& value );
      /// clean up
      ~XMLNode();
      /// Add a child to this node.
      /// @param child the child to add
      void addChild( XMLNode* child );
      /// Add an attribute to this node.
      /// @param key the key for the parameter
      /// @param value the value for the parameter
      void addAttribute( const std::string& key, const std::string& value );
      /// Set the value of this node
      /// @param value the new value
      void setValue( const std::string& value );
      /// @return the name of this node
      const std::string& getName() const;
      /// @return the value of this node
      const std::string& getValue() const;
      /// @return the attributes of this node
      const MAttributes& getAttributes() const;
      /// Get the named attribute value
      /// @param name the key
      /// @return the value
      std::string getAttribute( const std::string& name ) const;
      /// Get the children of this node
      /// @return the vector of child node pointers.
      const std::vector<XMLNode*>& getChildren() const;
      /// Get the named child of this node.
      /// @param name the key
      /// @return the named child node
      const XMLNode* getChild( const std::string& name ) const;  
      /// Get the value of the named child of this node.
      /// @param name the key
      /// @return the value of the named child node
      std::string getChildValue( const std::string& name ) const; 
    };
  }
}

/// Convert an INT64 to a string
inline std::string i64_to_string( INT64 value ) {
  std::stringstream number;

  if( value > 1000000000 ) {
    number << (value/1000000000) << std::setw(9) << std::setfill('0') << (value%1000000000);
  } else {
    number << value;
  }

  return number.str();
}

/// Convert a string to an INT64
inline INT64 string_to_i64( const std::string& str ) {
  INT64 result = 0;
  INT64 negative = 1;
  unsigned int i = 0;

  if( str.length() > 0 && str[0] == '-' ) {
    negative = -1;
    i = 1;
  }

  for( ; i<str.length(); i++ ) {
    result = result * 10 + (str[i] - '0');
  }

  return result * negative;
}

/// Convert a string to an int
inline int string_to_int( const std::string& str ) {
  return (int) string_to_i64( str );
}

/// Base64 encode an input block of memory into a string
/// @param input the input to encode
/// @param length the length of the input
/// @return the encoded string
inline std::string base64_encode( const void* input, int length ) {
  static unsigned char lookup[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
  };

  const unsigned char* in = (const unsigned char*) input;
  std::string result;
  unsigned int value;
  unsigned int mainLength;

  // mainlength is the total length of contiguous 3-byte chunks
  if( length%3 ) {
    mainLength = length - length%3;
  } else {
    mainLength = length;
  }

  // reserve enough string space to hold the result
  result.reserve( (length/2+1)*3 );

  // main loop encodes each group of 3 8-bit chars as
  // 4 6-bit chars
  for( unsigned int i=0; i<mainLength; i+=3 ) {
    value = (in[i+0] & 0xff) << 16 |
      (in[i+1] & 0xff) <<  8 |
      (in[i+2] & 0xff);

    unsigned char fourth = lookup[value & 0x3f];
    value >>= 6;
    unsigned char third = lookup[value & 0x3f];
    value >>= 6;
    unsigned char second = lookup[value & 0x3f];
    value >>= 6;
    unsigned char first = lookup[value & 0x3f];

    result.push_back( first );
    result.push_back( second );
    result.push_back( third );
    result.push_back( fourth );
  }

  if( mainLength != length ) {
    value = 0;
    int remaining = length - mainLength;

    {
      // build a value based on the characters we 
      // have left
      unsigned char first = 0;
      unsigned char second = 0;
      unsigned char third = 0;

      if( remaining >= 1 )
        first = in[mainLength+0];
      if( remaining >= 2 )
        second = in[mainLength+1];
      if( remaining >= 3 )
        third = in[mainLength+2];

      value = first << 16 |
        second << 8 |
        third;
    }

    {
      // encode them
      unsigned char fourth = '=';
      unsigned char third = '=';
      unsigned char second = '=';
      unsigned char first = '=';

      if( remaining >= 3 )
        fourth = lookup[value & 0x3f];
      value >>= 6;
      if( remaining >= 2 )
        third = lookup[value & 0x3f];
      value >>= 6;
      if( remaining >= 1 )
        second = lookup[value & 0x3f];
      value >>= 6;
      first = lookup[value & 0x3f];

      result.push_back( first );
      result.push_back( second );
      result.push_back( third );
      result.push_back( fourth );
    }
  }

  return result;
}

/// Base64 decode a string into an output block of memory
/// @param input the input to decode
/// @param output the block to decode into
/// @param outputLength the length of the output
/// @return true output length
inline int base64_decode( void* output, int outputLength, const std::string& input ) {
  assert( (input.size() % 4) == 0 );

  // encoding table built with a python script to match the encoding proposed in rfc1521
  static char lookup[] = {
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   62,   -1,   -1,   -1,   63,
    52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   -1,   -1,   -1,    0,   -1,   -1,
    -1,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   -1,   -1,   -1,   -1,   -1,
    -1,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
    41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51,   -1,   -1,   -1,   -1,   -1,
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1
  };

  char* out = (char*) output;
  int trueOutputLength = 0;

  for( size_t i=0; i<input.size(); i+=4 ) {
    // decode 4 byte chunks
    unsigned char first = input[i];
    unsigned char second = input[i+1];
    unsigned char third = input[i+2];
    unsigned char fourth = input[i+3];

    unsigned int value;

    value = lookup[first] << 18 |
      lookup[second] << 12 |
      lookup[third] << 6 |
      lookup[fourth];

    if( fourth == '=' ) {
      // this chunk ends in padding, so handle it in a special way
      if( third == '=' ) {
        // only one additional byte
        out[trueOutputLength]   = (value >> 16) & 0xff;
        trueOutputLength++;
      } else {
        // two additional bytes
        out[trueOutputLength]   = (value >> 16) & 0xff;
        out[trueOutputLength+1] = (value >> 8) & 0xff;
        trueOutputLength+=2;
      }
    } else {
      out[trueOutputLength]   = (value >> 16) & 0xff;
      out[trueOutputLength+1] = (value >> 8) & 0xff;
      out[trueOutputLength+2] = (value) & 0xff;

      trueOutputLength += 3;
    }
  }

  assert( trueOutputLength <= outputLength );
  return trueOutputLength;
}

/// Base64 decode a string into a new string
/// @param in the input to encode
/// @param out the string for the output
inline void base64_decode_string( std::string& out, const std::string& in ) {
  char* buf = new char[in.size()+1];
  size_t outLength = base64_decode( buf, (int)in.size()+5, in );
  buf[outLength] = 0;
  out = buf;
  delete[] buf;
}

#endif // MONITOR_XMLNODE_H

