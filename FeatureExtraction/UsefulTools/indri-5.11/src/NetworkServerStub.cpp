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
// 21 May 2004 -- tds
//

#include "indri/NetworkServerStub.hpp"
#include "indri/XMLNode.hpp"
#include "indri/NetworkMessageStream.hpp"
#include "indri/QueryServer.hpp"
#include "indri/QueryResponsePacker.hpp"
#include "indri/ParsedDocument.hpp"
#include "lemur/Exception.hpp"

indri::net::NetworkServerStub::NetworkServerStub( indri::server::QueryServer* server, indri::net::NetworkMessageStream* stream ) :
  _server(server),
  _stream(stream)
{
}

void indri::net::NetworkServerStub::_decodeMetadataRequest( const class indri::xml::XMLNode* request,
                                                            std::string& attributeName,
                                                            std::vector<std::string>& attributeValues )
{
  attributeName = request->getChildValue( "attributeName" );
  const std::vector<indri::xml::XMLNode*>& attributeValueNodes = request->getChild( "attributeValues" )->getChildren();

  for( size_t j=0; j<attributeValueNodes.size(); j++ ) {
    attributeValues.push_back( attributeValueNodes[j]->getValue() );
  }
}

void indri::net::NetworkServerStub::_sendDocumentsResponse( indri::server::QueryServerDocumentsResponse* docResponse ) {
  std::vector<indri::api::ParsedDocument*> documents = docResponse->getResults();

  // send them back
 std::auto_ptr<indri::xml::XMLNode> response( new indri::xml::XMLNode( "documents" ) );
  for( size_t i=0; i<documents.size(); i++ ) {
    indri::xml::XMLNode* docNode = _encodeDocument( documents[i] );
    response->addChild( docNode );
    delete documents[i];
  }

  _stream->reply( response.get() );
  _stream->replyDone();

  delete docResponse;
  //  delete response;
}

void indri::net::NetworkServerStub::_sendNumericResponse( const char* responseName, UINT64 number ) {
  indri::xml::XMLNode* response = new indri::xml::XMLNode( responseName, i64_to_string(number) );
  _stream->reply( response );
  _stream->replyDone();
  delete response;
}

indri::xml::XMLNode* indri::net::NetworkServerStub::_encodeDocument( const struct indri::api::ParsedDocument* document ) {
  indri::xml::XMLNode* docNode = new indri::xml::XMLNode( "document" );

  indri::xml::XMLNode* metadata = 0; 
  indri::xml::XMLNode* textNode = 0;
  indri::xml::XMLNode* contentNode = 0;
  indri::xml::XMLNode* contentLengthNode = 0;
  indri::xml::XMLNode* positions = 0;

  if( document->metadata.size() ) {
    metadata = new indri::xml::XMLNode( "metadata" );

    for( size_t j=0; j<document->metadata.size(); j++ ) {
      indri::xml::XMLNode* keyNode = new indri::xml::XMLNode( "key", document->metadata[j].key );
      std::string value = base64_encode( document->metadata[j].value, document->metadata[j].valueLength );
      indri::xml::XMLNode* valNode = new indri::xml::XMLNode( "value", value );

      indri::xml::XMLNode* datum = new indri::xml::XMLNode( "datum" );
      datum->addChild( keyNode );
      datum->addChild( valNode );

      metadata->addChild( datum );
    }
  }

  if( document->text ) {
    std::string text = base64_encode( document->text, (int)document->textLength );
    textNode = new indri::xml::XMLNode( "text", text );
  }

  if( document->content && document->text ) {
    INT64 contentOffset = document->content - document->text;
    contentNode = new indri::xml::XMLNode( "content", i64_to_string(contentOffset) );
    contentLengthNode = new indri::xml::XMLNode( "contentLength", i64_to_string(document->contentLength) );
  }

  if( document->positions.size() ) {
    positions = new indri::xml::XMLNode( "positions" );

    for( size_t j=0; j<document->positions.size(); j++ ) {
      indri::xml::XMLNode* position = new indri::xml::XMLNode( "position" );
      indri::xml::XMLNode* begin = new indri::xml::XMLNode( "begin", i64_to_string( document->positions[j].begin ) );
      indri::xml::XMLNode* end = new indri::xml::XMLNode( "end", i64_to_string( document->positions[j].end ) );
      
      position->addChild( begin );
      position->addChild( end );

      positions->addChild( position );
    }
  }

  if( metadata )
    docNode->addChild( metadata );

  if( textNode )
    docNode->addChild( textNode );

  if( contentNode ) {
    docNode->addChild( contentNode );
    docNode->addChild( contentLengthNode );
  }

  if( positions )
    docNode->addChild( positions );

  return docNode;
}

void indri::net::NetworkServerStub::_handleQuery( indri::xml::XMLNode* request ) {
  indri::lang::Unpacker unpacker(request);
  std::vector<indri::lang::Node*> nodes = unpacker.unpack();
  int resultsRequested = (int) string_to_i64( request->getAttribute( "resultsRequested" ) );
  bool optimize = request->getAttribute("optimize") == "1";

  indri::server::QueryServerResponse* response = _server->runQuery( nodes, resultsRequested, optimize );
  indri::infnet::InferenceNetwork::MAllResults results = response->getResults();

  QueryResponsePacker packer( results );
  packer.write( _stream );
  delete response;
}

void indri::net::NetworkServerStub::_handleDocuments( indri::xml::XMLNode* request ) {
  // rip out all the docIDs:
  std::vector<lemur::api::DOCID_T> documentIDs;
  for( size_t i=0; i<request->getChildren().size(); i++ ) {
    documentIDs.push_back( (lemur::api::DOCID_T) string_to_i64( request->getChildren()[i]->getValue() ) );
  }

  // get the documents
  indri::server::QueryServerDocumentsResponse* docResponse = _server->documents( documentIDs );
  _sendDocumentsResponse( docResponse );
}

void indri::net::NetworkServerStub::_handleDocumentsFromMetadata( indri::xml::XMLNode* request ) {
  // decode the request
  std::string attributeName;
  std::vector<std::string> attributeValues;
  indri::server::QueryServerDocumentsResponse* documents;

  _decodeMetadataRequest( request, attributeName, attributeValues );
  documents = _server->documentsFromMetadata( attributeName, attributeValues );
  _sendDocumentsResponse( documents ); 
}

void indri::net::NetworkServerStub::_handleDocumentIDsFromMetadata( indri::xml::XMLNode* request ) {
  // decode the request
  std::string attributeName;
  std::vector<std::string> attributeValues;
  indri::server::QueryServerDocumentIDsResponse* documentIDresponse;

  _decodeMetadataRequest( request, attributeName, attributeValues );
  documentIDresponse = _server->documentIDsFromMetadata( attributeName, attributeValues );
  const std::vector<lemur::api::DOCID_T>& documentIDs = documentIDresponse->getResults();
  indri::xml::XMLNode* response = new indri::xml::XMLNode( "documentIDs" );

  for( size_t i=0; i<documentIDs.size(); i++ ) {
    response->addChild( new indri::xml::XMLNode( "documentID", i64_to_string( documentIDs[i] ) ) );
  }
  delete documentIDresponse;

  _stream->reply( response );
  _stream->replyDone();
  delete response;
}

void indri::net::NetworkServerStub::_handleDocumentMetadata( indri::xml::XMLNode* request ) {
  std::vector<lemur::api::DOCID_T> documentIDs;
  std::string fieldAttributeName = "field";
  std::string field = request->getChild( fieldAttributeName )->getValue();

  const indri::xml::XMLNode* documents = request->getChild("documents");

  for( size_t i=0; i<documents->getChildren().size(); i++ ) {
    documentIDs.push_back( (lemur::api::DOCID_T) string_to_i64( documents->getChildren()[i]->getValue() ) );
  }

  // get the documents
  indri::server::QueryServerMetadataResponse* metadataResponse = _server->documentMetadata( documentIDs, field );
  std::vector<std::string> metadata = metadataResponse->getResults();
  delete metadataResponse;

  // send them back
  indri::xml::XMLNode* response = new indri::xml::XMLNode( "document-metadata" );
  for( size_t i=0; i<metadata.size(); i++ ) {
    std::string value = base64_encode( metadata[i].c_str(), (int)metadata[i].length() );
    indri::xml::XMLNode* datum = new indri::xml::XMLNode( "datum", value );
    response->addChild(datum);
  }

  _stream->reply( response );
  _stream->replyDone();
  delete response;
}

void indri::net::NetworkServerStub::_handleDocumentVectors( indri::xml::XMLNode* request ) {
  const std::vector<indri::xml::XMLNode*>& children = request->getChildren();
  indri::xml::XMLNode* response = new indri::xml::XMLNode( "document-vector" );

  // convert doc IDs into an array
  std::vector<lemur::api::DOCID_T> documentIDs;
  for( size_t i=0; i<request->getChildren().size(); i++ ) {
    documentIDs.push_back( (lemur::api::DOCID_T) string_to_i64( request->getChildren()[i]->getValue() ) );
  }

  // get the document vectors from the index
  indri::server::QueryServerVectorsResponse* vectorsResponse = _server->documentVectors( documentIDs );

  for( size_t i=0; i<vectorsResponse->getResults().size(); i++ ) {
    indri::api::DocumentVector* docVector = vectorsResponse->getResults()[i];

    indri::xml::XMLNode* docResponse = new indri::xml::XMLNode( "document" );
    indri::xml::XMLNode* stems = new indri::xml::XMLNode( "stems" );
    indri::xml::XMLNode* positions = new indri::xml::XMLNode( "positions" );
    indri::xml::XMLNode* fields = new indri::xml::XMLNode( "fields" );

    const std::vector<std::string>& stemsVector = docVector->stems();

    for( size_t j=0; j<stemsVector.size(); j++ ) {
      const std::string& stem = stemsVector[j];
      std::string encoded = base64_encode( stem.c_str(), (int)stem.length() );
      stems->addChild( new indri::xml::XMLNode( "stem", encoded ) );
    }

    const std::vector<int>& positionsVector = docVector->positions();

    for( size_t j=0; j<docVector->positions().size(); j++ ) {
      int position = positionsVector[j];
      positions->addChild( new indri::xml::XMLNode( "position", i64_to_string(position) ) );
    }

    for( size_t j=0; j<docVector->fields().size(); j++ ) {
      indri::xml::XMLNode* field = new indri::xml::XMLNode( "field" );

      std::string number = i64_to_string( docVector->fields()[j].number );
      std::string begin = i64_to_string( docVector->fields()[j].begin );
      std::string end = i64_to_string( docVector->fields()[j].end );
      std::string ordinal = i64_to_string( docVector->fields()[j].ordinal );
      std::string pOrdinal = i64_to_string( docVector->fields()[j].parentOrdinal );

      field->addChild( new indri::xml::XMLNode( "name", docVector->fields()[j].name ) );
      field->addChild( new indri::xml::XMLNode( "number", number ) );
      field->addChild( new indri::xml::XMLNode( "begin", begin ) );
      field->addChild( new indri::xml::XMLNode( "end", end ) );
      field->addChild( new indri::xml::XMLNode( "ordinal", ordinal ) );
      field->addChild( new indri::xml::XMLNode( "parentOrdinal", pOrdinal ) );

      fields->addChild( field );
    }

    docResponse->addChild(stems);
    docResponse->addChild(positions);
    docResponse->addChild(fields);

    response->addChild( docResponse );

    delete docVector;
  }

  _stream->reply( response );
  _stream->replyDone();

  delete response;
  delete vectorsResponse;
}

void indri::net::NetworkServerStub::_handleTermCount( indri::xml::XMLNode* request ) {
  INT64 termCount = _server->termCount();
  _sendNumericResponse( "term-count", termCount );
}
void indri::net::NetworkServerStub::_handleTermCountUnique( indri::xml::XMLNode* request ) {
  INT64 termCount = _server->termCountUnique();
  _sendNumericResponse( "term-count-unique", termCount );
}

void indri::net::NetworkServerStub::_handleStemCountText( indri::xml::XMLNode* request ) {
  INT64 termCount = _server->stemCount( request->getValue().c_str() );
  _sendNumericResponse( "stem-count-text", termCount );
}

void indri::net::NetworkServerStub::_handleTermCountText( indri::xml::XMLNode* request ) {
  INT64 termCount = _server->termCount( request->getValue().c_str() );
  _sendNumericResponse( "term-count-text", termCount );
}

void indri::net::NetworkServerStub::_handleTermName( indri::xml::XMLNode* request ) {
  std::string name = _server->termName( string_to_i64( request->getValue() ) );
  indri::xml::XMLNode* response = new indri::xml::XMLNode( "term-name", name );
  _stream->reply( response );
  _stream->replyDone();
  delete response;
}

void indri::net::NetworkServerStub::_handleTermID( indri::xml::XMLNode* request ) {
  lemur::api::TERMID_T termID = _server->termID( request->getValue().c_str() );
  _sendNumericResponse( "term-id", termID );
}

void indri::net::NetworkServerStub::_handleStemTerm( indri::xml::XMLNode* request ) {
  std::string name = _server->stemTerm( request->getValue()  );
  indri::xml::XMLNode* response = new indri::xml::XMLNode( "term-stem", name );
  _stream->reply( response );
  _stream->replyDone();
  delete response;
}

void indri::net::NetworkServerStub::_handleTermFieldCount( indri::xml::XMLNode* request ) {
  const indri::xml::XMLNode* termNode;
  const indri::xml::XMLNode* fieldNode;
  INT64 count;

  fieldNode = request->getChild( "field" );
  const std::string& fieldName = request->getValue();

  termNode = request->getChild( "term-text" );
  const std::string& termName = termNode->getValue();
  count = _server->termFieldCount( termName, fieldName );

  _sendNumericResponse( "term-field-count", count );
}

void indri::net::NetworkServerStub::_handleStemFieldCount( indri::xml::XMLNode* request ) {
  const indri::xml::XMLNode* termNode;
  const indri::xml::XMLNode* fieldNode;
  INT64 count;

  fieldNode = request->getChild( "field" );
  const std::string& fieldName = request->getValue();

  termNode = request->getChild( "stem-text" );
  const std::string& termName = termNode->getValue();
  count = _server->stemFieldCount( termName, fieldName );

  _sendNumericResponse( "term-field-count", count );
}

void indri::net::NetworkServerStub::_handleFieldList( indri::xml::XMLNode* request ) {
  indri::xml::XMLNode* response = new indri::xml::XMLNode( "field-list" );
  std::vector<std::string> fieldList = _server->fieldList();

  for( size_t i=0; i<fieldList.size(); i++ ) {
    response->addChild( new indri::xml::XMLNode( "field", fieldList[i] ) );
  }

  _stream->reply( response );
  _stream->replyDone();
  delete response;
}

void indri::net::NetworkServerStub::_handleDocumentLength( indri::xml::XMLNode* request ) {
  lemur::api::DOCID_T documentID = string_to_i64( request->getValue() );

  INT64 length = _server->documentLength( documentID );
  _sendNumericResponse( "document-length", length );
}

void indri::net::NetworkServerStub::_handleDocumentCount( indri::xml::XMLNode* request ) {
  INT64 count = _server->documentCount();
  _sendNumericResponse( "document-count", count );
}

void indri::net::NetworkServerStub::_handleDocumentTermCount( indri::xml::XMLNode* request ) {
  const std::string& term = request->getValue();
  INT64 count = _server->documentCount( term );
  _sendNumericResponse( "document-term-count", count );
}

void indri::net::NetworkServerStub::_handleDocumentStemCount( indri::xml::XMLNode* request ) {
  const std::string& term = request->getValue();
  INT64 count = _server->documentStemCount( term );
  _sendNumericResponse( "document-stem-count", count );
}

void indri::net::NetworkServerStub::_handlePathNames( indri::xml::XMLNode* request ) {
  std::vector<lemur::api::DOCID_T> documentIDs;
  std::vector<int> begins;
  std::vector<int> ends;

  // decode request
  const indri::xml::XMLNode* paths = request->getChild("paths");

  for( size_t i=0; i<paths->getChildren().size(); i++ ) {
    const indri::xml::XMLNode * path = paths->getChildren()[i];
    documentIDs.push_back( (lemur::api::DOCID_T) string_to_i64( path->getChild("document")->getValue() ) );
    begins.push_back( (int) string_to_i64( path->getChild("begin")->getValue() ) );
    ends.push_back( (int) string_to_i64( path->getChild("end")->getValue() ) );    
  }

  // get the paths
  indri::server::QueryServerMetadataResponse* pathNameResponse = _server->pathNames( documentIDs, begins, ends );
  std::vector<std::string> pathName = pathNameResponse->getResults();

  // send them back
  indri::xml::XMLNode* response = new indri::xml::XMLNode( "path-names" );
  for( size_t i=0; i<pathName.size(); i++ ) {
    std::string value = base64_encode( pathName[i].c_str(), (int)pathName[i].length() );
    indri::xml::XMLNode* datum = new indri::xml::XMLNode( "name", value );
    response->addChild(datum);
  } 

  _stream->reply( response );
  _stream->replyDone();
  delete pathNameResponse;
  delete response;
}

void indri::net::NetworkServerStub::_handleSetMaxWildcardTerms( indri::xml::XMLNode* request ) {
  int nTerms = string_to_int( request->getValue() );
  _server->setMaxWildcardTerms(nTerms);
  _sendNumericResponse( "max-wildcard-terms", nTerms );
}

void indri::net::NetworkServerStub::request( indri::xml::XMLNode* input ) {
  try {
    const std::string& type = input->getName();

    if( type == "query" ) {
      _handleQuery( input );
    } else if( type == "documents" ) {
      _handleDocuments( input );
    } else if( type == "document-metadata" ) {
      _handleDocumentMetadata( input );
    } else if( type == "document-vectors" ) {
      _handleDocumentVectors( input );
    } else if( type == "term-field-count" ) {
      _handleTermFieldCount( input );
    } else if( type == "stem-field-count" ) {
      _handleStemFieldCount( input );
    } else if( type == "term-id" ) {
      _handleTermID( input );
    } else if( type == "term-name" ) {
      _handleTermName( input );
    } else if( type == "term-stem" ) {
      _handleStemTerm( input );
    } else if( type == "term-count" ) {
      _handleTermCount( input );
    } else if( type == "term-count-unique" ) {
      _handleTermCountUnique( input );
    } else if( type == "term-count-text" ) {
      _handleTermCountText( input );
    } else if( type == "stem-count-text" ) {
      _handleStemCountText( input );
    } else if( type == "field-list" ) {
      _handleFieldList( input );
    } else if( type == "document-length" ) {
      _handleDocumentLength( input );
    } else if( type == "document-count" ) {
      _handleDocumentCount( input );
    } else if( type == "document-term-count" ) {
      _handleDocumentTermCount( input );
    } else if( type == "document-stem-count" ) {
      _handleDocumentStemCount( input );
    } else if( type == "docids-from-metadata" ) {
      _handleDocumentIDsFromMetadata( input );
    } else if( type == "documents-from-metadata" ) {
      _handleDocumentsFromMetadata( input );
    } else if( type == "path-names" ) {
      _handlePathNames( input );
    } else if( type == "max-wildcard-terms" ) {
      _handleSetMaxWildcardTerms( input );
    } else {
      _stream->error( std::string() + "Unknown XML message type: " + input->getName() );
    }
  } catch( lemur::api::Exception& e ) {
    _stream->error( e.what() );
  } catch( ... ) {
    _stream->error( "Caught unknown exception while processing request" );
  }
}

void indri::net::NetworkServerStub::reply( indri::xml::XMLNode* input ) {
  assert( false && "Shouldn't ever get a reply on the server" );
}

void indri::net::NetworkServerStub::reply( const std::string& name, const void* buffer, unsigned int length ) {
  assert( false && "Shouldn't ever get a reply on the server" );
}

void indri::net::NetworkServerStub::replyDone() {
  assert( false && "Shouldn't ever get a reply on the server" );
}

void indri::net::NetworkServerStub::error( const std::string& error ) {
  // TODO: fix this to trap the error and propagate up the chain on the next request.
  std::cout << "Caught error message from client: " << error.c_str() << std::endl;
}

void indri::net::NetworkServerStub::run() {
  while( _stream->alive() )
    _stream->read(*this);

}
