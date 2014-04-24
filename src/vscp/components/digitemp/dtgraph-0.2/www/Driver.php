<?php

//Since this is invoked from sub-directories 
//on occasion, this is a safety to make sure it knows 
//where to find the file
require_once(dirname(__FILE__) . '/utils.php');

/**
 * @version $Revision: 1.1 $
 * @since   Kronolith 0.1
 * @package kronolith
 */
class DTtemp_Driver {

    /**
     * A hash containing any parameters for the current driver.
     *
     * @var array $_params
     */
    var $_params = array();


    /**
     * Constructor - just store the $params in our newly-created
     * object. All other work is done by open().
     *
     * @param optional array $params  Any parameters needed for this driver.
     */
    function DTtemp_Driver($params = array())
    {
        $this->_params = $params;
    }

    /**
     * Attempts to return a concrete DTtemp_Driver instance based
     * on $driver.
     *
     * @param $driver   The type of concrete Kronolith_Driver subclass to return.
     *                  This is based on the sensor driver ($driver). The
     *                  code is dynamically included.
     *
     * @param $params   (optional) A hash containing any additional
     *                  configuration or connection parameters a subclass
     *                  might need.
     *
     * @return          The newly created concrete Kronolith_Driver instance, or false
     *                  on error.
     */
    function &factory($driver, $params = array())
    {
        $basename = DTtemp_Driver::makeDriverFileName($driver);
        require_once($basename);
        $class = 'DTtemp_Driver_' . $driver;
        if (class_exists($class)) {
            return new $class($params);
        } else {
            Utils::fatal(new PEAR_Error(sprintf(_("Unable to load the definition of %s."), $class)), __FILE__, __LINE__);
            
        }
    }

    function makeDriverFileName($driver)
    {
        return 'Driver/' . strtolower(basename($driver)) . '.php';
        
    }



}

?>
