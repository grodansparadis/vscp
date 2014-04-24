<?php
  //session_start();

require_once('conf.php');
require_once('Driver.php');
require_once('utils.php');
    
//Setup correct driver
$driver = &DTtemp_Driver::factory($conf['driver'], $conf['sql']);
$driver->connect();  //must call to connect

//check if sensor list is set to all
if (isset($sensor_all) && $sensor_all=="all") {
  unset($sensor);
 }

$list = $driver->listSensors();

//    print_r($stats);
$startOfData =  $driver->getStartOfData();

if ($conf['alarms']['display']) {
  $alarms = $driver->getActiveAlarms();
 }

$startTime = null;
$endTime = null;
if (isset($offset) || isset($endoffset)) {
  $startTime = time() + $offset;
  $endTime = time() + $endoffset;
 } else {
  //parse out components
  if (isset($startHour)) {
    //figuring that hour is enough to assume the others are set
    $startTime = mktime($startHour, $startMinute, 0, $startMonth, $startDay, $startYear);
    //echo "made startTime =$startTime";
  }
  if (isset($endHour)) {
    $endTime = mktime($endHour, $endMinute, 0, $endMonth, $endDay, $endYear);
    //echo "Made endTime = $endTime";
  }
 }

$times = Utils::getDisplayDataTimesAbs($startTime,$endTime);

$units = $conf['data']['units']; //use default
if (isset($toggleUnits)) {
  $units = getOtherUnit($units);
 }

if(isset($show_stats)) {
  $stats = $driver->getStats($sensor, $times['startTime'], $times['endTime'], $units);
  $stats_sorted = array();
 }

$driver->close();
    
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<HTML>
    <HEAD>
        <meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
        <TITLE>DTtemp</TITLE>
    </HEAD>
    <BODY bgcolor="<?php echo $conf['html']['bgcolor'] ?>">
    

    <FORM action="." method="<?php echo $conf['html']['formMethod']; ?>">
    <TABLE border="3">
    <TR>
            <TD>
                <INPUT type="submit" value="Show">
            </TD>
                <?php 
                    require "dateSelector.php";
                ?>
            <TD>
    <?php
        echo "Readings: ".$driver->countEvents($sensor, $times['startTime'], $times['endTime']);
    ?>
            </TD>
        </TR>
        <TR> 
            <TD colspan="3">
                <?php
                    $imgurl = "graph.php?";
                    $imgurl .= "startTime=$startTime";
                    if (isset($endTime)) {
                        $limitedEndTime = $endTime;
                        if ($endTime > time()) {
                            echo "Resetting end Date";
                            //if I don't do this,
                            //it restricts to nonexitstent (in this range)
                            //hours
                            $limitedEndTime = time();
                        }
                        $imgurl .= isset($endTime) ? "&amp;endTime=$limitedEndTime" : "";
                    }
                    $imgurl .= isset($showRepeats) ? "&amp;showRepeats=$showRepeats" : "";
                    $imgurl .= isset($showLegend) ? "&amp;showLegend=$showLegend" : "";
                    $imgurl .= isset($showMargin) ? "&amp;showMargin=$showMargin" : "";
                    $imgurl .= isset($showNegatives) ? "&amp;showNegatives=$showNegatives" : "";
                    $imgurl .= isset($showBands) ? "&amp;showBands=$showBands" : "";
                    $imgurl .= isset($showMarks) ? "&amp;showMarks=$showMarks" : "";
                    $imgurl .= isset($showAll) ? "&amp;showAll=$showAll" : "";
                    $imgurl .= isset($datesAbsolute) ? "&amp;datesAbsolute=$datesAbsolute" : "";
                    $imgurl .= isset($units) ? "&amp;units=$units" : "";
                    if(isset($sensor) && $sensor!="all") {
                        $imgurl .= "&amp;sensor[]=";
                        $imgurl .= join("&amp;sensor[]=",$sensor);
                    }
                ?>
                <input type="image" src="<?php echo $imgurl; ?>" alt="Main Graph of Temperatures. Clicking it will refresh.  Note that selecting too much data may cause the server to time out without producing an image - do not use the All Data checkbox carelessly.">

            </TD>

            <!-- add other options here -->

            <TD valign="top"> <!-- selection - it's own table-->
            <TABLE border="1" cellpadding="0" cellspacing="0">
                <TR>
                <TH colspan="2">Sensor</TH>
                <TH>&nbsp;</TH>
                </TR>
                <TR>
                <?php
                    
                    echo makeTDCheckBox("sensor_all", "all", false, "All",
                        "Select All and selection will clear on next redraw", "red");
                ?>
                    <TD colspan=1></TD>
                </TR>
                <?php 
                //print_r($list);
                    while (list($serial, $s) = each($list)) {
                        if (isset($show_stats) && isset($stats[$serial])) {
                            //cheating here - resorting 
                            //the Stats array on the fly
                            //Note that I'm assuming here
                            //that the list will always have all the items
                            //in the stats array... which seems to make sense
                            $stats_sorted[$serial] = $stats[$serial];
                        }
                        $description = $s['description']." [".$s['SerialNumber']."]";
                        echo '<TR>';
                        echo makeTDCheckBox("sensor[]", 
                                        $serial, 
                                        (is_array($sensor) && in_array($serial, $sensor)), 
                                        $s['name'],
                                        $description,
                                        $s['color']);
                        //alarm?
                        if (isset($alarms) && isset($alarms[$serial])) {
                            while (list($id, $alarm) = each($alarms[$serial])) {
                                $text .= $s['name'].':'.$alarm['description'].'. Active since '.$alarm['time_raised'];
                                //append each alarm
                            
                            }
                            echo makeTD($content, $text, 'red', 'icon-warning.gif');

                        } else {
                            echo '<TD></TD>';
                        }
                        echo '</TR>';
                    }
                ?>
                </TABLE>
                <P>
                <!-- options table -->
                <TABLE border="1" cellpadding="0" cellspacing="0">
                    <TR>
                        <TH colspan="2">Options</TH>
                    </TR>
                    <?php
                        echo makeTRCheckBox("show_stats",
                                            1,
                                            isset($show_stats),
                                            "Stats",
                                            "Show detailed Statistics for selected interval (below)");
                        echo makeTRCheckBox("showLegend",
                                            1,
                                            isset($showLegend),
                                            "Legend",
                                            "Show Legend box on the graph");
                        echo makeTRCheckBox("showMargin",
                                            1,
                                            isset($showMargin),
                                            "Margin",
                                            "Expand right margin for legend box");
                        echo makeTRCheckBox("showMarks",
                                            1,
                                            isset($showMarks),
                                            "Plot Marks",
                                            "Show Marks for each measurement on the graphs");
                        echo makeTRCheckBox("showBands",
                                            1,
                                            isset($showBands),
                                            "Range",
                                            "Highlight a tolerance range for each sensor for which min/max are set");
                        echo makeTRCheckBox("showNegatives",
                                            1,
                                            isset($showNegatives),
                                            "Negatives",
                                            "Allow negatives by using endTime ZERO instead of startTime");
                        echo makeTRCheckBox("toggleUnits",
                                            1,
                                            isset($toggleUnits),
                                            getOtherUnit($conf['data']['units']),
                                            'Show '.  getOtherUnit($conf['data']['units']) . ' units for all temperatures');
                        echo makeTRCheckBox("datesAbsolute",
                                            1,
                                            isset($datesAbsolute),
                                            'Absolute Dates',
                                            'Select date range using absolute (and fixed) terms instead of relative');
                        echo makeTRCheckBox("showRepeats",
                                            1,
                                            isset($showRepeats),
                                            "Repeats",
                                            "Show unchanged consecutive measurements on the graph",
                                            null,
                                            "Note: showing repeats is only noticeable when Plot Marks are on. Depending on readings, it may slow rendering down");
                        echo makeTRCheckBox("showAll",
                                            1,
                                            isset($showAll),
                                            "All Data",
                                            "Show All available measurements without trimming (may be extremely slow). Show Repeats still applies",
                                            null,
                                            "Warning: selecting too much data will not be guarded against! It may take too long to generate image.  Use with caution");
                    ?>
                </TABLE>
            </TD>
        </TR>
    </TABLE>
    </FORM>

    <!-- show alarm notices? -->

    <?php
        if (isset($alarms) ) {
            reset($alarms);
            if (count($alarms) > 0) {
            ?>
            <TABLE border="1">
            <TR><TH>&nbsp;</TH>
                <TH>Sensor</TH>
                <TH>Temp</TH>
                <TH>Alarm Type</TH>
                <TH>Time Raised</TH>
                <TH>Updated</TH>
                <TH>Description</TH>
            </TR>
            <?php
                while (list($serial, $alarm) = each ($alarms)) {
                    while (list($id, $alarm) = each($alarms[$serial])) {
                        //$text .= $s['name'].':'.$alarm['description'].'. Active since '.$alarm['time_raised'];
                        //append each alarm
                        echo '<TR>'.makeTD('Alarm', 'Alarm!', null, 'icon-warning.gif').'<TD>';
                        echo color($list[$serial]['name'], $list[$serial]['color']);
                        echo '</TD><TD>';
                        if (isset($toggleUnits)) {
                            echo Utils::myRound(Utils::toCelcius($alarm['Fahrenheit']),2).'C';
                        } else {
                            echo $alarm['Fahrenheit'].'F';
                        }
                        echo '</TD><TD>'.$alarm['alarm_type'].'</TD><TD>'.$alarm['time_raised'].'</TD><TD>'.$alarm['time_updated'].'</TD><TD>'.$alarm['description'].'</TD></TR>';
                    
                    }
                    //echo makeTD($content, $text, 'red', 'icon-warning.gif');
                }
            ?>
            </TABLE>
            <?php
            }

        }

    ?>
    
    <!-- show stats table? -->
    <?php
        if (isset($show_stats)) {
            ?>
                <TABLE border="1">
                    <?php
                        //drawing horizontally so need two nested loops
                        //$keylist = array_pop($stats); //this was one way
                        //Junk is needed cause i'm reading ahead
                        $keylist= array('Current' => 'Latest' , 'min' => 'Min', 'max' => 'Max', 'avg' => 'Average' ,'junk' => 'You should never see this');
                        reset($stats_sorted);
                        unset($key);
                        while (list($keynext, $keynext_desc) = each($keylist)) {
                            echo "<TR>";
                            //echo " Running with $keynext, $valtrash";
                            if (!isset($key)) {
                                echo "<TH>Sensor</TH>";
                            } else {
                                echo "<TH>$key_desc</TH>";
                            }
                            reset($stats_sorted);
                            while(list($ser,$st) = each($stats_sorted)) {
                                if (!isset($key)) {
                                    //draw header
                                    echo "<TH>";
                                    echo color($list[$ser]['name'], $list[$ser]['color']);
                                    echo "</TH>\n";
                                } else {
                                    echo '<TD>';
                                    echo Utils::myRound($st[$key], $conf['data']['displayPrecision']);
                                    echo "</TD>\n";
                                    // draw that value
                                }

                            }
                            $key = $keynext; ///next time it will be drawn :)
                            $key_desc = $keynext_desc; ///next time it will be drawn :)
                            echo "</TR>";
                        }

                    ?>
                </TABLE>
            <?php
        }

    ?>

    </BODY>

</HTML>


<?php
//Supporting functions
    /**
     * Used to draw a described checkbox in a table
     * takes care of everything including surrounding TR's
     * @param cginame - the input name
     * @param value - the input value to be passed back
     * @param state - boolean, if true - makes it checked
     * @param name - the Text to show next to the checkbox
     * @param description - the text to show with onhover (if supported)
     */
    function makeTDCheckBox($cginame, $value = 1, $state = false, $name, $description, $color = null, $onClickWarning = null) {
        $result =  "<TD>";
        $result .= "<INPUT type='checkbox' name='$cginame' value='$value' ".($state ? " CHECKED " : "");
        if (isset($onClickWarning)) {
            $result .= " onclick=\"if (value==checked) alert('$onClickWarning')\"; ";
        }
        $result .= ">";
        //$result .= "<TD><A onmouseout=\"window.status='';\" onmouseover=\"window.status='$description'; return true;\">";
        $result .= "</TD>\n";
        $result .= makeTD($name, $description, $color);

        return $result;
    }

    /**
     * Makes an active TD segment with an onclick/onmouseover
     */
    function makeTD($name, $description, $color, $imgname = null) {
        $result = "<TD><A onmouseout=\"window.status=''\"";
        $result .= " onmouseover=\"window.status='$description'\"";
        $result .= " onclick=\"alert('$name: $description')\">";
        if (isset($imgname)) {
            $result .= makeIMG($imgname, "$name: $description");
        } else {
            $result .= color($name, $color);
        }
        $result .= "</A></TD>\n";
        return $result;
    }

    function makeIMG($imgname, $alt = null) {
        return "<IMG src=\"images/$imgname\" alt=\"$alt\">";
    }

    function makeTRCheckBox($cginame, $value = 1, $state = false, $name, $description, $color = null, $onClickWarning = null) {
        return '<TR>'.makeTDCheckBox($cginame, $value, $state, $name, $description, $color, $onClickWarning).'</TR>';
    }
    /**
     * Returns a <FONT color="$color">$text</FONT>
     */
    function color($text, $color) {
        if (!isset($color)) {
            $result = $text;
        } else {
            $result = "<FONT color=\"$color\">$text</FONT>";
        }
        return $result;
    }

    /**
     * Returns "Celsius" given "Fahrenheit"
     * and vice versa
     */
    function getOtherUnit($unit) {
        if ($unit == 'Fahrenheit') {
            return 'Celsius';
        } else if ($unit == 'Celsius') {
            return 'Fahrenheit';
        } else {
            echo "ALARM! unknown unit : $unit";
        }
    }

?>

