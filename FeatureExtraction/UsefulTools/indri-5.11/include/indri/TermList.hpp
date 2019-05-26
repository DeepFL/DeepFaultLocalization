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
// TermList
//
// 23 November 2004 -- tds
//

#ifndef INDRI_TERMLIST_HPP
#define INDRI_TERMLIST_HPP

#include "indri/greedy_vector"
#include "indri/FieldExtent.hpp"
#include "lemur/RVLCompress.hpp"
#include "indri/Buffer.hpp"
#include "indri/RVLCompressStream.hpp"
#include "indri/RVLDecompressStream.hpp"
#include "lemur/IndexTypes.hpp"

namespace indri {
  namespace index {
    class TermList {
    private:
      indri::utility::greedy_vector<lemur::api::TERMID_T> _terms;
      indri::utility::greedy_vector<FieldExtent> _fields;

    public:
      void clear() {
        _terms.clear();
        _fields.clear();
      }
      
      void addField( const indri::index::FieldExtent& field ) {
        _fields.push_back( field );
      }
      
      void addTerm( const lemur::api::TERMID_T termID ) {
        _terms.push_back( termID );
      }
      
      indri::utility::greedy_vector<lemur::api::TERMID_T>& terms() {
        return _terms;
      }
      
      const indri::utility::greedy_vector<lemur::api::TERMID_T>& terms() const {
        return _terms;
      }
      
      indri::utility::greedy_vector<indri::index::FieldExtent>& fields() {
        return _fields;
      }
      
      const indri::utility::greedy_vector<indri::index::FieldExtent>& fields() const {
        return _fields;
      }
      
      void read( const char* buffer, int size ) {
        clear();
        indri::utility::RVLDecompressStream stream( buffer, size );
        
        int termCount;
        int fieldCount;
        
        stream >> termCount
               >> fieldCount;
        
        for( int i=0; i<termCount; i++ ) {
          lemur::api::TERMID_T termID;
          stream >> termID;

          assert( termID >= 0 );
          _terms.push_back( termID ); 
        }
        
        for( int i=0; i<fieldCount; i++ ) {
          FieldExtent extent;
          
          stream >> extent.id
                 >> extent.parentOrdinal
                 >> extent.begin
                 >> extent.end
                 >> extent.number;
          
          assert( extent.id >= 0 );
          assert( extent.parentOrdinal >= 0 );
          assert( extent.begin >= 0 );
          assert( extent.end >= extent.begin );
          
          extent.ordinal = i + 1;

          _fields.push_back( extent );
        }
      }
      
      void write( indri::utility::Buffer& buffer ) {
        // format:
        //   term count
        //   field count
        //   termID * termCount (compressed)
        //   ( fieldID, begin, end, number ) * fieldCount
        
        indri::utility::RVLCompressStream out( buffer );
        
        // write count of terms and fields in the document first
        int termCount = (int)_terms.size();
        int fieldCount = (int)_fields.size();

        out << termCount
            << fieldCount;
        
        // write out terms
        for( size_t i=0; i<_terms.size(); i++ ) {
          assert( _terms[i] >= 0 );
          out << _terms[i];
        }

        // write out fields
        for( size_t i=0; i<_fields.size(); i++ ) {

          assert( _fields[i].id >= 0 );
          assert( _fields[i].ordinal == i + 1 );

          out << _fields[i].id
              << _fields[i].parentOrdinal
              << _fields[i].begin
              << _fields[i].end
              << _fields[i].number;
        }
      }
    };
  }
}

#endif // INDRI_TERMLIST_HPP
