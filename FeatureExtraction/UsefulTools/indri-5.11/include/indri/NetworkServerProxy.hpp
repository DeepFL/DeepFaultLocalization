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
// NetworkServerProxy
//
// 23 March 2004 -- tds
//

#ifndef INDRI_NETWORKSERVERPROXY_HPP
#define INDRI_NETWORKSERVERPROXY_HPP

#include "indri/XMLNode.hpp"
#include "indri/XMLWriter.hpp"
#include "indri/QueryResponseUnpacker.hpp"
#include "indri/QueryServer.hpp"
#include "indri/Packer.hpp"
#include "indri/NetworkMessageStream.hpp"
#include "indri/Buffer.hpp"
namespace indri
{
  namespace server
  {   
    class NetworkServerProxy : public QueryServer {
    private:
      indri::net::NetworkMessageStream* _stream;

      INT64 _numericRequest( indri::xml::XMLNode* node );
      std::string _stringRequest( indri::xml::XMLNode* node );

    public:
      NetworkServerProxy( indri::net::NetworkMessageStream* stream );

      QueryServerResponse* runQuery( std::vector<indri::lang::Node*>& roots, int resultsRequested, bool optimize );
      QueryServerDocumentsResponse* documents( const std::vector<lemur::api::DOCID_T>& documentIDs );
      QueryServerMetadataResponse* documentMetadata( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::string& attributeName );

      QueryServerDocumentIDsResponse* documentIDsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues );
      QueryServerDocumentsResponse* documentsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues );

      QueryServerMetadataResponse* pathNames( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::vector<int>& pathBegins, const std::vector<int>& pathEnds );

      // terms -- implemented (but not on stub)
      INT64 termCount();
      INT64 termCount( int term );
      INT64 termCount( const std::string& term );
      INT64 termCountUnique( );
      INT64 stemCount( const std::string& stem );
      std::string termName( lemur::api::TERMID_T term );
      lemur::api::TERMID_T termID( const std::string& term );
      std::string stemTerm(const std::string &term);

      // fields
      std::vector<std::string> fieldList(); // unimpl
      INT64 termFieldCount( const std::string& term, const std::string& field );
      INT64 stemFieldCount( const std::string& stem, const std::string& field );

      // documents
      int documentLength( lemur::api::DOCID_T documentID );
      INT64 documentCount();
      INT64 documentCount( const std::string& term );
      INT64 documentStemCount( const std::string& term );

      // document vector
      QueryServerVectorsResponse* documentVectors( const std::vector<lemur::api::DOCID_T>& documentIDs );

      void setMaxWildcardTerms(int maxTerms);
    };
  }
}

#endif // INDRI_NETWORKSERVERPROXY_HPP
