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
// DocListIterator
//
// 9 January 2004 - tds
//

#ifndef INDRI_DOCLISTITERATOR_HPP
#define INDRI_DOCLISTITERATOR_HPP

#include "indri/greedy_vector"
#include "indri/TermData.hpp"
#include "lemur/IndexTypes.hpp"

namespace indri {
  namespace index {
    class DocListIterator {
    public:
      struct DocumentData {
        lemur::api::DOCID_T document;
        indri::utility::greedy_vector<int> positions;
      };

      struct TopDocument {
        struct less {
          bool operator() ( const TopDocument& one, const TopDocument& two ) const {
            double oneFrac = double(one.count) / double(one.length);
            double twoFrac = double(two.count) / double(two.length);
            return (oneFrac < twoFrac);
          }
        };

        struct greater {
          bool operator() ( const TopDocument& one, const TopDocument& two ) const {
            double oneFrac = double(one.count) / double(one.length);
            double twoFrac = double(two.count) / double(two.length);
            return (oneFrac > twoFrac);
          }
        };

        struct docid_less {
          bool operator() ( const TopDocument& one, const TopDocument& two ) const {
            return one.document < two.document;
          }
        };

        TopDocument( lemur::api::DOCID_T _document, int _count, int _length ) :
          document(_document),
          count(_count),
          length(_length)
        {
        }

        lemur::api::DOCID_T document;
        int count;
        int length;
      };
      
      virtual ~DocListIterator() {};

      // get the iterator ready to return data; call this before calling currentEntry or nextEntry
      virtual void startIteration() = 0;

      // get the termData structure associated with this term
      virtual TermData* termData() = 0;

      // get a list of top documents for this iterator (must call startIteration() first)
      virtual const indri::utility::greedy_vector<TopDocument>& topDocuments() = 0;

      // return the current document entry if we're not finished, null otherwise.
      virtual DocumentData* currentEntry() = 0;
    
      // move to the next document in the list; return false if there are no more valid documents
      virtual bool nextEntry() = 0;

      // find the first document that contains this term that has an id >= documentID.
      // returns false if no such document exists.
      virtual bool nextEntry( lemur::api::DOCID_T documentID ) = 0;

      // returns true if the iterator has no more entries
      virtual bool finished() = 0;
    };
  }
}

#endif // INDRI_DOCLISTITERATOR_HPP


