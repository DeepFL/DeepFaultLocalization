package lemurproject.indri;
import java.util.Map;

public class QueryResult {
    public String snippet;
    public String documentName;
    public int docid;
    public double score;
    public int begin;
    public int end;
    public Map metadata;
}

