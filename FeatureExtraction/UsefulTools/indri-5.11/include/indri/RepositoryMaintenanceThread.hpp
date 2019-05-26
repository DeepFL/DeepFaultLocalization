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

#ifndef INDRI_REPOSITORYMAINTENANCETHREAD_HPP
#define INDRI_REPOSITORYMAINTENANCETHREAD_HPP

#include "indri/UtilityThread.hpp"
#include <queue>
namespace indri
{
  namespace collection
  {
    
    class RepositoryMaintenanceThread : public indri::thread::UtilityThread {
    private:
      enum { WRITE, MERGE, TRIM };
      class Repository& _repository;

      indri::thread::Mutex _requestLock;
      std::queue<int> _requests;
      UINT64 _memory;

    public:
      RepositoryMaintenanceThread( class Repository& repository, UINT64 memory );

      UINT64 initialize();
      UINT64 work();
      bool hasWork();
      void deinitialize();

      void write();
      void merge();
    };
  }
}

#endif // INDRI_REPOSITORYMAINTENANCETHREAD_HPP

