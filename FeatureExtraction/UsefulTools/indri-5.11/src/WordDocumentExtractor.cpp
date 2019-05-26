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
// OfficeDocumentExtractor
//
// 14 June 2004 -- tds
//
// Code is based in part on the AutoWord class
// by Poonam Bajaj.
//

#ifdef WIN32
// windows.h has to be included first
// in case someone comes along and
// defines WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef WIN32
#include "indri/WordDocumentExtractor.hpp"


// using this wde_internal structure for windows types
// so that we keep them out of the headers
// this makes cross platform builds a little easier

struct wde_internal {
  IUnknown* _wordUnknown;
  IDispatch* _wordDispatch;
  IDispatch* _documentsDispatch;
  DISPID _openDispatchID;
  DISPID _closeDispatchID;
};

#define v_internal ( (wde_internal*) _internal )

indri::parse::WordDocumentExtractor::WordDocumentExtractor() {
  _internal = new wde_internal;

  v_internal->_documentsDispatch = 0;
  v_internal->_wordUnknown = 0;
  v_internal->_wordDispatch = 0;

  initialize();
  _documentWaiting = false;
}

indri::parse::WordDocumentExtractor::~WordDocumentExtractor() {
  uninitialize();
}

void indri::parse::WordDocumentExtractor::uninitialize() {
  if( v_internal ) {
    // close application too?
    if( v_internal->_documentsDispatch )
      v_internal->_documentsDispatch->Release();
    //  if( v_internal->_wordDispatch )
    //  v_internal->_wordDispatch->Release();
    //if( v_internal->_wordUnknown )
    //      v_internal->_wordUnknown->Release();

    delete v_internal;
  }
}

void indri::parse::WordDocumentExtractor::initialize() {
  ::CoInitialize( NULL );

  HRESULT hr;
  CLSID wordClsid;
  CLSIDFromProgID( L"Word.Application", &wordClsid );  

  hr = ::CoCreateInstance( wordClsid,
                           NULL,
                           CLSCTX_LOCAL_SERVER,
                           IID_IUnknown,
                           (void**) &v_internal->_wordUnknown );

  if( FAILED(hr) ) {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Couldn't get Unknown interface pointer to Word application." );
  }

  hr = v_internal->_wordUnknown->QueryInterface( IID_IDispatch, (void**) &v_internal->_wordDispatch );

  if( FAILED(hr) ) {
    v_internal->_wordUnknown->Release();
    v_internal->_wordUnknown = 0;
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Couldn't get Dispatch interface pointer to Word application." );
  }

  LPOLESTR documentsPropertyName = L"Documents";
  DISPID documentsDispatchID;

  hr = v_internal->_wordDispatch->GetIDsOfNames( IID_NULL,
                                                 &documentsPropertyName,
                                                 1,
                                                 LOCALE_SYSTEM_DEFAULT,
                                                 &documentsDispatchID );

  DISPPARAMS parameters = { NULL, NULL, 0, 0 };
  VARIANT result;

  hr = v_internal->_wordDispatch->Invoke( documentsDispatchID,
                                          IID_NULL,
                                          LOCALE_SYSTEM_DEFAULT,
                                          DISPATCH_PROPERTYGET,
                                          &parameters,
                                          &result,
                                          NULL,
                                          NULL );
                                      
  v_internal->_documentsDispatch = result.pdispVal;

  LPOLESTR closeMethodName = L"Close";

  hr = v_internal->_documentsDispatch->GetIDsOfNames( IID_NULL,
                                                      &closeMethodName,
                                                      1,
                                                      LOCALE_SYSTEM_DEFAULT,
                                                      &v_internal->_closeDispatchID );
  LPOLESTR openMethodName = L"Open";

  hr = v_internal->_documentsDispatch->GetIDsOfNames( IID_NULL,
                                                      &openMethodName,
                                                      1,
                                                      LOCALE_SYSTEM_DEFAULT,
                                                      &v_internal->_openDispatchID );
}

void indri::parse::WordDocumentExtractor::open( const std::string& filename ) {
  // open the document
  initialize();
  HRESULT hr;
  VARIANT result;
  DISPPARAMS parameters;
  VARIANT fileNameParameter;
  int location = filename.find_last_of("\\/");
  if(filename[location+1]=='~'){
    //skip temp-Word files
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Skipping temp Word Files starting with ~, they cause trouble parsing. " + filename + "." );
  }

  _documentPath = filename;

  WCHAR widePathCopy[ MAX_PATH ];
  ::MultiByteToWideChar( CP_UTF8, 0, filename.c_str(), filename.length()+1, widePathCopy, MAX_PATH );

  fileNameParameter.vt = VT_BSTR;
  fileNameParameter.bstrVal = ::SysAllocString( widePathCopy );

  parameters.cArgs = 1;
  parameters.cNamedArgs = 0;
  parameters.rgdispidNamedArgs = NULL;
  parameters.rgvarg = &fileNameParameter; 

  hr = v_internal->_documentsDispatch->Invoke( v_internal->_openDispatchID,
                                               IID_NULL,
                                               LOCALE_SYSTEM_DEFAULT,
                                               DISPATCH_METHOD,
                                               &parameters,
                                               &result,
                                               NULL,
                                               NULL );
  
  if( FAILED(hr) ) {
    _officeHelper.AutoWrap(DISPATCH_METHOD, NULL,v_internal->_wordDispatch, L"Quit", 0);
    v_internal->_wordDispatch->Release();
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Couldn't open file " + filename + "." );
  }

  IDispatch* documentDispatch = result.pdispVal;



  // get content dispatch ID
  DISPID contentDispatchID;
  LPOLESTR contentPropertyName = L"Content";

  hr = documentDispatch->GetIDsOfNames( IID_NULL,
                                        &contentPropertyName,
                                        1,
                                        LOCALE_SYSTEM_DEFAULT,
                                        &contentDispatchID );

  if( FAILED(hr) ) {
    closeWord(documentDispatch,true);
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Couldn't get ID of \"Content\" for file " + filename + "." );
  }


  // fetch the content property
  DISPPARAMS noParameters = { NULL, NULL, 0, 0 };

  hr = documentDispatch->Invoke( contentDispatchID,
                                 IID_NULL,
                                 LOCALE_SYSTEM_DEFAULT,
                                 DISPATCH_PROPERTYGET,
                                 &noParameters,
                                 &result,
                                 NULL,
                                 NULL );

  if( FAILED(hr) ) {
    closeWord(documentDispatch,true);
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Couldn't get content for file " + filename + "." );
  }

  IDispatch* contentDispatch = result.pdispVal;

  // get the Text property dispatch ID
  DISPID textDispatchID;
  LPOLESTR textPropertyName = L"Text";

  hr = contentDispatch->GetIDsOfNames( IID_NULL,
                                       &textPropertyName,
                                       1,
                                       LOCALE_SYSTEM_DEFAULT,
                                       &textDispatchID );

  if( FAILED(hr) ) {
    closeWord(documentDispatch,true);
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Couldn't get text for file " + filename + "." );
  }

  // call the text method on the content
  hr = contentDispatch->Invoke( textDispatchID,
                                IID_NULL,
                                LOCALE_SYSTEM_DEFAULT,
                                DISPATCH_PROPERTYGET,
                                &noParameters,
                                &result,
                                NULL,
                                NULL );

  if( FAILED(hr) ) {
    closeWord(documentDispatch,true);
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Couldn't get text for file " + filename + "." );
  }

  // get the document text as a string
  BSTR textResult = result.bstrVal;
  UINT textLength = ((UINT*)textResult)[-1];

  // multibyte could potentially have 3 bytes for every 1 double-byte char
  UINT convertedLength = 3*textLength;
  _documentTextBuffer.clear();

  _unparsedDocument.metadata.clear();
  _officeHelper.SetOfficeMetaData(documentDispatch,&_unparsedDocument);
  _officeHelper.writeHeaderToBuffer(_documentTextBuffer);


  

  //add to buffer
 
  
  //wordHeader


  // convert the document text into a multibyte representation
  int trueLength = ::WideCharToMultiByte( CP_UTF8, 0, textResult, -1,
                                          _documentTextBuffer.write( convertedLength ), convertedLength,
                                          NULL, NULL );
  _documentTextBuffer.unwrite( convertedLength-trueLength );

  indri::parse::MetadataPair pair;



  pair.key = "docno";
  pair.value = _documentPath.c_str();
  pair.valueLength = _documentPath.length()+1;
  _unparsedDocument.metadata.push_back( pair );

  pair.key = "path";
  pair.value = _documentPath.c_str();
  pair.valueLength = _documentPath.length()+1;
  _unparsedDocument.metadata.push_back( pair );

  pair.key = "filetype";
  pair.value = (void*) "MSWORD";
  pair.valueLength = 7;
  _unparsedDocument.metadata.push_back( pair );




  _unparsedDocument.text = _documentTextBuffer.front();
  _unparsedDocument.textLength = _documentTextBuffer.position();
  _unparsedDocument.content = _documentTextBuffer.front();
  _unparsedDocument.contentLength = _documentTextBuffer.position() - 1;

  // do something with this string here
  ::SysFreeString( textResult );
  ::SysFreeString( fileNameParameter.bstrVal );

  contentDispatch->Release();
  
  closeWord(documentDispatch,true);

  documentDispatch->Release();
  v_internal->_wordDispatch->Release();
  v_internal->_wordUnknown->Release();
  _documentWaiting = true;
}

void indri::parse::WordDocumentExtractor::closeWord(IDispatch* documentDispatch, bool quit){
  // Close the document without saving changes
  VARIANT x;
  x.vt = VT_BOOL;
  x.boolVal = false;
  _officeHelper.AutoWrap(DISPATCH_METHOD, NULL, documentDispatch, L"Close", 1, x);
  if(quit)
    _officeHelper.AutoWrap(DISPATCH_METHOD, NULL,v_internal->_wordDispatch, L"Quit", 0);
}

indri::parse::UnparsedDocument* indri::parse::WordDocumentExtractor::nextDocument() {
  if( _documentWaiting ) {
    _documentWaiting = false;
    return &_unparsedDocument;
  }

  return 0;
}

void indri::parse::WordDocumentExtractor::close() {
  _documentWaiting = false;
}

#endif // WIN32
