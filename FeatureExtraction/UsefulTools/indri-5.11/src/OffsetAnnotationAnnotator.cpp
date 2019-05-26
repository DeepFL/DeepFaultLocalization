/*==========================================================================
 * Copyright (c) 2003-2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// OffsetAnnotationAnnotator
//
// 18 September 2005 -- mwb
//
// Reads a supplied offset annotation file and adds the annotations to
// the parsed document.
//

// Format of the offset annotation file: 8-column, tab-delimited.
// From left-to-right, those columns are: 
//
//   docno     : external document id corresponding to the document in
//             : which the annotation occurs.
//
//   type      : TAG or ATTRIBUTE
//
//   id        : an id number for the annotation; each line should have a
//             : unique id >= 1.
//
//   name      : for TAG, name or type of the annotation
//             : for ATTRIBUTE, the attribute name, or key
//
//   start     : start and length define the annotation's extent;
//   length    : meaningless for an ATTRIBUTE
//
//   value     : for TAG, an INT64
//             : for ATTRIBUTE, a string that is the attribute's value
//
//   parentid  : for TAG, refers to the id number of another TAG to be
//             : considered the parent of this one; this is how hierarchical
//             : annotations can be expressed.
//             : a TAG that has no parent has parentid = 0
//             : for ATTRIBUTE, refers to the id number of a TAG to which
//             : it belongs and from which it inherits its start and length.
//             : *NOTE: the file must be sorted such that any line that uses 
//             : a given id in this column must be *after* the line that 
//             : uses that id in the id column.
//
//   debug     : ignored by the OffsetAnnotator; can contain any information
//             : that is beneficial to a human reading the file

// While the OffsetAnnotationAnnotator is transforming the
// ParsedDocument, it will directly operate on the data structures
// just as if it were the Parser, except that it adds annotations from
// its file as opposed to from the original TokenizedDocument text.

#include "indri/OffsetAnnotationAnnotator.hpp"
#include <fstream>
#include "indri/TagExtent.hpp"
#include "indri/ParsedDocument.hpp"
#include <vector>
#include <string.h>
#include <string>
#include <queue>
#include <map>


/************************************
 *  Construction / Destruction 
 ************************************/

indri::parse::OffsetAnnotationAnnotator::OffsetAnnotationAnnotator( Conflater* p_conflater ) { 

  _handler = NULL;
  _p_conflater = p_conflater;
  _first_open = true;
  _indexHintType = OAHintDefault;
	lastReadTag.docno=NULL;

  // allocate the default buffers
  lastBufferAllocationSize=16384;
  _buffers_allocated=new std::vector<char *>(lastBufferAllocationSize);
  _annotations=new indri::utility::HashTable<const char *,std::set<TagExtent*>*>(lastBufferAllocationSize);
  _converted_annotations=new indri::utility::HashTable<const char *,std::set<TagExtent*>*>(lastBufferAllocationSize);
  _tag_id_map=new indri::utility::HashTable<UINT64,TagExtent*>(lastBufferAllocationSize);
  _attribute_id_map=new indri::utility::HashTable<UINT64,AttributeValuePair*>(lastBufferAllocationSize);
}

indri::parse::OffsetAnnotationAnnotator::OffsetAnnotationAnnotator() {

  _handler = NULL;
  _p_conflater = NULL;
  _first_open = true;
  _indexHintType = OAHintDefault;
	lastReadTag.docno=NULL;

  // allocate the default buffers
  lastBufferAllocationSize=16384;
  _buffers_allocated=new std::vector<char *>(lastBufferAllocationSize);
  _annotations=new indri::utility::HashTable<const char *,std::set<TagExtent*>*>(lastBufferAllocationSize);
  _converted_annotations=new indri::utility::HashTable<const char *,std::set<TagExtent*>*>(lastBufferAllocationSize);
  _tag_id_map=new indri::utility::HashTable<UINT64,TagExtent*>(lastBufferAllocationSize);
  _attribute_id_map=new indri::utility::HashTable<UINT64,AttributeValuePair*>(lastBufferAllocationSize);
}

indri::parse::OffsetAnnotationAnnotator::~OffsetAnnotationAnnotator() {
  _cleanup();

	// if the file is still open, close it and clean it
	if (annotationFile.is_open()) {
		annotationFile.clear();
		annotationFile.close();
	}

  delete _buffers_allocated;
  delete _annotations;
  delete _converted_annotations;
  delete _tag_id_map;
  delete _attribute_id_map;
}

/************************************
 *  Private Functions 
 ************************************/

const char *indri::parse::OffsetAnnotationAnnotator::_getDocno( indri::api::ParsedDocument* document ) {

  // find DOCNO attribute in document
  const char *retVal = NULL;

  for ( size_t i=0; i<document->metadata.size(); i++ ) {
    const char* attributeName = document->metadata[i].key;
    const char* attributeValue = (const char*) document->metadata[i].value;

    if ( ! strcmp( attributeName, "docno" ) ) retVal = attributeValue;
  }

  return retVal;
}


indri::parse::TagExtent *indri::parse::OffsetAnnotationAnnotator::_getTag( UINT64 id ) {

  // return TagExtent corresponding to given id, or NULL if invalid id.

  if ( id == 0 ) return NULL;

  TagExtent** p = _tag_id_map->find( id );

  if ( ! p ) return NULL;
  else return *p;
}

indri::parse::AttributeValuePair *indri::parse::OffsetAnnotationAnnotator::_getAttribute( UINT64 id ) {

  // return Attribute corresponding to given id, or NULL if invalid id.

  if ( id == 0 ) return NULL;

  AttributeValuePair** p = _attribute_id_map->find( id );

  if ( ! p ) return NULL;
  else return *p;
}

bool indri::parse::OffsetAnnotationAnnotator::_is_unique_id( UINT64 id, int line ) {

  // Make sure ID has not already been used.

  if ( id == 0 ) {

    std::cerr << "WARN: Invalid id '0' used on line " << line 
              << "; ignoring line." << std::endl;
    return false;
  }

  if ( _getTag( id ) || _getAttribute( id ) ) {

    std::cerr << "WARN: Id '" << id << "' redefined on line " << line 
              << "; ignoring line." << std::endl;
    return false;
  }

  return true;
}

UINT64 indri::parse::OffsetAnnotationAnnotator::parse_UINT64( const char *str, int n ) {

  UINT64 result = 0;
  int i = 0;
  for ( const char* c = str; i < n && *c != '\0'; c++, i++ )
    result = result * 10 + ( *c - '0' );

  return result;
}

void indri::parse::OffsetAnnotationAnnotator::_cleanup() {
  // clear any allocated buffers
  for ( std::vector<char *>::iterator i = _buffers_allocated->begin();
        i != _buffers_allocated->end(); i++ )
    delete[] (*i);

  _buffers_allocated->clear();

  // Cleanup _annotations, _converted_annotations, _tag_id_map,
  // and _attribute_id_map in preparation for object
  // destruction, or for an open call on a new offset
  // annotations file.

  for ( indri::utility::HashTable<const char *,std::set<TagExtent*>*>::iterator i = _annotations->begin(); i != _annotations->end(); i++ ) {

    std::set<TagExtent*>* p_set = *(*i).second;

    for ( std::set<TagExtent*>::iterator j = p_set->begin(); 
          j != p_set->end(); j++ ) {

      delete (*j); // TagExtent
    }
    delete(p_set);
  }

  _annotations->clear();

  for ( indri::utility::HashTable<const char *,std::set<TagExtent*>*>::iterator i = _converted_annotations->begin(); i != _converted_annotations->end(); i++ ) {

    std::set<TagExtent*>* p_set = *(*i).second;

    for ( std::set<TagExtent*>::iterator j = p_set->begin(); 
          j != p_set->end(); j++ ) {

      delete (*j); // TagExtent
    }
    delete (*i).first;
    delete(p_set);
  }

  _converted_annotations->clear();

  // Note: every TagExtent pointed to by an element of the
  // _tag_id_map, and every AttributeValuePair pointed to by an
  // element of the _attribute_id_map will have already been
  // deleted above.

  _tag_id_map->clear();
  _attribute_id_map->clear();

}

void indri::parse::OffsetAnnotationAnnotator::convert_annotations( std::set<indri::parse::TagExtent*>* raw_tags,
                                                                   std::set<indri::parse::TagExtent*>* converted_tags, 
                                                                   indri::api::ParsedDocument* document ) {

  // At the top of this priority queue will be the tag that closes first.
  std::priority_queue<indri::parse::TagExtent*,std::vector<indri::parse::TagExtent*>,indri::parse::TagExtent::lowest_end_first> active_tags;

  std::set<indri::parse::TagExtent*>::iterator curr_raw_tag = raw_tags->begin();

  // to map the parent pointers
  std::map<indri::parse::TagExtent*, indri::parse::TagExtent*> tagMap;

  long tok_pos = 0;

  for ( indri::utility::greedy_vector<indri::parse::TermExtent>::iterator 
          token = document->positions.begin(); 
        token != document->positions.end();
        token++, tok_pos++ ) {

    // Check to see if there are any active tags that can be closed.
    // The top element on active_tags will always be the tag that
    // closes first.

    while ( ! active_tags.empty() &&
            active_tags.top()->end <= (*token).end ) {

      // Tag on top of queue closes before the end of the current
      // token.

      TagExtent *te = active_tags.top();
    
      // If the current active tag ends before the beginning of this
      // token:

      if ( te->end <= (*token).begin ) {

        te->end = tok_pos;

      } else { 
        te->end = tok_pos + 1; // Round up to next token boundary.
      }

      // ensure tag boundaries are still within the document
      if (te->end > (int)document->positions.size()) {
        te->end = (int)document->positions.size();
      } 
      if (te->end < 1) {
        te->end = 1;
      }

      converted_tags->insert( te );
      active_tags.pop();
    }

    // Now check to see if there are any tags that can be activated at
    // this token position.  Tags in raw_tags are sorted in first
    // and longest order.  To be able to activate a tag here, that tag
    // must begin before the current token ends.  Said in another way,
    // the tag must begin before the token begins or inside the token
    // to be activated here.

    while ( curr_raw_tag != raw_tags->end() && (*curr_raw_tag)->begin < (*token).end ) {

      // Tags that begin AND end before the current token are zero
      // token-length tags and must be skipped.
      if ( (*curr_raw_tag)->end <= (*token).begin ) {

        curr_raw_tag++;
        continue;
      }

      // To activate a tag, create a copy of it and insert that copy
      // into active_tags

      TagExtent* te = new TagExtent();
      te->name = (*curr_raw_tag)->name;
      te->number = (*curr_raw_tag)->number;
      te->parent = (*curr_raw_tag)->parent;
      te->attributes = (*curr_raw_tag)->attributes;

      // store this map so that we can convert the parent pointers
      tagMap[ *curr_raw_tag ] = te;

      // When the tag begins inside the token, we activate the tag at 
      // this token position

      te->begin = tok_pos;
      
      // Make sure tag boundaries are within the document
      if (te->begin >= (int)document->positions.size()) {
        te->begin = document->positions.size() - 1;
      } 
      if (te->begin < 0) {
        te->begin = 0;
      }      

      // End value must be filled in when the tag is closed; for now,
      // we just store the byte offset of the end of the tag.
      te->end = (*curr_raw_tag)->end;
     
      active_tags.push( te );

      // Move onto the next tag
      curr_raw_tag++;

    }
  }

  // Close any tags that remain on the active_tags list.

  while ( ! active_tags.empty() ) {

    TagExtent *te = active_tags.top();
    te->end = tok_pos;
    converted_tags->insert( te );
    active_tags.pop();
    
  }

  // Map the parent pointers to point to the right tag!
  std::set<indri::parse::TagExtent*>::iterator converted = converted_tags->begin();
  std::set<indri::parse::TagExtent*>::iterator convertedEnd = converted_tags->end();
  while( converted != convertedEnd ) {
    if ( (*converted)->parent != 0 ) {
      std::map<indri::parse::TagExtent*, indri::parse::TagExtent*>::iterator iter = 
        tagMap.find( (*converted)->parent );
      if ( iter != tagMap.end() ) {
        (*converted)->parent = iter->second;
      } else {
      (*converted)->parent = 0;
      }
    }
    converted++;
  }

}

indri::parse::OffsetAnnotationAnnotator::ReadAnnotationTag indri::parse::OffsetAnnotationAnnotator::parseLine(char *readLine, int lineCounter) {
  ReadAnnotationTag retTag;

  char field[256];

  int fieldStart = 0;
  int fieldCount = 0;
  int fieldOffset = 0;

  int len = 0;

  // set the return tag default values
  retTag.docno=NULL;
  retTag.name=NULL;
  retTag.s_value=NULL;
  retTag.type=0;
  retTag.id=0;
  retTag.i_value=0;
  retTag.parent=0;

  for ( char *c = readLine + fieldStart; *c != '\0' && fieldCount < 8 && fieldOffset < sizeof(field); c++, fieldOffset++ ) {

    if ( *c == '\t' ) {

      field[fieldOffset] = '\0';
            
      switch ( fieldCount ) {

      case 0: // DOCNO (string)
        len = strlen( field );
        retTag.docno = new char[len + 1];
        strncpy( retTag.docno, field, len);
        retTag.docno[len] = '\0';
        _buffers_allocated->push_back( retTag.docno );
        break;

      case 1: // TYPE (flag)
        if ( *field == 't' || *field == 'T' ) retTag.type = 1;
        else if ( *field == 'a' || *field == 'A' ) retTag.type = 2;
        else {
          std::cerr << "WARN: Could not understand type specification '" 
                    << field << "' on line " << lineCounter 
                    << "; ignoring line." << std::endl;
          fieldCount = 8;
          continue;
        }
        break;

      case 2: // ID (UINT64)
        retTag.id = parse_UINT64( field, fieldOffset );
        break;

      case 3: // NAME (string)
        len = strlen( field );
        retTag.name = new char[len + 1];
        strncpy( retTag.name, field, len );
        retTag.name[len] = '\0';
        // name should be case normalized to lower case.
        for (char *c = retTag.name; *c; c++) *c = tolower(*c);
        _buffers_allocated->push_back( retTag.name );
        break;

      case 4: // START (int)
        if ( retTag.type == 1 ) {
          retTag.start = atoi( field );
          if ( retTag.start < 0 ) {
            std::cerr << "WARN: tag named '" << retTag.name 
                      << "' starting at negative byte offest on line " 
                      << lineCounter << "; ignoring line." << std::endl;
            fieldCount = 8; 
            continue;
          }
        }
        break;

      case 5: // LENGTH (int)
        if ( retTag.type == 1 ) {
          retTag.length = atoi( field );
          if ( retTag.length <= 0 ) {
            std::cerr << "WARN: tag named '" << retTag.name 
                      << "' with zero or negative byte length on line " 
                      << lineCounter << "; ignoring line." << std::endl;
            fieldCount = 8;
            continue;
          }
        }
        break;

      case 6: // VALUE (UINT64 or string)
        if ( retTag.type == 1 ) retTag.i_value = parse_UINT64( field, fieldOffset );
        else { 
          len = strlen( field );
          retTag.s_value = new char[len + 1];
          strncpy( retTag.s_value, field, len );
          retTag.s_value[len] = '\0';
          _buffers_allocated->push_back( retTag.s_value );
        }
        break;

      case 7: // PARENT (UINT64; 0 indicates no parent );
        retTag.parent = parse_UINT64( field, fieldOffset );
        break;

      }

      fieldCount++;
      fieldStart += ( fieldOffset + 1 );
      fieldOffset = -1;

    } else {

      field[fieldOffset] = *c;
    }
  } // end for ( char *c = buf + fieldStart; *c != '\0' && fieldCount < 8 && 

	lastReadTag=retTag;
  return retTag;
}

void indri::parse::OffsetAnnotationAnnotator::readAnnotationTags(const char *docno) {

	char buf[65536]; // these may not be big enough someday.

  // read in the next set of OA tags...
	// if we've not read in anything - at least read in the first tag...

	if ((lastReadTag.docno==NULL) && (annotationFile.is_open()) && (!annotationFile.eof())) {
		annotationFile.getline( buf, sizeof(buf) - 1 );
		ReadAnnotationTag thisTag=parseLine(buf, offsetAnnotationFileLine );
	}

	// clean any existing annotations
	_cleanup();

	// keep reading tags until we're not in the expected document
	while ((lastReadTag.docno!=NULL) && (docno!=NULL)&& (!strcmp(docno, lastReadTag.docno)) && (!annotationFile.eof())) {

		if ( lastReadTag.id == lastReadTag.parent ) {

			std::cerr << "WARN: id and parent id are equal on line " << offsetAnnotationFileLine  
								<< "; ignoring line." << std::endl;
			offsetAnnotationFileLine++;
			continue;
		}

		// Now process the line we just read in.

		if ( lastReadTag.type == 1 ) {
	          
			if ( ! _is_unique_id( lastReadTag.id, offsetAnnotationFileLine  ) ) { offsetAnnotationFileLine ++; continue; }

			// Check that parent id is defined

			TagExtent *p_parent = NULL;

			if ( lastReadTag.parent != 0 ) {

				p_parent = _getTag( lastReadTag.parent );

				if ( p_parent == NULL ) {

					std::cerr << "WARN: Undefined parent id '" << lastReadTag.parent 
										<< "' used on line " << offsetAnnotationFileLine  
										<< "; ignoring line" << std::endl;

					offsetAnnotationFileLine ++;
					continue;

				}
			}

			// Create new TAG

			std::set<indri::parse::TagExtent*>** p = _annotations->find( lastReadTag.docno );

			std::set<indri::parse::TagExtent*>* tags = p ? *p : NULL;

			if ( ! tags ) { 
				tags = new std::set<indri::parse::TagExtent*>;
				_annotations->insert( lastReadTag.docno, tags );
			}

			TagExtent* te = new TagExtent;
			te->name = lastReadTag.name;
			te->number = lastReadTag.i_value;
			te->parent = p_parent;

			// Note that this is an abuse of the TagExtent's semantics.  The
			// begin and end fields are intended to be filled with token
			// positions, but here we are inserting byte positions.  In the
			// transform function, all of the annotations for a particular
			// document will be converted to token extents, en masse.  The
			// positions vector from the ParsedDocument is required to make
			// this conversion.
			te->begin = lastReadTag.start;
			te->end = lastReadTag.start + lastReadTag.length;

					// Conflate tag if necessary
			if ( _p_conflater ) _p_conflater->conflate( te );
	        
			tags->insert( te );
			_tag_id_map->insert( lastReadTag.id, te );

		} else if ( lastReadTag.type == 2 ) {

			// Add attribute to existing TAG

			TagExtent* p_te = _getTag( lastReadTag.parent );

			if ( p_te == NULL ) {

				std::cerr << "WARN: Attribute for undefined tag id '" << lastReadTag.parent
									<< "' appears on line " << offsetAnnotationFileLine 
									<< "; ignoring line" << std::endl;
				offsetAnnotationFileLine++;
				continue;
			}

			AttributeValuePair * avp = new AttributeValuePair;
			avp->attribute = lastReadTag.name;
			avp->value = lastReadTag.s_value;

			p_te->attributes.push_back( *avp );
			_attribute_id_map->insert( lastReadTag.id, avp );

		}

		// if we've made it here, we've had a match - 
		// get the next line from the file
		annotationFile.getline( buf, sizeof(buf) - 1 );

		if ( buf[0] == '\0' ) break;

		// parse the line
		// this will automatically set lastReadTag
		ReadAnnotationTag thisTag=parseLine(buf, offsetAnnotationFileLine );

		offsetAnnotationFileLine ++;
	}

	// if we're done - close the file.
	if (annotationFile.eof()) {
		annotationFile.clear();
		annotationFile.close();
		lastReadTag.docno=NULL;
	}
}

/************************************
 *  Public Functions 
 ************************************/

void indri::parse::OffsetAnnotationAnnotator::setHint(indri::parse::OffsetAnnotationIndexHint hintType) {
  _indexHintType=hintType;
}

void indri::parse::OffsetAnnotationAnnotator::setConflater(Conflater* p_conflater) {
  _p_conflater = p_conflater;
}

void indri::parse::OffsetAnnotationAnnotator::setHandler( ObjectHandler<indri::api::ParsedDocument>& handler ) {

  _handler = &handler;
}

void indri::parse::OffsetAnnotationAnnotator::handle( indri::api::ParsedDocument* document ) {

  _handler->handle( transform( document ) );
}

void indri::parse::OffsetAnnotationAnnotator::open( const std::string& offsetAnnotationsFile ) {
  
  // Only re-load this data if the new file is *different* from
  // the old file.

  if ( _offsetAnnotationsFile.compare( offsetAnnotationsFile ) == 0 )
    return;

  _offsetAnnotationsFile = offsetAnnotationsFile;

  if ( ! _first_open ) {

    _cleanup();
  }

  _first_open = false;

  // Load file, and check consistency.  Ensure that there are no
  // undefined parent ids.

  char buf[65536]; // these may not be big enough someday.

  annotationFile.open( offsetAnnotationsFile.c_str() );
	offsetAnnotationFileLine = 1;

  if (!annotationFile.is_open()) {
    std::cerr << "WARN: Attribute file " << offsetAnnotationsFile
              << "could not be opened. Ignoring annotations." << std::endl;    
    _cleanup();
    return;
  }

  /*********************
   
   New annotation processing:

   switch - to choose 1 of 2 options via parameters:

   1) unsure of order of annotations
    a) scan through annotations file to get count of annotations
    b) pre-size the hash table accordingly
    c) load in normally

   2) order of annotations is in doc order
    a) open only sets a flag
    b) add annotations as docId is needed
    c) close on completion


  *******************/

  
  if (_indexHintType!=OAHintOrderedAnnotations) {
  // default - unsure of order - 
  // perform a scan to see how large to set the buffer...

  int lineCounter=0;
  while ( annotationFile.good() || ! annotationFile.eof() ) {
    annotationFile.getline( buf, sizeof(buf) - 1 );
    lineCounter++;
  }
  annotationFile.clear();                   // forget we hit the end of file
  annotationFile.seekg(0, std::ios::beg);   // move to the start of the file

  // pre-allocate maps
  //_annotations.clear();
  //_converted_annotations.clear();
  //_tag_id_map.clear();
  //_attribute_id_map.clear();
  // all are type: indri::utility::HashTable

  // reset the buffer sizes (if needed)
  if (lastBufferAllocationSize < lineCounter) {

    _cleanup();

    delete _buffers_allocated;
    delete _annotations;
    delete _converted_annotations;
    delete _tag_id_map;
    delete _attribute_id_map;

    lastBufferAllocationSize=lineCounter;
    _buffers_allocated=new std::vector<char *>(lastBufferAllocationSize);
    _annotations=new indri::utility::HashTable<const char *,std::set<TagExtent*>*>(lastBufferAllocationSize);
    _converted_annotations=new indri::utility::HashTable<const char *,std::set<TagExtent*>*>(lastBufferAllocationSize);
    _tag_id_map=new indri::utility::HashTable<UINT64,TagExtent*>(lastBufferAllocationSize);
    _attribute_id_map=new indri::utility::HashTable<UINT64,AttributeValuePair*>(lastBufferAllocationSize);
  }


  // Primary parsing loop:
  while ( annotationFile.good() || ! annotationFile.eof() ) {

    // get the line from the file
    annotationFile.getline( buf, sizeof(buf) - 1 );

    if ( buf[0] == '\0' ) break;

    // parse the line
    ReadAnnotationTag thisTag=parseLine(buf, offsetAnnotationFileLine );

    if ( thisTag.id == thisTag.parent ) {

      std::cerr << "WARN: id and parent id are equal on line " << offsetAnnotationFileLine  
                << "; ignoring line." << std::endl;
      offsetAnnotationFileLine++;
      continue;
    }

    // Now process the line we just read in.

    if ( thisTag.type == 1 ) {
            
      if ( ! _is_unique_id( thisTag.id, offsetAnnotationFileLine  ) ) { offsetAnnotationFileLine ++; continue; }

      // Check that parent id is defined

      TagExtent *p_parent = NULL;

      if ( thisTag.parent != 0 ) {

        p_parent = _getTag( thisTag.parent );

        if ( p_parent == NULL ) {

          std::cerr << "WARN: Undefined parent id '" << thisTag.parent 
                    << "' used on line " << offsetAnnotationFileLine  
                    << "; ignoring line" << std::endl;

          offsetAnnotationFileLine ++;
          continue;

        }
      }

      // Create new TAG

      std::set<indri::parse::TagExtent*>** p = _annotations->find( thisTag.docno );

      std::set<indri::parse::TagExtent*>* tags = p ? *p : NULL;

      if ( ! tags ) { 
        tags = new std::set<indri::parse::TagExtent*>;
        _annotations->insert( thisTag.docno, tags );
      }

      TagExtent* te = new TagExtent;
      te->name = thisTag.name;
      te->number = thisTag.i_value;
      te->parent = p_parent;

      // Note that this is an abuse of the TagExtent's semantics.  The
      // begin and end fields are intended to be filled with token
      // positions, but here we are inserting byte positions.  In the
      // transform function, all of the annotations for a particular
      // document will be converted to token extents, en masse.  The
      // positions vector from the ParsedDocument is required to make
      // this conversion.
      te->begin = thisTag.start;
      te->end = thisTag.start + thisTag.length;

          // Conflate tag if necessary
      if ( _p_conflater ) _p_conflater->conflate( te );
          
      tags->insert( te );
      _tag_id_map->insert( thisTag.id, te );

    } else if ( thisTag.type == 2 ) {

      // Add attribute to existing TAG

      TagExtent* p_te = _getTag( thisTag.parent );

      if ( p_te == NULL ) {

        std::cerr << "WARN: Attribute for undefined tag id '" << thisTag.parent
                  << "' appears on line " << offsetAnnotationFileLine 
                  << "; ignoring line" << std::endl;
        offsetAnnotationFileLine++;
        continue;
      }

      AttributeValuePair * avp = new AttributeValuePair;
      avp->attribute = thisTag.name;
      avp->value = thisTag.s_value;

      p_te->attributes.push_back( *avp );
      _attribute_id_map->insert( thisTag.id, avp );

    }

    offsetAnnotationFileLine ++;
  }

  annotationFile.close();
  } // end if (_indexHintType!=OAHintOrderedAnnotations)
}

void indri::parse::OffsetAnnotationAnnotator::setTags (const char *docno, const std::vector<indri::parse::TagExtent *> &tagset) 
{
  // Create new TAG

  std::set<indri::parse::TagExtent*>** p = _annotations->find( docno );

  std::set<indri::parse::TagExtent*>* tags = p ? *p : NULL;

  if ( ! tags ) { 

    tags = new std::set<indri::parse::TagExtent*>;
    _annotations->insert( docno, tags );
  }

  for (unsigned int i = 0; i < tagset.size(); i++) {
    const TagExtent *source = tagset[i];
    TagExtent* te = new TagExtent;
    char *myName = new char[strlen(source->name) + 1];
    strcpy(myName, source->name);
    _buffers_allocated->push_back( myName );
    te->name = myName;    
    te->number = source->number;
    te->parent = source->parent;
    te->begin = source->begin;
    te->end = source->end;

    // Conflate tag if necessary
    if ( _p_conflater ) _p_conflater->conflate( te );
          
    tags->insert( te );
  }
}


indri::api::ParsedDocument* indri::parse::OffsetAnnotationAnnotator::transform( indri::api::ParsedDocument* document ) {

  const char *docno = _getDocno( document ); 
  std::set<indri::parse::TagExtent*>** p;

  //*****************//
  // check here to see what style we're using:
  // 1) standard - no changes ("unordered annotations")
  // 2) ordered annotations - instead of the below "find"
  //    create a set of annotations by reading the file (if opened)
	//    for only the matched docno via readAnnotationTags

  if (_indexHintType==OAHintOrderedAnnotations) {
		readAnnotationTags(docno);
  }

  // First, check if the annotations for this document have already been
  // converted to use token extents.

  p = _converted_annotations->find( docno );
  std::set<indri::parse::TagExtent*>* converted_tags = p ? *p : NULL;

  if ( ! converted_tags ) {

    // We must do the conversion, then.

    converted_tags = new std::set<indri::parse::TagExtent*>;

    // Check if we have any annotations for this document

    p = _annotations->find( docno );
    std::set<indri::parse::TagExtent*>* raw_tags = p ? *p : NULL;

    if ( raw_tags && ! raw_tags->empty() ) {

      // Do the conversion.
      convert_annotations( raw_tags, converted_tags, document );

    }

    // Store newly converted tags back to the converted annotations table. 
    //    _converted_annotations.insert( docno, converted_tags );
  }

  // Return right away if there are no annotations for this document.
  if ( converted_tags->empty() ) {
    //never inserted in _converted_annotations, delete it
    delete(converted_tags);
    return document;
  }
  // Add annotations from the offset annotations file to
  // the ParsedDocument rep.

  for ( std::set<TagExtent*>::iterator i = converted_tags->begin(); 
        i != converted_tags->end(); i++ ) {
    document->tags.push_back( (*i) );
  }
  // resort in case there were any annotations inline.
  std::sort( document->tags.begin(), document->tags.end(), indri::parse::LessTagExtent() );
  converted_tags->clear();
  //never inserted in _converted_annotations, delete it
  delete(converted_tags);
  return document;
}

