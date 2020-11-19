# Create variable

```css
    op=9 or op=CREATEVAR
```  
    
Create a remote variable. If the remote variable already exists the supplied parameters is updated in the existing variable.

**Requires a valid session parameter**

**General form:**

```css
http://host:port/vscp/rest?
    vscpsession=session-key& 
    format=plain|csv|xml|json|jsonp&
    op=9|createvar&
    variable=name&
    value=value&
    type=type&
    [persistent=true|false]&
    [accessright=0x777]&
    [note=Optional base64 encoded note about variable.]    
```

**Arguments:**


*  **op** - Set to 9|createvar

*  **format** - can be 2|xml, 3|json or 4|jsonp (0|plain and 1|csv returns error).

*  **vscpsession** - A valid session key received from the open method.

*  **variable** - Name of variable.

*  **value** - Value for variable ([string-types encoded in BASE64](http://www.vscp.org/docs/vscpd/doku.php?id=remote_variables#variable_types)).

*  **note** - Textual note about variable (always encoded in BASE64).

*  **type** - Type of variable to create.

*  **persistent** - Optional variable that should be set to true to make the variable persistent.

*  **accessright** - Acess-rights for this variable (For example "0x777" all rights to everyone). Number can be given in hexadecimal ( preceded by "0x" ) or in decimal. Default is 0x700 (all rights for owner, no rights for everyone else).

The owner of the variable will be the current logged in user.

## HTTP Request with GET

```css
http://demo.vscp.org:8884/vscp/rest?vscpsession=d1c13eb83f52f319f14d167962048521& 
    format=plain|csv|xml|json|jsonp&
    op=9|createvar&
    variable=name&
    value=value&
    type=type&
    [persistent=true|false]    
```


*  **type** is a variable type on [numerical form](./remote_variables#variable_types). Default is string.

*  **persistent** tells if variable should be persistent. That is saved to disk and loaded when the VSCP daemon is restarted. Default = false, not persistent.

to test this with **curl** use the following format

```css
curl -X GET "http://host:port/vscp/rest? \
    vscpsession=d1c13eb83f52f319f14d167962048521 & \
    format=plain|csv|xml|json|jsonp& \
    op=9|createvar"
```


## Examples

##### example GET HTTP request

```css
    http://localhost:8884/vscp/rest?  
              vscpsession=d1c13eb83f52f319f14d167962048521&
              format=plain&
              op=9
```  

## HTTP Request with POST

```css
curl -X POST "http://localhost:8884/vscp/rest" \
    -H "vscpsession: d1c13eb83f52f319f14d167962048521" \ 
    -d "op=createvar&format=plain"     
```

## Demo

There is a a [demo app.](https://github.com/grodansparadis/vscp-ux/tree/master/rest) in the source tree, that demonstrates this functionality using JavaScript.

### JavaScript Request with JSONP

```javascript
//////////////////////////////////////////////////////////////////
// do_createVariable
//
		
var do_createVariable = function() {
						
    if ( VscpSessionKey.length > 0 ) {

        var txtVariableName = window.prompt("Name of variable to create:","test");
        if ( null == txtVariableName ) return;
        var txtVariableType = window.prompt("Type:","string");
        if ( null == txtVariableType ) return;
        var txtVariableValue = window.prompt("Value:","This is a new string");
        if ( null == txtVariableValue ) return;
        var txtVariableNote = window.prompt("Note:","A base64 encoded note about this variable.");
        if ( null == txtVariableNote ) return;
        var txtVariableAccessRight = window.prompt("Access-rights:","777");
        if ( null == txtVariableAccessRight ) return;
        var txtVariablePersistent = window.prompt("Persistent (true/false):","false");
        if ( null == txtVariablePersistent ) return;
			
        // The value should be base64 encoded for 'string-types'
        if ( ( txtVariableType.toUpperCase() == "STRING" ) ||
            ( txtVariableType == "1" ) ||
            ( txtVariableType.toUpperCase() == "BLOB" ) ||
            ( txtVariableType == "16" ) ||
            ( txtVariableType.toUpperCase() == "MIME" ) ||
            ( txtVariableType == "100" ) ||
            ( txtVariableType.toUpperCase() == "HTML" ) ||
            ( txtVariableType == "101" ) ||
            ( txtVariableType.toUpperCase() == "JAVASCRIPT" ) ||
            ( txtVariableType == "102" ) ||
            ( txtVariableType.toUpperCase() == "JSON" ) ||
            ( txtVariableType == "103" ) ||
            ( txtVariableType.toUpperCase() == "XML" ) ||
            ( txtVariableType == "104" ) ||
            ( txtVariableType.toUpperCase() == "SQL" ) ||
            ( txtVariableType == "105" ) ||
            ( txtVariableType.toUpperCase() == "LUA" ) ||
            ( txtVariableType == "201" ) ||
            ( txtVariableType.toUpperCase() == "LUARES" ) ||
            ( txtVariableType == "202" ) ||
            ( txtVariableType.toUpperCase() == "UXTYPE1" ) ||
            ( txtVariableType == "300" )  ) { 
                txtVariableValue = vscp.b64EncodeUnicode( txtVariableValue );
        } 
                				
        $.ajax({
                url: VscpServer + '/vscp/rest?vscpsession=' + VscpSessionKey + 
                           '&format=jsonp&op=createvar&variable=' + txtVariableName +
                           '&value=' + txtVariableValue +
                           '&type=' + txtVariableType +
                           '&note=' + vscp.b64EncodeUnicode( txtVariableNote ) +
                           '&accessright=' + txtVariableAccessRight +
                           '&persistent=' + txtVariablePersistent,
                type : "GET",
                jsonpCallback: 'handler',
                cache: true,
                dataType: 'jsonp',
                success: function(response) {
                   // response will be a javaScript                        
                   // array of objects
                   console.log("-----------------------------------------------------------");
                   console.log("                     do_createVariable");
                   console.log("-----------------------------------------------------------");
                   console.log("Success = " + response.success );
                   console.log("Code = " + response.code );
                   console.log("Message = " + response.message );
                   console.log("Description = " + response.description );
                   console.log("Info = " + response.info );
                   if ( response.success ) {
                       $("#events").html( "OK Variable written" );
                   }					
					
                },
                error: function( xhr, status, error ) {
                    console.log( "Close:" + error + " Status:" + status );
                }
        });
    }
    else {
       alert("Interface is not open!");
    }

};
```

## Responses

### Plain

	
	


### CSV

	
	


### XML

```xml

```

### JSON

```css

```

### JSONP

```javascript

```



[filename](./bottom_copyright.md ':include')
