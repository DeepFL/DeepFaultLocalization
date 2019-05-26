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
// PowerPointDocumentExtractor
//
// 14 June 2004 -- tds
//

#ifndef INDRI_POWERPOINTDOCUMENTEXTRACTOR_HPP
#define INDRI_POWERPOINTDOCUMENTEXTRACTOR_HPP
#ifdef WIN32
#include <string>
#include "indri/Buffer.hpp"
#include "indri/UnparsedDocument.hpp"
#undef WIN32_LEAN_AND_MEAN
#undef NOGDI
#include <windows.h>
#include <unknwn.h>
#include <oaidl.h>
#include "indri/DocumentIterator.hpp"
#include "indri/OfficeHelper.hpp"

namespace indri
{
  namespace parse
  {
    
    class PowerPointDocumentExtractor : public DocumentIterator {
    private:
      IUnknown* _powerPointUnknown;
      IDispatch* _powerPointDispatch;
      IDispatch* _presentationsDispatch;
      UnparsedDocument _unparsedDocument;

      std::string _documentPath;
      indri::utility::Buffer _documentBuffer;
      bool _documentWaiting;
      OfficeHelper _officeHelper;

    public:
      PowerPointDocumentExtractor();
      ~PowerPointDocumentExtractor();

      void open( const std::string& filename );
      UnparsedDocument* nextDocument();
      void close();
    };
  }
}

#endif // WIN32
#endif // INDRI_POWERPOINTDOCUMENTEXTRACTOR_HPP
