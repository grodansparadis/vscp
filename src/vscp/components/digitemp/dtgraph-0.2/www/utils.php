<?php

class Utils {


    /**
     * Abort with a fatal error, displaying debug information to the
     * user.
     *
     * @access public
     *
     * @param object PEAR_Error $error  An error object with debug information.
     * @param integer $file             The file in which the error occured.
     * @param integer $line             The line on which the error occured.
     * @param optional boolean $log     Log this message via Horde::logMesage()?
     */
    function fatal($error, $file, $line, $log = true)
    {

        $errortext = _("<b>A fatal error has occurred:</b>") . "<br /><br />\n";
        if (is_object($error) && method_exists($error, 'getMessage')) {
            $errortext .= $error->getMessage() . "<br /><br />\n";
        }
        $errortext .= sprintf(_("[line %s of %s]"), $line, $file);

        if ($log) {
            $errortext .= "<br /><br />\n";
            $errortext .= _("Details have been logged for the administrator.");
        }

        // Log the fatal error  if requested.
        if ($log) {
         //   Horde::logMessage($error, $file, $line, LOG_EMERG);
        }

        // Hardcode a small stylesheet so that this doesn't depend on
        // anything else.
        echo <<< HTML
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "DTD/xhtml1-transitional.dtd">
<html>
<head>
<title>DTtemp :: Fatal Error</title>
<style type="text/css">
<!--
body { font-family: Geneva,Arial,Helvetica,sans-serif; font-size: 12px; background-color: #222244; color: #ffffff; }
.header { color: #ccccee; background-color: #444466; font-family: Verdana,Helvetica,sans-serif; font-size: 12px; }
-->
</style>
</head>
<body>
<table border="0" align="center" width="500" cellpadding="2" cellspacing="0">
<tr><td class="header" align="center">$errortext</td></tr>
</table>
</body>
</html>
HTML;

        exit;

    }

    /**
     * Converts now or supplied unix time
     * to mysql timestamp
     */
    function getTimeString($date = null) {
        if (!isset($date)) {
            //set it to now
            $date = time();
        }
        return date("YmdHis", $date);
    }

    /**
     * Converts a Fahrenheit Value to Celcius
     * If not set, returns null
     */
    function toCelcius($temp) {
        if (!isset($temp)) {
            return null;
        }
        return (($temp - 32) *  5 / 9 );
    }

    /**
     * Instead of zero that round would print,
     * this prints a question mark for undefined fields
     */
    function myRound($value, $precision) {
        if (isset($value)) {
            return round($value, $precision);
        } else {
            return '?';
        }
    }
    
    /**
     * Borrowed from  posted comments on php.net
     * This is a useful profiling print function
     *
     * use by:
     * global $stop_watch;
     * $stop_watch['Start'] = microtime();
     * $stop_watch['Some Event'] = microtime();
     * ..... so on
     * echo_stopwatch()
     *
     * (Output goes in html comments)
     */
    function echo_stopwatch()
    {
        global $stop_watch;

        echo "\n\n<!--\n";
        echo "\nTiming ***************************************************\n";

        $total_elapsed = 0;
        list($usec, $sec) = explode(" ",$stop_watch['Start']);
        $t_end = ((float)$usec + (float)$sec);

        foreach( $stop_watch as $key => $value )
        {
            list($usec, $sec) = explode(" ",$value);
            $t_start = ((float)$usec + (float)$sec);

            $elpased = abs($t_end - $t_start);
            $total_elapsed += $elpased;

            echo str_pad($key, 20, ' ', STR_PAD_LEFT).": ".number_format($elpased,3).' '.number_format($total_elapsed,3), 0 ;
            echo "\n";
            $t_end = $t_start;
        }

        echo "\n";
        echo str_pad("Elapsed time", 20, ' ', STR_PAD_LEFT).": ".number_format($total_elapsed,3), 0 ;
        echo "\n";
        echo "\n-->";
    }


    /**
     * Works with epoch seconds
     */
    function getDisplayDataTimesAbs(&$startTime , &$endTime) {
        global $conf;
        $now = time();
        if (!isset($startTime) || $startTime == "") {
            $startTime = $now + $conf['data']['defaultOffset'];
        }
        if (!isset($endTime) || $endTime == "") {
            $endTime = $now;
        }


        $result = array();

        $thisMorning = mktime(
                0,
                0,
                0,
                date("m", $endTime),
                date("d", $endTime),
                date("Y", $endTime));
        $requestedMorning = mktime(
                0,
                0,
                0,
                date("m", $startTime),
                date("d", $startTime),
                date("Y", $startTime));
        $thisHour = mktime(
                date("G", $endTime),
                0,
                0,
                date("m", $endTime),
                date("d", $endTime),
                date("Y", $endTime));
        $requestedHour = mktime(
                date("G", $startTime),
                0,
                0,
                date("m", $startTime),
                date("d", $startTime),
                date("Y", $startTime));
        $thisMonth = mktime(
                0,
                0,
                0,
                date("m", $endTime),
                0,
                date("Y", $endTime));
        $requestedMonth = mktime(
                0,
                0,
                0,
                date("m", $startTime),
                0,
                date("Y", $startTime));
        $thisYear = mktime(
                0,
                0,
                0,
                0,
                0,
                date("Y", $endTime));
        $requestedYear = mktime(
                0,
                0,
                0,
                0,
                0,
                date("Y", $startTime));
        $startRelative = $startTime - $requestedMorning;  //seonds since midnight of start day

        $result['thisMorning'] = $thisMorning;
        $result['requestedMorning'] = $requestedMorning;
        $result['thisHour'] = $thisHour;
        $result['requestedHour'] = $requestedHour;
        $result['thisMonth'] = $thisMonth;
        $result['requestedMonth'] = $requestedMonth;
        $result['thisYear'] = $thisYear;
        $result['requestedYear'] = $requestedYear;
        $result['startTime'] = $startTime;
        $result['startRelative'] = $startRelative;
        $result['endTime'] = $endTime;
        $result['duration'] = $endTime - $startTime;
        return $result;
    }
            

}
?>
