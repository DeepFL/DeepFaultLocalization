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
// RepositoryLoadThread
//
// 31 January 2005 -- tds
//

#include "indri/RepositoryLoadThread.hpp"
#include "indri/Repository.hpp"

const int FIVE_SECONDS = 5*1000*1000;
const int HALF_SECOND = 500*1000;

//
// constructor
//

indri::collection::RepositoryLoadThread::RepositoryLoadThread( indri::collection::Repository& repository, UINT64 memory ) :
  UtilityThread(),
  _repository( repository ),
  _memory( memory )
{
}

//
// initialize
//

UINT64 indri::collection::RepositoryLoadThread::initialize() {
  return FIVE_SECONDS;
}

//
// deinitialize
//

void indri::collection::RepositoryLoadThread::deinitialize() {
  // do nothing
}

//
// work
//

UINT64 indri::collection::RepositoryLoadThread::work() {
  _repository._incrementLoad();

  indri::collection::Repository::index_state state = _repository.indexes();
  indri::utility::greedy_vector<indri::index::MemoryIndex*> indexes;
  UINT64 memorySize = 0;

  for( size_t i=0; i<state->size(); i++ ) {
    indri::index::MemoryIndex* index = dynamic_cast<indri::index::MemoryIndex*>((*state)[i]);

    if( index ) {
      memorySize += index->memorySize();
    } else {
      // account for the size of the DiskIndexes (~22M max)
      memorySize += lemur_compat::min<size_t>(4*((*state)[i]->documentCount()),indri::index::DiskIndex::MAX_DOCLENGTHS_CACHE);
    }
  }

  if( memorySize > 1.25*_memory ) {
    _repository._setThrashing( true );
    return HALF_SECOND;
  } else {
    _repository._setThrashing( false );
  }
  
  if( memorySize > _memory ) {
    return HALF_SECOND;
  } else {
    return FIVE_SECONDS;
  }
}

//
// hasWork
//

bool indri::collection::RepositoryLoadThread::hasWork() {
  return false;
}



