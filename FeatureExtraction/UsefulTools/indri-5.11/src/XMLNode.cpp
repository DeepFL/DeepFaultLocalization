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

#include "indri/XMLNode.hpp"

indri::xml::XMLNode::XMLNode( const std::string& name ) : _name(name)
{
}

indri::xml::XMLNode::XMLNode( const std::string& name, const std::string& value ) : _name(name), _value(value)
{
}

indri::xml::XMLNode::XMLNode( const std::string& name, const MAttributes& attributes ) : _name(name), _attributes(attributes)
{
}

indri::xml::XMLNode::XMLNode( const std::string& name, const MAttributes& attributes, const std::string& value ) : _name(name), _attributes(attributes), _value(value)
{
}

indri::xml::XMLNode::~XMLNode() {
  for( size_t i=0; i<_children.size(); i++ )
    delete _children[i];
}

void indri::xml::XMLNode::addChild( XMLNode* child ) {
  _children.push_back(child);
}

void indri::xml::XMLNode::addAttribute( const std::string& key, const std::string& value ) {
  _attributes.insert( std::make_pair( key, value ) );
}

const std::string& indri::xml::XMLNode::getName() const {
  return _name;
}

const std::string& indri::xml::XMLNode::getValue() const {
  return _value;
}

const indri::xml::XMLNode::MAttributes& indri::xml::XMLNode::getAttributes() const {
  return _attributes;
}

std::string indri::xml::XMLNode::getAttribute( const std::string& name ) const {
  MAttributes::const_iterator iter;
  iter = _attributes.find( name );

  if( iter != _attributes.end() ) {
    return iter->second;
  }

  return std::string();
}

const std::vector<indri::xml::XMLNode*>& indri::xml::XMLNode::getChildren() const {
  return _children;
}

const indri::xml::XMLNode* indri::xml::XMLNode::getChild( const std::string& name ) const {
  for( size_t i=0; i<_children.size(); i++ ) {
    if( _children[i]->getName() == name )
      return _children[i];
  }

  return 0;
}

std::string indri::xml::XMLNode::getChildValue( const std::string& name ) const {
  const indri::xml::XMLNode* child = getChild(name);

  if( child ) {
    return child->getValue();
  }

  return std::string();
}

void indri::xml::XMLNode::setValue( const std::string& value ) {
  _value = value;
}


