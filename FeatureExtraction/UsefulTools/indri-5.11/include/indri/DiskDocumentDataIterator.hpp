/*==========================================================================
 * Copyright (c) 2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

//
// DiskDocumentDataIterator
//
// 21 January 2005 -- tds
//

#ifndef INDRI_DISKDOCUMENTDATAITERATOR_HPP
#define INDRI_DISKDOCUMENTDATAITERATOR_HPP

#include "indri/File.hpp"
#include "indri/SequentialReadBuffer.hpp"
#include "indri/DocumentDataIterator.hpp"
#include "indri/DocumentData.hpp"

namespace indri {
  namespace index {
    class DiskDocumentDataIterator : public DocumentDataIterator {
    private:
      indri::file::SequentialReadBuffer* _readBuffer;
      UINT64 _fileSize;

      DocumentData _documentData;
      indri::file::File& _documentDataFile;
      bool _finished;

    public:
      DiskDocumentDataIterator( indri::file::File& documentDataFile );
      ~DiskDocumentDataIterator();

      void startIteration();
      bool nextEntry();
      const DocumentData* currentEntry();
      bool finished();
    };
  }
}

#endif // INDRI_DISKDOCUMENTDATAITERATOR_HPP

