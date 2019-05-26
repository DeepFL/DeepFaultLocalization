package lemurproject.indri;

public class QueryRequest {
    public final static int HTMLSnippet=1;
    public final static int TextSnippet=2;
    public String query;
    public String[] formulators;
    public String[] metadata;
    public int resultsRequested;
    public int startNum;
    public int options;
}

