#ifndef _CGIOUTPUT_H
#define _CGIOUTPUT_H

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ifstream;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <iomanip>
using std::setw;

#include "LemurSearchCGIConstants.h"
#include "CGIConfiguration.h"
#include "DictionaryHash.h"
#include "SingleResultItem.h"

/* our template definitions */

/** the name of the results page template */
#define RESULTSPAGE_TEMPLATE_NAME       "ResultsPage.html"

/** the name of the single result template */
#define SINGLERESULT_TEMPLATE_NAME      "SingleResult.html"

/** the name of the generic page template */
#define GENERICPAGE_TEMPLATE_NAME       "GenericPage.html"

/** the name of the default search page template */
#define DEFAULTSEARCHPAGE_TEMPLATE_NAME "SearchPage.html"

/** the name of the default error page template */
#define ERRORPAGE_TEMPLATE_NAME         "ErrorPage.html"

/** the default page title to use if none is set */
#define DEFAULT_RESULTS_PAGE_TITLE      "Lemur Search"

/**
 * An enumeration describing the output format.
 * For standard interactive web use, use CGI_OUTPUT_INTERACTIVE. To return
 * results in a plaintext format, use CGI_OUTPUT_DIAGNOSTIC. Finally, to return
 * the results as a single stream of data with absolutely no formatting, use CGI_OUTPUT_PROGRAM.
 */
enum CGIOutputMode {
  CGI_OUTPUT_INTERACTIVE=1,
  CGI_OUTPUT_DIAGNOSTIC,
  CGI_OUTPUT_PROGRAM
};

/**
 * Our output conduit from the program to the world (or http server, most likely).
 *
 * @author Mark J. Hoy [http://www.cs.cmu.edu/~mhoy/]
 * @version 2/28/06
 */
class CGIOutput {
private:
  CGIOutputMode outputMode;

  string  scriptURL;
  int     maxResultsPerPage;
  string  queryTerms;
  
  string loadedPageTemplateStart;
  string loadedPageTemplateEnd;
  string resultItemTemplate;

  int     currentDatasourceID;
  int     startResultNum;
  int     endResultNum;
  int     totalResultNum;
  int     currentNumDisplayedResults;

  char toHex(char c);
  char fromHex(char c1, char c2);

  string loadPageTemplate(string templateFile);

  void replaceTemplateCommand(string *templatePage, string variableName, string replacement);
  void replaceCompileDateVersion(string *templatePage);
  string getTemplateParameter(string *templatePage, string parameterName);

  string processParameterCommand(string command, string paramValue);

  void replaceResultsPageParameters(string *templatePage);

  DictionaryHash substitutionValues;

  bool    haveHeadersBeenSent;

public:
  /**
   * Constructor
   *
   * @param cgiURL the path to the CGI script (usually given via argv[0] on main)
   */
  CGIOutput(string cgiURL);

  /**
   * Destructor
   */
  ~CGIOutput();

  void stringReplaceAll(string *s, string before, string after);

  /**
   * Sets our output mode.
   *
   * @param mode the output mode to use
   */
  void setOutputMode(CGIOutputMode mode) {
    outputMode=mode;
  }

  /**
   * Retrieves the current output mode
   *
   * @return the currently set output mode
   */
  CGIOutputMode getOutputMode() {
    return outputMode; 
  }

  /**
   * Sets the maximum number of results per page
   *
   * @param numPerPage the maximum number of results per page to set
   */
  void setMaxResultsPerPage(long numPerPage) {
    maxResultsPerPage=numPerPage;
  }

  /**
   * Encodes a string to be passed as a URL encoded string.
   * 
   * @param s the input string to encode
   * @return the encoded string
   */
  string URLEncodeString(string s);

  /**
   * Decodes a string from URL encoded string.
   * 
   * @param s the input string to decode
   * @return the decoded string
   */
  string URLDecodeString(string s);

  /**
   * Outputs a beginning HTTP header. The content type should be a valid
   * mime-type, most commonly &quot;text/plain&quot; or &quot;text/html&quot;
   *
   * @param contentType the mime-type of the content.
   */
  void outputHTTPHeader(string contentType);

  /**
   * Writes an error page with a specified title and message.
   *
   * @param errorTitle the title for the error page.
   * @param errorMessage the message to display.
   */
  void writeErrorMessage(string errorTitle, string errorMessage);

  /**
   * Writes a standard footer out (in the case that there is no other footer)
   */
  void writePlaintextFooter();

  /**
   * Writes a string out to the stream.
   *
   * @param stringToWrite the string to write out
   */
  void outputString(string stringToWrite);

  /**
   * Displays the default search page based on the search page template.
   */
  void displayDefaultSearchPage();

  /**
   * Resets any result lists and loads in the default results templates
   * 
   * @returns true on success, false on failure (i.e. could not load a template)
   */
  bool resetResultsPage();

  /**
   * Sets the result query.
   *
   * @param query the query string
   */
  void setResultQuery(string &query);

  /**
   * Sets the result statistics (i.e. result start #, end # and total results) for the page.
   *
   * @param start the starting result number
   * @param end the ending result number
   * @param total the total number of results in the entire set
   */
  void setResultStatistics(int datasourceID, int start, int end, int total);

  /**
   * Displays the beginning of a results page (up until the {%LemurSearchResults%} tag)
   */
  void displayResultsPageBeginning();

  /**
   * Displays the ending of a results page (after the {%LemurSearchResults%} tag)
   */
  void displayResultsPageEnding();

  /**
   * Display a listing of known indexes from the configuration
   */
  void displayIndexListingPage();

  /**
   * Displays a single generic page of data with an optional title.
   *
   * @param stringOfData the pre-formatted data to display on the page
   * @param pageTitle (optional) the title of the page
   * @param listData (optional) tells the output to prepare for a list of data (true) or a page of text (false)
   */
  void displayDataPage(string stringOfData, string pageTitle="", bool listData=true);

  /**
   * Displays a single search result for the results page.
   *
   * @param resultURL the URL of this result (to use for the A HREF link)
   * @param origURL the original URL where the result was gathered from (for site search applications)
   * @param resultTitle the title to use for the search result
   * @param resultSummary the summary for the result
   * @param resultScore the score of the result
   * @param datasource the datasource ID (index number) where the result came from
   * @param resultID the internal Lemur ID of the result
   * @return true if more results can be written to this page, false if the limit has been reached (with this result page).
   */
  bool writeSearchResult(string resultURL, string origURL, string resultTitle, string resultSummary, double resultScore, int datasource=0, int resultID=0);

  bool headersSent() { return haveHeadersBeenSent; }

}; // end class CGIOutput

#endif //_CGIOUTPUT_H


