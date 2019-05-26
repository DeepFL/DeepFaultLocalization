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
// QueryEnvironment
//
// 9 March 2004 -- tds
//
#include "indri/indri-platform.h"
#include "indri/QueryEnvironment.hpp"
#include "indri/CompressedCollection.hpp"

#include <vector>
#include <queue>
#include "indri/delete_range.hpp"

#include "indri/InferenceNetwork.hpp"
#include "indri/QuerySpec.hpp"
#include "indri/ScoredExtentResult.hpp"

#include "indri/LocalQueryServer.hpp"
#include "indri/NetworkServerProxy.hpp"
#include "indri/NetworkStream.hpp"
#include "indri/NetworkMessageStream.hpp"

#include "indri/QueryParserFactory.hpp"

#include "indri/DagCopier.hpp"
#include "indri/InferenceNetworkBuilder.hpp"
#include "indri/RawScorerNodeExtractor.hpp"
#include "indri/ContextSimpleCountCollectorCopier.hpp"
#include "indri/TreePrinterWalker.hpp"
#include "indri/ContextCountGraphCopier.hpp"
#include "indri/SmoothingAnnotatorWalker.hpp"
#include "indri/ExtentRestrictionModelAnnotatorCopier.hpp"

#include "indri/InferenceNetworkBuilder.hpp"
#include "indri/Packer.hpp"
#include "indri/Unpacker.hpp"
#include "indri/QueryAnnotation.hpp"

#include "indri/XMLReader.hpp"
#include "indri/IndriTimer.hpp"

#include "indri/IndexEnvironment.hpp"
#include "indri/Index.hpp"

#include <set>
#include <map>

#include "indri/Appliers.hpp"
#include "indri/TreePrinterWalker.hpp"

#include "indri/SnippetBuilder.hpp"

#include "indri/VocabularyIterator.hpp"

#include "indri/QueryTFWalker.hpp"

using namespace lemur::api;

// debug code: should be gone soon
#ifdef TIME_QUERIES
#define INIT_TIMER      indri::utility::IndriTimer t; t.start();
#define PRINT_TIMER(s)  { t.printElapsedMicroseconds( std::cout ); std::cout << ": " << s << std::endl; }
#else
#define INIT_TIMER
#define PRINT_TIMER(s)
#endif

// for debugging; this class prints a query tree
namespace indri
{
  namespace lang
  {
    class Printer : public indri::lang::Walker {
    private:
      int tabs;
    public:
      Printer() : tabs(0) {}

      void defaultBefore( indri::lang::Node* n ) {
        for( int i=0; i<tabs; i++ )
          std::cout << "\t";
        std::cout << n->typeName() << " " << n->nodeName() << " " << n->queryText() << std::endl;
        tabs++;
      }

      void defaultAfter( indri::lang::Node* n ) {
        tabs--;
      }

    };
  }
}

//
// Helper document methods
//

// split a document ID list into sublists, one for each query server
void qenv_scatter_document_ids( const std::vector<DOCID_T>& documentIDs, std::vector< std::vector<DOCID_T> >& docIDLists, std::vector< std::vector<DOCID_T> >& docIDPositions, int serverCount ) {
  docIDLists.resize( serverCount );
  docIDPositions.resize( serverCount );

  for( unsigned int i=0; i<documentIDs.size(); i++ ) {
    DOCID_T id = documentIDs[i];
    int serverID = id % serverCount;

    docIDLists[serverID].push_back( id / serverCount );
    docIDPositions[serverID].push_back(i);
  }
}

// retrieve a list of results from each query server, and fold those results into a master list
template<class _ResponseType, class _ResultType>
void qenv_gather_document_results( const std::vector< std::vector<DOCID_T> >& docIDLists,
                                   const std::vector< std::vector<DOCID_T> >& docIDPositions,
                                   indri::utility::greedy_vector<_ResponseType>& responses,
                                   std::vector<_ResultType>& results ) {
  for( size_t i=0; i<docIDLists.size(); i++ ) {
    if( docIDLists[i].size() ) {
      std::vector<_ResultType> serverResult = responses[i]->getResults();
      delete responses[i];
      
      // fold the document names back into one large result list
      for( size_t j=0; j<docIDLists[i].size(); j++ ) {
        int resultIndex = docIDPositions[i][j];
        results[ resultIndex ] = serverResult[j];
      }
    }
  }
}

//
// QueryEnvironment definition
//

indri::api::QueryEnvironment::QueryEnvironment() : _baseline(false) { 
  reformulator = new indri::query::ReformulateQuery(reformulatorParams);
}

indri::api::QueryEnvironment::~QueryEnvironment() {
  close();
}

void indri::api::QueryEnvironment::setMemory( UINT64 memory ) {
  _parameters.set( "memory", memory );
}

void indri::api::QueryEnvironment::setSingleBackgroundModel( bool background ) {
  _parameters.set( "singleBackgroundModel", background );
}

void indri::api::QueryEnvironment::setScoringRules( const std::vector<std::string>& rules ) {
  _parameters.set("rule","");
  for( unsigned int i=0; i<rules.size(); i++ ) {
    _parameters.append("rule").set( rules[i] );
  }
}

void indri::api::QueryEnvironment::setBaseline( const std::string& baseline ) {
  std::string rule = "method:" + baseline;
  _parameters.set("rule", rule);
  _baseline = true;
}

void indri::api::QueryEnvironment::setStopwords( const std::vector<std::string>& stopwords ) {
  _parameters.set("stopper","");
  Parameters p = _parameters.get("stopper");
  for( unsigned int i=0; i<stopwords.size(); i++ ) {
    p.append("word").set(stopwords[i]);
  }
}

void indri::api::QueryEnvironment::_copyStatistics( std::vector<indri::lang::RawScorerNode*>& scorerNodes, indri::infnet::InferenceNetwork::MAllResults& statisticsResults ) {
  for( size_t i=0; i<scorerNodes.size(); i++ ) {
    std::vector<ScoredExtentResult>& occurrencesList = statisticsResults[ scorerNodes[i]->nodeName() ][ "occurrences" ];
    std::vector<ScoredExtentResult>& contextSizeList = statisticsResults[ scorerNodes[i]->nodeName() ][ "contextSize" ];
    std::vector<ScoredExtentResult>& documentOccurrencesList = statisticsResults[ scorerNodes[i]->nodeName() ][ "documentOccurrences" ];
    std::vector<ScoredExtentResult>& documentCountList = statisticsResults[ scorerNodes[i]->nodeName() ][ "documentCount" ];

    double occurrences = occurrencesList[0].score;
    double contextSize = contextSizeList[0].score;
    int documentOccurrences = int(documentOccurrencesList[0].score);
    int documentCount = int(documentCountList[0].score);

    scorerNodes[i]->setStatistics( occurrences, contextSize, documentOccurrences, documentCount );
  }
}

//
// Runs a query in parallel across all servers, and returns a vector of responses.
// This method will block until all responses have been received.
//

std::vector<indri::server::QueryServerResponse*> indri::api::QueryEnvironment::_runServerQuery( std::vector<indri::lang::Node*>& roots, int resultsRequested ) {
  std::vector<indri::server::QueryServerResponse*> responses;
  
  // this ships out the requests to each server (doesn't necessarily block until they're done)
  for( size_t i=0; i<_servers.size(); i++ ) {
    indri::server::QueryServerResponse* response = _servers[i]->runQuery( roots, resultsRequested, true );
    responses.push_back( response );
  }

  // this just goes through all the results, blocking on each one,
  // making sure they've all arrived
  for( size_t i=0; i<_servers.size(); i++ ) {
    responses[i]->getResults();
  }

  return responses;
}

// 
// This method is used for combining raw scores from ContextCounterNodes.
//

void indri::api::QueryEnvironment::_sumServerQuery( indri::infnet::InferenceNetwork::MAllResults& results, std::vector<indri::lang::Node*>& roots, int resultsRequested ) {
  std::vector<indri::server::QueryServerResponse*> serverResults = _runServerQuery( roots, resultsRequested );
  results.clear();

  indri::infnet::InferenceNetwork::MAllResults::iterator nodeIter;
  indri::infnet::EvaluatorNode::MResults::iterator listIter;

  for( size_t i=0; i<serverResults.size(); i++ ) {
    indri::server::QueryServerResponse* response = serverResults[i];
    indri::infnet::InferenceNetwork::MAllResults& machineResults = response->getResults();
    indri::infnet::InferenceNetwork::MAllResults::iterator iter;

    for( nodeIter = machineResults.begin(); nodeIter != machineResults.end(); nodeIter++ ) {
      for( listIter = nodeIter->second.begin(); listIter != nodeIter->second.end(); listIter++ ) {
        std::vector<indri::api::ScoredExtentResult>& currentResultList = results[ nodeIter->first ][ listIter->first ];
        std::vector<indri::api::ScoredExtentResult>& machineResultList = listIter->second;

        if( currentResultList.size() == 0 ) {
          currentResultList.assign( machineResultList.begin(), machineResultList.end() );
        } else {
          assert( machineResultList.size() == currentResultList.size() );

          for( size_t i=0; i<machineResultList.size(); i++ ) {
            currentResultList[i].score += machineResultList[i].score;
          }
        }
      }
    }
  }

  indri::utility::delete_vector_contents<indri::server::QueryServerResponse*>( serverResults );
}

void indri::api::QueryEnvironment::_mergeQueryResults( indri::infnet::InferenceNetwork::MAllResults& results, std::vector<indri::server::QueryServerResponse*>& responses ) {
  results.clear();

  indri::infnet::InferenceNetwork::MAllResults::iterator nodeIter;
  indri::infnet::EvaluatorNode::MResults::iterator listIter;

  // merge all the results from these machines into one master list
  for( size_t i=0; i<responses.size(); i++ ) {
    indri::server::QueryServerResponse* response = responses[i];
    indri::infnet::InferenceNetwork::MAllResults& machineResults = response->getResults();

    for( nodeIter = machineResults.begin(); nodeIter != machineResults.end(); nodeIter++ ) {
      indri::infnet::EvaluatorNode::MResults& node = nodeIter->second;

      for( listIter = node.begin(); listIter != node.end(); listIter++ ) {
        const std::vector<indri::api::ScoredExtentResult>& partialResultList = listIter->second;
        std::vector<indri::api::ScoredExtentResult>& totalResultList = results[ nodeIter->first ][ listIter->first ];

        for( size_t j=0; j<partialResultList.size(); j++ ) {
          indri::api::ScoredExtentResult singleResult = partialResultList[j];
          singleResult.document = (singleResult.document*int(_servers.size())) + int(i);
          totalResultList.push_back( singleResult );
        }
      }
    }
  }
}

//
// This method is used to merge document results from multiple servers.  It does this
// by reassigning document IDs with the following function:
//      serverCount = _servers.size();
//      cookedDocID = rawDocID * serverCount + docServer;
// So, for document 6 from server 3 (out of 7 servers), the cooked docID would be:
//      (6 * 7) + 3 = 45.
// This function has the nice property that if there is only one server running,
// cookedDocID == rawDocID.
//

void indri::api::QueryEnvironment::_mergeServerQuery( indri::infnet::InferenceNetwork::MAllResults& results, std::vector<indri::lang::Node*>& roots, int resultsRequested ) {
  std::vector<indri::server::QueryServerResponse*> serverResults = _runServerQuery( roots, resultsRequested );
  results.clear();

  indri::infnet::InferenceNetwork::MAllResults::iterator nodeIter;
  indri::infnet::EvaluatorNode::MResults::iterator listIter;

  _mergeQueryResults( results, serverResults );

  // now, for each node, sort the result list, and trim off any results past the
  // requested amount
  for( nodeIter = results.begin(); nodeIter != results.end(); nodeIter++ ) {
    for( listIter = nodeIter->second.begin(); listIter != nodeIter->second.end(); listIter++ ) {
      std::vector<indri::api::ScoredExtentResult>& listResults = listIter->second;
      std::stable_sort( listResults.begin(), listResults.end(), indri::api::ScoredExtentResult::score_greater() );

      if( int(listResults.size()) > resultsRequested )
        listResults.resize( resultsRequested );
    }
  }

  indri::utility::delete_vector_contents<indri::server::QueryServerResponse*>( serverResults );
}

//
// addIndex
//

void indri::api::QueryEnvironment::addIndex( const std::string& pathname ) {
  std::map<std::string, std::pair<indri::server::QueryServer *, indri::collection::Repository *> >::iterator iter;
  iter = _repositoryNameMap.find(pathname);
  if (iter == _repositoryNameMap.end()) { // only add if not present
    indri::collection::Repository* repository = new indri::collection::Repository();
    repository->openRead( pathname, &_parameters );
    _repositories.push_back( repository );
    
    indri::server::LocalQueryServer *server = new indri::server::LocalQueryServer( *repository ) ;
    _servers.push_back( server );
    _repositoryNameMap[pathname] = std::make_pair(server, repository);
  } // else, could throw an Exception, as it is a logical error.
}

//
// addIndex
//
// This just adds a pointer to the repository that's
// owned by some IndexEnvironment object; the 
// repository will not be closed by this QueryEnvironment.
//

void indri::api::QueryEnvironment::addIndex( indri::api::IndexEnvironment& environment ) {
  _servers.push_back( new indri::server::LocalQueryServer( environment._repository ) );
}

//
// removeIndex
//

void indri::api::QueryEnvironment::removeIndex( const std::string& pathname ) {
  // close, delete, and remove opened Repository from _repositories
  // close, delete, and remove opened LocalQueryServer from _servers
  // renumber map entries after removal
  std::map<std::string, std::pair<indri::server::QueryServer *, indri::collection::Repository *> >::iterator iter;
  iter = _repositoryNameMap.find(pathname);
  if (iter != _repositoryNameMap.end()) {
    indri::server::QueryServer * s = iter->second.first;
    indri::collection::Repository * r = iter->second.second;
    for (size_t i = 0; i < _servers.size(); i++) {
      if (_servers[i] == s) {
        delete(_servers[i]);
        _servers.erase(_servers.begin() + i);
        break;
      }
    }
    
    for (size_t i = 0; i < _repositories.size(); i++) {
      if (_repositories[i] == r) {
        delete(_repositories[i]);
        _repositories.erase(_repositories.begin() + i);
        break;
      }
    }
    _repositoryNameMap.erase(iter);
  }
}

//
// addServer
//

void indri::api::QueryEnvironment::addServer( const std::string& hostname ) {
  std::map<std::string, std::pair<indri::server::QueryServer *, indri::net::NetworkStream *> >::iterator iter;
  iter = _serverNameMap.find(hostname);
  if (iter == _serverNameMap.end()) { // only add if not present

    indri::net::NetworkStream* stream = new indri::net::NetworkStream;
    unsigned int port = INDRID_PORT;
    std::string host = hostname;
    int colon = (int)hostname.find(':');

    if( colon > 0 ) {
      host = hostname.substr( 0, colon );
      port = atoi( hostname.substr( colon+1 ).c_str() );
    }

    if( !stream->connect( host.c_str(), port ) ) {
      delete stream;
      throw Exception( "QueryEnvironment", "Failed to connect to server" );
    }

    _streams.push_back( stream );
    indri::net::NetworkMessageStream* messageStream = new indri::net::NetworkMessageStream( stream );
    indri::server::NetworkServerProxy* proxy = new indri::server::NetworkServerProxy( messageStream );

    _messageStreams.push_back( messageStream );
    _servers.push_back( proxy );
    _serverNameMap[hostname] = std::make_pair(proxy, stream);
  }
}

//
// removeServer
//

void indri::api::QueryEnvironment::removeServer( const std::string& hostname ) {
  // close, delete, and remove opened NetworkStream from _streams
  // close, delete, and remove opened NetworkMessageStream from _messageStreams
  // close, delete and remove opened NetworkServerProxy from _servers
  // renumber map entries after removal as needed.
  std::map<std::string, std::pair<indri::server::QueryServer *, indri::net::NetworkStream *> >::iterator iter;
  iter = _serverNameMap.find(hostname);
  if (iter != _serverNameMap.end()) {
    indri::server::QueryServer * s = iter->second.first;
    indri::net::NetworkStream * n = iter->second.second;
    for (size_t i = 0; i < _servers.size(); i++) {
      if (_servers[i] == s) {
        delete(_servers[i]);
        _servers.erase(_servers.begin() + i);
        break;
      }
    }
    
    for (size_t i = 0; i < _streams.size(); i++) {
      if (_streams[i] == n) {
        delete(_streams[i]);
        _streams.erase(_streams.begin() + i);
        delete(_messageStreams[i]);
        _messageStreams.erase(_messageStreams.begin() + i);
        break;
      }
    }
    _serverNameMap.erase(iter);
  }
}

void indri::api::QueryEnvironment::close() {
  indri::utility::delete_vector_contents<indri::server::QueryServer*>( _servers );
  _servers.clear();
  indri::utility::delete_vector_contents<indri::net::NetworkMessageStream*>( _messageStreams );
  _messageStreams.clear();
  indri::utility::delete_vector_contents<indri::net::NetworkStream*>( _streams );
  _streams.clear();
  indri::utility::delete_vector_contents<indri::collection::Repository*>( _repositories );
  _repositories.clear();
  delete(reformulator);
  reformulator = NULL;
}

std::vector<std::string> indri::api::QueryEnvironment::documentMetadata( const std::vector<DOCID_T>& documentIDs, const std::string& attributeName ) {
  std::vector< std::vector<DOCID_T> > docIDLists;
  docIDLists.resize( _servers.size() );
  std::vector< std::vector<DOCID_T> > docIDPositions;
  docIDPositions.resize( _servers.size() );
  std::vector< std::string > results;
  results.resize( documentIDs.size() );

  // split document numbers into lists for each query server
  qenv_scatter_document_ids( documentIDs, docIDLists, docIDPositions, (int)_servers.size() );

  indri::utility::greedy_vector<indri::server::QueryServerMetadataResponse*> responses;

  // send out requests for execution
  for( size_t i=0; i<docIDLists.size(); i++ ) {
    indri::server::QueryServerMetadataResponse* response = 0;
    
    if( docIDLists[i].size() )
      response = _servers[i]->documentMetadata( docIDLists[i], attributeName );
    
    responses.push_back(response);
  }

  // fold the results back into one master list (this method will delete the responses)
  qenv_gather_document_results( docIDLists, docIDPositions, responses, results );

  return results;
}

std::vector<std::string> indri::api::QueryEnvironment::pathNames( const std::vector<indri::api::ScoredExtentResult>& results ) {
  std::vector<DOCID_T> documentIDs;
  documentIDs.reserve(results.size());
  std::vector<int> pathBegins;
  pathBegins.reserve(results.size());
  std::vector<int> pathEnds;
  pathEnds.reserve(results.size());

  for( size_t i=0; i<results.size(); i++ ) {
    documentIDs.push_back( results[i].document );
    pathBegins.push_back( results[i].begin );
    pathEnds.push_back( results[i].end );
  }


  std::vector< std::vector<DOCID_T> > docIDLists;
  std::vector< std::vector<int> > beginLists;
  std::vector< std::vector<int> > endLists;
  docIDLists.resize( _servers.size() );
  beginLists.resize( _servers.size() );
  endLists.resize( _servers.size() );
  std::vector< std::vector<DOCID_T> > docIDPositions;
  docIDPositions.resize( _servers.size() );
  std::vector< std::string > paths;
  paths.resize( documentIDs.size() );
  
  // split document numbers into lists for each query server
  qenv_scatter_document_ids( documentIDs, docIDLists, docIDPositions, (int)_servers.size() );

  // copy begins and ends over given the document scattering
  for( size_t i=0; i<docIDLists.size(); i++ ) {
    if( docIDLists[i].size() ) {      
      for( size_t j=0; j<docIDLists[i].size(); j++ ) {
        int resultIndex = docIDPositions[i][j];
        beginLists[i].push_back( pathBegins[ resultIndex ] );
        endLists[i].push_back( pathEnds[ resultIndex ] );
      }
    }
  }

  indri::utility::greedy_vector<indri::server::QueryServerMetadataResponse*> responses;
  // send out requests for execution
  for( size_t i=0; i<docIDLists.size(); i++ ) {
    indri::server::QueryServerMetadataResponse* response = 0;
    
    if( docIDLists[i].size() )
      response = _servers[i]->pathNames( docIDLists[i], beginLists[i], endLists[i] );
    
    responses.push_back(response);
  }

  // fold the results back into one master list (this method will delete the responses)
  qenv_gather_document_results( docIDLists, docIDPositions, responses, paths );

  return paths;
}

std::vector<std::string> indri::api::QueryEnvironment::documentMetadata( const std::vector<indri::api::ScoredExtentResult>& results, const std::string& attributeName ) {
  // copy into an int vector
  std::vector<DOCID_T> documentIDs;
  documentIDs.reserve(results.size());

  for( size_t i=0; i<results.size(); i++ ) {
    documentIDs.push_back( results[i].document );
  }

  return documentMetadata( documentIDs, attributeName );
}

std::vector<indri::api::ParsedDocument*> indri::api::QueryEnvironment::documents( const std::vector<DOCID_T>& documentIDs ) {
  std::vector< std::vector<DOCID_T> > docIDLists;
  std::vector< std::vector<DOCID_T> > docIDPositions;
  std::vector< indri::api::ParsedDocument* > results;
  results.resize( documentIDs.size() );

  // split document numbers into lists for each query server
  qenv_scatter_document_ids( documentIDs, docIDLists, docIDPositions, (int)_servers.size() );

  indri::utility::greedy_vector<indri::server::QueryServerDocumentsResponse*> responses;

  // send out requests for processing
  for( size_t i=0; i<docIDLists.size(); i++ ) {
    indri::server::QueryServerDocumentsResponse* response = 0;

    if( docIDLists[i].size() ) {
      response = _servers[i]->documents( docIDLists[i] );
    } 

    responses.push_back(response);
  }

  // fold the results back into one master list (this method will delete the responses)
  qenv_gather_document_results( docIDLists, docIDPositions, responses, results );

  return results;
}

// fetch the document names for a list of document IDs
std::vector<indri::api::ParsedDocument*> indri::api::QueryEnvironment::documents( const std::vector<indri::api::ScoredExtentResult>& results ) {
  // copy into an int vector
  std::vector<DOCID_T> documentIDs;
  documentIDs.reserve(results.size());

  for( size_t i=0; i<results.size(); i++ ) {
    documentIDs.push_back( results[i].document );
  }

  return documents( documentIDs );
}

//
// documentsFromMetadata 
//

std::vector<indri::api::ParsedDocument*> indri::api::QueryEnvironment::documentsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues ) {
  indri::server::QueryServerDocumentsResponse* response = 0;

  // we have to ask the same query to all nodes, because we don't really know which nodes will have answers.
  std::vector<indri::server::QueryServerDocumentsResponse*> responses;

  // send out requests for processing
  for( size_t i=0; i<_servers.size(); i++ ) {
    response = _servers[i]->documentsFromMetadata( attributeName, attributeValues );
    responses.push_back(response);
  }

  std::vector<indri::api::ParsedDocument*> results;

  // gather results
  for( size_t i=0; i<responses.size(); i++ ) {
    std::vector<indri::api::ParsedDocument*>& responseResults = responses[i]->getResults();
    
    std::copy( responseResults.begin(),
               responseResults.end(),
               std::back_inserter( results ) );
  }
  indri::utility::delete_vector_contents<indri::server::QueryServerDocumentsResponse*>( responses );
  return results;
}

//
// documentIDsFromMetadata
//

std::vector<DOCID_T> indri::api::QueryEnvironment::documentIDsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues ) {
  indri::server::QueryServerDocumentIDsResponse* response = 0;

  // we have to ask the same query to all nodes, because we don't really know which nodes will have answers.
  std::vector<indri::server::QueryServerDocumentIDsResponse*> responses;

  // send out requests for processing
  for( size_t i=0; i<_servers.size(); i++ ) {
    response = _servers[i]->documentIDsFromMetadata( attributeName, attributeValues );
    responses.push_back(response);
  }

  std::vector<DOCID_T> results;

  // gather results
  for( size_t i=0; i<responses.size(); i++ ) {
    std::vector<DOCID_T>& responseResults = responses[i]->getResults();

    for( size_t j=0; j<responseResults.size(); j++ ) {
      DOCID_T converted = (DOCID_T)((responseResults[j] * _servers.size()) + i);
      results.push_back( converted );
    }
  }
  indri::utility::delete_vector_contents<indri::server::QueryServerDocumentIDsResponse*>( responses );
  return results;
}

//
// _scoredQuery
//

void indri::api::QueryEnvironment::_scoredQuery( indri::infnet::InferenceNetwork::MAllResults& results, indri::lang::Node* queryRoot, std::string& accumulatorName, int resultsRequested, const std::vector<DOCID_T>* documentSet ) {
  // add a FilterNode, unique to each server
  // send off each query for evaluation
  std::vector< std::vector<DOCID_T> > docIDLists;
  std::vector< std::vector<DOCID_T> > docIDPositions;

  // scatter the document IDs out to the servers
  if( documentSet )
    qenv_scatter_document_ids( *documentSet, docIDLists, docIDPositions, (int)_servers.size() );

  accumulatorName = "";

  // For each server, make a FilterNode and an AccumulatorNode, then run the query.
  // The filter node makes sure that we only score the documents that are interesting
  // so we don't waste too much time here.  
  std::vector<indri::lang::Node*> nodes;
  indri::utility::VectorDeleter<indri::lang::Node*> nd(nodes);
  std::string filterName;

  std::vector<indri::server::QueryServerResponse*> queryResponses;
  indri::utility::VectorDeleter<indri::server::QueryServerResponse*> qd(queryResponses);

  for( size_t i=0; i<_servers.size(); i++ ) {
    indri::lang::ScoredExtentNode* scoredRoot = dynamic_cast<indri::lang::ScoredExtentNode*>(queryRoot);
    indri::lang::FilterNode* filterNode = 0;
    indri::lang::ScoreAccumulatorNode* accumulatorNode = 0;
    
    if( documentSet ) {
      filterNode = new indri::lang::FilterNode( scoredRoot, docIDLists[i] );
      accumulatorNode = new indri::lang::ScoreAccumulatorNode( filterNode );
      nodes.push_back( filterNode );
      nodes.push_back( accumulatorNode );
    } else {
      accumulatorNode = new indri::lang::ScoreAccumulatorNode( scoredRoot );
      nodes.push_back( accumulatorNode );
    }

    // name all the nodes the same thing
    if( filterNode ) {
      if( filterName.length() > 0 )
        filterNode->setNodeName( filterName );
      else
        filterName = filterNode->nodeName();
    }

    if( accumulatorNode ) {
      if( accumulatorName.length() > 0 ) {
        accumulatorNode->setNodeName( accumulatorName );
      } else {
        accumulatorName = accumulatorNode->nodeName();
      }
    }

    std::vector<indri::lang::Node*> root;
    root.push_back( accumulatorNode );

    // don't optimize these queries, otherwise we won't be able to distinguish some annotations from others
    indri::server::QueryServerResponse* response = _servers[i]->runQuery( root, resultsRequested, true );
    queryResponses.push_back(response);
  }

  // now, gather up all the responses, merge them into some kind of output structure, and return them
  _mergeQueryResults( results, queryResponses );
}

void indri::api::QueryEnvironment::_annotateQuery( indri::infnet::InferenceNetwork::MAllResults& results,
                                                   const std::vector<DOCID_T>& documentSet,
                                                   std::string& annotatorName,
                                                   indri::lang::Node* queryRoot ) {
  // add a FilterNode, unique to each server
  // send off each query for evaluation
  std::vector< std::vector<DOCID_T> > docIDLists;
  std::vector< std::vector<DOCID_T> > docIDPositions;

  // scatter the document IDs out to the servers
  qenv_scatter_document_ids( documentSet, docIDLists, docIDPositions, (int)_servers.size() );

  // For each server, make a FilterNode and an AnnotatorNode, then run the query.
  // The filter node makes sure that we only annotate the documents that are interesting
  // so we don't waste too much time here.  The AnnotatorNode collects the annotations.
  std::vector<indri::lang::FilterNode*> filterNodes;
  std::vector<indri::lang::AnnotatorNode*> annotatorNodes;
  std::vector<indri::server::QueryServerResponse*> queryResponses;
  indri::utility::VectorDeleter<indri::lang::FilterNode*> fd(filterNodes);
  indri::utility::VectorDeleter<indri::lang::AnnotatorNode*> ad(annotatorNodes);
  indri::utility::VectorDeleter<indri::server::QueryServerResponse*> qd(queryResponses);

  for( size_t i=0; i<_servers.size(); i++ ) {
    indri::lang::ScoredExtentNode* scoredRoot = dynamic_cast<indri::lang::ScoredExtentNode*>(queryRoot);
    indri::lang::FilterNode* filterNode = new indri::lang::FilterNode( scoredRoot, docIDLists[i] );
    indri::lang::AnnotatorNode* annotatorNode = new indri::lang::AnnotatorNode( filterNode );

    // name all the nodes the same thing
    if( i != 0 ) {
      filterNode->setNodeName( filterNodes[0]->nodeName() );
      annotatorNode->setNodeName( annotatorNodes[0]->nodeName() );
    }

    filterNodes.push_back( filterNode );
    annotatorNodes.push_back( annotatorNode );

    std::vector<indri::lang::Node*> root;
    root.push_back( annotatorNode );

    // don't optimize these queries, otherwise we won't be able to distinguish some annotations from others
    indri::server::QueryServerResponse* response = _servers[i]->runQuery( root, (int)docIDLists[i].size(), false );
    queryResponses.push_back(response);
  }

  // now, gather up all the responses, merge them into some kind of output structure, and return them
  _mergeQueryResults( results, queryResponses );
  
  if( annotatorNodes.size() )
    annotatorName = annotatorNodes[0]->nodeName();
}

//
// expressionList
//

std::vector<indri::api::ScoredExtentResult> indri::api::QueryEnvironment::expressionList( const std::string& expression, const std::string& queryType ) {
  QueryParserWrapper* parser = QueryParserFactory::get(expression, queryType);
  indri::lang::ScoredExtentNode* rootNode;

  try {
    rootNode = parser->query();
  } catch( antlr::ANTLRException e ) {
    LEMUR_THROW( LEMUR_PARSE_ERROR, "Couldn't understand this query: " + e.getMessage() );
  }

  indri::lang::RawScorerNode* rootScorer = dynamic_cast<indri::lang::RawScorerNode*>(rootNode);
  
  if( rootScorer == 0 ) {
    LEMUR_THROW( LEMUR_PARSE_ERROR, "This query does not appear to be a proximity expression" );
  }

  // replace the raw scorer node with a listAccumulator
  indri::lang::ListAccumulator* listAccumulator = new indri::lang::ListAccumulator( rootScorer->getRawExtent() );
  listAccumulator->setNodeName( rootScorer->nodeName() );
  
  std::vector<indri::lang::Node*> roots;
  roots.push_back( listAccumulator );

  indri::infnet::InferenceNetwork::MAllResults statisticsResults;
  _sumServerQuery( statisticsResults, roots, MAX_INT32 );
  
  std::vector<ScoredExtentResult>& occurrencesList = statisticsResults[ listAccumulator->nodeName() ][ "occurrences" ];
  delete parser;
  delete listAccumulator;
  return occurrencesList;
}

//
// _expressionCount
//

double indri::api::QueryEnvironment::_expCount( const std::string& expression, 
                                                const std::string& whichOccurrences,
                                                const std::string& queryType ) {
  QueryParserWrapper* parser = QueryParserFactory::get(expression, queryType);
  indri::lang::ScoredExtentNode* rootNode;

  try {
    rootNode = parser->query();
  } catch( antlr::ANTLRException e ) {
    LEMUR_THROW( LEMUR_PARSE_ERROR, "Couldn't understand this query: " + e.getMessage() );
  }

  indri::lang::RawScorerNode* rootScorer = dynamic_cast<indri::lang::RawScorerNode*>(rootNode);
  
  if( rootScorer == 0 ) {
    LEMUR_THROW( LEMUR_PARSE_ERROR, "This query does not appear to be a proximity expression" );
  }

  // replace the raw scorer node with a context counter node
  indri::lang::ContextCounterNode* contextCounter = new indri::lang::ContextCounterNode( rootScorer->getRawExtent(),
                                                                                         rootScorer->getContext() );
  contextCounter->setNodeName( rootScorer->nodeName() );

  std::vector<indri::lang::Node*> roots;
  roots.push_back( contextCounter );

  indri::infnet::InferenceNetwork::MAllResults statisticsResults;
  _sumServerQuery( statisticsResults, roots, MAX_INT32 ); // 1000
  
  std::vector<ScoredExtentResult>& occurrencesList = statisticsResults[ contextCounter->nodeName() ][ whichOccurrences ];
  delete parser;
  delete contextCounter;
  return occurrencesList[0].score;
}

double indri::api::QueryEnvironment::expressionCount( const std::string& expression, const std::string& queryType ) {
  return _expCount(expression, "occurrences", queryType);
}
double indri::api::QueryEnvironment::documentExpressionCount( const std::string& expression, const std::string& queryType ) {
  return _expCount(expression, "documentOccurrences", queryType);
}

// run a query (Indri query language)
std::vector<indri::api::ScoredExtentResult> indri::api::QueryEnvironment::_runQuery( indri::infnet::InferenceNetwork::MAllResults& results,
                                                                                     const std::string& q,
                                                                                     int resultsRequested,
                                                                                     const std::vector<DOCID_T>* documentSet,
                                                                                     indri::api::QueryAnnotation** annotation,
                                                                                     const std::string &queryType) {
  INIT_TIMER
    QueryParserWrapper *parser = QueryParserFactory::get(q, queryType);

  PRINT_TIMER( "Initialization complete" );

  indri::lang::ScoredExtentNode* rootNode;

  std::vector<indri::lang::Node*> nodes;
  indri::utility::VectorDeleter<indri::lang::Node*> nd(nodes);

  try {
    rootNode = parser->query();
  } catch( antlr::ANTLRException e ) {
    LEMUR_THROW( LEMUR_PARSE_ERROR, "Couldn't understand this query: " + e.getMessage() );
  }

  PRINT_TIMER( "Parsing complete" );

  if (_baseline) {    
    // Replace with a PlusNode
    indri::lang::UnweightedCombinationNode* rootScorer = dynamic_cast<indri::lang::UnweightedCombinationNode*>(rootNode);
    indri::lang::RawScorerNode* rawScorer = dynamic_cast<indri::lang::RawScorerNode*>(rootNode);
    indri::lang::WeightedCombinationNode* weightScorer = dynamic_cast<indri::lang::WeightedCombinationNode*>(rootNode);
    if (rootScorer == NULL && rawScorer == NULL) 
      {
        if (weightScorer == NULL) 
          {
            LEMUR_THROW( LEMUR_PARSE_ERROR, "Can't run baseline on this query: " + q + "\nindri query language operators are not allowed." );
          } else {
          indri::lang::WPlusNode * plusNode = new indri::lang::WPlusNode();
          plusNode->setNodeName(weightScorer->nodeName());
          const std::vector< std::pair<double, indri::lang::ScoredExtentNode*> >& children = weightScorer->getChildren();
          for (int i = 0; i < children.size(); i++) {
            plusNode->addChild(children[i].first, children[i].second);
          }
          rootNode = plusNode;
          nodes.push_back(plusNode);
          }
      }
    
    if (rootScorer) {
      // make sure it doesn't have any field restrictions
      if ( q.find(".") != std::string::npos) {
        LEMUR_THROW( LEMUR_PARSE_ERROR, "Can't run baseline on this query: " + q + "\nindri query language field restrictions are not allowed." );
      }
      indri::lang::PlusNode * plusNode = new indri::lang::PlusNode();
      plusNode->setNodeName(rootScorer->nodeName());
      const std::vector<indri::lang::ScoredExtentNode *> & children = rootScorer->getChildren();
      for (int i = 0; i < children.size(); i++) {
        plusNode->addChild(children[i]);
      }
      rootNode = plusNode;
      nodes.push_back(plusNode);
    }  else if (rawScorer) {        
      // if a RawScorerNode, just leave it
      // make sure it doesn't have any field restrictions
      if ( q.find(".") != std::string::npos) {
        LEMUR_THROW( LEMUR_PARSE_ERROR, "Can't run baseline on this query: " + q + "\nindri query language field restrictions are not allowed." );
      }
    }
  }
  
  // push down language models from ExtentRestriction nodes
  indri::lang::ExtentRestrictionModelAnnotatorCopier restrictionCopier;
  rootNode = dynamic_cast<indri::lang::ScoredExtentNode*>(rootNode->copy(restrictionCopier));

  // extract the raw scorer nodes from the query tree
  indri::lang::RawScorerNodeExtractor extractor;
  rootNode->walk(extractor);

  // copy out a new graph that has context counters in it -- this will be evaluated
  // so that we can get counts for everything in the query.  We need those counts
  // so that we can score the query terms correctly.
  std::vector<indri::lang::RawScorerNode*>& scorerNodes = extractor.getScorerNodes();

  bool noContext = false;
  if (_parameters.exists("singleBackgroundModel")) {
    noContext = (bool) _parameters.get("singleBackgroundModel");
  }

  indri::infnet::InferenceNetwork::MAllResults statisticsResults;
            
  if ( noContext ) {
    indri::lang::ApplyCopiers<indri::lang::NoContextCountGraphCopier, indri::lang::RawScorerNode> graph( scorerNodes );
    _sumServerQuery( statisticsResults, graph.roots(), resultsRequested ); //1000
  } else {
    indri::lang::ApplyCopiers<indri::lang::ContextCountGraphCopier, indri::lang::RawScorerNode> graph( scorerNodes );
    _sumServerQuery( statisticsResults, graph.roots(), resultsRequested ); //1000
  }

  PRINT_TIMER( "Statistics complete" );

  // feed the statistics we found back into the query network
  _copyStatistics( scorerNodes, statisticsResults );

  // annotate the graph with smoothing parameters
  indri::lang::SmoothingAnnotatorWalker smoother( _parameters );
  rootNode->walk(smoother);

  if (_baseline) {
    // update the qtf in the smoothing rules
    indri::lang::QueryTFWalker qtfWalker(_servers);
    rootNode->walk(qtfWalker);
  }
  
  // run a scored query (possibly including a document set)
  std::string accumulatorName;
  _scoredQuery( results, rootNode, accumulatorName, resultsRequested, documentSet );
  std::vector<indri::api::ScoredExtentResult> queryResults = results[accumulatorName]["scores"];
  std::stable_sort( queryResults.begin(), queryResults.end(), indri::api::ScoredExtentResult::score_greater() );
  if( (int)queryResults.size() > resultsRequested )
    queryResults.resize( resultsRequested );

  PRINT_TIMER( "Query complete" );

  if( annotation ) {
    std::string annotatorName;
    std::vector<DOCID_T> documentSet;

    for( size_t i=0; i<queryResults.size(); i++ ) {
      documentSet.push_back( queryResults[i].document );
    }

    _annotateQuery( results, documentSet, annotatorName, rootNode );
    *annotation = new indri::api::QueryAnnotation( rootNode, results[annotatorName], queryResults );
  }

  PRINT_TIMER( "Annotation complete" );
  delete(parser);
  return queryResults;
}

// put in api?
static void _getRawNodes( std::set<std::string>& nodeTerms, 
                          const indri::api::QueryAnnotationNode* node ) {
  if( node->type == "IndexTerm" ) {
    nodeTerms.insert( node->queryText );
  } else {
    for( size_t i=0; i<node->children.size(); i++ ) {
      _getRawNodes( nodeTerms, node->children[i] );
    }
  }
}

indri::api::QueryResults indri::api::QueryEnvironment::runQuery( indri::api::QueryRequest &request ) {
  // clone _runQuery for timers.
  // alternatively, skip parse time and use _runQuery directly.
  indri::api::QueryResults queryResult;  
  indri::infnet::InferenceNetwork::MAllResults results;
  indri::api::QueryAnnotation* annotation;
  std::string queryType = "indri";
  const float million = 1000000.0;
  indri::utility::IndriTimer timer; 
  timer.start();

  // need the other options, formulators in here
  QueryParserWrapper *parser = QueryParserFactory::get(request.query, queryType);

  indri::lang::ScoredExtentNode* rootNode;

  try {
    rootNode = parser->query();
  } catch( antlr::ANTLRException e ) {
    LEMUR_THROW( LEMUR_PARSE_ERROR, "Couldn't understand this query: " + e.getMessage() );
  }

  
  timer.stop(); 
  queryResult.parseTime = timer.elapsedTime()/million; 
  timer.start();

  // push down language models from ExtentRestriction nodes
  indri::lang::ExtentRestrictionModelAnnotatorCopier restrictionCopier;
  rootNode = dynamic_cast<indri::lang::ScoredExtentNode*>(rootNode->copy(restrictionCopier));

  // extract the raw scorer nodes from the query tree
  indri::lang::RawScorerNodeExtractor extractor;
  rootNode->walk(extractor);

  // copy out a new graph that has context counters in it -- this will be evaluated
  // so that we can get counts for everything in the query.  We need those counts
  // so that we can score the query terms correctly.
  std::vector<indri::lang::RawScorerNode*>& scorerNodes = extractor.getScorerNodes();

  bool noContext = false;
  if (_parameters.exists("singleBackgroundModel")) {
    noContext = (bool) _parameters.get("singleBackgroundModel");
  }

  indri::infnet::InferenceNetwork::MAllResults statisticsResults;

  // 1000 should be resultsRequested?
  if ( noContext ) {
    indri::lang::ApplyCopiers<indri::lang::NoContextCountGraphCopier, indri::lang::RawScorerNode> graph( scorerNodes );
    _sumServerQuery( statisticsResults, graph.roots(), request.resultsRequested + request.startNum );
  } else {
    indri::lang::ApplyCopiers<indri::lang::ContextCountGraphCopier, indri::lang::RawScorerNode> graph( scorerNodes );
    _sumServerQuery( statisticsResults, graph.roots(), request.resultsRequested + request.startNum );
  }

  // feed the statistics we found back into the query network
  _copyStatistics( scorerNodes, statisticsResults );

  // annotate the graph with smoothing parameters
  indri::lang::SmoothingAnnotatorWalker smoother( _parameters );
  rootNode->walk(smoother);

  // run a scored query (possibly including a document set)
  std::vector<lemur::api::DOCID_T>* documentSet = NULL;
  if (request.docSet.size() > 0) documentSet = &request.docSet;
  
  std::string accumulatorName;
  _scoredQuery( results, rootNode, accumulatorName, request.resultsRequested + request.startNum, documentSet );
  std::vector<indri::api::ScoredExtentResult> queryResults = results[accumulatorName]["scores"];
  std::stable_sort( queryResults.begin(), queryResults.end(), indri::api::ScoredExtentResult::score_greater() );
  // prune the list
  if (request.startNum > 0) {
    queryResults.erase(queryResults.begin(), queryResults.begin() + request.startNum);
  }

  if( (int)queryResults.size() > request.resultsRequested )
    queryResults.resize( request.resultsRequested );

  std::string annotatorName;
  std::vector<DOCID_T> docSet;

  for( size_t i=0; i<queryResults.size(); i++ ) {
    docSet.push_back( queryResults[i].document );
  }

  _annotateQuery( results, docSet, annotatorName, rootNode );
  annotation = new indri::api::QueryAnnotation( rootNode, results[annotatorName], queryResults );
  
  delete(parser);

  timer.stop(); 
  queryResult.executeTime = timer.elapsedTime()/million; 
  timer.start();

  // fill in the results bits

  std::vector<indri::api::ScoredExtentResult> _results = annotation->getResults();

  bool html = request.options == QueryRequest::HTMLSnippet;
  indri::api::SnippetBuilder builder(html);
  std::vector<indri::api::ParsedDocument*> docs;
  std::vector<indri::api::ScoredExtentResult> resultSubset;

  // slice these into blocks of 50/100/500?
  // 1000 is 29M on AP89.
  for( size_t start = 0; start < _results.size(); start += 100 ) {
    size_t end = lemur_compat::min<size_t>( start + 100, _results.size() );
    resultSubset.assign( _results.begin() + start, _results.begin() + end );
    docs = documents( resultSubset );
    indri::utility::greedy_vector<indri::parse::MetadataPair>::iterator iter;
  
    for( size_t i = 0; i < resultSubset.size(); i++ ) {
      indri::api::QueryResult res;

      iter = std::find_if( docs[i]->metadata.begin(),
                           docs[i]->metadata.end(),
                           indri::parse::MetadataPair::key_equal( "docno" ) );

      if( iter != docs[i]->metadata.end() )
        res.documentName = (char*) iter->value;
      else
        res.documentName = "No docno value";

      res.score = resultSubset[i].score;
      res.docid = resultSubset[i].document;
      res.begin = resultSubset[i].begin;
      res.end = resultSubset[i].end;
      res.snippet = builder.build( resultSubset[i].document, docs[i], annotation );

      for (size_t j = 0; j < request.metadata.size(); j++ ) {
        std::string &key = request.metadata[j];
        iter = std::find_if( docs[i]->metadata.begin(),
                             docs[i]->metadata.end(),
                             indri::parse::MetadataPair::key_equal( key.c_str() ) );

        if( iter != docs[i]->metadata.end() ) {
          // values have to be copied so they don't go out of scope.
          indri::api::MetadataPair meta;
          meta.key = (*iter).key;
          meta.value = (char *)(*iter).value;
          res.metadata.push_back(meta);
        }
      }
      queryResult.results.push_back( res );
      delete docs[i];
    }
  }
  
  // estimate the matches.
  std::set<std::string> queryTerms;
  _getRawNodes(queryTerms, annotation->getQueryTree());
  std::set<std::string>::iterator iter;
  int estCount = 0;
  for (iter = queryTerms.begin(); iter != queryTerms.end(); iter++) {
    int count = (int)documentCount(*iter);
    // argMax over df(t)
    if (count > estCount) estCount = count;
  }
  queryResult.estimatedMatches = estCount;

  timer.stop(); 
  queryResult.documentsTime = timer.elapsedTime()/million; 
  timer.start();

  return queryResult;
}

std::vector<indri::api::ScoredExtentResult> indri::api::QueryEnvironment::runQuery( const std::string& query, int resultsRequested, const std::string &queryType ) {
  indri::infnet::InferenceNetwork::MAllResults results;
  std::vector<indri::api::ScoredExtentResult> queryResult = _runQuery( results, query, resultsRequested, 0, 0, queryType );
  return queryResult;
}

std::vector<indri::api::ScoredExtentResult> indri::api::QueryEnvironment::runQuery( const std::string& query, const std::vector<DOCID_T>& documentSet, int resultsRequested, const std::string &queryType) {
  indri::infnet::InferenceNetwork::MAllResults results;
  std::vector<indri::api::ScoredExtentResult> queryResult = _runQuery( results, query, resultsRequested, &documentSet, 0, queryType );
  return queryResult;
}

indri::api::QueryAnnotation* indri::api::QueryEnvironment::runAnnotatedQuery( const std::string& query, int resultsRequested, const std::string &queryType ) {
  indri::infnet::InferenceNetwork::MAllResults results;
  indri::api::QueryAnnotation* annotation = 0;
  
  _runQuery( results, query, resultsRequested, 0, &annotation, queryType );
  return annotation;
}

indri::api::QueryAnnotation* indri::api::QueryEnvironment::runAnnotatedQuery( const std::string& query, const std::vector<DOCID_T>& documentSet, int resultsRequested, const std::string &queryType ) {
  indri::infnet::InferenceNetwork::MAllResults results;
  indri::api::QueryAnnotation* annotation = 0;
  
  _runQuery( results, query, resultsRequested, &documentSet, &annotation, queryType );
  return annotation;
}

std::string indri::api::QueryEnvironment::stemTerm(const std::string &term) {
  std::string stem;
  // return the first stem that differs from the input term
  // as servers may have different stemmers.
  for( size_t i=0; i<_servers.size(); i++ ) {
    stem = _servers[i]->stemTerm(term);
    if (stem != term) return stem;
  }
  return term;
}

//
// Term counts
//

INT64 indri::api::QueryEnvironment::termCountUnique() {
  // note that this only provides a lower bound estimate
  INT64 totalTermCount = 0;

  for( size_t i=0; i<_servers.size(); i++ ) {
    totalTermCount = lemur_compat::max(_servers[i]->termCountUnique(), 
                                        totalTermCount);
  }

  return totalTermCount;
}

INT64 indri::api::QueryEnvironment::termCount() {
  // note that we should probably send these requests asynchronously
  INT64 totalTermCount = 0;

  for( size_t i=0; i<_servers.size(); i++ ) {
    totalTermCount += _servers[i]->termCount();
  }

  return totalTermCount;
}

INT64 indri::api::QueryEnvironment::termCount( const std::string& term ) {
  // note that we should probably send these requests asynchronously
  INT64 totalTermCount = 0;

  for( size_t i=0; i<_servers.size(); i++ ) {
    totalTermCount += _servers[i]->termCount( term );
  }

  return totalTermCount;
}

INT64 indri::api::QueryEnvironment::stemCount( const std::string& stem ) {
  // note that we should probably send these requests asynchronously
  INT64 totalTermCount = 0;

  for( size_t i=0; i<_servers.size(); i++ ) {
    totalTermCount += _servers[i]->stemCount( stem );
  }

  return totalTermCount;
}

// 
// Field counts
//

INT64 indri::api::QueryEnvironment::termFieldCount( const std::string& term, const std::string& field ) {
  INT64 totalTermCount = 0;

  for( size_t i=0; i<_servers.size(); i++ ) {
    totalTermCount += _servers[i]->termFieldCount( term, field );
  }

  return totalTermCount;
}

INT64 indri::api::QueryEnvironment::stemFieldCount( const std::string& stem, const std::string& field ) {
  INT64 totalTermCount = 0;

  for( size_t i=0; i<_servers.size(); i++ ) {
    totalTermCount += _servers[i]->stemFieldCount( stem, field );
  }

  return totalTermCount;
}

//
// Field list
//

std::vector<std::string> indri::api::QueryEnvironment::fieldList() {
  std::vector<std::string> result;
  std::vector<std::string> machineResult;
  std::set<std::string> fieldSet;
  
  for( size_t i=0; i<_servers.size(); i++ ) {
    machineResult = _servers[i]->fieldList();

    for( size_t j=0; j<machineResult.size(); j++ ) {
      fieldSet.insert( machineResult[j] );
    }
  }

  std::set<std::string>::iterator iter;

  for( iter = fieldSet.begin(); iter != fieldSet.end(); iter++ ) {
    result.push_back( *iter );
  }
  
  return result;
}

//
// Documents
//

INT64 indri::api::QueryEnvironment::documentCount() {
  INT64 totalDocumentCount = 0;

  for( size_t i=0; i<_servers.size(); i++ ) {
    totalDocumentCount += _servers[i]->documentCount();
  }

  return totalDocumentCount;
}

INT64 indri::api::QueryEnvironment::documentCount( const std::string& term ) {
  INT64 totalDocumentCount = 0;

  for( size_t i=0; i<_servers.size(); i++ ) {
    totalDocumentCount += _servers[i]->documentCount( term );
  }

  return totalDocumentCount;
}

INT64 indri::api::QueryEnvironment::documentStemCount( const std::string& term ) {
  INT64 totalDocumentCount = 0;

  for( size_t i=0; i<_servers.size(); i++ ) {
    totalDocumentCount += _servers[i]->documentStemCount( term );
  }

  return totalDocumentCount;
}

std::vector<indri::api::DocumentVector*> indri::api::QueryEnvironment::documentVectors( const std::vector<DOCID_T>& documentIDs ) {
  std::vector< std::vector<DOCID_T> > docIDLists;
  docIDLists.resize( _servers.size() );
  std::vector< std::vector<DOCID_T> > docIDPositions;
  docIDPositions.resize( _servers.size() );
  std::vector< indri::api::DocumentVector* > results;
  results.resize( documentIDs.size() );

  // split document numbers into lists for each query server
  qenv_scatter_document_ids( documentIDs, docIDLists, docIDPositions, (int)_servers.size() );

  indri::utility::greedy_vector<indri::server::QueryServerVectorsResponse*> responses;

  // send out requests for processing
  for( size_t i=0; i<docIDLists.size(); i++ ) {
    indri::server::QueryServerVectorsResponse* response = 0;

    if( docIDLists[i].size() )
      response = _servers[i]->documentVectors( docIDLists[i] );
    
    responses.push_back(response);
  }

  // fold the results back into one master list (this method will delete the responses)
  qenv_gather_document_results( docIDLists, docIDPositions, responses, results );

  return results;
}

int indri::api::QueryEnvironment::documentLength(lemur::api::DOCID_T documentID) {
  int length = 0;
  int serverCount = (int)_servers.size();
  DOCID_T id = documentID/serverCount;
  int serverID = documentID % serverCount;
  length = _servers[serverID]->documentLength( id );
  return length;
}

//
// setMaxWildcardTerms
//
void indri::api::QueryEnvironment::setMaxWildcardTerms(int maxTerms) {
  // for each server - let the server know the max.
  for( size_t i=0; i<_servers.size(); i++ ) {
    _servers[i]->setMaxWildcardTerms(maxTerms);
  }

}

void indri::api::QueryEnvironment::setFormulationParameters(Parameters &p) {
  reformulatorParams = p;
  reformulator->setParameters(p);
}

std::string indri::api::QueryEnvironment::reformulateQuery(const std::string &query) {
  std::string reform = reformulator->transform(query);
  return reform;
}
