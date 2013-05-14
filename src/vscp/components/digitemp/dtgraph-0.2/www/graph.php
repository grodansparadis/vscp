<?php
/*
 * This file is part of an open-source project
 * licensed under the GPL
 *
 */

    //session_start();

    require_once('conf.php');
    require_once('Driver.php');
    require_once('utils.php');

    //JPGraph
    require_once($conf['jp_path']."/jpgraph.php");
    require_once($conf['jp_path']."/jpgraph_line.php");
    
    //Setup correct driver
    $driver = &DTtemp_Driver::factory($conf['driver'], $conf['sql']);
    $driver->connect();  //must call to connect
    global $colors;
    $colors = $conf['graph']['colors'];

    global $stop_watch;
    $stop_watch['Start'] = microtime();
    
    //echo "Sensor is passed in as $sensor";
    if ($sensor == "all") {
        unset($sensor);
    }

    if (!isset($units)) {
        $units = $conf['data']['units'];
    }
    $l = $driver->listSensors($sensor, $units);

    $stop_watch['Done list sensors'] = microtime();
    
    $times = Utils::getDisplayDataTimesAbs($startTime, $endTime);

    $stop_watch['Done get Display Data TImes'] = microtime();
    
    if (isset($showRepeats) && $showRepeats == 1) {
        $showRepeats = true;
    } else {
        $showRepeats = false;
    }

    $options = array();
    $options['showLegend'] = $showLegend;
    $options['showMargin'] = $showMargin;
    $options['showNegatives'] = $showNegatives;
    $options['showBands'] = $showBands;
    $options['showMarks'] = $showMarks;
    $options['showAll'] = $showAll;

    $options['units'] = $units;


    $d = $driver->listEvents($sensor, $times['startTime'], $times['endTime'], $showRepeats, !isOptionOn($options,'showAll'), $units); 
    $stop_watch['Done list Events'] = microtime();
    //echo "events starting at ". $times['startTime']. " ending at ".$times['endTime'];
    
    graph($d,$l, $times, $options);
    $stop_watch['Done Graph'] = microtime();
    //Utils::echo_stopwatch();

        
            /** Functions **/
            /***************/

    /**
     * @param data - the return of listEvents
     * @param list   - the return of listSensors
     * @param times  - the return of getDisplayDataTimes
     * @param options - array of various graph options/params
     */
    function graph($data, $list, $times, $options = array()) {
        global $colors; //for defaults if not set
        global $conf;
        $units = $options['units'];
        $xdata = array();
        $ydata = array();
        $names = array();

        $negs =  (isset($options['showNegatives']) && $options['showNegatives'] == 1);

        $varPrefix = $negs ? "this" : "requested";
        $varSuffix = 'Hour';
        $varName = $varPrefix.$varSuffix; //flexible graphing...

        //Time used to generate the explanatory text on X scale

        $startTime = $times['startTime'];
        $endTime = $times['endTime'];

        $explanationTime = $negs ? $endTime : $startTime;

        //$relativeStart = ( ($negs == true) ? $times['thisHour'] : $times['requestedHour']);
        //echo "$relativeStart == ? == ".$times['thisHour'] . " OR " . $times['requestedHour'];

        //echo "using negs =" . $negs ? "true " : " false";

        $conversionFactor = 60; // seconds in an X scale unit
        $XUnits = "Minutes since ".date("g A m/d", $explanationTime);
        if ( ($endTime - $startTime) > 3600 * 24 * 28) { //one month?
            $conversionFactor = 3600 * 24 * 30.5; 
            $XUnits = "Approximate Months starting " .date("Y", $explanationTime);
            $varSuffix = 'Year';
        } else if( ($endTime - $startTime) > 3600 * 24 * 1) { //one day :)
            $conversionFactor = 3600 * 24; //days
            $XUnits = "Calendar Days starting ".date("F", $explanationTime)." 1st";
            $varSuffix = 'Month';
        } else if( ($endTime - $startTime) > 3600 * 2) { //2 hours
            $conversionFactor = 3600;
            $XUnits = "Hours since midnight ".date("m/d", $explanationTime);
            $varSuffix = 'Morning';
        }  
        
        //This start point will be the ZERO
        $relativeStart = $times[$varPrefix.$varSuffix];
/**/
        //setup arrays
        while (list($serial,$s) = each($list)) {
            $xdata[$serial] = array();
            $ydata[$serial] = array();
            $names[$serial] = $s['name'];
        }
/**/
        //setup xdata and ydata
        while(list($row, $d) = each ($data)) {
            $serial = $d['SerialNumber'];
            //safety, should not be needed
            if (!is_array($xdata[$serial])) {
                $xdata[$serial] = array();
                $ydata[$serial] = array();
                $names[$serial] = $serial; //it must not be described
            }

            
            $time = $d['unixtime'];
            $x = sprintf("%0.3f", ($time - $relativeStart)/ $conversionFactor);
            
            //echo "Based on $startTime - $relativeStart div by $conversionFactor, it's $x <BR>";

            //$temp = $d['Fahrenheit'];
            $temp = $d[$units];
/**/
/**/
            array_push($xdata[$serial], $x);
            array_push($ydata[$serial], $temp);
            //echo "Adding point $x - $temp";

        }

        //$graph = setupGraph($mintime,$maxtime,$mintemp, $maxtemp);
        $count = count($data);
        $graph = setupGraph($XUnits, $startTime, $count, $options);


        reset($list); //already walked this one
        //make plots based on arrays
        
        $graphHasData = false; //if it doesn't have any data, it will blow up

        
        $plots = array();
        while (list($serial,$s) = each($list)) {
            $color = $list[$serial]['color'];
            if (!isset($color) || $color=='') {
                //Take the default just in case
                $color = array_pop($colors);
            }
            $plots[$serial] = makePlot($xdata[$serial], $ydata[$serial], $names[$serial], $color, $options);
            if (sizeof($xdata[$serial]) > 0 ) {
                $graphHasData = true;
            }
        
            //tossing each into its own var as it seems to be 
            //pass by ref and overwrites each time?
            if (isset($plots[$serial])) {
                /*
                echo "Adding a plot: " ;
                print_r($plot);
                echo "<P><HR>";
                */
                $graph->Add($plots[$serial]);
                
                if (isset($options['showBands']) && $options['showBands'] == 1) {
                    if (isset($list[$serial]['max'])) {
                        $max = $list[$serial]['max'];
     //                   $graph->Add(new PlotLine(HORIZONTAL,$max, $color, 1)); 
                    }
                    if (isset($list[$serial]['min'])) {
                        $min = $list[$serial]['min'];
    //                    $graph->Add(new PlotLine(HORIZONTAL,$min, $color, 1)); 
                    }

                    /******** ADD BAND ******** ? ***/

                    if (isset($min) && isset($max)) {
                        $uband=new PlotBand(HORIZONTAL,BAND_SOLID,$min,$max,"white", 1, DEPTH_BACK);
                        $uband->ShowFrame(true);
                        $uband->SetDensity(3); // % line density

                        $graph->AddBand($uband);
                        unset($uband);
                    }
                    /******* DONE ADD BAND *******/
                } //if showBands
                
            }
        }

        if (!$graphHasData) {

            $txt =new Text("There is no data in the selected timeframe. Consider expanding the range");
            $txt->Pos( 0.2,0.5);
            $txt->SetColor( "red");
            $graph->AddText( $txt);

            //override scale so it doesn't complain
            $graph->SetScale("linlin",0, 1, 0, 1);
        }
        $graph->Stroke();
        

    }

    function setupGraph($XUnits = "hours", $startTime, $count, $options = array()) {
        global $conf;
        $graph = new Graph($conf['graph']['width'],$conf['graph']['height'],"auto");
        $graph->SetScale("intlin");
        //$graph->SetScale("linlin",$mintemp,$maxtemp,$mintime, $maxtime);

        //$graph->SetShadow();
        $rightMargin = 20;
        if (isset($options['showMargin']) && $options['showMargin'] == 1) {
            $rightMargin = 110;
        }
        $graph->img->SetMargin(50,$rightMargin,20,40);
        $graph->SetBox(true,'black',2);
        $graph->SetColor($conf['graph']['bgcolor']);
        $graph->SetMarginColor($conf['html']['bgcolor']);


        //$graph->title->Set("Digitemp Activity");
        $graph->title->Set("Digitemp Activity starting ".date("H:i:s m/d/Y", $startTime));
        /**
        $txt =new Text("Starting ".date("H:i:s m/d/Y", $startTime));
        $txt->Pos( 0.59,0.01);
        $txt->SetColor( "blue");
        $graph->AddText( $txt);
        **/
    //junk:
        $graph->title->SetFont(FF_FONT1,FS_BOLD);

        $graph->xgrid->Show();

        $graph->legend->Pos(0.02,0.02,"right","top");

/*
        $graph->yaxis->SetPos(0);
        $graph->yaxis->SetWeight(2);
        $graph->yaxis->SetFont(FF_FONT1,FS_BOLD);
        $graph->yaxis->SetColor('black','darkblue');
*/
        $graph->xaxis->SetWeight(2);
        $graph->xaxis->SetFont(FF_FONT1,FS_BOLD);
        $graph->xaxis->SetColor('black','darkblue');


        
       // echo "Setting limits to $mintime, $maxtime, $mintemp, $maxtemp";
        //$graph->SetScale("linlin",$mintemp,$maxtemp,$mintime, $maxtime);
        $graph->xaxis->title->Set("Time ($XUnits)");
        $graph->yaxis->title->Set('Temperature ('. $options['units'] . ')');

        

        $txt2 =new Text("Measurements shown: $count");
        $txt2->Pos( 0.02,0.96);
        $txt2->SetColor( "blue");
        $graph->AddText( $txt2);

        return $graph;

    }

    function makePlot($xd, $yd, $plotName, $color, $options = array()) {
//        global $colors;
//        $color = array_pop($colors);
        //echo "xdata is ".sizeof($xd);
        if (sizeof($xd) == 0) {
            return;
        }
        $plot=new LinePlot($yd, $xd);
        //The only way to not have the legend box show
        //is to not set the info at all
        if (isOptionOn($options, 'showLegend')) {
            $plot->SetLegend($plotName);
        }
        $plot->SetColor($color);
        
        if (isOptionOn($options, 'showMarks')) {
            $plot->mark->SetType(MARK_DIAMOND);
            $plot->mark->Show();
            $plot->mark->SetColor($color);
        }
        //echo "<BR> Making plot for $plotName";
        //print_r ($plot);
        return $plot;
    }

    /**
     * Convenience method to check if the option has been 
     * passed in and the value is 1
     */
    function isOptionOn($options = array(), $optionName) {

        if (isset($options[$optionName]) && $options[$optionName] == 1) {
            return true;
        }
        return false;
    }

?>
