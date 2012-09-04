<?php
//////////////////////////////////////////////////////////////////////////////
// This is here to generate the date range selector
// Two modes should be supported: 
// -- Preset start/end drop downs
// -- Individual unit selections
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//  The following vars are expected to have been initialized:
//  $startOfData    (epoch seconds)
//  $datesAbsolute (1/unset)
//  $times          (array)
//////////////////////////////////////////////////////////////////////////////

$currentTime = time();
$duration = $currentTime - $startOfData;


if (isset($datesAbsolute)) {
    $startDataDate = getDate($startOfData);
    $todayDate = getDate($currentTime);
    $startDate = getDate($times['startTime']);
    $endDate = getDate($times['endTime']);

    $months = array('zero', 'Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec');
} else { //relative dates
    //Make current time
    $oneMinute = 60;
    $oneHour = $oneMinute * 60;
    $oneDay = $oneHour * 24;
    $oneMonth = $oneDay * 30; //estimation

    //setup an ass array of possible offsets
    $offsets = array();
    $numMonths = floor($duration / $oneMonth);
    $numDays = floor($duration / $oneDay);
    $numHours = floor($duration / $oneHour); //total number of hours
    $numMinutes = floor($duration / $oneMinute); //total num mins
    if ($numMinutes > 5) {
        for ($i=5 ;$i < $numMinutes && $i < 60 ; $i+=15) {
            $t = ($i * $oneMinute);
         //   echo "t is $t";
            $offsets["-$t"] = "$i Minute(s) Ago";
        }
    }
    
    if ($numHours > 1) {
        for ($i=1 ;$i < $numHours && $i < 24 ; $i+=2) {
            $t = ($i * $oneHour);
        //    echo "t is $t";
            $offsets["-$t"] = "$i Hour(s) Ago";
        }
    }
    if ($numDays > 1) {
        for( $l=1 ; $l < $numDays && $l < 30; $l++) {
            $ago = $l * $oneDay;
            //calculate evening so days end at night
            $agoTime = ($currentTime - $ago);
            //$agoEve = mktime(23,59,59,date("m",$agoTime),date("d",$agoTime) ,date("Y"),$agoTime);
            //$ago = time() - $agoEve;
            $offsets["-$ago"] = "$l Days ago (".date("m/d/y", $agoTime).")";
        }
        unset($ago);
        unset($agoTime);
    }
    if ($numMonths > 1) {
        for ($i = 1 ; $i <= $numMonths ; $i++) {
            $ago = $i * $oneMonth;
            $agoTime = ($currentTime - $ago);
            $offsets["-$ago"] = "$i Months ago (".date("m/d/y", $agoTime).")";
        }
    }
    $offsets[$startOfData - $currentTime] = "Beginning ".date("H:i:s m/d/y", $startOfData);

    //if switching modes, it will expect offset/endoffset but they won't be set
    if (!isset($offset)) {
        $offset = $times['startTime'] - $currentTime;
        $endoffset = $times['endTime'] - $currentTime;

    }

} 



if (!isset($datesAbsolute)) {
?>
    <TD>
        Start Time 
        <SELECT name="offset">
            <?php
                if (isset($offset)) {
                    echo "<OPTION value=$offset>".(0 - $offset)." seconds ago\n";
                }
            ?>
            <?php
                while(list($val,$desc) = each($offsets)) {
                    echo "<OPTION ";
                    if ($val == $offset) {
                        echo "SELECTED";
                    }
                    echo " value=\"$val\">$desc \n";
                }
            ?>
        </SELECT>
        
    </TD>
    <TD>
        End Time 
        <SELECT name="endoffset">
            <?php
                reset($offsets);
                if (isset($endoffset)) {
                    echo "<OPTION value=$endoffset>".(0 - $endoffset)." seconds ago\n";
                }
                //Now item
                echo "<OPTION value=\"\"";
                if (isset($endoffset) && $endoffset == 0) {
                    //echo "<OPTION value=\"$endoffset\">".(0 - $endoffset)." seconds ago\n";
                    echo " SELECTED ";
                }
                echo ">Now\n";
                    
                while(list($val,$desc) = each($offsets)) {
                    echo "<OPTION ";
                    if ($val == $endoffset) {
                        echo "SELECTED";
                    }
                    echo " value=$val>$desc \n";
                }
            ?>
        </SELECT>
    </TD>
    <?php
} else { //starting absolute here
    ?>
    <TD>Start
        <?php
            if ( $startDataDate['year'] != $todayDate['year'] ) {
                //only if we have > 1 year, show year
        ?>
            <SELECT name="startYear">
                <?php
                    for ($i = $startDataDate['year'] ; $i <= $todayDate['year'] ; $i++) {
                        echo "<OPTION value='$i'";
                        if ($i == $startDate['year']) {
                            echo " SELECTED ";
                        }
                        echo ">$i\n";
                    }
                ?>
            </SELECT>/
        <?php } else { ?> 
            <INPUT type="hidden" name="startYear" value="<?php echo $todayDate['year']; ?>">
        <?php } ?>
        <SELECT name="startMonth">
            <?php
                for ($i = 1 ; $i <= 12 ; $i++) {
                    echo "<OPTION value='$i'";
                    if ($i == $startDate['mon']) {
                        echo " SELECTED ";
                    }
                    echo ">$months[$i]\n";
                }
            ?>
        </SELECT>/
        <SELECT name="startDay">
            <?php
                for ($i = 1 ; $i <= 31 ; $i++) {
                    echo "<OPTION value='$i'";
                    if ($i == $startDate['mday']) {
                        echo " SELECTED ";
                    }
                    echo ">".str_pad($i, 2, "0", STR_PAD_LEFT)."</OPTION>\n";
                }
            ?>
        </SELECT>
        <SELECT name="startHour">
            <?php
                for ($i = 0 ; $i <= 23 ; $i++) {
                    echo "<OPTION value='$i'";
                    if ($i == $startDate['hours']) {
                        echo " SELECTED ";
                    }
                    echo ">".str_pad($i, 2, "0", STR_PAD_LEFT)."</OPTION>\n";
                }
            ?>
        </SELECT>:
        <SELECT name="startMinute">
            <?php
                for ($i = 1 ; $i <= 59 ; $i++) {
                    echo "<OPTION value='$i'";
                    if ($i == $startDate['minutes']) {
                        echo " SELECTED ";
                    }
                    echo ">".str_pad($i, 2, "0", STR_PAD_LEFT)."</OPTION>\n";
                }
            ?>
        </SELECT>
    </TD>
    <TD>End
        <?php
            if ( $startDataDate['year'] != $todayDate['year'] ) {
                //only if we have > 1 year, show year
        ?>
            <SELECT name="endYear">
                <?php
                    for ($i = $startDataDate['year'] ; $i <= $todayDate['year'] ; $i++) {
                        echo "<OPTION value='$i'";
                        if ($i == $endDate['year']) {
                            echo " SELECTED ";
                        }
                        echo ">$i\n";
                    }
                ?>
            </SELECT>
        <?php } else { ?> 
            <INPUT type="hidden" name="endYear" value="<?php echo $todayDate['year']; ?>">
        <?php } ?>
        <SELECT name="endMonth">
            <?php
                for ($i = 1 ; $i <= 12 ; $i++) {
                    echo "<OPTION value='$i'";
                    if ($i == $endDate['mon']) {
                        echo " SELECTED ";
                    }
                    echo ">$months[$i]\n";
                }
            ?>
        </SELECT>/
        <SELECT name="endDay">
            <?php
                for ($i = 1 ; $i <= 31 ; $i++) {
                    echo "<OPTION value='$i'";
                    if ($i == $endDate['mday']) {
                        echo " SELECTED ";
                    }
                    echo ">".str_pad($i, 2, "0", STR_PAD_LEFT)."</OPTION>\n";
                }
            ?>
        </SELECT>
        <SELECT name="endHour">
            <?php
                for ($i = 0 ; $i <= 23 ; $i++) {
                    echo "<OPTION value='$i'";
                    if ($i == $endDate['hours']) {
                        echo " SELECTED ";
                    }
                    echo ">".str_pad($i, 2, "0", STR_PAD_LEFT)."</OPTION>\n";
                }
            ?>
        </SELECT>:
        <SELECT name="endMinute">
            <?php
                for ($i = 1 ; $i <= 59 ; $i++) {
                    echo "<OPTION value='$i'";
                    if ($i == $endDate['minutes']) {
                        echo " SELECTED ";
                    }
                    echo ">".str_pad($i, 2, "0", STR_PAD_LEFT)."</OPTION>\n";
                }
            ?>
        </SELECT>

    </TD>


    <?php
}
?>
