<HTML>
    <HEAD><TITLE>Admin DTGraph Metadata</TITLE></HEAD>
    <BODY>
        <!-- DRAW existing table -->
        <FORM method="POST">
        <?php
            $dtDir = '../'; //may have to adjust if you move admin directory contents
            require_once($dtDir.'conf.php');
            //require_once($dtDir.'utils.php');
            require_once($dtDir.'Driver.php');
            $driver = &DTtemp_Driver::factory($conf['driver'], $conf['sql']);
            $driver->connect();
            
            //$driver->updateMetadata('2322', 'testing', "junky, ignore", 1, 23, 34, 1, 78.3, 600, 'brown');

            //First, read submitted info to see if there is anything to do.
            //check if a delete is pressed:
            if (isset($delete)) {
                //echo "NEED to delete $delete";
                $result = $driver->deleteMetadata($delete);
                if ($result == true) {
                    echo "Success: Deleted $delete";
                    echo "&nbsp; <SMALL>Note: Sensor will always show if there are any readings for it</SMALL>";
                } else {
                    echo "Failed to delete: $result";
                }
            } else {
                //echo "Sorry, no delete";
            }

            if (isset($total) && isset($update)) {
                $counter = 0;
                    echo "Committing changes... ";
                while ($counter < $total) {
                    $thisser = "SerialNumber$counter";
                    //echo "Need to process: ".$$thisser;
                    echo "$counter ";
                    $thisname = "name$counter";
                    $thisdesc = "description$counter";
                    $thiscolor = "color$counter";
                    $thisalarm = "alarm$counter";
                    $thismin   = "min$counter";
                    $thismax   = "max$counter";
                    $thismaxchange_alarm   = "maxchange_alarm$counter";
                    $thismaxchange   = "maxchange$counter";
                    $thismaxchange_int   = "maxchange_interval$counter";
                    
                    $counter++;

                    $driver->updateMetadata(
                        $$thisser,
                        $$thisname,
                        $$thisdesc,
                        ($$thisalarm == '1') ? 1 : 0,
                        $$thismin,
                        $$thismax,
                        ($$thismaxchange_alarm == 1) ? 1 : 0,
                        $$thismaxchange,
                        $$thismaxchange_int,
                        $$thiscolor);
                }
                unset($counter);
            }
            
        ?>


        <TABLE border="1" cellpadding="0" cellspacing="0">
            <?
            $sensors = $driver->listSensors();

            $counter = 0;
            
            $colNames = array('SerialNumber', 'name','description','color','Range Alarm','Min','Max','Max Change Alarm','Max Change Amount','Max Change Inteval (seconds)','Delete Metadata');
            echo makeTableHeader($colNames);

            while(list($sensor, $data) = each($sensors)) {
                //echo "$sensor";
                echo "<TR>";
                echo "<TD bgcolor='#EEEEEE'>$sensor</TD>";
                echo makeHiddenField("SerialNumber$counter", $sensor);
                echo '<TD>'.makeInputField("name$counter", $data['name']).'</TD>';
                //echo '<TD>'.makeInputField("description$counter", $data['description'], 40).'</TD>';
                echo '<TD>'.makeTextArea("description$counter", $data['description']).'</TD>';
                echo '<TD bgcolor='.$data['color'].'>*'.makeInputField("color$counter", $data['color']).'</TD>';
                echo '<TD>'.makeCheckBox("alarm$counter", $data['alarm']).'</TD>';
                echo '<TD>'.makeInputField("min$counter", $data['min'],5).'</TD>';
                echo '<TD>'.makeInputField("max$counter", $data['max'],5).'</TD>';
                    
                echo '<TD>'.makeCheckBox("maxchange_alarm$counter", $data['maxchange_alarm']).'</TD>';
                echo '<TD>'.makeInputField("maxchange$counter", $data['maxchange'], 5).'</TD>';
                echo '<TD>'.makeInputField("maxchange_interval$counter", $data['maxchange_interval'], 6).'</TD>';
                echo '<TD>'.makeDelete($sensor).'</TD>';
                echo '</TR>';
                $counter++;
            }
            
        ?>
        </TABLE>
        <INPUT type="hidden" name="total" value="<?php echo $counter; ?>">
        <INPUT type="submit" name="update" value="Update (Write Changes)">
        <BR>
        Note: Delete button removes the metadata entry for the given sensor (everything you see above) but not its readings
        <BR>
        Note: What you see above are sensors that have readings or metadata. To add entries, get readings.  This eliminates serial number mismatch issues
        </FORM>

    </BODY>
</HTML>

<?php


    function makeTableHeader($colnames = array()) {
        $result = "<TR bgcolor='#DDDDDD'>";
        while(list($index, $name) = each($colnames)) {
            $result .= "<TH>$name</TH>\n";
        }
        $result .= "</TR>";
        return $result;
    }

    function makeInputField($name, $value, $size="15") {
        return "<INPUT type='text' name='$name' value='$value' size='$size'>\n";
    }

    function makeTextArea($name, $value, $width = 25, $height = 2) {
        return "<TEXTAREA name='$name' rows='$height' cols='$width'>$value</TEXTAREA>\n";
    }

    function makeCheckBox($name, $state = 0) {
        return "<INPUT type='checkbox' name='$name' value='1'".(($state == 1) ? ' CHECKED ' : '').">\n";
    }

    function makeHiddenField($name, $value) {
        return "<INPUT type='hidden' name='$name' value='$value'>\n";
    }

    function makeDelete($serial) {
        return "<A href='admin.php?delete=$serial'>Delete</A>";
    }
?>
        
