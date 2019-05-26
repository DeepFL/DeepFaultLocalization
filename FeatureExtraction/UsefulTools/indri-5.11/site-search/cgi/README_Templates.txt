
LemurCGI template variable mini-help			 

All commands and variables must be encapsulated with the 
curly-brace ("{") / percent ("%") characters and cannot span 
multiple lines. For example: {%LemurSearchTitle%}. Please see
the templates provided with this package as an example.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ResultsPage.html Template 

This template defines what the search results will look like 
when returned back to the user. 

Available commands: 

LemurSearchSetTitle(x): Sets the page-wide title for this template. 

LemurSearchTitle: displays the page title. 

LemurSearchBox(x): displays a search box with the specified width. 

LemurSearchQueryTerms: displays the query term(s) searched for. 

LemurSearchResultsStartNum: displays the starting result number for
                            this page.

LemurSearchResultsEndNum: displays the ending result number for this page.

LemurSearchResultsTotalNum: displays the total number of results for
                            the search.

LemurSearchResults: Displays the search results. Individual results are
                    defined by the SingleResult.html template.

LemurSearchResultsPreviousPage(): Displays a previous page link (if
                                  available).

LemurSearchResultsNextPage(): Displays a next page link (if availiable)

LemurSearchResultPageNumbers(): Displays linked page numbers 

LemurSearchCompileDate: Displays the compilation date. 

LemurSearchVersion: Displays the version number. 

LemurCGIItems: used in the GenericPage as a placeholder for whatever needs
               to be here. For example, an inverted list for a term.
 
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- 
 
SingleResult.html Template 
 
This template defines the style of a single search result. 
By default, all search results are encapsulated within a 
<ol type=1 start=n></ol> tags with <li></li> tags around 
each individual search result. 
 
Available commands: 
 
ResOrigURL: Displays the origional URL for a result. 
 
ResURL: Displays the URL for a result. 
 
ResTitle: Displays the result title. 
 
ResSummary: Displays a summary for the result. 
 
ResCachedURL: Displays the URL for a cached result entry 
 
ResScore: Displays the result's score. 

