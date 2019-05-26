/*==========================================================================
 * Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/

//
// buildindex
//
// 10 February 2004 -- tds
//
// Indri index build driver application
//
/*! \page IndriParameters Indri Parameter Files

<P>The indri applications, IndriBuildIndex, IndriDaemon, and
IndriRunQuery accept parameters from either the command line or from a
file. The parameter file uses an XML format. The command line uses
dotted path notation. The top level element in the parameters file is
named <em>parameters</em>.

<H3> Repository construction parameters</h3>
<dl>
<dt>memory</dt>
<dd> an integer value specifying the number of bytes to use for the
indexing process. The value can include a scaling factor by adding a
suffix. Valid values are (case insensitive) K = 1000, M = 1000000, G =
1000000000. So 100M would be equivalent to 100000000. The value should
contain only decimal digits and the optional suffix. Specified as
&lt;memory&gt;100M&lt;/memory&gt; in the parameter file and as
<tt>-memory=100M</tt> on the command line. </dd> 
<dt>corpus</dt>
<dd>a complex element containing parameters related to a corpus. This
element can be specified multiple times. The parameters are 
<dl>
<dt>path</dt>
<dd>The pathname of the file or directory containing documents to
index. Specified as
&lt;corpus&gt;&lt;path&gt;/path/to/file_or_directory&lt;/path&gt;&lt;/corpus&gt;
in the parameter file and as
<tt>-corpus.path=/path/to/file_or_directory</tt> on the command
line.</dd> 
<dt>class</dt>
<dd>The FileClassEnviroment of the file or directory containing
documents to index. Specified as
&lt;corpus&gt;&lt;class&gt;trecweb&lt;/class&gt;&lt;/corpus&gt; in the
parameter file and as <tt>-corpus.class=trecweb</tt> on the command
line. The known classes are: 
<ul>
<li>html -- web page data.
<li>xml -- xml marked up data.
<li>trecweb -- TREC web format, eg terabyte track.
<li>trectext -- TREC format, eg TREC-3 onward.
<li>trecalt -- TREC format, eg TREC-3 onward, with only the TEXT field included.
<li>warc -- WARC (Web ARChive) format, such as can be output by the heritrix webcrawler.
<li>warcchar -- WARC (Web ARChive) format, such as can be output by the heritrix webcrawler. Tokenizes individual characters, enabling indexing of unsgemented text.
<li>doc -- Microsoft Word format (windows platform only).
<li>ppt -- Microsoft Powerpoint format (windows platform only).
<li>pdf --  Adobe PDF format.
<li>txt --  Plain text format.
</ul>
</dd>
<dt>annotations</dt>
<dd>The pathname of the file containing offset annotations for the documents 
specified in <tt>path</tt>. Specified as
&lt;corpus&gt;&lt;annotations&gt;/path/to/file&lt;/annotations&gt;&lt;/corpus&gt;
in the parameter file and as
<tt>-corpus.annotations=/path/to/file</tt> on the command
line.</dd> 
<dt>metadata</dt>
<dd>The pathname of the file or directory containing offset metadata for the 
documents specified in <tt>path</tt>. Specified as
&lt;corpus&gt;&lt;metadata&gt;/path/to/file&lt;/metadata&gt;&lt;/corpus&gt;
in the parameter file and as
<tt>-corpus.metadata=/path/to/file</tt> on the command
line.</dd>

Combining the first two of these elements, the parameter file would contain:
<br>
&lt;corpus&gt;<br>
&nbsp;&nbsp;&lt;path&gt;/path/to/file_or_directory&lt;/path&gt;<br>
&nbsp;&nbsp;&lt;class&gt;trecweb&lt;/class&gt;<br>
&lt;/corpus&gt;
</dd>
</dl>

<dt>metadata</dt>
<dd>a complex element containing one or more entries
specifying the metadata fields to index, eg title, headline.
There are three options
<ol>

<li> <tt>field</tt> -- Make the named field available for retrieval as
 metadata.  Specified as
 &lt;metadata&gt;&lt;field&gt;fieldname&lt;/field&gt;&lt;/metadata&gt;
 in the parameter file and as <tt>metadata.field=fieldname</tt> on the
 command line.

<li> <tt>forward</tt> -- Make the named field available for retrieval as
 metadata and build a lookup table to make retrieving the value more
 efficient.  Specified as
 &lt;metadata&gt;&lt;forward&gt;fieldname&lt;/forward&gt;&lt;/metadata&gt;
 in the parameter file and as <tt>metadata.forward=fieldname</tt> on the
 command line. The external document id field "docno" is automatically 
 added as a forward metadata field.

<li> <tt>backward</tt> -- Make the named field available for retrieval
 as metadata and build a lookup table for inverse lookup of documents
 based on the value of the field.  Specified as
 &lt;metadata&gt;&lt;backward&gt;fieldname&lt;/backward&gt;&lt;/metadata&gt;
 in the parameter file and as <tt>metadata.backward=fieldname</tt> on
 the command line. The external document id field "docno" is automatically 
 added as a backward metadata field.

</ol>
</dd>

<dt>field</dt>

<dd>a complex element specifying the fields to index as data, eg
TITLE. This parameter can appear multiple times in a parameter file.
<b>If provided on the command line, only the first field specified will
be indexed</b>. The subelements are:

<dl>
<dt>name</dt><dd>the field name, specified as
&lt;field&gt;&lt;name&gt;fieldname&lt;/name&gt;&lt;/field&gt; in the
parameter file and as <tt>-field.name=fieldname</tt> on the command
line.</dd> 
<dt>numeric</dt><dd>the symbol <tt>true</tt> if the field contains
numeric data, otherwise the symbol <tt>false</tt>, specified as
&lt;field&gt;&lt;numeric&gt;true&lt;/numeric&gt;&lt;/field&gt; in the
parameter file and as <tt>-field.numeric=true</tt> on the command
line. This is an optional parameter, defaulting to false. Note that <tt>0</tt>
can be used for false and <tt>1</tt> can be used for true. </dd>
<dt>parserName</dt><dd>the name of the parser to use to convert a numeric
field to an unsigned integer value. The default is NumericFieldAnnotator. If numeric field data is provided via offset annotations, you should use the value OffsetAnnotationAnnotator. If the field contains a formatted date (see <a href="DateFields.html">Date Fields</a>) you should use the value DateFieldAnnotator.
</dd>
</dl> 
</dd>
<dt>stemmer</dt>
<dd>a complex element specifying the stemming algorithm to use in the
subelement name. Valid options are:
<ul>
<li> porter -- Porter stemmer
<li> krovetz -- Krovetz stemmer
<li> arabic_stop -- Larkey stemmer, remove stopwords
<li>  arabic_norm2  -- Larkey stemmer, table normalization
<li>  arabic_norm2_stop -- Larkey stemmer, table normalization with stopping
<li>  arabic_light10 -- Larkey stemmer, light9 plus ll prefix
<li>  arabic_light10_stop -- Larkey stemmer, light10 and remove stop words
</ul>

Specified as
&lt;stemmer&gt;&lt;name&gt;stemmername&lt;/name&gt;&lt;/stemmer&gt; and
as <tt>-stemmer.name=stemmername</tt> on the command line. This is an
optional parameter with the default of no stemming.
</dd>
<dt>normalize</dt>
<dd><tt>true</tt> to perform case normalization when indexing, false to 
index with mixed case. Default <tt>true</tt>
</dd>
<dt>stopper</dt>
<dd>a complex element containing one or more subelements named word,
specifying the stopword list to use. Specified as
&lt;stopper&gt;&lt;word&gt;stopword&lt;/word&gt;&lt;/stopper&gt; and
as <tt>-stopper.word=stopword</tt> on the command line. This is an
optional parameter with the default of no stopping.</dd>
<dt>offsetannotationhint</dt>
<dd>An optional parameter to provide a hint to the indexer to speed
up indexing of offset annotations when using offset annotation files
as specified in the &lt;corpus&gt; parameter. Valid values here
are &quot;unordered&quot; and &quot;ordered&quot;. An &quot;unordered&quot;
hint (the default) will inform the indexer that the document IDs of the
annotations are not necessarily in the same order as the documents
in the corpus. The indexer will adjust its internal memory allocations
appropriately to pre-allocate enough memory before reading in the annotations
file. If you are absolutely certain that the annotations in the offset annotation
file are in the exact same order as the documents, then you can use the &quot;ordered&quot;
hint. This will tell the indexer to not read in the entire file at once, but rather
read in the offset annotations file as needed for only the annotations that
are specified for the currently indexing document ID.</dd>
</dl>

<H3>QueryEnvironment Parameters</H3>
<H4>Retrieval Parameters</H4>
<dl>
<dt>index</dt>
<dd> path to an Indri Repository. Specified as
&lt;index&gt;/path/to/repository&lt;/index&gt; in the parameter file and
as <tt>-index=/path/to/repository</tt> on the command line. This element
can be specified multiple times to combine Repositories.
</dd>
<dt>server</dt>
<dd> hostname of a host running an Indri server (IndriDaemon). Specified as
&lt;server&gt;hostname&lt;/server&gt; in the parameter file and
as <tt>-server=hostname</tt> on the command line. The hostname can
include an optional port number to connect to, using the form
<tt>hostname:portnum</tt>. This element
can be specified multiple times to combine servers.
</dd>
<dt>count</dt>
<dd>an integer value specifying the maximum number of results to
return for a given query. Specified as
&lt;count&gt;number&lt;/count&gt; in the parameter file and
as <tt>-count=number</tt> on the command line. </dd>
<dt>query</dt>
<dd>An indri query language query to run. This element can be specified
multiple times.
 The query element may take numerous optional parameters. 
With none of the optional parameters, the query text can be the body of 
the element, eg:<br> 

&lt;query&gt;#combine(query terms)&lt;/query&gt;
<p>
The optional parameters are:
<dl>
<dt>type</dt><dd>one of <tt>indri</tt>, to use the indri query language, 
or <tt>nexi</tt> to use the nexi query language. The default is 
<tt>indri</tt>. This element may appear 0 or 1 times.</dd>
<dt>number</dt><dd>The query number or identifier. This may be a 
non-numeric symbol. The default is to number the queries in the parameters in
order, starting with 0. This element may appear 0 or 1 times.</dd>
<dt>text</dt><dd>The query text, eg, "#combine(query terms)". This element 
may appear 0 or 1 times and must be used if any of the other 
parameters are supplied.</dd>
<dt>workingSetDocno</dt><dd>The external document id of a document to add to 
the working set for the query. This element may appear 0 or more times. 
When specified, query evaluation is restricted to the document ids 
specified.</dd>
<dt>feedbackDocno</dt><dd>The external document id of a document to add to 
the relevance feeedback set for the query. This element may appear 0 or more 
times. When specified, query expansion is performed using only the document 
ids specified. It is still necessary to specify a non-zero value for the
fbDocs parameter when specifying feedbackDocno elements.</dd>
</dl>

</dd>
<dt>rule</dt>
<dd>specifies the smoothing rule (TermScoreFunction) to apply. Format of
the rule is:<br> 

<tt>   ( key ":" value ) [ "," key ":" value ]* </tt>
<p>
Here's an example rule in command line format:<br>

   <tt>-rule=method:linear,collectionLambda:0.2,field:title</tt>
<p> and in parameter file format:<br>
<tt>
&lt;rule&gt;method:linear,collectionLambda:0.2,field:title&lt;/rule&gt;
</tt>

<p>This corresponds to Jelinek-Mercer smoothing with background lambda
equal to 0.2, only for items in a title field. 

<p>If nothing is listed for a key, all values are assumed.
So, a rule that does not specify a field matches all fields.  This makes
<tt>-rule=method:linear,collectionLambda:0.2</tt> a valid rule. 

<p>Valid keys:
<dl>
<dt>   method</dt><dd> smoothing method (text)</dd>
<dt>   field</dt><dd> field to apply this rule to</dd>
<dt>   operator
<dd> type of item in query to apply to { term, window }</dd>
</dl>

<p>Valid methods:
<dl>
<dt>   dirichlet</dt><dd> (also 'd', 'dir') (default mu=2500)</dd>
<dt>   jelinek-mercer</dt><dd> (also 'jm', 'linear') (default
collectionLambda=0.4, documentLambda=0.0),  collectionLambda is also
known as just "lambda", either will work </dt>
<dt>   twostage</dt><dd> (also 'two-stage', 'two') (default mu=2500,
lambda=0.4)</dd> 
</dl>
If the rule doesn't parse correctly, the default is Dirichlet, mu=2500.
</dd>
<dt>stopper</dt>
<dd>a complex element containing one or more subelements named word,
specifying the stopword list to use. Specified as
&lt;stopper&gt;&lt;word&gt;stopword&lt;/word&gt;&lt;/stopper&gt; and
as <tt>-stopper.word=stopword</tt> on the command line. This is an
optional parameter with the default of no stopping.</dd>
<dt>maxWildcardTerms</dt>
<dd>
<i>(optional)</i> An integer specifying the maximum number of wildcard terms that can 
be generated for a synonym list for this query or set of queries. If this limit
is reached for a wildcard term, an exception will be thrown. If this parameter
is not specified, a default of 100 will be used.
</dd>
</dl>
<H4>Baseline (non-LM) retrieval</H4>
<dl>
<dt>baseline</dt>
<dd>Specifies the baseline (non-language modeling) retrieval method to
apply. This enables running baseline experiments on collections too large
for the Lemur RetMethod API. When running a baseline experiment, the queries
may not contain any indri query language operators, they must contain only
terms.

<p>Format of the parameter value:<br>

<tt>   (tfidf|okapi) [ "," key ":" value ]* </tt>
<p>
Here's an example rule in command line format:<br>

   <tt>-baseline=tfidf,k1:1.0,b:0.3</tt>
<p> and in parameter file format:<br>
<tt>
&lt;baseline&gt;tfidf,k1:1.0,b:0.3&lt;/baseline&gt;
</tt>

<p>Methods:
<dl>

<dt>   tfidf</dt>

<dd> Performs retrieval via tf.idf scoring as implemented in 
lemur::retrieval::TFIDFRetMethod using BM25TF term weighting. 
Pseudo-relevance feedback may be performed via the parameters below.
<p>
<p>Parameters (optional):
<dl>
<dt>   k1</dt><dd>k1 parameter for term  weight (default 1.2)</dd>
<dt>   b</dt><dd>b parameter for term weight (default 0.75)</dd>
</dd>
</dl>
</dd>

<dt>   okapi</dt>

<dd> Performs retrieval via Okapi scoring as implemented in 
lemur::retrieval::OkapiRetMethod. Pseudo-relevance feedback may 
<bold>not</bold> be performed with this baseline method.
<p>
<p>Parameters (optional):
<dl>
<dt>   k1</dt><dd>k1 parameter for term  weight (default 1.2)</dd>
<dt>   b</dt><dd>b parameter for term weight (default 0.75)</dd>
<dt>   k3</dt><dd>k3 parameter for query term  weight (default 7) </dd>
</dl>
</dd>
</dl>
</dd>
</dl>

<H4>Formatting Parameters</H4>
<dl>
<dt>queryOffset</dt>
<dd>an integer value specifying one less than the starting query number,
eg 150 for TREC formatted output. Specified as
&lt;queryOffset&gt;number&lt;/queryOffset&gt; in the parameter file and
as <tt>-queryOffset=number</tt> on the command line.</dd>
<dt>runID</dt>
<dd>a string specifying the id for a query run, used in TREC scorable
output. Specified as
&lt;runID&gt;someID&lt;/runID&gt; in the parameter file and
as <tt>-runID=someID</tt> on the command line.</dd>
<dt>trecFormat</dt>
<dd>the symbol <tt>true</tt> to produce TREC scorable output, otherwise
the symbol <tt>false</tt>. Specified as
&lt;trecFormat&gt;true&lt;/trecFormat&gt; in the parameter file and
as <tt>-trecFormat=true</tt> on the command line.  Note that <tt>0</tt>
can be used for false, and <tt>1</tt> can be used for true.</dd>
</dl>
<H4>Pseudo-Relevance Feedback Parameters</H4>
<dl>
<dt>fbDocs</dt>
<dd>an integer specifying the number of documents to use for
feedback. Specified as 
&lt;fbDocs&gt;number&lt;/fbDocs&gt; in the parameter file and
as <tt>-fbDocs=number</tt> on the command line.</dd>
<dt>fbTerms</dt>
<dd>an integer specifying the number of terms to use for
feedback. Specified as 
&lt;fbTerms&gt;number&lt;/fbTerms&gt; in the parameter file and
as <tt>-fbTerms=number</tt> on the command line.</dd>
<dt>fbMu</dt>
<dd>a floating point value specifying the value of mu to use for
feedback. Specified as
&lt;fbMu&gt;number&lt;/fbMu&gt; in the parameter file and
as <tt>-fbMu=number</tt> on the command line.</dd>
<dt>fbOrigWeight</dt>
<dd>a floating point value in the range [0.0..1.0] specifying the weight
for the original query in the expanded query. Specified as
&lt;fbOrigWeight&gt;number&lt;/fbOrigWeight&gt; in the parameter file and
as <tt>-fbOrigWeight=number</tt> on the command line.</dd>
</dl>

<H3>IndriDaemon Parameters</H3>
<dl>
<dt>index</dt>
<dd> path to the Indri Repository to act as server for. Specified as
&lt;index&gt;/path/to/repository&lt;/index&gt; in the parameter file and
as <tt>-index=/path/to/repository</tt> on the command line.
</dd>
<dt>port</dt>
<dd> an integer value specifying the port number to use.Specified as
&lt;port&gt;number&lt;/port&gt; in the parameter file and as
<tt>-port=number</tt> on the command line. </dd> 
</dl>

*/
/*! \page IndriIndexer Indri Repository Builder
<P>
 This application builds an Indri Repository for a collection of documents.
 Parameter formats for all Indri applications are also described in
<a href="IndriParameters.html">IndriParameters.html</a>
<H3> Repository construction parameters</h3>
<dl>
<dt>memory</dt>
<dd> an integer value specifying the number of bytes to use for the
indexing process. The value can include a scaling factor by adding a
suffix. Valid values are (case insensitive) K = 1000, M = 1000000, G =
1000000000. So 100M would be equivalent to 100000000. The value should
contain only decimal digits and the optional suffix. Specified as
&lt;memory&gt;100M&lt;/memory&gt; in the parameter file and as
<tt>-memory=100M</tt> on the command line. </dd> 
<dt>corpus</dt>
<dd>a complex element containing parameters related to a corpus. This
element can be specified multiple times. The parameters are 
<dl>
<dt>path</dt>
<dd>The pathname of the file or directory containing documents to
index. Specified as
&lt;corpus&gt;&lt;path&gt;/path/to/file_or_directory&lt;/path&gt;&lt;/corpus&gt;
in the parameter file and as
<tt>-corpus.path=/path/to/file_or_directory</tt> on the command
line.</dd> 
<dt>class</dt>
<dd>The FileClassEnviroment of the file or directory containing
documents to index. Specified as
&lt;corpus&gt;&lt;class&gt;trecweb&lt;/class&gt;&lt;/corpus&gt; in the
parameter file and as <tt>-corpus.class=trecweb</tt> on the command
line. The known classes are: 
<ul>
<li>html -- web page data.
<li>xml -- xml marked up data.
<li>trecweb -- TREC web format, eg terabyte track.
<li>trectext -- TREC format, eg TREC-3 onward.
<li>trecalt -- TREC format, eg TREC-3 onward, with only the TEXT field included.
<li>warc -- WARC (Web ARChive) format, such as can be output by the heritrix webcrawler.
<li>warcchar -- WARC (Web ARChive) format, such as can be output by the heritrix webcrawler. Tokenizes individual characters, enabling indexing of unsgemented text.
<li>doc -- Microsoft Word format (windows platform only).
<li>ppt -- Microsoft Powerpoint format (windows platform only).
<li>pdf --  Adobe PDF format.
<li>txt --  Plain text format.
</ul>
</dd>
<dt>annotations</dt>
<dd>The pathname of the file containing offset annotations for the documents 
specified in <tt>path</tt>. Specified as
&lt;corpus&gt;&lt;annotations&gt;/path/to/file&lt;/annotations&gt;&lt;/corpus&gt;
in the parameter file and as
<tt>-corpus.annotations=/path/to/file</tt> on the command
line.</dd> 
<dt>metadata</dt>
<dd>The pathname of the file or directory containing offset metadata for the
documents specified in <tt>path</tt>. Specified as
&lt;corpus&gt;&lt;metadata&gt;/path/to/file&lt;/metadata&gt;&lt;/corpus&gt;
in the parameter file and as
<tt>-corpus.metadata=/path/to/file</tt> on the command
line.</dd>

Combining the first two of these elements, the parameter file would contain:
<br>
&lt;corpus&gt;<br>
&nbsp;&nbsp;&lt;path&gt;/path/to/file_or_directory&lt;/path&gt;<br>
&nbsp;&nbsp;&lt;class&gt;trecweb&lt;/class&gt;<br>
&lt;/corpus&gt;
</dd>
</dl>

<dt>metadata</dt>
<dd>a complex element containing one or more entries
specifying the metadata fields to index, eg title, headline.
There are three options
<ol>

<li> <tt>field</tt> -- Make the named field available for retrieval as
 metadata.  Specified as
 &lt;metadata&gt;&lt;field&gt;fieldname&lt;/field&gt;&lt;/metadata&gt;
 in the parameter file and as <tt>metadata.field=fieldname</tt> on the
 command line.

<li> <tt>forward</tt> -- Make the named field available for retrieval as
 metadata and build a lookup table to make retrieving the value more
 efficient.  Specified as
 &lt;metadata&gt;&lt;forward&gt;fieldname&lt;/forward&gt;&lt;/metadata&gt;
 in the parameter file and as <tt>metadata.forward=fieldname</tt> on the
 command line.

<li> <tt>backward</tt> -- Make the named field available for retrieval
 as metadata and build a lookup table for inverse lookup of documents
 based on the value of the field.  Specified as
 &lt;metadata&gt;&lt;backward&gt;fieldname&lt;/backward&gt;&lt;/metadata&gt;
 in the parameter file and as <tt>metadata.backward=fieldname</tt> on
 the command line.

</ol>
</dd>

<dt>field</dt>

<dd>a complex element specifying the fields to index as data, eg
TITLE. This parameter can appear multiple times in a parameter file.
<b>If provided on the command line, only the first field specified will
be indexed</b>. The subelements are:

<dl>
<dt>name</dt><dd>the field name, specified as
&lt;field&gt;&lt;name&gt;fieldname&lt;/name&gt;&lt;/field&gt; in the
parameter file and as <tt>-field.name=fieldname</tt> on the command
line.</dd> 
<dt>numeric</dt><dd>the symbol <tt>true</tt> if the field contains
numeric data, otherwise the symbol <tt>false</tt>, specified as
&lt;field&gt;&lt;numeric&gt;true&lt;/numeric&gt;&lt;/field&gt; in the
parameter file and as <tt>-field.numeric=true</tt> on the command
line. This is an optional parameter, defaulting to false. Note that <tt>0</tt>
can be used for false and <tt>1</tt> can be used for true. </dd>
<dt>parserName</dt><dd>the name of the parser to use to convert a numeric
field to an unsigned integer value. The default is NumericFieldAnnotator. If numeric field data is provided via offset annotations, you should use the value OffsetAnnotationAnnotator. If the field contains a formatted date (see <a href="DateFields.html">Date Fields</a>) you should use the value DateFieldAnnotator.
</dd>
</dl> 
</dd>
<dt>stemmer</dt>
<dd>a complex element specifying the stemming algorithm to use in the
subelement name. Valid options are:
<ul>
<li> porter -- Porter stemmer
<li> krovetz -- Krovetz stemmer
<li> arabic_stop -- Larkey stemmer, remove stopwords
<li>  arabic_norm2  -- Larkey stemmer, table normalization
<li>  arabic_norm2_stop -- Larkey stemmer, table normalization with stopping
<li>  arabic_light10 -- Larkey stemmer, light9 plus ll prefix
<li>  arabic_light10_stop -- Larkey stemmer, light10 and remove stop words
</ul>

Specified as
&lt;stemmer&gt;&lt;name&gt;stemmername&lt;/name&gt;&lt;/stemmer&gt; and
as <tt>-stemmer.name=stemmername</tt> on the command line. This is an
optional parameter with the default of no stemming.
</dd>
<dt>normalize</dt>
<dd><tt>true</tt> to perform case normalization when indexing, false to 
index with mixed case. Default <tt>true</tt>
</dd>
<dt>stopper</dt>
<dd>a complex element containing one or more subelements named word,
specifying the stopword list to use. Specified as
&lt;stopper&gt;&lt;word&gt;stopword&lt;/word&gt;&lt;/stopper&gt; and
as <tt>-stopper.word=stopword</tt> on the command line. This is an
optional parameter with the default of no stopping.</dd>
<dt>offsetannotationhint</dt>
<dd>An optional parameter to provide a hint to the indexer to speed
up indexing of offset annotations when using offset annotation files
as specified in the &lt;corpus&gt; parameter. Valid values here
are &quot;unordered&quot; and &quot;ordered&quot;. An &quot;unordered&quot;
hint (the default) will inform the indexer that the document IDs of the
annotations are not necessarily in the same order as the documents
in the corpus. The indexer will adjust its internal memory allocations
appropriately to pre-allocate enough memory before reading in the annotations
file. If you are absolutely certain that the annotations in the offset annotation
file are in the exact same order as the documents, then you can use the &quot;ordered&quot;
hint. This will tell the indexer to not read in the entire file at once, but rather
read in the offset annotations file as needed for only the annotations that
are specified for the currently indexing document ID.</dd>
</dl>

*/

#include <algorithm>
#include <cstring>
#include <string>
#include <cctype>

#include "indri/Parameters.hpp"
#include "indri/IndexEnvironment.hpp"
#include <time.h>
#include "indri/Path.hpp"
#include "indri/ConflationPattern.hpp"
#include "lemur/Exception.hpp"
#include "indri/FileTreeIterator.hpp"
#include <vector>
#include <map>
#include "indri/IndriTimer.hpp"

#include "indri/QueryEnvironment.hpp"
#include "indri/Thread.hpp"
#include "indri/SequentialWriteBuffer.hpp"

#include <math.h>

#include "indri/Repository.hpp"
#include "indri/CompressedCollection.hpp"
#include "indri/ScopedLock.hpp"
#include "indri/DirectoryIterator.hpp"
#include "indri/Path.hpp"

// Recover a repository that crashed during build to be consistent with
// its latest checkpoint. If it can't be recovered, create an empty one.
static bool _recoverRepository(const std::string &path) {
  indri::collection::Repository repo;
  try {
    repo.open(path);
  } catch (lemur::api::Exception &ex) {
    // failed to open, can't fix it, recreate.
    return false;
  }
  
  // count up the documents that made it to disk
  indri::collection::Repository::index_state indexes = repo.indexes();
  INT64 total = 0;
  for( size_t i = 0; i < indexes->size(); i++ ) {
    indri::thread::ScopedLock lock( (*indexes)[i]->statisticsLock() );
    total += (*indexes)[i]->documentCount();
  }
  total -= repo.deletedList().deletedCount();

  // identify the docids that are in the collection but not in a disk index
  indri::collection::CompressedCollection *col = repo.collection();
  indri::index::DeletedDocumentList del;
  bool marked = false;
  int numMarked = 0;
  for (int i = (int)total + 1; col->exists(i); i++) {
      del.markDeleted(i);
      marked = true;
      numMarked++;
  }
  // compact to delete the data associated with the unindexed docids.
  if (marked) {
    try {
      std::cerr << "Reovering Repository: " << path << "\nDeleting " 
                << numMarked << " uncommitted documents." << std::endl;
      col->compact(del);
      // check for any partial disk indexes (crash during write)
      // and remove them
      std::string indexPath = indri::file::Path::combine( path, "index" );
      indri::file::DirectoryIterator idirs( indexPath );
      while (! (idirs == indri::file::DirectoryIterator::end())) {
        // iterate over the subdirectories, removing any that don't have a 
        // manifest file.
        std::string current = *idirs;
        std::string manifest = indri::file::Path::combine(current, "manifest");
        if (!indri::file::Path::exists(manifest)) {
          std::cerr << "Removing corrupted index directory: " << current 
                    << std::endl;
          indri::file::Path::remove(current);
          }
        idirs++;
      }
    } catch (lemur::api::Exception &e) {
      // no recovery possible here...
      LEMUR_ABORT(e);
    }
  }
  repo.close();
  // successfully opened and closed
  return true;
}

static indri::utility::IndriTimer g_timer;

static void buildindex_start_time() {
  g_timer.start();
}

static void buildindex_print_status( const char* status, int count ) {
  g_timer.printElapsedSeconds(std::cout);
  std::cout << ": " << status << count << "\r";
}

static void buildindex_print_status( const char* status, int count, const char* status2, INT64 count2 ) {
  g_timer.printElapsedSeconds(std::cout);
  std::cout << ": " << status << count << status2 << count2 << "\r";
}

static void buildindex_flush_status() {
  std::cout.flush();
}

static void buildindex_print_event( const char* event ) {
  g_timer.printElapsedSeconds(std::cout);
  std::cout << ": " << event << std::endl;
}

static void buildindex_print_event( std::string event ) {
  buildindex_print_event( event.c_str() );
}

class StatusMonitor : public indri::api::IndexStatus {
  void operator() ( int code, const std::string& documentFile, const std::string& error, int documentsParsed, int documentsSeen ) {
    std::stringstream event;

    switch(code) {
      case indri::api::IndexStatus::FileOpen:
        event << "Opened " << documentFile;
        buildindex_print_event( event.str() ); 
        break;

      case indri::api::IndexStatus::FileClose:
        buildindex_print_status( "Documents parsed: ", documentsSeen, " Documents indexed: ", documentsParsed );
        buildindex_print_event( "" );
        event << "Closed " << documentFile;
        buildindex_print_event( event.str() ); 
        break;

      case indri::api::IndexStatus::FileSkip:
        event << "Skipped " << documentFile;
        buildindex_print_event( event.str() ); 
        break;

      case indri::api::IndexStatus::FileError:
        event << "Error in " << documentFile << " : " << error;
        buildindex_print_event( event.str() ); 
        break;

      default:
      case indri::api::IndexStatus::DocumentCount:
        if( !(documentsSeen % 500) ) {
          buildindex_print_status( "Documents parsed: ", documentsSeen, " Documents indexed: ", documentsParsed );
          buildindex_flush_status();
        }
        break;
    }
  }
};

static std::string downcase_string( const std::string& str ) {
  std::string result;
  result.resize( str.size() );

  for( size_t i=0; i<str.size(); i++ ) {
    result[i] = tolower(str[i]);
  }
 
  return result;
}

static void downcase_string_vector (std::vector<std::string>& vec) {
  for( size_t i=0; i<vec.size(); i++ ) {
    vec[i] = downcase_string( vec[i] );
  }
}

static bool copy_parameters_to_string_vector( std::vector<std::string>& vec, indri::api::Parameters p, const std::string& parameterName, const std::string* subName = 0 ) {
  if( !p.exists(parameterName) )
    return false;

  indri::api::Parameters slice = p[parameterName];
  
  for( size_t i=0; i<slice.size(); i++ ) {
    if( subName ) {
      if( slice[i].exists(*subName) ) {
        vec.push_back( slice[i][*subName] );
      }
    } else {
      vec.push_back( slice[i] );
    }
  }

  return true;
}

/*! Given a Specification and a field name, return a vector containing all
 * of the field names that conflate to that name as well as the original
 * name.
 * @param spec The indri::parse::FileClassEnvironmentFactory::Specification to inspect.
 * @param name The field name to look for.
 * @return a vector containing all of the field names that conflate to that name as well as the original name.
 */
static std::vector<std::string> findConflations(indri::parse::FileClassEnvironmentFactory::Specification *spec, std::string & name) {
  std::vector<std::string> retval;
  // have to walk the map and add an entry for each
  // conflation to a given name
  std::map<indri::parse::ConflationPattern*, std::string>::const_iterator iter;
  for (iter = spec->conflations.begin(); 
       iter != spec->conflations.end(); iter++) {
    if( iter->second == name )
      retval.push_back(iter->first->tag_name);
  }
  // put the original into the list
  retval.push_back(name);
  return retval;
}

/*! Add a string to a vector if not already present.
 * @return true if the string was added.
 */
static bool addNew(std::vector<std::string>& orig, 
                   std::vector<std::string>& vec, string &name, 
                   std::string &specName, const char *msg) {
  bool retval = false;
  if( std::find( orig.begin(), orig.end(), name ) == orig.end() ) {
    std::cerr << "Adding " << name << " to " << specName << msg << std::endl;
    vec.push_back(name);
    retval = true;
  }
  return retval;
}
/*! Add field names to index or metadata for an existing file class 
 * specification.
 */
static bool augmentSpec(indri::parse::FileClassEnvironmentFactory::Specification *spec,
                        std::vector<std::string>& fields,
                        std::vector<std::string>& metadata,
                        std::vector<std::string>& metadataForward,
                        std::vector<std::string>& metadataBackward ) {
  // add to index and metadata fields in spec if necessary. 
  // return true if a field is changed.
  bool retval = false;
  // input field names are potentially conflated names:
  // eg headline for head, hl, or headline tags.
  std::vector<std::string> conflations;
  std::vector<std::string>::iterator i1;
  std::vector<std::string> origIndex = spec->index;
  std::vector<std::string> origInclude = spec->include;
  
  for (i1 = fields.begin(); i1 != fields.end(); i1++) {
    // find any conflated names
    conflations = findConflations(spec, *i1);
    for (size_t j = 0; j < conflations.size(); j++) {
      // only add the field for indexing if it doesn't already exist
      if (addNew(origIndex, spec->index, conflations[j], 
                 spec->name, " as an indexed field")) {
        // added a field, make sure it is indexable
        // only add include tags if there are some already.
        // if it is empty, *all* tags are included.
        if( !spec->include.empty() ) {
          addNew(origInclude, spec->include, conflations[j], spec->name,
                 " as an included tag");
        }
        retval = true;
      }
    }
  }

  // add fields that should be marked metadata for retrieval
  for (i1 = metadata.begin(); i1 != metadata.end(); i1++) {
    // find any conflated names
    conflations = findConflations(spec, *i1);
    for (size_t j = 0; j < conflations.size(); j++)
      retval |= addNew(spec->metadata, spec->metadata, conflations[j], spec->name,
                       " as a metadata field");
  }
  // add fields that should have a metadata forward lookup table.
  for (i1 = metadataForward.begin(); i1 != metadataForward.end(); i1++) {
    // find any conflated names
    conflations = findConflations(spec, *i1);
    for (size_t j = 0; j < conflations.size(); j++) 
      retval |= addNew(spec->metadata, spec->metadata, conflations[j], spec->name,
                       " as a forward indexed metadata field");
  }
  // add fields that should have a metadata reverse lookup table.
  for (i1 = metadataBackward.begin(); i1 != metadataBackward.end(); i1++) {
    // find any conflated names
    conflations = findConflations(spec, *i1);
    for (size_t j = 0; j < conflations.size(); j++) 
      retval |= addNew(spec->metadata, spec->metadata, conflations[j], spec->name,
                       " as a backward indexed metadata field");
  }

  return retval;
}

//
// process_numeric_fields
//

static void process_numeric_fields( indri::api::Parameters parameters, indri::api::IndexEnvironment& env ) {
  std::string numName = "numeric";
  std::string subName = "name";
  indri::api::Parameters slice = parameters["field"];
  for( size_t i=0; i<slice.size(); i++ ) {
    bool isNumeric = slice[i].get(numName, false);
    if( isNumeric ) {
      // let user override default NumericFieldAnnotator for parser
      // enabling numeric fields in offset annotations.
      std::string parserName = slice[i].get("parserName", "NumericFieldAnnotator");
      std::string fieldName = slice[i][subName];
      fieldName = downcase_string( fieldName );
      env.setNumericField(fieldName, isNumeric, parserName);
    }
  }
}

//
// process_ordinal_fields
//

static void process_ordinal_fields( indri::api::Parameters parameters, indri::api::IndexEnvironment& env ) {
  std::string ordName = "ordinal";
  std::string subName = "name";
  indri::api::Parameters slice = parameters["field"];
  
  for( size_t i=0; i<slice.size(); i++ ) {
    bool isOrdinal = slice[i].get(ordName, false);

    if( isOrdinal ) {
      std::string fieldName = slice[i][subName];
      fieldName = downcase_string( fieldName );
      env.setOrdinalField(fieldName, isOrdinal);
    }
  }
}

//
// process_parental_fields
//
static void process_parental_fields( indri::api::Parameters parameters, indri::api::IndexEnvironment& env ) { // pto
  std::string parName = "parental";
  std::string subName = "name";
  indri::api::Parameters slice = parameters["field"];
  for( int i=0; i<slice.size(); i++ ) {
    bool isParental = slice[i].get(parName, false);
    if( isParental ) {
      std::string fieldName = slice[i][subName];
      fieldName = downcase_string( fieldName );
      env.setParentalField(fieldName, isParental);
    }
  }
}

void require_parameter( const char* name, indri::api::Parameters& p ) {
  if( !p.exists( name ) ) {
    LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must specify a " + name + " parameter." );
  }
}

int main(int argc, char * argv[]) {
  try {
    indri::api::Parameters& parameters = indri::api::Parameters::instance();
    parameters.loadCommandLine( argc, argv );

    require_parameter( "corpus", parameters );
    require_parameter( "index", parameters );

    StatusMonitor monitor;
    indri::api::IndexEnvironment env;
    std::string repositoryPath = parameters["index"];

    buildindex_start_time();

    if( parameters.get( "version", 0 ) ) {
      std::cout << INDRI_DISTRIBUTION << std::endl;
    }

    env.setMemory( parameters.get("memory", INT64(1024*1024*1024)) );

    env.setNormalization( parameters.get("normalize", true));
    env.setInjectURL( parameters.get("injectURL", true));
    env.setStoreDocs( parameters.get("storeDocs", true));

    std::string blackList = parameters.get("blacklist", "");
    if( blackList.length() ) {
        int count = env.setBlackList(blackList);
        std::cout << "Added to blacklist: "<< count << std::endl;
        std::cout.flush();
    }

    std::string offsetAnnotationHint=parameters.get("offsetannotationhint", "default");
    if (offsetAnnotationHint=="ordered") {
      env.setOffsetAnnotationIndexHint(indri::parse::OAHintOrderedAnnotations);
    } if (offsetAnnotationHint=="unordered") {
      env.setOffsetAnnotationIndexHint(indri::parse::OAHintSizeBuffers);
    } else {
      env.setOffsetAnnotationIndexHint(indri::parse::OAHintDefault);
    }

    std::string stemmerName = parameters.get("stemmer.name", "");
    if( stemmerName.length() )
      env.setStemmer(stemmerName);

    std::vector<std::string> stopwords;
    if( copy_parameters_to_string_vector( stopwords, parameters, "stopper.word" ) )
      env.setStopwords(stopwords);
    // fields to include as metadata (unindexed)
    std::vector<std::string> metadata;
    // metadata fields that should have a forward lookup table.
    std::vector<std::string> metadataForward;
    // metadata fields that should have a backward lookup table.
    std::vector<std::string> metadataBackward;
    copy_parameters_to_string_vector( metadata, parameters, "metadata.field" ); 
    downcase_string_vector(metadata);
    
    copy_parameters_to_string_vector( metadataForward, parameters, "metadata.forward" ); 
    downcase_string_vector(metadataForward);
    copy_parameters_to_string_vector( metadataBackward, parameters, "metadata.backward" );
    downcase_string_vector(metadataBackward);
    // docno is a special field, automagically add it as forward and backward.
    std::string docno = "docno";
    if( std::find( metadataForward.begin(), 
                   metadataForward.end(), 
                   docno ) == metadataForward.end() )
      metadataForward.push_back(docno);
    if( std::find( metadataBackward.begin(), 
                   metadataBackward.end(), 
                   docno ) == metadataBackward.end() )
      metadataBackward.push_back(docno);

    env.setMetadataIndexedFields( metadataForward, metadataBackward );
#if 0    
    // "document" is a special field.
    // automagically add it as an indexed field.
    indri::api::Parameters field = parameters.append("field");
    field.set( "name", "document" );
    field.set( "ordinal", true );
    field.set("parental", true);
#endif
    std::vector<std::string> fields;    
    std::string subName = "name";
    if( copy_parameters_to_string_vector( fields, parameters, "field", &subName ) ) {
      downcase_string_vector(fields);
      env.setIndexedFields(fields);
      process_numeric_fields( parameters, env );
      process_ordinal_fields( parameters, env );
      process_parental_fields( parameters, env ); //pto
    }

    if( indri::collection::Repository::exists( repositoryPath ) ) {
      // check if the repository was corrupted by an indexing crash
      // if so, recover it and continue.
      if (_recoverRepository(repositoryPath)) {
        env.open( repositoryPath, &monitor );
        buildindex_print_event( std::string() + "Opened repository " + repositoryPath ); 
      } else  {
        //  failed to open it, needs to be created from scratch.
        // create will remove any cruft.
        env.create( repositoryPath, &monitor );
        buildindex_print_event( std::string() + "Created repository " + repositoryPath );
      }
    } else {
      env.create( repositoryPath, &monitor );
      buildindex_print_event( std::string() + "Created repository " + repositoryPath );
    }

    indri::api::Parameters corpus = parameters["corpus"];

    for( unsigned int i=0; i<corpus.size(); i++ ) {
      indri::api::Parameters thisCorpus = corpus[i];
      require_parameter( "path", thisCorpus );
      std::string corpusPath = thisCorpus["path"];
      std::string fileClass = thisCorpus.get("class", "");
      
      // augment field/metadata tags in the environment if needed.
      if( fileClass.length() ) {
        indri::parse::FileClassEnvironmentFactory::Specification *spec = env.getFileClassSpec(fileClass);
        if( spec ) {
          // add fields if necessary, only update if changed.
          if( augmentSpec( spec, fields, metadata, metadataForward, metadataBackward ) ) 
            env.addFileClass(*spec);
          delete(spec);
        }
      }
      
      bool isDirectory = indri::file::Path::isDirectory( corpusPath );
 
      // First record the document root, and then the paths to any annotator inputs
      env.setDocumentRoot( corpusPath );

      // Support for anchor text
      std::string anchorText = thisCorpus.get("inlink", "");
      env.setAnchorTextPath( anchorText );

      // Support for offset annotations
      std::string offsetAnnotationsPath = thisCorpus.get( "annotations", "" );
      env.setOffsetAnnotationsPath( offsetAnnotationsPath );

      // Support for offset metadata file
      std::string offsetMetadataPath = thisCorpus.get( "metadata", "" );
      env.setOffsetMetadataPath( offsetMetadataPath );

      if( isDirectory ) {
        indri::file::FileTreeIterator files( corpusPath );

        for( ; files != indri::file::FileTreeIterator::end(); files++ ) {
          if( fileClass.length() )
            env.addFile( *files, fileClass );
          else {
            std::string extension = indri::file::Path::extension( *files );
            indri::parse::FileClassEnvironmentFactory::Specification *spec = env.getFileClassSpec(extension);
            if( spec ) {
              // add fields if necessary, only update if changed.
              if( augmentSpec( spec, fields, metadata, metadataForward, metadataBackward ) ) 
                env.addFileClass(*spec);
              delete(spec);
            }
            env.addFile( *files );
          }
        }
      } else {
        if( fileClass.length() )
          env.addFile( corpusPath, fileClass );
        else {
          std::string extension = indri::file::Path::extension( corpusPath );
          indri::parse::FileClassEnvironmentFactory::Specification *spec = env.getFileClassSpec(extension);
          if( spec ) {
            // add fields if necessary, only update if changed.
            if( augmentSpec( spec, fields, metadata, metadataForward, metadataBackward ) ) 
              env.addFileClass(*spec);
            delete(spec);
          }
          env.addFile( corpusPath );
        }
      }
    }

    buildindex_print_event( "Closing index" );
    env.close();
    buildindex_print_event( "Finished" );
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  }

  return 0;
}
