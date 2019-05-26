package edu.umass.cs.indri.hits;

public class Link implements Comparable {
  public Link( int from, int to ) {
    this.from = from;
    this.to = to;
  }

  public int hashCode() {
    return (from * 7 + to) % 65531;
  }

  public int compareTo( Object o ) {
    Link other = (Link) o;
    int difference = to - other.to;

    if( difference != 0 )
      return difference;

    return from - other.from;
  }

  public int from;
  public int to;
}
