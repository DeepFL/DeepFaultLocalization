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
// TagList
//
// March 2004 -- metzler
//

#include "indri/Tag.hpp"
#include <stdio.h>
#include <string.h>
#include <indri/greedy_vector>
#include "indri/TagExtent.hpp"
#include <iostream>
#include "indri/MetadataPair.hpp"
#include "indri/Buffer.hpp"

#ifndef _TAGLIST_HPP
#define _TAGLIST_HPP
namespace indri
{
  namespace parse
  {
    
    class TagList {
    private:
      struct tag_entry {
        const char* name;
        const char* conflation;
        int next;
        int begin;
        int end;
      };

      indri::utility::greedy_vector<tag_entry> _tags;
      int _openList;

      // Controls whether the list reconstructs a hierarchy of
      // tags when writeTagList is called.
      bool _findParents;

    public:
      TagList() : _findParents(true) {
        clear();
      }
      
      void setFindParents( bool findParents ) {
        _findParents = findParents;
      }

      void clear() {
        _tags.clear();
        _openList = -1;
      }

      // we assume here that name is more or less immutable
      // so we can store a pointer to it.  This is a reasonable
      // assumption, because if the tag is indexed, its name is
      // in a hash table somewhere, and we can just point to that
      // name copy.
      void addTag(const char *name, const char* conflation, int begin) {
        tag_entry t;
        t.name = name;
        t.conflation = conflation;
        t.begin = begin;
        t.end = -1;
        t.next = _openList;
        _tags.push_back(t);
        _openList = (int)_tags.size()-1;
      }

      void endTag(const char *name, const char* conflation, int end) {
        int list = _openList;
        int prev = -1;
        // finds the most recent open tag of this name
        while( list >= 0 ) {
          tag_entry& entry = _tags[list];

          if( !strcmp( entry.name, name ) ) {
            // found a tag to close
            entry.end = end;
            int next = entry.next;

            // unlink from open list
            if( prev == -1 ) {
              _openList = next;
            } else {
              _tags[prev].next = next;
            }
        
            return;
          } else {
            // this wasn't the tag, so keep looking
            prev = list;
            list = entry.next;
          }
        }
      }

      void writeTagList( indri::utility::greedy_vector<TagExtent *>& tags ) {
        // look through the tags vector; they're already in sorted order by open
        // position.  Only add closed tags.

        for( size_t i=0; i<_tags.size(); i++ ) {
          tag_entry& entry = _tags[i];

          if( entry.end >= 0 ) {// data field might be empty at head of doc
            TagExtent * extent = new TagExtent;
            extent->begin = entry.begin;
            extent->end = entry.end;
            extent->name = entry.conflation;
            extent->number = 0;

            if ( _findParents && (tags.size() > 0)) {
              // find this tag's parent
                TagExtent * parent = tags.back();
                while ( parent != NULL && 
                        parent->end <= extent->begin ) {
                  if ( parent->begin <= extent->begin &&
                       parent->end   >= extent->end ) break;
                  parent = parent->parent;
                }
                extent->parent = parent;
            } else {
              extent->parent = 0;
            }

            tags.push_back(extent);
          }
        }
      }

      // in this case, we'll treat the list of tags in this list
      // as if they were offsets into a metadata list
      void writeMetadataList( indri::utility::greedy_vector<MetadataPair>& pairs, indri::utility::Buffer& buffer, const char* docText ) {
        for( size_t i=0; i<_tags.size(); i++ ) {
          tag_entry& entry = _tags[i];

          if( entry.end > 0 ) {
            MetadataPair pair;
        
            // copy the text into a buffer
            int length = entry.end - entry.begin;
            char* spot = buffer.write(length+1);
            strncpy( spot, docText + entry.begin, length);
            spot[length] = 0;

            pair.key = entry.conflation;
            pair.value = spot;
            pair.valueLength = length+1;

            // docno is special -- its value must be stripped
            if( !strcmp( pair.key, "docno" ) ) {
              pair.stripValue();
            }

            pairs.push_back(pair);
          }
        }
      }

    };
  }
}

    
#endif
