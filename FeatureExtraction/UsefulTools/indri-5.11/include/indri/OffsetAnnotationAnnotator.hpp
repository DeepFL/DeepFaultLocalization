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

#ifndef INDRI_OFFSETANNOTATIONANNOTATOR_HPP
#define INDRI_OFFSETANNOTATIONANNOTATOR_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <utility>
#include <algorithm>

#include "indri/Buffer.hpp"
#include "indri/Transformation.hpp"
#include "indri/TagExtent.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/HashTable.hpp"
#include "indri/greedy_vector"
#include "indri/Conflater.hpp"

namespace indri {
  namespace parse {

    enum OffsetAnnotationIndexHint {
      OAHintDefault,
      OAHintOrderedAnnotations,
      OAHintSizeBuffers,
      OAHintNone
    };

    class OffsetAnnotationAnnotator : public Transformation {

    private:

      // structure to hold a read tag
      struct ReadAnnotationTag {
        char *docno;
        char *name;
        char *s_value;
        int type; // TAG = 1, ATTRIBUTE = 2
        UINT64 id;
        UINT64 i_value;
        UINT64 parent;
        int start;
        int length;
      };

      // 
      OffsetAnnotationIndexHint _indexHintType;

      // path to the offset annotation file
      std::string _offsetAnnotationsFile;

      // holds the size of the last allocation of the buffers
      // to see if we should re-size it
      int lastBufferAllocationSize;

      // Before the actual ParsedDocument is read in, we can not
      // convert byte extents from the .oa file to token extents.  The
      // TagExtents in this table have their begin and end values
      // expressed as byte extents, not token extents.
      indri::utility::HashTable<const char *,std::set<TagExtent*>*> *_annotations;

      // After a document's set of annotations has been converted
      // to token extents, we store the result in this table in case
      // someone asks for that same document's annotations again.
      indri::utility::HashTable<const char *,std::set<TagExtent*>*> *_converted_annotations;

      indri::utility::HashTable<UINT64,TagExtent*>          *_tag_id_map;
      indri::utility::HashTable<UINT64,AttributeValuePair*> *_attribute_id_map;

      // vector of stored buffers here to allow for
      // end cleanup of allocated char* structures
      std::vector<char *> *_buffers_allocated;

      bool _first_open;

      ObjectHandler<indri::api::ParsedDocument>* _handler;
      Conflater* _p_conflater;

      std::ifstream annotationFile;
			int offsetAnnotationFileLine;

      
      const char *_getDocno( indri::api::ParsedDocument* document );
      TagExtent *_getTag( UINT64 id );
      AttributeValuePair *_getAttribute( UINT64 id );
      bool _is_unique_id( UINT64 id, int line );
      UINT64 parse_UINT64( const char *str, int n );
      void _cleanup();

      ReadAnnotationTag parseLine(char *readLine, int lineCounter);

			// holds the last annotation tag that was read and parsed
			// (used for ordererd file parsing)
			ReadAnnotationTag lastReadTag;

			// reads in the tags for a document - used for ordered file parsing
			void readAnnotationTags(const char *docno);

      void convert_annotations( std::set<indri::parse::TagExtent*>* raw_tags,
                                std::set<indri::parse::TagExtent*>* converted_tags, 
                                indri::api::ParsedDocument* document );

    public:
      /** Construction / Destruction **/
      OffsetAnnotationAnnotator( Conflater* p_conflater );
      OffsetAnnotationAnnotator();
      ~OffsetAnnotationAnnotator();

      void setTags (const char *docno, const std::vector<indri::parse::TagExtent *> &tagset);
      
      void setConflater(Conflater* p_conflater);
      void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler );
      void handle( indri::api::ParsedDocument* document );

      void setHint(indri::parse::OffsetAnnotationIndexHint hintType);

      void open( const std::string& offsetAnnotationsFile );
      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document );

    };

  }
}

#endif // INDRI_OFFSETANNOTATIONANNOTATOR_HPP

