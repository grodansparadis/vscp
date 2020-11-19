# vscpws_stateButton

The **vscpws_stateButton** is a button that can be used as a state button or as a stateless button. It has the ability to send an event when it changes it state (one for each state) or for a stateless button one when it is pressed and one when released. Also the button can change it's state according to incoming events or a monitored VSCP Daemon variable value. The button can have one of several appearances shown below or a custom appearance defined by the user.

To assign a monitored variable to the button is a good solution if a state should be monitored over time. Instead of waiting for the next event that tell the button state a variable is read from the daemon which is kept persistent and up to date to get the state. This variable is written/changed in the decision matrix or by a user or an application through the TCP/IP interface of the daemon and can thus be influenced and controlled by some simple or advanced criteria. This also can be used to monitor the state for a specific condition by monitor a variable and just show the state on the page not allowing clicking the button. Typical scenarios can be to indicate an open window, an alarm condition or things like that.

The decision matrix of the VSCP daemon is also perfect to use if one wants to have a button that sends several events. This can be anything of course. Some dimmer commands combined with some on/offs and some curtain roll downs. This is then just added to the decision matrix and triggered on the event sent from the button. Typical use is to control for example any lamps from one button to set a specified scenery for a house or a stage.

Buttons can be set to be stateless or remember there state.

A state remembering button __as default__ send

    CLASS1.CONTROL,TurnOn index=0, zone=0, subzone=0 

when activated and

    CLASS1.CONTROL,TurnOff index=0, zone=0, subzone=0 

when inactivated. The button will change it's state either when a monitored variable change its state or go to the active state when a

    CLASS1.INFORMATION, On-event index=0, zone=0, subzone=0

is received and go to it's inactive state when a

    CLASS1.INFORMATION,off-event index=0, zone=0, subzone=0 is received.

A stateless button __as default__ send

    CLASS1.CONTROL,TurnOn index=0, zone=0, subzone=0 
    
when pressed and

    CLASS1.CONTROL,TurnOff index=0, zone=0, subzone=0 

when released. 

Creating a button is typically done with

    var btn1 = new vscpwsstateButton( "ws://192.168.1.20:8884", "buttonKitchen"); 

which will create a default state button of type=0 looking like this. 

{{:html5:default_state_button.jpg?nolink|}}


Here "buttonKitchen" refers to a canvas where the button is placed as explained below.

If the connection to the server is lost for some reason stateButtons will look like this

{{:html5:default_state_button_inactive.jpg?nolink|}}


where the cross over them indicate a broken connection to the server.

## Construction of a vscpws_stateButton

```javascript
var bt1 = vscpws_stateButton( username,          // username                                                      
                                passwordhash,    // passwordhash 
                                url,             // url to VSCP websocket i/f 
                                canvasName,      // Where it should be placed 
                                bLocal,          // Local visual control 
                                btnType,         // Button type 
                                bNoState,        // True for nonstate button 
                                bDisable,        // Disable user interactions 
                                customupsrc,     // Custom up image 
                                customdownsrc )  // Custom down image
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

### canvasName

This is the name of the canvas element where the button should be placed. Typically this is defined on the form

```javascript
<canvas id="buttonKitchen"
    style="z-index: 3;
        position:absolute;
        left:190px;
        top:310px;" height="30px" width="22px">
        Your browser does not support HTML5 Canvas.
`</canvas>`
```


The id is the parameter that goes for the canvasName. This name is also used to create the instance name for the button and the name set is preceded with vscpws_. The canvas specifies the position and the size for the button. Also z-order is possible to define so that objects can be placed behind, or partially behind each other to get nice visual effects. 

### bLocal

If set to true visual indication is handled locally. For a state button appearance will be changed on every click and for a stateless button appearance will be changed when the button is pressed and then again when it is released. It is better to have the visual appearance of the buttons controlled by external events as it also a confirmation that the sent event actually did what it was intended to do. In some situations however it may be good to handle this locally for some reason. Default=false 

### btnType

This is the visual appearances of the button. Check for possible values below. -1 is reserved for a user defined button. When used one has to supply a path to an image for both the "down" and the "up" state for the button in the *customupsrc* and *customdownsrc* parameters. parameters. Default = 0.   

##### Appearances types

The images below is shown in there default size. Most of the buttons is designed for a different size and with an appropriate background color.

**Type** = 0 Default \\ 
{{:html5:statebutton:statebutton_type0.png?nolink|}} \\ 


**Type** = 1\\ 
{{:html5:statebutton:statebutton_type1.png?nolink|}}\\ 


**Type** = 2 \\ 
{{:html5:statebutton:statebutton_type2.png?nolink|}}\\ 


**Type** = 3 \\ 
{{:html5:statebutton:statebutton_type3.png?nolink|}}\\ 



**Type** = 4 \\ 
{{:html5:statebutton:statebutton_type4.png?nolink|}}\\ 
 


**Type** = 5 \\ 
{{:html5:statebutton:statebutton_type5.png?nolink|}}\\ 



**Type** = 6 \\ 
{{:html5:statebutton:statebutton_type6.png?nolink|}}\\ 
 


**Type** = 7 \\ 
{{:html5:statebutton:statebutton_type7.png?nolink|}}\\ 



**Type** = 8 \\ 
{{:html5:statebutton:statebutton_type8.png?nolink|}}\\ 
 


**Type** = 9 \\ 
{{:html5:statebutton:statebutton_type9.png?nolink|}}\\ 
 


**Type** = 10 \\ 
{{:html5:statebutton:statebutton_type10.png?nolink|}}\\ 



**Type** = 11 \\ 
{{:html5:statebutton:statebutton_type11.png?nolink|}}\\ 



**Type** = 12 \\ 
{{:html5:statebutton:statebutton_type12.png?nolink|}}\\ 



**Type** = 13 \\ 
{{:html5:statebutton:statebutton_type13.png?nolink|}}\\ 
 


**Type** = 14 \\ 
{{:html5:statebutton:statebutton_type14.png?nolink|}}\\ 
 


**Type** = 15 \\ 
{{:html5:statebutton:statebutton_type15.png?nolink|}}\\ 



**Type** = 16 \\ 
{{:html5:statebutton:statebutton_type16.png?nolink|}}\\ 
 


**Type** = 17 \\ 
{{:html5:statebutton:statebutton_type17.png?nolink|}}\\ 



**Type** = 18 \\ 
{{:html5:statebutton:statebutton_type18.png?nolink|}}\\ 



**Type** = 19 \\ 
{{:html5:statebutton:statebutton_type19.png?nolink|}}\\ 
 


**Type** = 20 \\ 
{{:html5:statebutton:statebutton_type20.png?nolink|}}\\ 



**Type** = 21 \\ 
{{:html5:statebutton:statebutton_type21.png?nolink|}}\\ 



**Type** = 22 \\ 
{{:html5:statebutton:statebutton_type22.png?nolink|}}\\ 



**Type** = 23 \\ 
{{:html5:statebutton:statebutton_type23.png?nolink|}}\\ 



**Type** = 24 \\ 
{{:html5:statebutton:statebutton_type24.png?nolink|}}\\ 



**Type** = 25 \\ 
{{:html5:statebutton:statebutton_type25.png?nolink|}}\\ 



**Type** = 26 \\ 
{{:html5:statebutton:statebutton_type26.png?nolink|}}\\ 
 

**Type** = 27 \\ 
{{:html5:statebutton:statebutton_type27.png?nolink|}}\\ 
 


**Type** = 28 \\ 
{{:html5:statebutton:statebutton_type28.png?nolink|}}\\ 



Type = 29 \\ 
{{:html5:statebutton:statebutton_type29.png?nolink|}}\\ 



**Type** = 30 \\ 
{{:html5:statebutton:statebutton_type30.png?nolink|}}\\ 



Type = 31 \\ 
{{:html5:statebutton:statebutton_type31.png?nolink|}}\\ 



**Type** = 32 \\ 
{{:html5:statebutton:statebutton_type32.png?nolink|}}\\ 



**Type** = 33 \\ 
{{:html5:statebutton:statebutton_type33.png?nolink|}}\\ 



**Type** = 34 \\ 
{{:html5:statebutton:statebutton_type34.png?nolink|}}\\ 



**Type** = 35 \\ 
{{:html5:statebutton:statebutton_type35.png?nolink|}}\\ 


**Type** = 36 \\ 
{{:html5:statebutton:statebutton_type36.png?nolink|}}\\ 



**Type** = 37 \\ 
{{:html5:statebutton:statebutton_type37.png?nolink|}}\\ 



**Type** = 38 \\ 
{{:html5:statebutton:statebutton_type38.png?nolink|}}\\ 



**Type** = 39 \\ 
{{:html5:statebutton:statebutton_type39.png?nolink|}}\\ 



**Type** = 40 \\ 
{{:html5:statebutton:statebutton_type40.png?nolink|}}\\ 



**Type** = 41 \\ 
{{:html5:statebutton:statebutton_type41.png?nolink|}}\\ 
 


**Type** = 42 \\ 
{{:html5:statebutton:statebutton_type42.png?nolink|}}\\ 



**Type** = 43 \\ 
{{:html5:statebutton:statebutton_type43.png?nolink|}}\\ 



**Type** = 44 \\ 
{{:html5:statebutton:statebutton_type44.png?nolink|}}\\ 



**Type** = 45 \\ 
{{:html5:statebutton:statebutton_type45.png?nolink|}}\\ 



**Type** = 46 \\ 
{{:html5:statebutton:statebutton_type46.png?nolink|}}\\ 
 


**Type** = 47 \\ 
{{:html5:statebutton:statebutton_type47.png?nolink|}}\\ 
 


**Type** = 48 \\ 
{{:html5:statebutton:statebutton_type48.png?nolink|}}\\ 



**Type** = 49 \\ 
{{:html5:statebutton:statebutton_type49.png?nolink|}}\\ 



**Type **= 50 \\ 
{{:html5:statebutton:statebutton_type50.png?nolink|}}\\ 



**Type** = 51 \\ 
{{:html5:statebutton:statebutton_type51.png?nolink|}}\\ 



**Type** = 52 \\ 
{{:html5:statebutton:statebutton_type52.png?nolink|}}\\ 



**Type** = 53 \\ 
{{:html5:statebutton:statebutton_type53.png?nolink|}}\\ 



**Type** = 54 \\ 
{{:html5:statebutton:statebutton_type54.png?nolink|}}\\ 



**Type** = 55 \\ 
{{:html5:statebutton:statebutton_type55.png?nolink|}}\\ 



**Type** = 56 \\ 
{{:html5:statebutton:statebutton_type56.png?nolink|}}\\ 



**Type** = 57 \\ 
{{:html5:statebutton:statebutton_type57.png?nolink|}}\\ 
 


**Type** = 58 \\ 
{{:html5:statebutton:statebutton_type58.png?nolink|}}\\ 



**Type** = 59 \\ 
{{:html5:statebutton:statebutton_type59.png?nolink|}}\\ 



**Type** = 60 \\ 
{{:html5:statebutton:statebutton_type60.png?nolink|}}\\ 



**Type** = 61 \\ 
{{:html5:statebutton:statebutton_type61.png?nolink|}}\\ 


**Type** = 62 \\ 
{{:html5:statebutton:statebutton_type62.png?nolink|}}\\ 



**Type** = 63 \\ 
{{:html5:statebutton:statebutton_type63.png?nolink|}}\\ 



**Type** = 64 \\ 
{{:html5:statebutton:statebutton_type64.png?nolink|}}\\ 



**Type** = 65 \\ 
{{:html5:statebutton:statebutton_type65.png?nolink|}}\\ 



**Type** = 66 \\ 
{{:html5:statebutton:statebutton_type66.png?nolink|}}\\ 



**Type** = 67 \\ 
{{:html5:statebutton:statebutton_type67.png?nolink|}}\\ 



**Type** = 68 \\ 
{{:html5:statebutton:statebutton_type68.png?nolink|}}\\ 



**Type** = 69 \\ 
{{:html5:statebutton:statebutton_type69.png?nolink|}}\\ 



**Type** = 70 \\ 
{{:html5:statebutton:statebutton_type70.png?nolink|}}\\ 
 


**Type** = 71 \\ 
{{:html5:statebutton:statebutton_type71.png?nolink|}}\\ 
 


**Type** = 72 \\ 
{{:html5:statebutton:statebutton_type72.png?nolink|}}\\ 
 


**Type** = 73 \\ 
{{:html5:statebutton:statebutton_type73.png?nolink|}}\\ 



**Type** = 74 \\ 
{{:html5:statebutton:statebutton_type74.png?nolink|}}\\ 



**Type** = 75 \\ 
{{:html5:statebutton:statebutton_type75.png?nolink|}}\\ 



**Type** = 76 \\ 
{{:html5:statebutton:statebutton_type76.png?nolink|}}\\ 



**Type** = 77 \\ 
{{:html5:statebutton:statebutton_type77.png?nolink|}}\\ 



**Type** = 78 \\ 
{{:html5:statebutton:statebutton_type78.png?nolink|}}\\ 



**Type** = 79 \\ 
{{:html5:statebutton:statebutton_type79.png?nolink|}}\\ 



**Type** = 80 \\ 
{{:html5:statebutton:statebutton_type2.png?nolink|}}\\ 



**Type** = 81 \\ 
{{:html5:statebutton:statebutton_type81.png?nolink|}}\\ 



**Type** = 82 \\ 
{{:html5:statebutton:statebutton_type82.png?nolink|}}\\ 



**Type** = 83 \\ 
{{:html5:statebutton:statebutton_type83.png?nolink|}}\\ 



**Type** = 84 \\ 
{{:html5:statebutton:statebutton_type84.png?nolink|}}\\ 



**Type** = 85 \\ 
{{:html5:statebutton:statebutton_type85.png?nolink|}}\\ 



**Type** = 86 \\ 
{{:html5:statebutton:statebutton_type86.png?nolink|}}\\ 



**Type** = 87 \\ 
{{:html5:statebutton:statebutton_type87.png?nolink|}}\\ 



**Type** = 88 \\ 
{{:html5:statebutton:statebutton_type88.png?nolink|}}\\ 



**Type** = 89 \\ 
{{:html5:statebutton:statebutton_type89.png?nolink|}}\\ 



**Type** = 90 \\ 
{{:html5:statebutton:statebutton_type90.png?nolink|}}\\ 



**Type** = 91 \\ 
Blinking green and blinking red. \\ 
{{:html5:statebutton:statebutton_type91.png?nolink|}}\\ 




**Type** = 92 \\ 
{{:html5:statebutton:statebutton_type92.png?nolink|}}\\ 



**Type** = 93 \\ 
{{:html5:statebutton:statebutton_type93.png?nolink|}}\\ 


###  bNoState

Set to true to have a stateless button. This button will send one event when pressed and one when released. The default is to send

   CLASS1.CONTROL,TurnOn index=0, zone=0, subzone=0

when pressed and

   CLASS1.CONTROL,TurnOff index=0, zone=0, subzone=0 when released. Default = false; 

###  bDisable

Set to true to disable user interactions. Button clicks will no longer work. This is useful if one instead wants to monitor a variable. Default is false. 

###  customupsrc

This is the path to the "up" image for a custom button when type is -1. 

###  customdownsrc

This is the path to the "down" image for a custom button when type is -1.

## Methods

       
### setOnTransmitEvent

Set the event that is sent when a state button is activated or a stateless button is pressed down.

**Usage**

`<code=c>`
setOnTransmitEvent( vscpclass,    // VSCP class 
                       vscptype,   // VSCP type 
                       data,       // Array with databytes 
                       guid)       // GUID to use 
```


##### vscpclass

VSCP class to use for the event. Default=30 CLASS1.CONTROL

##### vscptype

VSCP type to use for the event. Default=5 as for CLASS1.CONTROL TurnOn-Event.

##### data

Array with data bytes to use for the event. Its the responsibility of the caller to use the correct number of data bytes. Default is "0,0,0" meaning index=0, zone=0, subzone=0,

##### guid

This is the sending GUID. Normally one just use the GUID of the sending interface of the VSCP daemon. So the default is 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00 or "-" with is a short hand version for the long form. Default="-"

If you don't want an event to be sent for the on-state set vscpclass=0.   
       
       
### setOffTransmitEvent

Set the event that is sent when a state button is activated or a stateless button is pressed down.

**Usage**
`<code=c>`
setOffTransmitEvent( vscpclass, // VSCP class
                       vscptype, // VSCP type
                       data,     // Array with data bytes
                       guid)     // GUID to use
```

##### vscpclass

VSCP class to use for the event. Default=30 CLASS1.CONTROL 

#####  vscptype 

VSCP type to use for the event. Default=6 as for CLASS1.CONTROL TurnOff. 

##### data

Array with data bytes to use for the event. Its the responsibility of the caller to use the correct number of data bytes. Default is "0,0,0" meaning index=0, zone=0, subzone=0, 

##### guid

This is the sending GUID. Normally one just use the GUID of the sending interface of the VSCP daemon. So the default is 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00 or "-" which is a short hand version for the long form. Default="-"

If you don't want an event to be sent for the off-state set vscpclass=0.

###  setOnReceiveEvent 

Set the event that should be received to set the visual indication of a button to it's on state.

**Usage**
`<code=c>`
setOnReceiveEvent( vscpclass,  // VSCP class 
                     vscptype, // VSCP type 
                     data,     // Array with databytes 
                     guid )    // GUID to use 
``` 
   
##### vscpclass

VSCP class that the incoming event must have. Default=20 CLASS1.INFORMATION 

##### vscptype

VSCP type that the incoming event must have. Default=3 as of the CLASS1.INFORMATION On-Event. 

##### data

Array with data bytes that the event should have. Set a data byte to -1 for a don't care. Default is "-1,0,0" meaning index=-1, zone=0, subzone=0, 

##### guid

This is the GUID the receiving event should have. Just leave undefined if the GUID should not be checked. Default=""   
   
### setOffReceiveEvent

Set the event that should be received to set the visual indication of a button to it's on state.

**Usage** 
`<code=c>`
setOffReceiveEvent( vscpclass,    // VSCP class 
                        vscptype, // VSCP type 
                        data,     // Array with databytes 
                        guid )    // GUID to use 
```  

##### vscpclass

VSCP class that the incoming event must have. Default=20 CLASS1.INFORMATION 

##### vscptype

VSCP type that the incoming event must have. Default=3 as of the CLASS1.INFORMATION Off-Event. 

##### data

Array with data bytes that the event should have. Set a data byte to -1 for a don't care. Default is "-1,0,0" meaning index=-1, zone=0, subzone=0, 

##### guid

This is the GUID the receiving event should have. Just leave undefined if the GUID should not be checked. Default=""


###  setOnTransmitZone 

As it is common to use one of the types in CLASS1.CONTROL as on-event for a button this is a convenient method to use to change the default zone/subzone for the outgoing event.

**Usage**
`<code=c>`
setOnTransmitZone( index,      // index
                     zone,      // zone 
                     subzone )  // subzone  
```

##### Index

0-255

##### zone

Zone to direct event to. 255 is all. 

##### subzone

Subzone to direct event to. 255 is all.   


### setOffTransmitZone

As it is common to use one of the types in CLASS1.CONTROL as off-event for a button this is a convenient method to use to change the default zone/subzone for the outgoing event.

**Usage**
`<code=c>`
setOffTransmitZone( index,   // index
                     zone,    // zone 
                     subzone) // subzone 
```

##### Index

0-255

##### zone

Zone to direct event to. 255 is all. 

##### subzone

Subzone to direct event to. 255 is all.

### setOnReceiveZone

As it is common to use one of the types in CLASS1.INFORMATION as a trigger for the on-state of a button this is a convenient method to use to change the default zone/subzone for the incoming event.

**Usage**
`<code=c>`
setOnReceiveZone( index,   // index
                  zone,    // zone 
                  subzone) // subzone 
```

##### Index

0-255 or -1 for don't care.

##### zone

Zone to direct event to. 255 is all. Or -1 for don't care.

##### subzone

Subzone to direct event to. 255 is all. Or -1 for don't care.

### setOffReceiveZone

As it is common to use one of the types in CLASS1.INFORMATION as a trigger for the off-state of a button this is a convenient method to use to change the default zone/subzone for the incoming event.

**Usage**
`<code=c>`
setOffReceiveZone( index,    // index
                   zone,     // zone 
                   subzone)  // subzone
```

##### Index

0-255 or -1 for don't care.

##### zone

Zone to direct event to. 255 is all. Or -1 for don't care.

##### subzone

Subzone to direct event to. 255 is all. Or -1 for don't care.


###  setTransmitBothZone 

This method setts Transmit On/Off and Receive On/Off index,zone and subzone in one call.

**Usage**
`<code=c>`
setTransmitBothZone ( index,      // index
                       zone,      // zone 
                       subzone )  // subzone  
```

##### Index

0-255.

##### zone

Zone to direct event to. 255 is all.

##### subzone

Subzone to direct event to. 255 is all.


###  setReceiveBothZone 

This method set receive On/Off index,zone and subzone in one call.

**Usage**
`<code=c>`
setReceiveBothZone( index,       // index
                      zone,      // zone 
                      subzone )  // subzone  
```

##### Index

0-255. Set to -1 for don't care.

##### zone

Zone to direct event to. 255 is all. Set to -1 for don't care.

##### subzone

Subzone to direct event to. 255 is all. Set to -1 for don't care.

### setMonitorVariable

With this method one can set a VSCP daemon boolean variable that should be monitored with a specific interval. If the variable is true the state for the state button will be set to true and vice versa. The variable is also updated the other way around If the button is down the variable will be set to true and vice versa. If the variable does not exist it will be created.

**Usage**
`<code=c>`
setMonitorVariable( variablename, // variable name 
                       interval)  // monitoring interval in milliseconds 
```

##### variablename

The name for the VSCP daemon boolean variable. 

##### interval

The interval in milliseconds between variable reads. Set to zero to disable. Default=1000 (one second). To test try to set the variable name to "statbutton1" and issue

    variable write statebutton1,,,true 
and
    variable write statebutton1,,,false
    
in the VSCP daemon TCP/IP interface to see the change the state for the state button in your browser page.

###  draw 

Draw the widget. Normally no need to use.

**Usage** 
`<code=c>`
draw() // Draw the widget
```


### setValue

Set the value for the widget. The state will also be

**Usage**
`<code=c>`
setValue( value,       // Boolean value for state 
            bUpdate )  // True (default) if a draw should be done 
                       // after value is set. 
```

##### value

Boolean value that set the state of the widget. 

##### bUpdate

True (default) if a draw should be done after value is set.

## Using the vscpws_stateButton widget

All vscpws controls have a sample in the **vscp_html5/testws** folder of the
repository [VSCP HTML5 code](http://github.com/grodansparadis/vscp_html5). For the  vscpws_stateButton control this sample is  
[here](http://github.com/grodansparadis/vscp_html5/blob/master/testws/statebutton.html)


\\ 
----
{{  ::copyright.png?600  |}}

`<HTML>``<p style="color:red;text-align:center;">``<a href="http://www.grodansparadis.com">`Grodans Paradis AB`</a>``</p>``</HTML>`
