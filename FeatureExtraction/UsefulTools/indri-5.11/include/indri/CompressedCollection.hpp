/*==========================================================================
 * Copyright (c) 2003-2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// CompressedCollection.hpp
//
// 12 May 2004 -- tds
//

#ifndef INDRI_COMPRESSEDCOLLECTION_HPP
#define INDRI_COMPRESSEDCOLLECTION_HPP

#include "indri/Collection.hpp"
#include "lemur/string-set.h"
#include <string>
#include <vector>
#include "lemur/Keyfile.hpp"
#include "indri/Buffer.hpp"
#include "indri/SequentialWriteBuffer.hpp"
#include "indri/SequentialReadBuffer.hpp"
#include "indri/HashTable.hpp"
#include "indri/File.hpp"
#include "indri/Mutex.hpp"
#include "lemur/IndexTypes.hpp"
#include "indri/DeletedDocumentList.hpp"

typedef struct z_stream_s* z_stream_p;

namespace indri
{
  namespace collection
  {
    
    class CompressedCollection : public Collection {
    private:
      indri::thread::Mutex _lock;

      std::string _basePath;
      lemur::file::Keyfile _lookup;
      indri::file::File _storage;
      indri::file::SequentialWriteBuffer* _output;
      indri::utility::Buffer _positionsBuffer;
      z_stream_p _stream;

      indri::utility::HashTable<const char*, lemur::file::Keyfile*> _reverseLookups;
      indri::utility::HashTable<const char*, lemur::file::Keyfile*> _forwardLookups;
      String_set* _strings;

      void _writePositions( indri::api::ParsedDocument* document, int& keyLength, int& valueLength );
      void _writeMetadataItem( indri::api::ParsedDocument* document, int i, int& keyLength, int& valueLength );
      void _writeText( indri::api::ParsedDocument* document, int& keyLength, int& valueLength );
      void _writeContent( indri::api::ParsedDocument* document, int& keyLength, int& valueLength );
      void _writeContentLength( indri::api::ParsedDocument* document, int& keyLength, int& valueLength );

      void _readPositions( indri::api::ParsedDocument* document, const void* positionData, int positionDataLength );

      void _removeForwardLookups( indri::index::DeletedDocumentList& deletedList, lemur::file::Keyfile& keyfile );
      void _removeReverseLookups( indri::index::DeletedDocumentList& deletedList, lemur::file::Keyfile& keyfile );

      void _copyForwardLookup( const std::string& name,
                               lemur::file::Keyfile& other,
                               indri::index::DeletedDocumentList& deletedList,
                               lemur::api::DOCID_T documentOffset );

      void _copyReverseLookup( const std::string& name,
                               lemur::file::Keyfile& other,
                               indri::index::DeletedDocumentList& deletedList,
                               lemur::api::DOCID_T documentOffset );


      void _copyStorageEntry( indri::file::SequentialReadBuffer* input,
                              indri::file::SequentialWriteBuffer* output, 
                              int key,
                              UINT64 position,
                              UINT64 length, 
                              lemur::file::Keyfile& lookup );
      void _copyStorageData( indri::file::SequentialReadBuffer* input,
                             indri::file::SequentialWriteBuffer* output,
                             indri::index::DeletedDocumentList& deletedList,
                             lemur::api::DOCID_T documentOffset,
                             lemur::file::Keyfile& sourceLookup,
                             lemur::file::Keyfile& destLookup,
                             UINT64 storageLength );
      void _copyForwardLookup( const std::string& name, lemur::file::Keyfile& other, lemur::api::DOCID_T documentOffset );

      bool _storeDocs;      
    public:
      CompressedCollection();
      ~CompressedCollection();

      void create( const std::string& fileName );
      void create( const std::string& fileName, const std::vector<std::string>& indexedFields );
      void create( const std::string& fileName, const std::vector<std::string>& forwardIndexedFields, const std::vector<std::string>& reverseIndexedFields,  bool storeDocs = true );
      void reopen( const std::string& fileName );
      void open( const std::string& fileName );
      void openRead( const std::string& fileName );
      void close();
      bool exists(lemur::api::DOCID_T documentID);
      indri::api::ParsedDocument* retrieve( lemur::api::DOCID_T documentID );
      std::string retrieveMetadatum( lemur::api::DOCID_T documentID, const std::string& attributeName );
      std::vector<indri::api::ParsedDocument*> retrieveByMetadatum( const std::string& attributeName, const std::string& value );
      std::vector<lemur::api::DOCID_T> retrieveIDByMetadatum( const std::string& attributeName, const std::string& value );

      void addDocument( lemur::api::DOCID_T documentID, indri::api::ParsedDocument* document );
      void compact( indri::index::DeletedDocumentList& deletedList );
      void append( indri::collection::CompressedCollection& other, indri::index::DeletedDocumentList& deletedList, lemur::api::DOCID_T documentOffset );

      std::vector<std::string> forwardFields();
      std::vector<std::string> reverseFields();
    };
  }
}

#endif // INDRI_COMPRESSEDCOLLECTION_HPP
