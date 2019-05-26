<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<head>
<title>Indri search</title>
<link rel="stylesheet" type="text/css" href="style/style.css" title="stylesheet" />
</head>
<body>

<?php include("include/config.php"); ?>

<div id="content">
  <?php include("include/header.php"); ?>

  <form action="query.php" method="post">
    <div id="query">
     <textarea name="query" cols="70" rows="4"></textarea><br/>
     <input type="submit" value="Search"><br/>
     <br/>
     <?= $indri_param['search_text'] ?>
    </div> <!-- query -->
   </form>

  <?php include("include/footer.php"); ?>
</div> <!-- content -->

</body>
</html>

