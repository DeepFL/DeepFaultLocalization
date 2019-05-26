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
// XMLWriter.h
//
// 8 October 2003 - tds
//

#ifndef MONITOR_XMLWRITER_H
#define MONITOR_XMLWRITER_H

#include "indri/XMLNode.hpp"
#include <string>
#include <map>
namespace indri
{
  namespace xml
  {
    
    class XMLWriter {
    private:
      XMLNode* _node;

      void _writeChar( char ch, std::string& output ) const;
      void _writeTabs( int tabs, std::string& output ) const;
      void _writeTag( const std::string& tag,
                      const std::map<std::string,std::string>& attributes,
                      std::string& output,
                      bool opening ) const;
      void _writeEndOfLine( std::string& output ) const;
      void _writeXML( int tabs, const XMLNode* node, std::string& output ) const;

    public:
      XMLWriter( XMLNode* node ); 
      void write( std::string& output );
    };
  }
}

#endif // MONITOR_XMLWRITER_H

