/*==========================================================================
 * Copyright (c) 2003-2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/


//
// PDFDocumentExtractor
//
// 25 June 2004 -- tds
//

#include "indri/PDFDocumentExtractor.hpp"
#include "indri/Buffer.hpp"

#include "GString.h"
#include "TextOutputDev.h"
#include "PDFDoc.h"

#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Page.h"
#include "CharTypes.h"
#include "GlobalParams.h"
#include "lemur/Exception.hpp"

static void buffer_write( void* stream, char* text, int len ) {
  indri::utility::Buffer* buffer = (indri::utility::Buffer*) stream;

  if( buffer->position() ) {
    buffer->unwrite(1);
  }

  memcpy( buffer->write(len), text, len );
  if( text[len-1] != 0 )
    *buffer->write(1) = 0;
}

indri::parse::PDFDocumentExtractor::PDFDocumentExtractor() {
  globalParams = new GlobalParams(0);
  _title="";
  _author="";
}

indri::parse::PDFDocumentExtractor::~PDFDocumentExtractor() {
  delete globalParams;
  globalParams = 0;
}


void indri::parse::PDFDocumentExtractor::seekValue(indri::xml::XMLNode* node, std::string &metaTag) {
  if (node == NULL) {
    return;
  }

  const std::vector<indri::xml::XMLNode*>& children = node->getChildren();
  for( size_t i=0; i<children.size(); i++ ) {
    indri::xml::XMLNode* child = children[i];
    metaTag = child->getValue();
	if(metaTag.length()==0)
		seekValue(child,metaTag);
	else
		return;
  }

}

void indri::parse::PDFDocumentExtractor::appendPdfMetaData(indri::xml::XMLNode* node) {
  indri::xml::XMLNode* current = 0;

  if (node == NULL) {
    return;
  }

  const std::vector<indri::xml::XMLNode*>& children = node->getChildren();

  for( size_t i=0; i<children.size(); i++ ) {
    indri::xml::XMLNode* child = children[i];
    std::string name = child->getName();
	if(name=="dccreator")
	{
		seekValue(child,_author);
	}
	if(name=="dctitle")
	{
		seekValue(child,_title);
	}
	appendPdfMetaData(child);

  }



}

void indri::parse::PDFDocumentExtractor::open( const std::string& filename ) {
  _documentTextBuffer.clear();
  _documentPath = filename;
}

void indri::parse::PDFDocumentExtractor::close() {
  _documentPath = "";
}

indri::parse::UnparsedDocument* indri::parse::PDFDocumentExtractor::nextDocument() {
  if( !_documentPath.length() )
    return 0;

  PDFDoc* doc = 0;
  TextOutputDev* textOut = 0;
  GString* gfilename = new GString(_documentPath.c_str());
  doc = new PDFDoc( gfilename );
  // if the doc is not ok, or ok to copy, it
  // will be a document of length 0.
  if( doc->isOk() && doc->okToCopy() ) {
    void* stream = &_documentTextBuffer;
    textOut = new TextOutputDev( buffer_write, stream, gFalse, gFalse);
    if ( textOut->isOk() ) {
      int firstPage = 1;
      int lastPage = doc->getNumPages();
	  double hDPI=72.0;
	  double vDPI=72.0;
	  int rotate=0;
	  GBool useMediaBox=gFalse;
	  GBool crop=gTrue; 
	  GBool printing=gFalse; 
	  if(doc->readMetadata()!=NULL)
	  {
		  GString rawMetaData = doc->readMetadata();
		  GString preparedMetaData="";

		  //zoek <rdf:RDF  en eindig bij </rdf:RDF>!! 
		  for(int x=0; x<rawMetaData.getLength(); x++) {
			  if(rawMetaData.getChar(x)!='?' && rawMetaData.getChar(x)!=':') {
				  //skip characters which the XMLReader doesn't understand
				  preparedMetaData.append(rawMetaData.getChar(x));
			  }
		  }
		  std::string metaData(preparedMetaData.getCString());
		  int startbegin = metaData.find("<rdf");
		  int stopend = metaData.find(">", metaData.rfind("</rdf") );
		  metaData = metaData.substr(startbegin, (stopend-startbegin)+1 );
	  

     	  indri::xml::XMLReader reader;

		  try {
			  std::auto_ptr<indri::xml::XMLNode> result( reader.read( metaData.c_str() ) );
			  appendPdfMetaData( result.get() );
		  } catch( lemur::api::Exception& e ) {
			LEMUR_RETHROW( e, "Had trouble reading PDF metadata" );
		  } 
		  if( _author.length()>0 || _title.length()>0 )
		  {
			std::string createdPdfHeader;
			createdPdfHeader="<head>\n";
			if(_title.length()>0) {
				createdPdfHeader+="<title>";
				createdPdfHeader+=_title;
				createdPdfHeader+="</title>\n";
			}
			if(_author.length()>0) {
				createdPdfHeader+="<author>";
				createdPdfHeader+=_author;
				createdPdfHeader+="</author>\n";
			}
			createdPdfHeader+="</head>\n";
			char *metastream = _documentTextBuffer.write( createdPdfHeader.length()+1 );
			strcpy(metastream, createdPdfHeader.c_str());
		  }
	  }
      doc->displayPages(textOut, firstPage, lastPage, hDPI, vDPI, rotate, useMediaBox, crop, printing);
    }
  }
  

  delete textOut;
  delete doc;

  _unparsedDocument.textLength = _documentTextBuffer.position();
  _unparsedDocument.contentLength = _unparsedDocument.textLength ? _documentTextBuffer.position() - 1 : 0 ; // no null 0 if text is empty.
  char* docnoPoint = _documentTextBuffer.write( _documentPath.length()+1 );
  strcpy( docnoPoint, _documentPath.c_str() );
  _unparsedDocument.text = _documentTextBuffer.front();
  _unparsedDocument.content = _documentTextBuffer.front();
  _unparsedDocument.metadata.clear();

  indri::parse::MetadataPair pair;

  pair.key = "path";
  pair.value = docnoPoint;
  pair.valueLength = _documentPath.length()+1;
  _unparsedDocument.metadata.push_back( pair );

  _docnostring.assign(_documentPath.c_str() );
  cleanDocno();
  pair.value = _docnostring.c_str();
  pair.valueLength = _docnostring.length()+1;
  pair.key = "docno";
  _unparsedDocument.metadata.push_back( pair );
 
  _documentPath = "";

  return &_unparsedDocument;
}
