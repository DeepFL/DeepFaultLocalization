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
// XMLWriter
//
// 8 October 2003 - tds
//

#include "indri/XMLWriter.hpp"

void indri::xml::XMLWriter::_writeChar( char ch, std::string& output ) const {
  output += ch;
}

void indri::xml::XMLWriter::_writeTabs( int tabs, std::string& output ) const {
  for( int i=0; i<tabs; i++ ) {
    _writeChar( '\t', output );
  }
}

void indri::xml::XMLWriter::_writeTag( const std::string& tag, const indri::xml::XMLNode::MAttributes& attributes, std::string& output, bool opening ) const {
  _writeChar( '<', output );

  if( !opening ) {
    _writeChar( '/', output );
  }
  output += tag;

  if( opening ) {
    indri::xml::XMLNode::MAttributes::const_iterator iter;
    for( iter = attributes.begin(); iter != attributes.end(); iter++ ) {
      _writeChar( ' ', output );
      output += iter->first;

      _writeChar( '=', output );
      _writeChar( '\"', output );
      output += iter->second;
      _writeChar( '\"', output );
    }
  }

  _writeChar( '>', output );
}

void indri::xml::XMLWriter::_writeEndOfLine( std::string& output ) const {
  _writeChar( '\n', output );
}

void indri::xml::XMLWriter::_writeXML( int tabs, const indri::xml::XMLNode* node, std::string& output ) const {
  // write opening tag
  _writeTabs( tabs, output );
  _writeTag( node->getName(), node->getAttributes(), output, true );

  if( node->getChildren().size() ) {
    _writeEndOfLine( output );

    for( unsigned int i=0; i<node->getChildren().size(); i++ ) {
      _writeXML( tabs+1, (node->getChildren())[i], output );
    }

    _writeTabs( tabs, output );
  } else {
    output += node->getValue();
  }

  _writeTag( node->getName(), node->getAttributes(), output, false );
  _writeEndOfLine( output );
}

indri::xml::XMLWriter::XMLWriter( indri::xml::XMLNode* node ) : _node(node)
{
}

void indri::xml::XMLWriter::write( std::string& output ) {
  _writeXML( 0, _node, output );
}

