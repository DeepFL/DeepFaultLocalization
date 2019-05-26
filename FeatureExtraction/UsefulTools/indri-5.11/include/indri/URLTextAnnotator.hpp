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
// URLTextAnnotator
//
// 23 September 2006 -- tds
// 
// Reads the URL text from the metadata field, parses it,
// and adds it to the parsed document text for indexing.
//

#ifndef INDRI_URLTEXTANNOTATOR_HPP
#define INDRI_URLTEXTANNOTATOR_HPP

#include <algorithm>
#include "indri/Buffer.hpp"
#include "indri/Transformation.hpp"
#include "indri/TagExtent.hpp"
#include "indri/ParsedDocument.hpp"

/*! Top level namespace for all indri components. */
namespace indri
{
  /*! \brief File input, parsing, stemming, and stopping classes. */
  namespace parse
  {
    /*! Reads the URL text from the metadata field, parses it, and adds it to
        the document text for indexing.
    */    
    class URLTextAnnotator : public Transformation {
      indri::utility::Buffer _buffer;
      ObjectHandler<indri::api::ParsedDocument>* _handler;

    public:
      URLTextAnnotator() {
        _handler = 0;
      }

      ~URLTextAnnotator() {
      }

      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document ) {
        // find the url metadata pair
        indri::utility::greedy_vector<indri::parse::MetadataPair>::iterator iter;
       
        iter = std::find_if( document->metadata.begin(),
                             document->metadata.end(),
                             indri::parse::MetadataPair::key_equal( "url" ) );   
       
        // no need to stick around if there is no url                     
        if( iter == document->metadata.end() )
          return document;                          
        
        // need to copy this into the buffer and parse it:
        _buffer.clear();                                   
        _buffer.grow( iter->valueLength + 1 );
        char* urlText = _buffer.write( iter->valueLength );                  
	// pushes the buffer pointer, trash in urlText
	//        memcpy( _buffer.write( iter->valueLength ), iter->value, iter->valueLength );
        memcpy( urlText, iter->value, iter->valueLength );
        *_buffer.write(1) = '\0';
        
        // now we're pointing to the copied urlText, so we can start parsing
        int urlStart = (int)document->terms.size();
        char* c = urlText;    
        bool lastSkipped = true; 
        bool foundSlash = false;
        int remainingStart = -1;
        
        // skip the beginning stuff (http://)
        for( c = urlText; *c; c++ ) {
          if( *c == '/' && c[1] && c[1] == '/' ) {
            urlText = c + 2;                            
          }
        }
        int cnt = 0;
        
        // now, try to find the 
        for( c = urlText; *c; c++ ) {
          if( (*c >= 'A' && *c <= 'Z') ||
              (*c >= 'a' && *c <= 'z') ||
              (*c >= '0' && *c <= '9') ) 
          {
            if( lastSkipped ) {
	      lastSkipped = false;
              document->terms.push_back( c );
              cnt++;
            }
          } else if( *c == '/' && remainingStart < 0 ) {
            *c = 0;
	    lastSkipped = true;
            remainingStart = document->terms.size();
          } else {
	    lastSkipped = true;
            *c = 0;
          }            
        }

        // put in phony positions entries
        int tokEnd = document->positions.size() ? document->positions[document->positions.size()-1].end : 0;
        for (size_t n = document->terms.size()-cnt; n < document->terms.size(); n++) {
          TermExtent extent;
          extent.begin = tokEnd++; // hope this doesn't run off the end
          extent.end = tokEnd;
          document->positions.push_back( extent );
        }

        // the URL text is now parsed and stored in the document
        // all we need to do now is put some tags around the text.
        TagExtent *url = new TagExtent;
        url->begin = urlStart;
        url->end = document->terms.size();
        url->name = "url";
        url->number = 0;
        document->tags.push_back(url);
                        
        TagExtent *domain = new TagExtent;
        domain->begin = urlStart;
        domain->end = (remainingStart >= 0) ? remainingStart : document->terms.size();
        domain->name = "urldomain";      
        domain->number = 0;
        document->tags.push_back(domain);
        
        if( remainingStart > 0 ) {
          indri::parse::TagExtent *urlpath = new TagExtent;
          urlpath->begin = remainingStart;
          urlpath->end = document->terms.size();
          urlpath->name = "urlpath";
          urlpath->number = 0;
          document->tags.push_back(urlpath);
        }
  
        return document;
      }

      void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler ) {
        _handler = &handler;
      }

      void handle( indri::api::ParsedDocument* document ) {
        _handler->handle( transform( document ) );
      }
    };
  }
}

#endif // INDRI_URLTEXTANNOTATOR_HPP
                                      

