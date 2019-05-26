
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<?php include("indrihelpers") ?>

<head>
<title>Indri document result</title>
</head>
<body>

<?php 
   // make a query environment.
   $env = new QueryEnvironment();
   $numdocs = indri_setupenvironment( $env, $_REQUEST );
   $numericDocID = (int) $_REQUEST['documentID'];
   $reqdocs = array( $numericDocID );
   $docs = $env->documentsdocids( $reqdocs );
?>

<pre>
<?= $docs[0]->text ?>
</pre>

<?= $env->close(); ?>

</body>
</html>

