/*==========================================================================
 * Copyright (c) 2004-2008 Carnegie Mellon University and University of
 * Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/

#ifndef _HARVESTSORTMERGE_HPP
#define _HARVESTSORTMERGE_HPP

// class to sort and merge multiple text files into one
// with early rejection of URLs that do not exist within the keyfile

#include "SortMergeTextFiles.hpp"
#include "Keyfile.hpp"
#include "SHA1.hpp"

namespace lemur {
  namespace file {

    class HarvestSortMerge : public SortMergeTextFiles {
    protected:
      lemur::file::Keyfile *_docNoKeyfile;
	  lemur::utility::SHA1 SHA1Hasher;
      virtual void _doSingleFileMergesort(std::string &inputFile, std::string &outputFile, std::vector<std::string> &chunkList, int chunkRecordSize=16384*10);

    public:
      HarvestSortMerge(std::string &outputFilePath, std::string &tempDirectory, lemur::file::Keyfile *docNoKeyfile, int numMergeThreads=4, bool displayStatus=false);
      ~HarvestSortMerge();

      static void splitLineOnTabs(char *inputLine, std::vector<std::string> &retVec);

    }; // end class HarvestSortMerge
  } // end namespace file
} // end namespace lemur

#endif // _HARVESTSORTMERGE_HPP
