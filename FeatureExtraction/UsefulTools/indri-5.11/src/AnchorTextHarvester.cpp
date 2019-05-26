#include "indri/AnchorTextHarvester.hpp"

#include <vector>
#include <list>
#include <map>
#include <sstream>

using namespace indri::parse;
using namespace indri::api;

AnchorTextHarvester::AnchorTextHarvester( const std::string &linkFilePath, 
                                          const std::string& docOrderPath, 
                                          lemur::file::Keyfile *docNoKeyfile,
                                          lemur::file::Keyfile *redirectKeyfile ) 
{
  assert(docNoKeyfile && "docNoKeyfile is null");
  assert( linkFilePath.length() && "linkFile path cannot be empty");

  _docNoKeyfile=docNoKeyfile;
  _redirectKeyfile=redirectKeyfile;
  _linkFile.open(linkFilePath.c_str(), std::ios::out | std::ios::binary);
  _docOrder.open(docOrderPath.c_str(), std::ios::out | std::ios::binary);

  //  _linkFile.rdbuf()->pubsetbuf(this->linkFileOutBuffer, lemur::file::FileMergeThread::MAX_INPUT_LINESIZE);
  //  _docOrder.rdbuf()->pubsetbuf(this->docOrderOutBuffer, lemur::file::FileMergeThread::MAX_INPUT_LINESIZE);
  _linkFile.rdbuf()->pubsetbuf(this->linkFileOutBuffer, 5*1024*1024);
  _docOrder.rdbuf()->pubsetbuf(this->docOrderOutBuffer, 3*1024*1024);
}

AnchorTextHarvester::~AnchorTextHarvester() {
  _linkFile.close();
  _docOrder.close();
}

void AnchorTextHarvester::handle( indri::api::ParsedDocument* document ) {
  indri::utility::greedy_vector<MetadataPair>::iterator iter;

  iter = std::find_if( document->metadata.begin(),
                       document->metadata.end(),
                       MetadataPair::key_equal( "docno" ) );

  const char* docno = (char*)iter->value;

  iter = std::find_if( document->metadata.begin(),
                       document->metadata.end(),
                       MetadataPair::key_equal( "url" ) );

  const char* page = (char*)iter->value;
  const char* url = 0;
  int count = 0;
  int urlEnd = -1;

  // find the third slash, which should occur
  // right after the domain name
  const char* slash = 0;
  if(page)  slash = strchr( page, '/' );
  if(slash) slash = strchr( slash+1, '/' );
  if(slash) slash = strchr( slash+1, '/' );

  size_t domainLength;
  if( slash )
    domainLength = slash - page;
  else
    domainLength = strlen(page);

  // count links
  for( unsigned int i=0; i<document->tags.size(); i++ ) {
    TagExtent& extent = *(document->tags[i]);

    // we only extract absolute urls
    if( !strcmp( extent.name, "absolute-url" ) ||
        !strcmp( extent.name, "relative-url" ) ) {
      url = document->terms[ extent.begin ];
      urlEnd = extent.end;

      // if it has the same domain, throw it out
      //if( url && page && !lemur_compat::strncasecmp( url, page, domainLength ) ) {
      //  url = 0;
      //  urlEnd = -1;
      //}
    } else if( !strcmp( extent.name, "a" ) &&  // this is anchor text
               url &&                          // we've seen a url
               urlEnd == extent.begin &&       // this text is associated with an absolute-url
               extent.end - extent.begin > 0 ) // there is some text here
      {
        count++;
        url = 0;
      }
  }

  // print output

  // first - our document / page so we can sort it by url
  _docOrder << page << '\t' << docno << "\n"; //std::endl;
  _docOrder.flush();
  // tag this to our docno keyfile
  // account for the trailing \0

  // need a SHA1 / Hex hash on page
  if (strlen(page) > 511) {
    char hashBuffer[128];
    SHA1Hasher.hashStringToHex(page, hashBuffer, 128);
    _docNoKeyfile->put(hashBuffer, docno, strlen(docno)+1);
  } else {
    _docNoKeyfile->put(page, docno, strlen(docno)+1);
  }

  // _out << "DOCNO=" << docno << "\n"; //std::endl;
  // _out << "DOCURL=" << page << "\n"; //std::endl;
  // _out << "LINKS=" << count << "\n"; //std::endl;

  // write out the docno and URL for the doc order file
  // _docOrder << docno << "\t" << page << std::endl;

  url = 0;
  urlEnd = -1;

  for( unsigned int i=0; i<document->tags.size(); i++ ) {
    TagExtent& extent = *(document->tags[i]);

    if( !strcmp( extent.name, "absolute-url" ) ||
        !strcmp( extent.name, "relative-url" ) ) {  // this is an absolute url
      url = document->terms[ extent.begin ];
      urlEnd = extent.end;

      // if it has the same domain, throw it out
      //if( url && page && !lemur_compat::strncasecmp( url, page, domainLength ) ) {
      //  url = 0;
      //  urlEnd = -1;
      //}
    } else if( !strcmp( extent.name, "a" ) &&  // this is anchor text
               url &&                          // we've seen a url
               urlEnd == extent.begin &&       // this text is associated with an absolute-url
               extent.end - extent.begin > 0 ) // there is some text here
      {

      // do we need to conflate the URL based on the redirects?
      std::string finalURLTarget;
      if (url) {
        finalURLTarget=url;
        if (_redirectKeyfile) {
          // we have redirects to check

          if (finalURLTarget.length() > 511) {
            char hashBuffer[128];
            SHA1Hasher.hashStringToHex(finalURLTarget.c_str(), hashBuffer, 128);
            finalURLTarget=hashBuffer;
          } // end if (finalURLTarget.length() ...

          char *redirectUrl;
          int redirectUrlSize;
          if (_redirectKeyfile->get(finalURLTarget.c_str(), &redirectUrl, redirectUrlSize)) {
            finalURLTarget=redirectUrl;
          } // end if (_redirectKeyfile->get( ...
        } // end if (_redirectKeyfile) 
      } // end  if (url) 
      
      urlEnd = extent.end;
      std::stringstream outputLine;
      outputLine.clear();
      outputLine << finalURLTarget << '\t' << page << '\t';
      _linkFile << finalURLTarget << '\t' << page << '\t';

      const int maxInputLinesize=(lemur::file::FileMergeThread::MAX_INPUT_LINESIZE - 256);

      int currentLineLen=outputLine.str().length();

      for( size_t j=extent.begin; (int(j) < extent.end) && (currentLineLen < maxInputLinesize); j++ ) {
        if( !document->terms[j] )
          continue;

        currentLineLen += strlen(document->terms[j])+1;
        _linkFile << document->terms[j] << " ";
      } // end for( size_t j=extent.begin; int(j) < extent.end && textLength < 60000; j++ )

      _linkFile << "\n"; //std::endl;
      _linkFile.flush();
      // only do the same link once
      url = 0;

    } // end if( !strcmp( extent.name, "absolute-url" )...
  } // end for( unsigned int i=0; i<document->tags.size(); i++ )
}

