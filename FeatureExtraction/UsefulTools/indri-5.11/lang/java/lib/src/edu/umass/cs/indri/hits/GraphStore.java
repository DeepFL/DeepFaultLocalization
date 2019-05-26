package edu.umass.cs.indri.hits;

import java.io.IOException;

/**
 * Created by IntelliJ IDEA.
 * User: trevor
 * Date: Mar 15, 2005
 * Time: 3:24:16 PM
 * To change this template use File | Settings | File Templates.
 */
public interface GraphStore {
  /**
   * Closes any server connections
   * or files held by this GraphStore instance.
   */

  void close();

  /**
   * Get any links that point from these documents to
   * anywhere else in the collection.
   *
   * @param documents
   * @return
   * @throws IOException
   */

  Link[] getLinksFrom( int[] documents ) throws IOException;

  /**
   * Get any links that point to these documents from any
   * others in the collection.
   *
   * @param documents
   * @return
   * @throws IOException
   */

  Link[] getLinksTo( int[] documents ) throws IOException;
}
