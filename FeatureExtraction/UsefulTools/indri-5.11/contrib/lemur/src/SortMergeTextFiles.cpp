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

#include "SortMergeTextFiles.hpp"
#include "indri/Path.hpp"
#include "Exception.hpp"
#include "indri/ScopedLock.hpp"

lemur::file::SortMergeTextFiles::SortMergeTextFiles(std::string &outputFilePath, std::string &tempDirectory, int numMergeThreads, bool displayStatus) :
_outputFilePath(outputFilePath), _tempDirectory(tempDirectory),
_displayStatus(displayStatus), _numMergeThreads(numMergeThreads)
{
}

lemur::file::SortMergeTextFiles::~SortMergeTextFiles() {
}

bool lemur::file::SortMergeTextFiles::_readLine(FILE *_in, char*& beginLine, size_t& lineLength, indri::utility::Buffer &_buffer ) {
  lineLength = 0;
  size_t actual;

  // make a buffer of a reasonable size so we're not always allocating
  if( _buffer.size() < 1024*1024 )
    _buffer.grow( 1024*1024 );
  // if we're running out of room, add 25MB
  if( (_buffer.size() -  _buffer.position()) < 512*1024 ) {
    _buffer.grow( _buffer.size() + 1024*1024*25 );
  }
  if( _buffer.position() ) {
    // get rid of null terminator from previous call
    _buffer.unwrite(1);
  }

  size_t readAmount = _buffer.size() - _buffer.position() - 2;

  // fetch next document line
  char* buffer = _buffer.write( readAmount );
  char* result = fgets( buffer, (int)readAmount, _in );

  if(!result) {
    return false;
  }

  actual = strlen(buffer);
  lineLength += actual;
  _buffer.unwrite( readAmount - actual );

  // all finished reading
  *_buffer.write(1) = 0;
  beginLine = _buffer.front() + _buffer.position() - lineLength - 1;

  // strip the \n off the end
  if (beginLine[lineLength-1]=='\n') {
    beginLine[lineLength-1]=0;
  }

  return true;
}

std::string lemur::file::SortMergeTextFiles::_flushChunks(std::string& basePathname, std::vector<std::string> *inMemRecords, int currentChunkNumber) {
  // sort the in-memory buffer
  std::sort(inMemRecords->begin(), inMemRecords->end());

  // write the chunk out
  std::stringstream chunkPathname;
  time_t rawtime;
  time(&rawtime);
  chunkPathname << basePathname << "." << (rawtime+clock()) << "_" << currentChunkNumber << ".mchunk";

  std::ofstream outfile(chunkPathname.str().c_str(), std::ios::out | std::ios::binary);
  for (std::vector<std::string>::iterator vIter=inMemRecords->begin(); vIter!=inMemRecords->end(); vIter++) {
    outfile << (*vIter).c_str() << "\n"; //std::endl;
  }
  outfile.flush();
  outfile.close();
  return chunkPathname.str();
}

void lemur::file::SortMergeTextFiles::_doSingleFileMergesort(std::string &inputFile, std::string &outputFile, std::vector<std::string> &chunkList, int chunkRecordSize) {
  // our in-memory chunks
  std::vector<std::string> inMemoryChunk;
  inMemoryChunk.reserve(chunkRecordSize);

  int currentChunkRecord=0;

  // clear the input buffer
  _inputBuffer.clear();

  FILE* _in;
  _in = fopen( inputFile.c_str(), "rb" );

  if( !_in ) {
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open file " + inputFile + "." );
  }

  // reset the buffer size
  //  setvbuf(_in, NULL, _IOFBF, 65536);
  setvbuf(_in, NULL, _IOFBF, 5*1024*1024);
  std::vector<std::string> outputChunks;

  int countInputRecords=0;

  char* thisLine;
  size_t lineLength;

  while (_readLine(_in,  thisLine, lineLength, _inputBuffer)) {
    if (currentChunkRecord==chunkRecordSize) {
      chunkList.push_back(_flushChunks(outputFile, &inMemoryChunk, chunkList.size()));
      inMemoryChunk.clear();
      currentChunkRecord=0;
    }

    // straight fill-up the buffer
    if ((lineLength > 0) && (thisLine)) {
      inMemoryChunk.push_back(std::string(thisLine));

      // increment our counters
      ++currentChunkRecord;
      ++countInputRecords;
    }
  }

  if (currentChunkRecord > 0) {
    chunkList.push_back(_flushChunks(outputFile, &inMemoryChunk, chunkList.size()));
  }

  // close the input file, we're done with it
  fclose(_in);

  // now, merge sort the chunks
  // _doFinalMergesortFiles(outputChunks, outputFile);
}

std::vector<std::string> lemur::file::SortMergeTextFiles::_doMidFinalMerge(std::vector<std::string> &inputList, std::string &outputPathBase, int &recordCounter) {
  if (inputList.size() < 2) {
    // nothing to do
    return inputList;
  }

  std::vector<std::string> retList;

  std::list<FileMergeThread*> threadPool;
  std::vector<std::string> currentWorkingFileList;

  std::vector<std::string>::iterator vIter=inputList.begin();
  int pCounter=0;

  while (vIter!=inputList.end()) {
    currentWorkingFileList.clear();

    // merge the next items (up to MAX_INPUT_FILES)
    for (int i=0; (i < lemur::file::FileMergeThread::MAX_INPUT_FILES) && (vIter!=inputList.end()); i++) {
      currentWorkingFileList.push_back(*vIter);
      ++pCounter;
      // std::cout << "added file: " << (*vIter) << " - number " << pCounter << " of " << inputList.size() << std::endl;
      vIter++;
    }

    std::stringstream outputFile;
    time_t rawtime;
    time(&rawtime);
    outputFile << outputPathBase << "." << (rawtime+clock()) << "_" << retList.size() << ".mchunk";

    // create our thread
    FileMergeThread* thisThread=new FileMergeThread(currentWorkingFileList, outputFile.str());

    // can we run, or do we have to wait for a spot in the pool?
    bool waitAnimation=true;
    while (threadPool.size() >= _numMergeThreads) {
      // see if we can clear any of the threads
      for (std::list<FileMergeThread*>::iterator tIter=threadPool.begin(); tIter!=threadPool.end(); tIter++) {
        if (!(*tIter)->isThreadActive()) {
          // this one is done - remove it
          recordCounter=(*tIter)->getRecordCounter();
          (*tIter)->join();
          delete (*tIter);
          threadPool.erase(tIter);
          break;
        }
        // (*tIter)->dumpWhichFiles();
      }

      // no? sleep for 500 ms and loop
      //indri::thread::Thread::sleep(250);
      indri::thread::Thread::yield();
    }

    // add our new thread to the pool
    threadPool.push_back(thisThread);

    // and start up the thread
    thisThread->start();

    // record our output file name
    retList.push_back(outputFile.str());
  }

  // wait for all threads to finish up...
  for (std::list<FileMergeThread*>::iterator tIter=threadPool.begin(); tIter!=threadPool.end(); tIter++) {
    while ((*tIter)->isThreadActive()) {
      indri::thread::Thread::sleep(250);
    }
    (*tIter)->join();
  }

  // cleanup our remaining threads
  for (std::list<FileMergeThread*>::iterator tIter=threadPool.begin(); tIter!=threadPool.end(); tIter++) {
    recordCounter=(*tIter)->getRecordCounter();
    delete (*tIter);
  }

  return retList;
}

int lemur::file::SortMergeTextFiles::_doFinalMergesortFiles(std::vector<std::string> &inputFiles, std::string &outputFile) {

  std::vector<std::string> currentInputList;
  if (inputFiles.size() > 32) {
    currentInputList.reserve(inputFiles.size());
  }

  // copy our input files as the current input list
  for (std::vector<std::string>::iterator vIter=inputFiles.begin(); vIter!=inputFiles.end(); vIter++) {
    currentInputList.push_back(*vIter);
  }

  int recordCounter=0;
  int passCounter=1;
  while (currentInputList.size() > 1) {
    if (_displayStatus) {
      std::cout << "merging pass " << passCounter << " - " << currentInputList.size() << " blocks                \r";
      std::cout.flush();
    }

    currentInputList=_doMidFinalMerge(currentInputList, outputFile, recordCounter);
    ++passCounter;
  }

  if (_displayStatus) {
    std::cout << std::endl;
  }

  // all done - if we have a file, delete the old and rename the new
  if (currentInputList.size() == 1) {
    if (indri::file::Path::exists(outputFile)) {
      remove(outputFile.c_str());
    }
    indri::file::Path::rename(currentInputList[0].c_str(), outputFile.c_str());
  }
  return recordCounter;
}

int lemur::file::SortMergeTextFiles::sort(std::vector<std::string> &inputFilePaths) {
  assert((_outputFilePath.length() > 0) && "SortMergeTextFiles: output file path not set");
  assert((_tempDirectory.length() > 0) && "SortMergeTextFiles: temp directory not set");

  // ensure the temp directory exists - if not create it
  if (!indri::file::Path::exists(_tempDirectory)) {
    indri::file::Path::make(_tempDirectory);
  }

  // ensure the directory for the output file exists - if not, create it
  std::string outputFileDirectory=indri::file::Path::directory(_outputFilePath);
  if (!indri::file::Path::exists(outputFileDirectory)) {
    indri::file::Path::make(outputFileDirectory);
  }

  int inputFileCount=inputFilePaths.size();
  if (_displayStatus) {
    std::cout << "-- sorting input link files" << std::endl;
  }
  int inputFileCounter=1;
  std::vector<std::string> tempMergeFilepaths;
  for (std::vector<std::string>::iterator inputFile=inputFilePaths.begin(); inputFile!=inputFilePaths.end(); inputFile++) {
    std::string thisInputFilename=indri::file::Path::filename(*inputFile);
    std::string thisTempFile=indri::file::Path::combine(_tempDirectory, thisInputFilename);
    if (_displayStatus) {
      std::cout << "-- sorting input file " << inputFileCounter << " of " << inputFileCount << "\r";
      std::cout.flush();
      ++inputFileCounter;
    }
    // std::vector<std::string> chunkList;
    _doSingleFileMergesort(*inputFile, thisTempFile, tempMergeFilepaths);
    // tempMergeFilepaths.push_back(thisTempFile);
  }

  if (_displayStatus) {
    std::cout << std::endl << "-- merging sorted input files" << std::endl;
  }

  int retVal=_doFinalMergesortFiles(tempMergeFilepaths, _outputFilePath);

  return retVal;
}

/*******************************************/
/* file merge thread functions             */
/*******************************************/

lemur::file::FileMergeThread::FileMergeThread(std::vector<std::string> &inputFileList, const std::string& outputFile )
{
  isActive=true;
  outputFilePath=outputFile;
  int totalInputFiles=inputFileList.size();
  numInputFiles=0;
  for (int i=0; i < MAX_INPUT_FILES; i++) {
    _buffer[i][0]=0;
    if (i < totalInputFiles) {
      filePath[i]=inputFileList[i];
      ++numInputFiles;
    } else {
      filePath[i]="";
    }
  }
}

int lemur::file::FileMergeThread::chooseNextBuffer() {
  int currentRetVal=-1;
  char *currentLeastValue=NULL;

  for (int i=0; i < numInputFiles; i++) {
    if (!fileDone[i]) {
      if (!currentLeastValue || (strcmp(currentLeastValue, _buffer[i]) > 0)) {
        currentLeastValue=_buffer[i];
        currentRetVal=i;
      }
    }
  }

  return currentRetVal;
}

UINT64 lemur::file::FileMergeThread::work() {

  for (int i=0; i < MAX_INPUT_FILES; i++) {
    fileDone[i]=false;
  }

  int recordCounter=0;
  int numFilesActive=numInputFiles;

  // preload the buffers with the first line
  for (int i=0; i < numInputFiles; i++) {
    if (filePath[i].length()==0 || !fgets(_buffer[i], MAX_INPUT_LINESIZE, inputFile[i])) {
      fileDone[i]=true;
      --numFilesActive;
    } else {
      // strip the \n off the end
      int lineLength=strlen(_buffer[i]);
      if (_buffer[i][lineLength-1]=='\n') {
        _buffer[i][lineLength-1]=0;
      }
    }
  }

  while (numFilesActive > 0) {

    if (!outfile.good()) {
      std::string isFail = (outfile.fail() ? "true" : "false");
      std::string isBad = (outfile.bad() ? "true" : "false");
      LEMUR_THROW( LEMUR_IO_ERROR, "Output file " + outputFilePath + " is not good: fail bit: " + isFail + " / bad bit: " + isBad );
    }


    int whichBuffer=chooseNextBuffer();
    if (whichBuffer > -1) {
      outfile << _buffer[whichBuffer] << "\n"; //std::endl;
      ++recordCounter;
      if (!fgets(_buffer[whichBuffer], MAX_INPUT_LINESIZE, inputFile[whichBuffer])) {
        fileDone[whichBuffer]=true;
        --numFilesActive;
      } else {
        // strip the \n off the end
        int lineLength=strlen(_buffer[whichBuffer]);
        if (_buffer[whichBuffer][lineLength-1]=='\n') {
          _buffer[whichBuffer][lineLength-1]=0;
        }
      }
    }
  }

  this->signal();
  return 0;
}

UINT64 lemur::file::FileMergeThread::initialize() {
  for (int i=0; i < numInputFiles; i++) {
    inputFile[i]=fopen(filePath[i].c_str(), "rb");
    if (!inputFile[i]) {
      LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open temp file " + filePath[i] + "." );
    }
    // reset the buffer size to 64k
    //    setvbuf(inputFile[i], NULL, _IOFBF, 5*1024*1024);
    setvbuf(inputFile[i], NULL, _IOFBF, 1*1024*1024);
  }
  outfile.open(outputFilePath.c_str(), std::ios::out | std::ios::binary);
  outfile.rdbuf()->pubsetbuf(this->_outputBuffer, sizeof(this->_outputBuffer));

  recordCounter=0;

  return 0;
}

void lemur::file::FileMergeThread::deinitialize() {
  outfile.flush();
  outfile.close();

  for (int i=0; i < numInputFiles; i++) {
    fclose(inputFile[i]);
    remove(filePath[i].c_str());
  }

  isActive=false;
}

