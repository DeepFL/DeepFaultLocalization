
<head>
<title>Indri: Query parse error</title>
<link rel="stylesheet" type="text/css" href="style/style.css" title="stylesheet" />
</head>

<body>
  <div id="content">
  <?php include( "header.php" ); ?>
    <h3>The system was unable to understand your query: <?= $_REQUEST['query'] ?></h3>
    Please consult the query language documentation and try again.
  <?php include( "footer.php" ); ?>
  </div>
</body>

