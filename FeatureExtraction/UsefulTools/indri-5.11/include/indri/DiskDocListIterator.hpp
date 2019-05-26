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
// DiskDocListIterator
//
// 10 December 2004 -- tds
//

#ifndef INDRI_DISKDOCLISTITERATOR_HPP
#define INDRI_DISKDOCLISTITERATOR_HPP

#include "indri/DocListIterator.hpp"
#include "indri/SequentialReadBuffer.hpp"
#include "lemur/Keyfile.hpp"

namespace indri { 
  namespace index {
    class DiskDocListIterator : public DocListIterator {
    private:
      const char* _list;
      const char* _listEnd;
      lemur::api::DOCID_T _skipDocument;

      indri::file::SequentialReadBuffer* _file;
      UINT64 _startOffset;
      UINT64 _endOffset;
      bool _hasTopdocs;
      bool _isFrequent;

      indri::utility::greedy_vector<TopDocument> _topdocs;
      DocumentData _data;
      DocumentData* _result;
      
      TermData* _termData;
      bool _ownTermData;
      char _term[ lemur::file::Keyfile::MAX_KEY_LENGTH+1 ];
      int _fieldCount;

      void _readEntry();
      void _readSkip();
      void _readTopdocs();
      void _readTermData( int headerLength );

    public:
      DiskDocListIterator( indri::file::SequentialReadBuffer* buffer, UINT64 startOffset, int fieldCount );
      ~DiskDocListIterator();
      void setStartOffset( UINT64 startOffset, TermData* termData );

      const indri::utility::greedy_vector<TopDocument>& topDocuments();

      void startIteration();
      bool nextEntry();
      bool nextEntry( lemur::api::DOCID_T documentID );
      DocumentData* currentEntry();
      bool finished();
      bool isFrequent() const;
      TermData* termData();
    };
  }
}

#endif // INDRI_DISKDOCLISTITERATOR_HPP

