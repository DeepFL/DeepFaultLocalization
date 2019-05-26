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
// NetworkServerStub
//
// 23 March 2004 -- tds
//

#ifndef INDRI_NETWORKSERVERSTUB_HPP
#define INDRI_NETWORKSERVERSTUB_HPP

#include "indri/XMLNode.hpp"
#include "indri/NetworkMessageStream.hpp"
#include "indri/QueryServer.hpp"
#include "indri/QueryResponsePacker.hpp"
namespace indri
{
  namespace net
  {
    
    class NetworkServerStub : public MessageStreamHandler {
    private:
      indri::server::QueryServer* _server;
      NetworkMessageStream* _stream;

      indri::xml::XMLNode* _encodeDocument( const struct indri::api::ParsedDocument* document );
      void _decodeMetadataRequest( const class indri::xml::XMLNode* request,
                                   std::string& attributeName,
                                   std::vector<std::string>& attributeValues );
      void _sendDocumentsResponse( class indri::server::QueryServerDocumentsResponse* response );
      void _sendNumericResponse( const char* responseName, UINT64 number );

      void _handleDocuments( indri::xml::XMLNode* input );
      void _handleDocumentMetadata( indri::xml::XMLNode* request );
      void _handleDocumentVectors( indri::xml::XMLNode* request );

      void _handleDocumentIDsFromMetadata( indri::xml::XMLNode* request );
      void _handleDocumentsFromMetadata( indri::xml::XMLNode* request );

      void _handleQuery( indri::xml::XMLNode* input );

      void _handleTermCount( indri::xml::XMLNode* request );
      void _handleTermCountUnique( indri::xml::XMLNode* request );
      void _handleStemCountText( indri::xml::XMLNode* request );
      void _handleTermCountText( indri::xml::XMLNode* request );

      void _handleTermName( indri::xml::XMLNode* request );
      void _handleTermID( indri::xml::XMLNode* request );
      void _handleStemTerm( indri::xml::XMLNode* request );

      void _handleTermFieldCount( indri::xml::XMLNode* request );
      void _handleStemFieldCount( indri::xml::XMLNode* request );
      void _handleDocumentCount( indri::xml::XMLNode* request );
      void _handleDocumentTermCount( indri::xml::XMLNode* request );
      void _handleDocumentStemCount( indri::xml::XMLNode* request );
      void _handleDocumentLength( indri::xml::XMLNode* request );
      void _handleFieldList( indri::xml::XMLNode* request );

      void _handlePathNames( indri::xml::XMLNode* request );

      void _handleSetMaxWildcardTerms( indri::xml::XMLNode* request );

    public:
      NetworkServerStub( indri::server::QueryServer* server, NetworkMessageStream* stream );
      void request( indri::xml::XMLNode* input );
      void reply( indri::xml::XMLNode* input );
      void reply( const std::string& name, const void* buffer, unsigned int length );
      void replyDone();
      void error( const std::string& error );
      void run();
    };
  }
}

#endif // INDRI_NETWORKSERVERSTUB_HPP
