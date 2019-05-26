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
// OffsetMetadataAnnotator
//
// 3 November 2005 -- jcb 
//
// Reads supplied offset metadata file and adds the metadata to
// the parsed document.
//

// Format of the offset metadata file: 3-column, tab-delimited.
// From left-to-right, those columns are: 
//
//   docno     : external doc id for document to annotate (string) (e.g. 10) 
//
//   key       : the key/name of the metadata element (string) (e.g. origURL)
//
//   value     : the value of the metadata element (string) (e.g. http://bla) 

// While the OffsetMetadataAnnotator is transforming the
// ParsedDocument, it will directly operate on the data structures
// just as if it were the Parser, except that it adds metadata from
// its file as opposed to from the original TokenizedDocument text.

#ifndef INDRI_OFFSETMETADATAANNOTATOR_HPP
#define INDRI_OFFSETMETADATAANNOTATOR_HPP

#include "indri/Transformation.hpp"
#include "indri/MetadataPair.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/HashTable.hpp"
#include "indri/greedy_vector"
#include <iostream>
#include <vector>
#include <string.h>
#include <string>

namespace indri {
  namespace parse {

    class OffsetMetadataAnnotator : public Transformation {

    private:
      std::string _offsetMetadataFile;

      indri::utility::HashTable<const char *,indri::utility::greedy_vector<MetadataPair*>*> _annotations;
      std::vector<char *> _buffers_allocated;
      bool _first_open;

      ObjectHandler<indri::api::ParsedDocument>* _handler;
      
      const char *_getDocno( indri::api::ParsedDocument* document ) {
        //Find DOCNO attribute in document
        for( size_t i=0; i<document->metadata.size(); i++ ) {
          const char* attributeName = document->metadata[i].key;
          const char* attributeValue = (const char*) document->metadata[i].value;

          if( ! strcmp( attributeName, "docno" ) ) return attributeValue;
        }
        return NULL;
      }


      void _cleanup() {
        //Cleanup _annotations in preparation for object destruction,
        // or for an open call on a new offset metadata file.
        for( indri::utility::HashTable<const char *,indri::utility::greedy_vector<MetadataPair*>*>::iterator i = _annotations.begin(); 
             i != _annotations.end(); i++ ) {
          indri::utility::greedy_vector<MetadataPair*>* p_vec = *(*i).second;
          for( indri::utility::greedy_vector<MetadataPair*>::iterator j = 
                  p_vec->begin(); j != p_vec->end(); j++ ) {
            delete (*j); //MetadataPair 
          }
        }
        _annotations.clear();
      }

    public:
      OffsetMetadataAnnotator() {
        _handler = NULL;
        _first_open = true;
      }

      ~OffsetMetadataAnnotator() {
        _cleanup();

        for ( std::vector<char *>::iterator i = _buffers_allocated.begin(); i != _buffers_allocated.end(); i++ )
          delete[] (*i);
      }

      void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler ) {
        _handler = &handler;
      }

      void handle( indri::api::ParsedDocument* document ) {
        _handler->handle( transform( document ) );
      }

      // Defined in OffsetMetadataAnnotator.cpp
      void open( const std::string& offsetMetadataFile );
      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document );

    };

  }
}

#endif // INDRI_OFFSETMETADATAANNOTATOR_HPP

