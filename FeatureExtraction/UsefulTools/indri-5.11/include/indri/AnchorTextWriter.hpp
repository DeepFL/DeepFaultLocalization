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
// AnchorTextWriter
//
// 20 May 2004 -- tds
//

#ifndef INDRI_ANCHORTEXTWRITER_HPP
#define INDRI_ANCHORTEXTWRITER_HPP

#include <iostream>
#include <algorithm>
#include "indri/Path.hpp"
#include "lemur/lemur-compat.hpp"
namespace indri
{
  namespace parse
  {
    /*! Writes anchor text from a parsed document out to a file. */
    class AnchorTextWriter : public ObjectHandler<indri::api::ParsedDocument> {
    private:
      std::ofstream _out;

    public:
      AnchorTextWriter( const std::string& outputPath ) {
        std::string directory = indri::file::Path::directory( outputPath );
        indri::file::Path::make( directory );
        _out.open( outputPath.c_str(), std::ios::out | std::ios::binary);
      }

      ~AnchorTextWriter() {
        _out.close();
      }

      void handle( indri::api::ParsedDocument* document ) {
        indri::utility::greedy_vector<MetadataPair>::iterator iter;

        iter = std::find_if( document->metadata.begin(),
                             document->metadata.end(),
                             MetadataPair::key_equal( "docno" ) );

        const char* docno = (char*)iter->value;

        iter = std::find_if( document->metadata.begin(),
                             document->metadata.end(),
                             MetadataPair::key_equal( "url" ) );

        const char* page = (char*)iter->value;
        const char* url = 0;
        int count = 0;
        int urlEnd = -1;

        // find the third slash, which should occur
        // right after the domain name
        const char* slash = 0;
        if(page)  slash = strchr( page, '/' );
        if(slash) slash = strchr( slash+1, '/' );
        if(slash) slash = strchr( slash+1, '/' );

        size_t domainLength;
        if( slash )
          domainLength = slash - page;
        else
          domainLength = strlen(page);

        // count links
        for( unsigned int i=0; i<document->tags.size(); i++ ) {
          TagExtent& extent = *(document->tags[i]);

          // we only extract absolute urls
          if( !strcmp( extent.name, "absolute-url" ) ||
              !strcmp( extent.name, "relative-url" ) ) {
            url = document->terms[ extent.begin ];
            urlEnd = extent.end;

            // if it has the same domain, throw it out
            //if( url && page && !lemur_compat::strncasecmp( url, page, domainLength ) ) {
            //  url = 0;
            //  urlEnd = -1;
            //}
          } else if( !strcmp( extent.name, "a" ) &&  // this is anchor text
                     url &&                          // we've seen a url
                     urlEnd == extent.begin &&       // this text is associated with an absolute-url
                     extent.end - extent.begin > 0 ) // there is some text here
            {
              count++;
              url = 0;
            }
        }

        // print output
        _out << "DOCNO=" << docno << std::endl;
        _out << "DOCURL=" << page << std::endl;
        _out << "LINKS=" << count << std::endl;
        url = 0;
        urlEnd = -1;

        for( unsigned int i=0; i<document->tags.size(); i++ ) {
          TagExtent& extent = *(document->tags[i]);

          if( !strcmp( extent.name, "absolute-url" ) ||
              !strcmp( extent.name, "relative-url" ) ) {  // this is an absolute url
            url = document->terms[ extent.begin ];
            urlEnd = extent.end;

            // if it has the same domain, throw it out
            //if( url && page && !lemur_compat::strncasecmp( url, page, domainLength ) ) {
            //  url = 0;
            //  urlEnd = -1;
            //}
          } else if( !strcmp( extent.name, "a" ) &&  // this is anchor text
                     url &&                          // we've seen a url
                     urlEnd == extent.begin &&       // this text is associated with an absolute-url
                     extent.end - extent.begin > 0 ) // there is some text here
            {
              int textLength = 0;

              _out << "LINKURL=" << url << std::endl;
              _out << "TEXT=\"";
              for( size_t j=extent.begin; int(j) < extent.end && textLength < 60000; j++ ) {
                if( !document->terms[j] )
                  continue;

                textLength += strlen(document->terms[j])+1;
                _out << document->terms[j] << " ";
              }
              _out << "\"" << std::endl;

              // only do the same link once
              url = 0;
            }
        }
      }
    };
  }
}

#endif // INDRI_ANCHORTEXTWRITER_HPP

