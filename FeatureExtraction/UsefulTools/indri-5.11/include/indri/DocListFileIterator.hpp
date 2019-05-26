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
// DocListFileIterator
//
// 9 January 2004 - tds
//

#ifndef INDRI_DOCLISTFILEITERATOR_HPP
#define INDRI_DOCLISTFILEITERATOR_HPP

#include "indri/DocListIterator.hpp"
#include "indri/TermData.hpp"

namespace indri {
  namespace index {
    class DocListFileIterator {
    public:
      struct DocListData {
        DocListIterator* iterator;
        TermData* termData;
      };

      struct iterator_greater {
        bool operator() ( const DocListFileIterator*const& one, const DocListFileIterator*const& two ) const {
          assert( !one->finished() && !two->finished() );
          
          const DocListData* oneData = one->currentEntry();
          const DocListData* twoData = two->currentEntry();

          const char* oneTerm = oneData->termData->term;
          const char* twoTerm = twoData->termData->term;

          int result = strcmp( oneTerm, twoTerm );

          // if terms don't match, we're done
          if( result < 0 )
            return false;
          if( result > 0 )
            return true;

          // terms match, so go by document
          lemur::api::DOCID_T oneDocument = oneData->iterator->currentEntry() ? oneData->iterator->currentEntry()->document : 0;
          lemur::api::DOCID_T twoDocument = twoData->iterator->currentEntry() ? twoData->iterator->currentEntry()->document : 0;

          return oneDocument > twoDocument;
        }
      };

      virtual ~DocListFileIterator() {};
      
      virtual bool finished() const = 0;
      virtual void startIteration() = 0;

      virtual bool nextEntry() = 0;
      virtual DocListData* currentEntry() = 0;
      virtual const DocListData* currentEntry() const = 0;
    };
  }
}

#endif // INDRI_DOCLISTITERATOR_HPP

