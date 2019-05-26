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
// Index
//
// 15 November 2004 -- tds
//

#ifndef INDRI_INDEX_HPP
#define INDRI_INDEX_HPP

#include <string>
#include <vector>

#include "indri/DocListIterator.hpp"
#include "indri/DocExtentListIterator.hpp"
#include "indri/DocListFileIterator.hpp"
#include "indri/FieldListIterator.hpp"
#include "indri/VocabularyIterator.hpp"
#include "indri/TermList.hpp"
#include "indri/TermListFileIterator.hpp"
#include "indri/DocumentDataIterator.hpp"
#include "indri/Lockable.hpp"
#include "lemur/IndexTypes.hpp"

namespace indri {
  namespace index {

    class Index {
    public:
      /// Field data
      struct FieldDescription {
        /// name of the field
        std::string name;
        /// does the field contain numeric data
        bool numeric;
        /// parser for the numeric field data
        std::string parserName;
        /// does the field have an ordinal
        bool ordinal;
        /// does the field have its parent
        bool parental;
      };

      virtual ~Index() {};

      //
      // Actions
      //

      virtual void close() = 0;

      //
      // Counts
      //
      
      virtual lemur::api::DOCID_T documentBase() = 0;
      /// The documentMaximum is at least one greater than the largest documentID used in this index.
      virtual lemur::api::DOCID_T documentMaximum() = 0;

      virtual lemur::api::TERMID_T term( const char* term ) = 0;
      virtual lemur::api::TERMID_T term( const std::string& term ) = 0;
      virtual std::string term( lemur::api::TERMID_T termID ) = 0;

      virtual int field( const char* fieldName ) = 0;
      virtual int field( const std::string& fieldName ) = 0;
      virtual std::string field( int fieldID ) = 0;

      virtual int documentLength( lemur::api::DOCID_T documentID ) = 0;
      virtual UINT64 documentCount() = 0;
      virtual UINT64 documentCount( const std::string& term ) = 0;

      virtual UINT64 uniqueTermCount() = 0;

      virtual UINT64 termCount( const std::string& term ) = 0;
      virtual UINT64 termCount() = 0;

      virtual UINT64 fieldTermCount( const std::string& field ) = 0;
      virtual UINT64 fieldTermCount( const std::string& field, const std::string& term ) = 0;

      virtual UINT64 fieldDocumentCount( const std::string& field ) = 0;
      virtual UINT64 fieldDocumentCount( const std::string& field, const std::string& term ) = 0;

      // Lists
      virtual DocListIterator* docListIterator( lemur::api::TERMID_T termID ) = 0;
      virtual DocListIterator* docListIterator( const std::string& term ) = 0;
      virtual DocListFileIterator* docListFileIterator() = 0;
      virtual DocExtentListIterator* fieldListIterator( int fieldID ) = 0;
      virtual DocExtentListIterator* fieldListIterator( const std::string& field ) = 0;
      virtual const TermList* termList( lemur::api::DOCID_T documentID ) = 0;
      virtual TermListFileIterator* termListFileIterator() = 0;
      virtual DocumentDataIterator* documentDataIterator() = 0;

      // Vocabulary
      virtual VocabularyIterator* frequentVocabularyIterator() = 0;
      virtual VocabularyIterator* infrequentVocabularyIterator() = 0;
      virtual VocabularyIterator* vocabularyIterator() = 0;

      // Locks
      virtual indri::thread::Lockable* iteratorLock() = 0;
      virtual indri::thread::Lockable* statisticsLock() = 0;
                        
    };
  }
}

#endif // INDRI_INDEX_HPP

