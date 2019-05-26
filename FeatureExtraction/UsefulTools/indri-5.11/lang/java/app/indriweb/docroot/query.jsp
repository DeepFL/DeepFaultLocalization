<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<%@ page isELIgnored ="false" %>
<%@ page import="javax.naming.InitialContext,edu.umass.cs.indri.web.*, java.util.regex.*, edu.umass.cs.indri.*, edu.umass.cs.indri.hits.*, java.util.*, java.lang.*" %> 

<%! Integer documentID = null; %>

<% 
  InitialContext context = new InitialContext();
 
  long beginQuery = System.currentTimeMillis();
  long endQuery = 0;
  long endRerankTime = 0;
  long endDocumentTime = 0;

  String query = request.getParameter("query");
  String indexName = (String) context.lookup( "java:comp/env/index.indri" );
  boolean rerank = ((Boolean) context.lookup( "java:comp/env/rerank" )).booleanValue();
  int rerankCount = ((Integer) context.lookup( "java:comp/env/rerankCount" )).intValue();
  int resultCount = ((Integer) context.lookup( "java:comp/env/resultCount" )).intValue();
  String database = (String) context.lookup( "java:comp/env/index.db" );
  String dbuser = (String) context.lookup( "java:comp/env/index.dbuser" );
  String dbpassword = (String) context.lookup( "java:comp/env/index.dbpassword" );
  boolean judge = ((Boolean) context.lookup( "java:comp/env/judge" )).booleanValue();

  QueryEnvironment qenv = new QueryEnvironment();
  qenv.addIndex( indexName );

  int[] reranked = null;
  ScoredExtentResult[] results = null;
  QueryAnnotation annotation = null;

  if( rerank ) {
    results = qenv.runQuery( query, rerankCount );
    endQuery = System.currentTimeMillis();

    Query hits = new Query();
    reranked = hits.rerank( qenv, database, dbuser, dbpassword, results, resultCount );
    annotation = qenv.runAnnotatedQuery( query, reranked, reranked.length ); 
    endRerankTime = System.currentTimeMillis();
  } else {
    annotation = qenv.runAnnotatedQuery( query, resultCount ); 

    reranked = new int[annotation.getResults().length];
    for( int i=0; i<reranked.length; i++ ) {
      reranked[i] = annotation.getResults()[i].document;
    }

    endQuery = endRerankTime = System.currentTimeMillis();
  }

  ParsedDocument documents[] = qenv.documents( reranked );
  endDocumentTime = System.currentTimeMillis();

  double queryTime = (endQuery - beginQuery) / 1000.0;
  double rerankTime = (endRerankTime - endQuery) / 1000.0;
  double documentTime = (endDocumentTime - endRerankTime) / 1000.0;
%>

<head>
<title>Indri: Results for <%= query %></title>
<link rel="stylesheet" type="text/css" href="style/style.css" title="stylesheet" />
</head>
<body>

<div id="content">
  <div id="header">
    <a href="http://www.lemurproject.org/indri"><h1>INDRI</h1></a>
    <h2>Language modeling meets inference networks</h2>
  </div>

  <div id="resultbanner">
    <h2>Results for <%= query %></h2> 
    Query: <%= queryTime %>s | Documents: <%= documentTime %>s | Rerank: <%= rerankTime %>s
  </div>

  <div id="results">
  <%
     for( int i=0; i<documents.length; i++ ) {
       ParsedDocument doc = documents[i];
       Map meta = doc.metadata;
       String text = doc.text;
       ParsedDocument.TermExtent[] positions = doc.positions;
       
       byte[] docnoBytes = (byte[]) meta.get( "docno" );
       byte[] titleBytes = (byte[]) meta.get( "title" );
       byte[] urlBytes = (byte[]) meta.get("url");
       String title = null; 
       String url = null;
       String docno = null;

       if( titleBytes != null )
         title = new String( titleBytes );
       if( urlBytes != null )
         url = new String( urlBytes );
       if( docnoBytes != null )
         docno = new String( docnoBytes );

       if( title == null ) title = docno;

       String beginLink = url != null ? ("<a href=\"" + url + "\">") : "";
       String endLink = url != null ? "</a>" : "";
       String snippet = SnippetBuilder.buildSnippet( annotation, reranked[i], text, positions, 100, 10, 35 );
       documentID = new Integer(reranked[i]);
  %>
     <div id="result">
        <h2><%= beginLink + title + endLink %></h2>
        <div id="snippet">
          <%= snippet %>
        </div>
        [ <a href="showdoc.jsp?documentID=<%= documentID %>">Cached</a> ] 
     </div>
  <% } %>
  </div>

  <div id="footer">
    <a href="http://ciir.cs.umass.edu/"><img src="images/ciirlogo.gif" alt="CIIR @ UMass" /></a>
  </div> <!-- footer -->
</div> <!-- content -->

<% qenv.close(); %>

</body>
</html>

