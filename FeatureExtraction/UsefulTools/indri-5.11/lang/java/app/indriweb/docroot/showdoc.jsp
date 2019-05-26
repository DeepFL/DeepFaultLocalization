
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<head>
<title>Indri document result</title>
</head>
<body>

<%@ page import="javax.naming.InitialContext, edu.umass.cs.indri.*, edu.umass.cs.indri.hits.*, java.util.*, java.lang.*" %>

<% 
   InitialContext context = new InitialContext();
   String index = (String) context.lookup( "java:comp/env/index.indri" );
   boolean preformatted = ((Boolean) context.lookup( "java:comp/env/preformatted" )).booleanValue();

   // make a query environment.
   QueryEnvironment env = new QueryEnvironment();
   env.addIndex( index );
   int numericDocID = Integer.parseInt( request.getParameter("documentID") );
   int[] documentIDs = new int[1];
   documentIDs[0] = numericDocID;

   ParsedDocument[] docs = env.documents( documentIDs );
%>

<% if( preformatted ) { %>
     <pre>
<% } %>
<%= docs[0].text %>
<% if( preformatted ) { %>
     </pre>
<% } %>
<% env.close(); %>

</body>
</html>

