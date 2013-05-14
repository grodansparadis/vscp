<HTML>
    <HEAD><TITLE>Alarms Updating @ <?php echo date("H:i", time()); ?></TITLE></HEAD>
    <BODY>
<?php
    $dtDir = '../'; //may have to adjust if you move admin directory contents

    //need to get conf so that invoked classes can use it
    require_once($dtDir.'conf.php');
    require_once($dtDir.'Driver.php');

    //Setup correct driver
    $driver = &DTtemp_Driver::factory($conf['driver'], $conf['sql']);
    $driver->connect();  //must call to connect

    ////////////////////// Initializied //////////////////

    echo "Updating alarms.... ";
    $newAlarms = $driver->updateAlarms();
    echo "<P>Alarm updating complete";

    $driver->close();
    if (is_array($newAlarms) && count($newAlarms) > 0 ) {
        echo "<P><B>New Alarms raised!</B>";

        if ($conf['alarms']['notify']) {
            echo "Notifying ".$conf['alarms']['notifyEmail'];
            while (list ($num, $alarm) = each ($newAlarms)) {
                if (is_array($alarm)) {
                    $text .= $alarm['sensor'].' : '.$alarm['text']."\n";
                } else {
                    $text .= $alarm."\n";
                }
            }
            mail($conf['alarms']['notifyEmail'], 'Temperature Alarm(s) raised', $text);
        } else {
            echo 'Notification turned off... doing nothing more';
        }
    } else {
        echo '<BR> No new alarms';
    }
    //echo "<P>Checking alarms:";
    //$alarms = $driver->getActiveAlarms();
    //print_r($alarms);
?>
