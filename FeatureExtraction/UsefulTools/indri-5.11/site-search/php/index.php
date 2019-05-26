<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<?php include("include/config.php"); ?>
<head>
<?php include("include/title.php"); ?>
<link rel="stylesheet" type="text/css" href="style/style.css" title="stylesheet" />
</head>
<body>

<div id="content">
  <?php include("include/header.php"); ?>

  <form action="query.php" method="post">
    <div id="query">
     <input type=text name="query" size="70"></textarea>
<p>     <input type="submit" value="Search <?= $indri_param['sitename']?>">
     <?= $indri_param['search_text'] ?>
    </div> <!-- query -->
   </form>

  <?php include("include/footer.php"); ?>
</div> <!-- content -->

</body>
</html>

