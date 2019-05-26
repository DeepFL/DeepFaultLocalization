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
// IndexEnvironment
//
// 19 July 2004 -- tds
//

#include "indri/IndexEnvironment.hpp"
#include "indri/CompressedCollection.hpp"
#include "lemur/Exception.hpp"
#include "indri/AnchorTextAnnotator.hpp"
#include "indri/TaggedDocumentIterator.hpp"
#include "indri/IndriParser.hpp"
#include "indri/IndriTokenizer.hpp"
#include "indri/Path.hpp"
#include "indri/Conflater.hpp"
#include <iostream>

void indri::api::IndexEnvironment::_getParsingContext( indri::parse::Parser** parser,
                                                       indri::parse::Tokenizer** tokenizer,
                                                       indri::parse::DocumentIterator** iterDoc,
                                                       indri::parse::Conflater** conflater,
                                                       const std::string& className ) {
  std::string parserName;
  std::string iteratorName;

  *parser = 0;
  *tokenizer = 0;
  *iterDoc = 0;
  *conflater = 0;

  // look for an already-built environment
  std::map<std::string, indri::parse::FileClassEnvironment*>::iterator iter;
  iter = _environments.find(className);

  if( iter != _environments.end() ) {
    *parser = iter->second->parser;
    *tokenizer = iter->second->tokenizer;
    *iterDoc = iter->second->iterator;
    *conflater = iter->second->conflater;
    return;
  }

  indri::parse::FileClassEnvironment* fce = _fileClassFactory.get( className );

  if( fce ) {
    _environments[className] = fce;
    *parser = fce->parser;
    *tokenizer = fce->tokenizer;
    *iterDoc = fce->iterator;
    *conflater = fce->conflater;
  }
}

std::vector<indri::parse::Transformation*> indri::api::IndexEnvironment::_createAnnotators( const std::string& fileName,
                                                                                            const std::string& fileClass, 
                                                                                            indri::parse::Conflater** conflater ) {
  indri::parse::AnchorTextAnnotator* annotator = 0;
  indri::parse::OffsetAnnotationAnnotator* oa_annotator = 0;
  indri::parse::OffsetMetadataAnnotator* om_annotator = 0;

  std::vector<indri::parse::Transformation*> annotators;

  // get the path to the filename relative to the document root
  std::string relativePath = indri::file::Path::relative( _documentRoot, fileName );

  if( _anchorTextRoot.length() ) {
    // if the user specified some anchor text, we'll add it in
    std::string anchorTextPath;
    if( relativePath.length() > 0 )
      anchorTextPath = indri::file::Path::combine( _anchorTextRoot, relativePath );
    else
      anchorTextPath = _anchorTextRoot;
    _annotator.open( anchorTextPath );
    annotator = &_annotator;
    annotators.push_back( annotator ); 
  }

  if( _offsetAnnotationsRoot.length() ) {
    // If the user specified an offset annotations file, we'll use it.
    std::string offsetAnnotationsPath;
    // only do path combining on directories. Otherwise, assume a single file
    // for all corpus documents.
    if( indri::file::Path::isDirectory( _offsetAnnotationsRoot ) && relativePath.length() > 0 )
      offsetAnnotationsPath = indri::file::Path::combine( _offsetAnnotationsRoot, relativePath );
    else
      offsetAnnotationsPath = _offsetAnnotationsRoot;
    _oa_annotator.setConflater( *conflater );
    _oa_annotator.open( offsetAnnotationsPath );
    oa_annotator = &_oa_annotator;
    annotators.push_back( oa_annotator );
  }

  if( _offsetMetadataRoot.length() ) {
    // If the user specified an offset metadata path, we'll use it.
    std::string offsetMetadataPath;
    // only do path combining on directories. Otherwise, assume a single file
    // for all corpus documents.
    if( indri::file::Path::isDirectory( _offsetMetadataRoot ) && relativePath.length() > 0 ) 
      offsetMetadataPath = indri::file::Path::combine( _offsetMetadataRoot, relativePath );
    else 
      offsetMetadataPath = _offsetMetadataRoot;
    _om_annotator.open( offsetMetadataPath );
    om_annotator = &_om_annotator;
    annotators.push_back( om_annotator );
  }

  return annotators;
}

indri::api::ParsedDocument* indri::api::IndexEnvironment::_applyAnnotators( std::vector<indri::parse::Transformation*>& annotators,
                                                                            indri::api::ParsedDocument* parsed ) {
  for( size_t i=0; i<annotators.size(); i++ ) {
    if( annotators[i] ) { 
      parsed = annotators[i]->transform( parsed ); 
    }
  }

  return parsed;
}

indri::api::IndexEnvironment::IndexEnvironment() :
  _callback(0),
  _options(0),
  _documentsIndexed(0),
  _documentsSeen(0)
{
}

indri::api::IndexEnvironment::~IndexEnvironment() {
  close();
  indri::utility::delete_map_contents<std::string, indri::parse::FileClassEnvironment>( _environments );
}

int indri::api::IndexEnvironment::setBlackList( const std::string& blackListFile)
{
    ifstream inf(blackListFile.data(), ifstream::in);

    std::string line;

    while(getline(inf, line))
    {
        _blackedDocs.insert(line);
    }

    inf.close();

    return _blackedDocs.size();
}


void indri::api::IndexEnvironment::setNormalization( bool flag ) {
  _parameters.set( "normalize", flag );
}

void indri::api::IndexEnvironment::setStoreDocs( bool flag ) {
  _parameters.set( "storeDocs", flag );
}

void indri::api::IndexEnvironment::setInjectURL( bool flag ) {
  _parameters.set( "injectURL", flag );
}

void indri::api::IndexEnvironment::setMemory( UINT64 memory ) {
  _parameters.set("memory", memory);
}

void indri::api::IndexEnvironment::setOffsetAnnotationsPath( const std::string& offsetAnnotationsRoot ) {
  _offsetAnnotationsRoot = offsetAnnotationsRoot;
}

void indri::api::IndexEnvironment::setOffsetMetadataPath( const std::string& offsetMetadataRoot ) {
  _offsetMetadataRoot = offsetMetadataRoot;
}

void indri::api::IndexEnvironment::setAnchorTextPath( const std::string& anchorTextRoot ) {
  _anchorTextRoot = anchorTextRoot;
}

void indri::api::IndexEnvironment::setDocumentRoot( const std::string& documentRoot ) {
  _documentRoot = documentRoot;
}

void indri::api::IndexEnvironment::setStopwords( const std::vector<std::string>& stopwords ) {
  _parameters.set("stopper","");
  Parameters p = _parameters.get("stopper");
  for( unsigned int i=0; i<stopwords.size(); i++ ) {
    p.append("word").set(stopwords[i]);
  }
}

void indri::api::IndexEnvironment::setStemmer( const std::string& stemmer ) {
  _parameters.set("stemmer.name", stemmer);
}

void indri::api::IndexEnvironment::setOffsetAnnotationIndexHint(indri::parse::OffsetAnnotationIndexHint hintType) {
  _oa_annotator.setHint(hintType);
}

void indri::api::IndexEnvironment::addFileClass( const std::string& name, 
                                                 const std::string& iter,
                                                 const std::string& parser,
                                                 const std::string& tokenizer,
                                                 const std::string& startDocTag,
                                                 const std::string& endDocTag,
                                                 const std::string& endMetadataTag,
                                                 const std::vector<std::string>& include,
                                                 const std::vector<std::string>& exclude,
                                                 const std::vector<std::string>& index,
                                                 const std::vector<std::string>& metadata, 
                                                 const std::map<indri::parse::ConflationPattern*,std::string>& conflations )
{
  this->_fileClassFactory.addFileClass( name, iter, parser, tokenizer, startDocTag, endDocTag, endMetadataTag,
                                        include, exclude, index, metadata, conflations );

}

void indri::api::IndexEnvironment::setIndexedFields( const std::vector<std::string>& fieldNames ) {
  bool existingFields = _parameters.exists( "field" );
  // if setNumericField or setOrdinalField was called before setIndexedFields, there
  // will be an existing entry in the fields.
  for( unsigned int i=0; i<fieldNames.size(); i++) { 
    bool found = false;
    if( existingFields ) {
      Parameters fields = _parameters["field"];
      for( size_t j=0; j<fields.size(); j++ ) {
        std::string parameterFieldName = fields[j]["name"];
        if( parameterFieldName == fieldNames[i] ) {
          // already there, don't add
          found = true;
        }
      }
    }
    if (! found) 
      _parameters.append("field").set("name",fieldNames[i]);
  }
}

void indri::api::IndexEnvironment::setNumericField( const std::string& fieldName, bool isNumeric, const std::string & parserName ) {
  bool existingFields = _parameters.exists( "field" );

  if (existingFields) {
    Parameters fields = _parameters["field"];
    for( size_t i=0; i<fields.size(); i++ ) {
      std::string parameterFieldName = fields[i]["name"];

      if( parameterFieldName == fieldName ) {
        fields[i].set( "numeric", isNumeric );
        fields[i].set( "parserName", parserName );
        return;
      }
    }
  }
  Parameters field = _parameters.append("field");
  field.set( "name", fieldName );
  field.set( "numeric", isNumeric );
  field.set( "parserName", parserName );
}

void indri::api::IndexEnvironment::setOrdinalField( const std::string& fieldName, bool isOrdinal ) {
  bool existingFields = _parameters.exists( "field" );

  if ( existingFields ) {
    Parameters fields = _parameters["field"];
    
    for( size_t i=0; i<fields.size(); i++ ) {
      std::string parameterFieldName = fields[i]["name"];
      
      if( parameterFieldName == fieldName ) {
        fields[i].set( "ordinal", isOrdinal );
        return;
      }
    }
  }  
  Parameters field = _parameters.append("field");
  field.set( "name", fieldName );
  field.set( "ordinal", isOrdinal );
}

void indri::api::IndexEnvironment::setParentalField( const std::string& fieldName, bool isParental ) {
  bool existingFields = _parameters.exists( "field" );
  
  if ( existingFields ) {
    Parameters fields = _parameters["field"];

    for( int i=0; i<fields.size(); i++ ) {
      std::string parameterFieldName = fields[i]["name"];

      if( parameterFieldName == fieldName ) {
        fields[i].set( "parental", isParental );
        return;
      }
    }
  }
  Parameters field = _parameters.append("field");
  field.set( "name", fieldName );
  field.set( "parental", isParental );
}


//
// setMetadataIndexedFields
//

void indri::api::IndexEnvironment::setMetadataIndexedFields( const std::vector<std::string>& forwardFields, const std::vector<std::string>& backwardFields ) {
  if( !_parameters.exists("collection") )
    _parameters.set("collection", "");

  Parameters collection = _parameters.get("collection");
  for( unsigned int i=0; i<forwardFields.size(); i++ ) {
    if( i==0 )
      collection.set("forward", forwardFields[i]);
    else
      collection.append("forward").set(forwardFields[i]);
  }

  for( unsigned int i=0; i<backwardFields.size(); i++ ) {
    if( i==0 )
      collection.set("backward", backwardFields[i]);
    else
      collection.append("backward").set(backwardFields[i]);
  }
}

//
// create
//

void indri::api::IndexEnvironment::create( const std::string& repositoryPath, indri::api::IndexStatus* callback ) {
  _callback = callback;
  _repository.create( repositoryPath, &_parameters );
}

//
// open
//

void indri::api::IndexEnvironment::open( const std::string& repositoryPath, indri::api::IndexStatus* callback ) {
  _callback = callback;
  _repository.open( repositoryPath, &_parameters );
}

//
// close
//

void indri::api::IndexEnvironment::close() {
  _repository.close();
}

//
// addFile
//

void indri::api::IndexEnvironment::addFile( const std::string& fileName ) {
  std::string extension = indri::file::Path::extension( fileName );
  addFile( fileName, extension );
}

//
// addFile
//

void indri::api::IndexEnvironment::addFile( const std::string& fileName, const std::string& fileClass ) {
  indri::parse::Parser* parser = 0;
  indri::parse::Tokenizer* tokenizer = 0;
  indri::parse::DocumentIterator* iterator = 0;
  indri::parse::Conflater* conflater = 0;
  
  _getParsingContext( &parser, &tokenizer, &iterator, &conflater, fileClass );

  if( !parser || !iterator ) {
    _documentsSeen++;
    if( _callback ) (*_callback) ( indri::api::IndexStatus::FileSkip, fileName, _error, _documentsIndexed, _documentsSeen );
  } else {
    try {
      indri::parse::UnparsedDocument* document;
      indri::parse::TokenizedDocument* tokenized;
      ParsedDocument* parsed;

      iterator->open( fileName );

      std::vector<indri::parse::Transformation*> annotators = _createAnnotators( fileName, fileClass, &conflater );

      // notify caller that the file was successfully parsed
      if( _callback ) (*_callback)( indri::api::IndexStatus::FileOpen, fileName, _error, _documentsIndexed, _documentsSeen );

      while( (document = iterator->nextDocument()) ) {
        _documentsSeen++;

        tokenized = tokenizer->tokenize( document );
        parsed = parser->parse( tokenized );
        parsed = _applyAnnotators( annotators, parsed );
        // can't know for sure that we have the docno element until after
        // the annotators have been applied
        // check if this document is in the index already
        // find the docno
        std::string docIDStr = "";
        for( size_t i=0; i<parsed->metadata.size(); i++ ) {
          const char * key = parsed->metadata[i].key;
          if( !strcmp( key, "docno" ) ) {
            docIDStr = (const char *)parsed->metadata[i].value;
            break;
          }
        }
        // Check if blacklisted
        if(_blackedDocs.find(docIDStr)!=_blackedDocs.end())
            continue;

        // look up the id.
        std::vector<lemur::api::DOCID_T> ids = _repository.collection()->retrieveIDByMetadatum("docno", docIDStr);
        // if not found, add the document.
        if (ids.size() == 0)  {
            _repository.addDocument( parsed );
            _documentsIndexed++;
        } // else mention the dupe?
        
        if( _callback ) (*_callback)( indri::api::IndexStatus::DocumentCount, fileName, _error, _documentsIndexed, _documentsSeen );
      }

      // notify caller that the file was successfully closed
      if( _callback ) (*_callback)( indri::api::IndexStatus::FileClose, fileName, _error, _documentsIndexed, _documentsSeen );

      iterator->close();
    } catch( lemur::api::Exception& e ) {
      if( iterator )
        iterator->close();

      // notify caller of errors
      if( _callback ) (*_callback)( indri::api::IndexStatus::FileError, fileName, e.what(), _documentsIndexed, _documentsSeen );
    }
  }
}

//
// addString
//

lemur::api::DOCID_T indri::api::IndexEnvironment::addString( const std::string& documentString, const std::string& fileClass, const std::vector<indri::parse::MetadataPair>& metadata ) {
  indri::parse::UnparsedDocument document;
  indri::parse::Parser* parser;
  indri::parse::Tokenizer* tokenizer;
  indri::parse::DocumentIterator* iterator;
  indri::parse::Conflater* conflater;
  std::string nothing;

  _documentsSeen++;

  document.text = documentString.c_str();
  document.textLength = documentString.length() + 1; // for the null
  document.metadata = metadata;
  document.content = document.text;
  document.contentLength = document.textLength - 1;
  
  _getParsingContext( &parser, &tokenizer, &iterator, &conflater, fileClass );

  if( parser == 0 ) {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "File class '" + fileClass + "' wasn't recognized." );
  }
  indri::parse::TokenizedDocument* tokenized = tokenizer->tokenize( &document );

  ParsedDocument* parsed = parser->parse( tokenized );
  lemur::api::DOCID_T documentID =_repository.addDocument( parsed );

  _documentsIndexed++;
  if( _callback ) (*_callback)( indri::api::IndexStatus::DocumentCount, nothing, _error, _documentsIndexed, _documentsSeen );

  return documentID;
}

//
// For UIMA with offset annotations
//
lemur::api::DOCID_T indri::api::IndexEnvironment::addString( const std::string& documentString, const std::string&
                                             fileClass, const std::vector<indri::parse::MetadataPair>& metadata, const std::vector<indri::parse::TagExtent *> &tags ) {
  indri::parse::UnparsedDocument document;
  indri::parse::Parser* parser;
  indri::parse::Tokenizer* tokenizer;
  indri::parse::DocumentIterator* iterator;
  indri::parse::Conflater* conflater;
  indri::parse::OffsetAnnotationAnnotator *annote;
  std::string docno = "";
  
  for ( size_t i=0; i<metadata.size(); i++ ) {
    const char* attributeName = metadata[i].key;
    const char* attributeValue = (const char*) metadata[i].value;
    if ( ! strcmp( attributeName, "docno" ) ) docno = attributeValue;
  }
  
  std::string nothing;

  _documentsSeen++;

  document.text = documentString.c_str();
  document.textLength = documentString.length() + 1; // for the null
  document.metadata = metadata;
  document.content = document.text;
  document.contentLength = document.textLength - 1;
  
  _getParsingContext( &parser, &tokenizer, &iterator, &conflater, fileClass );
  annote = new indri::parse::OffsetAnnotationAnnotator(conflater);
  annote->setTags(docno.c_str(), tags);
  
  if( parser == 0 ) {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "File class '" + fileClass + "' wasn't recognized." );
  }
  indri::parse::TokenizedDocument* tokenized = tokenizer->tokenize( &document );

  ParsedDocument* parsed = parser->parse( tokenized );
  parsed = annote->transform(parsed);
  
  lemur::api::DOCID_T documentID =_repository.addDocument( parsed );

  _documentsIndexed++;
  if( _callback ) (*_callback)( indri::api::IndexStatus::DocumentCount, nothing, _error, _documentsIndexed, _documentsSeen );

  return documentID;
}


//
// addParsedDocument
//

lemur::api::DOCID_T indri::api::IndexEnvironment::addParsedDocument( ParsedDocument* document ) {
  std::string nothing;

  _documentsSeen++;
  lemur::api::DOCID_T documentID = _repository.addDocument( document );
  _documentsIndexed++;
  if( _callback ) (*_callback)( indri::api::IndexStatus::DocumentCount, nothing, _error, _documentsIndexed, _documentsSeen );
  
  return documentID;
}

//
// deleteDocument
//

void indri::api::IndexEnvironment::deleteDocument( lemur::api::DOCID_T documentID ) {
  _repository.deleteDocument( documentID );
}

//
// documentsIndexed
//

int indri::api::IndexEnvironment::documentsIndexed() {
  return _documentsIndexed;
}

//
// documentsSeen
//

int indri::api::IndexEnvironment::documentsSeen() {
  return _documentsSeen;
}

//
// compact
//

void indri::api::IndexEnvironment::compact() {
  _repository.compact();
}

//
// merge
//

void indri::api::IndexEnvironment::merge( const std::string& outputIndex, const std::vector<std::string>& inputIndexes ) {
  indri::collection::Repository::merge( outputIndex, inputIndexes );
}
