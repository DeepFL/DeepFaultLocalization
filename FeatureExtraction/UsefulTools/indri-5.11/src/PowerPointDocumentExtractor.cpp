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
// PowerPointDocumentExtractor
//
// 14 June 2004 -- tds
//

#ifdef WIN32
// windows.h has to be included first
// in case someone comes along and
// defines WIN32_LEAN_AND_MEAN
#include <windows.h>


#include "indri/PowerPointDocumentExtractor.hpp"
#include "indri/Buffer.hpp"
#include "lemur/lemur-compat.hpp"
#include "lemur/Exception.hpp"

//
// PowerPoint.Application:
//    doc = Documents.Open( filename )
//    foreach PowerPoint.Slide slide in doc.Slides:
//      foreach PowerPoint.Shape shape in slide.Shapes:
//        text = shape.TextFrame.TextRange.Text;
//


indri::parse::PowerPointDocumentExtractor::PowerPointDocumentExtractor() {
  ::CoInitialize( NULL );

  HRESULT hr;
  VARIANT result;
  CLSID pptClsid;
  CLSIDFromProgID( L"PowerPoint.Application", &pptClsid );  

  hr = ::CoCreateInstance( pptClsid,
                           NULL,
                           CLSCTX_LOCAL_SERVER,
                           IID_IUnknown,
                           (void**) &_powerPointUnknown );

  if( FAILED(hr) ) {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Couldn't get IUnknown interface pointer to PowerPoint application." );
  }

  hr = _powerPointUnknown->QueryInterface( IID_IDispatch, (void**) &_powerPointDispatch );

  if( FAILED(hr) ) {
    _powerPointUnknown->Release();
    _powerPointUnknown = 0;
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Couldn't get Dispatch interface pointer to PowerPoint application." );
  }

  // Make PowerPoint invisible
  // By default PowerPoint is invisible, till you make it visible:
  VARIANT param;
  param.vt = VT_INT;
  param.intVal = -1;
  _officeHelper.com_property_put( &result, _powerPointDispatch, L"Visible", &param );

  _officeHelper.com_property_get( &result, _powerPointDispatch, L"Presentations" );                                          
  _presentationsDispatch = result.pdispVal;

  _documentWaiting = false;
}

indri::parse::PowerPointDocumentExtractor::~PowerPointDocumentExtractor() {
  _presentationsDispatch->Release();
  _powerPointDispatch->Release();
  _powerPointUnknown->Release();
}

void indri::parse::PowerPointDocumentExtractor::open( const std::string& filename ) {
  HRESULT hr;
  VARIANT result;

  _documentPath = filename;
  _documentBuffer.clear();


  *_documentBuffer.write(1) = 0;

  // copy the path into a form powerpoint will understand
  WCHAR widePathCopy[ MAX_PATH ];
  ::MultiByteToWideChar( CP_UTF8, 0, filename.c_str(), filename.length()+1, widePathCopy, MAX_PATH );

  // open the document in powerpoint
  DISPPARAMS parameters;
  VARIANTARG arguments[4];

  parameters.cArgs = 4;
  parameters.cNamedArgs = 0;
  parameters.rgdispidNamedArgs = NULL;
  parameters.rgvarg = arguments;

  arguments[3].vt = VT_BSTR;
  arguments[3].bstrVal = ::SysAllocString( widePathCopy ); // filename

  arguments[2].vt = VT_I4;
  arguments[2].lVal = -1; // readonly

  arguments[1].vt = VT_I4;
  arguments[1].lVal = 0; // untitled

  arguments[0].vt = VT_I4;
  arguments[0].lVal = -1; // withwindow

  _officeHelper.com_method_execute( &result, _presentationsDispatch, L"Open", &parameters );
  IDispatch* documentDispatch = result.pdispVal;

  _unparsedDocument.metadata.clear();
  _officeHelper.SetOfficeMetaData(documentDispatch,&_unparsedDocument);
  _officeHelper.writeHeaderToBuffer(_documentBuffer);

  // get rid of the filename parameter
  ::SysFreeString( arguments[3].bstrVal );

  // get the Slides dispatch
  _officeHelper.com_property_get( &result, documentDispatch, L"Slides" );
  IDispatch* slidesDispatch = result.pdispVal;

  // get the count of slides
  _officeHelper.com_property_get( &result, slidesDispatch, L"Count" );
  long slidesCount = result.lVal;
  VARIANT index;
  memset( &index, 0, sizeof index );
  index.vt = VT_I2;
  
  for( int i=1; i<=slidesCount; i++ ) {
    index.lVal = i;

    // fetch the i^th slide
    _officeHelper.com_method_execute( &result, slidesDispatch, L"Item", &index );
    IDispatch* slideDispatch = result.pdispVal;

    // fetch the shapes
    _officeHelper.com_property_get( &result, slideDispatch, L"Shapes" );
    IDispatch* shapesDispatch = result.pdispVal;

    // fetch shapes count
    _officeHelper.com_property_get( &result, shapesDispatch, L"Count" );
    long shapesCount = result.lVal;

    for( int j=1; j<=shapesCount; j++ ) {
      index.lVal = j;

      // fetch the j^th shape
      _officeHelper.com_method_execute( &result, shapesDispatch, L"Item", &index );
      IDispatch* shapeDispatch = result.pdispVal;

      // fetch the textframe
      _officeHelper.com_property_get( &result, shapeDispatch, L"TextFrame" );
      IDispatch* textFrameDispatch = result.pdispVal;

      // fetch the textrange
      IDispatch* textRangeDispatch = 0;
      BSTR textResult = 0;

      try {
        _officeHelper.com_property_get( &result, textFrameDispatch, L"TextRange" );
        IDispatch* textRangeDispatch = result.pdispVal;

        // fetch the text itself
        _officeHelper.com_property_get( &result, textRangeDispatch, L"Text" );
        BSTR textResult = result.bstrVal;

        _officeHelper.copy_bstr_to_buffer( _documentBuffer, textResult );
      } catch( lemur::api::Exception& e ) {
        if( textResult )
          ::SysFreeString( textResult );
        if( textRangeDispatch )
          textRangeDispatch->Release();
      }

      // clean up
      textFrameDispatch->Release();
      shapeDispatch->Release();
    }

    // clean up
    shapesDispatch->Release();
    slideDispatch->Release();
  }

  // clean up: release the slides
  slidesDispatch->Release();

  // add metadata to identify this file
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
  pair.value = (void*) "MSPPT";
  pair.valueLength = 6;
  _unparsedDocument.metadata.push_back( pair );



  _unparsedDocument.text = _documentBuffer.front();
  _unparsedDocument.textLength = _documentBuffer.position();
  _unparsedDocument.content = _documentBuffer.front();
  _unparsedDocument.contentLength = _documentBuffer.position() - 1;

  // close the document
  _officeHelper.com_method_execute( &result, documentDispatch, L"Close" );
  documentDispatch->Release();
  _documentWaiting = true;
}

indri::parse::UnparsedDocument* indri::parse::PowerPointDocumentExtractor::nextDocument() {
  if( _documentWaiting ) {
    _documentWaiting = false;
    return &_unparsedDocument;
  }
  return 0;
}

void indri::parse::PowerPointDocumentExtractor::close() {
}
#endif
