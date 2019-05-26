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

#ifndef INDRI_OFFICEHELPER_HPP
#define INDRI_OFFICEHELPER_HPP


#ifdef WIN32
#include "lemur/lemur-compat.hpp"
#include "indri/Buffer.hpp"
#include "indri/UnparsedDocument.hpp"
#include "lemur/Exception.hpp"
#include <string>
#include "indri/DocumentIterator.hpp"


#include <ole2.h>

#define OFFICE_AUTHOR ("Author")//<field>author</field>	
#define OFFICE_TITLE ("Title")//<field>title</field>	
#define OFFICE_SUBJECT ("Subject")//<field>subject</field>	
#define OFFICE_LASTAUTHOR ("Last author")//<field>lastauthor</field>	
#define OFFICE_COMPANY ("Company")//<field>company</field>

namespace indri
{
  namespace parse
  {

    class OfficeHelper{
    private:
      static const char* _office_properties[];
      char _keyNames[10][100];
      //buildInDocumentproperties
      std::string _author;
      std::string _subject;
      std::string _lastAuthor;
      std::string _company;
      std::string _title;
      int _length_office_properties;


      std::string GetPropertyValue(IDispatch *pDoc, OLECHAR *pItem);
      std::string* GetOfficeMemberVariable(const char* pOfficeProperty);
      void setMetadataPair(UnparsedDocument* unparsedDocument,int propertyNr);
      void setMetadataPair(UnparsedDocument* unparsedDocument,const char* pOfficeProperty);
    public:
      OfficeHelper();
      ~OfficeHelper();
      HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...);		  

      //get the MetaData from the OfficeDocument and Set them in the IndriMetaData
      void SetOfficeMetaData(IDispatch* iDocumentDispatch, UnparsedDocument* unparsedDocument);
      void SetOfficeMetaData(const char *officeType, BSTR *bStr, UnparsedDocument *unparsedDocument);
      //com helpers
      void com_property_get( VARIANT* result, IDispatch* dispatch, LPOLESTR name ) ;
      void com_property_put( VARIANT* result, IDispatch* dispatch, LPOLESTR name, VARIANT* parameter );
      void com_method_execute( VARIANT* result, IDispatch* dispatch, LPOLESTR name, VARIANT* parameter = 0 );
      void com_method_execute( VARIANT* result, IDispatch* dispatch, LPOLESTR name, DISPPARAMS* parameters );
      DISPID com_get_dispatch_id( IDispatch* dispatch, LPOLESTR name );
      void copy_bstr_to_buffer( indri::utility::Buffer& docBuffer, BSTR bstr );
      void copy_bstr_to_string(std::string& pString, BSTR bstr, bool append=false);

      std::string GetAuthor();
      std::string GetSubject();
      std::string GetLastAuthor();
      std::string GetCompany();
      std::string GetTitle();
      void writeHeaderToBuffer(indri::utility::Buffer& docBuffer);
    };
  }
}
#endif
#endif // INDRI_OFFICEHELPER_HPP
