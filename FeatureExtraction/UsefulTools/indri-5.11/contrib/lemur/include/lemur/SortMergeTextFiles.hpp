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

#ifndef _SORTMERGETEXTFILES_HPP
#define _SORTMERGETEXTFILES_HPP

// class to sort and merge multiple text files into one

#include <time.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <list>

#include "indri/Buffer.hpp"
#include "indri/Path.hpp"
#include "indri/UtilityThread.hpp"
#include "lemur-compat.hpp"
#include "Exception.hpp"

namespace lemur {
  namespace file {

    /**
     * thread for merging two sorted files into one
     */
    class FileMergeThread : public indri::thread::UtilityThread {
    public:
      enum {
        MAX_INPUT_FILES=32,
        MAX_INPUT_LINESIZE=65536
      };

    private:
      FILE *inputFile[MAX_INPUT_FILES];
      std::ofstream outfile;

      std::string filePath[MAX_INPUT_FILES];
      std::string outputFilePath;

      char _buffer[MAX_INPUT_FILES][MAX_INPUT_LINESIZE];
      bool fileDone[MAX_INPUT_FILES];
      //      char _outputBuffer[lemur::file::FileMergeThread::MAX_INPUT_LINESIZE];
      char _outputBuffer[2*1024*1024];

      int numInputFiles;

      int recordCounter;
      bool isActive;

      int chooseNextBuffer();

    public:
      FileMergeThread(std::vector<std::string> &inputFileList, const std::string& outputFile);
      ~FileMergeThread() { }

      virtual bool hasWork() { return false; }
      virtual UINT64 work();
      virtual UINT64 initialize();
      virtual void deinitialize();

      int getRecordCounter() { return recordCounter; }
      bool isThreadActive() { return isActive; }

    };

    class SortMergeTextFiles {
    protected:
      std::string _outputFilePath;
      std::string _tempDirectory;

      indri::utility::Buffer _inputBuffer;
      indri::utility::Buffer _inputBufferTwo;

      bool _displayStatus;
      int _numMergeThreads;

      std::string _flushChunks(std::string& basePathname, std::vector<std::string> *inMemRecords, int currentChunkNumber);
      int _mergeSortTwoFiles(std::string &firstFilePath, std::string &secondFilePath, std::string &outputFile, bool doCleanup=true);
      std::vector<std::string> _doMidFinalMerge(std::vector<std::string> &inputList, std::string &outputPathBase, int &recordCounter);
      int _doFinalMergesortFiles(std::vector<std::string> &inputFiles, std::string &outputFile);

      virtual void _doSingleFileMergesort(std::string &inputFile, std::string &outputFile, std::vector<std::string> &chunkList, int chunkRecordSize=16384*10*10);

    public:
      SortMergeTextFiles(std::string &outputFilePath, std::string &tempDirectory, int numMergeThreads=4, bool displayStatus=false);
      ~SortMergeTextFiles();

      int sort(std::vector<std::string> &inputFilePaths);
      static bool _readLine(FILE *_in, char*& beginLine, size_t& lineLength, indri::utility::Buffer &_buffer);
      void showStatus(bool displayStatus) { _displayStatus=displayStatus; }

    };
  } // end namespace file
} // end namespace lemur

#endif // _SORTMERGETEXTFILES_HPP
