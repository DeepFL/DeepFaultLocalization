<?php include( "config.php" ); ?>

<?php

if (!dl( $indri_param['library'] )) {
  // failed to load shared library, bail the page
?>
<html>
<head>
<title>Indri PHP test not ready</title>
</head>
<body>
Unable to load library: <?= $indri_param['library'] ?>
</body>
</html>
<?php
  return;
}
include ( "libindri_php.php"); // wrapper class definitions
// library loaded, run the bits.
// increase the memory limit for the script.
ini_set("memory_limit", $indri_param[ 'memory_limit' ] );
ini_set("max_execution_time", $indri_param[ 'max_execution_time' ] );
//
// indri_getRawNodes
//                                        

function indri_getRawNodes( $node ) {
	$results = array();

	if( $node->type == "RawScorerNode" ) {
		$results[] = $node->name;
	} else {     
		$children = $node->children;
	
		foreach( $children as $child ) {
      $result = indri_getRawNodes($child);
      $previous = $results;
      $results = array_merge( $previous, $result );
		}                                        
	}                                           
	
	return $results;
}
                        
//
// indri_matchCompare
//                

function indri_matchCompare( $one, $two ) {
	if( $one->begin < $two->begin )
		return -1;
	
	if( $one->begin > $two->begin )
		return 1;
		       
	return 0;
}           

//
// position class
//

class position {
  var $begin;
  var $end;

  function position( $b, $e ) {
    $this->begin = $b;
    $this->end = $e;
  }
};

//
// indri_documentMatches
//
                        
function indri_documentMatches( $document, $annotations, $nodeNames, $range ) {         
  $rawmatches = array();
  
  foreach( $nodeNames as $node ) {
    $positions = 0;
    if( isset($annotations[$node]) ) {
      $positions = $annotations[$node];
    
      foreach( $positions as $position ) {
	if( $document == $position->document &&
	    $range->begin <= $position->begin &&
	    $range->end >= $position->end ) {
	  $rawmatches[] = $position;                
	}
      }
    }
  }   
  
  // sort the array
  usort( $rawmatches, "indri_matchCompare" );
  
  // remove and coalesce duplicates
  $matches = array(); 

  if( count($rawmatches) > 0 ) {
    $begin = $rawmatches[0]->begin;
    $end = $rawmatches[0]->end;

    for( $i = 1; $i < count($rawmatches); $i++ ) {
      if( $rawmatches[$i]->begin > $end ) {
        // add a match
        $matches[] = new position( $begin, $end );

        $begin = $rawmatches[$i]->begin;
      }

      if( $end < $rawmatches[$i]->end ) {
        $end = $rawmatches[$i]->end;
      }
    }

    // add a match
    $matches[] = new position( $begin, $end );
  }

	return $matches;
}
 
   
//
// yank out any tags from $rawsnippet
//

function indri_sanitizetext( $rawsnippet ) {
	$rawsnippet = preg_replace("'<style[^>]*>.*</style>'siU",'', $rawsnippet );
	$rawsnippet = ereg_replace("~<script[^>]*>.+</script[^>]*>~isU", "", $rawsnippet ); 
	return strip_tags( $rawsnippet );
}

//
// $matches is an array of extents of term matches in the document
// $text is the text of the document                    
// $positions is an array of byte offsets of words the document text
//        
// This method tries to put as many matches as possible into $windowSize
// words.  However, we give at least 7 words of context for every match;
// that takes precidence.  Therefore, some matches may get left out of
// the snippet.
//
// If two matches are close enough, they will be contiguous in the 
// snippet.  Otherwise, each match (and the context of the match)
// is preceded by an ellipsis ('...').
//
// The first part of this method figures out which matches will be 
// included in the snippet, while the second part builds the snippet text.
//

function indri_buildsnippet( $text, $matches, $positions, $windowSize, $range ) {
  $characters = 0;
  $output = "...";   
  
  $matchWidth = (int) ($windowSize / count($matches));
  
  if( $matchWidth < 5 ) {
    // want at least 7 words around each match
    $matchWidth = 5;
  } else if( $matchWidth > 30 ) {
    $matchWidth = 30;
  }                         
	                                  
  $matchBegin = $matches[0]->begin;
  $matchEnd = $matches[0]->end;
  $match = array( "begin" => $matchBegin, "end" => $matchEnd );
  $words = 0;
	
  $begin = $matchBegin - (int) ceil($matchWidth / 2);
  $end = $matchEnd + (int) floor($matchWidth / 2);            

  if( $begin < $range->begin )
    $begin = $range->begin;
  if( $end >= $range->end ) 
    $end = $range->end;

  if( $range->end - $range->begin <= $windowSize ) {
    $begin = $range->begin;
    $end = $range->end;
  }
		
  $segment = array("begin" => $begin,
                   "end" => $end,
                   "matches" => array( $match ) );
  $segments = array();

  // figure out what matches to coalesce
  for( $i=1; $i<count($matches); $i++ ) {
    $match = array( "begin" => $matches[$i]->begin,
                    "end" => $matches[$i]->end );
    $begin = $matches[$i]->begin - (int) ceil($matchWidth / 2);
    $end = $matches[$i]->end + (int) floor($matchWidth / 2);
    
    if( $begin < $range->begin )
      $begin = $range->begin;
    if( $end >= $range->end )
      $end = $range->end;

    if( ($words + ($segment["end"] - $segment["begin"])) > $windowSize ) {
      break;
    }
    
    if( $segment["end"] >= $begin ) {
      $segment["end"] = $end;
      $segment["matches"][] = $match;
      $words += ($segment["end"] - $segment["begin"]);
    } else {
      $segments[] = $segment;
      $words += ($segment["end"] - $segment["begin"]);
      $segment = array( "begin" => $begin,
                        "end" => $end, "matches" => array($match));
    }                                                          
    
    if( $words > $windowSize ) {
      break;
    }
  }                                                            
  $segments[] = $segment;
  
  $output = "";
  
  // build snippet from the list of segments
  for( $i=0; $i<count($segments); $i++ ) {
    $segment = $segments[$i];
    
    $begin = $segment["begin"];
    $end = $segment["end"];
    $matches = $segment["matches"];
    
    if( $begin > $range->begin && $i == 0 ) {
      $output .= "<strong>...</strong>";
    }                 
    if ( !isset($positions[$end-1]) || !isset($positions[$begin])) { 
      continue; 
    }

    $beginByte = $positions[$begin]->begin;
    $endByte = $positions[$end-1]->end;
    
    $current = $beginByte;

    for( $j=0; $j<count($matches); $j++ ) {
      $beginMatch = $matches[$j]["begin"];
      $endMatch = $matches[$j]["end"];
      $output .= indri_sanitizetext( substr( $text, $current, $positions[$beginMatch]->begin - $current ) );
      $output .= "<strong>";
      $output .= indri_sanitizetext( substr( $text,
                                             $positions[$beginMatch]->begin,
                                             $positions[$endMatch-1]->end - $positions[$beginMatch]->begin ) );
      $output .= "</strong>";
			
      $current = $positions[$endMatch-1]->end;
    }
      $output .= indri_sanitizetext( substr( $text, $current, $endByte - $current ) );
		
    if( $end < $range->end-1 ) {
      $output .= "<strong>...</strong> ";
    }
  }
	 
  return $output;
}                         

//
// indri_setupenvironment
//

function indri_setupenvironment( $param, $env, $request ) {
   $env->addIndex( $param[ 'index' ] );
   $rules = array( "method:dirichlet,mu:250,field:mainbody,operator:term", "method:dirichlet,mu:1000,field:mainbody,operator:window", "method:dirichlet,mu:100,field:inlink,operator:term", "method:dirichlet,mu:100,field:inlink,operator:window", "method:dirichlet,mu:10,field:title,operator:term", "method:dirichlet,mu:5,field:title,operator:window", "method:dirichlet,mu:40,field:heading,operator:term", "method:dirichlet,mu:80,field:heading,operator:window" );
   $env->setScoringRules( $rules );

   if (isset($request['startdoc'])) {
     $startdoc = urldecode($request['startdoc']);
   } else {
     $startdoc = 0;
   }

   return $startdoc;
}

function indri_cleanquery( $query ) {
  // remove punctuation that the query parser won't like.
  // map "term term" to #1(term term) and wrap in #combine
  //
  if( strpos( $query, "#" ) !== false )
    // don't modify a structured query
    return $query;
  $query = preg_replace("/['\?;:!,\.\+=<>\(\){}\[\]\*\&\^\%\$~`\\\|]/", "", $query);
  // spaces for email addresses
  $query = preg_replace("/[@]/", " ", $query);
  $query = preg_replace('/"([^"]+)"/', "#1(" . '\1' . ")", $query);
  if( strpos( $query, "#" ) !== false )
    // if we added a #1, wrap in a combine.
    $query = "#combine( " . $query . " )";
  return $query;
}

function indri_timer() {
   list($usec, $sec) = explode(" ", microtime());
   return ((float)$usec + (float)$sec); 
}

function indri_buildlink( $request, $document ) {
   $server = "";
   $index = "";

   return "showdoc.php?documentID=" . $document . $server . $index;
}

function indri_nextlink( $request, $startdoc, $prefix ) {
   $server = "";
   $index = "";

   if (isset($request['query']) && $request['query'] != "None") {
      $query = "&query=" . $request['query'];
   }
   $url = "query.php?startdoc=" . $startdoc . $query ;
   if ($prefix != "") {
     $url .= "&prefix=" . $prefix;
   }
   
   return indri_escapeurl( $url );
}

function indri_escapeurl( $text ) {
  $search = array( ' ', '#', '[', ']' );
  $replace = array( '%20', '%23', '%5B', '%5D' );

  return str_replace( $search, $replace, $text );
}

function indri_escapetags( $text ) {
  $search = array( '<', '>' );
  $replace = array( '&lt;', '&gt;' );
  $text = str_replace( $search, $replace, $text );
	return $text;
}

function indri_printlinks( $request, $startdoc, $resultCount,  $pagedocs, $prefix ) {
  $nextlink = indri_nextlink( $request, $startdoc + $pagedocs, $prefix );
  $prevlink = indri_nextlink( $request, $startdoc - $pagedocs, $prefix );

  $nextfull = "<a href=\"" . $nextlink . "\">Next " . $pagedocs . "</a>";
  $prevfull = "<a href=\"" . $prevlink . "\">Previous " . $pagedocs . "</a>";

  $showprev = ( $startdoc >= $pagedocs );
  $shownext = ( $resultCount >= $pagedocs );

  $result = "<h4>";

  if( $showprev )              { $result .= $prevfull; }
  if( $showprev && $shownext ) { $result .= '|'; }
  if( $shownext )              { $result .= $nextfull; }

  $result .= "</h4>";
  return $result;
}


function indri_insert_base_tag( $document ) {
  if( !isset( $document->metadata["url"] ) ) {
    return $document->content;
  }

  $text = $document->content;
  $meta = $document->metadata;
  $url = $meta["url"];

  $has_base = strpos( $text, "<base" );

  if( $has_base === true ) {
    // don't need to add a base tag if it's already there
    return $text;
  }

  $base_url = $url;
  $last_slash = strrpos( $url, "/" );
  $last_dot = strrpos( $url, "." );

  if( $last_slash > $last_dot ) {
    $base_url = $url;
  } else {
    $base_url = substr( $url, 0, $last_slash . "/" );
  }

  // clean up image links
  $text = preg_replace( '@<img src="(?!http)([^"]*)@siU', '<img src="' . $base_url . '\1', $text );
  // clean up a links
  $text = preg_replace( '@<a href="(?!http)([^"]*)@siU', '<a href="' . $base_url . '\1', $text );
  return $text;
}

// dependence model
function indri_formulate_dm( $query ) {
  // don't reformulate if this query contains a query operator
  if( strpos( $query, "#" ) !== false )
    return $query;

  // split the query into terms
  $terms = preg_split( '/ /', $query, -1, PREG_SPLIT_NO_EMPTY );

  // don't build a complex query for more than 4 terms
  if (count($terms) > 4) return "#combine( " . $query . " )";

  $queryT = "#combine( ";
  $queryO = "#combine( ";
  $queryU = "#combine( ";
  for( $start_pos = 0; $start_pos < count( $terms ); $start_pos++ ) {
    for( $end_pos = $start_pos; $end_pos < count( $terms ); $end_pos++ ) {
      if( $start_pos == $end_pos ) {
	$queryT .= $terms[ $start_pos ] . " ";
	continue;
      }
      $queryO .= " #1( ";
      $queryU .= " #uw" . 4*( $end_pos - $start_pos + 1 ) . "( ";
      for( $i = $start_pos; $i <= $end_pos; $i++ ) {
	$queryO .= $terms[ $i ] . " ";
	$queryU .= $terms[ $i ] . " ";
      }
      $queryO .= " )";
      $queryU .= " )";
    }
  }
  $queryT .= ")";
  $queryO .= ")";
  $queryU .= ")";

  $ret = "#weight( ";
  if( $queryT != "#combine( )" )
    $ret .= "0.8 $queryT ";
  if( $queryO != "#combine( )" )
    $ret .= "0.1 $queryO ";
  if( $queryU != "#combine( )" )
    $ret .= "0.1 $queryU ";
  $ret .= ")";
  
  return $ret;
}

// named page
function indri_formulate_np( $query ) {
  // don't reformulate if this query contains a query operator
  if( strpos( $query, "#" ) !== false )
    return $query;

  // split the query into terms
  $terms = preg_split( '/ /', $query, -1, PREG_SPLIT_NO_EMPTY );

  // don't build a complex query for more than 4 terms
  if (count($terms) > 4) return "#combine( " . $query . " )";

  $queryAll = "#weight( ";
  for( $start_pos = 0; $start_pos < count( $terms ); $start_pos++ ) {
    $w = $terms[ $start_pos ];
    $queryAll .= "1.0 #wsum( 1.0 $w.(inlink) 1.0 $w.(title) 3.0 $w.(mainbody) 1.0 $w.(heading) ) ";
  }
  $queryAll .= ")";

  $ret = "#weight( 0.1 #weight( 1.0 #prior(pagerank) 0.75 #prior(inlinks) ) 1.0 $queryAll )";
  
  return $ret;
}

// named page + dependence model
function indri_formulate_npdm( $query ) {
  // don't reformulate if this query contains a query operator
  if( strpos( $query, "#" ) !== false )
    return $query;

  // split the query into terms
  $terms = preg_split( '/ /', $query, -1, PREG_SPLIT_NO_EMPTY );
  // don't build a complex query for more than 4 terms
  if (count($terms) > 4) return "#combine( " . $query . " )";
  
  $queryT = "#combine( ";
  $queryO = "#combine( ";
  $queryU = "#combine( ";
  for( $start_pos = 0; $start_pos < count( $terms ); $start_pos++ ) {
    for( $end_pos = $start_pos; $end_pos < count( $terms ); $end_pos++ ) {
      if( $start_pos == $end_pos ) {
	$w = $terms[ $start_pos ];
        $queryT .= "#wsum( 1.0 $w.(inlink) 1.0 $w.(title) 3.0 $w.(mainbody) 1.0 $w.(heading) ) ";
	//$queryT .= "#wsum( 1.0 $w.(title) 3.0 $w 1.0 $w.(heading) ) ";
        continue;
      }
      $t = "";
      for( $i = $start_pos; $i <= $end_pos; $i++ )
	$t .= $terms[ $i ] . " ";
      $w = " #1( $t )";
      $queryO .= "#wsum( 1.0 $w.(inlink) 1.0 $w.(title) 3.0 $w.(mainbody) 1.0 $w.(heading) ) ";
      //$queryO .= "#wsum( 1.0 $w.(title) 3.0 $w 1.0 $w.(heading) ) ";
      $w = " #uw" . 4*( $end_pos - $start_pos + 1 ) . "( $t )";
      $queryU .= "#wsum( 1.0 $w.(inlink) 1.0 $w.(title) 3.0 $w.(mainbody) 1.0 $w.(heading) ) ";
      //$queryU .= "#wsum( 1.0 $w.(title) 3.0 $w 1.0 $w.(heading) ) ";
    }
  }
  $queryT .= ")";
  $queryO .= ")";
  $queryU .= ")";
  
  //  $ret = "#weight( 0.1 #weight( 1.0 #prior(pagerank) 0.75 #prior(inlinks) ) 1.0 #weight( ";
  $ret = "#weight( ";
  if( $queryT != "#combine( )" )
    $ret .= "0.8 $queryT ";
  if( $queryO != "#combine( )" )
    $ret .= "0.1 $queryO ";
  if( $queryU != "#combine( )" )
    $ret .= "0.1 $queryU ";
  //  $ret .= ") )";
  $ret .= ")";
  
  //  print $ret;
  
  return $ret;
}

function deduplicate( $results, $documents ) {
   usort( $documents, "indri_rankedlist_sort" );
   $base = "";
   foreach( $documents as $doc ) {
     $meta = $doc->metadata;
     $url = $meta["url"];
     print "URL: " . $url . "<br>";
     $pos = strpos( $url, $base );
     if( $base == "" || strpos( $url, $base ) === false) {
       $base = $url;
       print "NEW BASE: " . $base . "<br>\n";
     }
     else {
     }
   }
}

function indri_rankedlist_sort( $a, $b ) {
  $meta_a = $a->metadata;
  $meta_b = $b->metadata;  
  return strcasecmp( $meta_a["url"], $meta_b["url"] );
}

