# vscpws_simpleText

The vscpws_simpleText is a widget for formatted dynamic text that can be displayed on a web page. This may not sound very exciting but it is. In the past Ajax was used for this and the server was polled for data. Here websockets are used giving a much more lightweight data handling. The text can be a dynamically updated. Temperature value or some other measurement which is formatted nicely as you have all formatting capabilities of HTML5 at your disposal. You can display information from received events or data from a VSCP daemon variable. 

## Construction of a vscpws_simpleText

`<code=c>`
vscpws_simpleText( username,          // username                                                      
                     passwordhash     // passwordhash,
                     url,             // url to VSCP websocket i/f 
                     id,              // Where it should be placed 
                     vscpclass,       // VSCP event class e.g. 10/60/65/15 
                     vscptype,        // VSCP Event type 
                     sensorindex,     // Data coding index 
                     decimals,        // Number of decimals 
                     formatstr,       // A value format string 
                     guid,            // GUID we are interested in 
                     fncallback )     // If set function to call 
                                      // when data arrives 
```

### username

Username to logon to the websocket server.

### passwordhash

Password hash to logon to the websocket server

### url

Url to the websocket server. This typically is on the form 

    "ws://192.168.1.20:8884"
    
or

    "wss://192.168.1.20:8884" 
    
if SSL is used.  

With all widgets having there own user/password/url specified for the websocket server it is possible to create web pages that control nodes/units/hardware that are located in different locations from the same page. 

### id

This is the id for the HTML element where the result should be placed. This can be any HTML element which can display text and all formating capabilities of HTML can obviously be used.

### vscpclass

This is the the VSCP class of the event we are interested in. Normally this is one of the measurement events which have all SI units and more (usually derived units) specified. Set to -1 for don't care.

### vscptype

This is the the VSCP type of the event we are interested in. Set to -1 for don't care.

### sensorindex

This is the sensor index for the sensor in the device sending out the event. This information is part of the datacoding and can be a value 0,1,2,3,4,5,6,7. Set to -1 for don't care. Default=0.

### decimals

Number of decimals in the displayed value. Default is two.

### formatstr

This is a string that can be used to format the value before it is written to the HTML element. The string have a format on this form 

"{0} degrees Celsius"

where the escapes in {n} will be replaced by real time data. 

 | Escape | Description                                                                          | 
 | ------ | -----------                                                                          | 
 | {0}    | Measurement value e.g "-12.5".                                                       | 
 | {1}    | Measurement unit in real text form e.g. for temperature "Celsius", "Fahrenheit" etc. | 
 | {2}    | Measurement unit in numerical form e.g. "0", "1", "2" etc.                           | 
 | {3}    | Sensor index e.g. "0", "1", "2"...”7”.                                           | 

### guid

This can be used to match a specific GUID the event should come from. Default is "" meaning events from all sensors are of interest.


### fncallback

Here it is possible to enter a user defined function that get the value, the unit and the full event as its arguments. The function should have the following form

    userfunction(value,unit,event)
    
This function should return a string that will be written to the HTML page or null if the callback do this write itself or for some other reason does not want the page to be updated. This can be useful if one wants to do some special handling with the event.
 
## Methods

###  setExtraParameters

To use the vscpws_simpleTextEvent widget with events in class VSCP_CLASS1_MEASUREZONE and VSCP_CLASS1_SETVALUEZONE you may want to filter on more than sensorindex and GUID. This method allows the extra info available in these classes to be checked as well. 

**Usage**
`<code=c>`
setExtraParameters( index,      // Index if applicable 
                      zone,     // Zone if applicable 
                      subzone ) // Subzone if applicable 
```

##### index

This can be used to match a specific index. Default is -1 meaning don't care. Only VSCP_CLASS1_MEASUREZONE and VSCP_CLASS1_SETVALUEZONE have an index to check among the measurement classes.

#####  zone

This can be used to match a specific zone. Default is -1 meaning don't care. Only VSCP_CLASS1_MEASUREZONE and VSCP_CLASS1_SETVALUEZONE have a zone to check among the measurement classes.

##### subzone

This can be used to match a specific subzone. Default is -1 meaning don't care. Only VSCP_CLASS1_MEASUREZONE and VSCP_CLASS1_SETVALUEZONE have a subzone to check among the measurement classes.


## Using the vscpws_simpleText widget

All vscpws controls have a sample in the **vscp_html5/testws** folder of the
repository [VSCP HTML5 code](http://github.com/grodansparadis/vscp_html5). For the  vscpws_simpletext control this sample is  
[here](http://github.com/grodansparadis/vscp_html5/blob/master/testws/simpletext.html).

Suppose you have a temperature sensor like the [Kelvin NTC](http://www.auto.grodansparadis.com/kelvinntc10k/kelvin_ntc10ka.html)  which sends out temperature events at even intervals. From this device you will get events like

    VSCP CLASS = 10 i.e. CLASS1.MEASUREMENT we have a measurement event.
    VSCP Type = 6 Telling that this is a temperature measurement.
    DATA = 0x8A,0x02,0x09,0x72 telling that this is data from sensor 2 
                              and that the temperature now is +24.18 degrees
                              Celsius.

If you are interested in the format and want to understand how to decode it please see the specification (see section about data coding in the specification document ). 

To display this temperature measurement in a HTML page we can insert a line like this 

    `<div>` This is just some text in a div where a temperature 
      is equal to `<span id="id1">` `</span>` is displayed in-line 
      in the text. `</div>`

Nothing to fancy. The "id1" is the interesting part here. This where the temperature value will be placed. Naturally this span-element can have all attributes a HTML element can have just as the `<div>`. Also it can be some other HTML element that can display text. 

To display values with two decimals from sensor 2 we insert this code in our HTML page

```javascript
`<script type="text/javascript" >`
    var txt1 = 
         new vscpws_simpleTextEvent( "ws://192.168.1.20:7681", 
                                        "id1", 
                                        VSCP_CLASS1_MEASUREMENT,
                                        VSCP_TYPE_MEASUREMENT_TEMPERATURE, 											     
                                        3, 2 ); 
`</script>`
```

This is all that is needed. A live web page is now available that updates the temperature with the interval set when configuring the sensor.

{{:html5:simpletext_example1.png?nolink&800|}}

You can add a format string if you want to change the appearance of the output. Add a line to the above so it looks like this instead


```javascript
`<script type="text/javascript" >` 
var txt1 = new vscpws_simpleTextEvent( "ws://192.168.1.20:7681", 											
                                           "id1",                                                                                       								                 
                                           VSCP_CLASS1_MEASUREMENT,                                                                                      									                                                                                      
                                           VSCP_TYPE_MEASUREMENT_TEMPERATURE, 										
                                           3, 2,											
                                           "{0} nice degrees Celsius" ); 
`</script>`
```

which will look like this

{{:html5:simpletext_example2.png?nolink&800|}}
 

In the format string you can put in any text you like. {0} is the temperature value but there are also other escapes you can use. For example {1} is the unit symbol (e.g. for temperature “Kelvin”, "Celsius", "Fahrenheit" etc.) for the measurement. This is possible because all VSCP events carry this information with them. Trying this

```javascript
`<script type="text/javascript" >`          	
var txt1 = new vscpws_simpleTextEvent( "ws://192.168.1.20:7681",
                                             "id1",
                                             VSCP_CLASS1_MEASUREMENT,
                                             VSCP_TYPE_MEASUREMENT_TEMPERATURE,
                                             2,
                                             3,
                                             "{0} degrees {1}"); 
`</script>`
```

Gives this result

{{:html5:simpletext_example3.png?nolink&800|}} 

But by inserting

`<code=html>`
`<div>`     
    Temperature placed inside &lt;H1&gt; tags. `<h1 id="id2">``</h1>` 
`</div>`
```

and creating the control

```javascript
`<script>`
var txt2 = new vscpws_simpleTextEvent( "ws://192.168.1.20:7681",
                                             "id2",
                                             VSCP_CLASS1_MEASUREMENT,
                                             VSCP_TYPE_MEASUREMENT_TEMPERATURE,
                                             0,
                                             3,
                                             "Temperature is {0} degrees {1}"); 
`</script>`
```

also works and give this result

{{:html5:simpletext_example4.png?nolink&800|}}

showing dynamic temperature values for sensor 0. 

If you instead want to have the measurement value in some other from you can use the callback to do some calculations on the value before it is written to the page. The callback is called before the formatting string is applied. For example if you want the example above to be displayed with the Fahrenheit unit incited of Celsius you can use the built in conversion code and

```javascript
script>
var txt2 = new vscpws_simpleTextEvent( "ws://192.168.1.20:7681",
                                         "id2",
                                         VSCP_CLASS1_MEASUREMENT,
                                         VSCP_TYPE_MEASUREMENT_TEMPERATURE,
                                         0,
                                         3,
                                         "Temperature is {0} degrees Fahrenheit",
                 					 function(value,unit,event) { 
    return ("" + vscpws_toFixed(vscpws_convertCelsiusToFahrenheit(value),2));} );
`</script>`
```

or in a more readable form

```javascript
`<script>`
var fnconvert = function(value,unit,event)         {
    return ("" + vscpws_toFixed(vscpws_convertCelsiusToFahrenheit(value),2));
}

var txt2 = new vscpws_simpleTextEvent( "ws://192.168.1.20:7681",
                                           "id2",
                                           VSCP_CLASS1_MEASUREMENT,
                                           VSCP_TYPE_MEASUREMENT_TEMPERATURE,
                                           0,
                                           3,
                                           "Temperature is {0} degrees Fahrenheit",
                 		           fnconvert );
`</script>`
```


which outputs

{{:html5:simpletext_example5.png?nolink&800|}}


The widget can be used for other events then measurement event also. To code something that lists the GUID for each new node discovered in the system one can use the following code

`<code=html>`
`<div>`     
   `<h1>`Unit discovery`</h1>`     
   `<div id="id3">``</div>` 
`</div>`
```

and

```javascript
`<script>`
    var idArray = new Array();

    var newnode = function(value,unit,event) {
    //alert("Heartbeat:" + event);
    var guid = event[3];
    var bFound = false;

    for (i=0;i<idArray.length;i++) {
        if ( guid == idArray[i]) {
            bFound = true;
            break;
        }
    }
                 
    if (!bFound) {
        var strOut = new String();
        idArray.push(guid);
        for (i=0;i<idArray.length;i++) {
            strOut += idArray[i] + " - ";
        }
        document.getElementById("id3").textContent = strOut;
    }
    return null;
};
                                           
new vscpws_simpleTextEvent( "ws://192.168.1.20:7681",
                              null,
                              VSCP_CLASS1_INFORMATION,
                              VSCP_TYPE_INFORMATION_NODE_HEARTBEAT,
                              -1,
                              -1,
                              "",
                              "",
                              newnode ); 
`</script>`
```

which list the nodes

{{:html5:simpletext_example6.png?nolink&800|}}




\\ 
----
{{  ::copyright.png?600  |}}

`<HTML>``<p style="color:red;text-align:center;">``<a href="http://www.grodansparadis.com">`Grodans Paradis AB`</a>``</p>``</HTML>`
