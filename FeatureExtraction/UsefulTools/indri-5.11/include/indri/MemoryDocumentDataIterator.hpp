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
// MemoryDocumentDataIterator
//
// 21 January 2005 -- tds
//

#ifndef INDRI_MEMORYDOCUMENTDATAITERATOR_HPP
#define INDRI_MEMORYDOCUMENTDATAITERATOR_HPP

#include "indri/DocumentDataIterator.hpp"
#include <vector>
#include "indri/DocumentData.hpp"

namespace indri {
  namespace index {
    class MemoryDocumentDataIterator : public DocumentDataIterator {
    private:
      const std::vector<DocumentData>& _dataVector;
      std::vector<DocumentData>::const_iterator _iterator;

    public:
      MemoryDocumentDataIterator( const std::vector<DocumentData>& documentData );
      ~MemoryDocumentDataIterator();

      void startIteration();
      bool nextEntry();
      const DocumentData* currentEntry();
      bool finished();
    };
  }
}

#endif // INDRI_DOCUMENTDATAITERATOR_HPP

