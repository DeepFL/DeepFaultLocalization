<?php
  //
  // index: Complete path to the Indri index to be queried.
  //        Make sure that the web server has permission to read
  //        this index, and that there are no processes writing to
  //        this index.
  //

//  $indri_param['index'] = "/usr/ind1/tmp2/dfisher/cs-crawl2";
// swap in new crawl
  $indri_param['index'] = "/usr/ind1/tmp2/dfisher/cs-crawl-linkz";

  //
  // document_format: Use 'text' if the document is a text file (or 
  //        an XML file, but you want to show the tags in the browser).
  //        Use 'html' to attempt to render the document in the browser.
  //

  $indri_param['document_format'] = "html";

  //
  // sitename: the name of the top level host to search.
  //

  $indri_param['sitename'] = "www.cs.umass.edu";

  //
  // search_text: Text to be printed under the search box
  //

  $indri_param['search_text'] = <<<END
<p><a href="http://www.lemurproject.org/lemur/IndriQueryLanguage.html">Indri query language documentation</a>
END
  ;

  //
  // library: Location and name of the Indri PHP shared library,
  //          probably called 'libindri.so'.  Check your PHP
  //          installation to see where this library needs to be stored.
  //

  $indri_param['library'] = "libindri_php.so";

  //
  // snippet_length: Maximum length, in words, of the snippet that
  //          appears below each search result.
  //

  $indri_param['snippet_length'] = 20;

  //
  // memory_limit: Maximum number of bytes that the PHP process should
  //          use while processing query requests.
  //

  $indri_param['memory_limit'] = 1024*1024*1024;

  //
  // max_execution_time: Maximum number of sectonds that the PHP process should
  //          use while processing query requests.
  //

  $indri_param['max_execution_time'] = 300;

  //
  // page_docs: Number of documents to display on each query result page.
  //

  $indri_param['page_docs'] = 10;
?>

