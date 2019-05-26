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
// DiskFrequentVocabularyIterator
//
// 19 January 2005 -- tds
//

#ifndef INDRI_DISKFREQUENTVOCABULARYITERATOR_HPP
#define INDRI_DISKFREQUENTVOCABULARYITERATOR_HPP

#include "indri/File.hpp"
#include "indri/Buffer.hpp"
#include "indri/TermData.hpp"
#include "indri/VocabularyIterator.hpp"
#include "indri/RVLDecompressStream.hpp"

namespace indri {
  namespace index {
    class DiskFrequentVocabularyIterator : public VocabularyIterator {
    private:
      indri::file::File& _file;
      indri::utility::RVLDecompressStream _stream;
      indri::utility::Buffer _buffer;
      const char* _current;
      int _fieldCount;
      bool _finished;

      DiskTermData* _data;
      char *_dataBuffer;

      // this tells us if the last nextEntry() came from 
      // a start iteration or not - needed for nextEntry(const char*)
      // call
      bool _justStartedIteration;

    public:
      DiskFrequentVocabularyIterator( indri::file::File& frequentTermsData, int fieldCount ); 
      ~DiskFrequentVocabularyIterator() { delete[] _dataBuffer; };
      
      void startIteration();
      bool finished();
      bool nextEntry();
      bool nextEntry(const char *skipTo);
      DiskTermData* currentEntry();
    };
  } 
}

#endif // INDRI_DISKFREQUENTVOCABULARYITERATOR_HPP

