<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<?php include("indrihelpers") ?>

<head>
<title>Indri: Results for <?= $_REQUEST['query']; ?></title>
<link rel="stylesheet" type="text/css" href="http://ciir.cs.umass.edu/~strohman/indri/style.css" title="stylesheet" />
</head>
<body>

<?php 
   $env = new QueryEnvironment();
   
   $startdoc = indri_setupenvironment( $env, $_REQUEST );
   $query = indri_cleanquery( $_REQUEST['query'] );

   // run the query
   $start_time = indri_timer();
   $annotatedResults = $env->runAnnotatedQuery( $query, $startdoc + 10 );
 	 $results = array_slice( $annotatedResults->getResults(), $startdoc );
   $query_end = indri_timer();
   $documents = $env->documents( $results );
   $doc_end = indri_timer();        

	 $nodes = indri_getRawNodes( $annotatedResults->getQueryTree() );

   $query_time = $query_end - $start_time;
   $doc_time = $doc_end - $query_end;
   $total_time = $doc_end - $start_time;
?>

<div id="content">
  <div id="header">
    <a href="http://www.lemurproject.org/indri"><h1>INDRI</h1></a>
    <h2>Language modeling meets inference networks</h2>
  </div>

  <div id="resultbanner">
    <h2>Results for <?= $_REQUEST['query']; ?></h2> 
    [<strong>query</strong><? printf("%5.2fs", $query_time ); ?>, 
    <strong>documents</strong><? printf("%5.2fs", $doc_time ); ?>,
    <strong>total</strong><? printf("%5.2fs", $total_time ); ?>]
  </div>

  <div id="results">
  <?php
     for( $i=0; $i<count($results); $i++ ) {
       $doc = $documents[$i];
       $meta = $doc->metadata;

       $matches = indri_documentMatches( $results[$i]->document,
													  $annotatedResults->getAnnotations(),
													  $nodes );
       $snippet = indri_buildsnippet( $doc->text, $matches, $doc->positions, 250 );

       $title = substr( $snippet, 0, 50 ) . "...";
       $title = isset($meta["docno"]) ? $meta["docno"] : $title; 
       $title = isset($meta["title"]) ? $meta["title"] : $title; 
       $prefix = "";

       if( isset($meta["docno"]) and isset($meta["title"]) ) {
         $prefix = $meta["docno"] . ": ";
       }

       $beginlink = isset($meta["url"]) ? ("<a href=\"" . $meta["url"] . "\">") : "";
       $endlink = isset($meta["url"]) ? "</a>" : "";
       $cachedlink = indri_buildlink( $_REQUEST, $results[$i]->document );
       $nextlink = indri_nextlink( $_REQUEST, $startdoc + 10 );
  ?>
     <div id="result">
        <h2><?= $prefix . $beginlink . $title . $endlink ?></h2>
        <div id="snippet">
          <?= $snippet ?>
        </div>
        [ <a href="<?= $cachedlink ?>">Cached</a> ] 
     </div>
  <?php } ?>

  <?php
     $nextlink = indri_nextlink( $_REQUEST, $startdoc + 10 );
     $prevlink = indri_nextlink( $_REQUEST, $startdoc - 10 );
     if( $startdoc >= 10 ) {
  ?>
       <h4><a href="<?= $prevlink ?>">Previous 10</a> | <a href="<?= $nextlink ?>">Next 10</a></h4>
  <?php } else { ?>
       <h4><a href="<?= $nextlink ?>">Next 10</a></h4>
  <?php } ?>
  <?php $env->close(); ?>
  </div>

  <div id="footer">
    <a href="http://ciir.cs.umass.edu/"><img src="http://ciir.cs.umass.edu/images/ciirlogo.gif" alt="CIIR @ UMass" /></a>
  </div> <!-- footer -->
</div> <!-- content -->

</body>
</html>

