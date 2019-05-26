/*==========================================================================
 * Copyright (c) 2002-2010 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/
/* Author: Allard Dijk
 */


#ifdef WIN32
#include "indri/OfficeHelper.hpp"

const char* indri::parse::OfficeHelper::_office_properties[] = { OFFICE_AUTHOR, OFFICE_TITLE, OFFICE_SUBJECT, OFFICE_LASTAUTHOR, OFFICE_COMPANY };

indri::parse::OfficeHelper::OfficeHelper() {
  _length_office_properties = sizeof(_office_properties)/sizeof(char*);
  for(int propertyNr=0;propertyNr<_length_office_properties;propertyNr++){
    int length = strlen(_office_properties[propertyNr]);
    int propertyLength = strlen(_office_properties[propertyNr]);
    int keyLength=0;
    for(int charNr=0; charNr<propertyLength; charNr++){//foreach char in keyName
      if(_office_properties[propertyNr][charNr]!=' '){
        //skip space and convert to lowercase
        _keyNames[propertyNr][keyLength] = tolower(_office_properties[propertyNr][charNr]);
        keyLength++;		  
      }		      
    }
    _keyNames[propertyNr][keyLength]='\0';
  }


}

indri::parse::OfficeHelper::~OfficeHelper() {
}

std::string* indri::parse::OfficeHelper::GetOfficeMemberVariable(const char* pOfficeProperty){
  if(!strcmp(pOfficeProperty,OFFICE_AUTHOR))
    {
      return &_author;
    }
  if(!strcmp(pOfficeProperty,OFFICE_TITLE))
    {
      return &_title;
    }
  if(!strcmp(pOfficeProperty,OFFICE_SUBJECT))
    {
      return &_subject;
    }
  if(!strcmp(pOfficeProperty,OFFICE_LASTAUTHOR))
    {
      return &_lastAuthor;
    }
  if(!strcmp(pOfficeProperty,OFFICE_COMPANY))
    {
      return &_company;
    }	
	
  LEMUR_THROW( LEMUR_RUNTIME_ERROR, "GetOfficeMemberVariable() returns NULL" );

}

void indri::parse::OfficeHelper::setMetadataPair(UnparsedDocument* unparsedDocument,const char* pOfficeProperty){
  for(int propertyNr=0;propertyNr<_length_office_properties;propertyNr++){
    if(!strcmp(pOfficeProperty, _office_properties[propertyNr]))
      setMetadataPair(unparsedDocument, propertyNr);
  }
}

void indri::parse::OfficeHelper::setMetadataPair(UnparsedDocument* unparsedDocument,int propertyNr){
  indri::parse::MetadataPair pair;

  std::string *officeMemberVariable = GetOfficeMemberVariable( _office_properties[propertyNr] );
  if(officeMemberVariable->length()>0){
    pair.key = _keyNames[propertyNr];
    pair.value = officeMemberVariable->c_str();
    pair.valueLength = officeMemberVariable->length()+1;
    unparsedDocument->metadata.push_back( pair );
  }
}

void indri::parse::OfficeHelper::SetOfficeMetaData(const char *officeType, BSTR *bStr, UnparsedDocument *unparsedDocument){
  std::string *officeMemberVariable = GetOfficeMemberVariable( officeType );
  copy_bstr_to_string(*officeMemberVariable,*bStr);

  for(int propertyNr=0;propertyNr<_length_office_properties;propertyNr++){
    if(!strcmp(officeType,_office_properties[propertyNr])){
      //find corresponding propertyNr
      setMetadataPair(unparsedDocument,propertyNr );
    }
  }


	

}

void indri::parse::OfficeHelper::SetOfficeMetaData(IDispatch *iDocumentDispatch, UnparsedDocument *unparsedDocument){

  //get some build in document properties

  for(int propertyNr=0;propertyNr<_length_office_properties;propertyNr++){
    std::string *officeMemberVariable = GetOfficeMemberVariable( _office_properties[propertyNr] );
    WCHAR officeProperty[ MAX_PATH ];
    ::MultiByteToWideChar( CP_UTF8, 0, _office_properties[propertyNr] , strlen(_office_properties[propertyNr])+1, officeProperty, MAX_PATH );
    *officeMemberVariable = GetPropertyValue( iDocumentDispatch, officeProperty );
    setMetadataPair(unparsedDocument,propertyNr );
  }

 
}


// 
// AutoWrap() - Automation helper function...
// 
HRESULT indri::parse::OfficeHelper::AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...) 
{
  // Begin variable-argument list...
  va_list marker;
  va_start(marker, cArgs);

  if(!pDisp) {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "NULL IDispatch passed to AutoWrap()." );
  }

  // Variables used...
  DISPPARAMS dp = { NULL, NULL, 0, 0 };
  DISPID dispidNamed = DISPID_PROPERTYPUT;
  DISPID dispID;
  HRESULT hr;
  char buf[200];
  char szName[200];
   
  // Convert down to ANSI
  WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL);
   
  // Get DISPID for name passed...
  hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, 
                            &dispID);
  if(FAILED(hr)) {
    sprintf(buf, 
            "IDispatch::GetIDsOfNames(\"%s\") failed w/err0x%08lx",
            szName, hr);
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "AutoWrap() " + buf + "." );
    return hr;
  }
   
  // Allocate memory for arguments...
  VARIANT *pArgs = new VARIANT[cArgs+1];

  // Extract arguments...
  for(int i=0; i<cArgs; i++) {
    pArgs[i] = va_arg(marker, VARIANT);
  }
   
  // Build DISPPARAMS
  dp.cArgs = cArgs;
  dp.rgvarg = pArgs;
   
  // Handle special-case for property-puts!
  if(autoType & DISPATCH_PROPERTYPUT) {
    dp.cNamedArgs = 1;
    dp.rgdispidNamedArgs = &dispidNamed;
  }
   
  // Make the call!
  hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, 
                     &dp, pvResult, NULL, NULL);
  if(FAILED(hr)) {
    sprintf(buf,
            "IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx", 
            szName, dispID, hr);
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "AutoWrap() " + buf + "." );
    return hr;
  }
  // End variable-argument section...
  va_end(marker);
   
  delete [] pArgs;
   
  return hr;

}



std::string indri::parse::OfficeHelper::GetPropertyValue(IDispatch *pDoc, OLECHAR *pItem){
  // Get BuiltinDocumentProperties collection
  HRESULT hr;

  IDispatch *pProps;
  {
    VARIANT result;
    VariantInit(&result);
    hr=AutoWrap(DISPATCH_PROPERTYGET, &result, pDoc, 
                L"BuiltinDocumentProperties", 0);
    if( FAILED(hr) ) {
      LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Failed to get BuiltinDocumentProperties" );
    }
    pProps = result.pdispVal;
  }    
	
  // Get "Subject" from BuiltInDocumentProperties.Item("Subject")
  IDispatch *pPropSubject;
  {
    VARIANT result;
    VariantInit(&result);
    VARIANT x;
    x.vt = VT_BSTR;
    x.bstrVal = ::SysAllocString(pItem);
    hr=AutoWrap(DISPATCH_PROPERTYGET, &result, pProps, L"Item", 1, x);
    if( FAILED(hr) ) {
      LEMUR_THROW( LEMUR_RUNTIME_ERROR, "GetPropertyValue() Failed to get Item" );
    }
    pPropSubject = result.pdispVal;
    SysFreeString(x.bstrVal);
  }

  // Get the Value of the Subject property and display it

  VARIANT result;
  VariantInit(&result);
  AutoWrap(DISPATCH_PROPERTYGET, &result, pPropSubject, L"Value", 0);
  if( FAILED(hr) ) {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "GetPropertyValue() Failed to get Value" );
  }

		 


  // get the document text as a string
  std::string propertyValuereturn;
  copy_bstr_to_string(propertyValuereturn, result.bstrVal);

  return propertyValuereturn;


}

//
// com_property_get
//

void indri::parse::OfficeHelper::com_property_get( VARIANT* result, IDispatch* dispatch, LPOLESTR name ) {
  DISPID dispatchID = com_get_dispatch_id( dispatch, name );
  DISPPARAMS nullParameters = { NULL, NULL, 0, 0 };

  HRESULT hr;
  hr = dispatch->Invoke( dispatchID,
                         IID_NULL,
                         LOCALE_SYSTEM_DEFAULT,
                         DISPATCH_PROPERTYGET,
                         &nullParameters,
                         result,
                         NULL,
                         NULL );

  if( FAILED(hr) ) {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Failed to get the property." );
  }
}


void indri::parse::OfficeHelper::copy_bstr_to_string(std::string& pString, BSTR bstr, bool append) {
  if(bstr==NULL){
    pString="";
    return;
  }


  UINT textLength = ((UINT*)bstr)[-1];
  if(textLength>0){
    // multibyte could potentially have 3 bytes for every 1 double-byte char
    UINT convertedLength = 3*textLength;

    char *propertyValue = new char[convertedLength];
    // convert the document text into a multibyte representation
    int trueLength = ::WideCharToMultiByte( CP_UTF8, 0, bstr, -1,
                                            propertyValue, convertedLength,
                                            NULL, NULL );
    propertyValue[trueLength]='\0';
    if(append)
      pString.append(propertyValue);
    else
      pString = propertyValue;
    delete[] propertyValue;
  }
  else{
    pString="";
  }

}


//
// copy_bstr_to_buffer
//
void indri::parse::OfficeHelper::copy_bstr_to_buffer( indri::utility::Buffer& docBuffer, BSTR bstr ) {
  UINT textLength = ((UINT*)bstr)[-1];
  
  if( !textLength )
    return;

  // get rid of any trailing nulls
  if( docBuffer.position() > 0 ) {
    docBuffer.unwrite(1);
    *docBuffer.write(1) = ' ';
  }
 
  // multibyte could potentially have 3 bytes for every 1 double-byte char
  UINT convertedLength = 3*(textLength+1);

  // convert the document text into a multibyte representation
  int trueLength = ::WideCharToMultiByte( CP_UTF8, 0, bstr, -1,
                                          docBuffer.write( convertedLength ), convertedLength,
                                          NULL, NULL );

  docBuffer.unwrite( convertedLength-trueLength );
}

//
// com_get_dispatch_id
//

DISPID indri::parse::OfficeHelper::com_get_dispatch_id( IDispatch* dispatch, LPOLESTR name ) {
  DISPID dispatchID;
  HRESULT hr;

  hr = dispatch->GetIDsOfNames( IID_NULL,
                                &name,
                                1,
                                LOCALE_SYSTEM_DEFAULT,
                                &dispatchID );
  
  if( FAILED(hr) ) {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Failed to get the ID of method." );
  }

  return dispatchID;
}



//
// com_method_execute
//

void indri::parse::OfficeHelper::com_method_execute( VARIANT* result, IDispatch* dispatch, LPOLESTR name, DISPPARAMS* parameters ) {
  HRESULT hr;
  DISPID dispatchID = com_get_dispatch_id( dispatch, name );
  EXCEPINFO exceptionInfo;

  hr = dispatch->Invoke( dispatchID,
                         IID_NULL,
                         LOCALE_SYSTEM_DEFAULT,
                         DISPATCH_METHOD,
                         parameters,
                         result,
                         &exceptionInfo,
                         NULL );

  if( FAILED(hr) ) {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Failed to execute method." );
  }
}

//
// com_method_execute
//

void indri::parse::OfficeHelper::com_method_execute( VARIANT* result, IDispatch* dispatch, LPOLESTR name, VARIANT* parameter ) {
  DISPPARAMS oneParameter;

  oneParameter.cArgs = 1;
  oneParameter.cNamedArgs = 0;
  oneParameter.rgdispidNamedArgs = NULL;
  oneParameter.rgvarg = parameter;

  DISPPARAMS noParameters = { 0, 0, NULL, NULL };

  DISPPARAMS* parameters;

  if( parameter )
    parameters = &oneParameter;
  else
    parameters = &noParameters;

  com_method_execute( result, dispatch, name, parameters );
}

void indri::parse::OfficeHelper::com_property_put( VARIANT* result, IDispatch* dispatch, LPOLESTR name, VARIANT* parameter ) {
  HRESULT hr;
  DISPID dispatchID = com_get_dispatch_id( dispatch, name );
  EXCEPINFO exceptionInfo;
  unsigned int badArgument;

  DISPPARAMS parameters;
  DISPID putID = DISPID_PROPERTYPUT;

  parameters.cArgs = 1;
  parameters.cNamedArgs = 1;
  parameters.rgdispidNamedArgs = &putID;
  parameters.rgvarg = parameter;

  hr = dispatch->Invoke( dispatchID,
                         IID_NULL,
                         LOCALE_SYSTEM_DEFAULT,
                         DISPATCH_PROPERTYPUT,
                         &parameters,
                         NULL,
                         &exceptionInfo,
                         &badArgument );

  if( FAILED(hr) ) {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Failed to put property." );
  }
}

std::string indri::parse::OfficeHelper::GetAuthor(){
  return _author;
}

std::string indri::parse::OfficeHelper::GetSubject(){
  return _subject;
}

std::string indri::parse::OfficeHelper::GetLastAuthor(){
  return _lastAuthor;
}

std::string indri::parse::OfficeHelper::GetCompany(){
  return _company;
}

std::string indri::parse::OfficeHelper::GetTitle(){
  return _title;
}

void indri::parse::OfficeHelper::writeHeaderToBuffer(indri::utility::Buffer& docBuffer){
  
  std::string officeHeader="";
  if(_title.length()>0)
    officeHeader+="<title>"+_title+"</title>\n";
  if(_author.length()>0)
    officeHeader+="<author>"+_author+"</author>\n";
  if(_company.length()>0)
    officeHeader+="<company>"+_company+"</company>\n";
  if(_lastAuthor.length()>0)
    officeHeader+="<lastauthor>"+_lastAuthor+"</lastauthor>\n";
  if(_subject.length()>0)
    officeHeader+="<subject>"+_subject+"</subject>\n";

  if(officeHeader.length()>0 ){
    officeHeader="<head>\n" + officeHeader +  "</head>\n";
    char *pBuffer = docBuffer.write( officeHeader.length() + 1 );
    strcpy(pBuffer, officeHeader.c_str() );
  }
}

#endif //WIN32
