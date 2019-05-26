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
// DocumentVector
//
// 26 July 2004 -- tds
//

#include "indri/DocumentVector.hpp"
#include "indri/TermList.hpp"
#include "indri/Index.hpp"

//
// _init
//

void indri::api::DocumentVector::_init( indri::index::Index* index, const indri::index::TermList* termList, std::map<int,std::string>* termStringMap ) {
  const indri::utility::greedy_vector<int>& terms = termList->terms();
  const indri::utility::greedy_vector<indri::index::FieldExtent>& fields = termList->fields();

  // build a vocabulary
  std::map<int, int> termIDMap;
  _stems.push_back( "[OOV]" );

  for( size_t i=0; i<terms.size(); i++ ) {
    if( terms[i] == 0 ) {
      _positions.push_back(0);
    } else {
      std::map<int, int>::iterator iter = termIDMap.find( terms[i] );
      
      if( iter != termIDMap.end() ) {
        // we've seen this stem, add its position
        _positions.push_back(iter->second);
      } else {
        std::string termString;
        std::map<int,std::string>::iterator siter;
        
        // the term string map may be passed in from a caller that's fetching a bunch
        // of doc vectors at once--the cache can help us find terms a little faster
        if( termStringMap && (siter = termStringMap->find(terms[i])) != termStringMap->end() ) {
          termString = siter->second;
        } else {
          termString = index->term(terms[i]);
        }

        _stems.push_back( termString );
        _positions.push_back((int)_stems.size()-1);
        termIDMap[terms[i]] = (int)_stems.size()-1;

        // put the string in the termStringMap for future DocumentVector builders
        if( termStringMap && siter == termStringMap->end() ) {
          (*termStringMap)[terms[i]] = termString;
        }
      }
    }
  }

  for( size_t i=0; i<fields.size(); i++ ) {
    Field f(fields[i]);

    f.name = index->field(fields[i].id);

    _fields.push_back(f);
  }
}

indri::api::DocumentVector::DocumentVector() {
}

indri::api::DocumentVector::DocumentVector( indri::index::Index* index, const indri::index::TermList* termList ) {
  _init( index, termList, 0 );
}

indri::api::DocumentVector::DocumentVector( indri::index::Index* index, const indri::index::TermList* termList, std::map<int,std::string>& termStringMap ) {
  _init( index, termList, &termStringMap );
}

const std::vector<std::string>& indri::api::DocumentVector::stems() const {
  return _stems;
}

std::vector<std::string>& indri::api::DocumentVector::stems() {
  return _stems;
}

const std::vector<int>& indri::api::DocumentVector::positions() const {
  return _positions;
}

std::vector<int>& indri::api::DocumentVector::positions() {
  return _positions;
}

const std::vector<indri::api::DocumentVector::Field>& indri::api::DocumentVector::fields() const {
  return _fields;
}

std::vector<indri::api::DocumentVector::Field>& indri::api::DocumentVector::fields() {
  return _fields;
}

