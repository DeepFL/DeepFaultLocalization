
package edu.umass.cs.indri.web;

public class Strip {
  private static String stripStringPair( String input, String tagStart, String tagEnd ) {
    StringBuffer builder = null; 
    int start = 0;
    int begin;
    int end;

    while( true ) {
      begin = input.indexOf( tagStart, start );
      end = input.indexOf( tagEnd, begin );

      if( builder == null ) {
        if( begin < 0 ) {
          return input;
        } else {
          builder = new StringBuffer();
        }
      }

      if( begin < 0 ) {
        builder.append( input.substring( start ) );
        break;
      } else {
        builder.append( input.substring( start, begin ) );
      }

      start = end + tagEnd.length();
    }

    return builder.toString();
  }

  public static String strip( String input ) {
    // strip comments
    input = stripStringPair( input, "<!--", "-->" );
    // strip script 
    input = stripStringPair( input, "<script", "</script>" );
    // strip tags
    input = stripStringPair( input, "<", ">" );

    return input;
  }
}


