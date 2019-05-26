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
// PriorListIterator.hpp
//
// 22 July 2005 -- tds
//

#ifndef INDRI_PRIORLISTITERATOR_HPP
#define INDRI_PRIORLISTITERATOR_HPP

#include "indri/SequentialReadBuffer.hpp"
#include "indri/greedy_vector"
#include "lemur/IndexTypes.hpp"

namespace indri {
  namespace collection {
    class PriorListIterator {
    public:
      struct Entry {
        lemur::api::DOCID_T document;
        double score;
      };
    
    private:
      indri::file::SequentialReadBuffer* _file;
      Entry _entry;
      indri::utility::greedy_vector<double> _lookup;
      
      bool _finished;
      UINT32 _entryCount;
      UINT32 _entryLength;
      UINT32 _tableLength;
      
    public:
      PriorListIterator( indri::file::SequentialReadBuffer* file );
      ~PriorListIterator();
    
      void startIteration();
      void nextEntry();
      void nextEntry( lemur::api::DOCID_T document );
      Entry* currentEntry();
      bool finished();
    };
  }
}

#endif // INDRI_PRIORLISTITERATOR_HPP
