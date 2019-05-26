<?php

class Grouping {
  var $base;
  var $best;
  var $all;
  var $score;
  var $count;

  function Grouping( $base_url, $base_doc, $base_result ) {
    $this->base = array( $base_url, $base_doc, $base_result );
    $this->all = array();
    $this->count = 0;
    $this->addURL( $base_url, $base_doc, $base_result );    
  }

  function addURL( $url, $doc, $result ) {
    $this->all[$this->count] = array( $url, $doc, $result );
    if( $result->score > $this->score ) {
      $this->best = array( $url, $doc, $result );
      $this->score = $result->score;
      //      print $this->best[0] . "<br>";
    }
    $this->count++;
  }

  function setDocs( $env ) {
    $docs = $env->documents( array($this->best[2]) );
    $this->best[1] = $docs[0];
    if ($this->base[2] != $this->best[2] ) {
      $docs = $env->documents( array($this->base[2]) );
      $this->base[1] = $docs[0];
    }
  }
  
  function printGrouping( $annotations, $nodes, $indri_param ) {
    $this->printResult( $this->best, true, $annotations, $nodes, $indri_param );
    if( $this->base[2] != $this->best[2] )
      $this->printResult( $this->base, false, $annotations, $nodes, $indri_param );
  }
  
  function printResult( $entry, $primary, $annotations, $nodes, $indri_param ) {
    $doc = $entry[1];
    $meta = $doc->metadata;
    $result = $entry[2];
    
    $range = new position( $result->begin, $result->end );
    
    $matches = indri_documentMatches( $result->document,
				      $annotations,
				      $nodes, 
				      $range );
    
    $snippet = indri_buildsnippet( $doc->text, $matches, $doc->positions, $indri_param[ 'snippet_length' ], $range );

    $title = substr( $snippet, 0, 50 ) . "...";
    $title = isset($meta["docno"]) ? $meta["docno"] : $title; 
    $title = isset($meta["path"]) ? substr( $meta["path"],
					    strrpos( $meta["path"], '/' ) + 1,
					    strlen($meta["path"]) ) : $title; 
    $title = isset($meta["url"])  ? $meta["url"] : $title;
    $title = (isset($meta["title"]) && strlen($meta["title"]) > 0 ) ? 
      $meta["title"] : $title; 
    $title = preg_replace("/^http:\/\//", "", $title);
    $prefix = "";
    
    $beginlink = isset($meta["url"]) ? ("<a href=\"" . $meta["url"] . "\">") : "";
    $endlink = isset($meta["url"]) ? "</a>" : "";
    $cachedlink = indri_buildlink( $_REQUEST, $result->document );

    if( $primary === true ) {
    ?>
    <div id="result">
       <div id="resulttitle">
          <?= $prefix . $beginlink . $title . $endlink ?>
       </div>
       <div id="snippet">
          <?= $snippet ?>
       </div>
       <span id="url">
          <?= preg_replace("/^http:\/\//", "", $meta["url"] )?>
       </span>
       <span id="cachedlink">
       <a href="<?= $cachedlink ?>">Cached</a>
       </span>
       <!-- [ Score = <?= number_format(exp($result->score), 7) ?> ] -->
<?php
       if ($this->count > 1) {
     $end = strlen( $this->base[0] ) - strlen( strrchr( $this->base[0], "/" ) ) + 1;
	if ($end > 7 ) {
          $base_prefix = substr( $this->base[0], 0, $end );   
        } else {
          // don't reduce to http://
          $base_prefix = $this->base[0];
     }
     $clean_base = preg_replace("/^http:\/\//", "", $base_prefix);
?>   - 
        <span id="morelink">
        <a href="query.php?query=<?= indri_escapeurl($_REQUEST['query']) ?>&prefix=<?= indri_escapeurl($base_prefix) ?>">More from <?= $clean_base ?>...</a>
       </span>
<?php
       }
    
?>
   </div>
   <?php }
   else {
     $end = strlen( $this->base[0] ) - strlen( strrchr( $this->base[0], "/" ) ) + 1;
	if ($end > 7 ) {
          $base_prefix = substr( $this->base[0], 0, $end );   
        } else {
          // don't reduce to http://
          $base_prefix = $this->base[0];
     }
     $clean_base = preg_replace("/^http:\/\//", "", $base_prefix);
?>
    <div id="indentedresult">
       <div id="indresulttitle">
       <?= $prefix . $beginlink . $title . $endlink ?>
       </div>
       <div id="snippet">
          <?= $snippet ?>
       </div>
       <span id="url">
          <?= preg_replace("/^http:\/\//", "", $meta["url"] ) ?>
       </span>
       <span id="cachedlink">
       <a href="<?= $cachedlink ?>">Cached</a>
       </span>
       <!-- [ Score = <?= number_format(exp($result->score), 7) ?> ] -->
       - <span id="morelink">
       <a href="query.php?query=<?= indri_escapeurl($_REQUEST['query']) ?>&prefix=<?= indri_escapeurl($base_prefix) ?>">More from <?= $clean_base ?>...</a>
      </span>
   </div>
   <?php } }

}

?>
