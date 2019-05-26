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
// Repository
//
// 21 May 2004 -- tds
//

#ifndef INDRI_REPOSITORY_HPP
#define INDRI_REPOSITORY_HPP

#include <iterator>

#include "indri/Parameters.hpp"
#include "indri/Transformation.hpp"
#include "indri/MemoryIndex.hpp"
#include "indri/DiskIndex.hpp"
#include "indri/ref_ptr.hpp"
#include "indri/DeletedDocumentList.hpp"
#include "indri/PriorListIterator.hpp"
#include <string>
// 512 -- syslimit can be 1024
#define MERGE_FILE_LIMIT 768 
namespace indri
{
  /// document manager and ancillary collection components.
  namespace collection
  {
    
    /*! Encapsulates document manager, index, and field indexes. Provides access 
     *  to collection for both IndexEnvironment and QueryEnvironment.
     */
    class Repository {
    public:
      struct Load {
        float one;
        float five;
        float fifteen;
      };

      struct Field {
        std::string name;
        std::string parserName;
        bool numeric;
        bool ordinal;
        bool parental;
      };

      typedef std::vector<indri::index::Index*> index_vector;
      typedef indri::atomic::ref_ptr<index_vector> index_state;

    private:
      friend class RepositoryMaintenanceThread;
      friend class RepositoryLoadThread;

      class RepositoryMaintenanceThread* _maintenanceThread;
      class RepositoryLoadThread* _loadThread;

      indri::thread::Mutex _stateLock; /// protects against state changes
      std::vector<index_state> _states;
      index_state _active;
      int _indexCount;

      // running flags
      volatile bool _maintenanceRunning;
      volatile bool _loadThreadRunning;

      indri::thread::Mutex _addLock; /// protects addDocument

      class CompressedCollection* _collection;
      indri::index::DeletedDocumentList _deletedList;

      indri::api::Parameters _parameters;
      std::vector<indri::parse::Transformation*> _transformations;
      std::vector<Field> _fields;
      std::vector<indri::index::Index::FieldDescription> _indexFields;
      std::map<std::string, indri::file::File*> _priorFiles;

      std::string _path;
      bool _readOnly;

      INT64 _memory;

      UINT64 _lastThrashTime;
      volatile bool _thrashing;

      enum { LOAD_MINUTES = 15, LOAD_MINUTE_FRACTION = 12 };

      indri::atomic::value_type _queryLoad[ LOAD_MINUTES * LOAD_MINUTE_FRACTION ];
      indri::atomic::value_type _documentLoad[ LOAD_MINUTES * LOAD_MINUTE_FRACTION ];

      static std::vector<std::string> _fieldNames( indri::api::Parameters& parameters );
      static std::string _stemmerName( indri::api::Parameters& parameters );

      static void _mergeClosedIndexes( const std::string& outputPath,
                                       const std::vector<std::string>& repositories,
                                       const std::vector<indri::collection::Repository::Field>& indexFields,
                                       const std::vector<lemur::api::DOCID_T>& documentMaximums );
      static void _writeMergedManifest( const std::string& path, indri::api::Parameters& firstManifest );
      static void _mergeBitmaps( const std::string& outputPath, const std::vector<std::string>& repositories, const std::vector<lemur::api::DOCID_T>& documentCounts );
      static void _mergeCompressedCollections( const std::string& outputPath,
                                                                 const std::vector<std::string>& repositories,
                                                                 const std::vector<lemur::api::DOCID_T>& documentMaximums );
      static void _cleanAndCreateDirectory( const std::string& path );

      void _writeParameters( const std::string& path );
      void _checkpoint();
      void _incrementLoad();
      void _countDocumentAdd();
      Load _computeLoad( indri::atomic::value_type* loadArray );
      
      void _openPriors( const std::string& path );
      void _closePriors();

      void _buildFields();
      void _buildChain( indri::api::Parameters& parameters,
                        indri::api::Parameters *options );

      void _copyParameters( indri::api::Parameters& options );

      void _removeStates( std::vector<index_state>& toRemove );
      void _remove( const std::string& path );

      void _openIndexes( indri::api::Parameters& params, const std::string& parentPath );
      std::vector<index_state> _statesContaining( std::vector<indri::index::Index*>& indexes );
      bool _stateContains( index_state& state, std::vector<indri::index::Index*>& indexes );
      void _swapState( std::vector<indri::index::Index*>& oldIndexes, indri::index::Index* newIndex );
      void _closeIndexes();
      static std::vector<indri::index::Index::FieldDescription> _fieldsForIndex( const std::vector<Repository::Field>& _fields );
      void _merge( index_state& state );
      indri::index::Index* _mergeStage( index_state& state );
      UINT64 _mergeMemory( const std::vector<indri::index::Index*>& indexes );
      unsigned int _mergeFiles( const std::vector<indri::index::Index*>& indexes );

      // these methods should only be called by the maintenance thread
      /// merge all known indexes together
      void _merge(); 
      /// write the active index to disk
      void _write();
      /// merge together some of the more recent indexes
      void _trim();

      void _startThreads();
      void _stopThreads();

      void _setThrashing( bool flag );
      UINT64 _timeSinceThrashing();
      void _addMemoryIndex();

    public:
      Repository() {
        _collection = 0;
        _readOnly = false;
        _lastThrashTime = 0;
        _thrashing = false;
        memset( (void*) _documentLoad, 0, sizeof(indri::atomic::value_type)*LOAD_MINUTES*LOAD_MINUTE_FRACTION );
        memset( (void*) _queryLoad, 0, sizeof(indri::atomic::value_type)*LOAD_MINUTES*LOAD_MINUTE_FRACTION );
      }

      ~Repository() {
        close();
      }
      /// add a parsed document to the repository.
      /// @param document the document to add.
      /// @param inCollection if true, add the document to the CompressedCollection.
      int addDocument( indri::api::ParsedDocument* document, bool inCollection  = true );
      /// delete a document from the repository
      /// @param documentID the internal ID of the document to delete
      void deleteDocument( int documentID );
      /// @return the indexed fields for this collection
      const std::vector<Field>& fields() const;
      /// @return the tags for this collection
      std::vector<std::string> tags() const;
      /// @return the named priors list for this collection
      std::vector<std::string> priors() const;
      /// Process, possibly transforming, the given term
      /// @param term the term to process
      /// @return the processed term
      std::string processTerm( const std::string& term );
      /// @return the compressed document collection
      class CompressedCollection* collection();
      /// Create a new empty repository.
      /// @param path the directory to create the repository in
      /// @param options additional parameters
      void create( const std::string& path, indri::api::Parameters* options = 0 );
      /// Open an existing repository.
      /// @param path the directory to open the repository from
      /// @param options additional parameters
      void open( const std::string& path, indri::api::Parameters* options = 0 );
      /// Open an existing repository in read only mode.
      /// @param path the directory to open the repository from
      /// @param options additional parameters
      void openRead( const std::string& path, indri::api::Parameters* options = 0 );
      /// @return true if a valid Indri Repository resides in the named path
      /// false otherwise.
      /// @param path the directory to open the repository from
      static bool exists( const std::string& path );
      /// Close the repository
      void close();

      /// Compact the repository by removing all information about
      /// deleted documents from disk.
      void compact();

      /// Indexes in this repository
      index_state indexes();
      
      /// Return a prior iterator
      indri::collection::PriorListIterator* priorListIterator( const std::string& priorName );

      /// Notify the repository that a query has happened
      void countQuery();

      /// Write the most recent state out to disk
      void write();

      /// Merge all internal indexes together
      void merge(); 
      
      /// Make an empty repository directory on disk
      static void makeEmpty( const std::string& path );

      /// Merge two or more repositories together
      static void merge( const std::string& outputIndex, const std::vector<std::string>& inputIndexes );

      /// List of deleted documents in this repository
      indri::index::DeletedDocumentList& deletedList();

      /// Returns the average number of documents added each minute in the last 1, 5 and 15 minutes
      Load queryLoad();

      /// Returns the average number of documents added each minute in the last 1, 5 and 15 minutes
      Load documentLoad();
    };
  }
}

#endif // INDRI_REPOSITORY_HPP

