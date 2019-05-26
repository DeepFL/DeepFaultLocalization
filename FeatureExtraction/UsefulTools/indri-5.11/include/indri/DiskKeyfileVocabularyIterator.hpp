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
// DiskKeyfileVocabularyIterator
//
// 20 January 2005 -- tds
//

#ifndef INDRI_DISKKEYFILEVOCABULARYITERATOR_HPP
#define INDRI_DISKKEYFILEVOCABULARYITERATOR_HPP

#include "indri/DiskTermData.hpp"
#include "lemur/Keyfile.hpp"
#include "indri/VocabularyIterator.hpp"
#include "indri/Mutex.hpp"
#include "indri/BulkTree.hpp"

namespace indri {
  namespace index {
    class DiskKeyfileVocabularyIterator : public VocabularyIterator {
    private:
      DiskTermData* _diskTermData;
      int _baseID;
      indri::file::BulkTreeReader& _bulkTree;
      indri::file::BulkTreeIterator* _bulkIterator;
      int _fieldCount;

      char _termString[1024];

      indri::utility::Buffer _compressedData;
      indri::utility::Buffer _decompressedData;

      indri::thread::Mutex& _mutex;
      bool _holdingLock;
      bool _finished;

      void _acquire();
      void _release();
      bool _readData();

      // this tells us if the last nextEntry() came from 
      // a start iteration or not - needed for nextEntry(const char*)
      // call
      bool _justStartedIteration;

    public:
      DiskKeyfileVocabularyIterator( int baseID, indri::file::BulkTreeReader& bulkTree, indri::thread::Mutex& mutex, int fieldCount );
      ~DiskKeyfileVocabularyIterator();
      
      void startIteration();
      bool nextEntry();
      bool nextEntry(const char *skipTo);
      indri::index::DiskTermData* currentEntry();
      bool finished();
    };
  }
}

#endif // INDRI_DISKKEYFILEVOCABULARYITERATOR_HPP

