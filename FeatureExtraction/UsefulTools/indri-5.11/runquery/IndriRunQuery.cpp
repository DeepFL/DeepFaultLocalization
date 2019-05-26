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
// runquery
//
// 24 February 2004 -- tds
//
// 18 August 2004 -- dam
// incorporated multiple query, query expansion, and TREC output support
//
//
// Indri local machine query application
//
/*! \page IndriRunQuery Indri Query Retrieval
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
multiple times. The query element may take numerous optional parameters. 
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
<dt>inex participant-id</dt>
<dd>triggers output of results in INEX format and specifies the participant-id attribute used in submissions.
Specified as &lt;inex&gt;&lt;particpantID&gt;someID&lt;/participantID&gt;&lt;inex&gt; in the parameter file and as
<tt>-inex.participantID=someID</tt> on the command line.
</dd>
<dt>inex task</dt>
<dd>triggers output of results in INEX format and specifies the task attribute (default CO.Thorough).
Specified as &lt;inex&gt;&lt;task&gt;someTask&lt;/task&gt;&lt;inex&gt; in the parameter file and as
<tt>-inex.task=someTask</tt> on the command line.
</dd>
<dt>inex query</dt>
<dd>triggers output of results in INEX format and specifies the query attribute (default automatic).
Specified as &lt;inex&gt;&lt;query&gt;someQueryType&lt;/query&gt;&lt;inex&gt; in the parameter file and as
<tt>-inex.query=someQueryType</tt> on the command line.
</dd>
<dt>inex topic-part</dt>
<dd>triggers output of results in INEX format and specifies the topic-part attribute (default T).
Specified as &lt;inex&gt;&lt;topicPart&gt;someTopicPart&lt;/topicPart&gt;&lt;inex&gt; in the parameter file and as
<tt>-inex.topicPart=someTopicPart</tt> on the command line.
</dd>
<dt>inex description</dt>
<dd>triggers output of results in INEX format and specifies the contents of the description tag.
Specified as &lt;inex&gt;&lt;description&gt;some description&lt;/description&gt;&lt;inex&gt; in the parameter file and as
<tt>-inex.description="some description"</tt> on the command line.
</dd>
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
*/

#include <time.h>
#include "indri/QueryEnvironment.hpp"
#include "indri/LocalQueryServer.hpp"
#include "indri/delete_range.hpp"
#include "indri/NetworkStream.hpp"
#include "indri/NetworkMessageStream.hpp"
#include "indri/NetworkServerProxy.hpp"

#include "indri/ListIteratorNode.hpp"
#include "indri/ExtentInsideNode.hpp"
#include "indri/DocListIteratorNode.hpp"
#include "indri/FieldIteratorNode.hpp"

#include "indri/Parameters.hpp"

#include "indri/ParsedDocument.hpp"
#include "indri/Collection.hpp"
#include "indri/CompressedCollection.hpp"
#include "indri/TaggedDocumentIterator.hpp"
#include "indri/XMLNode.hpp"

#include "indri/QueryExpander.hpp"
#include "indri/RMExpander.hpp"
#include "indri/PonteExpander.hpp"
// need a QueryExpanderFactory....
#include "indri/TFIDFExpander.hpp"

#include "indri/IndriTimer.hpp"
#include "indri/UtilityThread.hpp"
#include "indri/ScopedLock.hpp"
#include "indri/delete_range.hpp"
#include "indri/SnippetBuilder.hpp"

#include <queue>

static bool copy_parameters_to_string_vector( std::vector<std::string>& vec, indri::api::Parameters p, const std::string& parameterName ) {
  if( !p.exists(parameterName) )
    return false;

  indri::api::Parameters slice = p[parameterName];

  for( size_t i=0; i<slice.size(); i++ ) {
    vec.push_back( slice[i] );
  }

  return true;
}

struct query_t {
  struct greater {
    bool operator() ( query_t* one, query_t* two ) {
      return one->index > two->index;
    }
  };

  query_t( int _index, std::string _number, const std::string& _text, const std::string &queryType,  std::vector<std::string> workSet,   std::vector<std::string> FBDocs) :
    index( _index ),
    number( _number ),
    text( _text ), qType(queryType), workingSet(workSet), relFBDocs(FBDocs)
  {
  }

  query_t( int _index, std::string _number, const std::string& _text ) :
    index( _index ),
    number( _number ),
    text( _text )
  {
  }

  std::string number;
  int index;
  std::string text;
  std::string qType;
  // working set to restrict retrieval
  std::vector<std::string> workingSet;
  // Rel fb docs
  std::vector<std::string> relFBDocs;
};

class QueryThread : public indri::thread::UtilityThread {
private:
  indri::thread::Lockable& _queueLock;
  indri::thread::ConditionVariable& _queueEvent;
  std::queue< query_t* >& _queries;
  std::priority_queue< query_t*, std::vector< query_t* >, query_t::greater >& _output;

  indri::api::QueryEnvironment _environment;
  indri::api::Parameters& _parameters;
  int _requested;
  int _initialRequested;

  bool _printDocuments;
  bool _printPassages;
  bool _printSnippets;
  bool _printQuery;

  std::string _runID;
  bool _trecFormat;
  bool _inexFormat;

  indri::query::QueryExpander* _expander;
  std::vector<indri::api::ScoredExtentResult> _results;
  indri::api::QueryAnnotation* _annotation;

  // Runs the query, expanding it if necessary.  Will print output as well if verbose is on.
  void _runQuery( std::stringstream& output, const std::string& query,
                  const std::string &queryType, const std::vector<std::string> &workingSet, std::vector<std::string> relFBDocs ) {
    try {
      if( _printQuery ) output << "# query: " << query << std::endl;
      std::vector<lemur::api::DOCID_T> docids;;
      if (workingSet.size() > 0) 
        docids = _environment.documentIDsFromMetadata("docno", workingSet);

      if (relFBDocs.size() == 0) {
          if( _printSnippets ) {
            if (workingSet.size() > 0) 
              _annotation = _environment.runAnnotatedQuery( query, docids, _initialRequested, queryType ); 
            else
              _annotation = _environment.runAnnotatedQuery( query, _initialRequested );
            _results = _annotation->getResults();
          } else {
            if (workingSet.size() > 0)
              _results = _environment.runQuery( query, docids, _initialRequested, queryType );
            else
              _results = _environment.runQuery( query, _initialRequested, queryType );
          }
      }
      
      if( _expander ) {
        std::vector<indri::api::ScoredExtentResult> fbDocs;
        if (relFBDocs.size() > 0) {
          docids = _environment.documentIDsFromMetadata("docno", relFBDocs);
          for (size_t i = 0; i < docids.size(); i++) {
            indri::api::ScoredExtentResult r(0.0, docids[i]);
            fbDocs.push_back(r);
          }
        }
        std::string expandedQuery;
        if (relFBDocs.size() != 0)
          expandedQuery = _expander->expand( query, fbDocs );
        else
          expandedQuery = _expander->expand( query, _results );
        if( _printQuery ) output << "# expanded: " << expandedQuery << std::endl;
        if (workingSet.size() > 0) {
          docids = _environment.documentIDsFromMetadata("docno", workingSet);
          _results = _environment.runQuery( expandedQuery, docids, _requested, queryType );
        } else {
          _results = _environment.runQuery( expandedQuery, _requested, queryType );
        }
      }
    }
    catch( lemur::api::Exception& e )
    {
      _results.clear();
      LEMUR_RETHROW(e, "QueryThread::_runQuery Exception");
    }
  }

  void _printResultRegion( std::stringstream& output, std::string queryIndex, int start, int end  ) {
    std::vector<std::string> documentNames;
    std::vector<indri::api::ParsedDocument*> documents;

    std::vector<indri::api::ScoredExtentResult> resultSubset;

    resultSubset.assign( _results.begin() + start, _results.begin() + end );


    // Fetch document data for printing
    if( _printDocuments || _printPassages || _printSnippets ) {
      // Need document text, so we'll fetch the whole document
      documents = _environment.documents( resultSubset );
      documentNames.clear();

      for( size_t i=0; i<resultSubset.size(); i++ ) {
        indri::api::ParsedDocument* doc = documents[i];
        std::string documentName;

        indri::utility::greedy_vector<indri::parse::MetadataPair>::iterator iter = std::find_if( documents[i]->metadata.begin(),
          documents[i]->metadata.end(),
          indri::parse::MetadataPair::key_equal( "docno" ) );

        if( iter != documents[i]->metadata.end() )
          documentName = (char*) iter->value;

        // store the document name in a separate vector so later code can find it
        documentNames.push_back( documentName );
      }
    } else {
      // We only want document names, so the documentMetadata call may be faster
      documentNames = _environment.documentMetadata( resultSubset, "docno" );
    }

    std::vector<std::string> pathNames;
    if ( _inexFormat ) {
      // retrieve path names
      pathNames = _environment.pathNames( resultSubset );
    }

    // Print results
    for( size_t i=0; i < resultSubset.size(); i++ ) {
      int rank = start+i+1;
      std::string queryNumber = queryIndex;

      if( _trecFormat ) {
        // TREC formatted output: queryNumber, Q0, documentName, rank, score, runID
        output << queryNumber << " "
                << "Q0 "
                << documentNames[i] << " "
                << rank << " "
                << resultSubset[ i ].score << " "
                << _runID << std::endl;
      } else if( _inexFormat ) {

  output << "    <result>" << std::endl
         << "      <file>" << documentNames[i] << "</file>" << std::endl
         << "      <path>" << pathNames[i] << "</path>" << std::endl
         << "      <rsv>" << resultSubset[i].score << "</rsv>"  << std::endl
         << "    </result>" << std::endl;
      }
      else {
        // score, documentName, firstWord, lastWord
        output << resultSubset[i].score << "\t"
                << documentNames[i] << "\t"
                << resultSubset[i].begin << "\t"
                << resultSubset[i].end << std::endl;
      }

      if( _printDocuments ) {
        output << documents[i]->text << std::endl;
      }

      if( _printPassages ) {
        int byteBegin = documents[i]->positions[ resultSubset[i].begin ].begin;
        int byteEnd = documents[i]->positions[ resultSubset[i].end-1 ].end;
        output.write( documents[i]->text + byteBegin, byteEnd - byteBegin );
        output << std::endl;
      }

      if( _printSnippets ) {
        indri::api::SnippetBuilder builder(false);
        output << builder.build( resultSubset[i].document, documents[i], _annotation ) << std::endl;
      }

      if( documents.size() )
        delete documents[i];
    }
  }

  void _printResults( std::stringstream& output, std::string queryNumber ) {
    if (_inexFormat) {
      // output topic header
      output << "  <topic topic-id=\"" << queryNumber << "\">" << std::endl
             << "    <collections>" << std::endl
             << "      <collection>ieee</collection>" << std::endl
             << "    </collections>" << std::endl;
    }
    for( size_t start = 0; start < _results.size(); start += 50 ) {
      size_t end = std::min<size_t>( start + 50, _results.size() );
      _printResultRegion( output, queryNumber, start, end );
    }
    if( _inexFormat ) {
      output << "  </topic>" << std::endl;
    }
    delete _annotation;
    _annotation = 0;
  }


public:
  QueryThread( std::queue< query_t* >& queries,
               std::priority_queue< query_t*, std::vector< query_t* >, query_t::greater >& output,
               indri::thread::Lockable& queueLock,
               indri::thread::ConditionVariable& queueEvent,
               indri::api::Parameters& params ) :
    _queries(queries),
    _output(output),
    _queueLock(queueLock),
    _queueEvent(queueEvent),
    _parameters(params),
    _expander(0),
    _annotation(0)
  {
  }

  ~QueryThread() {
  }

  UINT64 initialize() {
    try {        
    _environment.setSingleBackgroundModel( _parameters.get("singleBackgroundModel", false) );

    std::vector<std::string> stopwords;
    if( copy_parameters_to_string_vector( stopwords, _parameters, "stopper.word" ) )
      _environment.setStopwords(stopwords);

    std::vector<std::string> smoothingRules;
    if( copy_parameters_to_string_vector( smoothingRules, _parameters, "rule" ) )
      _environment.setScoringRules( smoothingRules );

   if( _parameters.exists( "index" ) ) {
      indri::api::Parameters indexes = _parameters["index"];

      for( size_t i=0; i < indexes.size(); i++ ) {
        _environment.addIndex( std::string(indexes[i]) );
      }
    }

    if( _parameters.exists( "server" ) ) {
      indri::api::Parameters servers = _parameters["server"];

      for( size_t i=0; i < servers.size(); i++ ) {
        _environment.addServer( std::string(servers[i]) );
      }
    }

    if( _parameters.exists("maxWildcardTerms") )
        _environment.setMaxWildcardTerms(_parameters.get("maxWildcardTerms", 100));

    _requested = _parameters.get( "count", 1000 );
    _initialRequested = _parameters.get( "fbDocs", _requested );
    _runID = _parameters.get( "runID", "indri" );
    _trecFormat = _parameters.get( "trecFormat" , false );
    _inexFormat = _parameters.exists( "inex" );

    _printQuery = _parameters.get( "printQuery", false );
    _printDocuments = _parameters.get( "printDocuments", false );
    _printPassages = _parameters.get( "printPassages", false );
    _printSnippets = _parameters.get( "printSnippets", false );

    if (_parameters.exists("baseline")) {
      // doing a baseline
      std::string baseline = _parameters["baseline"];
      _environment.setBaseline(baseline);
      // need a factory for this...
      if( _parameters.get( "fbDocs", 0 ) != 0 ) {
        // have to push the method in...
        std::string rule = "method:" + baseline;
        _parameters.set("rule", rule);
        _expander = new indri::query::TFIDFExpander( &_environment, _parameters );
      }
    } else {
      if( _parameters.get( "fbDocs", 0 ) != 0 ) {
        _expander = new indri::query::RMExpander( &_environment, _parameters );
      }
    }

    if (_parameters.exists("maxWildcardTerms")) {
      _environment.setMaxWildcardTerms((int)_parameters.get("maxWildcardTerms"));
    }    
    } catch ( lemur::api::Exception& e ) {      
      while( _queries.size() ) {
        query_t *query = _queries.front();
        _queries.pop();
        _output.push( new query_t( query->index, query->number, "query: " + query->number + " QueryThread::_initialize exception\n" ) );
        _queueEvent.notifyAll();
        LEMUR_RETHROW(e, "QueryThread::_initialize");
      }
    }
    return 0;
  }

  void deinitialize() {
    delete _expander;
    _environment.close();
  }

  bool hasWork() {
    indri::thread::ScopedLock sl( &_queueLock );
    return _queries.size() > 0;
  }

  UINT64 work() {
    query_t* query;
    std::stringstream output;

    // pop a query off the queue
    {
      indri::thread::ScopedLock sl( &_queueLock );
      if( _queries.size() ) {
        query = _queries.front();
        _queries.pop();
      } else {
        return 0;
      }
    }

    // run the query
    try {
      if (_parameters.exists("baseline") && ((query->text.find("#") != std::string::npos) || (query->text.find(".") != std::string::npos)) ) {
        LEMUR_THROW( LEMUR_PARSE_ERROR, "Can't run baseline on this query: " + query->text + "\nindri query language operators are not allowed." );
      }
      _runQuery( output, query->text, query->qType, query->workingSet, query->relFBDocs );
    } catch( lemur::api::Exception& e ) {
      output << "# EXCEPTION in query " << query->number << ": " << e.what() << std::endl;
    }

    // print the results to the output stream
    _printResults( output, query->number );

    // push that data into an output queue...?
    {
      indri::thread::ScopedLock sl( &_queueLock );
      _output.push( new query_t( query->index, query->number, output.str() ) );
      _queueEvent.notifyAll();
    }

    delete query;
    return 0;
  }
};

void push_queue( std::queue< query_t* >& q, indri::api::Parameters& queries,
                 int queryOffset ) {

  for( size_t i=0; i<queries.size(); i++ ) {
    std::string queryNumber;
    std::string queryText;
    std::string queryType = "indri";
    if( queries[i].exists( "type" ) )
      queryType = (std::string) queries[i]["type"];
    if (queries[i].exists("text"))
      queryText = (std::string) queries[i]["text"];
    if( queries[i].exists( "number" ) ) {
      queryNumber = (std::string) queries[i]["number"];
    } else {
      int thisQuery=queryOffset + int(i);
      std::stringstream s;
      s << thisQuery;
      queryNumber = s.str();
    }
    if (queryText.size() == 0)
      queryText = (std::string) queries[i];

    // working set and RELFB docs go here.
    // working set to restrict retrieval
    std::vector<std::string> workingSet;
    // Rel fb docs
    std::vector<std::string> relFBDocs;
    copy_parameters_to_string_vector( workingSet, queries[i], "workingSetDocno" );
    copy_parameters_to_string_vector( relFBDocs, queries[i], "feedbackDocno" );

    q.push( new query_t( i, queryNumber, queryText, queryType, workingSet, relFBDocs ) );

  }
}

int main(int argc, char * argv[]) {
  try {
    indri::api::Parameters& param = indri::api::Parameters::instance();
    param.loadCommandLine( argc, argv );

    if( param.get( "version", 0 ) ) {
      std::cout << INDRI_DISTRIBUTION << std::endl;
    }

    if( !param.exists( "query" ) )
      LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must specify at least one query." );

    if( !param.exists("index") && !param.exists("server") )
      LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must specify a server or index to query against." );

    if (param.exists("baseline") && param.exists("rule"))
      LEMUR_THROW( LEMUR_BAD_PARAMETER_ERROR, "Smoothing rules may not be specified when running a baseline." );

    int threadCount = param.get( "threads", 1 );
    std::queue< query_t* > queries;
    std::priority_queue< query_t*, std::vector< query_t* >, query_t::greater > output;
    std::vector< QueryThread* > threads;
    indri::thread::Mutex queueLock;
    indri::thread::ConditionVariable queueEvent;

    // push all queries onto a queue
    indri::api::Parameters parameterQueries = param[ "query" ];
    int queryOffset = param.get( "queryOffset", 0 );
    push_queue( queries, parameterQueries, queryOffset );
    int queryCount = (int)queries.size();

    // launch threads
    for( int i=0; i<threadCount; i++ ) {
      threads.push_back( new QueryThread( queries, output, queueLock, queueEvent, param ) );
      threads.back()->start();
    }

    int query = 0;

    bool inexFormat = param.exists( "inex" );
    if( inexFormat ) {
      std::string participantID = param.get( "inex.participantID", "1");
      std::string runID = param.get( "runID", "indri" );
      std::string inexTask = param.get( "inex.task", "CO.Thorough" );
      std::string inexTopicPart = param.get( "inex.topicPart", "T" );
      std::string description = param.get( "inex.description", "" );
      std::string queryType = param.get("inex.query", "automatic");
      std::cout << "<inex-submission participant-id=\"" << participantID
    << "\" run-id=\"" << runID
    << "\" task=\"" << inexTask
    << "\" query=\"" << queryType
    << "\" topic-part=\"" << inexTopicPart
    << "\">" << std::endl
    << "  <description>" << std::endl << description
    << std::endl << "  </description>" << std::endl;
    }

    // acquire the lock.
    queueLock.lock();

    // process output as it appears on the queue
    while( query < queryCount ) {
      query_t* result = NULL;

      // wait for something to happen
      queueEvent.wait( queueLock );

      while( output.size() && output.top()->index == query ) {
        result = output.top();
        output.pop();

        queueLock.unlock();

        std::cout << result->text;
        delete result;
        query++;

        queueLock.lock();
      }
    }
    queueLock.unlock();

    if( inexFormat ) {
      std::cout << "</inex-submission>" << std::endl;
    }

    // join all the threads
    for( size_t i=0; i<threads.size(); i++ )
      threads[i]->join();

    // we've seen all the query output now, so we can quit
    indri::utility::delete_vector_contents( threads );
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  } catch( ... ) {
    std::cout << "Caught unhandled exception" << std::endl;
    return -1;
  }

  return 0;
}

