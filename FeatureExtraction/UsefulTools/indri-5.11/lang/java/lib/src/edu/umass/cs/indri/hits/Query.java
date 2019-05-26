package edu.umass.cs.indri.hits;

import java.util.*;
import edu.umass.cs.indri.*;
import java.io.*;

public class Query {
  public static Document[] buildLinkGraph( GraphStore store, ScoredExtentResult[] results ) throws IOException {
    // build a documentID list
    int[] documentIDs = new int[results.length];

    for( int i=0; i<results.length; i++ ) {
      documentIDs[i] = results[i].document;
    }

    // get all links to and from the inital set
    Link[] incomingLinks = store.getLinksTo( documentIDs );
    Link[] outgoingLinks = store.getLinksFrom( documentIDs );

    // we also want all links for the incoming and outgoing docsets
    Set others = new HashSet();

    for( int i=0; i<incomingLinks.length; i++ ) {
      others.add( new Integer(incomingLinks[i].from) );
    }

    for( int i=0; i<outgoingLinks.length; i++ ) {
      others.add( new Integer(outgoingLinks[i].to) );
    }

    // now, get all of them too
    int[] otherDocumentIDs = new int[others.size()];
    Iterator iter = others.iterator();

    for( int i=0; iter.hasNext(); i++ ) {
      otherDocumentIDs[i] = ((Integer) iter.next()).intValue();
    }

    Link[] additionalIncomingLinks = store.getLinksTo( otherDocumentIDs );
    Link[] additionalOutgoingLinks = store.getLinksFrom( otherDocumentIDs );

    // now we have all the links we need, we just need to turn them
    // into documents and link them all up

    Set all = new HashSet();

    for( int i=0; i<otherDocumentIDs.length; i++ ) {
      all.add( new Integer(otherDocumentIDs[i]) );
    }

    for( int i=0; i<documentIDs.length; i++ ) {
      all.add( new Integer(documentIDs[i]) );
    }

    Map documents = new HashMap();
    iter = all.iterator();

    while( iter.hasNext() ) {
      Integer id = (Integer) iter.next();
      documents.put( id, new Document( id.intValue() ) );
    }

    // all the documents are built, now just rip through and build links
    Set links = new HashSet();

    for( int i=0; i<incomingLinks.length; i++ ) {
      links.add( incomingLinks[i] );
    }

    for( int i=0; i<outgoingLinks.length; i++ ) {
      links.add( outgoingLinks[i] );
    }

    for( int i=0; i<additionalIncomingLinks.length; i++ ) {
      links.add( additionalIncomingLinks[i] );
    }

    for( int i=0; i<additionalOutgoingLinks.length; i++ ) {
      links.add( additionalOutgoingLinks[i] );
    }

    iter = links.iterator();

    while( iter.hasNext() ) {
      Link link = (Link) iter.next();

      // insert link into the graph
      Document from = (Document) documents.get( new Integer(link.from) );
      Document to = (Document) documents.get( new Integer(link.to) );

      if( from != null && to != null ) {
        from.addLinkTo( to );
        to.addLinkFrom( from );
      }
    }

    return (Document[]) documents.values().toArray( new Document[0] );
  }

  public static int[] rerank( QueryEnvironment index, String database, String dbDriver, String userdb, String passdb, ScoredExtentResult[] results, int resultCount ) throws IOException {
    GraphStore graph = new SQLGraphStore( dbDriver, database, userdb, passdb );

    // step 3 -- unpack the results using the link graph, building Document objects as appropriate
    Document[] documents = buildLinkGraph( graph, results );

    // step 4 -- iterate, checking for convergence on each iteration
    boolean converged;
    int iterations = 0;

    do {
      converged = Document.iterate( documents );
    }
    while( !converged && ++iterations < 200 );

    // step 5 -- rip through the documents, and rank them for output
    Document[] hubs = new Document[documents.length];
    System.arraycopy( documents, 0, hubs, 0, hubs.length );
    Arrays.sort( hubs, new Document.HubComparator() );

    Document[] authorities = new Document[documents.length];
    System.arraycopy( documents, 0, authorities, 0, authorities.length );
    Arrays.sort( authorities, new Document.AuthorityComparator() );

    int[] docresults = new int[authorities.length];

    for( int i=0; i<authorities.length; i++ ) {
      docresults[i] = authorities[i].getID();
    }

    graph.close();
    return docresults;
  }
}

