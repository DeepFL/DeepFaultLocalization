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
// QueryResponseUnpacker
//
// 23 March 2004 -- tds
//

#ifndef INDRI_QUERYRESPONSEUNPACKER_HPP
#define INDRI_QUERYRESPONSEUNPACKER_HPP

#include "indri/NetworkMessageStream.hpp"
#include "indri/InferenceNetwork.hpp"
#include "lemur/Exception.hpp"
namespace indri
{
  namespace net
  {
    class QueryResponseUnpacker : public MessageStreamHandler {
    private:
      NetworkMessageStream* _stream;
      indri::infnet::InferenceNetwork::MAllResults _results;
      std::string _exception;
      bool _done;

    public:
      QueryResponseUnpacker( NetworkMessageStream* stream ) :
        _stream(stream),
        _done(false)
      {
      }

      indri::infnet::InferenceNetwork::MAllResults& getResults() {
        while( !_done && _stream->alive() && !_exception.length() )
          _stream->read(*this);

        if( _exception.length() )
          LEMUR_THROW( LEMUR_RUNTIME_ERROR, _exception );
    
        return _results;
      }

      void reply( indri::xml::XMLNode* node ) {
        assert( false && "Query responses are binary only for now" );
      }

      void reply( const std::string& name, const void* buffer, unsigned int length ) {
        std::string nodeName;
        std::string listName;
    
        nodeName = name.substr( 0, name.find(':') );
        listName = name.substr( name.find(':')+1 );
    
        indri::api::ScoredExtentResult aligned;
        int count = length / (sizeof(INT32)*5 + sizeof(double) + sizeof(INT64));
        std::vector<indri::api::ScoredExtentResult>& resultVector = _results[nodeName][listName];
    
        const char* p = (const char*) buffer;

        for( int i=0; i<count; i++ ) {
          // copy for alignment
          memcpy( &aligned.score, p, sizeof(double) );
          p += sizeof(double);

          memcpy( &aligned.document, p, sizeof(INT32) );
          p += sizeof(INT32);

          memcpy( &aligned.begin, p, sizeof(INT32) );
          p += sizeof(INT32);

          memcpy( &aligned.end, p, sizeof(INT32) );
          p += sizeof(INT32);

          memcpy( &aligned.number, p, sizeof(UINT64) );
          p += sizeof(INT64);

          memcpy( &aligned.ordinal, p, sizeof(INT32) );
          p += sizeof(INT32);

          memcpy( &aligned.parentOrdinal, p, sizeof(INT32) );
          p += sizeof(INT32);

          aligned.begin = ntohl(aligned.begin);
          aligned.end = ntohl(aligned.end);
          aligned.document = ntohl(aligned.document);
          aligned.score = lemur_compat::ntohd(aligned.score);
#ifndef __APPLE__
          aligned.number = lemur_compat::ntohll(aligned.number);
#else
          aligned.number = ntohll(aligned.number);
#endif
          aligned.ordinal = ntohl(aligned.ordinal);
          aligned.parentOrdinal = ntohl(aligned.parentOrdinal);

          resultVector.push_back(aligned);
        }
      }

      void replyDone() {
        _done = true;
      }

      void request( indri::xml::XMLNode* node ) {
        assert( false && "No requests expected from the query server" );
      }

      void error( const std::string& err ) {
        _exception = err;
      }

    };
  }
}

#endif // INDRI_QUERYRESPONSEUNPACKER_HPP


