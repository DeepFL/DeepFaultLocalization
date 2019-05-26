#include "CGIOutput.h"

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/** Construction / Destruction **/

CGIOutput::CGIOutput(string cgiURL) {
  scriptURL=cgiURL;
  maxResultsPerPage=DEFAULT_RESULTS_PER_PAGE;
  outputMode=CGI_OUTPUT_INTERACTIVE;
  haveHeadersBeenSent=false;
}

CGIOutput::~CGIOutput() {

}

/** private functions **/

char CGIOutput::toHex(char c) {
  return (c > 9) ? (c+55) : (c+48);
}

char CGIOutput::fromHex(char c1, char c2) {
  char retVal=0;
  if ((c1 > 47) && (c1 < 58)) {
    retVal=(c1-48) << 4;
  } else {
    retVal=(c2-55) << 4;
  }
  if ((c2 > 47) && (c2 < 58)) {
    retVal+=(c2-48);
  } else {
    retVal+=(c2-55);
  }
  return retVal;
}

void CGIOutput::outputHTTPHeader(string contentType) {
  if (!haveHeadersBeenSent) {
    if (outputMode!=CGI_OUTPUT_DIAGNOSTIC) {
      cout << "Content-type: " << contentType << "\n\n";
    } else {
      cout << "Content-Type: text/plain\n\n";
    }
    haveHeadersBeenSent=true;
  }
}

string CGIOutput::loadPageTemplate(string templateFile) {
  string outputString;

  string fullTemplatePath=CGIConfiguration::getInstance().getTemplatePath() + templateFile;

  ifstream infile;
  infile.open(fullTemplatePath.c_str());
  if (!infile.is_open()) {
    return outputString;
  }

  string buffString;

  while (!infile.eof()) {
    getline(infile, buffString);
    outputString+=buffString;
    outputString+="\n";
  }

  infile.close();

  replaceCompileDateVersion(&outputString);
  return outputString;
}

void CGIOutput::stringReplaceAll(string *s, string before, string after) {
  size_t i;

  i = s->find (before);
  while (i != string::npos) {
      s->replace (i, before.length(), after);
      i = s->find (before, i+after.length());
  };
}

string CGIOutput::URLEncodeString(string s) {
  stringstream outputString;

  int sLen=s.length();
  for (int i=0; i < sLen; i++) {
    char cChar=s[i];
    if (isalnum(cChar)) {
      outputString << cChar;
    } else {
      if (isspace(cChar)) {
        outputString << "+";
      } else {
        outputString << "%" << toHex(cChar >> 4) << toHex(cChar%16);
      }
    }
  }

  return outputString.str();
}

string CGIOutput::URLDecodeString(string s) {
  stringstream outputString;
  int sLen=s.length();

  int cPos=0;
  while (cPos < sLen) {
    char cChar=s[cPos];
    if (cChar=='+') {
      outputString << " ";
      cPos++;
    } else if (cChar=='%') {
      if ((cPos+2)>=sLen) {
        outputString << cChar;
        cPos++;
      } else {
        char fDigit=s[cPos+1];
        char sDigit=s[cPos+2];
        outputString << fromHex(fDigit, sDigit);
        cPos+=3;
      }
    } else {
      outputString << cChar;
      cPos++;
    }
  }

  return outputString.str();
}

void CGIOutput::replaceTemplateCommand(string *templatePage, string variableName, string replacement) {
  string fullVariableString;
  fullVariableString="{%" + variableName + "%}";
  size_t fullVarLen=fullVariableString.length();

  size_t currentPlace=templatePage->find(fullVariableString);
  while (currentPlace!=std::string::npos) {
    templatePage->replace(currentPlace, fullVarLen, replacement);
    currentPlace=templatePage->find(fullVariableString);
  }
}

void CGIOutput::replaceCompileDateVersion(string *templatePage) {
  replaceTemplateCommand(templatePage, "LemurSearchCompileDate", LEMUR_CGI_COMPILE_DATE);
  replaceTemplateCommand(templatePage, "LemurSearchVersion", LEMUR_CGI_VERSION_NUMBER);
}

string CGIOutput::processParameterCommand(string command, string paramValue) {
  string actualParamValue;

  // if the paramvalue has quotes around it - remove them...
  if ((paramValue.length() > 0) && ((paramValue[0]=='\"') && (paramValue[paramValue.length()-1]=='\"'))) {
    size_t pValueLength=paramValue.length();
    string tmpactualParamValue=paramValue.substr(1, pValueLength-2);
    actualParamValue=tmpactualParamValue;
  } else {
    actualParamValue=paramValue;
  }

  if (command=="LemurSearchSetTitle") {
    // set the page title
    substitutionValues.put("LemurSearchTitle", actualParamValue.c_str());
    return "";
  }

  if (command=="LemurSearchBox") {
    // display a search box
    stringstream retString;
    string formValue=queryTerms;
    stringReplaceAll(&formValue, "\"", "&quot;");
    retString << "<form action=\"" << scriptURL << "\" method=GET>\n"
              << "<input type=\"hidden\" name=\"x\" value=\"false\">"
              << "<input type=\"text\" name=\"q\" size=\"" << actualParamValue << "\""
              <<  " value=\"" << URLDecodeString(formValue) << "\"><input type=\"submit\" value=\"Search\"></form>";

    return retString.str();
  }


  // the rest of the possible commands deal with the page...
  long numPages = (totalResultNum / maxResultsPerPage) + 1;
  if ((!(totalResultNum % maxResultsPerPage)) && (numPages > 1)) {
    numPages--;
  }
  long nextPageStart=startResultNum+maxResultsPerPage;
  long previousPageStart=MAX(nextPageStart - maxResultsPerPage - maxResultsPerPage, 0);
  long thisPage = nextPageStart / maxResultsPerPage;
  long startPage = MAX(thisPage - 4, 1);
  long endPage =   MAX(MIN(thisPage + 5, numPages), 10);
  if (endPage > numPages) endPage=numPages;

  nextPageStart=MIN(nextPageStart, totalResultNum-1);

  if (command=="LemurSearchResultsPreviousPage") {
    // display a previous page link...
    stringstream previousLink;

    if (startResultNum > 0) {

      previousLink << "&laquo;&nbsp;<a href=\"" << scriptURL << "?d=" << currentDatasourceID << "&s=" << previousPageStart
                   << "&n=" << maxResultsPerPage << "&q=" << URLEncodeString(queryTerms) << "\">Previous</a>&nbsp;&#124;";
    }

    return previousLink.str();
  }

  if (command=="LemurSearchResultsNextPage") {
    // display a next page link...
    stringstream nextLink;

    if (nextPageStart < (totalResultNum-1)) {
      nextLink << "&nbsp;&#124;&nbsp;<a href=\"" << scriptURL << "?d=" << currentDatasourceID << "&s=" << nextPageStart << "&n=" << maxResultsPerPage << "&q="
               << URLEncodeString(queryTerms) << "\">Next</a>&nbsp;&raquo;";
      return nextLink.str();
    }

    return "";
  }

  if ((command=="LemurSearchResultPageNumbers") && (totalResultNum > 0)) {

    if (startPage==endPage) return "";

    stringstream outString;

    if (startPage > 1) {
      outString << "&nbsp;<a href=\"" << scriptURL << "?d=" << currentDatasourceID << "&s=0"
                << "&n=" << maxResultsPerPage
                << "&q=" << URLEncodeString(queryTerms)
                << "\">" << 1 << "</a>";
      outString << "&nbsp;...";
    }

    for (int i=startPage; i <=endPage; i++) {
      long pageStartRank=((i-1)*maxResultsPerPage);
      if (i==thisPage) {
        outString << "&nbsp;" << i;
      } else {
        outString << "&nbsp;<a href=\"" << scriptURL << "?d=" << currentDatasourceID << "&s=" << pageStartRank
                  << "&n=" << maxResultsPerPage
                  << "&q=" << URLEncodeString(queryTerms)
                  << "\">" << i << "</a>";
      }
    }

    if (endPage < numPages) {
      outString << "&nbsp;...";
      outString << "&nbsp;<a href=\"" << scriptURL << "?d=" << currentDatasourceID << "&s=" << MIN(((numPages-1)*maxResultsPerPage), totalResultNum)
                << "&n=" << maxResultsPerPage
                << "&q=" << URLEncodeString(queryTerms)
                << "\">" << numPages << "</a>";
    }

    return outString.str();
  }

  return "";
}

// checks and replaces page parameters
void CGIOutput::replaceResultsPageParameters(string *templatePage) {

  size_t currentPlace=templatePage->find("{%");
  while (currentPlace!=std::string::npos) {
    size_t endTag=templatePage->find("%}");

    if (endTag==std::string::npos) {
      // mismatched ending tag - just exit.
      break;
    }

    // get the variable...
    string thisCommand=templatePage->substr(currentPlace+2, (endTag-currentPlace-2));

    string parameterValue="";

    // does this have a parameter for this command?
    size_t parenStart=thisCommand.find("(");
    if (parenStart!=std::string::npos) {
      // look for the end paren...
      size_t parenEnd=thisCommand.rfind(")");
      if (parenEnd==std::string::npos) {
        // not found! Indicate error...
        parameterValue="[ERROR: Missing right parentheses for command: " + thisCommand + "]";
      } else {
        // strip the command
        string commandOnly=thisCommand.substr(0, parenStart);
        string paramOnly=thisCommand.substr(parenStart+1, parenEnd-parenStart-1);
        parameterValue=processParameterCommand(commandOnly, paramOnly);
      }
    } else {
      // no command? try replace with straight subtitution value perhaps.
      char *value=substitutionValues.get(thisCommand.c_str());
      if (value) {
        parameterValue=(value);
      }
    }

    // do the final replacement.
    templatePage->erase(currentPlace, (endTag-currentPlace)+2);
    templatePage->insert(currentPlace, parameterValue);

    // loop
    currentPlace=templatePage->find("{%");
  }
}

/** public functions **/

void CGIOutput::writeErrorMessage(string errorTitle, string errorMessage) {
  string errorPageTemplate;

  errorPageTemplate=loadPageTemplate(ERRORPAGE_TEMPLATE_NAME);
  if ((errorPageTemplate.length() > 0) && (outputMode!=CGI_OUTPUT_PROGRAM)) {
    replaceTemplateCommand(&errorPageTemplate, "LemurSearchErrorTitle", errorTitle);
    replaceTemplateCommand(&errorPageTemplate, "LemurSearchErrorMessage", errorMessage);
    outputHTTPHeader("text/html");
    cout << errorPageTemplate << "\n";
  } else {
    outputHTTPHeader("text/plain");
    cout << "Error retrieving error page template: " << ERRORPAGE_TEMPLATE_NAME << "\n";
    cout << "Check ERRORPAGE_TEMPLATE_NAME.\n\n";
    cout << "Error Title: " << errorTitle << endl;
    cout << "Error Message: " << errorMessage << endl;
  }
}

void CGIOutput::writePlaintextFooter() {
  cout << "-=-=-=-=-=-=-=-=-=-=-\nPowered by the Lemur Toolkit (www.lemurproject.org)\n"
        << "Version: " << LEMUR_CGI_VERSION_NUMBER << " (compiled on: " << LEMUR_CGI_COMPILE_DATE << ")\n"
        << "(C) 2006 - The Lemur Project (www.lemurproject.org)\n";
}

void CGIOutput::outputString(string stringToWrite) {
  cout << stringToWrite;
}

void CGIOutput::displayDefaultSearchPage() {
  string defaultPage=loadPageTemplate(DEFAULTSEARCHPAGE_TEMPLATE_NAME);
  if (defaultPage=="") {
    writeErrorMessage("Cannot load template.", "Cannot load default search template.");
    return;
  }

  replaceResultsPageParameters(&defaultPage);

  outputHTTPHeader("text/html");
  cout << defaultPage << "\n";
}

bool CGIOutput::resetResultsPage() {
   std::string aBlankString="";
  // reset any statistics
  setResultStatistics(0, 0, 0, 0);
  setResultQuery(aBlankString);
  currentNumDisplayedResults=0;

  substitutionValues.clear();

  substitutionValues.put("LemurSearchTitle", DEFAULT_RESULTS_PAGE_TITLE);

  loadedPageTemplateStart="";
  loadedPageTemplateEnd="";

  // load in the template
  string templatePage=loadPageTemplate(RESULTSPAGE_TEMPLATE_NAME);

  if (templatePage=="") {
    // couldn't load it!
    writeErrorMessage("Cannot load results page template.", "Cannot load results page template. Template file could not be opened.");
    return false;
  }

  // split the template into [start] [results] [end]
  size_t resultsPosition=templatePage.find("{%LemurSearchResults%}");
  if (resultsPosition==std::string::npos) {
    // no results tag?
    writeErrorMessage("No LemurSearchResults tag found.", "Error in results page template. A {%LemurSearchResults%} tag could not be found.");
    return false;
  }

  loadedPageTemplateStart=templatePage.substr(0, resultsPosition);
  loadedPageTemplateEnd=templatePage.substr(resultsPosition + 22, templatePage.length());

  // ensure there is one and only one {%LemurSearchResults%} tag...
  if (loadedPageTemplateEnd.find("{%LemurSearchResults%}")!=std::string::npos) {
    writeErrorMessage("Multiple LemurSearchResults tags found.", "Error in results page template. Multiple {%LemurSearchResults%} tags were found. Only one is allowed.");
    return false;
  }

  // process any pre-processor tags (# results, etc.)

  // load in the result item template
  resultItemTemplate=loadPageTemplate(SINGLERESULT_TEMPLATE_NAME);

  return true;
}

void CGIOutput::setResultQuery(string &query) {
  queryTerms="";
  queryTerms.append(query);
  substitutionValues.put("LemurSearchQueryTerms", queryTerms.c_str());
}

void CGIOutput::setResultStatistics(int datasourceID, int start, int end, int total) {
  currentDatasourceID=datasourceID;
  startResultNum=start;
  endResultNum=end;
  totalResultNum=total;

  stringstream tmpBuffer;

  tmpBuffer.str("");
  if (end==0) {
    tmpBuffer << "0";
  } else {
    tmpBuffer << (start+1);
  }
  substitutionValues.put("LemurSearchResultsStartNum", tmpBuffer.str().c_str());
  tmpBuffer.str("");
  tmpBuffer << end;
  substitutionValues.put("LemurSearchResultsEndNum", tmpBuffer.str().c_str());
  tmpBuffer.str("");
  tmpBuffer << total;
  substitutionValues.put("LemurSearchResultsTotalNum", tmpBuffer.str().c_str());
}

void CGIOutput::displayResultsPageBeginning() {
    replaceResultsPageParameters(&loadedPageTemplateStart);
    outputHTTPHeader("text/html");
    cout << loadedPageTemplateStart << endl;
}

void CGIOutput::displayResultsPageEnding() {
    replaceResultsPageParameters(&loadedPageTemplateEnd);
    cout << loadedPageTemplateEnd << "\n" << endl;
}

void CGIOutput::displayDataPage(string stringOfData, string pageTitle, bool listData) {

  if (outputMode==CGI_OUTPUT_DIAGNOSTIC) {
    outputHTTPHeader("text/plain");
    cout << stringOfData << endl;
    return;
  }

  if (outputMode==CGI_OUTPUT_PROGRAM) {
    outputHTTPHeader("text/html");
    cout << "<html>\n<head>\n<title>" << pageTitle << "</title>\n</head>\n<body>\n" << stringOfData << "\n</body>\n</html>\n\n";
    return;
  }

  string genericPage=loadPageTemplate(GENERICPAGE_TEMPLATE_NAME);
  if (genericPage=="") {
    writeErrorMessage("Cannot load template.", "Cannot load template for index display.");
    return;
  }

  // build our LemurCGIItems string...
  stringstream outputString;

  if (listData) {
    outputString << "<PRE>\n" << stringOfData << "\n</PRE>\n";
  } else {
    outputString << stringOfData << "\n";
  }

  substitutionValues.put("LemurCGIItems", outputString.str().c_str());
  substitutionValues.put("LemurSubTitle", pageTitle.c_str());

  replaceResultsPageParameters(&genericPage);

  outputHTTPHeader("text/html");
  cout << genericPage << "\n";

}

void CGIOutput::displayIndexListingPage() {
  string genericPage=loadPageTemplate(GENERICPAGE_TEMPLATE_NAME);
  if (genericPage=="") {
    writeErrorMessage("Cannot load template.", "Cannot load template for index display.");
    return;
  }

  // build our LemurCGIItems string...
  stringstream lemurIndexItems;

  lemurIndexItems << "<PRE>\n";

  int nIndices=CGIConfiguration::getInstance().getNumIndices();

  lemurIndexItems << "IndexID " << setw(9) << "Index Description\n\n";

  for (int i=0; i < nIndices; i++) {
    lemurIndexItems << i << " " << setw(9) << CGIConfiguration::getInstance().getIndexDescription(i) << "\n";
  }

  lemurIndexItems << "</PRE>\n";

  displayDataPage(lemurIndexItems.str(), "Index Listing", false);
}


bool CGIOutput::writeSearchResult(string resultURL, string origURL, string resultTitle, string resultSummary, double resultScore, int datasource, int resultID) {
  if (currentNumDisplayedResults >= maxResultsPerPage) return false;

  SingleResultItem thisItem(resultItemTemplate);

  thisItem.setVariable("URL", resultURL);
  thisItem.setVariable("origURL", origURL);
  thisItem.setVariable("title", resultTitle);
  thisItem.setVariable("summary", resultSummary);
  stringstream sScore;
  sScore << resultScore;
  thisItem.setVariable("score", sScore.str());
  stringstream sResultID;
  sResultID << resultID;
  thisItem.setVariable("id", sResultID.str());
  stringstream sDataSourceID;
  sDataSourceID << datasource;
  thisItem.setVariable("datasource", sDataSourceID.str());
  thisItem.setVariable("scriptname", scriptURL);

  if (CGIConfiguration::getInstance().getStripRootPathFlag()) {
    string dRootString(CGIConfiguration::getInstance().getRootPath(0));
    string oURLCopy(origURL);

    if (oURLCopy.find(dRootString)==0) {
      // remove the data root from the docext path...
      oURLCopy.erase(0,dRootString.length());
      thisItem.setVariable("URL", oURLCopy);

      // should we add anything?
      oURLCopy.insert(0, CGIConfiguration::getInstance().getRootAddPath());

      if ((oURLCopy.find("http://")!=0) && (oURLCopy.find("HTTP://")!=0)) {
        oURLCopy="http://" + oURLCopy;
      }
    }
    thisItem.setVariable("origURL", oURLCopy);

    // see if the title has the same problem...
    string tCopy(resultTitle);
    if (tCopy.find(dRootString)==0) {
      tCopy.erase(0, dRootString.length());
      thisItem.setVariable("title", tCopy);
    }
  }

  cout << "<li>" << thisItem.toString() << "</li>\n";

  currentNumDisplayedResults++;

  if (currentNumDisplayedResults >= maxResultsPerPage) return false;

  return true;
}


