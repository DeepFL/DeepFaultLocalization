package edu.umass.cs.indri.hits;

import java.util.List;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Comparator;

public class Document {
  List outlinks;
  List inlinks;
  int id;

  class Scores {
    Scores() {
      authority = 1;
      hub = 1;
    }

    double authority;
    double hub;
  }

  public static class AuthorityComparator implements Comparator {
    public int compare( Object o, Object t ) {
      Document one = (Document) o;
      Document two = (Document) t;

      double difference = two.getAuthority() - one.getAuthority();

      if( difference > 0 )
        return 1;
      else if( difference < 0 )
        return -1;

      return 0;
    }
  }

  public static class HubComparator implements Comparator {
    public int compare( Object o, Object t ) {
      Document one = (Document) o;
      Document two = (Document) t;

      double difference = two.getHub() - one.getHub();

      if( difference > 0 )
        return 1;
      else if( difference < 0 )
        return -1;

      return 0;
    }
  }

  Scores next;
  Scores current;

  final static double EPSILON = 0.0001;

  public Document( int id ) {
    this.id = id;
    this.outlinks = new LinkedList();
    this.inlinks = new LinkedList();

    next = new Scores();
    current = new Scores();
  }

  public void addLinkTo( Document other ) {
    outlinks.add( other );
  }

  public void addLinkFrom( Document other ) {
    inlinks.add( other );
  }

  public int getID() {
    return id;
  }

  public static boolean iterate( Document[] documents ) {
    double ssAuthority = 0;
    double ssHubs = 0;

    // compute next scores
    for( int i=0; i<documents.length; i++ ) {
      Document document = documents[i];
      document.computeNext();

      double authority = document.next.authority;
      double hub = document.next.hub;

      ssAuthority += authority*authority;
      ssHubs += hub*hub;
    }

    // compute normalization factors
    double normalAuthority = 1.0 / Math.sqrt( ssAuthority );
    double normalHub = 1.0 / Math.sqrt( ssHubs );
    boolean converged = true;

    for( int i=0; i<documents.length; i++ ) {
      Document document = documents[i];

      document.next.authority *= normalAuthority;
      document.next.hub *= normalHub;

      if( converged ) {
        // check for convergence if we still think this time is the charm
        double deltaAuthority =  Math.abs(document.next.authority -
                                          document.current.authority);
        double deltaHubs = Math.abs(document.next.hub -
                                    document.current.hub);

        converged = converged &&
                    deltaHubs < EPSILON &&
                    deltaAuthority < EPSILON;
      }

      document.flip();
    }

    return converged;
  }


  public double computeAuthority() {
    double authority = 0;
    Iterator iter = inlinks.iterator();

    while( iter.hasNext() ) {
      authority += ((Document)iter.next()).current.hub;
    }

    return authority;
  }

  public double computeHub() {
    double hub = 0;
    Iterator iter = outlinks.iterator();

    while( iter.hasNext() ) {
      hub += ((Document)iter.next()).current.authority;
    }

    return hub;
  }

  public void computeNext() {
    next.authority = computeAuthority();
    next.hub = computeHub();
  }

  public double getAuthority() {
    return current.authority;
  }

  public double getHub() {
    return current.hub;
  }

  public void flip() {
    current = next;
    next = new Scores();
  }
}
