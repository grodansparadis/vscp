<?php
/*
 * This file is part of an open-source project
 * licensed under the GPL
 *
 * Structure loosely based on horde's components
 *  (www.horde.org)
 */
    
    //This is invoked from subdirectories, 
    //so utils.php will be relative the subdir.
    //locating it 'absolutely' instead
require_once(dirname(__FILE__) . '/../' . 'utils.php');
require_once("DB.php");

/**
 * The Kronolith_Driver_sql:: class implements the Kronolith_Driver
 * API for a SQL backend.
 *
 * $Utils: kronolith/lib/Driver/sql.php,v 1.76 2003/01/24 05:10:41 chuck Exp $
 *
 * @author  Luc Saillard <luc.saillard@fr.alcove.com>
 * @author  Chuck Hagenbuch <chuck@horde.org>
 * @version $Revision: 1.1 $
 * @since   Kronolith 0.3
 * @package kronolith
 */
class DTtemp_Driver_sql extends DTtemp_Driver {

    /**
     * The object handle for the current database connection.
     * @var object DB $_db
     */
    var $_db;

    /**
     * Boolean indicating whether or not we're currently connected to
     * the SQL server.
     * @var boolean $_connected
     */
    var $_connected = false;

    function connect()
    {
        $this->_connect();
    }


    /**
     *  dates will be in native format
     *  yyyyMMddhhmmss
     *  
     *  Results are returned as 2 dim array
     *  with key as key and columns as the inner ass. array
     *  Inner array will contain time as mysql timestamp
     *  and unixtime keys
     *  
     *  If sensor is not supplied, all are fetched
     *
     *  @param startDate - if set, will be used as beginning of query, likewise for end
     *  @param fetchUnchanged   if false, of any number of consecutive readings with 
     *         the same value only the outermost two will be returned
     *  @param thinOut - if true, data will be thinned out by getting only every Nth result
     *  @param units - default will be used from configuration, but request may override it by setting this to Celcius
     *
     */
    function listEvents($sensors = null, $startDate = null, $endDate = null, $fetchUnchanged = false, $thinOut = false, $units = null)
    {
        $result = array();
        $t = time();
        global $stop_watch;
        
        global $conf;
        if (!isset($units)) {
            $units = $conf['data']['units']; //use default
        }

        $started = false;
        //echo "Got list of sensors:";
        //print_r($sensors);

        //determine the thinout factor if needed

        $q = 'SELECT  *, unix_timestamp(time) unixtime';
        $q .= ($units == 'Celcius') ? ', ((Fahrenheit -32) *  5 / 9 ) Celcius' : '';
        $q .= ' FROM ' . $this->_params['table'];
        $q .= $this->makeWhereClause($sensors, $startDate, $endDate, $thinOut);
        $q .= ' order by time ASC';
        $stop_watch['Made Query'] = microtime();

        //echo "Running query: $q";
        /* Run the query. */
        $qr = $this->_db->query($q);
        $stop_watch['Ran Query'] = microtime();
        //echo "Got results: ".$qr->numRows();
        
        $backup = array();
        $lastReadings = array();
        if (!DB::isError($qr)) {
            if ($conf['data']['thinOutMethod'] == 'CODE') {
                //setup thin out factor
                $excess =  $qr->numRows() - $conf['data']['maxThinnedOutDataPoints'];
                echo "Excess is $excess based on ".$qr->numRows()." and ".$conf['data']['maxThinnedOutDataPoints'];

                if ($excess > 0) {
                    //we should drop every so many readings
                    $thinOutEvery = $qr->numRows() / $excess;
                }
            }
            $row = $qr->fetchRow(DB_FETCHMODE_ASSOC);
            $backup[$row['SerialNumber']] = $row;
            
            if ($fetchUnchanged == false) {
                //Always tack on first reading (if in unch mode)
                $result[$row['dtKey']] = $row;
            }
            
            $thinOutCounter = 0;
            //echo "thinoutevery=$thinOutEvery ... ";
            while ($row && !DB::isError($row)) {
                if (isset($thinOutEvery)) {
                    if ($thinOutCounter >= $thinOutEvery) {
                        $thinOutCounter = 0;
                        echo "thinning out ".$row['dtKey'];
                        $row = $qr->fetchRow(DB_FETCHMODE_ASSOC);
                        continue;
                    }
                    $thinOutCounter++;
                }
                    
                if ($fetchUnchanged == false) {
                    //record only if backup is not the same as this
                    //Note this causes problems if last two items are the same -
                    //the last reading will be missing, screwing up the graph
                    $currentTemp = $row['Fahrenheit'];
                    $backupTemp = $backup[$row['SerialNumber']]['Fahrenheit'];
                    //echo "CurrentTemp = $currentTemp, backupTemp = $backupTemp";
                    if ($currentTemp != $backupTemp) {
                       // echo "Adding ";
                       //add backup too so it's accurate!
                       $result[$backup[$row['SerialNumber']]['dtKey']] = $backup[$row['SerialNumber']];
                        $result[$row['dtKey']] = $row;
                    }
                    ///echo "<br>";
                    //backup last readings so that we have the last data point visible
                    $lastReadings[$row['SerialNumber']] = $row;

                } else {
                    $result[$row['dtKey']] = $row;
                }
                
                $backup[$row['SerialNumber']] = $row;
                $row = $qr->fetchRow(DB_FETCHMODE_ASSOC);
            }
            if ($fetchUnchanged == false) {
                //now tack on the last reading backups
                while (list($key, $val) = each($lastReadings)) {
                    $result[$val['dtKey']] = $val;
                }
            }
        }
        $stop_watch['Post Processed Query'] = microtime();
        return $result;
    }


    /**
     * Delete all sensor's events.
     *
     * @param string $sensor The name of the sensor to delete.
     *
     * @return mixed  True or a PEAR_Error on failure.
     */
    function deleteMetadata($sensor)
    {
        $this->_connect();

        $query = sprintf('DELETE FROM %s WHERE SerialNumber = %s',
                    $this->_params['table_meta'],
                    $this->_db->quote($sensor));

        /* Log the query at a DEBUG log level. */

        $res = $this->_db->query($query);
        if (DB::isError($res)) {
            return $res;
        }

        return true;
    }


    /**
     * Delete all sensor's events.
     *
     * @param string $sensor The name of the sensor to delete.
     *
     * @return mixed  True or a PEAR_Error on failure.
     */
    function delete($sensor)
    {
        $this->_connect();

        $query = sprintf('DELETE FROM %s WHERE SerialNumber = %s',
                    $this->_params['table'],
                    $this->_db->quote($sensor));

        /* Log the query at a DEBUG log level. */

        $res = $this->_db->query($query);
        if (DB::isError($res)) {
            return $res;
        }

        return true;
    }

    /**
     * Event id is the dtKey
     */
    function deleteEvent($eventID)
    {
        $eventID = (int)$eventID;
        $query = sprintf('DELETE FROM %s WHERE dtKey = %s ',
                         $this->_params['table'],
                         $this->_db->quote($eventID));

        /* Log the query at a DEBUG log level. */

        if (DB::isError($res = $this->_db->query($query))) {
            return false;
        }

        return true;
    }

    /**
     *  Returns the number of events
     *  if list of sensors supplied, 
     *  then just for them (all of them)
     */
    function countEvents($sensors = null, $startDate = null, $endDate = null) {
        $result = ""; //need a default of something

        $q = 'SELECT COUNT(*) from '.$this->_params['table'];
        $q .= $this->makeWhereClause($sensors, $startDate, $endDate);

        //echo "Running count query: $q";

        /* Run the query. */
        $qr = $this->_db->query($q);

        if (!DB::isError($qr)) {
            $row = $qr->fetchRow();
            return array_pop($row);
        }

    }

    /**
     *  Returns the unix timestamp representing
     *  start of data
     */
    function getStartOfData($sensors = null) {
        $result = ""; //need a default of something

        $q = 'SELECT MIN(unix_timestamp(time)) from '.$this->_params['table'];
        if (isset($sensors)) {
            $q .= ' WHERE SerialNumber IN ('. $this->makeInList($sensors).')';
        }

        /* Run the query. */
        $qr = $this->_db->query($q);


        if (!DB::isError($qr)) {
            $row = $qr->fetchRow();
            return array_pop($row);
        }
    }

    /**
     * Returns an info array about each sensor:
     * (actual readings)
     * [sensor][min, max, avg, Current]
     *
     * @access public
     */
    function getStats($sensors = null, $startTime = null, $endTime = null, $units = null) {
        global $conf;
        if (!isset($units)) {
            $units = $conf['data']['units']; //use default
        }
        $result = array();

        if (isset($sensors) && is_array($sensors)) {
            //we're good
        } else { //need to get list of sensors

            $sensors= $this->listDistinctSensors();
        }

        //NOw we have a list of sensors to work with
        reset($sensors);
        while(list($num, $sensor) = each($sensors)) {
            unset($sArray);
            $sArray = array($sensor);
            $q = 'select min(Fahrenheit) min, max(Fahrenheit) max, avg(Fahrenheit) avg';
            $q .=',max(time) time FROM ' . $this->_params['table'] ;
            $q .= $this->makeWhereClause($sArray, $startTime, $endTime);
            //echo "Now should run $q";
            /* Run the query. */
            $qr = $this->_db->query($q);

            if (!DB::isError($qr)) {
                $row = $qr->fetchRow(DB_FETCHMODE_ASSOC);
                //echo "Got row";
                //print_r($row);
                if($units == 'Celcius') {
                    $row['min'] = Utils::toCelcius($row['min']);
                    $row['max'] = Utils::toCelcius($row['max']);
                    $row['avg'] = Utils::toCelcius($row['avg']);
                }
                $result[$sensor] = $row;
                
            }
            $subQ = 'SELECT ';
            $subQ .= ($units == 'Celcius') ? ' ((Fahrenheit -32) *  5 / 9 ) Celcius' : 'Fahrenheit';
            $subQ .= ' from '. $this->_params['table'] .
                " WHERE SerialNumber = '$sensor' and time = ". $row['time'];
            //now run the current reading query
            $qr = $this->_db->query($subQ);

            if (!DB::isError($qr)) {
                $row = $qr->fetchRow();
                //echo "Got row";
                //print_r($row);
                $result[$sensor]['Current'] = $row[0];
            }

        }
        return $result;
    }

    /**
     * This runs the distinct query
     * which gets a list of sensors that have 
     * actual readings
     */
    function listDistinctSensors() {
        $result = array();
        $q = 'SELECT distinct SerialNumber FROM '. $this->_params['table'];
        $qr = $this->_db->query($q);


        if (!DB::isError($qr)) {
            $row = $qr->fetchRow();
            //echo "Got row";
            //print_r($row);
            while ($row && !DB::isError($row)) {
            //print_r($row);
                array_push($result,$row[0]);
                $row = $qr->fetchRow();
            }
        }
        //echo "list Distinct returning: ";
        //print_r($result);
        return $result;
    }

    /**
     * Normally gets info about all sensors
     *  If no list supplied, gets all sensors whether or not they are described
     *
     * if sensor SerialNumber array is supplied, just gets 
     * the requested ones
     *
     *  Returned structure is
     *  array[serialNumber][dataAboutit]
     *  dataAboutIt includes name, description, allowed min, max, alarms, etc
     *  (all columns of the metadata table)
     */
    function listSensors($sensors = null, $units = null) {
        $result = array();
        if (!isset($units)) {
            $units = $conf['data']['units'];
        }
//        $q = 'SELECT DISTINCT b.* FROM ' . $this->_params['table']. " a , ". $this->_params['table_meta']. " b where a.SerialNumber = b.SerialNumber ";
        $q = 'SELECT * FROM ' . $this->_params['table_meta'] . ' b ';

        if (isset($sensors) && is_array($sensors)) {
            $q .= ' WHERE b.SerialNumber in (';
            $q .= $this->makeInList($sensors);
            $q .= ')';
        }

        $q .= ' order by b.name asc';
        //echo "Running query : $q";
            
        /* Run the query. */
        $qr = $this->_db->query($q);


        if (!DB::isError($qr)) {
            $row = $qr->fetchRow(DB_FETCHMODE_ASSOC);
            //echo "Got row";
            while ($row && !DB::isError($row)) {
            //print_r($row);
                if ($units == 'Celcius') {
                    $row['max'] = Utils::toCelcius($row['max']);
                    $row['min'] = Utils::toCelcius($row['min']);
                }
                $result[$row['SerialNumber']]= $row;

                $row = $qr->fetchRow(DB_FETCHMODE_ASSOC);
            }
        }

        unset($qr); //just to clear things up

        if ($sensors == null) {
            //now, if sensor list is null, get the real list of readings from the table..
            //Cause until now the list is based on the metadata contents
            //This will add fake defaults to the resulting array for any non-described sensors
            //This way a new sensor will be displayed before it is described
            //and the array sort will remain unchanged

            $distinctList = $this->listDistinctSensors();
            while(list($num, $ser) = each($distinctList)) {
                if (!isset($result[$ser])) {
                    //echo "Adding fake entry for ".$ser;
                    $result[$ser] =
                        $this->makeFakeArray($ser);
                }
            }
        } else if (isset($sensors) && is_array($sensors)) {
            //now check for sensors that were not describecd
            //but were passed in on the list
            reset($sensors);
            while (list($num, $ser) = each($sensors)) {
                if (!isset($result[$ser])) {
                    //put a fake in there
                    //echo "calling make fake array with $ser";
                    $result[$ser] = $this->makeFakeArray($ser);
                }
            }
        }

        //echo "Done<P>Returning";
        //print_r($result);
        return $result;

    }
    
    /**
     * Automatic update/insert method
     */
    function updateMetadata(
        $serial, 
        $name, 
        $description = null, 
        $alarm = null, 
        $min = null, 
        $max = null, 
        $maxchangeAlarm = null, 
        $maxchangeTemp = null, 
        $maxchangeTime = null,
        $color = null) {

        $q = 'SELECT SerialNumber from '.$this->_params['table_meta'];
        $q .= " WHERE SerialNumber = '$serial'";

       // echo "Running UPDATE exists check query: $q";

        /* Run the query. */
        $qr = $this->_db->query($q);

        if (!DB::isError($qr)) {
            $row = $qr->fetchRow();
            //$ser =  array_pop($row);
            if (isset($row) && is_array($row)) {
                //exists
                //echo "EXISTS: ".array_pop($row);
            } else {
                //echo "DOES NOT EXIST";
                //does not exist
                //insert bare minimum, then have it update either way
                $qi = 'INSERT INTO '.$this->_params['table_meta'].' (SerialNumber, Name)';
                $qi .= " VALUES ('$serial', 'BogusName')";
                $insertResult = $this->_db->query($qi);
                if (!DB::isError($insertResult)) {
             //       echo "INSERT SUccessfull";
                } else {
                    echo "ERROR inserting :".$insertResult;
                    return;
                }
                
            }
        } else { 
            echo "Error Checking!";
            return;
        }

        //now setup the update query using all available data
        $qu = 'UPDATE '.$this->_params['table_meta'].' SET ';
        $qu .= " name = '$name'";
        $qu .= (isset($description)) ? " , description = '$description' " : '';
        $qu .= (isset($alarm) && is_numeric($alarm)) ? " , alarm = $alarm " : '';
        $qu .= (isset($max) && is_numeric($max)) ? " , max = $max " : '';
        $qu .= (isset($min) && is_numeric($min)) ? " , min = $min " : '';
        $qu .= (isset($maxchangeAlarm) && is_numeric($maxchangeAlarm)) ? " , maxchange_alarm = $maxchangeAlarm " : '';
        $qu .= (isset($maxchangeTemp) && is_numeric($maxchangeTemp)) ? " , maxchange= $maxchangeTemp " : '';
        $qu .= (isset($maxchangeTime) && is_numeric($maxchangeTime)) ? " , maxchange_interval = $maxchangeTime " : '';
        $qu .= (isset($color)) ? " , color = '$color' " : '';
    
        $qu .= " WHERE SerialNumber = '$serial'";
        
        //echo "Running update query: $qu";
        
        $updateResult = $this->_db->query($qu);
        if (!DB::isError($updateResult)) {
         //   echo " UPdate Successfull";
        } else {
            echo " <P>Update failed!<P>: $updateResult";
        }
    }

    /**
     * In the absence of metadata for a sensor,
     * Constructs a fake representation of it 
     * based on the serial number
     * Sets color to black
     */
    function makeFakeArray($serial) {
        $result = array();
        $result['SerialNumber'] = $serial;
        $result['name'] = $serial;
        $result['description'] = 'This sensor has not yet been described';
        $result['color'] = 'black'; //default
        return $result;
    }


    /**
     * Intended to analyze the current data
     * and put in/update records in the alarms table
     * if warranted.
     * Since a number of queries run for this, 
     * this should not be invoked too often
     * 
     * @return an array of newly raised alarms (strings)
     */
    function updateAlarms() 
    {
        $newAlarms = array();

        $sql = 'SELECT max(Fahrenheit) realmax, min(Fahrenheit) realmin from ';
        $sql .= $this->_params['table'].' a, ';
        $sql .= $this->_params['table_meta'].' b ' ;
        $sql .= ' where a.SerialNumber = ? and a.SerialNumber = b.SerialNumber and time > FROM_UNIXTIME(UNIX_TIMESTAMP() - ?)'; 

        $ps = $this->_db->prepare($sql);
        
        // Step 1. Load metadata for sensors
        // note: using default units.  
        //To support celcius, need to rethink
        $metadata = $this->listSensors();

        $info = $this->getStats();
        
        // Loop over the metadata because we only care about 
        //sensors that have it set
        while (list($serial, $s) = each($metadata)) {
            $current = $info[$serial]['Current'];

            $params = array($serial);
            $interval = $s['maxchange_interval'];
            if (isset($interval) && $interval > 0) {
                //then I'll just use that for min/max query
                //save me some time
                array_push($params, $interval);
            } else {
                array_push($params, 3600); //default amount of time
                //this isn't a problem cause then maxchange_alarm
                //shouldn't be set
            }

            //run the alarms query
            $rs = $this->_db->execute($ps, $params);

            $row = $rs->fetchRow(DB_FETCHMODE_ASSOC);
            $realmin = $row['realmin'];
            $realmax = $row['realmax'];

            //now I have everything, check if there are problems.

            echo "\n<BR>Checking sensor $serial";

            if (isset($s['alarm']) && $s['alarm'] == 1) {
                echo " maxmin :";
                //min and max absolute alarm
                
                if ($current < $s['min']) {
                    echo "yes, min";
                    //problem
                    //raise min alarm
                    $text = "Reading too low - $current";
                    if($this->raiseAlarm($serial, $current, 'minmax', $text)) {
                        array_push($newAlarms, array('sensor' => $metadata[$serial]['name'] , 'text' => $text));
                    }
                } else if ($current > $s['max']) {
                    echo "yes, max";
                    //raise max alarm
                    $text = "Reading too high - $current";
                    if ($this->raiseAlarm($serial, $current, 'minmax', $text)) {
                        array_push($newAlarms, array('sensor' => $metadata[$serial]['name'] , 'text' => $text));
                    }
                } else {
                    echo "no";
                    //unset
                    $this->clearAlarm($serial, 'minmax');
                }
            } else {
                //if someone shut off the checkbox, existing alarms
                //will remain active .... so I have to shut them off just in case
                $this->clearAlarm($serial, 'minmax');
                //unfortunately that means extra DB access...
                // but this is only called from cron anyway
            }

            if (isset($s['maxchange_alarm']) && $s['maxchange_alarm'] == 1) {
                //maxchange stuff 
                echo " Maxchange: ";
                if (($realmax - $realmin) > $s['maxchange']) {
                    echo " yes";
                    //raise maxchange alarm
                    $text = "Changing too fast: went from $realmin to $realmax in ".$s['maxchange_interval'];
                    if($this->raiseAlarm($serial, $current, 'maxchange', $text)) {
                        array_push($newAlarms, array('sensor' => $metadata[$serial]['name'] , 'text' => $text));
                    }
                } else {
                    echo " no ($realmax - $realmin) is not > ".$s['maxchange'];
                    $this->clearAlarm($serial, 'maxchange');
                }
            } else {
                //if someone shut off the checkbox, existing alarms
                //will remain active .... so I have to shut them off just in case
                $this->clearAlarm($serial, 'maxchange');
            }

        }
        return $newAlarms;
    }

    /**
     * Reads an array of alarm info
     * (3 dimensional array)
     * $array[serial][alarm_id][data] (data is Fahrenheit, type, description, time_*)
     */
    function getActiveAlarms() {
       $sql  = 'SELECT * from ';
       $sql .= $this->_params['table_alarms'];
       $sql .= ' where time_cleared is null ';

       $result = array();

       $rs = $this->_db->query($sql);
       if (!DB::isError($rs)) {
           $row = $rs->fetchRow(DB_FETCHMODE_ASSOC);
           //echo "Got row";
           while ($row && !DB::isError($row)) {
               $serial = $row['SerialNumber'];
               $alarm_id = $row['alarm_id'];

               //echo "setting row";
               //print_r($row);
               if (!isset($result[$serial])) {
                   $result[$serial] = array();
               }
               $result[$serial][$alarm_id] = $row;
               $row = $rs->fetchRow(DB_FETCHMODE_ASSOC);
           }
       }
       return $result;
    }

    /**
     * Raises alarms (puts entry in table)
     *
     * @returns true if this was the first time it was raised
     */
    function raiseAlarm($serial, $temp, $type, $description) {
        $wasNew = false;

        //check that it's already set
        $sqlRead = 'SELECT alarm_id from ';
        $sqlRead .= $this->_params['table_alarms'];
        $sqlRead .= ' where SerialNumber = ? and alarm_type = ? ';
        $sqlRead .= ' and time_cleared is null ';

        $ps = $this->_db->prepare($sqlRead);
        $rs = $this->_db->execute($ps, array($serial, $type));

        if (!DB::isError($rs)) {
            if ($rs->numRows() > 0) {
                echo "updating alarm $type";
                //already in table, update
                $row = $rs->fetchRow(DB_FETCHMODE_ORDERED);
                $id = $row[0];
                

                $sqlUpdate = 'UPDATE ';
                $sqlUpdate .=  $this->_params['table_alarms'];  
                $sqlUpdate .= ' set Fahrenheit = ? ' ;
                //$sqlUpdate .= ' and time_updated = null ';
                $sqlUpdate .= ' , description = ? ';
                $sqlUpdate .= ' where SerialNumber = ? and alarm_type = ? ';
                echo "<BR>Running $sqlUpdate (temp = $temp)";
                $ps = $this->_db->prepare($sqlUpdate);
                $this->_db->execute($ps, array( $temp, $description, $serial,  $type ));


            } else {
                echo "inserting alarm $type";
                //not yet in table, insert
                $sqlInsert = ' INSERT into ';
                $sqlInsert .=  $this->_params['table_alarms'];
                $sqlInsert .= ' (SerialNumber, Fahrenheit, alarm_type, time_raised, description) ';
                $sqlInsert .= ' VALUES (?, ?, ?, !, ?)';

                $ps = $this->_db->prepare($sqlInsert);
                $this->_db->execute($ps, array($serial, $temp, $type, 'now()' , $description));

                //hope all was well
                $wasNew = true;

            }
        }
        return $wasNew;

    }

    /**
     * clears all alarms with the given type
     * for given serial
     */
    function clearAlarm($serial, $type) {

        $sqlUpdate = 'UPDATE ';
        $sqlUpdate .=  $this->_params['table_alarms'];  
        $sqlUpdate .= ' set time_cleared = ! ' ;
        $sqlUpdate .= ' where SerialNumber = ? and alarm_type = ? and time_cleared is NULL';

        $ps = $this->_db->prepare($sqlUpdate);
        $this->_db->execute($ps, array('now()', $serial, $type));

    }
    


    /**
     * Attempts to open a persistent connection to the SQL server.
     *
     * @return boolean True.
     */
    function _connect()
    {
        if (!$this->_connected) {
            require_once 'DB.php';

            if (!is_array($this->_params)) {
                Utils::fatal(PEAR::raiseError(_("No configuration information specified for SQL Calendar.")), __FILE__, __LINE__);
            }
            if (!isset($this->_params['phptype'])) {
                Utils::fatal(PEAR::raiseError(_("Required 'phptype' not specified in calendar configuration.")), __FILE__, __LINE__);
            }
            if (!isset($this->_params['hostspec'])) {
                Utils::fatal(PEAR::raiseError(_("Required 'hostspec' not specified in calendar configuration.")), __FILE__, __LINE__);
            }
            if (!isset($this->_params['username'])) {
                Utils::fatal(PEAR::raiseError(_("Required 'username' not specified in calendar configuration.")), __FILE__, __LINE__);
            }
            if (!isset($this->_params['password'])) {
                Utils::fatal(PEAR::raiseError(_("Required 'password' not specified in calendar configuration.")), __FILE__, __LINE__);
            }

            /* Connect to the SQL server using the supplied parameters. */
            //$this->_db = &DB::connect($this->_params, true);
            $dsn = $this->_params['phptype']."://". 
                $this->_params['username'] . ":" .
                $this->_params['password'] . "@" .
                $this->_params['hostspec'] . "/" .
                $this->_params['database'];
            //echo "Connecting to $dsn";
            $this->_db = &DB::connect($dsn, true);
            if (is_a($this->_db, 'PEAR_Error')) {
                Utils::fatal($this->_db, __FILE__, __LINE__);
            }

            /* Enable the "portability" option. */
            //$this->_db->setOption('optimize', 'portability');

            $this->_connected = true;

        }

        return true;
    }

    function close()
    {
        return true;
    }

    /**
     * Disconnect from the SQL server and clean up the connection.
     *
     * @return boolean true on success, false on failure.
     */
    function _disconnect()
    {
        if ($this->_connected) {
            $this->_connected = false;
            return $this->_db->disconnect();
        }

        return true;
    }



    /**
     * Private helper to make a string representation
     * of array values separated by ,'s
     *  and surrounded by parenthesis (for IN sql clause)
     * @access private
     */
    function makeInList($list) {
        if (is_array($list)) {
            return "'".join("','",$list)."'";
        }
        return "";
    }

    /**
     * Makes a generic WHERE and everything after part
     * of an sql statement
     *
     * Adds sections based on which args are not null,
     * such as "sensor IN ('s1','s2')"
     * and "time < startTime"
     * @access private
     */
    function makeWhereClause($sensors = null, $startDate = null, $endDate = null, $thinOut = false)
    {
        global $conf;
        $qu = "";
        
        if (isset($sensors) || isset($startDate) || isset($endDate)) {
            $qu .= ' WHERE ';
        }
        
        if (isset($sensors) && is_array($sensors)) {
            $qu .= ' SerialNumber in (';
            $qu .= $this->makeInList($sensors);
            $qu .= ')';
            $started = true;
        }
            
        if (isset($startDate)) {
            if ($started == true) {
                $qu .= ' AND ';
            }
            $qu .= ' time > FROM_UNIXTIME(' . $this->_db->quote($startDate) .')';
            $started = true;
        }

        
        if (isset($endDate)) {
            if ($started == true) {
                $qu .= ' AND ';
            }
            $qu .= ' time < FROM_UNIXTIME(' . $this->_db->quote($endDate). ')';
        }

        if (isset($thinOut) && $thinOut && $conf['data']['thinOutMethod'] == 'SQL') {
            //echo "Thinning out SQL";
            if (isset($sensors) && is_array($sensors)) {
                $sensorCount = count($sensors);
            } else {
                $sensorCount = count($this->listDistinctSensors());
            }

            $duration = $this->getDuration($startDate, $endDate);
            $thinOut =  $this->makeThinOutQueryComponent($duration, $sensorCount);
            
            if (isset($thinOut)) {
                $qu .= ' AND '.$thinOut;
            }
           //echo "Made $qu";
        }
        return $qu;
    }


    /**
     * Basically subtracts the two but 
     * accounts for unset fields 
     */
    function getDuration($startTime, $endTime) {
        if (!isset($startTime)) {
            $startTime = $this->getStartOfData();
        }
        if (!isset($endTime)) {
            $endTime = time();
        }
        return $endTime - $startTime;
    }
    
    /**
     * compares the anticipated number of datapoints
     * to the max from configuration,
     * and returns a factor. If it's >1, thinning out
     * by that much is needed
     * This is useful for the SQL thinOut method
     */
    function calculateThinOutFactor($duration,$numSensors) {
        global $conf;
            
        //base on collectionInterval and maxThinnedOutDataPoints
        //try to make this more exact
        
        $interval =  $conf['data']['collectionInterval'];
        if (!isset($interval)) {
        
            Utils::fatal(PEAR::raiseError(_("Please set the collectionInterval in configuration")), __FILE__, __LINE__);
        }
        //estimating raw point count
        $rawPoints = $numSensors * ($duration / 60) / $interval ;
        $maxPoints = $conf['data']['maxThinnedOutDataPoints'];


        //now break it up into big categories of hours, days, etc and then 
        //ponder making it relatively precise within those bounds 

        $thinOutFactor = $rawPoints / $maxPoints;

        return $thinOutFactor;

    }

    /**
     * This function sets up the thin out part of the 
     * where clause (without the leading AND)
     * based on the configuration parameters 
     * and the duration which should be the seconds 
     * in the requested interval
     * 
     * if none needed, returns null
     */
    function makeThinOutQueryComponent($duration, $numSensors) {
        global $conf;
        $result = null;
        if ($numSensors == 0) {
            return $result;
        }

        $thinOutFactor = $this->calculateThinOutFactor($duration,$numSensors);
            
        if ($thinOutFactor < 1) {
            return $result; //no thinning out needed
        }
        
        $interval =  $conf['data']['collectionInterval'];


        //now break it up into big categories of hours, days, etc and then 
        //ponder making it relatively precise within those bounds 


        //first, restrict minutes 
        $needAnd = false;
        $readingsPerHour = 60 / $interval;

        $alreadyThinned = 1; //this is a factor
        //echo "Numsensors = $numSensors";
        if ($readingsPerHour > 1) {
            //then we thin out readings per hour, possibly to as little as one per
            

            $desiredReadingsPerHour = $readingsPerHour / $thinOutFactor;
            if ($desiredReadingsPerHour < 1) {
                $desiredReadingsPerHour = 1;
            }
            $result .= ' extract(minute from time) < '. (($interval * $desiredReadingsPerHour) ); 
            //so I will only get the first part of an hour unfortunately
            //instead of an even distribution
            $needAnd = true;
            $alreadyThinned = $readingsPerHour / $desiredReadingsPerHour;
        }

        if ($readingsPerHour < $thinOutFactor) { 
            $factor = ($interval > 60) ? (60 / $interval) : 1; 
            //then we need to thin out more, hours I guess
            $readingsPerDay = 24 / $factor; //at this point it's once per hour or less!

            //but we already thinned something out...
            $desiredReadingsPerDay = $readingsPerDay / $thinOutFactor * $alreadyThinned;
            if ($desiredReadingsPerDay < 1) {
                $desiredReadingsPerDay = 1;
            }
            //echo "Doing days based on: $alreadyThinned / $readingsPerDay / $thinOutFactor = $desiredReadingsPerDay , that's for $numSensors sensors ";
            $result .= $needAnd ? ' AND ' : '';
            //correction in case interval is over one hour
            $result .= ' extract(hour from time) < '. ($desiredReadingsPerDay * $factor) ;
        }

        return $result;
        
    }

}
?>
