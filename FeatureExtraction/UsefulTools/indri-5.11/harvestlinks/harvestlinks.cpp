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

/*! \page HarvestLinks Harvestlinks Utility

<p>
The HarvestLinks application extracts all links (and link text) from a collection
of web pages. It can be used to gather anchor text and in-links for HTML and TREC Web data. The default file class is trecweb. To process WARC files, such as those distributed with ClueWeb09, use the optional parameter <b>class</b>.
This in turn can be added to an index in the form of &quot;inlink&quot; fields for use for
direct retrieval or for page-rank calculations.
</p>

<p>
The two required parameters for the harvestlinks application are:
<ul>
<li><b>corpus</b>: The path to the directory holding the corpus files you're trying to index</li>
<li><b>output</b>: The path to a directory where the link harvesting output should go</li>
</ul>

<p>
For example, running this from the command line might look like:
<pre>
  ./harvestlinks -corpus=/path/to/corpus -output=/path/to/output
</pre>
</p>

<p>
Once you have gathered your links, you must tell the indexer to index them along with your
source data. In your index parameter file, you should add the following to your &lt;corpus&gt; parameter set:<br />
<pre>
  &lt;inlink&gt;/path/to/output/sorted&lt;/inlink&gt;
</pre>
<br />
(where the &quot;sorted&quot; directory is the directory named &quot;sorted&quot; under the output
directory for harvestlinks). And also, so that the indexer knows about the inlink fields:<br />
<pre>
  &lt;field&gt;&lt;name&gt;inlink&lt;/name&gt;&lt;/field&gt;
</pre>
<br />
This will allow you to perform retrieval tasks on the anchor text.
</p>

<H3>Harvestlinks Parameters</H3>
<ul>
<li>
  <b>corpus: (<i>required</i>)</b> The path to the directory holding the
  corpus files you're trying to index
</li>
<li>
  <b>class: (<i>optional</i>)</b> The file class of the corpus. One of trecweb (the default) or warc.
</li>
<li>
  <b>output: (<i>required</i>)</b> The path to a directory where the link
  harvesting output should go
</li>
<li>
  <b>redirect</b>: specifies a redirect file that maps from source
  to target URLs to create aliases for links. The redirect file
  is a text file with one entry per line in the form of:<br />
  &nbsp;&nbsp;[SOURCE_URL] [TARGET_URL]<br />
  Where the source URL is the original URL to be found and the 
  target URL will be what is searched for instead of the original
  source URL.
</li>
<li>
  <b>mergethreads</b>: specified the number of threads to use for
  the file sort and merge operations (default 4, recommended less
  than 8 max.)
</li>
<li>
  <b>delete</b>: set to false to not delete any existing directories
  in the output directory (default true: do delete)
</li>
<li>
  <b>harvest</b>: perform the harvesting step (default true, set to false to skip)
</li>
<li>
  <b>sort</b>: perform the sorting/merge step (default true, set to false to skip)
</li>
<li>
  <b>clean</b>: perform cleaning of temporary files after sort (default true, set to false to skip)
</li>
<li>
  <b>combine</b>: perfom final combination of links (default true, set to false to skip)
</li>

</ul>

*/


#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include "indri/Parameters.hpp"

#include "indri/TokenizedDocument.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/TaggedDocumentIterator.hpp"
#include "indri/WARCDocumentIterator.hpp"
#include "indri/TaggedTextParser.hpp"
#include "indri/HTMLParser.hpp"
#include "indri/TokenizerFactory.hpp"
#include "indri/ConflationPattern.hpp"
#include "indri/AnchorTextHarvester.hpp"
#include "indri/FileTreeIterator.hpp"
#include "indri/Path.hpp"
#include "lemur/Exception.hpp"
#include "indri/Combiner.hpp"
#include "lemur/Keyfile.hpp"
#include "lemur/HarvestSortMerge.hpp"
#include "indri/IndriTimer.hpp"
#include "lemur/SHA1.hpp"

std::vector<std::string> harvestedLinkPaths;
static indri::utility::IndriTimer g_timer;
static lemur::utility::SHA1 SHA1Hasher;

std::string getFinalHarvestPath(const std::string &corpusPath, const std::string &filePath, const std::string &harvestPath) {
  std::string workingPath=indri::file::Path::directory(filePath);
  std::string::size_type corpusLoc=workingPath.find_first_of(corpusPath);
  if (corpusLoc!=0) {
    return harvestPath;
  }

  workingPath=filePath.substr(corpusPath.length()+1);

  if (workingPath.length()==0) {
    return harvestPath;
  }
  return indri::file::Path::combine(harvestPath, workingPath);
}

static lemur::file::Keyfile *createRedirectKeyfile(const std::string& redirectKeyfilePath, 
                                                   const std::string& redirectPath) 
{
  if (redirectPath.length()==0) { return NULL; }
  lemur::file::Keyfile *keyfile=new lemur::file::Keyfile();
  if (keyfile) {
    keyfile->create(redirectKeyfilePath.c_str());

    // open our redirect file path
    FILE *_in=fopen(redirectPath.c_str(), "rb");
    if (_in) {
      char *currentLine;
      size_t currentLineLen;
      indri::utility::Buffer docBuffer;

      while (lemur::file::SortMergeTextFiles::_readLine(_in, currentLine, currentLineLen, docBuffer)) {
        if (currentLine) {
          char *space=strchr(currentLine, ' ');
          if (!space) { continue; }
          *space=0;
          std::string source=currentLine;
          std::string target=(space+1);
          if (source.length() > 511) {
            char hashBuffer[128];
            SHA1Hasher.hashStringToHex(source.c_str(), hashBuffer, 128);
            source=hashBuffer;
          }
          if (target.length() > 511) {
            char hashBuffer[128];
            SHA1Hasher.hashStringToHex(target.c_str(), hashBuffer, 128);
            target=hashBuffer;
          }
          if (source.length() > 0 && target.length() > 0) {
            keyfile->put(source.c_str(), target.c_str(), (target.length() + 1));
          }
        }
      }

      fclose(_in);
    }

    keyfile->close();

    keyfile->openRead(redirectKeyfilePath.c_str());
  }
  return keyfile;
}

static void harvest_anchor_text_file( const std::string& path,
                                      const std::string& linkFilePath,
                                      const std::string& docOrderPath,
                                      lemur::file::Keyfile *redirectKeyfile,
                                      indri::parse::HTMLParser& parser,
                                      indri::parse::Tokenizer *tokenizer,
                                      lemur::file::Keyfile *keyfile,
                                      const std::string& fileClass)
{
  indri::parse::DocumentIterator *iterator = 0;
  
  if (fileClass == "trecweb") {
    
    iterator = new indri::parse::TaggedDocumentIterator();
    ((indri::parse::TaggedDocumentIterator *)iterator)->setTags(
                                                              "<DOC>",       // startDocTag
                                                              "</DOC>",    // endDocTag
                                                              "</DOCHDR>"    // endMetadataTag
                                                              );
  } else if (fileClass == "warc") {
    iterator = new  indri::parse::WARCDocumentIterator();
  } else {
    LEMUR_THROW( LEMUR_BAD_PARAMETER_ERROR, "Unrecognized class paramter: " + 
                 fileClass);
  }
  
  
  iterator->open( path );  
  indri::parse::UnparsedDocument *unparsed=NULL;
  indri::parse::TokenizedDocument* tokenized;
  indri::parse::AnchorTextHarvester writer(linkFilePath, docOrderPath, keyfile, redirectKeyfile);

  while( (unparsed = iterator->nextDocument()) != 0 ) {
    tokenized = tokenizer->tokenize( unparsed );
    indri::api::ParsedDocument* parsed = parser.parse( tokenized );
    writer.handle(parsed);
  }

  // close up everything
  iterator->close();
  delete iterator;
  harvestedLinkPaths.push_back(linkFilePath);
}

static void harvest_anchor_text( const std::string& corpusPath,
                                 const std::string& fileClass,
                                 const std::string& harvestPath,
                                 const std::string& docUrlNoKeyfilePath,
                                 const std::string& preSortPath,
                                 const std::string& redirectPath
                               )
{
  std::vector<std::string> include;
  include.push_back( "absolute-url" );
  include.push_back( "relative-url" );
  include.push_back( "a" );
  std::vector<std::string> empty;
  std::map<indri::parse::ConflationPattern*,std::string> mempty;

  indri::parse::HTMLParser parser;
  indri::parse::Tokenizer* tokenizer = indri::parse::TokenizerFactory::get( "word" );
  parser.setTags( empty, empty, include, empty, mempty );

  // create our keyfile for the docurls
  lemur::file::Keyfile docUrlNoKeyfile;
  docUrlNoKeyfile.create(docUrlNoKeyfilePath.c_str(), (20*1024*1024));

  // do we have a redirect path we need to read in 
  // and create a keyfile for?
  std::string redirectKeyfilePath=indri::file::Path::combine(preSortPath, "redirect.key");

  lemur::file::Keyfile *redirectKeyfile=NULL;
  if (redirectPath.length() > 0) {
    redirectKeyfile=createRedirectKeyfile(redirectKeyfilePath, redirectPath);
  }

  if( indri::file::Path::isDirectory( corpusPath ) ) {
    indri::file::FileTreeIterator files( corpusPath );

    int fCounter=1;

    for( ; files != indri::file::FileTreeIterator::end(); files++ ) {
      std::string filePath = *files;
      std::cout << "harvesting " << filePath << std::endl;

      std::string finalHarvestPath=getFinalHarvestPath(corpusPath, filePath, harvestPath);
      std::string finalHarvestDirectory=indri::file::Path::directory(finalHarvestPath);
      if (!indri::file::Path::isDirectory( finalHarvestDirectory )) {
        indri::file::Path::make(finalHarvestDirectory);
      }

      std::string docOrderFilename("docOrder-");
      std::string linkFileFilename("linkfile-");
      docOrderFilename += indri::file::Path::filename(filePath);
      linkFileFilename += indri::file::Path::filename(filePath);

      std::string linkFilePath = indri::file::Path::combine( finalHarvestDirectory, linkFileFilename );
      std::string docOrderPath = indri::file::Path::combine( finalHarvestDirectory, docOrderFilename );
      try {
        // now harvest the anchor text in the file
        harvest_anchor_text_file( *files, linkFilePath, docOrderPath, redirectKeyfile, parser, tokenizer, &docUrlNoKeyfile, fileClass );
      } catch( lemur::api::Exception& e ) {
        std::cout << e.what() << std::endl;
      }
      ++fCounter;
    }

    std::cout << std::endl;

  } else {
    std::string docOrderFilename("docOrder-");
    std::string linkFileFilename("linkfile-");
    docOrderFilename += indri::file::Path::filename(corpusPath);
    linkFileFilename += indri::file::Path::filename(corpusPath);
    std::string linkFilePath = indri::file::Path::combine( harvestPath, linkFileFilename );
    std::string docOrderPath = indri::file::Path::combine( harvestPath, docOrderFilename );

    harvest_anchor_text_file( corpusPath, linkFilePath, docOrderPath, redirectKeyfile, parser, tokenizer, &docUrlNoKeyfile, fileClass );
  }
  // close the doc keyfile and cleanup
  docUrlNoKeyfile.close();
  if (tokenizer) { delete tokenizer; }
  if (redirectKeyfile) {
    redirectKeyfile->close();
    delete redirectKeyfile;
  }
}

static void collect_harvest_paths( const std::string& corpusPath,
                                 const std::string& fileClass,
                                 const std::string& harvestPath,
                                 const std::string& docUrlNoKeyfilePath,
                                 const std::string& preSortPath,
                                 const std::string& redirectPath
                                   ) 
{
  if( indri::file::Path::isDirectory( corpusPath ) ) {
    indri::file::FileTreeIterator files( corpusPath );
    for( ; files != indri::file::FileTreeIterator::end(); files++ ) {
      std::string filePath = *files;
      std::string finalHarvestPath=getFinalHarvestPath(corpusPath, filePath, harvestPath);
      std::string finalHarvestDirectory=indri::file::Path::directory(finalHarvestPath);
      std::string linkFileFilename("linkfile-");
      linkFileFilename += indri::file::Path::filename(filePath);
      std::string linkFilePath = indri::file::Path::combine( finalHarvestDirectory, linkFileFilename );
      harvestedLinkPaths.push_back(linkFilePath);
    }
  } else {
    std::string linkFileFilename("linkfile-");
    linkFileFilename += indri::file::Path::filename(corpusPath);
    std::string linkFilePath = indri::file::Path::combine( harvestPath, linkFileFilename );
    harvestedLinkPaths.push_back(linkFilePath);
  }
}

//can't stack allocate this buffer on Win XP without modifying the default stack
// size parameter
char _outputBuffer[5*1024*1024];

void combineOutputFile(const std::string& corpusFile, const std::string& sortedPath,
                       const std::string& outputSortedLinkFile, const std::string& docOrderPath,
                       lemur::file::Keyfile *urlKeyfile, FILE *sortedDestFile,
                       lemur::file::Keyfile *docNoKeyfile)
{

  if (sortedDestFile==NULL) {
    std::cerr << "NULL sorted destination file specified for combineOutputFile. Exiting." << std::endl;
    return;
  }

  std::cout << "-- combining " << indri::file::Path::filename(corpusFile) << std::endl;

  // reset the file pointer...
  rewind(sortedDestFile);

  // start our output file
  std::string outputFilePath=indri::file::Path::combine(sortedPath, indri::file::Path::filename(corpusFile));
  std::ofstream outfile(outputFilePath.c_str(),std::ios::out | std::ios::binary);
  outfile.rdbuf()->pubsetbuf(_outputBuffer, sizeof(_outputBuffer));
  // open the docOrder file
  FILE *docOrder=fopen(docOrderPath.c_str(), "rb");

  if (docOrder==NULL) {
    std::cerr << "Error opening document order file '" << docOrderPath << "'. File cannot be opened." << std::endl;
    return;
  }

  // reset the buffer size to 3MB
  setvbuf(docOrder, NULL, _IOFBF, 3*1024*1024);

  char *currentDocURL;
  size_t currentDocURLLen;
  indri::utility::Buffer docBuffer;

  char *currentSortedLine;
  size_t currentSortedLineSize;
  indri::utility::Buffer sortedBuffer;

  std::vector<std::string> matchedURLs;
  std::vector<std::string> matchedAnchorText;
  std::vector<std::string> matchedDocNos;

  // reserve some allocated space
  matchedURLs.reserve(1024);
  matchedAnchorText.reserve(1024);
  matchedDocNos.reserve(1024);

  std::vector<std::string> splitLine;
  std::vector<std::string> thisDestUrlAndDocno;

  while (lemur::file::SortMergeTextFiles::_readLine(docOrder, currentDocURL, currentDocURLLen, docBuffer)) {
    // split on the tab into destinationURL -> docno
    thisDestUrlAndDocno.clear();
    lemur::file::HarvestSortMerge::splitLineOnTabs(currentDocURL, thisDestUrlAndDocno);
    if (thisDestUrlAndDocno.size() == 2) {
      // get dest URL from doc file
      std::string thisDestURL=thisDestUrlAndDocno[0];
      std::string thisDestDocNo=thisDestUrlAndDocno[1];

      std::string finalDestURL(thisDestURL);

      // need a SHA1 / Hex hash on thisDestURL
      if (thisDestURL.length() > 511) {
        char hashBuffer[128];
        SHA1Hasher.hashStringToHex(thisDestURL.c_str(), hashBuffer, 128);
        finalDestURL=hashBuffer;
      }

      // get dest URL from link files
      //long linkFilePosStart; // need to use off_t, fseeko (_fseeki64)
#ifdef WIN32
#if MSC_VER >= 1400 /* VS2003, 32-bit only */
	  _int64 linkFilePosStart;
#else
	  long linkFilePosStart;
#endif
#else
	  off_t linkFilePosStart;
#endif
      int linkFilePosStartSize;
      if (urlKeyfile->get(finalDestURL.c_str(), &linkFilePosStart, linkFilePosStartSize, sizeof(linkFilePosStart))) {
        // it's here -

        // cleanup from any last items
        matchedURLs.clear();
        matchedDocNos.clear();
        matchedAnchorText.clear();

        // set our sorted file pointer
#ifdef WIN32
#if MSC_VER >= 1400
        _fseeki64(sortedDestFile, linkFilePosStart, SEEK_SET);
#else
		fseek(sortedDestFile, linkFilePosStart, SEEK_SET);
#endif
#else
		fseeko(sortedDestFile, linkFilePosStart, SEEK_SET);
#endif
        // process until URLs do not match
        // lines are in destURL->srcURL->anchor_text order
        bool keepReading=true;
        while (keepReading && lemur::file::SortMergeTextFiles::_readLine(sortedDestFile, currentSortedLine, currentSortedLineSize, sortedBuffer)) {
          splitLine.clear();
          lemur::file::HarvestSortMerge::splitLineOnTabs(currentSortedLine, splitLine);
          if (splitLine.size()!=3) {
            keepReading=false;
          } else {
            if (thisDestURL.compare(splitLine[0])) {
              // urls don't match - we're done here
              keepReading=false;
            } else {
              // urls match - get the docno for the source - if we have it
              char *sourceDocNo;
              int sourceDocNoSize;

              // need a SHA1 / Hex has on splitLine[1]
              std::string finalSplitLineOne(splitLine[1]);
              if ((finalSplitLineOne.length() > 0) && (splitLine[2].length() > 0)) {
                if (finalSplitLineOne.length() > 511) {
                  char hashBuffer[128];
                  SHA1Hasher.hashStringToHex(splitLine[1].c_str(), hashBuffer, 128);
                  finalSplitLineOne=hashBuffer;
                }

                if (docNoKeyfile->get(finalSplitLineOne.c_str(), &sourceDocNo, sourceDocNoSize)) {
                  // yep- we've got it - add these items
                  matchedURLs.push_back(splitLine[1]);
                  matchedAnchorText.push_back(splitLine[2]);
                  matchedDocNos.push_back(std::string(sourceDocNo));
                  if (sourceDocNo) { delete[](sourceDocNo); }
                } // end if (docNoKeyfile->get(splitLine[1].c_str(), sourceDocNo, sourceDocNoSize))
              } // if (finalSplitLineOne.length() > 0)
            } // end if (thisDestURL.compare(splitLine[0]))
          } // end if (splitLine.size()!=3)
          sortedBuffer.clear();
        } // end while (keepReading && lemur::file::SortMergeTextFiles::_readLine(sortedDestFile, ...

        // ok - we should have all matches (if any) - let's output them...
        // output file will be doc no -> doc URL -> array of (src url, anchor text)
        int numLinks=matchedURLs.size();
        if (numLinks > 0) {
          outfile << "DOCNO=" << thisDestDocNo.c_str() << "\n"; //std::endl;
          outfile << thisDestURL.c_str() << "\n"; //std::endl;
          outfile << "LINKS=" << numLinks << "\n"; //std::endl;
          for (int i=0; i < numLinks; i++) {
            outfile << "LINKDOCNO=" << matchedDocNos[i].c_str() << "\n"; //std::endl;
            outfile << "LINKFROM=" << matchedURLs[i].c_str() << "\n"; //std::endl;
            outfile << "TEXT=\"" << matchedAnchorText[i].c_str() << "\"" << "\n"; //std::endl;
          } // end for (int i=0; i < numLinks; i++)
        } // end if (numLinks > 0)
      } // end if (urlKeyfile->get(thisDestURL.c_str(), &linkFilePosStart ...
    } // end if (thisDestUrlAndDocno.size() == 2)
  } // end while (lemur::file::SortMergeTextFiles::_readLine(docOrder ...

  fclose(docOrder);
  outfile.close();
}

void combineSortedFiles(const std::string& corpusPath, const std::string& harvestPath,
                        const std::string& outputSortedLinkFile, const std::string& preSortPath,
                        const std::string& sortedPath, lemur::file::Keyfile *docNoKeyfile
                        ) {

  // scan through sorted link file and build keyfile of URL->fileposition
  // create a temp key file
  std::string keyfileTempPath=indri::file::Path::combine(preSortPath, "linkFilePos.key");
  lemur::file::Keyfile urlKeyfile;
  // 20 MB buffer
  urlKeyfile.create(keyfileTempPath.c_str(), (20*1024*1024));

  FILE *_sortIn;
  _sortIn=fopen(outputSortedLinkFile.c_str(), "rb");
  // ensure we have a sorted file!
  if (_sortIn==NULL) {
    std::cerr << "Error opening sorted link file '" << outputSortedLinkFile << "'. File cannot be opened." << std::endl;
    return;
  }
  // reset the buffer size to 64k
  //  setvbuf(_sortIn, NULL, _IOFBF, 65536);
  setvbuf(_sortIn, NULL, _IOFBF, 5*1024*1024);
  
  indri::utility::Buffer lineBuffer;
  char lastString[lemur::file::FileMergeThread::MAX_INPUT_LINESIZE];
  char *currentLine;
  size_t currentLineLen;
#ifdef WIN32
#if MSC_VER >= 1400
  _int64 lastFilePos=0;
#else
  long lastFilePos=0;
#endif
#else
  off_t  lastFilePos=0;
#endif
  lastString[0]=0;

  std::cout << "-- gathering destination link positions..." << std::endl;

  long linkCounter=0;
  long lineCounter=0;
  while (lemur::file::SortMergeTextFiles::_readLine(_sortIn, currentLine, currentLineLen, lineBuffer)) {
    // get the dest URL (up to the first \t)
    char *currentChar=currentLine;
    while ((*currentChar) && (*currentChar!='\t')) {
      ++currentChar;
    }
    if ((*currentChar)=='\t') {
      (*currentChar)=0;
    }
    if (strcmp(currentLine, lastString)) {
      try {
        
      // they differ - we're starting a new destination URL
      if (strlen(currentLine) > 511) {
        // need a SHA1 / Hex hash on currentLine - and we can remove the 511 max restriction
        char currentLineHash[128];
        SHA1Hasher.hashStringToHex((const char*)currentLine, currentLineHash, 128);
        urlKeyfile.put(currentLineHash, &lastFilePos, sizeof(lastFilePos));
      } else {
        urlKeyfile.put(currentLine, &lastFilePos, sizeof(lastFilePos));
      }
      strncpy(lastString, currentLine, lemur::file::FileMergeThread::MAX_INPUT_LINESIZE-1);
      lastString[lemur::file::FileMergeThread::MAX_INPUT_LINESIZE-1]=0;
      ++linkCounter;
      } catch ( lemur::api::Exception& e ) {
        std::cerr << "BAD KEY: previous: " << lastString << " current: " << currentLine << std::endl;
      }
      
      lineBuffer.clear();
      if (!(linkCounter % 5000)) {
        cout << "-- found " << linkCounter << " unique links...\r";
        cout.flush();
      }
    }
#ifdef WIN32
#if MSC_VER >= 1400 /*VS 2003 -- 32-bit only files */
    lastFilePos=_ftelli64(_sortIn); // need to use ftello (_ftelli64)
#else
	lastFilePos=ftell(_sortIn);
#endif
#else
    lastFilePos=ftello(_sortIn); // need to use ftello (_ftelli64)
#endif
  }
  cout << "-- found " << linkCounter << " unique links..." << std::endl;

  // for each item we have in the corpus...
  if( indri::file::Path::isDirectory( corpusPath ) ) {
    indri::file::FileTreeIterator files( corpusPath );

    for( ; files != indri::file::FileTreeIterator::end(); files++ ) {
      std::string filePath = *files;

      std::string finalHarvestPath=getFinalHarvestPath(corpusPath, filePath, harvestPath);
      std::string finalHarvestDirectory=indri::file::Path::directory(finalHarvestPath);
      if (!indri::file::Path::isDirectory( finalHarvestDirectory )) {
        indri::file::Path::make(finalHarvestDirectory);
      }

      std::string finalSortedPath=getFinalHarvestPath(corpusPath, filePath, sortedPath);
      std::string finalSortedDirectory=indri::file::Path::directory(finalSortedPath);
      if (!indri::file::Path::isDirectory( finalSortedDirectory )) {
        indri::file::Path::make(finalSortedDirectory);
      }


      std::string docOrderFilename("docOrder-");
      docOrderFilename += indri::file::Path::filename(filePath);
      std::string docOrderPath = indri::file::Path::combine( finalHarvestDirectory, docOrderFilename );

      try {
        combineOutputFile(*files, finalSortedDirectory, outputSortedLinkFile, docOrderPath, &urlKeyfile, _sortIn, docNoKeyfile);
      } catch( lemur::api::Exception& e ) {
        std::cout << e.what() << std::endl;
      }
    }
  } else {
    std::string docOrderFilename("docOrder-");
    docOrderFilename += indri::file::Path::filename(corpusPath);
    std::string docOrderPath = indri::file::Path::combine( harvestPath, docOrderFilename );

    combineOutputFile(corpusPath, sortedPath, outputSortedLinkFile, docOrderPath, &urlKeyfile, _sortIn, docNoKeyfile);
  }

  // we're done here.
  fclose(_sortIn);
  urlKeyfile.close();
}

void usage() {
  std::cerr << "Usage: harvestlinks -corpus=<path_to_corpus> -output=<output_dir> [options]\n" << std::endl;
  std::cerr << "Extracts links from a TrecWeb or a HTML corpus for use with indexing into\n";
  std::cerr << "an Indri index.\n" << std::endl;
  std::cerr << "== Required Parameters ==\n";
  std::cerr << "  -corpus=<path_to_corpus> | path to the corpus of trecweb or html files\n";
  std::cerr << "  -output=<output_dir> | output directory\n" << std::endl;
  std::cerr << "== Optional Parameters ==\n";
  std::cerr << "  -redirect=<path_to_redirect_file> | path to an optional redirect file\n";
  std::cerr << "            that specifies how to conflate target URLs\n";
  std::cerr << "  -mergethreads=<#_threads> | number of threads to use while merging files\n";
  std::cerr << "                (default is 4, recommended less than 8)\n";
  std::cerr << "  -delete=(0|1) | tells the program to delete existing output files (1=yes)\n";
  std::cerr << "  -harvest=(0|1) | tells the program to perform the harvesting step (1=yes)\n";
  std::cerr << "  -sort=(0|1) | tells the program to perform the sorting step (1=yes)\n";
  std::cerr << "  -clean=(0|1) | tells the program to clean up after sorting (1=yes)\n";
  std::cerr << "  -combine=(0|1) | tells the program to do the final combine (1=yes)\n" << std::endl;
}

int main(int argc, char * argv[]) {

  try {
    indri::api::Parameters& parameters = indri::api::Parameters::instance();
    parameters.loadCommandLine( argc, argv );
    // check for required parameters
    if (! parameters.exists("corpus") || ! parameters.exists("output")) {
      usage();
      exit (1);
    }
    
    if( parameters.get( "version", false ) ) {
      std::cout << INDRI_DISTRIBUTION << std::endl;
    }

    std::string corpusPath = parameters["corpus"];
    std::string fileClass = parameters.get("class", "trecweb");
    std::string outputPath = parameters["output"];
    std::string redirectPath = parameters.get("redirect", "");

    int numMergeThreads=parameters.get("mergethreads", 4);

    // setup our paths for temporary storage + our final sorted output
    std::string harvestPath = indri::file::Path::combine( outputPath, "harvest" );
    std::string preSortPath = indri::file::Path::combine( outputPath, "presort" );
    std::string sortedPath = indri::file::Path::combine( outputPath, "sorted" );

    if( parameters.get( "delete", 1 ) ) {
      if( indri::file::Path::isDirectory( harvestPath ) )
        indri::file::Path::remove( harvestPath );
      if( indri::file::Path::isDirectory( preSortPath ) )
        indri::file::Path::remove( preSortPath );
      if( indri::file::Path::isDirectory( sortedPath ) )
        indri::file::Path::remove( sortedPath );
      if( ! indri::file::Path::exists( outputPath )  ) {
        lemur_compat::mkdir( outputPath.c_str(), 0755 );
      }
      indri::file::Path::make( harvestPath );
      indri::file::Path::make( preSortPath );
      indri::file::Path::make( sortedPath );
    }
    
    if( ! indri::file::Path::exists( outputPath )  ) {
      lemur_compat::mkdir( outputPath.c_str(), 0755 );
	}
	if ( ! indri::file::Path::exists( harvestPath ) ) {
      indri::file::Path::make( harvestPath );
	}
	if ( ! indri::file::Path::exists( preSortPath) ) {
      indri::file::Path::make( preSortPath );
	}
	if ( ! indri::file::Path::exists( sortedPath ) ) {
      indri::file::Path::make( sortedPath );
    }

    std::string docUrlNoKeyfilePath=indri::file::Path::combine(preSortPath, "docUrlNo.key");

    g_timer.start();

    // step 1: harvest text
    if( parameters.get( "harvest", true ) ) {
      g_timer.printElapsedSeconds(std::cout);
      std::cout << " Phase 1: Harvesting anchor URLs and text..." << std::endl;
      harvest_anchor_text( corpusPath, fileClass, harvestPath, docUrlNoKeyfilePath, preSortPath, redirectPath);
    }

    // re-open the keyfile for the document URLs - read only
    lemur::file::Keyfile docUrlNoKeyfile;
    std::string tempDirectory=indri::file::Path::combine(harvestPath, "tmp");
    std::string outputSortedLinkFile=indri::file::Path::combine(harvestPath, "linkFile.sorted");

    if ( parameters.get( "sort", true ) ) {
      if (harvestedLinkPaths.size() == 0)
		  collect_harvest_paths( corpusPath, fileClass, harvestPath, docUrlNoKeyfilePath, preSortPath, redirectPath);
      docUrlNoKeyfile.openRead(docUrlNoKeyfilePath.c_str(), (20*1024*1024));

      // step 2: combine and sort our (destURL->srcURL->anchorText) files
      g_timer.printElapsedSeconds(std::cout);
      std::cout << " Phase 2: Sorting harvested files..." << std::endl;

      indri::file::Path::make(tempDirectory);
      // create our sorting object (with display option on)
      lemur::file::HarvestSortMerge sortLinkFiles(outputSortedLinkFile, tempDirectory, &docUrlNoKeyfile, numMergeThreads, true);
      sortLinkFiles.sort(harvestedLinkPaths);
      docUrlNoKeyfile.close();
    }

    if ( parameters.get( "clean", true ) ) {
      g_timer.printElapsedSeconds(std::cout);
      // delete the temp directory
      std::cout << " Phase 3: intermediate cleanup..." << std::endl;
      indri::file::Path::remove(tempDirectory);

      // cleanup the old files
      for (std::vector<std::string>::iterator fIter=harvestedLinkPaths.begin(); fIter!=harvestedLinkPaths.end(); fIter++) {
        remove((*fIter).c_str());
      }
    }

    // step 3: combine link file with doc file

    if (parameters.get( "combine", true ) ) {
      g_timer.printElapsedSeconds(std::cout);
      std::cout << " Phase 4: Combining harvested links to final output..." << std::endl;

      docUrlNoKeyfile.openRead(docUrlNoKeyfilePath.c_str(), (20*1024*1024));
      combineSortedFiles(corpusPath, harvestPath, outputSortedLinkFile, preSortPath, sortedPath, &docUrlNoKeyfile);
      std::cout << std::endl;

      docUrlNoKeyfile.close();
    }

    g_timer.printElapsedSeconds(std::cout);
    std::cout << " Harvestlinks Complete." << std::endl;
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  }

  return 0;
}


