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
// DiskDocExtentListIterator
//
// 13 December 2004 -- tds
//

#ifndef INDRI_DISKDOCEXTENTLISTITERATOR_HPP
#define INDRI_DISKDOCEXTENTLISTITERATOR_HPP

#include "indri/SequentialReadBuffer.hpp"
#include "indri/DocExtentListIterator.hpp"

namespace indri {
  namespace index {
    class DiskDocExtentListIterator : public DocExtentListIterator {
    private:
      const char* _list;
      const char* _listEnd;
      int _skipDocument;

      indri::file::SequentialReadBuffer* _file;
      UINT64 _startOffset;
      bool _numeric;
      bool _ordinal;
      bool _parental;

      DocumentExtentData _data;
      bool _finished;

      void _readEntry();
      void _readSkip();

    public:
      DiskDocExtentListIterator( indri::file::SequentialReadBuffer* buffer, UINT64 startOffset );
      ~DiskDocExtentListIterator();
      
      void setStartOffset( UINT64 startOffset );

      bool finished() const;
      void startIteration();
      bool nextEntry();
      bool nextEntry( lemur::api::DOCID_T documentID );
      DocumentExtentData* currentEntry();
    };
  }
}

#endif // INDRI_DISKDOCEXTENTLISTITERATOR_HPP

