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
// RepositoryMaintenanceThread
//
// 31 January 2005 -- tds
//

#include "indri/RepositoryMaintenanceThread.hpp"
#include "indri/Repository.hpp"
#include "indri/ScopedLock.hpp"
#include <iostream>

const UINT64 TIME_DELAY = 10*1000*1000;
const UINT64 SHORT_TIME_DELAY = 3*1000*1000;
const UINT64 THRASHING_MERGE_DELAY = 300*1000*1000;
const int MAXIMUM_INDEX_COUNT = 50;


//
// maintenance_smoothed_load
//

static float maintenance_smoothed_load( indri::collection::Repository::Load& load ) {
  const float fifteenWeight = 0.2f;
  const float fiveWeight = 0.2f;
  const float oneWeight = 0.6f;

  return load.fifteen * fifteenWeight + load.five * fiveWeight + load.one * oneWeight;
}

//
// maintenance_should_merge
//

static bool maintenance_should_merge( indri::collection::Repository::index_state& state, indri::collection::Repository::Load& documentLoad, indri::collection::Repository::Load& queryLoad, UINT64 lastThrashing ) {
  float smoothedQueryLoad = maintenance_smoothed_load( queryLoad ) + 1;
  float smoothedDocumentLoad = maintenance_smoothed_load( documentLoad );
  float addRatio = smoothedDocumentLoad / (smoothedQueryLoad+1); 

  bool hasntThrashedRecently = lastThrashing > THRASHING_MERGE_DELAY;
  bool significantQueryLoad = smoothedQueryLoad > 2;
  bool insignificantDocumentLoad = smoothedDocumentLoad < 1;
  int indexesToMerge = (int)state->size(); 
  bool couldUseMerge = indexesToMerge >= 3; // trim expects 3

  // extremely heuristic choice for when indexes should be merged:
  //   we merge if there are a lot of incoming queries relative to
  //   the amount of documents added; and this
  //   is all weighted by the number of indexes we have to merge.

  return couldUseMerge &&
    hasntThrashedRecently &&
    ((addRatio/50) < indexesToMerge && (significantQueryLoad || insignificantDocumentLoad));
}

//
// maintenance_should_trim
//

static bool maintenance_should_trim( indri::collection::Repository::index_state& state, indri::collection::Repository::Load& documentLoad, indri::collection::Repository::Load& queryLoad, UINT64 lastThrashing ) {
  return state->size() > MAXIMUM_INDEX_COUNT;
}

//
// constructor
//

indri::collection::RepositoryMaintenanceThread::RepositoryMaintenanceThread( indri::collection::Repository& repository, UINT64 memory ) :
  UtilityThread(),
  _repository( repository ),
  _memory( memory )
{
}

//
// initialize
//

UINT64 indri::collection::RepositoryMaintenanceThread::initialize() {
  return TIME_DELAY;
}

//
// deinitialize
//

void indri::collection::RepositoryMaintenanceThread::deinitialize() {
  // do nothing
}

//
// work
//

UINT64 indri::collection::RepositoryMaintenanceThread::work() {
  // fetch current index state
  bool write = false;
  bool merge = false;
  bool trim = false;
  UINT64 memorySize = 0;

  {
    // lock the request queue
    indri::thread::ScopedLock l( _requestLock );

    // if nobody has any requests, check to see if we should be working
    if( ! _requests.size() ) {
      // get the memory size of the active memory index
      Repository::index_state state = _repository.indexes();
      indri::index::MemoryIndex* index = dynamic_cast<indri::index::MemoryIndex*>(state->back());

      if( index ) {
        // if the index is too big, we'd better get to work
        memorySize = index->memorySize();
	// account for the size of the DiskIndexes (~22M)
        for( size_t i=0; i<state->size()-1; i++ ) {
          memorySize += lemur_compat::min<size_t>(4*((*state)[i]->documentCount()),indri::index::DiskIndex::MAX_DOCLENGTHS_CACHE);
        }

        if( _memory < memorySize ) {
          _requests.push( WRITE );
        }

        Repository::Load documentLoad = _repository.documentLoad();
        Repository::Load queryLoad = _repository.queryLoad();
        UINT64 lastThrashing = _repository._timeSinceThrashing();

        // if the number of open index files is pushing the open file limit,
        // also schedule a merge.
        unsigned int openFiles = _repository._mergeFiles(*state);
        bool should_merge = openFiles > MERGE_FILE_LIMIT;

        if( should_merge || maintenance_should_merge( state, documentLoad, queryLoad, lastThrashing ) ) {
	  // do a trim, final close will merge down to a single disk index.
          _requests.push( TRIM );
        } else if( maintenance_should_trim( state, documentLoad, queryLoad, lastThrashing ) ) {
          _requests.push( TRIM );
        }
      }
    }

    // now, figure out what needs to be done
    while( _requests.size() ) {
      switch( _requests.front() ) {
      case MERGE:
        merge = true;
        break;

      case TRIM:
        trim = true;
        break;

      case WRITE:
        write = true;
        break;
      }

      _requests.pop();
    }
  
    // unlock request queue
  }
  // may want to reset the thrashing flag after trim/write
  _repository._setThrashing( false );

  if( merge ) {
    _repository._merge();
  } else if( trim ) {
    _repository._trim();
  } else if( write ) {
    _repository._write();
  }

  if( memorySize > 0.75*_memory ) {
    return SHORT_TIME_DELAY;
  } else {
    return TIME_DELAY;
  }
}

//
// hasWork
//

bool indri::collection::RepositoryMaintenanceThread::hasWork() {
  indri::thread::ScopedLock l( _requestLock );
  return _requests.size() > 0;
}

//
// write
//

void indri::collection::RepositoryMaintenanceThread::write() {
  indri::thread::ScopedLock l( _requestLock );
  _requests.push( WRITE );
}

//
// merge
//

void indri::collection::RepositoryMaintenanceThread::merge() {
  indri::thread::ScopedLock l( _requestLock );
  _requests.push( MERGE );
}


