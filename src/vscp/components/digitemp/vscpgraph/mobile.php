<HTML>
    <HEAD><TITLE>Mobile Temp @ <?php echo date("H:i", time()); ?></TITLE></HEAD>
    <BODY>
<?php
    require_once('conf.php');
    require_once('Driver.php');
    require_once('utils.php');

    //Setup correct driver
    $driver = &DTtemp_Driver::factory($conf['driver'], $conf['sql']);
    $driver->connect();  //must call to connect

    ////////////////////// Initializied //////////////////

    if (!isset($hours)) {
        $hours = 1; //default - one hour
    }
    $duration = 3600 * $hours; 
    if ($duration > time()) {
        $duration = time(); //safety to avoid negative startTimes
    }
    
    $list = $driver->listSensors();
    //$stats = $driver->getStats($sensor, $times['startTime'], $times['endTime']);
    global $conf;
    $stats = $driver->getStats(null, time() - $duration, time() , $conf['data']['units']);
    $precision = $conf['data']['displayPrecision'];
    //print out stats
    while (list($serial, $s) = each($list)) {
        echo $s['name'];
        echo ':';
        if (isset($showStats)) {
            echo Utils::myRound($stats[$serial]['min'], $precision);
            echo '-';
            echo Utils::myRound($stats[$serial]['max'], $precision);
            echo '~' ;
            echo Utils::myRound($stats[$serial]['avg'], $precision);
        } else {
            echo Utils::myRound($stats[$serial]['Current'], $precision);
        }
        echo '<BR>';
    }

    if (isset($showStats)) {
        echo "<A HREF=\"mobile.php\">Normal</A>";
    } else {
        echo "<FORM action=\"mobile.php\" method=\"GET\">";
        echo "Hrs<INPUT type=\"text\" name=\"hours\" size=\"3\" value=\"$hours\">";
        echo "<INPUT type=\"hidden\" name=\"showStats\" value=\"1\">";
        echo "<INPUT type=\"submit\" value=\"Show Stats\">";
        //echo "<A HREF=\"mobile.php?showStats=1\">Statistics</A>";
        echo "</FORM>";
    }
    
        
?>

</BODY>
</HTML>
