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
// DiskIndex
//
// 8 December 2004 -- tds
//

#ifndef INDRI_DISKINDEX_HPP
#define INDRI_DISKINDEX_HPP

#include "indri/Index.hpp"
#include "indri/File.hpp"
#include "lemur/Keyfile.hpp"
#include "indri/TermData.hpp"
#include "indri/FieldStatistics.hpp"
#include "indri/CorpusStatistics.hpp"
#include "indri/DiskTermData.hpp"
#include <vector>
#include <string>
#include "indri/BulkTree.hpp"
#include "indri/SequentialReadBuffer.hpp"

namespace indri {
  namespace index {
    class DiskIndex : public Index {
    private:
      indri::thread::Mutex _lock;

      std::string _path;

      indri::file::BulkTreeReader _frequentStringToTerm;
      indri::file::BulkTreeReader  _infrequentStringToTerm;

      indri::file::BulkTreeReader _frequentIdToTerm;
      indri::file::BulkTreeReader _infrequentIdToTerm;

      indri::file::File _frequentTermsData;

      indri::file::File _documentLengths;
      indri::file::File _documentStatistics;

      indri::file::File _invertedFile;
      indri::file::File _directFile;
      indri::file::File _fieldsFile;

      indri::file::SequentialReadBuffer _lengthsBuffer;

      std::vector<FieldStatistics> _fieldData;
      lemur::api::DOCID_T  _documentBase;
      int _infrequentTermBase;

      indri::index::DiskTermData* _fetchTermData( lemur::api::TERMID_T termID );
      indri::index::DiskTermData* _fetchTermData( const char* termString );

      CorpusStatistics _corpusStatistics;
      void _readManifest( const std::string& manifestPath );

    public:
      DiskIndex() : _lengthsBuffer(_documentLengths) {}

      void open( const std::string& base, const std::string& relative );
      void close();

      const std::string& path();
      lemur::api::DOCID_T documentBase();

      int field( const char* fieldName );
      int field( const std::string& fieldName );
      std::string field( int fieldID );

      lemur::api::TERMID_T term( const char* term );
      lemur::api::TERMID_T term( const std::string& term );
      std::string term( lemur::api::TERMID_T termID );

      int documentLength( lemur::api::DOCID_T documentID );
      UINT64 documentCount();
      UINT64 documentCount( const std::string& term );
      lemur::api::DOCID_T documentMaximum();
      UINT64 uniqueTermCount();

      UINT64 termCount( const std::string& term );
      UINT64 termCount();

      UINT64 fieldTermCount( const std::string& field );
      UINT64 fieldTermCount( const std::string& field, const std::string& term );

      UINT64 fieldDocumentCount( const std::string& field );
      UINT64 fieldDocumentCount( const std::string& field, const std::string& term );

      //
      // Lists
      //
      
      DocListIterator* docListIterator( lemur::api::TERMID_T termID );
      DocListIterator* docListIterator( const std::string& term );
      DocListFileIterator* docListFileIterator();
      DocExtentListIterator* fieldListIterator( int fieldID );
      DocExtentListIterator* fieldListIterator( const std::string& field );
      const TermList* termList( lemur::api::DOCID_T documentID );
      TermListFileIterator* termListFileIterator();

      VocabularyIterator* vocabularyIterator();
      VocabularyIterator* frequentVocabularyIterator();
      VocabularyIterator* infrequentVocabularyIterator();

      DocumentDataIterator* documentDataIterator();

      indri::thread::Lockable* iteratorLock();
      indri::thread::Lockable* statisticsLock();
      // cache limit
      enum {
        /// Maximum size of the cache for the document lengths file.
        // 250,000 documents/megabyte.
        MAX_DOCLENGTHS_CACHE = 20*1024*1024
      };
    };
  }
}

#endif // INDRI_DISKINDEX_HPP
