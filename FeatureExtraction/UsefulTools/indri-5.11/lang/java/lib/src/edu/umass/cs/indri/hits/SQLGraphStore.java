package edu.umass.cs.indri.hits;

import java.sql.*;
import java.io.IOException;
import java.util.ArrayList;

public class SQLGraphStore implements GraphStore {
  private Connection connection;

  private Link[] getLinks(int[] documents, String fieldName) throws IOException {
    ArrayList list = null;

    try {
      if( documents.length == 0 )
        return new Link[0];

      Statement statement = connection.createStatement();
      StringBuffer query = new StringBuffer();

      query.append( "select source, dest " );
      query.append( "from linktable " );
      query.append( "where local = 1 and " );
      query.append( fieldName );
      query.append( " in ( ");

      for( int i=0; i<documents.length; i++ ) {
        if( i != 0 )
          query.append( ", " );

        query.append( Integer.toString(documents[i]) );
      }

      query.append( ")" );
      ResultSet set = statement.executeQuery( query.toString() );
      list = new ArrayList();

      while( set.next() ) {
        list.add( new Link( set.getInt(1), set.getInt(2) ) );
      }

    } catch( SQLException e ) {
      throw new IOException( "Something weird happened while trying to get link information" + e.toString() );
    }

    return (Link[]) list.toArray( new Link[0] );
  }

  public SQLGraphStore( String hostname, String driverName ) throws IOException  {
    try {
      Class.forName( driverName );
      connection = DriverManager.getConnection( hostname );
    } catch( Exception e ) {
      throw new IOException( "Something weird happened while trying to open a database: " + e.toString() );
    }
  }

  public SQLGraphStore( String hostname, String driverName, String username, String password ) throws IOException {
    try {
      Class.forName( driverName );
      connection = DriverManager.getConnection( hostname, username, password );
    } catch( Exception e ) {
      throw new IOException( "Something weird happened while trying to open a database: " + hostname + " " + username + " " +  e.toString() );
    }
  }

  public void close() {
    try {
      connection.close();
      connection = null;
    } catch( SQLException e ) {
      // forget this exception, we're closing the db
    }
  }

  public Link[] getLinksFrom( int[] documents ) throws IOException {
    return getLinks(documents, "dest" );
  }

  public Link[] getLinksTo( int[] documents ) throws IOException {
    return getLinks(documents, "source");
  }
};
