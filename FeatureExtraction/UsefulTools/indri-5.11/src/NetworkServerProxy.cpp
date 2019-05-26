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
// 20 May 2004 -- tds
//

#include "indri/NetworkServerProxy.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/ScopedLock.hpp"
#include <iostream>

namespace indri
{
  namespace server
  {
    
    //
    // NetworkServerProxyResponse
    //
    class NetworkServerProxyResponse : public QueryServerResponse {
    private:
      indri::net::QueryResponseUnpacker _unpacker;
      indri::net::NetworkMessageStream* _stream;

    public:
      NetworkServerProxyResponse( indri::net::NetworkMessageStream* stream ) :
        _unpacker( stream ),
        _stream( stream )
      {
      }

      ~NetworkServerProxyResponse() {
        _stream->mutex().unlock();
      }

      indri::infnet::InferenceNetwork::MAllResults& getResults() {
        return _unpacker.getResults();
      }
    };

    //
    // NetworkServerProxyDocumentsResponse
    //

    class NetworkServerProxyDocumentsResponse : public QueryServerDocumentsResponse {
    private:
      indri::net::NetworkMessageStream* _stream;
      std::vector<indri::api::ParsedDocument*> _documents;

    public:
      NetworkServerProxyDocumentsResponse( indri::net::NetworkMessageStream* stream ) :
        _stream(stream)
      {
      }

      ~NetworkServerProxyDocumentsResponse() {
        _stream->mutex().unlock();
      }

      // caller deletes the ParsedDocuments
      std::vector<indri::api::ParsedDocument*>& getResults() {
        indri::net::XMLReplyReceiver r;
        r.wait(_stream);

        if( r.getReply() ) {
          indri::xml::XMLNode* reply = r.getReply();
          size_t numChildren = reply->getChildren().size();

          for( size_t i=0; i<numChildren; i++ ) {
            indri::utility::Buffer buffer;
            int textOffset = 0;
            indri::utility::greedy_vector<int> metadataKeyOffset;
            indri::utility::greedy_vector<int> metadataValueOffset;

            const indri::xml::XMLNode* child = reply->getChildren()[i];
            const indri::xml::XMLNode* metadata = child->getChild( "metadata" );

            // allocate room for the ParsedDocument
            buffer.write( sizeof(indri::api::ParsedDocument) );

            if( metadata ) {
              for( size_t j=0; j<metadata->getChildren().size(); j++ ) {
                std::string key = metadata->getChildren()[j]->getChildValue("key");
                std::string value = metadata->getChildren()[j]->getChildValue("value");           
            
                metadataKeyOffset.push_back( (int)buffer.position() );
                strcpy( buffer.write( key.size()+1 ), key.c_str() );

                metadataValueOffset.push_back( (int)buffer.position() );
                int length = base64_decode( buffer.write( value.size() ), (int)value.size(), value );
                buffer.unwrite( value.size()-length );
              }
            }

            const indri::xml::XMLNode* textNode = child->getChild("text");
            std::string text;

            if( textNode ) {
              text = textNode->getValue();
              textOffset = (int)buffer.position();
              int length = base64_decode( buffer.write( text.size() ), (int)text.size(), text );
              buffer.unwrite( text.size()-length );
            }

            INT64 contentOffset = 0;
            INT64 contentLength = 0;
            const indri::xml::XMLNode* contentNode = child->getChild("content");
            if (contentNode) {
              contentOffset = string_to_i64(contentNode->getValue());
            }
            const indri::xml::XMLNode* contentLengthNode = child->getChild("contentLength");
            if (contentLengthNode) {
              contentLength = string_to_i64(contentLengthNode->getValue());
            }
            
            // now all of our data is in the buffer, so we can allocate a return structure
            new(buffer.front()) indri::api::ParsedDocument;
            indri::api::ParsedDocument* parsedDocument = (indri::api::ParsedDocument*) buffer.front();

            const indri::xml::XMLNode* positions = child->getChild( "positions" );

            if( positions ) {
              const std::vector<indri::xml::XMLNode*>& children = positions->getChildren();
          
 
              for( size_t j=0; j<children.size(); j++ ) {
                indri::parse::TermExtent extent;
                std::string begin = children[j]->getChildValue("begin");
                std::string end = children[j]->getChildValue("end");    

                extent.begin = (int) string_to_i64( begin );
                extent.end = (int) string_to_i64( end );

                parsedDocument->positions.push_back( extent );
              }
            }

            for( size_t j=0; j<metadataKeyOffset.size(); j++ ) {
              indri::parse::MetadataPair pair;

              pair.key = buffer.front() + metadataKeyOffset[j];
              pair.value = buffer.front() + metadataValueOffset[j];

              if( metadataKeyOffset.size() > j+1 )
                pair.valueLength = metadataKeyOffset[j+1] - metadataValueOffset[j];
              else
                pair.valueLength = textOffset;

              parsedDocument->metadata.push_back( pair );
            }

            parsedDocument->text = buffer.front() + textOffset;
            parsedDocument->textLength = buffer.position() - textOffset;
            parsedDocument->content = parsedDocument->text + contentOffset;
            parsedDocument->contentLength = contentLength;
            buffer.detach();

            _documents.push_back( parsedDocument );
          }
        }
  
        return _documents;
      }
    };

    //
    // NetworkServerProxyMetadataResponse
    //

    class NetworkServerProxyMetadataResponse : public QueryServerMetadataResponse {
    private:
      std::vector<std::string> _metadata;
      indri::net::NetworkMessageStream* _stream;

    public:
      NetworkServerProxyMetadataResponse( indri::net::NetworkMessageStream* stream ) :
        _stream(stream)
      {
      }

      ~NetworkServerProxyMetadataResponse() {
        _stream->mutex().unlock();
      }

      std::vector<std::string>& getResults() {
        indri::net::XMLReplyReceiver r;
        r.wait(_stream);

        // parse the result
        indri::xml::XMLNode* reply = r.getReply();
        indri::utility::Buffer metadataBuffer;

        for( size_t i=0; i<reply->getChildren().size(); i++ ) {
          const indri::xml::XMLNode* meta = reply->getChildren()[i];
          const std::string& input = meta->getValue();

          std::string value;
          base64_decode_string( value, input );
          _metadata.push_back(value);

          metadataBuffer.clear();
        }

        return _metadata;
      }
    };

    //
    // NetworkServerProxyVectorsResponse
    //

    class NetworkServerProxyVectorsResponse : public QueryServerVectorsResponse {
    public:
      std::vector<indri::api::DocumentVector*> _vectors;
      indri::net::NetworkMessageStream* _stream;
      bool _readResponse;

    public:
      NetworkServerProxyVectorsResponse( indri::net::NetworkMessageStream* stream ) 
        :
        _stream(stream),
        _readResponse(false)
      {
      }

      ~NetworkServerProxyVectorsResponse() {
        _stream->mutex().unlock();
      }

      std::vector<indri::api::DocumentVector*>& getResults() {
        if( !_readResponse ) {
          indri::net::XMLReplyReceiver r;
          r.wait( _stream );

          const indri::xml::XMLNode* reply = r.getReply();
          const std::vector<indri::xml::XMLNode*>& children = reply->getChildren();

          for( size_t i=0; i<children.size(); i++ ) {
            const indri::xml::XMLNode* stems = children[i]->getChild("stems");
            const indri::xml::XMLNode* positions = children[i]->getChild("positions");
            const indri::xml::XMLNode* fields = children[i]->getChild("fields");

            indri::api::DocumentVector* result = new indri::api::DocumentVector;

            for( size_t j=0; j<stems->getChildren().size(); j++ ) {
              // have to use base64 coding, in case the stem contains '<', '>', etc.
              std::string stem;
              base64_decode_string(stem, stems->getChildren()[j]->getValue());
              result->stems().push_back( stem );
            }

            std::vector<int>& positionsVector = result->positions();

            for( size_t j=0; j<positions->getChildren().size(); j++ ) {
              const std::string& stringText = positions->getChildren()[j]->getValue();
              INT64 position = string_to_i64( stringText );
              positionsVector.push_back( int(position) );
            }

            std::vector<indri::api::DocumentVector::Field>& fieldVector = result->fields();

            for( size_t j=0; j<fields->getChildren().size(); j++ ) {
              const indri::xml::XMLNode* field = fields->getChildren()[j];

              const indri::xml::XMLNode* nameField = field->getChild("name");
              const indri::xml::XMLNode* numberField = field->getChild("number");
              const indri::xml::XMLNode* beginField = field->getChild("begin");
              const indri::xml::XMLNode* endField = field->getChild("end");
              const indri::xml::XMLNode* ordinalField = field->getChild("ordinal");
              const indri::xml::XMLNode* pOrdinalField = field->getChild("parentOrdinal");

              indri::api::DocumentVector::Field f;

              f.name = nameField->getValue();
              f.number = string_to_i64( numberField->getValue() );
              f.begin = int(string_to_i64( beginField->getValue() ));
              f.end = int(string_to_i64( endField->getValue() ));
              f.ordinal = int(string_to_i64( ordinalField->getValue() ));
              f.parentOrdinal = int(string_to_i64( pOrdinalField->getValue() ));

              fieldVector.push_back(f);
            }

            _vectors.push_back(result);
          }

          _readResponse = true;
        }

        return _vectors;
      }
    };

    //
    // NetworkServerProxyDocumentIDsResponse
    //

    class NetworkServerProxyDocumentIDsResponse : public QueryServerDocumentIDsResponse {
    public:
      std::vector<lemur::api::DOCID_T> _documentIDs;
      indri::net::NetworkMessageStream* _stream;
      bool _readResponse;

    public:
      NetworkServerProxyDocumentIDsResponse( indri::net::NetworkMessageStream* stream ) 
        :
        _stream(stream),
        _readResponse(false)
      {
      }

      ~NetworkServerProxyDocumentIDsResponse() {
        _stream->mutex().unlock();
      }

      std::vector<lemur::api::DOCID_T>& getResults() {
        if( !_readResponse ) {
          indri::net::XMLReplyReceiver r;
          r.wait( _stream );

          const indri::xml::XMLNode* reply = r.getReply();
          const std::vector<indri::xml::XMLNode*>& children = reply->getChildren();
      
          for( size_t i=0; i<children.size(); i++ ) {
            indri::xml::XMLNode* child = children[i];
            _documentIDs.push_back( string_to_i64( child->getValue() ) );
          }
        }

        return _documentIDs;
      }
    };
  }
}

//
// NetworkServerProxy code
//


indri::server::NetworkServerProxy::NetworkServerProxy( indri::net::NetworkMessageStream* stream ) :
  _stream(stream)
{
}

//
// _numericRequest
//
// Sends a request for a numeric quantity; deletes the node
// passed in as a parameter
//

INT64 indri::server::NetworkServerProxy::_numericRequest( indri::xml::XMLNode* node ) {
  indri::thread::ScopedLock lock( _stream->mutex() );
  _stream->request( node );
  delete node;

  indri::net::XMLReplyReceiver r;
  r.wait( _stream );

  indri::xml::XMLNode* reply = r.getReply();
  return string_to_i64( reply->getValue() );
}

//
// _stringRequest
//
// Sends a request for a string quantity; deletes the node
// passed in as a parameter
//

std::string indri::server::NetworkServerProxy::_stringRequest( indri::xml::XMLNode* node ) {
  indri::thread::ScopedLock lock( _stream->mutex() );
  _stream->request( node );
  delete node;

  indri::net::XMLReplyReceiver r;
  r.wait( _stream );

  indri::xml::XMLNode* reply = r.getReply();
  return reply->getValue();
}

//
// runQuery
//

indri::server::QueryServerResponse* indri::server::NetworkServerProxy::runQuery( std::vector<indri::lang::Node*>& roots, int resultsRequested, bool optimize ) {
  indri::lang::Packer packer;

  for( size_t i=0; i<roots.size(); i++ ) {
    packer.pack( roots[i] );
  }

  indri::xml::XMLNode* query = packer.xml();
  query->addAttribute( "resultsRequested", i64_to_string(resultsRequested) );
  query->addAttribute( "optimize", optimize ? "1" : "0" );

  _stream->mutex().lock();
  _stream->request( query );

  return new indri::server::NetworkServerProxyResponse( _stream );
}

//
// documentMetadata
//

indri::server::QueryServerMetadataResponse* indri::server::NetworkServerProxy::documentMetadata( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::string& attributeName ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "document-metadata" );
  indri::xml::XMLNode* field = new indri::xml::XMLNode( "field", attributeName );
  indri::xml::XMLNode* documents = new indri::xml::XMLNode( "documents" );

  // build request
  for( size_t i=0; i<documentIDs.size(); i++ ) {
    documents->addChild( new indri::xml::XMLNode( "document", i64_to_string( documentIDs[i] ) ) );
  }
  request->addChild( field );
  request->addChild( documents );

  // send request
  _stream->mutex().lock();
  _stream->request( request );
  delete request;

  return new indri::server::NetworkServerProxyMetadataResponse( _stream );
}

//
// pathNames
//

indri::server::QueryServerMetadataResponse* indri::server::NetworkServerProxy::pathNames( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::vector<int>& begins, const std::vector<int>& ends ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "path-names" );
  indri::xml::XMLNode* documents = new indri::xml::XMLNode( "paths" );

  // build request
  for( size_t i=0; i<documentIDs.size(); i++ ) {
    documents->addChild( new indri::xml::XMLNode( "document", i64_to_string( documentIDs[i] ) ) );
    documents->addChild( new indri::xml::XMLNode( "begin", i64_to_string( begins[i] ) ) );
    documents->addChild( new indri::xml::XMLNode( "end", i64_to_string( ends[i] ) ) );
  }
  request->addChild( documents );

  // send request
  _stream->mutex().lock();
  _stream->request( request );
  delete request;

  return new indri::server::NetworkServerProxyMetadataResponse( _stream );
}

//
// documents
//

indri::server::QueryServerDocumentsResponse* indri::server::NetworkServerProxy::documents( const std::vector<lemur::api::DOCID_T>& documentIDs ) {
  indri::xml::XMLNode* docRequest = new indri::xml::XMLNode( "documents" );

  for( size_t i=0; i<documentIDs.size(); i++ ) {
    docRequest->addChild( new indri::xml::XMLNode("doc", i64_to_string(documentIDs[i])) );
  }

  _stream->mutex().lock();
  _stream->request( docRequest );
  delete docRequest;

  return new indri::server::NetworkServerProxyDocumentsResponse( _stream );
}

//
// documentsFromMetadata
//

indri::server::QueryServerDocumentsResponse* indri::server::NetworkServerProxy::documentsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues ) {
  indri::xml::XMLNode* docRequest = new indri::xml::XMLNode( "documents-from-metadata" );

  // store the attribute name
  docRequest->addChild( new indri::xml::XMLNode( "attributeName", attributeName ) );

  // serialize the attributeValues (in a tree called "attributeValues", with members called "attributeValue")
  indri::xml::XMLNode* attributeValuesNode = new indri::xml::XMLNode( "attributeValues" );
  for( size_t i=0; i<attributeValues.size(); i++ ) {
    attributeValuesNode->addChild( new indri::xml::XMLNode( "attributeValue", attributeValues[i] ) );
  }
  docRequest->addChild( attributeValuesNode );

  // request the documents
  _stream->mutex().lock();
  _stream->request( docRequest );
  delete docRequest;

  return new indri::server::NetworkServerProxyDocumentsResponse( _stream );
}

//
// documentIDsFromMetadata
//

indri::server::QueryServerDocumentIDsResponse* indri::server::NetworkServerProxy::documentIDsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues ) {
  indri::xml::XMLNode* docRequest = new indri::xml::XMLNode( "docids-from-metadata" );
 
  // store the attribute name
  docRequest->addChild( new indri::xml::XMLNode( "attributeName", attributeName ) );

  // serialize the attributeValues (in a tree called "attributeValues", with members called "attributeValue")
  indri::xml::XMLNode* attributeValuesNode = new indri::xml::XMLNode( "attributeValues" );
  for( size_t i=0; i<attributeValues.size(); i++ ) {
    attributeValuesNode->addChild( new indri::xml::XMLNode( "attributeValue", attributeValues[i] ) );
  }
  docRequest->addChild( attributeValuesNode );

  // request the documents
  _stream->mutex().lock();
  _stream->request( docRequest );
  delete docRequest;

  return new indri::server::NetworkServerProxyDocumentIDsResponse( _stream );
}

//
// termCount
//

INT64 indri::server::NetworkServerProxy::termCount() {
  //  std::auto_ptr<indri::xml::XMLNode> request( new indri::xml::XMLNode( "term-count" ) );
  indri::xml::XMLNode *request = new indri::xml::XMLNode( "term-count" ) ;
  return _numericRequest( request );
}

//
// termCount
//

INT64 indri::server::NetworkServerProxy::termCount( const std::string& term ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "term-count-text", term );
  return _numericRequest( request );
}

INT64 indri::server::NetworkServerProxy::termCountUnique( ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "term-count-unique" );
  return _numericRequest( request );
}

INT64 indri::server::NetworkServerProxy::stemCount( const std::string& term ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "stem-count-text", term );
  return _numericRequest( request );
}

std::string indri::server::NetworkServerProxy::termName( lemur::api::TERMID_T term ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "term-name", i64_to_string(term) );
  return _stringRequest( request );
}

std::string indri::server::NetworkServerProxy::stemTerm( const std::string & term ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "term-stem", term );
  return _stringRequest( request );
}

lemur::api::TERMID_T indri::server::NetworkServerProxy::termID( const std::string& term ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "term-id", term );
  return _numericRequest( request );
}

INT64 indri::server::NetworkServerProxy::termFieldCount( const std::string& term, const std::string& field ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "term-field-count" );
  indri::xml::XMLNode* termNode = new indri::xml::XMLNode( "term-text", term );
  indri::xml::XMLNode* fieldNode = new indri::xml::XMLNode( "field", field );
  request->addChild( termNode );
  request->addChild( fieldNode );

  return _numericRequest( request );
}

INT64 indri::server::NetworkServerProxy::stemFieldCount( const std::string& stem, const std::string& field ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "stem-field-count" );
  indri::xml::XMLNode* stemNode = new indri::xml::XMLNode( "stem-text", stem );
  indri::xml::XMLNode* fieldNode = new indri::xml::XMLNode( "field", field );
  request->addChild( stemNode );
  request->addChild( fieldNode );

  return _numericRequest( request );
}

std::vector<std::string> indri::server::NetworkServerProxy::fieldList() {
  std::auto_ptr<indri::xml::XMLNode> request( new indri::xml::XMLNode( "field-list" ) );
  indri::thread::ScopedLock( _stream->mutex() );
  _stream->request( request.get() );

  indri::net::XMLReplyReceiver r;
  r.wait( _stream );

  indri::xml::XMLNode* reply = r.getReply();
  const std::vector<indri::xml::XMLNode*>& children = reply->getChildren();
  std::vector<std::string> result;

  for( size_t i=0; i<children.size(); i++ ) {
    result.push_back( children[i]->getValue() );
  }

  return result;
}

int indri::server::NetworkServerProxy::documentLength( lemur::api::DOCID_T documentID ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "document-length", i64_to_string(documentID) );
  return (int) _numericRequest( request );
}

INT64 indri::server::NetworkServerProxy::documentCount() {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "document-count" );
  return _numericRequest( request );
}

INT64 indri::server::NetworkServerProxy::documentCount( const std::string& term ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "document-term-count", term );
  return _numericRequest( request );
}

INT64 indri::server::NetworkServerProxy::documentStemCount( const std::string& term ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "document-stem-count", term );
  return _numericRequest( request );
}

indri::server::QueryServerVectorsResponse* indri::server::NetworkServerProxy::documentVectors( const std::vector<lemur::api::DOCID_T>& documentIDs ) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "document-vectors" );

  for( size_t i=0; i<documentIDs.size(); i++ ) {
    request->addChild( new indri::xml::XMLNode( "document", i64_to_string(documentIDs[i]) ) );
  }

  _stream->mutex().lock();
  _stream->request( request );
  delete request;

  return new indri::server::NetworkServerProxyVectorsResponse( _stream );
}

//
// setMaxWildcardTerms
//
void indri::server::NetworkServerProxy::setMaxWildcardTerms(int maxTerms) {
  indri::xml::XMLNode* request = new indri::xml::XMLNode( "max-wildcard-terms", i64_to_string(maxTerms) );
  INT64 termMax=_numericRequest( request );
}

