# vscpws_Variable

The vscpws_Variable widget makes it very easy to work with variables. You can easily check the value, change the value and even create new variables on the VSCP daemon with this widget. 

## Construction of a vscpws_Variable

`<code=c>`
var ev1 = vscpws_Variable( username,           // Username for websocket server  
                           passwordhash,       // Password hash for websocket
                           serverurl,          // url to VSCP websocket i/f
                           variablename,       // The variable to monitor
                           interval,           // Variable check interval in 
                                               // milliseconds
                           fnCallback,         // function to call when value of
                                               // variable is changed or when a
                                               // response of a command is 
                                               // received.
                            bAll )             // Call function for every read, 
                                               // not just changes, if set to 
                                               // true. 
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

### variablename

This is the variable that should be monitored. To be monitored it must obviously exist. But variablename can be set to an empty string or whatever you want if you just want to use the available commands and not want to monitor a variable value.

### interval

This is the interval in milliseconds between checks of the monitored variable.

### fnCallback

Define this callback function to get notifications of a changed variable value or results of the outcome of any of the commands. The callback function should have the following definition
`<code=c>`
var fncallback= function( bResult,      // True or false for command outcome
                             command,   // The command that resulted in this
                             msgarray ) // VSCP websocket message array.
```

For a monitored variable that have a changed value the command will be equal to "READVAR" for the command responses see below.

### bAll

This default to false and does not have to be entered but if set to true not just changes in variables will generate the function callback but the function callback
will be called every time it is checked i.e. with a period set by interval.

## Methods

###  SetInterval

With setinterval it is possible to change the interval by which varaibles are checked and the mintoring can also be turnoff all together by setting the interval to zero.

###  writeVariable

Use writeVariable when you want to set the value for a variable that already exists. The function callback will have command set to **WRITEVAR** when the writing has been performed. 

The **name** should be set to the name of ab existing variable.

The format for the **value** is different for different types of variables and is defined [here](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_variable_persistent_storage_format).

**Usage**
`<code="c">`
vscpws_Variable.writeVariable( name, value )
```

###  createVariable

Add a new variable of the specified type and with the specified value. The function callback will have command set to **CREATEVAR** when the command has been performed. 

The **name** should be unique. 

The **type** of the variable can be set either with a numerical values or as a token (see possible values [here](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_variable_persistent_storage_format). 

The format for the **value** is different for different types of variables and is defined [here](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_variable_persistent_storage_format).

The **persistent** argument defaults to *false* and does not have to be given. If false the variable will not be save over time. If set to true the variables will be saved on disk and be persistent over time until deleted.

**Usage**
`<code="c">`
vscpws_Variable.createVariable( name, type, value, persistence )
```

###  resetVariable

Reset a variable to its default value. The function callback will have command set to **RESETVAR** when the command has been performed. All variables have a default value that is specific to there type as described [here](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_variable_string_write_format&#reset_variable_values).

**Usage**
`<code="c">`
vscpws_Variable.resetVariable( name  )
```

### removeVariable

With this command a variable can be removed (be deleted). The function callback will have command set to **RESETVAR** when the command has been performed.

**Usage**
`<code="c">`
vscpws_Variable.removeVariable( name  )
```

###  lengthVariable

Intended for the string type variable type this gives the length of the variable. The function callback will have command set to **LENGTHVAR** when the command has been performed.

**Usage**
`<code="c">`
vscpws_Variable.lengthVariable( name  )
```

###  lastchangeVariable

This command returns the last date+time the variable was changed. The first time available is when VSCP daemon loads the *variables.xml* file. The function callback will have command set to **LASTCHANGEVAR** when the command  has been performed.

**Usage**
`<code="c">`
vscpws_Variable.lastchangeVariable( name  )
```

###  listVariable

With this command it is possible to list all variables and there values. The function callback will have command set to **LISTVAR** when the command has been performed.

The function callback will be called once for each variable defined. The message array will have the following format

    +;LISTVAR;0;"variable0";type;persistence;value"
    +;LISTVAR;1;"variable1";type;persistence;value"
 1. --
    +;LISTVAR;n;"variablen";type;persistence;value"

**cmd** is set to "LISTVAR" and **result** is set to "true".

**Usage**
`<code="c">`
vscpws_Variable.listVariable()
```

###  saveVariable

This command save persistent varibles to disk (varibles.xml). The function callback will have command set to **SAVEVAR** when the command has been performed.

**Usage**
`<code="c">`
vscpws_Variable.saveVariable()
```

## Using the vscpws_variable widget

All vscpws controls have a sample in the **vscp_html5/testws** folder of the
repository [VSCP HTML5 code](http://github.com/grodansparadis/vscp_html5). For the  vscpws_event control this sample is  
[here](http://github.com/grodansparadis/vscp_html5/blob/master/testws/variable.html).


\\ 
----
{{  ::copyright.png?600  |}}

`<HTML>``<p style="color:red;text-align:center;">``<a href="http://www.grodansparadis.com">`Grodans Paradis AB`</a>``</p>``</HTML>`
