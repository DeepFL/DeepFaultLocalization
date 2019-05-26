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
// QueryServer
//
// 15 March 2004 -- tds
//

#ifndef INDRI_QUERYSERVER_HPP
#define INDRI_QUERYSERVER_HPP

#include "indri/QuerySpec.hpp"
#include "indri/InferenceNetwork.hpp"
#include "indri/DocumentVector.hpp"
#include "lemur/IndexTypes.hpp"
#include <vector>
namespace indri
{
  namespace server
  {
    
    class QueryServerResponse {
    public:
      virtual ~QueryServerResponse() {};
      virtual indri::infnet::InferenceNetwork::MAllResults& getResults() = 0;
    };

    class QueryServerDocumentsResponse {
    public:
      virtual ~QueryServerDocumentsResponse() {};
      virtual std::vector<indri::api::ParsedDocument*>& getResults() = 0;
    };

    class QueryServerMetadataResponse {
    public:
      virtual ~QueryServerMetadataResponse() {};
      virtual std::vector<std::string>& getResults() = 0;
    };

    class QueryServerVectorsResponse {
    public:
      virtual ~QueryServerVectorsResponse() {};
      virtual std::vector<indri::api::DocumentVector*>& getResults() = 0;
    };

    class QueryServerDocumentIDsResponse {
    public:
      virtual ~QueryServerDocumentIDsResponse() {};
      virtual std::vector<lemur::api::DOCID_T>& getResults() = 0;
    };


    class QueryServer {
    public:
      virtual ~QueryServer() {};
      virtual QueryServerResponse* runQuery( std::vector<indri::lang::Node*>& roots, int resultsRequested, bool optimize ) = 0;
      virtual QueryServerDocumentsResponse* documents( const std::vector<lemur::api::DOCID_T>& documentIDs ) = 0;
      virtual QueryServerMetadataResponse* documentMetadata( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::string& attributeName ) = 0;
      virtual QueryServerDocumentsResponse* documentsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues ) = 0;
      virtual QueryServerDocumentIDsResponse* documentIDsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues ) = 0;
      virtual QueryServerMetadataResponse* pathNames( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::vector<int>& pathBegins, const std::vector<int>& pathEnds ) = 0;

      // terms
      virtual INT64 termCount() = 0;
      virtual INT64 termCount( const std::string& term ) = 0;
      virtual INT64 termCountUnique( ) = 0;
      virtual INT64 stemCount( const std::string& stem ) = 0;

      virtual std::string termName( lemur::api::TERMID_T term ) = 0;
      virtual lemur::api::TERMID_T  termID( const std::string& term ) = 0;
      virtual std::string stemTerm(const std::string &term) = 0;
      
      // fields
      virtual std::vector<std::string> fieldList() = 0;
      virtual INT64 termFieldCount( const std::string& term, const std::string& field ) = 0;
      virtual INT64 stemFieldCount( const std::string& stem, const std::string& field ) = 0;

      // documents
      virtual int documentLength( lemur::api::DOCID_T documentID ) = 0;
      virtual INT64 documentCount() = 0;
      virtual INT64 documentCount( const std::string& term ) = 0;
      virtual INT64 documentStemCount( const std::string& term ) = 0;
  
      // document vector
      virtual QueryServerVectorsResponse* documentVectors( const std::vector<lemur::api::DOCID_T>& documentIDs ) = 0;

      // max wildcard terms 
      virtual void setMaxWildcardTerms(int maxTerms) = 0;
    };
  }
}

#endif // INDRI_QUERYSERVER_HPP

