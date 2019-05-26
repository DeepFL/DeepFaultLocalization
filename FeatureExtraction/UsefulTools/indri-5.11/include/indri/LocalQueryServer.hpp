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
// LocalQueryServer
//
// 15 March 2004 -- tds
//
// Allows a QueryEnvironment to run queries against a
// local connection (without doing a network hop).
// This is especially useful for testing.
//

#ifndef INDRI_LOCALQUERYSERVER_HPP
#define INDRI_LOCALQUERYSERVER_HPP

#include "indri/QueryServer.hpp"
#include "indri/Collection.hpp"
#include "indri/Repository.hpp"
#include "indri/DocumentVector.hpp"
#include "indri/ListCache.hpp"
namespace indri
{
  /*! \brief Indri query server classes. */
  namespace server
  {
    
    class LocalQueryServer : public QueryServer {
    private:
      // hold the value of the Parameter optimize, so only one call to
      // get is required. Globally disable query optimization if
      // the parameter is false.
      bool _optimizeParameter;
      indri::collection::Repository& _repository;
      indri::lang::ListCache _cache;

      int _maxWildcardMatchesPerTerm;

      indri::index::Index* _indexWithDocument( indri::collection::Repository::index_state& state, lemur::api::DOCID_T documentID );

    public:
      LocalQueryServer( indri::collection::Repository& repository );

      // query
      QueryServerResponse* runQuery( std::vector<indri::lang::Node*>& roots, int resultsRequested, bool optimize );

      // single document queries
      indri::api::ParsedDocument* document( lemur::api::DOCID_T documentID );
      std::string documentMetadatum( lemur::api::DOCID_T documentID, const std::string& attributeName );

      QueryServerDocumentIDsResponse* documentIDsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues );
      QueryServerDocumentsResponse* documentsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues );

      // batch queries
      QueryServerDocumentsResponse* documents( const std::vector<lemur::api::DOCID_T>& documentIDs );
      QueryServerMetadataResponse* documentMetadata( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::string& attributeName );

      QueryServerMetadataResponse* pathNames( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::vector<int>& pathBegins, const std::vector<int>& pathEnds );

      // terms
      INT64 termCount();
      INT64 termCount( const std::string& term );
      INT64 termCountUnique( );
      INT64 stemCount( const std::string& stem );
      std::string termName( lemur::api::TERMID_T term );
      lemur::api::TERMID_T termID( const std::string& term );
      std::string stemTerm(const std::string &term);
      
      // fields
      std::vector<std::string> fieldList();
      INT64 termFieldCount( const std::string& term, const std::string& field );
      INT64 stemFieldCount( const std::string& stem, const std::string& field );

      // documents
      int documentLength( lemur::api::DOCID_T documentID );
      INT64 documentCount();
      INT64 documentCount( const std::string& term );
      INT64 documentStemCount( const std::string& term );

      // vector
      QueryServerVectorsResponse* documentVectors( const std::vector<lemur::api::DOCID_T>& documentIDs );

      ///
      /// \brief sets the maximum number of terms to be generated for a wildcard
      /// term. If the synonym list is greater than this, an exception will be thrown
      /// @param maxTerms the maximum number of terms
      void setMaxWildcardTerms(int maxTerms);

    };
  }
}

#endif // INDRI_LOCALQUERYSERVER_HPP
