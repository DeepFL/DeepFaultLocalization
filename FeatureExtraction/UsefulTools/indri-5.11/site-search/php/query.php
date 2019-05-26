<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<?php 
include("include/libindri.php");
include("include/results.php");

$env = new QueryEnvironment();
$startdoc = indri_setupenvironment( $indri_param, $env, $_REQUEST );
$query = indri_cleanquery( $_REQUEST['query'] );
$query = indri_formulate_npdm( $query );
// run the query
$start_time = indri_timer();
$annotatedResults = $env->runAnnotatedQuery( $query, $startdoc + 1000 );
$doc_end = $query_end = indri_timer();
$results = array();

if( $annotatedResults ) {
  $results = $annotatedResults->getResults();
  // urls for grouping
  $urls = $env->documentMetadata($results, "url");
  $nodes = indri_getRawNodes( $annotatedResults->getQueryTree() );
?>

<head>
<title>Results for <?= $_REQUEST['query']; ?></title>
<link rel="stylesheet" type="text/css" href="style/style.css" title="stylesheet" />
</head>
<body>

<?php
} else {
  include( "include/error.php" );
?>
</html>
<?php
  return;
}

$prefix = "";
if( isset( $_REQUEST['prefix'] ) ) { $prefix = $_REQUEST['prefix']; }

$r = new Results( $results, $urls );
// only get the documents in the slice, after grouping
$r->groupize( $prefix, $env, $startdoc, $indri_param[ 'page_docs' ] );
$doc_end = indri_timer();        
// prune the front of the list
$groupings = array_slice($r->groupings, $startdoc );

$query_time = $query_end - $start_time;
$doc_time = $doc_end - $query_end;
$total_time = $doc_end - $start_time;
?>

<div id="content">
  <?php include( "include/header.php" ) ?>

<?php
$cleanQ = $_REQUEST['query'];
$cleanQ = preg_replace("/'/", "&#39;", $cleanQ);
$cleanQ = preg_replace("/</", "&lt;", $cleanQ);
$cleanQ = preg_replace("/>/", "&gt;", $cleanQ);

?>

  <form action="query.php" method="post">
    <div id="query">
     <input type=text name="query" size="50" value='<?= $cleanQ ; ?>'>
 <input type="submit" value="Search <?= $indri_param['sitename']?>">
    </div> <!-- query -->
   </form>

  <div id="resultbanner">
  Results for <strong><?= $_REQUEST['query']; ?></strong>
    [<strong>query</strong> <? printf("%5.2fs", $query_time ); ?>, 
    <strong>documents</strong> <? printf("%5.2fs", $doc_time ); ?>,
    <strong>total</strong> <? printf("%5.2fs", $total_time ); ?>]
<!--  <div id="expandedquery">Expansion: <?= $query ?></div> -->
  <?php
     if( $prefix != "" ) { ?>
       <br><strong>Using search prefix:</strong>
       <span id="url"><?= $prefix ?></span>
  <?php } ?>			  
  </div>

  <div id="results">
  <?php
  // at most page_docs groupings per page 
  $max = $indri_param[ 'page_docs' ];
  for( $i=0; $i < count($groupings) && $i < $max; $i++ ) {
       $grouping = $groupings[$i];
       $grouping->printGrouping( $annotatedResults->getAnnotations(), $nodes, $indri_param );
     }
  ?>

  <?= indri_printlinks( $_REQUEST, $startdoc, count($groupings), $indri_param[ 'page_docs' ], $prefix ) ?>
  </div>
  <div id="searchbox">
  <form action="query.php" method="post">
    <div id="searchboxquery">

     <input type=text name="query" size="50" value='<?= $cleanQ ; ?>'>
 <input type="submit" value="Search <?= $indri_param['sitename']?>">
</div>
   </form>
</div>

  <?php include("include/footer.php"); ?>
  <?php $env->close(); ?>
</div> <!-- content -->

</body>
</html>

