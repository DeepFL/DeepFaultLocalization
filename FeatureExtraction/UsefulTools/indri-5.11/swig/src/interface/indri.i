#ifdef SWIGJAVA
%module (directors="1") indri
#endif
%{
#include "indri/indri-platform.h"
#ifdef INDRI_STANDALONE
#include "lemur/lemur-compat.hpp"
#else
#include "lemur-compat.hpp"
#endif
#include "indri/QueryEnvironment.hpp"
#include "indri/QueryExpander.hpp"
#include "indri/RMExpander.hpp"
#include "indri/PonteExpander.hpp"
#include "indri/ScoredExtentResult.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/IndexEnvironment.hpp"
#include "indri/Parameters.hpp"
#include "indri/ConflationPattern.hpp"
#include "indri/ReformulateQuery.hpp"

#ifdef INDRI_STANDALONE
#include "lemur/Exception.hpp"
#else
#include "Exception.hpp"
#endif
  %}

typedef long long INT64;
typedef long long UINT64;

%include "std_string.i"
%include "exception.i"

#ifdef SWIGCSHARP
%module (directors="1") indri_csharp

%apply const std::string & {std::string &};
%apply const char * {char *};

%include "std_map.i"
%include "std_vector.i"

%include "LemurException.i"
%include "IntVector.i"
%include "StringVector.i"
%include "ConflationPattern.i"
%include "MResults.i"
%include "MetadataPairVector.i"
%include "ParsedDocument.i"
%include "QueryAnnotationNode.i"
%include "ScoredExtentArray.i"
%include "Parameters.i"
%include "DocumentVector.i"
%include "StringMap.i"
%include "Specification.i"
%include "QueryEnvironment.i"
%include "QueryExpander.i"
%include "IndexEnvironment.i"
#endif

#ifdef SWIGJAVA
%pragma(java) jniclassimports="import java.util.Map;";

%include "LemurException.i"
%include "ConflationPattern.i"
%include "MResults.i"
%include "ScoredExtentArray.i"
%include "ParsedDocument.i"
%include "QueryAnnotationNode.i"

%include "IntVector.i"
%include "StringVector.i"
%include "Parameters.i"
%include "DocumentVector.i"
%include "MetadataPairVector.i"
%include "StringMap.i"
%include "Specification.i"
%include "QueryEnvironment.i"
%include "QueryExpander.i"
%include "IndexEnvironment.i"

#ifdef INDRI_STANDALONE
%pragma(java) jniclasscode=%{
  static {
    System.loadLibrary("indri_jni");
  }
  %}
#else
%pragma(java) jniclasscode=%{
  static {
    System.loadLibrary("lemur_jni");
  }
  %}
#endif
#endif
#ifdef SWIGPHP5
%module libindri_php
%{
#ifdef INDRI_STANDALONE
#include "lemur/lemur-compat.hpp"
#else
#include "lemur-compat.hpp"
#endif
#include "indri/QueryEnvironment.hpp"
#include "indri/QueryExpander.hpp"
#include "indri/RMExpander.hpp"
#include "indri/PonteExpander.hpp"
#ifdef INDRI_STANDALONE
#include "lemur/Exception.hpp"
#else
#include "Exception.hpp"
#endif

  // remap overloaded method names.
  // may want to use %rename here?
#define onetermCount termCount
#define onedocumentCount documentCount
#define runQuerydocset runQuery
#define runAnnotatedQuerydocset runAnnotatedQuery
#define documentsdocids documents
#define documentMetadatadocids documentMetadata

#define set_int set
#define set_bool set
#define set_string set
#define set_UINT64 set
#define set_double set
#define get_bool get
#define get_int get
#define get_string get
#define get_INT64 get
#define get_double get

  %}

%include "typemaps.i"
%include "std_string.i"
%include "std_vector.i"
%include "exception.i"
%include "indritypemaps.i"
%include "LemurException.i"

namespace indri{
  namespace parse{
    struct TermExtent {
    public:
      int begin;
      int end;

      ~TermExtent();
    };

  }

  namespace api{
    class Parameters {
    public:
      /// Create
      Parameters();
      /// Clean up.
      ~Parameters();
      /// Retrieve the entry associated with name.
      /// @param name the key value.
      /// @return a Parameters object.
      // broken when swig wrapped
//      Parameters get( const std::string& name );

      /// Create a new empty parameter_value for the key given in path
      /// @param path the key to create the value for
      /// @return the Parameters object initialized with the new value.
      // broken when swig wrapped
//      Parameters append( const std::string& path );

      /// Set the value of the Parameters object
      /// @param value the value
      void set( const std::string& value );

      bool get_bool( const std::string& name, bool def );
      /// Retrieve the entry associated with name.
      /// @param name the key value.
      /// @param def the default value for the key
      /// @return the value associated with the key or def if no entry
      /// exists.
      int get_int( const std::string& name, int def );
      /// Retrieve the entry associated with name.
      /// @param name the key value.
      /// @param def the default value for the key
      /// @return the value associated with the key or def if no entry
      /// exists.
      double get_double( const std::string& name, double def );
      /// Retrieve the entry associated with name.
      /// @param name the key value.
      /// @param def the default value for the key
      /// @return the value associated with the key or def if no entry
      /// exists.
      INT64 get_INT64( const std::string& name, INT64 def );

      /// Retrieve the entry associated with name.
      /// @param name the key value.
      /// @param def the default value for the key
      /// @return the value associated with the key or def if no entry
      /// exists.
      std::string get_string( const std::string& name, const std::string& def );

      /// Remove an entry from the table. Does nothing if the key does not
      /// exist.
      /// @param path the key to remove.
      void remove( const std::string& path );

      /// Set the value  for the given key.
      /// @param name the key
      /// @param value the value
      void set_bool( const std::string& name, bool value );

      /// Set the value  for the given key.
      /// @param name the key
      /// @param value the value
      void set_string( const std::string& name, const std::string& value );
      /// Set the value  for the given key.
      /// @param name the key
      /// @param value the value
      void set_int( const std::string& name, int value );
      /// Set the value  for the given key.
      /// @param name the key
      /// @param value the value
      void set_UINT64( const std::string& name, UINT64 value );
      /// Set the value  for the given key.
      /// @param name the key
      /// @param value the value
      void set_double( const std::string& name, double value );

      /// Clear the parameter tree
      void clear();

      /// @return the size of the object.
      size_t size();
      /// @param name the key to probe.
      /// @return true if an entry exists for this key, false otherwise.
      bool exists( const std::string& name );
      /// load an XML parameters string.
      void load( const std::string& text );
    };


    class ScoredExtentResult {

      double score;
      int document;
      int begin;
      int end;
      INT64 number;
      int ordinal;
      int parentOrdinal;
      std::string attributes;
    public:
      ~ScoredExtentResult();
    };


    class ParsedDocument {
      indri::utility::greedy_vector<char*> terms;
      indri::utility::greedy_vector<indri::parse::TagExtent> tags;
      indri::utility::greedy_vector<indri::parse::TermExtent> positions;
      indri::utility::greedy_vector<indri::parse::MetadataPair> metadata;


      const char* text;
      size_t textLength;

      const char* content;
      size_t contentLength;

    public:
      ~ParsedDocument();
      std::string getContent();
    };


    class QueryAnnotationNode {
      std::string name;
      std::string type;
      std::string queryText;
      std::vector<QueryAnnotationNode*> children;
    };

%nodefaultctor QueryAnnotation;

    class QueryAnnotation {
    public:
      const QueryAnnotationNode* getQueryTree() const;
      const indri::infnet::EvaluatorNode::MResults& getAnnotations() const;
      const std::vector<ScoredExtentResult>& getResults() const;
      ~QueryAnnotation();
    };
    setEx(QueryEnvironment::runQuery);
    setEx(QueryEnvironment::runQuerydocset);
    setEx(QueryEnvironment::runAnnotatedQuery);
    setEx(QueryEnvironment::runAnnotatedQuerydocset);
    setEx(QueryEnvironment::documents);
    setEx(QueryEnvironment::documentsdocids);
    setEx(QueryEnvironment::documentMetadata)
    setEx(QueryEnvironment::documentMetadatadocids);
    setEx(QueryEnvironment::documentIDsFromMetadata);
    setEx(QueryEnvironment::setFormulationParameters);
    setEx(QueryEnvironment::reformulateQuery);

    class QueryEnvironment {
    public:
      ~QueryEnvironment();
      void addServer( const std::string& hostname );
      void addIndex( const std::string& pathname );
      void close();
  
      void setMemory( UINT64 memory );
      void setScoringRules( const std::vector<std::string>& rules );
      void setStopwords( const std::vector<std::string>& stopwords );

      std::vector<ScoredExtentResult> runQuery( const std::string& query, int resultsRequested );
      std::vector<ScoredExtentResult> runQuerydocset( const std::string& query, const std::vector<lemur::api::DOCID_T>& documentSet, int resultsRequested );

      QueryAnnotation* runAnnotatedQuery( const std::string& query, int resultsRequested );
      QueryAnnotation* runAnnotatedQuerydocset( const std::string& query, const std::vector<lemur::api::DOCID_T>& documentSet, int resultsRequested );

      std::vector<ParsedDocument*> documentsdocids( const std::vector<lemur::api::DOCID_T>& documentIDs );
      std::vector<ParsedDocument*> documents( const std::vector<ScoredExtentResult>& results );

      std::vector<std::string> documentMetadatadocids( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::string& attributeName );
      std::vector<std::string> documentMetadata( const std::vector<ScoredExtentResult>& documentIDs, const std::string& attributeName );

      std::vector<lemur::api::DOCID_T> documentIDsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValue ) ;

      INT64 termCount();
      INT64 onetermCount( const std::string& term );
      INT64 stemCount( const std::string& term );
      INT64 termFieldCount( const std::string& term, const std::string& field );
      INT64 stemFieldCount( const std::string& term, const std::string& field );
      std::vector<std::string> fieldList();
      INT64 documentCount();
      INT64 onedocumentCount( const std::string& term );
      double expressionCount( const std::string& expression, const std::string &queryType = "indri" );
      double documentExpressionCount( const std::string& expression, const std::string &queryType = "indri" );
      std::vector<indri::api::ScoredExtentResult> expressionList( const std::string& expression,  const std::string& queryType = "indri" );
      int documentLength( int documentID ) ;
      void setFormulationParameters(Parameters &p);
      std::string reformulateQuery(const std::string &query);
    };
  }
}
namespace indri
{
  namespace query
  {
    class QueryExpander {
    public:
      QueryExpander( indri::api::QueryEnvironment * env , indri::api::Parameters& param );
      virtual ~QueryExpander() {};

      // runs original query, expands query based on results ( via expand( .. ) ), then runs expanded query
      std::vector<indri::api::ScoredExtentResult> runExpandedQuery( std::string originalQuery , int resultsRequested , bool verbose = false );
  
      // creates expanded query from an original query and a ranked list of documents
      virtual std::string expand( std::string originalQuery , std::vector<indri::api::ScoredExtentResult>& results ) = 0;
    };

    class RMExpander : public QueryExpander  {
    public:
      RMExpander( indri::api::QueryEnvironment * env , indri::api::Parameters& param );

      virtual std::string expand( std::string originalQuery , std::vector<indri::api::ScoredExtentResult>& results );
    };
    class PonteExpander : public QueryExpander  {
    public:
      PonteExpander( indri::api::QueryEnvironment * env , indri::api::Parameters& param );

      virtual std::string expand( std::string originalQuery, std::vector<indri::api::ScoredExtentResult>& results );
    };
  }
}

#endif
