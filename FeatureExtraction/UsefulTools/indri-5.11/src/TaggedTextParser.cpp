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
// TaggedTextParser
//
// 15 September 2005 -- revised by mwb
//

#include "indri/TaggedTextParser.hpp"

indri::parse::TaggedTextParser::TaggedTextParser() :
  tl(new TagList()),
  _metaList(new TagList()),
  _tagTable(1024),
  _handler(0),
  _p_conflater(0)
{
  _exclude = false;
  _include = _defaultInclude = true;
  _startIncludeRegion = 0;
  _startExcludeRegion = 0;
}

indri::parse::TaggedTextParser::~TaggedTextParser() {
  delete tl;
  delete _metaList;
  delete _p_conflater;
  for (size_t t = 0; t < _document.tags.size(); t++) {
    delete _document.tags[t];
  }

  indri::utility::HashTable<const char*,tag_properties*>::iterator iter;
  for( iter = _tagTable.begin(); iter != _tagTable.end(); iter++ ) {
    free( *iter->second );
                }
}

void indri::parse::TaggedTextParser::initialize( indri::parse::TokenizedDocument* document, 
                                                 indri::api::ParsedDocument* parsed ) {

  _exclude = false;
  _include = _defaultInclude;
  _startIncludeRegion = 0;
  _startExcludeRegion = 0;

  tl->clear();
  _metaList->clear();
}

void indri::parse::TaggedTextParser::cleanup( indri::parse::TokenizedDocument* document,
                                              indri::api::ParsedDocument* parsed ) {

  tl->writeTagList( parsed->tags );
  _metaList->writeMetadataList( parsed->metadata, _termBuffer, document->text );
}

indri::parse::TaggedTextParser::tag_properties* indri::parse::TaggedTextParser::_findTag( std::string name ) {

  tag_properties** p = _tagTable.find( name.c_str() );

  if ( ! p ) return NULL;
  else return *p;
}

indri::parse::TaggedTextParser::tag_properties* indri::parse::TaggedTextParser::_buildTag( std::string name ) {

  tag_properties* result = 0;

  if ( (result = _findTag( name.c_str()) ) )
    return result;

  result = (tag_properties*) malloc( sizeof(tag_properties) + name.length() + 1 );

  result->index = false;
  result->exclude = false;
  result->include = false;
  result->metadata = false;

  char* pName = (char *) result + sizeof(tag_properties);
  result->name = pName;
  strcpy( pName, name.c_str() );

  _tagTable.insert( result->name, result );

  return result;
}

void indri::parse::TaggedTextParser::setTags( const std::vector<std::string>& include,
                                              const std::vector<std::string>& exclude,
                                              const std::vector<std::string>& index,
                                              const std::vector<std::string>& metadata, 
                                              const std::map<indri::parse::ConflationPattern*,std::string>& 
                                              conflations ) {

  _defaultInclude = true;

  delete _p_conflater;
  _p_conflater = new Conflater( conflations );

  std::vector<std::string>::const_iterator i;

  for ( i = include.begin(); i != include.end(); i++ ) {

    tag_properties* result = _buildTag( *i );
    result->include = true;
    _defaultInclude = false;
                }

  for ( i = exclude.begin(); i != exclude.end(); i++ ) {

    tag_properties* result = _buildTag( *i );
    result->exclude = true;
                }

  for ( i = index.begin(); i != index.end(); i++ ) {

    tag_properties* result = _buildTag( *i );
    result->index = true;
        }

  for ( i = metadata.begin(); i != metadata.end(); i++ ) {

    tag_properties* result = _buildTag( *i );
    result->metadata = true;
        }

}

indri::api::ParsedDocument* indri::parse::TaggedTextParser::parse( indri::parse::TokenizedDocument* document ) {
  _termBuffer.clear();
  _termBuffer.grow( document->textLength * 4 );
  // need to leave room here for relative->absolute URL expansion

  _document.text = document->text;
  _document.textLength = document->textLength;

  for (size_t t = 0; t < _document.tags.size(); t++) {
    delete _document.tags[t];
  }

  _document.terms.clear();
  _document.tags.clear();
  _document.positions.clear();

  _document.metadata = document->metadata;
  // have to process metadata tag conflations.
  for (size_t idx = 0; idx < _document.metadata.size(); idx++) {
    _document.metadata[idx].key = _p_conflater->conflate(_document.metadata[idx].key);
  }
  

  _document.content = document->content;
  _document.contentLength = document->contentLength;
  

  token_pos = 0;
  tokens_excluded = 0;

  indri::utility::greedy_vector<TermExtent>::iterator i  = 
    document->positions.begin();

  indri::utility::greedy_vector<char*>::iterator j  = 
    document->terms.begin();

  indri::utility::greedy_vector<TagEvent>::iterator k = 
    document->tags.begin();

  initialize(document, &_document);

  // Add a global parent element for all tags.
  // parameters need to be set for this in the FCE
  TagEvent globalTag;  
  globalTag.name = "document";
  globalTag.open_tag = true;
  globalTag.pos = 0; // start at the beginning
  globalTag.begin = 0;
  globalTag.end = 0;
  handleTag(&globalTag);

  while ( i != document->positions.end() ) {

    // As it iterates through the token events, Parser must also run
    // through the TagEvents recognized by the Tokenizer and call
    // handleTag on each event when its token position is reached.
    // handleTag builds the TagList, which checks for overlapping
    // tags, uses the Conflater to conflate tags, and also sets the
    // include and exclude regions.  Tags are checked first, because
    // if there is a tag at position i that starts an include region,
    // then we also need to include the token at position i.

    while ( k != document->tags.end() && token_pos == (*k).pos ) { // There may be multiple tags at a token position
      
      // Adjust actual token position for tokens that may have
      // been excluded:
      (*k).pos -= tokens_excluded;
      
      handleTag( &(*k) );
      k++;
    }

    // The Parser's job is to have a look at the token stream produced
    // by the Tokenizer, and insert, discard or rewrite any tokens as
    // it sees fit.  Any kind of token-level processing that a Parser
    // writer sees fit could be done here.

    if ( ! ( _exclude || ! _include ) ) {

      _document.positions.push_back( (*i) );
      _document.terms.push_back( (*j) );

      //       std::cout << "Token [" << (*j) << "] <" << (*i).begin 
      //                << ", " << (*i).end << ">" << std::endl;

    } else {

      tokens_excluded++;
    }

    i++;
    j++;

    token_pos++;
  }

  // We've reached the end of the term positions, so close any tags
  // we've opened.

  while ( k != document->tags.end() && token_pos == (*k).pos ) { // There may be multiple tags at a token position
    
    // Adjust actual token position for tokens that may have
    // been excluded:
    (*k).pos -= tokens_excluded;
    
    handleTag( &(*k) );
    k++;
  }
  // close the global document tag;
  
  globalTag.open_tag = false;
  globalTag.pos = _document.positions.size();
  globalTag.begin = _document.textLength;
  globalTag.end = _document.textLength;
  handleTag(&globalTag);
  
  // Tag lists are actually written in the cleanup function:
  cleanup(document, &_document);
  return &_document;
}


void indri::parse::TaggedTextParser::handleTag( TagEvent* te ) {

  // Here, we know what the element is, what the attributes are, and
  // whether we are a close tag or an open tag.

  bool atEnd = ! te->open_tag;

  // Conflate
  
  const char* original_name = te->name;
  
  _p_conflater->conflate( te );

  // Now check for tag_properties using conflated form, since
  // tag_properties can only be set per the tag name (ie, not per an
  // attribute-value pattern).

  const tag_properties* tagProps = _findTag( te->name );
  
  bool oldInclude = _include;
  
  if( tagProps ) { 
    // set _include and _exclude flags appropriately:
    if( atEnd ) {
      if( _exclude ) {
        if( tagProps == _startExcludeRegion ) {
          // this is an end tag, and it matches the start of an exclude region
          _startExcludeRegion = 0;
          _exclude = false;
        }
      } else if( _include && tagProps == _startIncludeRegion ) {
        _startIncludeRegion = 0;
        _include = false;
      }
    } else { // !atEnd
      // if we're in exclude mode, new tags don't matter
      if( ! _exclude ) {
        // not in an exclude
        if( !_include ) {
          // not in included territory
          if( tagProps->include && _startIncludeRegion == 0 ) {
            _startIncludeRegion = tagProps;
            _include = true;
          }
        } else {
          // !_exclude && _include
          if( tagProps->exclude && _startExcludeRegion == 0 ) {
            _startExcludeRegion = tagProps;
            _exclude = true;
          }
        }
      }
    }
    
    // index the tags if necessary
    // this may be an end include tag, so we allow oldInclude
    if( (tagProps->index && !_exclude && (_include || oldInclude)) || tagProps == _findTag("document") ) {
      if( atEnd ) {
        endTag( original_name, tagProps->name, te->pos );
      } else {
        addTag( original_name, tagProps->name, te->pos );
      }
    }
    
    // index metadata if necessary
    if( tagProps->metadata ) {
      if( atEnd ) {
        // te->begin is the byte offset of the beginning of the end
        // tag ('<'), or end of the enclosed region.
        endMetadataTag( original_name, tagProps->name, te->begin );
      } else {
        // te->end is the byte offset of the end of the begin tag
        // ('>'), or beginning of the enclosed region.
        addMetadataTag( original_name, tagProps->name, te->end );
      }
    }
  }
}

void indri::parse::TaggedTextParser::handle( indri::parse::TokenizedDocument* document ) {
  _handler->handle( parse(document) );
}

void indri::parse::TaggedTextParser::setHandler( ObjectHandler<indri::api::ParsedDocument>& h ) {
  _handler = &h;
}


