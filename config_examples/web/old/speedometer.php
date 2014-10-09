<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">
<title>Temperature widget 1</title>

<script type="text/javascript" src="../lib/wz_jsgraphics.js"></script>
<script type="text/javascript" src="../lib/ohas.js"></script>

</head>

<body>

<h1>Test Speedometer widget 1</h1>


<script type="text/javascript" >
	var speed1 = new speedometer1(<?php echo $_GET["x"]; ?>,<?php echo $_GET["y"]; ?>);
	speed1.init();
	speed1.setvalue(<?php echo $_GET["value"]; ?>);
</script>
</div>
</body>
</html>