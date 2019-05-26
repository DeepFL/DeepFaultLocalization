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
// TermTranslator
//
// 14 January 2005 -- tds
//

#ifndef INDRI_TERMTRANSLATOR_HPP
#define INDRI_TERMTRANSLATOR_HPP

#include "indri/HashTable.hpp"
#include <vector>
#include "indri/TermBitmap.hpp"

namespace indri {
  namespace index {
    class TermTranslator {
    private:
      TermBitmap* _bitmap;
      int _previousFrequentCount;
      int _currentFrequentCount;
      int _previousTermCount;
      int _currentTermCount;

      std::vector<lemur::api::TERMID_T>* _frequentMap;
      indri::utility::HashTable<lemur::api::TERMID_T, lemur::api::TERMID_T>* _wasInfrequentMap;

    public:
      ~TermTranslator() {
        delete _frequentMap;
      }

      TermTranslator( int previousFrequentCount,
                      int currentFrequentCount,
                      int previousTermCount,
                      int currentTermCount,
                      std::vector<lemur::api::TERMID_T>* frequentMap,
                      indri::utility::HashTable<lemur::api::TERMID_T, lemur::api::TERMID_T>* wasInfrequentMap,
                      TermBitmap* bitmap ) 
        :
        _bitmap(bitmap),
        _frequentMap(frequentMap),
        _wasInfrequentMap(wasInfrequentMap)
      {
        assert( currentFrequentCount >= 0 );
        assert( previousFrequentCount >= 0 );
        assert( previousTermCount >= 0 );
        assert( currentTermCount >= 0 );

        _previousFrequentCount = previousFrequentCount;
        _currentFrequentCount = currentFrequentCount;
        _previousTermCount = previousTermCount;
        _currentTermCount = currentTermCount;
      }

      lemur::api::TERMID_T operator() ( lemur::api::TERMID_T termID ) {
        assert( termID >= 0 );
        assert( termID <= _previousTermCount );
        lemur::api::TERMID_T result = 0;
        lemur::api::TERMID_T* value;

        if( termID <= _previousFrequentCount ) {
          // common case, termID is a frequent term
          assert( _frequentMap->size() > termID );
          result = (*_frequentMap)[termID];
        } else {
          // term may have become frequent, so check the wasInfrequentMap
          value = (*_wasInfrequentMap).find( termID - _previousFrequentCount - 1 );

          if( value ) {
            result = *value;
          } else {
            // term wasn't frequent and isn't now either, so get it from the bitmap
            result = 1 + _currentFrequentCount + _bitmap->get( termID - _previousFrequentCount - 1 );
          }
        }

        assert( result >= 0 );
        assert( result <= _currentTermCount );
        return result;
      }
    };
  }
}

#endif // INDRI_TERMTRANSLATOR_HPP



