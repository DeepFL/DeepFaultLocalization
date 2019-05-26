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
// DiskDocListFileIterator
//
// 13 December 2004 -- tds
//

#ifndef INDRI_DISKDOCLISTFILEITERATOR_HPP
#define INDRI_DISKDOCLISTFILEITERATOR_HPP

#include "indri/DocListFileIterator.hpp"
#include "indri/DiskDocListIterator.hpp"
#include "indri/Buffer.hpp"
#include "lemur/Keyfile.hpp"
#include "indri/SequentialReadBuffer.hpp"
#include "indri/File.hpp"

namespace indri {
  namespace index {
    class DiskDocListFileIterator : public DocListFileIterator {
    private:
      indri::file::SequentialReadBuffer* _file;
      UINT64 _fileLength;

      int _fieldCount;
      indri::utility::Buffer _header;

      char _term[ lemur::file::Keyfile::MAX_KEY_LENGTH+1 ];
      TermData* _termData;
      DiskDocListIterator _iterator;
      DocListData _docListData;
      bool _finished;

      void _readEntry();

    public:
      DiskDocListFileIterator( indri::file::File& docListFile, int fieldCount );
      ~DiskDocListFileIterator();
      
      bool finished() const;
      void startIteration();

      bool nextEntry();
      DocListData* currentEntry();
      const DocListData* currentEntry() const;
    };
  }
}

#endif // INDRI_DISKDOCLISTFILEITERATOR_HPP

