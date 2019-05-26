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

#include "HarvestSortMerge.hpp"

lemur::file::HarvestSortMerge::HarvestSortMerge(std::string &outputFilePath, std::string &tempDirectory,
                                                lemur::file::Keyfile *docNoKeyfile, int numMergeThreads,
                                                bool displayStatus) :
lemur::file::SortMergeTextFiles(outputFilePath, tempDirectory, numMergeThreads, displayStatus),
_docNoKeyfile(docNoKeyfile)
{
  assert(docNoKeyfile && "Doc No Keyfile is NULL");
}

lemur::file::HarvestSortMerge::~HarvestSortMerge() {
}


void lemur::file::HarvestSortMerge::_doSingleFileMergesort(std::string &inputFile, std::string &outputFile,
                                                           std::vector<std::string> &chunkList, int chunkRecordSize)
{
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

  std::vector<std::string> linePieces;

  int rejectionCount=0;
  int lineCount=0;

  while (_readLine(_in,  thisLine, lineLength, _inputBuffer)) {
    ++lineCount;
    if (currentChunkRecord==chunkRecordSize) {
      chunkList.push_back(_flushChunks(outputFile, &inMemoryChunk, chunkList.size()));
      inMemoryChunk.clear();
      currentChunkRecord=0;
    }

    // straight fill-up the buffer
    if ((lineLength > 0) && (thisLine)) {
      // split the URL/rest
      linePieces.clear();
      splitLineOnTabs(thisLine, linePieces);
      // see if our destination URL is in the keyfile
      // ensure we're not over the 511+\0 size limit!
      if (linePieces[0].length() > 511) {
		  char hashBuffer[128];
		  SHA1Hasher.hashStringToHex(linePieces[0].c_str(), hashBuffer, 128);
		  linePieces[0] = hashBuffer;
      }
      if ((linePieces.size() > 0) && (_docNoKeyfile->getSize(linePieces[0].c_str()) > -1)) {
        inMemoryChunk.push_back(std::string(thisLine));

        // increment our counters
        ++currentChunkRecord;
        ++countInputRecords;
      } else {
        ++rejectionCount;
      }
    }
  }

  if (currentChunkRecord > 0) {
    chunkList.push_back(_flushChunks(outputFile, &inMemoryChunk, chunkList.size()));
  }

  // close the input file, we're done with it
  fclose(_in);
}

void lemur::file::HarvestSortMerge::splitLineOnTabs(char *inputLine, std::vector<std::string> &retVec) {
  if ((!inputLine) || (!inputLine[0])) {
    return;
  }

  char *startPos=inputLine;
  while (*startPos) {
    char *currentPos=startPos;
    while (*currentPos && (*currentPos!='\t')) {
      ++currentPos;
    }
    if (*currentPos=='\t') {
      // break this chunk off
      char thisChar=(*currentPos);
      *currentPos=0;
      retVec.push_back(std::string(startPos));
      *currentPos=thisChar;
      ++currentPos;
    } else {
      // if there's data here, it's the last item - push it
      if (*startPos) {
        retVec.push_back(std::string(startPos));
      }
    }
    startPos=currentPos;
  }
}
