///////////////////////////////////////////////////////////////////////////////////
// vscputils
//
// Code originally from the dtGraph project of DigiTemp (http://www.digitemp.com)
//
// 
<?

///////////////////////////////////////////////////////////////////////////////////
// getTimeString
//
// Converts now or supplied unix time
// to mysql timestamp
//

function getTimeString($date = null) {
  if ( !isset( $date ) ) {
    // set it to now
    $date = time();
  }
  return date("YmdHis", $date);
}



///////////////////////////////////////////////////////////////////////////////////
// toFahrenheit 
//
// Converts a Celsius value to Fahrenheit
// If not set, returns null
//

function toFahrenheit( $temp ) {

  if ( !isset( $temp ) ) {
    return null;
  }
  // F.value = (212-32)/100 * this.value + 32
  return ( 100/(212-32) * ($temp - 32 ));
}

///////////////////////////////////////////////////////////////////////////////////
// toCelsius
//
// Converts a Fahrenheit Value to Celcius
// If not set, returns null
//

function toCelsius($temp) {

  if (!isset($temp)) {
    return null;
  }
  // C.value = 100/(212-32) * (this.value - 32 )
  return (($temp - 32) *  5 / 9 );
}

///////////////////////////////////////////////////////////////////////////////////
// myRound
//
// Instead of zero that round would print,
// this prints a question mark for undefined fields
//

function myRound($value, $precision) {
  if (isset($value)) {
    return round($value, $precision);
  } else {
    return '?';
  }
}
  
///////////////////////////////////////////////////////////////////////////////////
// echo_stopwatch
//  
// Borrowed from  posted comments on php.net
// This is a useful profiling print function
//
// use by:
// global $stop_watch;
// $stop_watch['Start'] = microtime();
// $stop_watch['Some Event'] = microtime();
// ..... so on
// echo_stopwatch()
//
// (Output goes in html comments)
//

function echo_stopwatch()
{
  global $stop_watch;
  
  echo "\n\n<!--\n";
  echo "\nTiming ***************************************************\n";
  
  $total_elapsed = 0;
  list($usec, $sec) = explode(" ",$stop_watch['Start']);
  $t_end = ((float)$usec + (float)$sec);
  
  foreach( $stop_watch as $key => $value ) {
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


///////////////////////////////////////////////////////////////////////////////////
// getDisplayDataTimesAbs
//
// Works with epoch seconds
//

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

?>
