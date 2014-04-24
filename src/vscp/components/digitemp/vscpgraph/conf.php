<?php

/*
 * DTTemp Configuration File
 *
 * Structure loosely based on horde's components
 *  (www.horde.org)
 */

/************** Database Properties *****************/
/****************************************************/
$conf['driver'] = 'sql';

$conf['sql']['phptype'] = 'mysql';

// What protocol will we use to connect to the database.  The default
// is tcp, but if you wish to use UNIX sockets, change it to 'unix'.
$conf['sql']['protocol'] = 'tcp';

// If you are using UNIX sockets, you may need to specify the socket's
// location.
// $conf['sql']['socket'] = '/var/lib/mysql/mysql.sock';

// What port is the database server running on? Not required for all
// databases.
// $conf['sql']['port'] = 5432;

// What hostname is the database server running on, or what is the
// name of the system DSN to use?
$conf['sql']['hostspec'] = 'localhost';

// What username do we authenticate to the database server as?
$conf['sql']['username'] = 'apache';

// What password do we authenticate to the database server with?
$conf['sql']['password'] = '';

// What database name/tablespace are we using?
$conf['sql']['database'] = 'stats';

//Main table name that digitemp_mysql.pl logs to
$conf['sql']['table'] = 'digitemp';

//Metadata table describing available sensors
$conf['sql']['table_meta'] = 'digitemp_metadata';

//Alarms table describing raised/old alarms
$conf['sql']['table_alarms'] = 'digitemp_alarms';

// What charset does the database use internally?
$conf['sql']['charset'] = 'iso-8859-1';




/************** JPGraph Properties ******************/
/****************************************************/

//The dir path under which jpgraph.php and others can be found
//This may be relative or absolute (filesystem, not url)
$conf['jp_path'] = "/var/www/html/jpgraph";


/************** Graph Properties ********************/
/****************************************************/
$conf['graph'] = array(); //leave this line alone!

//The colors define the graph colors in reverse order
//The colors now come from the metadata table, but
//these are defaults for yet undescribed sensors :)
$conf['graph']['colors'] = array("black","gray","blue","red","green","yellow","magenta","purple");

//Background color of the graph image
$conf['graph']['bgcolor'] = '#F0F0FF';

//Graph size below (in pixels):
//$conf['graph']['width'] = 800;
//$conf['graph']['height'] = 600;
$conf['graph']['width'] = 640;
$conf['graph']['height'] = 480;

$conf['data'] = array(); //leave alone

/*** UNITS ****/
//Temp display units. Logging is assumed to be done in fahrenheit
//(As is suggested by the schema)
//Possible values are : "Celcius", "Fahrenheit"
//Note that changing this changes default, 
//as well as changing the checkbox in the inteface to offer the
//other unit of measurement instead
$conf['data']['units'] = 'Fahrenheit';
//$conf['data']['units'] = 'Celcius';

/*** Precision ***/
//Number of digits to show after decimal point
//in stats, etc
$conf['data']['displayPrecision'] = 2;


/**** Initial ****/
//negative number of seconds to set start time to
//when the page is first visited
$conf['data']['defaultOffset'] = -3600 * 3;  //3 hours ago


/**** THIN OUT ********/


//Thinning out data (dropping every Nth record from display)
//This is basically an attempt to keep the graph drawing engine
//From being overloaded with data (it will take forever)

//There are two supported ways of skipping data
//SQL and CODE
//Code will read all data for the requested timeframe, and then
//decide what to skip. This results in more datatransfer from DB,
//but for MySQL that appears not to be significant. 
//CODE results in much smoother distribution of points (read on)
//
//SQL complicates the WHERE part of the select statement by restricting
//time to being less than a calculated amount per hour,day,month, etc
//this is crude but fast, and results in a dense bunch of points at the 
//beginning of each calculated interval
$conf['data']['thinOutMethod'] = "SQL"; 


//First I need to know how often temperature is collected
//This is in minutes.  This should agree with the interval 
//in your cron schedule, if the itervals are uneven - 
//put in average number of minutes
$conf['data']['collectionInterval'] = 5; //minutes


//The algorithm will attempt to thin out the data to a point
//where the resulting datapoint count will be below this value
//Setting this to an reasonable value 
//will keep rendering time reasonable
//This value is the total points for all sensors being displayed
//Note that having collectionInterval set correctly 
//is imperative to correct guessing
$conf['data']['maxThinnedOutDataPoints'] = 1000; 


/************** HTML Properties *********************/
/****************************************************/
$conf['html'] = array(); //no movo this lino

//Background color for main page
//as well as graph margin
//Tip:  using simple colors (FF,DD, etc) usually
//yields exact color matches between graph margin
//and HTML background
$conf['html']['bgcolor'] = "#DDDDFF";

/***** FORM submission METHOD *****/
//set to GET to be able to bookmark the url
//with all the options,
//set to POST to have have a more aesthetically pleasing URL
$conf['html']['formMethod'] = "POST"; 

/************** ALARMS ******************************/
/****************************************************/

//Enable alarms support... this makes dtgraph
//check for active alarms every time the main page is hit
$conf['alarms']['display'] = true;

//Notify whenever an alarm is raised
$conf['alarms']['notify'] = true;
$conf['alarms']['notifyEmail'] = 'akom';


?>
