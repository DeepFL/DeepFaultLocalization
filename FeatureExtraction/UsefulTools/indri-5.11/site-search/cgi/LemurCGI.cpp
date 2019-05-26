#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <map>
using std::map;

#include "LemurSearchCGIConstants.h"
#include "CGIConfiguration.h"
#include "CGIOutput.h"
#include "DictionaryHash.h"
#include "DBInterface.h"
#include "indri/Thread.hpp"

#define DEFAULT_TIMEOUT 300

// these are for offline debugging...
//#define OFFLINEDEBUGGING
//#define OFFLINEDEBUGGING_QUERY "x=false&q=%23combine[inlink](obama)"

// our query parameters

typedef struct kvPair_T {
  string key;
  string value;

  kvPair_T(string k, string v) {
    key=k; value=v;
  }

} t_kvPair;

std::vector<t_kvPair*> queryParameters;

string appPath;

/*
 *  Extract a name/value pair from a CGI query string.  Return the
 *  query string with that pair removed.
 */
static void extractNameValue (string *query, string *name, string *value)
{
  size_t i = query->find ("=");
  size_t j = query->find ("&");

  if (j == string::npos)
    j = query->length ();

  *name =  query->substr (0, i);
  *value = query->substr (i+1, (j-i)-1);

  if ((*query)[j] == '&')
    *query = query->substr (j+1);
  else
    *query = query->substr (j);
}

bool getCGIParams(CGIOutput *output, string requestMethod) {
  // only currently support get...
  if (requestMethod=="GET") {

#ifndef OFFLINEDEBUGGING
    string query;
    char *qStringEnv=getenv("QUERY_STRING");
    if (qStringEnv) {
      query=qStringEnv;
    }
#else
    string query(OFFLINEDEBUGGING_QUERY);
#endif

    while (query!="") {
      string tmpName;
      string tmpValue;

      extractNameValue (&query, &tmpName, &tmpValue);
      tmpValue=output->URLDecodeString(tmpValue);

      // TODO : don't use dictionary hash - use vector
      // with structure for key=value!
      // multiple key/value pairs and process them in order...
      // old --> queryParameters.put(tmpName.c_str(), tmpValue.c_str());
      queryParameters.push_back(new t_kvPair(tmpName, tmpValue));
    }

    return true;
  }

  return false;
}

void printOutHelp(CGIOutput *output) {
  if (!output) return;

  output->outputHTTPHeader("text/plain");

  stringstream helpMessage;
  helpMessage << "Lemur CGI - Help:\n\n"
              << "Invoke the CGI interface by "
              << appPath
              << "?name=value&name=value&....\n\n"
              << "Name=value pairs are processed from left to right, in order\n\n"
              << "NAME=VALUE ARGUMENTS\n"
              << "(c=<term> | termstats=<term>) prints corpus statistics for term\n"
              << "(d=<n> | datasource=<n>) sets the database to the n'th database\n"
              << "(d=? | listdatasources) lists the available databases\n"
              << "(D=<n> | datasourcestats=<n>) displays the statistics for the database ID\n"
              << "(e=<string> | getdocext=<string>) fetches the document with external id <string>\n"
              << "(f=? | listfields) lists the available fields for the index\n"
              << "(g=d | setoutput=debug) sets the CGI interface to Diagnostic mode\n"
              << "(g=i | setoutput=interactive) sets the CGI interface to Interactive mode\n"
              << "(g=p | setoutput=program) sets the CGI interface to Program mode\n"
              << "(h=<anything> | help) prints this help message\n"
              << "(i=<integer> | getdoc=<integer>) fetches the document with internal id <integer>\n"
              << "(I=<integer> | getparseddoc=<integer>) fetches the parsed form of the document with internal id <integer>\n"
              << "(m=<term> | getterm=<term>) shows the lexicalized (stopped, stemmed) form of <term>\n"
              << "(n=<n> | maxresults=<n>) sets the number of documents to retrieve to <n>\n"
              << "(q=<string> | query=<string>) uses the query <string> to search the database\n"
              << "(s=<n> | start=<n>) starts the query results at rank n\n"
              << "(t=<query_type> | querytype=<query_type>) tells the CGI which type of query interface to use (currently \"indri\" [default] or \"inquery\")\n"
              << "(v=<term> | invlist=<term>) returns the inverted list for term (use term.field for field specific list)\n"
              << "(V=<term> | invposlist=<term>) returns the inverted list for term with positions (use term.field for field specific list)\n"
              << "(x=false | queryexpansion=false) if using indri-style queries, this will not expand the query (expansion is turned on by default)\n";

  helpMessage << "\n";

  output->outputString(helpMessage.str());
  output->writePlaintextFooter();
}

std::string keyFirstCharToWholeKey(char keyChar) {
  switch (keyChar) {
    case 'c':
			return "termstats";
    case 'd':
			return "datasource";
    case 'D':
			return "datasourcestats";
    case 'e':
			return "getdocext";
    case 'f':
			return "listfields";
    case 'g':
			return "setoutput";
    case 'h':
			return "help";
    case 'i':
			return "getdoc";
    case 'I':
			return "getparseddoc";
    case 'm':
			return "getterm";
    case 'n':
			return "maxresults";
    case 'q':
			return "query";
    case 'Q':
			return "querydebug";
    case 's':
			return "start";
    case 't':
			return "querytype";
    case 'v':
			return "invlist";
    case 'V':
			return "invposlist";
    case 'x':
			return "queryexpansion";
		default: return "";
	}; // end switch (firstKeyChar)
}

void processRequest(CGIOutput *output) {
  if (!output) return;

  if (queryParameters.size()==0) {
    // no parameters - display search box
    output->displayDefaultSearchPage();
    return;
  }

  // set up some default items...

  // create our database interface...
  DBInterface db(output);

  if ((CGIConfiguration::getInstance().getStripRootPathFlag()) && (CGIConfiguration::getInstance().getNumRootPaths())) {
    // for now, only set the first root path...
    db.setDataRoot(CGIConfiguration::getInstance().getRootPath(0));
  }

  // default is interactive mode...
  output->setOutputMode(CGI_OUTPUT_INTERACTIVE);

  int datasourceToUse=0;  // set default index path...
  db.setIndexPath(CGIConfiguration::getInstance().getIndexPath(datasourceToUse));

  // set max # documents to retrieve...
  int maxDocsToRetrievePerPage=DEFAULT_RESULTS_PER_PAGE;

  // query results start rank...
  int startRank=0;

  // check to make sure we've outputted something
  bool hasOutput=false;

  // set query expansion for indri on by default
  CGIConfiguration::getInstance().putKVItem("expandindriquery", "true");

  DBInterface::QUERY_INTERFACE_TYPE currentQueryType=DBInterface::QUERY_INTERFACE_INDRI;

  std::vector<t_kvPair*>::iterator vIter=queryParameters.begin();
  while (vIter!=queryParameters.end()) {

    string thisKey=(*vIter)->key;
    string thisVal=(*vIter)->value;

    if (thisKey.length()==0) {
      // should never happen... but just in case...
      vIter++;
      continue;
    }

    if (thisKey.length()==1) {
      // backwards compatibility for single-letter functions
      thisKey=keyFirstCharToWholeKey(thisKey[0]);
    }

    /* I know, bad programming style follows... */

    if (thisKey=="termstats") {
      db.getTermCorpusStats(&thisVal);
      hasOutput=true;
    } else if (thisKey=="datasource") {
      if (thisVal.length() > 0) {
        if (thisVal[0]=='?') {
          // display datasource list and exit...
          output->displayIndexListingPage();
	  hasOutput=true;
	} else {
	  datasourceToUse=atoi(thisVal.c_str());
	  if (datasourceToUse >= CGIConfiguration::getInstance().getNumIndices()) {
	    // error out - unknown datasource...
	    stringstream tmpDSource;
	    tmpDSource << "Unknown datasource value: d=" << datasourceToUse;
	    output->writeErrorMessage("Unknown datasource.", tmpDSource.str());
	    return;
	  } // end if (datasourceToUse >= CGIConfiguration::getInstance().getNumIndices())

	  db.setIndexPath(CGIConfiguration::getInstance().getIndexPath(datasourceToUse));
	} // end if (thisVal[0]=='?')
      } // end if (thisVal.length() > 0)
    } else if (thisKey=="listdatasources") {
      output->displayIndexListingPage();
      hasOutput=true;
    } else if (thisKey=="datasourcestats") {
      // get database statistics
      if (thisVal.length() > 0) {
        if (thisVal[0]=='?') {
          // display datasource list and exit...
          output->displayIndexListingPage();
	  hasOutput=true;
	} else {
	  int whichDatasource=atoi(thisVal.c_str());

	  if (whichDatasource >= CGIConfiguration::getInstance().getNumIndices()) {
	    // error out - unknown datasource...
	    stringstream tmpDSource;
	    tmpDSource << "Unknown datasource value: D=" << whichDatasource;
	    output->writeErrorMessage("Unknown datasource.", tmpDSource.str());
	    return;
	  }

	  db.setIndexPath(CGIConfiguration::getInstance().getIndexPath(whichDatasource));
	  db.displayIndexStatistics(whichDatasource);
	  db.setIndexPath(CGIConfiguration::getInstance().getIndexPath(datasourceToUse));
	  hasOutput=true;
	} //end if (thisVal[0]=='?')
      } // end if (thisVal.length() > 0)
    } else if (thisKey=="getdocext") {
      db.getDocXID(&thisVal);
      hasOutput=true;
    } else if (thisKey=="listfields") {
      db.listIndexFields();
      hasOutput=true;
    } else if (thisKey=="setoutput") {
      if (thisVal=="d" || thisVal=="debug") {
        output->setOutputMode(CGI_OUTPUT_DIAGNOSTIC);
      } else if (thisVal=="p" || thisVal=="program") {
        output->setOutputMode(CGI_OUTPUT_PROGRAM);
      } else if (thisVal=="i" || thisVal=="interactive") {
	output->setOutputMode(CGI_OUTPUT_INTERACTIVE);
      }
    } else if (thisKey=="help") {
      printOutHelp(output);
      hasOutput=true;
    } else if (thisKey=="getdoc") {
      long internalID=atol(thisVal.c_str());
      db.getDocIID(internalID);
      hasOutput=true;
    } else if (thisKey=="getparseddoc") {
      long internalID=atol(thisVal.c_str());
      db.getParsedDoc(internalID);
      hasOutput=true;
    } else if (thisKey=="getterm") {
      db.getWordStem(&thisVal);
      hasOutput=true;
    } else if (thisKey=="maxresults") {
      // set max # of documents to retrieve per page
      maxDocsToRetrievePerPage=atoi(thisVal.c_str());
      if (maxDocsToRetrievePerPage < 1) {
        maxDocsToRetrievePerPage=DEFAULT_RESULTS_PER_PAGE;
      }
    } else if (thisKey=="query") {
      if (thisVal=="") {
        output->displayDefaultSearchPage();      
      } else {
        db.search(datasourceToUse, thisVal, maxDocsToRetrievePerPage, 
		  startRank, currentQueryType);
      }  
      hasOutput=true;
    } else if (thisKey=="querydebug") {
      CGIConfiguration::getInstance().putKVItem("displayquerydebug", "true");
      if (thisVal=="") {
        output->displayDefaultSearchPage();      
      } else {
        db.search(datasourceToUse, thisVal, maxDocsToRetrievePerPage, 
		  startRank, currentQueryType);
      }  
      hasOutput=true;
    } else if (thisKey=="start") {
      startRank=atoi(thisVal.c_str());
      if (startRank < 0) {
        startRank=0;
      }

      if (DEFAULT_MAX_DOCUMENTS_TO_RETRIEVE!=0) {
        if (startRank > DEFAULT_MAX_DOCUMENTS_TO_RETRIEVE) {
          startRank=DEFAULT_MAX_DOCUMENTS_TO_RETRIEVE-1;
        }
      }
    } else if (thisKey=="querytype") {
      // set query type...
      if (thisVal.compare("indri")==0) {
        currentQueryType=DBInterface::QUERY_INTERFACE_INDRI;
      } else if (thisVal.compare("inquery")==0) {
        currentQueryType=DBInterface::QUERY_INTERFACE_INQUERY;
      } else {
        // ignore it...
      }
    } else if (thisKey=="invlist") {
      db.getTermInvListField(&thisVal);
      hasOutput=true;
    } else if (thisKey=="invposlist") {
      db.getTermInvPosListField(&thisVal);
      hasOutput=true;
    } else if (thisKey=="queryexpansion") {
      CGIConfiguration::getInstance().putKVItem("expandindriquery", thisVal);
    }
    vIter++;
  } // end while (vIter!=queryParameters.end())
  if (!hasOutput) {
    // nothing was output? Do a default page      
    output->displayDefaultSearchPage();
  }
}

char* getFileFromPath(const char *filePath) {
  if ((!filePath) || (!strlen(filePath)))
    return NULL;

  char *fPtr=(char*)(filePath+strlen(filePath)-1);
  //accounts for *nix, Windows/MS-DOS, & Macintosh path names
  while ((fPtr!=filePath) && ((*fPtr)!='/') 
	 && ((*fPtr)!='\\') && ((*fPtr)!=':')) {
    fPtr--;
  }
  if (fPtr!=filePath) {
    return fPtr+1;
  } else {
    if ((*fPtr=='/') || (*fPtr=='\\') || (*fPtr==':'))
      return fPtr+1;
    else
      return fPtr;
  }
}

// timeout is in seconds
struct timer_thread_info {
  CGIOutput *output;
  int timeout;
  indri::thread::Thread *thread;
};

void timer_thread(void *c) {
  timer_thread_info *info = (timer_thread_info *)c;
#ifdef WIN32
    Sleep(info->timeout);
#else
  sleep(info->timeout);
#endif
  if (!info->output->headersSent()) {
    info->output->writeErrorMessage("Query Timeout", "The query you have entered is taking too long to process. Please restate the query.\n\n");
    exit(0);
  }
}

int main (int argc, char **argv) {

  // right off the bat - get the request method and our pathf
#ifndef OFFLINEDEBUGGING
  string cgi_request_method;
  if (getenv("REQUEST_METHOD")==NULL) {
    cgi_request_method="GET";
  } else {
    cgi_request_method=getenv("REQUEST_METHOD");
  }
#else
  string cgi_request_method("GET");
#endif

  appPath = argv[0];

  // strip the path off the appPath so we're left with the filename
  appPath=getFileFromPath(appPath.c_str());

  // get our configuration
  //config=new CGIConfiguration(LEMURCGI_CONFIG_PATH);
  CGIConfiguration::getInstance().loadConfiguration(LEMURCGI_CONFIG_PATH);

  // create our output object
  CGIOutput output(appPath);

  // get the parameters
  if (!getCGIParams(&output, cgi_request_method)) {
    output.writeErrorMessage("Unsuppored Request Method.", "Error:  Unsupported request method. Please notify Web site administrator.");
  } else {
    timer_thread_info *info = new timer_thread_info;
    info->output = &output;
    info->timeout = CGIConfiguration::getInstance().getQueryTimeout();
    if (info->timeout <= 0) info->timeout = DEFAULT_TIMEOUT;
    indri::thread::Thread *thread = new indri::thread::Thread(timer_thread,
							      info);
    info->thread = thread;
    processRequest(&output);
  }

  queryParameters.clear();

  // all finished... exit.
  return 0;
}

