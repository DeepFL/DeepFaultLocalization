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

#ifndef INDRI_PDFDOCUMENTEXTRACTOR_HPP
#define INDRI_PDFDOCUMENTEXTRACTOR_HPP

#include "lemur/lemur-compat.hpp"
#include "indri/Buffer.hpp"
#include "indri/UnparsedDocument.hpp"
#include "indri/DocumentIterator.hpp"
#include "indri/XMLReader.hpp"
#include "indri/XMLNode.hpp"
#include "indri/XMLWriter.hpp"
#include <string>
namespace indri
{
  namespace parse
  {
    
    class PDFDocumentExtractor : public DocumentIterator {
      indri::utility::Buffer _documentTextBuffer;
      UnparsedDocument _unparsedDocument;
      std::string _documentPath;
  
    public:
      PDFDocumentExtractor();
      ~PDFDocumentExtractor();

      void open( const std::string& filename );
      UnparsedDocument* nextDocument();
	  void appendPdfMetaData(indri::xml::XMLNode* node);
	  void seekValue(indri::xml::XMLNode* node, std::string &metaTag);
      void close();
	private:
	  std::string _title;
	  std::string _author;

    };
  }
}

#endif // INDRI_PDFDOCUMENTEXTRACTOR_HPP
