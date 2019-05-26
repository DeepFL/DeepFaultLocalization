package lemurproject.indri;

/**
 * ScoredExtentResult
 *
 * 10 August 2004 -- tds
 *
 * This object should stay in sync with the
 * C++ version.
 */

public class ScoredExtentResult {
  public double score;
  public int document;
  public int begin;
  public int end;
  public long number;
  public int ordinal;
  public int parentOrdinal;
}

