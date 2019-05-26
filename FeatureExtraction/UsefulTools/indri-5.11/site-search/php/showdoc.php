
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<?php include("include/libindri.php") ?>

<head>
<title>Indri document result</title>
</head>
<body>

<?php 
   // make a query environment.
   $env = new QueryEnvironment();
   $numdocs = indri_setupenvironment( $indri_param, $env, $_REQUEST );
   $numericDocID = (int) $_REQUEST['documentID'];
   $reqdocs = array( $numericDocID );
   $docs = $env->documentsdocids( $reqdocs );
?>
<?php
  if( $indri_param[ "document_format" ] == "text" ) {
    echo "<pre>\n";
    echo indri_escapetags( $docs[0]->content );
    echo "</pre>\n";
  } else {
    echo indri_insert_base_tag( $docs[0] );
  }
?>

<?= $env->close(); ?>

</body>
</html>

