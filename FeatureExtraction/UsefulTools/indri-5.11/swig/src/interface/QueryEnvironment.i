#ifdef SWIGJAVA
%typemap(javaimports) indri::api::QueryEnvironment "import java.util.Map;"
#endif
%include "QueryRequest.i"
%include "QueryResults.i"
%include "Parameters.i"
namespace indri {
  namespace api {
    class QueryEnvironment {

      %newobject runAnnotatedQuery;

    public:
      QueryEnvironment();
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Add a remote server
       @param hostname the host the server is running on
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      void addServer( const std::string& hostname ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Add a local repository
       @param pathname the path to the repository.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      void addIndex( const std::string& pathname ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Remove a remote server
       @param hostname the host the server is running on
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      void removeServer( const std::string& hostname ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Remove a local repository
       @param pathname the path to the repository.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      void removeIndex( const std::string& pathname ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
       Close the QueryEnvironment.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      void close() throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Set the amount of memory to use.
       @param memory number of bytes to allocate
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif
  
      void setMemory( UINT64 memory ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Set the scoring rules
       @param rules the vector of scoring rules.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      void setScoringRules( const std::vector<std::string>& rules ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Set the stopword list for query processing
       @param stopwords the list of stopwords
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      void setStopwords( const std::vector<std::string>& stopwords ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Run an Indri query language query.
       @param query the query to run
       @param resultsRequested maximum number of results to return
       @return the vector of ScoredExtentResults for the query
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif
  
      std::vector<indri::api::ScoredExtentResult> runQuery( const std::string& query, int resultsRequested ) throw (lemur::api::Exception) ;
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Run an Indri query language query on a set of documents.
       @param query the query to run
       @param resultsRequested maximum number of results to return
@param documentSet the list of document ids to score.
       @return the vector of ScoredExtentResults for the query


@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      std::vector<indri::api::ScoredExtentResult> runQuery( const std::string& query, const std::vector<int>& documentSet, int resultsRequested ) throw (lemur::api::Exception);

#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Run an Indri query request
       @param request the query request to run
       @return the QueryResults


@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif
#ifdef SWIGJAVA
      indri::api::QueryResults runQuery(indri::api::QueryRequest &request) throw (lemur::api::Exception);
#endif

#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Run an Indri query language query.
       @param query the query to run
       @param resultsRequested maximum number of results to return
       @return pointer to QueryAnnotations for the query
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      indri::api::QueryAnnotation* runAnnotatedQuery( const std::string& query, int resultsRequested ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Run an Indri query language query on a set of documents.
       @param query the query to run
@param documentSet list of document ids to score.
       @param resultsRequested maximum number of results to return
       @return pointer to QueryAnnotations for the query
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      indri::api::QueryAnnotation* runAnnotatedQuery( const std::string& query, const std::vector<int>& documentSet, int resultsRequested ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Fetch the parsed documents for a given list of ScoredExtentResults
       @param documentIDs the list of ScoredExtentResults
       @return the vector of ParsedDocument pointers.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

  
      std::vector<indri::api::ParsedDocument*> documents( const std::vector<int>& documentIDs ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Fetch the parsed documents for a given list of ScoredExtentResults
       @param results the list of ScoredExtentResults
       @return the vector of ParsedDocument pointers.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      std::vector<indri::api::ParsedDocument*> documents( const std::vector<indri::api::ScoredExtentResult>& results ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Fetch the named metadata attribute for a list of document ids
       @param documentIDs the list of ids
       @param attributeName the name of the metadata attribute
       @return the vector of string values for that attribute
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      std::vector<std::string> documentMetadata( const std::vector<int>& documentIDs, const std::string& attributeName ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Fetch the named metadata attribute for a list of ScoredExtentResults
       @param documentIDs the list of ScoredExtentResults
       @param attributeName the name of the metadata attribute
       @return the vector of string values for that attribute
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      std::vector<std::string> documentMetadata( const std::vector<indri::api::ScoredExtentResult>& documentIDs, const std::string& attributeName ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return a list of document IDs where the document has a metadata key that matches attributeName, with a value matching one of the attributeValues.
       @param attributeName the name of the metadata attribute (e.g. 'url' or 'docno')
       @param attributeValue values that the metadata attribute should match
       @return a vector of ParsedDocuments that match the given metadata criteria
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      std::vector<int> documentIDsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValue ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Fetch all documents with a metadata key that matches attributeName, with a value matching one of the attributeValues.
       @param attributeName the name of the metadata attribute (e.g. 'url' or 'docno')
       @param attributeValues values that the metadata attribute should match
       @return a vector of ParsedDocuments that match the given metadata criteria
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      std::vector<indri::api::ParsedDocument*> documentsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValue ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return total number of terms.
       @return total number of terms in the aggregated collection

@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif


      INT64 termCount() throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return total number of term occurrences.
       @param term the term to count
       @return total frequency of this term in the aggregated collection
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      INT64 termCount( const std::string& term ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return total number of term occurrences within a field.
       @param term the term to count
       @param field the name of the field
       @return total frequency of this term within this field in the 
       aggregated collection
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      INT64 termFieldCount( const std::string& term, const std::string& field ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return the list of fields.
       @return vector of field names.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      std::vector<std::string> fieldList() throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return total number of documents in the collection.
       @return total number of documents in the aggregated collection
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      INT64 documentCount() throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return total number of documents in the collection for the given term.
@param term the term to count documents for
       @return total number of documents containing the term in the aggregated collection
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      INT64 documentCount( const std::string& term ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Fetch a document vector for a list of documents. 
       @param documentIDs the vector of document ids.
       @return DocumentVector pointer for the specified document.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif


      std::vector<indri::api::DocumentVector*> documentVectors( const std::vector<int>& documentIDs ) throw (lemur::api::Exception);
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return the total number of times this expression appears in the collection.
       @param expression The expression to evaluate, probably an ordered or unordered window expression
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      double expressionCount( const std::string& expression, const std::string &queryType = "indri" ) throw (lemur::api::Exception);

#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return the total number of documents this expression appears in the collection.
       @param expression The expression to evaluate, probably an ordered or unordered window expression
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif
      double documentExpressionCount( const std::string& expression,
                              const std::string &queryType = "indri" )  throw (lemur::api::Exception);

#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return all the occurrences of this expression in the collection.
       Note that the returned vector may be quite large for large collections, and therefore
       has the very real possibility of exhausting the memory of the machine.  Use this method
       with discretion.
       @param expression The expression to evaluate, probably an ordered or unordered window expression
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      std::vector<indri::api::ScoredExtentResult> expressionList( const std::string& expression,  const std::string& queryType = "indri" ) throw (lemur::api::Exception);

#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
        Return all the length of a document.
       @param documentID The internal document id.
       @return the length of the document.
*/
public";
#endif

      int documentLength( int documentID ) throw (lemur::api::Exception);

      void setFormulationParameters(indri::api::Parameters &p);
      std::string reformulateQuery(const std::string &query);

      std::string stemTerm(const std::string &term);
      INT64 termCountUnique();
      INT64 stemCount( const std::string& term );
      INT64 stemFieldCount( const std::string& term, const std::string& field );
      INT64 documentStemCount( const std::string& stem );
    };
  }
}
