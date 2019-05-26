<?php
  //
  // index: Complete path to the Indri index to be queried.
  //        Make sure that the web server has permission to read
  //        this index, and that there are no processes writing to
  //        this index.
  //

  $indri_param['index'] = "/usr/ind1/tmp1/indri/strohman/index/shakespeare";

  //
  // document_format: Use 'text' if the document is a text file (or 
  //        an XML file, but you want to show the tags in the browser).
  //        Use 'html' to attempt to render the document in the browser.
  //

  $indri_param['document_format'] = "text";

  //
  // search_text: Text to be printed under the search box
  //

  $indri_param['search_text'] = <<<END
<p><strong>The Complete Plays of William Shakespeare</strong><br/></p>

Try the following queries:<br/>
<br/>
<a href="query.php?query=juliet">juliet</a><br/>
<em>Find documents containing Juliet.</em><br/>
<br/>

<a href="query.php?query=%231(romeo%20and%20juliet).title">#1(romeo and juliet).title</a><br/>
<em>Find documents containing the phrase<br/>
'Romeo and Juliet' as a title.</em> <br/>
<br/>

<a href="query.php?query=%231(what%20light%20through%20yonder%20window%20breaks)">#1(what light through yonder window breaks)</a><br/>
<em>Find documents containing the phrase<br/>
'what light through yonder window breaks'.</em><br/>
<br/>

<a href="query.php?query=%23combine%5Bspeech%5D(%20%23uw(%20%235(struts%20frets)%20%231(out%20out)%20stage%20)%20)">#combine[speech]( #uw( #5(struts frets) #1(out out) stage ) )</a><br/>
<em>Find spoken lines containing the phrases 'out out', 'stage',<br/>
and the words 'struts' and 'frets' within 5 words of each other.</em><br/>
<br/>

<a href="query.php?query=%23combine%5Bspeech%5D(juliet.speaker%20romeo)">#combine[speech]( juliet.speaker romeo )</a><br/>
<em>Find lines that are either spoken by Juliet or that mention Romeo;<br/>
lines that meet both criteria should be ranked highest.</em>
<br/>
<br/>
<a href="http://www.lemurproject.org/lemur/IndriQueryLanguage.html">Query language documentation</a>
<br/>
<br/>
All document tags can be used in queries; <br/>
Look at the plays by clicking the Cached links in the<br/>
results page in order to understand what tags are available.<br/>

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

  $indri_param['snippet_length'] = 250;

  //
  // memory_limit: Maximum number of bytes that the PHP process should
  //          use while processing query requests.
  //

  $indri_param['memory_limit'] = 1024*1024*1024;

  //
  // page_docs: Number of documents to display on each query result page.
  //

  $indri_param['page_docs'] = 10;
?>

