%typemap(javaimports) indri::query::QueryExpander "import java.util.Map;"
%typemap(javaimports) indri::query::RMExpander "import java.util.Map;"
%typemap(javaimports) indri::query::PonteExpander "import java.util.Map;"
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
