# Write variable

```css
    op=8 or op=WRITEVAR
```  
    
Write/change a remote variable value. If you want to change other parameters then the value of a variable use [create Variable](./rest_interface_createvar.md).

**Requires a valid session parameter**

**General form:**
http://host:port/vscp/rest?
    vscpsession=session-key&
    format=plain|csv|xml|json|jsonp&
    op=8|writevar&
    variable=name&
    value=value

**Arguments:**


*  **op** - Set to 8|writevar

*  **format** - can be 2|xml, 3|json or 4|jsonp (0|plain and 1|csv returns error).

*  **vscpsession** - A valid session key received from the open method.

*  **variable** - Name of variable.

*  **value** - Value for variable.

## HTTP Request with GET

```css
http://demo.vscp.org:8884/vscp/rest?vscpsession=d1c13eb83f52f319f14d167962048521 &format=plain|csv|xml|json|jsonp&op=8|writevar&variable=name&value=value    
```

to test this with **curl** use the following format

```css
curl -X GET "http://host:port/vscp/rest? \
    vscpsession=d1c13eb83f52f319f14d167962048521 & \
    format=plain|csv|xml|json|jsonp& \
    op=8|writevar"
```


## Examples

##### example GET HTTP request

```css
    http://localhost:8884/vscp/rest?  
              vscpsession=d1c13eb83f52f319f14d167962048521&
              format=plain&
              op=8
```  

## Curl example HTTP Request with POST

```css
curl -X POST "http://localhost:8884/vscp/rest" \
    -H "vscpsession: d1c13eb83f52f319f14d167962048521" \ 
    -d "op=writevar&format=plain"     
```

## Demo

There is a a [demo app.](https://github.com/grodansparadis/vscp-ux/tree/master/rest) in the source tree, that demonstrates this functionality using JavaScript.

### JavaScript Request with JSONP

```javascript
//////////////////////////////////////////////////////////////////
// do_writeVariable
//
		
var do_writeVariable = function() {
			
    if ( VscpSessionKey.length > 0 ) {

        var txtVariableName = window.prompt("Name of variable to write:","test");
        var txtVariableValue = window.prompt("New value (BASE64 encoded for string types):","");
				
        $.ajax({
            url: VscpServer + '/vscp/rest?vscpsession=' + VscpSessionKey + 
                    '&format=jsonp&op=writevar&variable=' + txtVariableName +
                    '&value=' + encodeURIComponent( txtVariableValue ),
                    type : "GET",
            jsonpCallback: 'handler',
            cache: true,
            dataType: 'jsonp',
            success: function(response) {
                // response will be a javascript
                // array of objects
                console.log("-----------------------------------------------------------");
                console.log("                   do_writeVariable");
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

	
	1 1 Success 
	
	Everything is fine.


### CSV

	
	success-code,error-code,message,description
	1,1,Success,Success.


### XML

```xml
`<vscp-rest success="true" code="1" message="Success" description="Success."/>`
```

### JSON

```css
{"success":true,"code":1,"message":"success","description":"Success"}
```

### JSONP

```javascript
typeof handler === 'function' && handler({"success":true,"code":1,"message":"success","description":"Success"});
```



[filename](./bottom_copyright.md ':include')
