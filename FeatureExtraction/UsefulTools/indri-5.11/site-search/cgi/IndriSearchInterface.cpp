#include "IndriSearchInterface.h"
#include "CGIConfiguration.h"
#include "lemur-compat.hpp"

#include <time.h>
#include <cctype>
#include <algorithm>
#include <cmath>
IndriSearchInterface::IndriSearchInterface(CGIOutput *_output, indri::api::QueryEnvironment *_queryEnvironment, string _dataRoot) {
  output=_output;
  queryEnvironment=_queryEnvironment;
  dataRoot=_dataRoot;
}

IndriSearchInterface::~IndriSearchInterface() {
}


void IndriSearchInterface::addToStringVector(std::vector<std::string> *origVec, std::vector<std::string> *addVec) {
  if ((!origVec)||(!addVec)) return;
  std::vector<std::string>::iterator vIter=addVec->begin();
  while (vIter!=addVec->end()) {
    origVec->push_back(*vIter);
    vIter++;
  }
}

std::vector<std::string> IndriSearchInterface::getRawNodes(indri::api::QueryAnnotationNode *node) {
  std::vector<std::string> retArray;
  if (node) {
    if (node->type=="RawScorerNode") {
      retArray.push_back(node->name);
    } else {
      std::vector<indri::api::QueryAnnotationNode*>::iterator vIter=node->children.begin();
      while (vIter!=node->children.end()) {
        std::vector<std::string> childNodeVec=getRawNodes(*vIter);
        addToStringVector(&retArray, &childNodeVec);
        vIter++;
      }
    }
  }
  return retArray;
}

std::vector<IndriSearchInterface::indriTermMatches> IndriSearchInterface::getMatches(int docID, std::map<std::string, std::vector<indri::api::ScoredExtentResult> > *annotations, std::vector<string> *rawNodes) {
  std::vector<IndriSearchInterface::indriTermMatches> retVec;

  // thin down the annotation matches to just the ones for this document
  std::vector<indri::api::ScoredExtentResult> rawMatches;

  std::vector<std::string>::iterator nIter=rawNodes->begin();
  while (nIter!=rawNodes->end()) {
    std::map<std::string, std::vector<indri::api::ScoredExtentResult> >::iterator iterThisNode=annotations->find(*nIter);
    if (iterThisNode!=annotations->end()) {
      std::vector<indri::api::ScoredExtentResult>::iterator pIter=iterThisNode->second.begin();
      while (pIter!=iterThisNode->second.end()) {
        if ((*pIter).document==docID) {
          rawMatches.push_back(*pIter);
        }
        pIter++;
      }
    }
    nIter++;
  }

  // sort the array
  std::sort(rawMatches.begin(), rawMatches.end(), sortScoredExtentByPosition());

  // remove and coalesce duplicates
  int matchSize=rawMatches.size();
  if (matchSize > 0) {
    int begin=rawMatches[0].begin;
    int end=rawMatches[0].end;
    for (int i=1; i < matchSize; i++) {
      if (rawMatches[i].begin > end) {
        retVec.push_back(IndriSearchInterface::indriTermMatches(begin, end));
        begin=rawMatches[i].begin;
      }
      if (end < rawMatches[i].end) {
        end=rawMatches[i].end;
      }
    }
    retVec.push_back(IndriSearchInterface::indriTermMatches(begin, end));
  }

  // return it.
  return retVec;
}

string IndriSearchInterface::stripHtmlTags(string inputString) {
  string retString="";
  string::size_type oPos=inputString.find("<");
  if (oPos==string::npos) {
    return inputString;
  }

  string::size_type cPos=inputString.find(">", oPos);
  if (cPos==string::npos) {
    return inputString;
  }

  string::size_type lastPos=0;

  while ((oPos!=string::npos) && (cPos!=string::npos)) {
    int len=oPos-lastPos;
    if (len > 0) {
      retString.append(inputString.substr(lastPos, oPos-lastPos));
      retString.append(" ");
    }
    lastPos=cPos+1;
    oPos=inputString.find("<", cPos);
    cPos=inputString.find(">", oPos);
  }

  return retString;
}

string IndriSearchInterface::escapeHtmlTags(string s) {
  stringstream outputString;

  int sLen=s.length();
  for (int i=0; i < sLen; i++) {
    char cChar=s[i];
    switch(cChar) {                                                                                                                                                           
    case 38 :  outputString << "&amp;";       break;                                                                                                                      \
    case 34  : outputString << "&quot;";      break;                                                                                                                      \
    case 39  : outputString << "&apos;";      break;                                                                                                                      \
    case 60:  outputString << "&lt;";        break;                                                                                                                       \
    case 62:  outputString << "&gt;";        break;                                                                                                                       \
    default:   outputString << cChar;        break;
    }
  }
  return outputString.str();
}


#define INDRI_SNIPPET_MAX_WINDOWSIZE 50

std::string IndriSearchInterface::getScoredExtentSummaryString(indri::api::ScoredExtentResult &result, std::vector<std::string> *nodes, std::map< std::string, std::vector<indri::api::ScoredExtentResult> > *annotations, string docext) {

  std::vector<lemur::api::DOCID_T> docIDVec;
  // int thisDocID=db->document(docext);
  // docIDVec.push_back(thisDocID);
  std::vector<indri::api::ScoredExtentResult> resVec;
  resVec.push_back(result);
  // std::vector< indri::api::ParsedDocument*> dVec=queryEnvironment->documents(docIDVec);
  std::vector< indri::api::ParsedDocument*> dVec=queryEnvironment->documents(resVec);
  if (dVec.size()==0) return "...";
  indri::utility::greedy_vector<indri::parse::TermExtent> termPositions=dVec[0]->positions;

  string fullDocText = dVec[0]->text;

  // get snippet
  std::vector<IndriSearchInterface::indriTermMatches> matches=getMatches(result.document, annotations, nodes);
  if (matches.size()==0) {   dVec.clear(); return "..."; }

  int mw=(INDRI_SNIPPET_MAX_WINDOWSIZE / matches.size());
  if( mw < 7 ) {
    // want at least 7 words around each match
    mw = 7;
  } else if( mw > 30 ) {
    mw = 30;
  }

  int matchBegin = matches[0].begin;
  int matchEnd = matches[0].end;
  //$match = array( "begin" => $matchBegin, "end" => $matchEnd );
  int nWords = 0;

  int mwStart=(int)ceil((double)(mw) / 2.0);
  int mwEnd=(int)floor((double)(mw) / 2.0);

  int begin = matchBegin - mwStart;
  int end = matchEnd + mwEnd;

  if( begin < result.begin )
    begin=result.begin;
  if( end >= result.end )
    end = result.end;

  if( result.end - result.begin <= INDRI_SNIPPET_MAX_WINDOWSIZE ) {
    begin = result.begin;
    end = result.end;
  }

  int numMatches=matches.size();
  IndriSearchInterface::internalPos startInternal;
  startInternal.begin=matchBegin;
  startInternal.end=matchEnd;

  std::vector<IndriSearchInterface::indriTermMatches> segments;
  IndriSearchInterface::indriTermMatches workingSegment(begin, end);
  workingSegment.internalPositions.push_back(startInternal);

  for (int i=1; i < numMatches; i++) {
    IndriSearchInterface::internalPos matchInternal;
    matchInternal.begin=matches[i].begin;
    matchInternal.end=matches[i].end;

    begin=matches[i].begin - mwStart;
    end=matches[i].end + mwEnd;
    if (begin < result.begin) begin=result.begin;
    if (end > result.end) end=result.end;
    if ((nWords + (workingSegment.end-workingSegment.begin)) > INDRI_SNIPPET_MAX_WINDOWSIZE) {
      break;
    }

    if (workingSegment.end >= begin) {
      workingSegment.end=end;
      workingSegment.internalPositions.push_back(matchInternal);
      nWords+=(workingSegment.end-workingSegment.begin);
    } else {
      segments.push_back(workingSegment);
      nWords+=(workingSegment.end-workingSegment.begin);
      workingSegment.begin=begin;
      workingSegment.end=end;
      workingSegment.internalPositions.clear();
      workingSegment.internalPositions.push_back(matchInternal);
    }
    if (nWords > INDRI_SNIPPET_MAX_WINDOWSIZE) {
      break;
    }
  }
  segments.push_back(workingSegment);

  stringstream outString;
  for (int i=0; i < segments.size(); i++) {
    IndriSearchInterface::indriTermMatches thisSegment=segments[i];
    begin=thisSegment.begin;
    end=thisSegment.end;

    if ((begin > result.begin) && (i==0))
      outString << "<strong>...</strong>";

    if (((end-1)>=termPositions.size()) || (begin < 0)) {
      continue;
    }

    int beginByte=termPositions[begin].begin;
    int endByte=termPositions[end-1].end;
    int currentByte=beginByte;

    for (int j=0; j < thisSegment.internalPositions.size(); j++) {
      int beginMatch=thisSegment.internalPositions[j].begin;
      int endMatch=thisSegment.internalPositions[j].end;
      int tPos=termPositions[beginMatch].begin-currentByte;
      int ePos=termPositions[endMatch-1].end-termPositions[beginMatch].begin;

      if ((tPos > 0) && (ePos > tPos)) {
        outString << stripHtmlTags(fullDocText.substr(currentByte, termPositions[beginMatch].begin-currentByte)) << "<strong>";
        outString << stripHtmlTags(fullDocText.substr(termPositions[beginMatch].begin, termPositions[endMatch-1].end-termPositions[beginMatch].begin)) << "</strong>";
      }
      currentByte=termPositions[endMatch-1].end;
    }
    if ((currentByte < fullDocText.length()) && ((endByte-currentByte) > 0)) {
      outString << stripHtmlTags(fullDocText.substr(currentByte, endByte-currentByte));
    }

    if (end < (result.end-1)) {
      outString << "<strong>...</strong> ";
    }
  }

  dVec.clear();

  return outString.str();
}


void IndriSearchInterface::findAndReplace(std::string &source, const std::string &find, const std::string &replace) {
  size_t f;
  if (find != replace) {
    for (; (f=source.find(find))!=std::string::npos;) {
      source.replace(f, find.length(), replace);
    }
  }
}

std::string IndriSearchInterface::getASCIIFromPercentEncoding(std::string inputSequence) {
  if (inputSequence.length() < 3) { return inputSequence; }
  if (inputSequence[0]!='%') { return inputSequence; }

  unsigned char retChar;
  char firstNibble=inputSequence[1];
  char secondNibble=inputSequence[2];

  if ((firstNibble >='0') && (firstNibble <= '9')) {
    retChar=(firstNibble-'0')*16;
  } else if ((firstNibble >='a') && (firstNibble <= 'f')) {
    retChar=((firstNibble-'a')+10)*16;
  } else if ((firstNibble >='A') && (firstNibble <= 'F')) {
    retChar=((firstNibble-'A')+10)*16;
  } else {
    // invalid
    return inputSequence;
  }

  if ((secondNibble >='0') && (secondNibble <= '9')) {
    retChar+=(secondNibble-'0');
  } else if ((secondNibble >='a') && (secondNibble <= 'f')) {
    retChar+=((secondNibble-'a')+10);
  } else if ((secondNibble >='A') && (secondNibble <= 'F')) {
    retChar+=((secondNibble-'A')+10);
  } else {
    // invalid
    return inputSequence;
  }

  if (retChar < 32) { retChar=' '; }
  if (retChar > 127) { retChar=' '; }

  std::string retString;
  retString=retChar;

  return retString;
}

std::string IndriSearchInterface::normalizeURL(std::string inputURL) {
  // intermediate - get the protocol, domain, port, path, file, 
  // and any parameters
  std::string urlProtocol = "";
  std::string urlDomain = "";
  std::string urlPort = "";
  std::string urlPath = "";
  std::string urlFile = "";
  std::string urlParameters = "";

  std::string thisURL=inputURL;

  // get protocol
  size_t endUrlProtocol=thisURL.find_first_of("://");
  if (endUrlProtocol==std::string::npos) {
    // assume http
    urlProtocol="http";
  } else {
    urlProtocol=thisURL.substr(0, endUrlProtocol);
    thisURL=thisURL.substr(endUrlProtocol + 3);
  }

  // get domain - first check for a port
  size_t possiblePortSep=thisURL.find_first_of(":");
  if (possiblePortSep!=std::string::npos) {
    // see if there's a / before the port - if so, not a port...
    size_t firstSlash=thisURL.find_first_of("/");
    if (firstSlash!=std::string::npos && (firstSlash < possiblePortSep)) {
      // no port - just get the domain
      urlDomain=thisURL.substr(0, firstSlash);
      urlPort="";
      thisURL=thisURL.substr(firstSlash+1);
    } else {
      // we're good - assume the items before the : are the domain
      // and port after :, but before next /
      urlDomain=thisURL.substr(0, possiblePortSep);
      if (firstSlash==std::string::npos) {
        // no slash -
        urlPort=thisURL.substr(possiblePortSep+1);
        thisURL="";
      } else {
        urlPort=thisURL.substr(possiblePortSep+1);
        urlPort=urlPort.substr(0, urlPort.find_first_of("/"));
        thisURL=thisURL.substr(firstSlash+1);
      }
    }
  } else {
    // no port - find the first /
    urlPort="";
    size_t firstSlash=thisURL.find_first_of("/");
    if (firstSlash!=std::string::npos) {
      urlDomain=thisURL.substr(0, firstSlash);
      thisURL=thisURL.substr(firstSlash+1);
    } else {
      urlDomain=thisURL;
      thisURL="";
    }
  }

  // get the path (if any)
  size_t lastSlash=thisURL.find_last_of("/");
  if (lastSlash!=std::string::npos) {
    urlPath=thisURL.substr(0, lastSlash);
    thisURL=thisURL.substr(lastSlash+1);
  } else {
    urlPath="";
  }

  // get the filename
  size_t paramSep=thisURL.find_first_of("?");
  if (paramSep!=std::string::npos) {
    urlFile=thisURL.substr(0, paramSep);
    urlParameters=thisURL.substr(paramSep+1);
  } else {
    // no parameters found
    urlFile=thisURL;
    urlParameters="";
  }

  // step 1 - lowercase protocol and domain
  
  std::transform(urlProtocol.begin(), urlProtocol.end(), urlProtocol.begin(),
		 (int(*)(int))std::tolower);
  std::transform(urlDomain.begin(), urlDomain.end(), urlDomain.begin(),
		 (int(*)(int))std::tolower);

  // step 2 - look for default directory index and remove it if it exists
  std::string tempFilename=urlFile;
  std::transform(tempFilename.begin(), tempFilename.end(),
		 tempFilename.begin(), (int(*)(int))std::tolower);
  if ((tempFilename=="index.htm") ||
      (tempFilename=="index.html") ||
      (tempFilename=="index.php") ||
      (tempFilename=="index.phtml") ||
      (tempFilename=="default.asp") ||
      (tempFilename=="default.aspx")) {
    urlFile="";
  }

  // step 3 - transform escape sequences in the path, filename, and parameters
  size_t pctPlace=urlPath.find_first_of("%");
  size_t strlen = 0;

  while (pctPlace!=std::string::npos) {
    std::string pctItem=urlPath.substr(pctPlace, 3);
    findAndReplace(urlPath, pctItem, getASCIIFromPercentEncoding(pctItem));
    strlen = urlPath.length();
    if (strlen > pctPlace+1)
      pctPlace=urlPath.find_first_of("%", pctPlace+1);
    else
      pctPlace = -1;
  }
  pctPlace=urlFile.find_first_of("%");
  while (pctPlace!=std::string::npos) {
    std::string pctItem=urlFile.substr(pctPlace, 3);
    findAndReplace(urlFile, pctItem, getASCIIFromPercentEncoding(pctItem));
    strlen = urlPath.length();
    if (strlen > pctPlace+1)
      pctPlace=urlFile.find_first_of("%", pctPlace+1);
    else
      pctPlace = -1;
  }
  pctPlace=urlParameters.find_first_of("%");
  while (pctPlace!=std::string::npos) {
    std::string pctItem=urlParameters.substr(pctPlace, 3);
    findAndReplace(urlParameters, pctItem, 
		   getASCIIFromPercentEncoding(pctItem));
    strlen = urlParameters.length();
    if (strlen > pctPlace+1)
      pctPlace=urlParameters.find_first_of("%", pctPlace+1);
    else
      pctPlace = -1;
  }

  // step 4 - recombine final URL
  std::string retURL;
  if (urlProtocol.length() > 0) {
    retURL=urlProtocol + "://";
  } else {
    retURL="http://";
  }

  retURL+=urlDomain;

  if (urlPort.length() > 0) {
    if (urlPort!="80") {
      retURL+=":" + urlPort;
    }
  }

  retURL += "/";

  if (urlPath.length() > 0) {
    retURL += urlPath + "/";
  }

  retURL += urlFile;

  if (urlParameters.length() > 0) {
    retURL += "?" + urlParameters;
  }

  return retURL;
}

// NAM - Can't use lemur index here - won't work with indri daemons
// need to use indri query environment instead
//std::vector<indri::api::ScoredExtentResult> IndriSearchInterface::indriRemoveDuplicateResults(std::vector<indri::api::ScoredExtentResult> results, lemur::api::Index *db) {
std::vector<indri::api::ScoredExtentResult> IndriSearchInterface::indriRemoveDuplicateResults(std::vector<indri::api::ScoredExtentResult> results, indri::api::QueryEnvironment *indriEnvironment) {
  std::vector<indri::api::ScoredExtentResult> retVector;
  std::vector<indri::api::ScoredExtentResult>::iterator vIter=results.begin();
  std::map<std::string, int> seenIDs; // map of normalized URL, document ID
  std::map<int, int> vecPositions; // map of vectorPosition, document ID


  std::vector<std::string> urlStrings;
 
  std::string attribute = CGIConfiguration::getInstance().getDuplicateResultAttribute();

  if (( CGIConfiguration::getInstance().useDuplicateResultAttribute() ) && 
      ( attribute.length() != 0 ) ){
    urlStrings = indriEnvironment->documentMetadata(results, attribute);    
  }
  else {
    // no Attribute or attribute has an empty string. We will not do duplicate record checking in this case.
    return results;
  }


  retVector.clear();
  seenIDs.clear();
  vecPositions.clear();

  // do our normalization / de-duplication
  int vecPosition=0;
  while (vIter!=results.end()) {
    //std::string thisURL=db->document((*vIter).document);
    std::string thisURL=urlStrings[vecPosition];
    thisURL=normalizeURL(thisURL);
    if ((thisURL.length() > 0) && (seenIDs.find(thisURL)==seenIDs.end())) {
      seenIDs.insert(make_pair(thisURL, (*vIter).document));
      vecPositions.insert(make_pair(vecPosition, (*vIter).document));
    }
    vIter++;
    vecPosition++;
  } // end while (vIter!=results.end())

  // generate the output results set
  vIter=results.begin();
  vecPosition=0;
  while (vIter!=results.end()) {
    if (vecPositions.find(vecPosition)!=vecPositions.end()) {
      retVector.push_back(*vIter);
    }
    vIter++;
    vecPosition++;
  } // end while (vIter!=results.end())

  return retVector;
}

int IndriSearchInterface::findOccurrence(char *input, char *chrSet) {
  if ((!input) || (!chrSet)) return -1;
  char *tmp=input;
  int cCounter=0;
  while (*tmp) {
    char *cTmp=chrSet;
    while (*cTmp) {
      if (*cTmp==*tmp) {
        return cCounter;
      }
      cTmp++;
    }
    cCounter++;
    tmp++;
  }
  return -1;
}

string IndriSearchInterface::replaceAllByChr(string inputString, 
					     string characters) {
  char *inputCopy=strdup(inputString.c_str());
  char *setCopy=strdup(characters.c_str());

  int thisOccurence=findOccurrence(inputCopy, setCopy);
  while (thisOccurence > -1) {
    strcpy(inputCopy+thisOccurence, inputCopy+thisOccurence+1);
    thisOccurence=findOccurrence(inputCopy, setCopy);
  }

  string outputString;
  outputString=inputCopy;
  delete inputCopy;
  return outputString;
}

string IndriSearchInterface::indriDefaultQueryExpansion(string &origQuery, 
							bool usePagerank) {

  if (CGIConfiguration::getInstance().getKVItem("expandindriquery")!="true") {
    return origQuery;
  }

  // don't reformulate if this query contains a query operator
  if (origQuery.find("#")!=string::npos) {
    return origQuery;
  }

  // first - remove any bad possible punctuation
  // this includes ?;:!,.+-
  origQuery=replaceAllByChr(output->URLDecodeString(origQuery), 
			    "?;:!,.+-<>/{}[]_=|\\\'");

  // also - don't reformulate if we find any quotes...
  if ((origQuery.find("\"")!=string::npos) || (origQuery.find("%22")!=string::npos)) {
    // however, if we do find quotes - wrap the terms in a #1(...)
    // replace any %22 with actual quotes to make it easier...
    string::size_type tPos=origQuery.find("%22");
    while (tPos!=string::npos) {
      origQuery.replace(tPos, 3, "\"");
      tPos=origQuery.find("%22");
    }

    char *qCopy=strdup(origQuery.c_str());
    char *tmpPlace=qCopy;
    // replace quotes with what needs to be there...
    // 0x01 for the start quote - 0x02 for an end quote
    bool hasStarted=false;
    while (*tmpPlace) {
      if ((*tmpPlace)=='\"') {
        if (hasStarted) {
          *tmpPlace=0x02;
        } else {
          *tmpPlace=0x01;
        }
        hasStarted=!hasStarted;
      }
      tmpPlace++;
    }

    stringstream wrappedString;
    wrappedString << "#combine( ";

    // ok - now wrap the items.
    tmpPlace=qCopy;
    while (*tmpPlace) {
      if (*tmpPlace==0x01) {
        wrappedString << " #1( ";
      } else if (*tmpPlace==0x02) {
        wrappedString << " ) ";
      } else {
        wrappedString << (*tmpPlace);
      }
      tmpPlace++;
    }
    wrappedString << " )";

    // return the new string
    delete qCopy;
    if (usePagerank) {
      stringstream toWrap;
      toWrap << "#weight( 0.1 #weight( 1.0 #prior(pagerank) ) 1.0 " 
	     << wrappedString.str() << " )";
      return toWrap.str();
    }
    return wrappedString.str();
  }

  stringstream retString;
  retString << "#combine( " << origQuery << " )";
  if (usePagerank) {
    stringstream toWrap;
    toWrap << "#weight( 0.1 #weight( 1.0 #prior(pagerank) ) 1.0 " 
	   << retString.str() << " )";
    return toWrap.str();
  }
  return retString.str();
}

std::vector<std::string> IndriSearchInterface::getRawScoringNodes(const indri::api::QueryAnnotationNode *node) {
  std::vector<std::string> retVec;
  if (node->type=="RawScorerNode") {
    retVec.push_back(node->name);
  } else {
    std::vector<indri::api::QueryAnnotationNode*>::const_iterator cIter=node->children.begin();
    while (cIter!=node->children.end()) {
      std::vector<std::string> childResults=getRawScoringNodes(*cIter);
      retVec.insert(retVec.end(), childResults.begin(), childResults.end());
      cIter++;
    }
  }
  return retVec;
}

void IndriSearchInterface::performSearch(string &query, int maxNumResults, 
					 int indexID, int listLength, 
					 int rankStart) {
  if (!output) return;

  if (!queryEnvironment) {
    output->writeErrorMessage("No Query Environment?", "No Query Environment set on call to IndriSearchInterface::performSearch()");
    return;
  }

  FILE *oQueryLog=NULL;

  time_t rawtime;
  struct tm * timeinfo;

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  // see if we need to perform logging...
  if (CGIConfiguration::getInstance().useQueryLogging()) {
    oQueryLog=fopen(CGIConfiguration::getInstance().getQueryLogPath().c_str(),
		    "a");
  }

  // set our environment scoring rules
  std::vector<std::string> scoringRules;
  scoringRules.push_back("method:dirichlet,mu:250,field:mainbody,operator:term");
  scoringRules.push_back("method:dirichlet,mu:1000,field:mainbody,operator:window");
  scoringRules.push_back("method:dirichlet,mu:100,field:inlink,operator:term");
  scoringRules.push_back("method:dirichlet,mu:100,field:inlink,operator:window");
  scoringRules.push_back("method:dirichlet,mu:10,field:title,operator:term");
  scoringRules.push_back("method:dirichlet,mu:5,field:title,operator:window");
  scoringRules.push_back("method:dirichlet,mu:40,field:heading,operator:term");
  scoringRules.push_back("method:dirichlet,mu:80,field:heading,operator:window");
  queryEnvironment->setScoringRules(scoringRules);

  // clean up our query string if needed...
  string origQueryCopy="";
  origQueryCopy.append(query);
  origQueryCopy=output->URLDecodeString(origQueryCopy);

  if (oQueryLog) {
    string logTimeString;
    logTimeString.append(asctime(timeinfo));
    output->stringReplaceAll(&logTimeString, "\n", "");

    fprintf(oQueryLog, "[%s] Query: %s : ", logTimeString.c_str(), 
	    origQueryCopy.c_str());
  }

  // expand the query if we can...
  string reformulatedQuery=indriDefaultQueryExpansion(query, CGIConfiguration::getInstance().getSupportPageRankPrior());
  CGIConfiguration::getInstance().putKVItem("reformulatedQuery", 
					    reformulatedQuery);

  indri::api::QueryAnnotation *qaResults=NULL;
  std::vector< indri::api::ScoredExtentResult> qResults;
  
  // run the query...
  try {
    int totalNumResults=lemur_compat::min(maxNumResults, (rankStart+1000));
    qaResults=queryEnvironment->runAnnotatedQuery(reformulatedQuery, 
						  totalNumResults);
  } catch (...) {
    output->resetResultsPage();
    string escapedResultQuery = escapeHtmlTags(origQueryCopy);
    output->setResultQuery(escapedResultQuery);
    output->setResultStatistics(0, 0, 0, 0);
    output->displayResultsPageBeginning();
    output->outputString("The query &quot;&nbsp;<strong>" + escapedResultQuery + "</strong>&nbsp;&quot; could not be properly parsed. Please restate the query.");
    output->displayResultsPageEnding();
    if (oQueryLog) {
      fprintf(oQueryLog, "(exception thrown)\n");
      fclose(oQueryLog);
    }
    return;
  }

  // ok - we should now have a set of annotated results...
  // let's remove any duplicates...
  //std::vector<indri::api::ScoredExtentResult> finalResults=indriRemoveDuplicateResults(qaResults->getResults(), index);
  //std::vector<indri::api::ScoredExtentResult> finalResults=qaResults->getResults();

  std::vector<indri::api::ScoredExtentResult> finalResults=indriRemoveDuplicateResults(qaResults->getResults(), queryEnvironment);

  // get our raw scoring nodes from the annotation tree...
  //std::vector<std::string> rawScoringNodeNames=getRawScoringNodes(qaResults->getQueryTree());

  // reset out results page to initialize it...
  output->resetResultsPage();
  string resultQuery = escapeHtmlTags(origQueryCopy);
  output->setResultQuery(resultQuery);

  if (oQueryLog) {
    fprintf(oQueryLog, "(%ld results)\n", finalResults.size());
    fclose(oQueryLog);
  }

  // Display results
  //
  if (finalResults.size() == 0) {
    output->setResultStatistics(0, 0, 0, 0);
    output->displayResultsPageBeginning();
    output->outputString("No results.");
    if (CGIConfiguration::getInstance().getKVItem("displayquerydebug")=="true") {
      output->outputString("<hr />Reformulated Query: " + reformulatedQuery + "<hr />");
    }
    output->displayResultsPageEnding();
  } else {

    // get annotations for only those nodes we will be displaying...
    // create a docID vector...
    // std::vector<lemur::api::DOCID_T> displayNodes;

    int startItem=rankStart;
    int endItem=(rankStart+listLength);
    if (endItem > finalResults.size()) { endItem=finalResults.size(); }

    // start the page...
    output->setMaxResultsPerPage(listLength);

    //
    //  If someone tries to go past the end of the list, don't.
    //
    if (rankStart >= finalResults.size()) {
      rankStart = finalResults.size() - 1;
    }

    int maxResultsToGet=finalResults.size();
    if (DEFAULT_MAX_DOCUMENTS_TO_RETRIEVE!=0) {
      maxResultsToGet=lemur_compat::min((int)finalResults.size(), 
                                        DEFAULT_MAX_DOCUMENTS_TO_RETRIEVE);
    }

    output->setResultStatistics(indexID, rankStart,
                                lemur_compat::min(rankStart+listLength, maxResultsToGet),
                                maxResultsToGet);

    output->displayResultsPageBeginning();

    stringstream htmlListStart;
    htmlListStart << "<ol type=1 start=\"" << (rankStart + 1) << "\">\n";
    output->outputString(htmlListStart.str());

    std::vector<indri::api::ScoredExtentResult> resultSubset;
    resultSubset.assign(finalResults.begin()+rankStart, 
			finalResults.begin()+lemur_compat::min(rankStart+listLength, 
						 maxResultsToGet));

    std::vector<indri::api::ParsedDocument*> parsedDocs = queryEnvironment->documents(resultSubset);

    for (int i=0;(i<listLength) && (i<maxResultsToGet);i++) {
      indri::api::ScoredExtentResult thisResult=resultSubset[i];
      //
      // get DocMgr
      //

      std::vector< lemur::api::DOCID_T > docIDVec;
      docIDVec.clear(); // just to make sure...
      docIDVec.push_back(thisResult.document);

      //
      // fetch possible title
      //
      //string docext = index->document(thisResult.document);
      std::vector<std::string> docnos = queryEnvironment->documentMetadata(docIDVec, "docno");
      string docext = (*(docnos.begin()));

      //
      // Get the summary item (if any)
      //
      //string buf = getScoredExtentSummaryString( index, q, thisResult, &rawScoringNodeNames, &annotations, docext);
      indri::api::SnippetBuilder builder(true);
      string buf = builder.build(thisResult.document,parsedDocs[i],qaResults);

      //
      // if we're using an Indri index - check for metadata fields...
      //
      std::string thisTitle="";
      std::string thisURL="";

      // get the URL for this result
      std::vector< std::string > urlStrings=queryEnvironment->documentMetadata(docIDVec, "url");
      if (urlStrings.size() > 0) {
        thisURL=(*(urlStrings.begin()));
      } else {
        thisURL=docext;
      }
      // get a title field (if any)
      std::vector< std::string > titleStrings=queryEnvironment->documentMetadata(docIDVec, "title");
      if (titleStrings.size() > 0) {
        thisTitle=(*(titleStrings.begin()));
	// Trim the string to see if we have anyting we can use
	string::size_type pos = thisTitle.find_last_not_of(' ');
	if(pos != string::npos) {
	  thisTitle.erase(pos + 1);
	  pos = thisTitle.find_first_not_of(' ');
	  if(pos != string::npos) thisTitle.erase(0, pos);
	}
	else thisTitle.erase(thisTitle.begin(), thisTitle.end());
	// if we have an empty string then we use the URL.
	if (thisTitle.empty())
	    thisTitle=thisURL;
      } else {
        thisTitle=thisURL;
      }
      // should we strip the root path?
      if ((CGIConfiguration::getInstance().getStripRootPathFlag()) && (thisURL.find(dataRoot)==0)) {
        // remove the data root from the docext path...
        thisURL.erase(0,dataRoot.length());
      }

      // depending on if we have a title and/or URL, decide how we want to display it
      if ((thisTitle.length() > 0) && (thisURL.length() > 0)) {
        output->writeSearchResult(thisURL, docext, thisTitle, buf, 
				  thisResult.score, indexID, 
				  thisResult.document);
      } else if (thisURL.length() > 0) {
        output->writeSearchResult(thisURL, docext, thisURL, buf, 
				  thisResult.score, indexID, 
				  thisResult.document);
      } else if (thisTitle.length() > 0) {
        output->writeSearchResult("", docext, thisTitle, buf, 
				  thisResult.score, indexID, 
				  thisResult.document);
      } else {
        output->writeSearchResult("", docext, thisURL, buf, 
				  thisResult.score, indexID, 
				  thisResult.document);
      }
    } // for (int i=rankStart;(i<listLength+rankStart)&&(i<results.size());i++)

    output->outputString("</ol>\n");

    if (CGIConfiguration::getInstance().getKVItem("displayquerydebug")=="true") {
      output->outputString("<hr />Reformulated Query: " + CGIConfiguration::getInstance().getKVItem("reformulatedQuery") + "<hr />");
    }

    output->displayResultsPageEnding();

  } // end [else] if (results.size() == 0)

  finalResults.clear();
}
