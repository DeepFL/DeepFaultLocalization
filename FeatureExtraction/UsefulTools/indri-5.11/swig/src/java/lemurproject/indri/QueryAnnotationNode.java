package lemurproject.indri;

/**
 * QueryAnnotationNode
 *
 * 10 August 2004 -- tds
 */

public class QueryAnnotationNode {
  public QueryAnnotationNode( String name, String type, String queryText, QueryAnnotationNode[] children ) {
    this.name = name;
    this.type = type;
    this.queryText = queryText;
    this.children = children;
  }

    public String name;
    public String type;
    public String queryText;
    public QueryAnnotationNode[] children;
}
