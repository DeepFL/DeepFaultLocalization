/*==========================================================================
 * Copyright (c) 2002-2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// CombinedVocabularyIterator
//
// 20 January 2005 -- tds
//

#ifndef INDRI_COMBINEDVOCABULARYITERATOR_HPP
#define INDRI_COMBINEDVOCABULARYITERATOR_HPP

#include "indri/VocabularyIterator.hpp"
#include "indri/DiskTermData.hpp"
/// namespaces within the indri system
namespace indri {
  /// index construction and interaction components
  namespace index {
    class CombinedVocabularyIterator : public VocabularyIterator {
    private:
      VocabularyIterator* _first;
      VocabularyIterator* _second;
      int _secondBase;
      bool _finished;
      bool _usingSecond;

    public:
      CombinedVocabularyIterator( VocabularyIterator* first, VocabularyIterator* second, int secondBase );
      ~CombinedVocabularyIterator();

      void startIteration();
      bool nextEntry();
      bool nextEntry(const char *skipTo);
      DiskTermData* currentEntry();
      bool finished();
    };
  }
}

#endif // INDRI_COMBINEDVOCABULARYITERATOR_HPP

