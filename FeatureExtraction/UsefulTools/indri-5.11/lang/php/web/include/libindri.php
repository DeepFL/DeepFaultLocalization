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

// library loaded, run the bits.
// increase the memory limit for the script.
ini_set("memory_limit", $indri_param[ 'memory_limit' ] );

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
		}                                   
		
		foreach( $positions as $position ) {
			if( $document == $position->document &&
          $range->begin <= $position->begin &&
          $range->end >= $position->end ) {
				$rawmatches[] = $position;                
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
	
	if( $matchWidth < 15 ) {
		// want at least 7 words around each match
		$matchWidth = 15;
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
			$output .= "<strong>...</strong>";
		}
	}
	 
	return $output;
}                         

//
// indri_setupenvironment
//

function indri_setupenvironment( $param, $env, $request ) {
   $env->addIndex( $param[ 'index' ] );

   if (isset($request['startdoc'])) {
     $startdoc = urldecode($request['startdoc']);
   } else {
     $startdoc = 0;
   }

   return $startdoc;
}

function indri_cleanquery( $query ) {
  return preg_replace("/\?/", "", $query);
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

function indri_nextlink( $request, $startdoc ) {
   $server = "";
   $index = "";

   if (isset($request['query']) && $request['query'] != "None") {
      $query = "&query=" . $request['query'];
   }

   return indri_escapeurl( "query.php?startdoc=" . $startdoc . $query );
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

function indri_printlinks( $request, $startdoc, $resultCount,  $pagedocs ) {
  $nextlink = indri_nextlink( $request, $startdoc + $pagedocs );
  $prevlink = indri_nextlink( $request, $startdoc - $pagedocs );

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
    return $document->text;
  }

  $text = $document->text;
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

