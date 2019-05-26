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
// XMLReader
//
// 8 October 2003 - tds
//

#ifndef _MONITOR_XMLREADER_H
#define _MONITOR_XMLREADER_H

#include <string>
#include "indri/XMLNode.hpp"
namespace indri
{
  namespace xml
  {
    
    class XMLReader {
    private:
      enum {
        TAG_OPEN_TYPE,
        TAG_CLOSE_TYPE,
        TAG_OPEN_CLOSE_TYPE,
        TAG_CDATA_TYPE
      };

      int _tryFindChar( char ch, const char* buffer, int start, int finish );
      int _findChar( char ch, const char* buffer, int start, int finish );
      int _tryFindBeginTag( const char* buffer, int start, int finish );
      int _findBeginTag( const char* buffer, int start, int finish );
      int _findEndTag( const char* buffer, int start, int finish );
      int _tryFindText( const char* buffer, int start, int finish );
      int _findText( const char* buffer, int start, int finish );
      int _findNotName( const char* buffer, int start, int finish );
      int _findSpace( const char* buffer, int start, int finish );
      int _readTag( const char* buffer, int bufferStart, int bufferEnd, std::string* tagName, std::map<std::string, std::string>* attributes, int* tagType );
      int _findClosingTag( const char* buffer, int start, int end, std::string& openingTagName, bool* tagsBetween );
      void _read( XMLNode** parent, const char* buffer, int start, int end );

    public:
      XMLNode* read( const char* buffer, size_t length );
      XMLNode* read( const std::string& str );
    };
  }
}

#endif // _MONITOR_XMLREADER_H
