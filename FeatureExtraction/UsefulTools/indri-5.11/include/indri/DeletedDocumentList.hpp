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
// DeletedDocumentList
//
// 3 February 2005 -- tds
//
// While vector<bool> might have been a nice clean choice for this
// bitmap, there was no platform-independent way to just blit it
// to disk.  I decided to roll my own using a Buffer.
//

#ifndef INDRI_DELETEDDOCUMENTLIST_HPP
#define INDRI_DELETEDDOCUMENTLIST_HPP

#include <vector>
#include "indri/ReadersWritersLock.hpp"
#include "indri/ReaderLockable.hpp"
#include "indri/WriterLockable.hpp"
#include "indri/Buffer.hpp"
#include "lemur/IndexTypes.hpp"

namespace indri
{
  namespace index
  {
    
    class DeletedDocumentList {
    private:
      bool _modified;
      indri::thread::ReadersWritersLock _lock;
      indri::thread::ReaderLockable _readLock;
      indri::thread::WriterLockable _writeLock;
      UINT64 _deletedCount;

      indri::utility::Buffer _bitmap;
      void _grow( lemur::api::DOCID_T documentID );
      void _calculateDeletedCount();

    public:
      class read_transaction {
      private:
        indri::thread::ReadersWritersLock& _lock;
        indri::utility::Buffer& _bitmap;

      public:
        read_transaction( DeletedDocumentList& list );
        ~read_transaction();

        lemur::api::DOCID_T nextCandidateDocument( lemur::api::DOCID_T documentID );
        bool isDeleted( lemur::api::DOCID_T documentID ) const;
      };

      DeletedDocumentList();

      void append( DeletedDocumentList& other, int documentCount );
      void markDeleted( lemur::api::DOCID_T documentID );
      bool isDeleted( lemur::api::DOCID_T documentID );
      UINT64 deletedCount() const;
      read_transaction* getReadTransaction();

      void read( const std::string& filename );
      void write( const std::string& filename );
    };
  }
}

#endif // INDRI_DELETEDDOCUMENTLIST_HPP


