# Lua Scripts and callbacks

**This documentation about Lua scripts is work in progress**

Lua is a powerful scripting language that (as an alternative to the JavaScript engine) is built into the VSCP server.

To learn Lua you have a crash course [here](http://luatut.com/crash_course.html).

A Lua callbacks is functions that you can call from Lua code in a decision matrix action parameter or in the other places where VSCP Lua can be executed.

## Predefined Lua variables

The VSCP server creates some predefined variables when a Lua is executed. 


*  **vscp.feedevent_str** - This is the event that triggered the execution of the JavaScript as a string. __The variable is read only.__

*  **vscp.feedevent_xml** - This is the event that triggered the execution of the XML as a string. __The variable is read only.__

*  **vscp.feedevent_json** - This is the event that triggered the execution of the JSON as a string. __The variable is read only.__

*  **vscp-version** - VSCP server version.

*  **vscp.lua_type** - Will hold "dmscript" when executed here.

---

## vscp.print

Can be used for debugging when the server is started in non-debug mode. Output will be outputted in the console window. Nothing will be output if the VSCP server is run as a daemon.

### Parameters

```lua
vscp.print("This is a very extensive test!");
```

---

## vscp.log

Log data to the VSCP daemon logs. Very well suited for reporting errors and log information. As an alternative one can send VSCP events instead. [CLASS1.LOG](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.log) and [CLASS1.ERROR](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.error) is intended for this use.

### Parameters

```lua
vscp.log("message"[,nDebugLevel, nType] );
```


*  **message** is required and is the message that is sent to the logs. Please end this message with a new line.

*  **nDebugLevel** is optional and should be 0,1 or 2 where 0 is the lowest debug level, 1 is the medium debug level and 2 is the high debug level. Can be omitted and if so defaults to 1.

*  **nType** is optional and should be 0,1,2,3 and 0 is general log messages, 1 is security log messages, 2 is access log messages and 3 is decision matrix related log messages. Can be omitted and if so defaults to 0.

### Return value

This function will always return a boolean true.

### Example

This script write out the current date on ISO format (YYMMDD) and "Hello World!" to the general log file. The **%isodate** is a [decision matrix escape](./decision_matrix.md#variable_substitution_for_parameters_escapes) and it is replaces with information before the script is executed.

```lua
vscp.log("%isodate  Hello World! \n" );
```

---

## vscp.sleep

Sleep a number of milliseconds.

### Parameters

```lua
vscp.sleep( sleeptime );
```


*  **sleeptime** Time to sleep in milliseconds.

### Return value

This function will always return a boolean true.

### Example

```lua
vscp.log('Before sleep 1')
vscp.sleep(5000)
vscp.log('After sleep 1')
vscp.sleep(5000)
vscp.log('After sleep 2')
vscp.log('THE END')
```

---

## vscp.base64encode

BASE64 encode a string

### Parameters

```lua
encoded_str =  vscp.base64encode("This is a string")
```

### Returns

Returns a BASE64 encoded string

---

## vscp.base64decode

BASE64 decode a string

### Parameters

```lua
decoded_str = vscp.base64decode("VGhpcyBpcyBhIHN0cmluZw==")
```

### Returns

Returns a BASE64 decoded string


---


##  vscp.readvariable

Fetch a VSCP remote variable by name. Variable information is returned in string (default), XML, JSON format or as only the value or as the note. An error is returned if no variable with that name exists. An error is returned if no variable with that name exists. You must be the owner of a variable or have read rights to read it.

String type values and notes are returned BASE65 encoded. You can use **vscp.base64decode** to decode them.

### Parameters

```lua
vscp.readvariable( "name"[,format]);
```


*  **name** of variable.

*  optional **format** the variable should be returned on. 0=string format (default), 1=xml format, 2=JSON format.

### Return

Variable information is returned in string (default), XML, JSON format or as only the value or as the note. An error is returned if no variable with that name exists.

#### String

```javascript
name;type;persistence;user;access-rights;last-change;value;note
```

The variable types are [listed here](./remote_variables.md#variable_types).

The variable value will be BASE64 encoded for certain variable types. The note will always be BASE64 encoded.

#### XML

```xml
<variable
    name="variable-name"
    type="1"
    user="2"
    access-rights="1911"
    persistence="true|false"
    last-change="YYYYMMDDTHHMMSS"
    value="Variable value"
    note="Variable note" />
```

The variable types are [listed here](./remote_variables.md#variable_types).

The variable value will be BASE64 encoded for certain variable types. The note will always be BASE64 encoded.

#### JSON

```javascript
{  
    "name": "variable-name",
    "type": 1,
    "user": 2,
    "accessrights": 1911,
    "persistence": true|false,
    "lastchange": "YYYYMMDDTHHMMSS",
    "bnumeric": true|false,
    "bbase64": true|false,
    "value": "Variable value"|Variable value,
    "note": "Variable note"
}
```

The variable types are [listed here](./remote_variables.md#variable_types).


*  **bnumeric** is true for a numerical variable. 

*  **bbase64** is true if the value is coded in BASE64, this is true for all string formats.

The variable value will be BASE64 encoded for certain variable types. The note will always be BASE64 encoded.

#### value

The value of the variable is returned. The value is always BASE64 encoded for a string type value.

#### Note

The variable note. The note is always BASE64 encoded.


 
### Example 1

```lua
local str 

-- Read variable in string format
vscp.log('Before read variable command - string (default)')
str = vscp.readvariable('vscp.version.str')
vscp.log( str )

-- Read variable in XML format
vscp.log('Before read variable command - XML')
str = vscp.readvariable('vscp.version.str',1)
vscp.log( str )

-- Read variable in JSON format
vscp.log('Before read variable command - JSON')
str = vscp.readvariable('vscp.version.str',2)
vscp.log( str )

-- Read variable in just value
vscp.log('Before read variable command - just value')
str = vscp.readvariable('vscp.version.str',3)
vscp.log( str )

-- Read variable in just note
vscp.log('Before read variable command - just note')
str = vscp.readvariable('vscp.version.str',4)
vscp.log( str )

vscp.log('THE END')
```



### Example 2

This example reads the variable **test1** which is a numerical VSCP remote variable in the default configuration, it increase the value of it with one and then write it back and finally write the value to the general log file.

```lua
local value
value = vscp.readVariable("test1",3);              // Read the remote variable 'test1'
value = value + 1;                                 // Increase its value 
vscp.writeVariablevalue("test1",value);            // Write the new value
value = vscp_readVariable("test1",3);              // Read back the remote variable value
vscp.log("Variable value = " + value + "\n" );     // Log 
```


----


##  vscp.writeVariable

Create or change a VSCP remote variable. The variable is created if it does not exist. You must be the owner of a variable or have write access to write it.

### Parameters

```lua
vscp.writeVariable(variable[,format]);
```


*  **variable:** The variable defined either as *string*, *XML* or *JSON* format.

*  **format:** If format is set to **0** the variable is in string format (default), if format is set to **1** the variable is in XML format and if format is set to **2** the variable is in JSON format.

The variable types are [listed here](./remote_variables.md#variable_types).

#### String defined variable

```javascript
name;type;persistence;user;access-rights;value;note
```

**Warning!** The semicolon (;) is a reserved character and can't be used in values or notes. The solution is to BASE64 encode the string.

#### XML defined variable

```xml
<variable
    name="variable-name"
    type="1|STRING"
    user="2"
    access-rights="1911"
    persistence="true|false"
    
    value="value for variable"
    valuebase64="value for variable"
    
    note="variable note"
    notebase64="variable note"
    
`</variable>`
```

`<value>` and `<valuebase64** (__you should never have both defined at the same time__) set the same value but in different ways. **<value>` set  the value as is. So depending on the type it should be BASE64 encoded or not. The `<valuebase64>` always base64 encode the value before setting it.

`<note>` and `<notebase64>` (__you should never have both defined at the same time__) set the note value. `<note>` set the note as is and `<notebase64>` always base64 encode the note before setting it.

**Warning!** [XML reserved characters](http://xml.silmaril.ie/specials.html) can't be used in values and notes. The solution is to BASE64 encode the string.

#### JSON defined variable

```javascript
{  
    "name": "variable-name",
    "type": 1,
    "user": 2,
    "accessrights": 1911,
    "persistence": true|false,
    "lastchange": "YYYYMMDDTHHMMSS",
    "value": "BASE64 encoded variable value"|Variable value,
    "note": "BASE64 encoded variable note"
    "valuebase64": "Variable value"|Variable value,
    "notebase64": "Variable note"
}
```

The **value** item string should be BASE64 encoded if the type require that. You can use **valuebase64** to encode your value before it is stored. 

The **note** item string should always be BASE64 encoded. You can use **notebase64** to always encode your note before it is stored.

**Warning!** Double quotes in strings must be escaped (\").

### Return

Returns error message on failure.

**Example 1**
```lua
-- Create string variable testa
vscp.writevariable('testa;0x01;false;0;0x777;VGhpcyBpcyBhIHRlc3QgdmFyaWFibGU=;VGhpcyBpcyBhIG5vdGU=')

-- Create string variable testb
vscp.writevariable('testb;string;false;0;0x777;BASE64:This is a test;BASE64:Supertest')

-- Create floating point variable testc
vscp.writevariable('testc;flot;false;0;0x777;3.14;BASE64:Important constant')

```

**Example 2**

__Create__ a string variable with name test1 that is readable/writable by all and own by the admin used (0). If the variable already exists it's data will be updated. 

```lua
local str 

-- Create variable from XML
str = '&lt;variable \
    name="testvarxml" \
    type="1" \
    user="0"\
    access-rights="0x777" \
    persistence="false" \
    value="This is a test" \
    note="test" /&gt;'
vscp.writevariable(str,1)
```

**Example 2**
```lua
local str 

-- Create string variable from JSON
str = '{ \
    "name": "test_json", \
    "type": 1, \
    "user": 0, \
    "accessrights": 1920, \
    "persistence": false, \
    "value": "This is a JSON test", \
    "note": "Just a comment"\
}'
vscp.print( "JSON str =  ", str )

vscp.writevariable(str,2)
```


---

##  vscp.writevariablevalue

Write a new value to an existing variable.

### Parameters

```lua
vscp.writevariablevalue(name,value[,bBase64]);
```


*  **name** Name for variable.

*  **value** Value for variable. Value should be of the same type as the variable (boolean, int, long, float, string) but a conversion from string will be done for numbers.

*  **bBase64** Optional boolean value that tells if the given value should be BASE64 encoded before it is set as the new variable value. Default is false.


**Example 1**

Increase a numerical variable value by one and save the variable again

```lua
local val

-- Read value for test1
-- ( 'test1' must be defined elsewhere )
val = vscp.readvariable("test1",3)
val = val + 1
vscp.writevariablevalue("test1",val)
val = vscp.readvariable("test1",3)
vscp_log("Variable value = " + val )
```

**Example 2**

This is a complete decision matrix element that creates an integer  variable 'testint" when the VSCP server is started and then add one to this variable every second

```xml
<?xml version = "1.0" encoding = "UTF-8" ?>

<dm>
       
    <row enable="true" groupid="test" >
    
    <comment>
        Create string variable from string
    </comment>
        
    <mask  priority="0"  
             class="65535"  
             type="65535"  
             GUID=" 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" />
      
    <filter  priority="0"  
             class="65535"  
             type="23"  
             GUID=" 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01" />    
    
    <action>`0x100`</action>
    <param>
       local str 

       -- Create integer variable from JSON
       str = '{ \
           "name": "testint", \
           "type": 3, \
           "user": 0, \
           "accessrights": 1920, \
           "persistence": false, \
           "value": 1000, \
           "note": "This is an integer with intial value set to 1000"\
       }'
       
       vscp.print( "JSON str =  ", str )
       vscp.writevariable(str,2)
    </param>


    <allowed_from></allowed_from>
    <allowed_to></allowed_to>
    <allowed_weekdays></allowed_weekdays>
    <allowed_time></allowed_time>

</row>



<row enable="true" groupid="test" >
    
    <comment>
        Add one to integer variable every second
    </comment>
        
    <mask  priority="0"  
             class="65535"  
             type="65535"  
             GUID=" 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" />
      
    <filter  priority="0"  
             class="65535"  
             type="5"  
             GUID=" 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01" />    
    
    <action>0x100</action>
    <param>
        local val 

        -- Read integer value
        val = vscp.readvariable("testint",3)
        vscp.print("value =", val )
        val = val + 1;
        vscp.writevariable("testint",3,val)
    </param>


    <allowed_from></allowed_from>
    <allowed_to></allowed_to>
    <allowed_weekdays></allowed_weekdays>
    <allowed_time></allowed_time>

    </row>
    
</dm>
```


---



##  vscp.writevariablenote

Write the note of an existing variable.

### Parameters

```lua
vscp.writevariablenote(name,note,bBase64);
```


*  **name** Name of an existing variable.

*  **note** Note for the variable. The note can be given as BASE64 encoded or in clear text depending on the bBase64 flag

*  **bBase64** Boolean value that tells if the given note should be BASE64 encoded before it is set. A note should always be BASE64 encoded so this is a way to set it in clear text. 


---



##  vscp.deletevariable

Delete a VSCP remote variable. You must be the owner of a variable or have write access to be able to delete it.

### Parameters

```lua
vscp.deletevariable( name );
```


*  **name** Name of variable.

### Return

Return boolean *true* on success and *false* on failure. 

### Example

```lua
if ( vscp.deletevariable( "testVariable" ) ) {
    vscp_log("Variable deleted.\n" )
}
else {
    vscp_log("Failed to delete variable!\n" )
}
```


---

##  vscp.isVariableBase64Encoded

Check if variable has a value that is BASE64 encoded.

### Parameter

```lua
local result = vscp.isVariableBase64Encoded(name);
```


*  **name** Name of variable.
*  
### Return

Boolean true if the variables value is BASE64 encoded, false otherwise.

---

##  vscp.isVariablePersistent

Check if a variable is persistent.

### Parameter

```lua
local result = vscp.isVariablePersistent(name);
```


*  **name** Name of variable.
*  
### Return

Boolean true if the variables is persistent, false otherwise.

---

##  vscp.isVariableNumerical

Check if a variable is numerical.

### Parameter

```lua
local result = vscp.isVariableNumerical(name);
```

*  **name** Name of variable.
*  
### Return

Boolean true if the variables is numerical, false otherwise.

---

##  vscp.isStockVariable

Check if a variable is a stock variable.

### Parameter

```lua
local result = vscp.isStockVariable(name);
```


*  **name** Name of variable.

### Return

Boolean true if the variables is a stock variable, false otherwise.

---

##  vscp.sendevent

Send a VSCP event.

### Parameters

```lua
vscp.sendevent( event[,format] );
```

* **event** The VSCP event to send. It can be on string, XML or JSON format.
* **format** The format of the event. If format = 0 the event is in string form, if format = 1 the event is in XML form. if format = 2 the event is in JSON form.

#### String format

obid, datetime, timestamp and GUID can be left empty and if so will be set to default values.

```javascript
head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
```

#### XML format

```xml
<event 
    head="96" 
    obid="0" 
    datetime="20161102T190032" 
    timestamp="0" 
    class="10" 
    type="6" 
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" 
    data="0x8A,0x81,0,0xca" />
```

#### JSON format

```json
{
    "datetime": "20161102T190032",
    "head": 96,
    "timestamp": 0,
    "obid":  0,
    "class": 10,
    "type": 6,
    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    "data": [138,129,0,202]
}
```

**note** has no use in this case and can be left out. **head** if left out defaults to 96 which is *normal priority*. **timestamp** and **obid** both defaults to zero meaning they will be filled in by the system. **guid** can be set to "-" or all nulls (same as being omitted) and the GUID of the interface is then used. **data** can be omitted and is in that case the same as no data. If "time" is left out the current UTC time is used. 

### Return

Error message on failure.

### Example 1

send [CLASS1.MEASUREMENT, TYpe=6,Temperature](http://docs.vscp.org/spec/latest/#/./class1.measurement#type6) event. The temperature 20.4C is sent (unit=1 which is centigrades).

Note that no values are given for obid,timestamp,datetime and GUID. This will set them to default values. obid is set to the interface id, timestamp to a current timestamp, datetime to the UTC date + time, GUID to the GUID of the interface.

```lua
local strevent 

-- Create string event
strevent = "0,10,6,,,,,0x8A,0x81,0,0xca"

vscp.sendevent(strevent)
vscp.log("Event successfully sent from Lua script.")
```

### Example 2

send [CLASS1.MEASUREMENT, TYpe=6,Temperature](http://docs.vscp.org/spec/latest/#/./class1.measurement#type6) event from XML string. The temperature 20.4C is sent (unit=1 which is centigrades).

```lua
local strevent 

-- Create string event
strevent = '&lt;event \
    head="0" \
    obid="0" \
    timestamp="0" \
    class="10" \
    type="6" \
    guid="-" \
    data="0x8A,0x81,0,0xca" /&gt;'
    
vscp.sendevent(strevent,1)
vscp.log("Event successfully sent from Lua script.")
```


###  Example 3

send [CLASS1.MEASUREMENT, TYpe=6,Temperature](http://docs.vscp.org/spec/latest/#/./class1.measurement#type6) event from JSON string. The temperature 20.4C is sent (unit=1 which is centigrades).

```lua

```

---

##  vscp.receiveevent

Receive a VSCP event from the local queue.

### Parameters

```lua
var event = vscp.receiveevent([format])
```


* * format Optional format for received event. If format = 0 (default) the event is returned on the string form. If format = 1 the event is returned on XML form. If format = 2 the event is returned on JSON form.

####  Format for event on string form 

```javascript
head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
```


####  Format for event on XML form 

```xml
<event 
    head="96" 
    obid="0" 
    datetime="20161102T190032" 
    timestamp="0" 
    class="10" 
    type="6" 
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"
    sizedata="4" 
    data="0x8A,0x81,0,0xca" />
```

####  Format for event on JSON form 

```lua
{
    "datetime": "20161102T190032",
    "head": 96,
    "timestamp": 0,
    "obid":  0,
    "class": 10,
    "type": 6,
    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    "data": [138,129,0,202]
}
```

### Return

Return VSCP Event on success, null if there is no event to fetch and an error on failure. 

### Example 1

Wait until any event is received and print out the received event.

```lua
local event   
local bQuit = false

vscp.log("Waiting for VSCP event")

-- Keep looking until event is available
while not bQuit do

    -- Fetch event from client queue
    event = vscp.receiveevent()  -- Fetch event in string form

    if event then

        vscp.log("VSCP event received. Event = " .. event )
        bQuit = true
        
    else 
        vscp.sleep( 500 )
    end
end

vscp.log("Done!\n")
```


---

##  vscp.countevent

Return number of VSCP events in the local queue.

### Parameters

```lua
cnt = vscp.countevents()
```

### Return

Return the number of events waiting in the client queue. A client is opened when the script is started and remains open until the script has terminated.

### Example

Check the client queue ten times with a one second delay between the checks.  

```lua
local cnt   

for i=0,10 do
    cnt = vscp.countevent()
    vscp.log("Count = " .. cnt )
    vscp.sleep( 1000 )
end

vscp.log("Done!\n")
```


---


##  vscp.setfilter

Set a VSCP filter for this clients local queue. This filter limits the events received on this interface.

### Parameters

```lua
vscp.setfilter( filter[,format] )
```

* **filter** A filter specifying a complete VSCP receive filter (mask + filter).
* **format** If format = 0 (default) the filter is expected to be on string form. If format = 1 the filter is expected to be on XML form. If format = 2 the filter is expected to be on JSON format.

#### Filter on string format

```javascript
filter-priority,filter-class,filter-type,filter-GUID,mask-priority, mask-class,mask-type,mask-GUID
```

#### Filter on XML format

```xml
<filter mask_priority="number"
    mask_class="number"
    mask_type="number"
    mask_guid="string"
    filter_priority="number"
    filter_class="number"
    filter_type="number"
    filter_guid="string" />
```

#### Filter on JSON format

```javascript
{
    "mask_priority": number,
    "mask_class": number,
    "mask_type": number,
    "mask_guid": "string",
    "filter_priority": number,
    "filter_class": number,
    "filter_type": number,
    "filter_guid" : "string"
}
```

### Return

Return an error on failure.

### Example 1

Using format=0 (string) set filer so that only [CLASS1.MEASUREMENT](http://docs.vscp.org/spec/latest/#/./class1.measurement) events is received.

```javascript
local filter

filter="0,10,0,-,0,0xffff,0,-"
vscp.setfilter( filter )
vscp.log("Done!\n")
```

### Example 2

Using format=2 (XML) set filer so that only [CLASS1.MEASUREMENT](http://docs.vscp.org/spec/latest/#/./class1.measurement) events is received.

```lua
local filter

filter='&lt;filter mask_priority="0"\
    mask_class="65535"\
    mask_type="0"\
    mask_guid="-"\
    filter_priority="0"\
    filter_class="10"\
    filter_type="0"\
    filter_guid="-" /&gt;'
vscp.setfilter( filter, 1 )
vscp.log("Done!\n")
```

### Example 3

Using format=2 (JSON) set filer so that only [CLASS1.MEASUREMENT](http://docs.vscp.org/spec/latest/#/./class1.measurement) events is received. 

```lua
-- Allow only CLASS1.MEASUREMENT events, from all, 
-- with all priorities
local filter = '{\
    "mask_priority": 0,\
    "mask_class": 65535,\
    "mask_type": 0,\
    "mask_guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",\
    "filter_priority": 0,\
    "filter_class": 10,\
    "filter_type": 0,\
    "filter_guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"\
}'
    
-- Set the filter
vscp.setfilter( filter,2 )
vscp_log("Filter set successfully!\n")
```

---

##  vscp.sendmeasurement

Send a measurement event. A level I or Level II measurement event can be sent with this function with the measurement supplied as a floating point value.

### Parameters

```lua
vscp.sendmeasurement( level, bString, value, guid, vscp-type[, unit, sensor-index, zone, subzone]  );
```


*  **level** Set to 1 to send a level I event, set to 2 to send a level II event.

*  **bString** Only valid for Level II events. If true a [CLASS2.MEASUREMENT_STR](http://docs.vscp.org/spec/latest/#/./class2.measurement_str). If false a [CLASS2.MEASUREMENT_FLOAT](http://docs.vscp.org/spec/latest/#/./class2.measurement_float).

*  **value** A double representing the measurement value. __ **Note** as with all decimal numbers in VSCP a period is used as decial separator.__ 

*  **guid** GUID to use as originating GUID.

*  **vscp-type** This is the [measurement type](http://www.vscp.org/docs/vscpspec/doku.php?id=class1.measurement).

*  **unit** The unit for the measurement. Defaults to 0 (the default unit). Can be 0-3 for a level I measurement and 0-255 for a level II measurement. Optional.

*  **sensor-index** The sensor index for the measurement. Defaults to 0.  Can be 0-7 for a level I measurement and 0-255 for a level II measurement. Optional.

*  ** zone** The zone for the measurement. Defaults to 0.  Can be 0-255 for a level I measurement and 0-255 for a level II measurement. Optional.

*  **subzone** The subzone for the measurement.Defaults to 0.  Can be 0-255 for a level I measurement and 0-255 for a level II measurement. Optional.



 

### Return

Return errot on failure.

### Example 1

```lua
-- Send a measurement
-- Level 1, temperature, floating point value

    
-- Send the measurement
vscp.sendmeasurement( 1, false, 123.5, "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00", 6, 1 )
vscp.log("Sent measurement event successfully!\n")
```



### Example 2

```lua
-- Send a measurement
-- Level II, force, string, default unit
  
-- Send the meaurement
vscp.sendmeasurement( 2, true, 0.678956 , "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00", 11, 0 )
vscp.log("Sent measurement event successfully!\n")
```

---

##  vscp.ismeasurement

Check if this VSCP event is a measurement event.

### Parameters

```lua
result = vscp.ismeasurement( event[, format] )
```


*  **event** A string that holds a VSCP event.

*  **format** Format for the event string. If format=0 (default) the event is in string format, if format=1 the string is in XML format and if format=2 the string is in JSON format.

#### String format

obid, datetime, timestamp and GUID can be left empty and if so will be set to default values.

```javascript
head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
```

#### XML format

```xml
<event 
    head="96" 
    obid="0" 
    datetime="20161102T190032" 
    timestamp="0" 
    class="10" 
    type="6" 
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" 
    data="0x8A,0x81,0,0xca" />
```

#### JSON format

```json
{
    "datetime": "20161102T190032",
    "head": 96,
    "timestamp": 0,
    "obid":  0,
    "class": 10,
    "type": 6,
    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    "data": [138,129,0,202]
}
```

**note** has no use in this case and can be left out. **head** if left out defaults to 96 which is *normal priority*. **timestamp** and **obid** both defaults to zero meaning they will be filled in by the system. **guid** can be set to "-" or all nulls (same as being omitted) and the GUID of the interface is then used. **data** can be omitted and is in that case the same as no data. If "time" is left out the current UTC time is used. 

### Return

Return boolean *true* if the event is a measurement event and *false* if not.


---


##  vscp.getmeasurementvalue

Get a measurement value from a VSCP measurement event.

### Parameters

```lua
value = vscp.getmeasurementvalue( event[, format] )
```


*  **event** A string that holds a VSCP event.

*  **format** Format for the event string. If format=0 (default) the event is in string format, if format=1 the string is in XML format and if format=2 the string is in JSON format.

#### String format

obid, datetime, timestamp and GUID can be left empty and if so will be set to default values.

```javascript
head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
```

#### XML format

```xml
<event 
    head="96" 
    obid="0" 
    datetime="20161102T190032" 
    timestamp="0" 
    class="10" 
    type="6" 
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" 
    data="0x8A,0x81,0,0xca" />
```

#### JSON format

```json
{
    "datetime": "20161102T190032",
    "head": 96,
    "timestamp": 0,
    "obid":  0,
    "class": 10,
    "type": 6,
    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    "data": [138,129,0,202]
}
```

**note** has no use in this case and can be left out. **head** if left out defaults to 96 which is *normal priority*. **timestamp** and **obid** both defaults to zero meaning they will be filled in by the system. **guid** can be set to "-" or all nulls (same as being omitted) and the GUID of the interface is then used. **data** can be omitted and is in that case the same as no data. If "time" is left out the current UTC time is used. 

### Return

Return the value for the measurement as a double or null if error or if event is not a measurement or an error occures.



---


##  vscp.getmeasurementunit

Get a measurement unit from a VSCP measurement event.

### Parameters

```lua
result = vscp.getmeasurementunit( event[, format] )
```


*  **event** A string that holds a VSCP event.

*  **format** Format for the event string. If format=0 (default) the event is in string format, if format=1 the string is in XML format and if format=2 the string is in JSON format.

#### String format

obid, datetime, timestamp and GUID can be left empty and if so will be set to default values.

```javascript
head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
```

#### XML format

```xml
<event 
    head="96" 
    obid="0" 
    datetime="20161102T190032" 
    timestamp="0" 
    class="10" 
    type="6" 
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" 
    data="0x8A,0x81,0,0xca" />
```

#### JSON format

```json
{
    "datetime": "20161102T190032",
    "head": 96,
    "timestamp": 0,
    "obid":  0,
    "class": 10,
    "type": 6,
    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    "data": [138,129,0,202]
}
```

**note** has no use in this case and can be left out. **head** if left out defaults to 96 which is *normal priority*. **timestamp** and **obid** both defaults to zero meaning they will be filled in by the system. **guid** can be set to "-" or all nulls (same as being omitted) and the GUID of the interface is then used. **data** can be omitted and is in that case the same as no data. If "time" is left out the current UTC time is used. 

### Return

Return the unit for the measurement as an integer or null if error or if event is not a measurement.



---


##  vscp.getmeasurementsensorindex

Get a measurement sensor index from a VSCP measurement event.

### Parameters

```lua
result = vscp.getmeasurementsensorindex( event[, format] )
```


*  **event** A string that holds a VSCP event.

*  **format** Format for the event string. If format=0 (default) the event is in string format, if format=1 the string is in XML format and if format=2 the string is in JSON format.

#### String format

obid, datetime, timestamp and GUID can be left empty and if so will be set to default values.

```javascript
head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
```

#### XML format

```xml
<event 
    head="96" 
    obid="0" 
    datetime="20161102T190032" 
    timestamp="0" 
    class="10" 
    type="6" 
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" 
    data="0x8A,0x81,0,0xca" />
```

#### JSON format

```json
{
    "datetime": "20161102T190032",
    "head": 96,
    "timestamp": 0,
    "obid":  0,
    "class": 10,
    "type": 6,
    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    "data": [138,129,0,202]
}
```

**note** has no use in this case and can be left out. **head** if left out defaults to 96 which is *normal priority*. **timestamp** and **obid** both defaults to zero meaning they will be filled in by the system. **guid** can be set to "-" or all nulls (same as being omitted) and the GUID of the interface is then used. **data** can be omitted and is in that case the same as no data. If "time" is left out the current UTC time is used. 

### Return

Return the sensorindex for the measurement as an integer or null if error or if event is not a measurement.



---


##   vscp.getmeasurementzone

Get a measurement zone from a VSCP measurement event.

### Parameters

```lua
result = vscp.getmeasurementzone( event[, format] )
```


*  **event** A string that holds a VSCP event.

*  **format** Format for the event string. If format=0 (default) the event is in string format, if format=1 the string is in XML format and if format=2 the string is in JSON format.

#### String format

obid, datetime, timestamp and GUID can be left empty and if so will be set to default values.

```javascript
head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
```

#### XML format

```xml
<event 
    head="96" 
    obid="0" 
    datetime="20161102T190032" 
    timestamp="0" 
    class="10" 
    type="6" 
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" 
    data="0x8A,0x81,0,0xca" />
```

#### JSON format

```json
{
    "datetime": "20161102T190032",
    "head": 96,
    "timestamp": 0,
    "obid":  0,
    "class": 10,
    "type": 6,
    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    "data": [138,129,0,202]
}
```

**note** has no use in this case and can be left out. **head** if left out defaults to 96 which is *normal priority*. **timestamp** and **obid** both defaults to zero meaning they will be filled in by the system. **guid** can be set to "-" or all nulls (same as being omitted) and the GUID of the interface is then used. **data** can be omitted and is in that case the same as no data. If "time" is left out the current UTC time is used. 

### Return

Return the zone for the measurement as an integer or null if error or if event is not a measurement. Level I events that does not have a zone defined will get a zero returned.




---


##   vscp.getmeasurementsubzone

Get a measurement Sib Zone from a VSCP measurement event.

### Parameters

```lua
result = vscp.getmeasurementsubzone( event[, format] )
```


*  **event** A string that holds a VSCP event.

*  **format** Format for the event string. If format=0 (default) the event is in string format, if format=1 the string is in XML format and if format=2 the string is in JSON format.

#### String format

obid, datetime, timestamp and GUID can be left empty and if so will be set to default values.

```javascript
head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
```

#### XML format

```xml
<event 
    head="96" 
    obid="0" 
    datetime="20161102T190032" 
    timestamp="0" 
    class="10" 
    type="6" 
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" 
    data="0x8A,0x81,0,0xca" />
```

#### JSON format

```json
{
    "datetime": "20161102T190032",
    "head": 96,
    "timestamp": 0,
    "obid":  0,
    "class": 10,
    "type": 6,
    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    "data": [138,129,0,202]
}
```

**note** has no use in this case and can be left out. **head** if left out defaults to 96 which is *normal priority*. **timestamp** and **obid** both defaults to zero meaning they will be filled in by the system. **guid** can be set to "-" or all nulls (same as being omitted) and the GUID of the interface is then used. **data** can be omitted and is in that case the same as no data. If "time" is left out the current UTC time is used. 

### Return

Return the subzone for the measurement as an integer or null if error or if event is not a measurement.  Level I events that does not have a zone defined will get a zero returned.


##  File system manipulation library 

This library offers these functions:
 

*  lfs.attributes (filepath [, attributename])
*  lfs.chdir (path)
*  lfs.currentdir ()
*  lfs.dir (path)
*  lfs.lock (fh, mode)
*  lfs.lock_dir (path)
*  lfs.mkdir (path)
*  lfs.rmdir (path)
*  lfs.setmode (filepath, mode)
*  lfs.symlinkattributes (filepath [, attributename]) -- thanks to Sam Roberts
*  lfs.touch (filepath [, atime [, mtime]])
*  lfs.unlock (fh)

## LuaXML

See documentation [here](https://github.com/LuaDist/luaxml).


## LuaSqlite3

See documentation [here](https://github.com/LuaDist/lsqlite3).


[filename](./bottom_copyright.md ':include')


