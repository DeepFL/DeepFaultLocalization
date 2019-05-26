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

#include "indri/LocalQueryServer.hpp"
#include "indri/QuerySpec.hpp"
#include "lemur/lemur-platform.h"
#include "lemur/lemur-compat.hpp"
#include <vector>

#include "indri/UnnecessaryNodeRemoverCopier.hpp"
#include "indri/ContextSimpleCountCollectorCopier.hpp"
#include "indri/FrequencyListCopier.hpp"
#include "indri/DagCopier.hpp"

#include "indri/InferenceNetworkBuilder.hpp"
#include "indri/InferenceNetwork.hpp"

#include "indri/CompressedCollection.hpp"
#include "indri/delete_range.hpp"
#include "indri/WeightFoldingCopier.hpp"

#include "indri/Appliers.hpp"
#include "indri/ScopedLock.hpp"

#include "indri/TreePrinterWalker.hpp"

#include "indri/DocumentStructure.hpp"

//
// Response objects
//
namespace indri
{
  namespace server
  {
    
    class LocalQueryServerResponse : public QueryServerResponse {
    private:
      indri::infnet::InferenceNetwork::MAllResults _results;

    public:
      LocalQueryServerResponse( const indri::infnet::InferenceNetwork::MAllResults& results ) :
        _results(results) {
      }
  
      indri::infnet::InferenceNetwork::MAllResults& getResults() {
        return _results;
      }
    };

    class LocalQueryServerDocumentsResponse : public QueryServerDocumentsResponse {
    private:
      std::vector<indri::api::ParsedDocument*> _documents;

    public:
      LocalQueryServerDocumentsResponse( const std::vector<indri::api::ParsedDocument*>& results )
        :
        _documents(results)
      {
      }

      // caller's responsibility to delete these results
      std::vector<indri::api::ParsedDocument*>& getResults() {
        return _documents;
      }
    };

    class LocalQueryServerMetadataResponse : public QueryServerMetadataResponse {
    private:
      std::vector<std::string> _metadata;

    public:
      LocalQueryServerMetadataResponse( const std::vector<std::string>& metadata ) :
        _metadata(metadata)
      {
      }

      std::vector<std::string>& getResults() {
        return _metadata;
      }
    };

    class LocalQueryServerVectorsResponse : public QueryServerVectorsResponse {
    private:
      std::vector<indri::api::DocumentVector*> _vectors;

    public:
      LocalQueryServerVectorsResponse( int vectorCount ) {
        _vectors.reserve( vectorCount );
      }

      void addVector( indri::api::DocumentVector* vec ) {
        _vectors.push_back( vec );
      }

      // caller deletes indri::api::DocumentVector objects
      std::vector<indri::api::DocumentVector*>& getResults() {
        return _vectors;
      }
    };

    class LocalQueryServerDocumentIDsResponse : public QueryServerDocumentIDsResponse {
    private:
      std::vector<lemur::api::DOCID_T> _documentIDs;

    public:
      LocalQueryServerDocumentIDsResponse( const std::vector<lemur::api::DOCID_T>& documents ) : 
        _documentIDs(documents)
      {
      }

      std::vector<lemur::api::DOCID_T>& getResults() {
        return _documentIDs;
      }
    };
  }
}

//
// Class code
//

indri::server::LocalQueryServer::LocalQueryServer( indri::collection::Repository& repository ) :
  _repository(repository), _maxWildcardMatchesPerTerm(indri::infnet::InferenceNetworkBuilder::DEFAULT_MAX_WILDCARD_TERMS)
{
  // if supplied and false, turn off optimization for all queries.
  _optimizeParameter = indri::api::Parameters::instance().get( "optimize", true );
}

//
// _indexWithDocument
//

indri::index::Index* indri::server::LocalQueryServer::_indexWithDocument( indri::collection::Repository::index_state& indexes, lemur::api::DOCID_T documentID ) {
  for( size_t i=0; i<indexes->size(); i++ ) {
    indri::thread::ScopedLock lock( (*indexes)[i]->statisticsLock() );
    lemur::api::DOCID_T lowerBound = (*indexes)[i]->documentBase();
    lemur::api::DOCID_T upperBound = (*indexes)[i]->documentMaximum();
    
    if( lowerBound <= documentID && upperBound > documentID ) {
      return (*indexes)[i];
    }
  }
  
  return 0;
}

//
// document
//

indri::api::ParsedDocument* indri::server::LocalQueryServer::document( lemur::api::DOCID_T documentID ) {
  indri::collection::CompressedCollection* collection = _repository.collection();
  indri::api::ParsedDocument* document = collection->retrieve( documentID );
  return document;
}

std::string indri::server::LocalQueryServer::documentMetadatum( lemur::api::DOCID_T documentID, const std::string& attributeName ) {
  indri::collection::CompressedCollection* collection = _repository.collection();
  return collection->retrieveMetadatum( documentID, attributeName );
}

indri::server::QueryServerMetadataResponse* indri::server::LocalQueryServer::documentMetadata( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::string& attributeName ) {
  std::vector<std::string> result;

  std::vector<std::pair<lemur::api::DOCID_T, int> > docSorted;
  for( size_t i=0; i<documentIDs.size(); i++ ) {
    docSorted.push_back( std::make_pair( documentIDs[i], i ) );
  }
  std::sort( docSorted.begin(), docSorted.end() );

  for( size_t i=0; i<docSorted.size(); i++ ) {
    result.push_back( documentMetadatum(docSorted[i].first, attributeName) );
  }

  std::vector<std::string> actual;
  actual.resize( documentIDs.size() );
  for( size_t i=0; i<docSorted.size(); i++ ) {
    actual[docSorted[i].second] = result[i];
  }

  return new indri::server::LocalQueryServerMetadataResponse( actual );
}

indri::server::QueryServerDocumentsResponse* indri::server::LocalQueryServer::documents( const std::vector<lemur::api::DOCID_T>& documentIDs ) {
  std::vector<indri::api::ParsedDocument*> result;
  for( size_t i=0; i<documentIDs.size(); i++ ) {
    result.push_back( document(documentIDs[i]) );
  }
  return new indri::server::LocalQueryServerDocumentsResponse( result );
}

indri::server::QueryServerDocumentsResponse* indri::server::LocalQueryServer::documentsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues ) {
  indri::collection::CompressedCollection* collection = _repository.collection();
  std::vector<indri::api::ParsedDocument*> result;
  
  for( size_t i=0; i<attributeValues.size(); i++ ) {
    std::vector<indri::api::ParsedDocument*> documents = collection->retrieveByMetadatum( attributeName, attributeValues[i] );
    std::copy( documents.begin(), documents.end(), std::back_inserter( result ) );
  }

  return new indri::server::LocalQueryServerDocumentsResponse( result );
}

indri::server::QueryServerDocumentIDsResponse* indri::server::LocalQueryServer::documentIDsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues ) {
  indri::collection::CompressedCollection* collection = _repository.collection();
  std::vector<lemur::api::DOCID_T> result;
  
  for( size_t i=0; i<attributeValues.size(); i++ ) {
    std::vector<lemur::api::DOCID_T> documents = collection->retrieveIDByMetadatum( attributeName, attributeValues[i] );
    std::copy( documents.begin(), documents.end(), std::back_inserter( result ) );
  }

  return new indri::server::LocalQueryServerDocumentIDsResponse( result );
}

INT64 indri::server::LocalQueryServer::termCount() {
  indri::collection::Repository::index_state indexes = _repository.indexes();
  INT64 total = 0;

  for( size_t i=0; i<indexes->size(); i++ ) {
    indri::thread::ScopedLock lock( (*indexes)[i]->statisticsLock() );
    total += (*indexes)[i]->termCount();
  }

  return total;
}

INT64 indri::server::LocalQueryServer::termCountUnique() {
  indri::collection::Repository::index_state indexes = _repository.indexes();
  INT64 total = 0;

  for( size_t i=0; i<indexes->size(); i++ ) {
    indri::thread::ScopedLock lock( (*indexes)[i]->statisticsLock() );
    total += (*indexes)[i]->uniqueTermCount();
  }

  return total;
}

INT64 indri::server::LocalQueryServer::termCount( const std::string& term ) {
  std::string stem = _repository.processTerm( term );
  // stopwords return a string of length 0, causing Keyfile to throw.
  if( stem.length() != 0 ) {
    return stemCount( stem );
  } else {
    return 0;
  }
}

INT64 indri::server::LocalQueryServer::stemCount( const std::string& stem ) {
  indri::collection::Repository::index_state indexes = _repository.indexes();
  INT64 total = 0;

  for( size_t i=0; i<indexes->size(); i++ ) {
    indri::thread::ScopedLock lock( (*indexes)[i]->statisticsLock() );
    total += (*indexes)[i]->termCount( stem );
  }

  return total;
}

INT64 indri::server::LocalQueryServer::termFieldCount( const std::string& term, const std::string& field ) {
  std::string stem = _repository.processTerm( term );

  if( stem.length() != 0 ) {
    return stemFieldCount( stem, field );
  } else {
    return 0;
  }
}

INT64 indri::server::LocalQueryServer::stemFieldCount( const std::string& stem, const std::string& field ) {
  indri::collection::Repository::index_state indexes = _repository.indexes();
  INT64 total = 0;

  for( size_t i=0; i<indexes->size(); i++ ) {
    indri::thread::ScopedLock lock( (*indexes)[i]->statisticsLock() );
    total += (*indexes)[i]->fieldTermCount( field, stem );
  }

  return total;
}

std::string indri::server::LocalQueryServer::termName( lemur::api::TERMID_T term ) {
  indri::collection::Repository::index_state indexes = _repository.indexes();
  indri::index::Index* index = (*indexes)[0];
  indri::thread::ScopedLock lock( index->statisticsLock() );
  return index->term( term );
}

lemur::api::TERMID_T indri::server::LocalQueryServer::termID( const std::string& term ) {
  indri::collection::Repository::index_state indexes = _repository.indexes();
  indri::index::Index* index = (*indexes)[0];
  std::string processed = _repository.processTerm( term );
  indri::thread::ScopedLock lock( index->statisticsLock() );

  if( processed.length() != 0 ) {
    return index->term( processed.c_str() );
  } else {
    return 0;
  }
}

std::string indri::server::LocalQueryServer::stemTerm( const std::string& term ) {
  std::string stem  = _repository.processTerm(term);
  return stem;
}

std::vector<std::string> indri::server::LocalQueryServer::fieldList() {
  std::vector<std::string> result;
  const std::vector<indri::collection::Repository::Field>& fields = _repository.fields();

  for( size_t i=0; i<fields.size(); i++ ) {
    result.push_back( fields[i].name );
  }

  return result;
}

int indri::server::LocalQueryServer::documentLength( lemur::api::DOCID_T documentID ) {
  indri::collection::Repository::index_state indexes = _repository.indexes();
  indri::index::Index* index = _indexWithDocument( indexes, documentID );

  if( index ) {
    indri::thread::ScopedLock lock( index->statisticsLock() );
    return index->documentLength( documentID );
  }

  return 0;
}

INT64 indri::server::LocalQueryServer::documentCount() {
  indri::collection::Repository::index_state indexes = _repository.indexes();
  INT64 total = 0;
  
  for( size_t i=0; i<indexes->size(); i++ ) {
    indri::thread::ScopedLock lock( (*indexes)[i]->statisticsLock() );
    total += (*indexes)[i]->documentCount();
  }
  
  total -= _repository.deletedList().deletedCount();
  return total;
}

INT64 indri::server::LocalQueryServer::documentCount( const std::string& term ) {
  std::string stem = _repository.processTerm( term );
  return documentStemCount(stem);
}

INT64 indri::server::LocalQueryServer::documentStemCount( const std::string& stem ) {
  indri::collection::Repository::index_state indexes = _repository.indexes();
  INT64 total = 0;
  if( stem.length() == 0 ) return total;
  for( size_t i=0; i<indexes->size(); i++ ) {
    indri::thread::ScopedLock lock( (*indexes)[i]->statisticsLock() );
    total += (*indexes)[i]->documentCount( stem );
  }
  return total;
}

indri::server::QueryServerResponse* indri::server::LocalQueryServer::runQuery( std::vector<indri::lang::Node*>& roots, int resultsRequested, bool optimize ) {

  indri::lang::TreePrinterWalker printer;

  // use UnnecessaryNodeRemover to get rid of window nodes, ExtentAnd nodes and ExtentOr nodes
  // that only have one child and LengthPrior nodes where the exponent is zero
  indri::lang::ApplyCopiers<indri::lang::UnnecessaryNodeRemoverCopier> unnecessary( roots );

  // run the contextsimplecountcollectorcopier to gather easy stats
  indri::lang::ApplyCopiers<indri::lang::ContextSimpleCountCollectorCopier> contexts( unnecessary.roots(), _repository );

  // use frequency-only nodes where appropriate
  indri::lang::ApplyCopiers<indri::lang::FrequencyListCopier> frequency( contexts.roots(), _cache );

  // fold together any nested weight nodes
  indri::lang::ApplyCopiers<indri::lang::WeightFoldingCopier> weight( frequency.roots() );

  // make all this into a dag
  indri::lang::ApplySingleCopier<indri::lang::DagCopier> dag( weight.roots(), _repository );

  std::vector<indri::lang::Node*>& networkRoots = dag.roots();
  // turn off optimization if called with optimize == false
  // turn off optimization if called the Parameter optimize == false
  if( !optimize || !_optimizeParameter ) {
    // we may be asked not to perform optimizations that might
    // drastically change the structure of the tree; for instance,
    // annotation queries may ask for this
    networkRoots = contexts.roots();
  }
  /*
    indri::lang::TreePrinterWalker printer;
    indri::lang::ApplyWalker<indri::lang::TreePrinterWalker> printTree(networkRoots, &printer);
  */

  // build an inference network
  indri::infnet::InferenceNetworkBuilder builder( _repository, _cache, resultsRequested, _maxWildcardMatchesPerTerm );
  indri::lang::ApplyWalker<indri::infnet::InferenceNetworkBuilder> buildWalker( networkRoots, &builder );

  indri::infnet::InferenceNetwork* network = builder.getNetwork();
  indri::infnet::InferenceNetwork::MAllResults result;
  result = network->evaluate();

  return new indri::server::LocalQueryServerResponse( result );
}

indri::server::QueryServerVectorsResponse* indri::server::LocalQueryServer::documentVectors( const std::vector<lemur::api::DOCID_T>& documentIDs ) {
  indri::server::LocalQueryServerVectorsResponse* response = new indri::server::LocalQueryServerVectorsResponse( (int)documentIDs.size() );
  indri::collection::Repository::index_state indexes = _repository.indexes();
  std::map<int, std::string> termIDStringMap;

  for( size_t i=0; i<documentIDs.size(); i++ ) {
    indri::index::Index* index = _indexWithDocument( indexes, documentIDs[i] );

    {
      indri::thread::ScopedLock lock( index->statisticsLock() );
  
      const indri::index::TermList* termList = index->termList( documentIDs[i] );
      indri::api::DocumentVector* result = new indri::api::DocumentVector( index, termList, termIDStringMap );
      delete termList;
      response->addVector( result );
    }
  }

  return response;
}

indri::server::QueryServerMetadataResponse* indri::server::LocalQueryServer::pathNames( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::vector<int>& pathBegins, const std::vector<int>& pathEnds ) {

  int lastDoc = 0;
  indri::index::DocumentStructure docStruct;
  std::vector<std::string> result;

  std::vector<std::pair<lemur::api::DOCID_T, int> > docSorted;
  for( size_t i=0; i<documentIDs.size(); i++ ) {
    docSorted.push_back( std::make_pair( documentIDs[i], i ) );
  }
  std::sort( docSorted.begin(), docSorted.end() );

  for( size_t i=0; i<docSorted.size(); i++ ) {
    indri::collection::Repository::index_state indexes = _repository.indexes();
    bool docStructLoaded = true;
    lemur::api::DOCID_T documentID = docSorted[i].first;
    if ( documentID != lastDoc ) {
      indri::index::Index * index = _indexWithDocument(indexes, documentID);
      const indri::index::TermList * termList = index->termList( documentID );
      if ( termList != 0 ) {
        docStruct.setIndex( *index );
        docStruct.loadStructure( termList->fields() );
        delete termList;
        lastDoc = docStructLoaded;
      } else {
        docStructLoaded = false;
      }       
    }

    std::string path = "";
    if ( docStructLoaded ) {
      path = docStruct.path( docStruct.findLeaf( pathBegins[docSorted[i].second], 
                                                 pathEnds[docSorted[i].second] ) );
    }
    result.push_back( path );
  }

  std::vector<std::string> actual;
  actual.resize( documentIDs.size() );
  for( size_t i=0; i<docSorted.size(); i++ ) {
    actual[docSorted[i].second] = result[i];
  }

  return new indri::server::LocalQueryServerMetadataResponse( actual );
}

//
// setMaxWildcardTerms
//
void indri::server::LocalQueryServer::setMaxWildcardTerms(int maxTerms) {
  _maxWildcardMatchesPerTerm = maxTerms;
}
