# vscpws_Event

The vscpws_Event widgets is a convenient tool to use to send and receive events through the websocket interface. You basically just need to construct it and set a receiver of incoming event and you are up and running. It can't be easier.

## Construction of a vscpws_Event

`<code=c>`
var ev1 = vscpws_Event( username,              // Username for websocket server  
                           passwordhash,       // Password hash for websocket
                           serverurl,          // url to VSCP websocket i/f
                           fncallbackresponse, // If set function to call when 
                                               // response arrives over websocket
                                               // i/f (can be null)
                           fncallbackevent )   // If set function to call when
                                               // events arrive (can be null)
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

### fncallbackresponse

If used define a call back function here that will receive asynchronous responses (positive or negative) from commands.

The callback function should have the following definition
`<code=c>`
var eventReceived = function( bResult,      // True or false for command outcome
                                 msgitems ) // Full websocket reply in array
```

Can be set null if not needed.
### fncallbackevent

If used define a call back function here that will receive incoming events.

The callback function should have the following definition
`<code=c>`
var eventReceived = function( vscphead,        // VSCP head code
                                vscpclass,     // VSCP class
                                vscptype,      // VSCP type
                                vscpobid,      // obid
                                vscptimestamp, // timestamp
                                vscpguid,      // GUID
                                vscpdata )     // Array with data 
```

Can be set null if not needed.

## Methods

###  setFilter

Set the event filter for incoming events. This makes it possible to just get the events you are interested in. Default is getting all events.

**Usage**
`<code=c>`
vscpws_Event.prototype.setFilter = function( filter_class,   // Filter class
                                                filter_type, // Filter type
                                                filter_guid, // Filter GUID string
                                                mask_class,  // Mask class
                                                mask_type,   // Mask type
                                                mask_guid  ) // Mask GUID string
```


##### filter_class

Set the VSCP class you are interested to receive here

##### filter_type

Set the VSCP type you are interested to receive here

##### filter_guid

Set the VSCP GUID you are interested to receive here. It should be on the standard string form "FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF" with most significant byte first.

##### mask_class

Set a one for each bit in the incoming event that should be equal with what is set in filter_class. A zero is a don't care. If you want to receive events of a specific class set this value to 0xFFFF or 65535 and the VSCP class you want to receive as the value for filter_class. If you on the other hand is interested in all VSCP classes set the mask_class to zero. 

##### mask_type

Set a one for each bit in the incoming event that should be equal with what is set in filter_type. A zero is a don't care. If you want to receive events of a specific class set this value to 0xFFFF or 65535 and the VSCP class you want to receive as the value for filter_type. If you on the other hand is interested in all VSCP type set the mask_type to zero. 

##### mask_guid

Set a one for each bit in the incoming event that should be equal with what is set in filter_guid. A zero is a don't care. If you want to receive events of a specific class set this value to "FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF"  and the GUID you want to receive as the value for filter_guid. If you on the other hand is interested in all VSCP GUID's set the mask_guid to "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00". 

### sendEvent

Send an outgoing event.

**Usage**

`<code=c>`
sendEvent( vscphead,           // VSCP head
               vscpclass,      // VSCP class
               vscptype,       // VSCP type
               vscpobid,       // obid
               vscptimestamp,  // timestamp
               vscpguid,       // GUID as string
               vscpdata )      // VSCP data as array or string
```

##### vscphead

The VSCP heasd is seldom used  so it is normally set to zero.

#####  vscpclass

The VSCP class of the outgoing event.

#####  vscptype

The VSCP type of the outgoing event.

#####  vscpobid

This is value used by the the VSCP daemon and will be overwritten. 

#####  vscptimestamp

Set a timestamp of your own choice in milliseconds or set to zero to let the daemon set it for you

#####  vscpguid

Set the GUID for the outgoing event as a string on the form "FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF" or use "-" and the GUID of the interface will be used.

##### vscpdata

VSCP data as a comma separated string or as an array.

### clrQueue

Clear the input queue 

**Usage**

`<code=c>`
   clrQueue()
```

## Using the vscpws_Event widget

All vscpws controls have a sample in the **vscp_html5/testws** folder of the
repository [VSCP HTML5 code](http://github.com/grodansparadis/vscp_html5). For the  vscpws_event control this sample is  
[here](http://github.com/grodansparadis/vscp_html5/blob/master/testws/event.html).


\\ 
----
{{  ::copyright.png?600  |}}

`<HTML>``<p style="color:red;text-align:center;">``<a href="http://www.grodansparadis.com">`Grodans Paradis AB`</a>``</p>``</HTML>`
