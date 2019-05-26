package lemurproject.indri;

import java.util.Map;

/**
 * ParsedDocument
 *
 * 10 August 2004 -- tds
 */

public class ParsedDocument {
  public static class TermExtent {
    public TermExtent( int b, int e ) {
      begin = b;
      end = e;
    }
    
    public int begin;
    public int end;
  }

  public String text;
  public String content;

  public String[] terms;
  public TermExtent[] positions;
  public Map metadata;
}
