<?php
include( "include/grouping.php" );

class Results {
  var $results;
  var $groupings;

  function Results( $results, $urls ) {
    $this->groupings = array();
    $this->results = array();    
      
    for( $i = 0; $i < count( $results ); $i++ )
      $this->results[$i] = array( $results[$i], $urls[$i] );
  }
  
  function groupize( $prefix, $env, $startdoc, $count ) {
    // quick, generic deduplication
    // probability of two documents having EXACTLY the same
    // score and being different is very small
    // only fetches the actual documents for the ids in the
    // groupings slice starting at $startdoc for $count docs
    
    $deduped_results = array();
    $deduped_count = 0;
    $old_score = 0;
    usort( $this->results, array("Results", "sort_docs_by_score" ) );
    foreach( $this->results as $result ) {
      if( $result[0]->score != $old_score ) {
	$deduped_results[$deduped_count++] = $result;
      }
      $old_score = $result[0]->score;
    }

    // sort deduplicated documents by URL
    $group_num = 0;
    usort( $deduped_results, array("Results", "sort_docs_by_url" ) );

    $base = "";
    $base_prefix = "";
    foreach( $deduped_results as $result ) {
      $url = $result[1];
      if (strlen($url) < 8) {
        // bogus, drop it
        continue;
      }
      
      if( ( $prefix != "" && strpos( $url, $prefix ) !== false ) ||
	  ( $prefix == "" && ( $base == "" ||
			       ( strpos( $url, $base ) === false &&
				 ( $base_prefix == "" || strpos( $url, $base_prefix ) === false ) ) ) ) ) {
	$base = $url;
	$end = strlen( $base ) - strlen( strrchr( $base, "/" ) ) + 1;
	if ($end > 7 ) {
          // don't reduce to http://
          $base_prefix = substr( $base, 0, $end ); 
        }

	$grouping =& new Grouping( $base, $result[1], $result[0] );
	$this->groupings[$group_num++] = &$grouping;
      }
      else {
	if( $prefix == "" ) // only add subgrouped items if there's no prefix
	  $grouping->addURL( $url, $result[1], $result[0] );
      }
    }

    usort( $this->groupings, array("Results", "sort_groups_by_score" ) );
    for( $i=$startdoc; $i < count($this->groupings) && $i < $startdoc + $count; $i++ ) {
      $this->groupings[$i]->setDocs($env);
    }
    
  }

  function sort_docs_by_url( $a, $b ) {
    return strcasecmp( $a[1], $b[1] );    
  }

  function sort_docs_by_score( $a, $b ) {
    $score_a = $a[0]->score;
    $score_b = $b[0]->score;
    if( $score_a == $score_b )
      return 0;

    return ( $score_a > $score_b ) ? -1 : 1;
  }

  function sort_groups_by_score( &$a, &$b ) {
    if( $a->score == $b->score )
      return 0;

    return ( $a->score > $b->score ) ? -1 : 1;
  }
}

?>
