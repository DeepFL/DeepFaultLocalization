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

#ifndef INDRI_REPOSITORYLOADTHREAD_HPP
#define INDRI_REPOSITORYLOADTHREAD_HPP

#include "indri/UtilityThread.hpp"
namespace indri
{
  namespace collection
  {
    
    class RepositoryLoadThread : public indri::thread::UtilityThread {
    private:
      class Repository& _repository;
      UINT64 _memory;

    public:
      RepositoryLoadThread( class Repository& repository, UINT64 memory );

      UINT64 initialize();
      void deinitialize();
      UINT64 work();
      bool hasWork();
    };
  }
}


#endif // INDRI_REPOSITORYLOADTHREAD_HPP


