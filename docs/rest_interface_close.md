# Close

```css
    op=2 or op=CLOSE
```

Close a session with the server. **Requires a valid session parameter**

**General format:**
```css
http://host:port/vscp/rest?
    vscpsession=session-key&
    format=plain|csv|xml|json|jsonp&
    op=2|close    
```

**Arguments:**


*  **op** - Set to 2|close

*  **format** - can be 0|plain, 1|csv, 2|xml, 3|json or 4|jsonp.

*  **vscpsession** - A valid session key received from the open method.

## HTTP Request with GET

```css
http://demo.vscp.org:8884/vscp/rest?vscpsession=d1c13eb83f52f319f14d167962048521 &format=plain|csv|xml|json|jsonp&op=2|close    
```

To test this with **curl** use the following format

```css
curl -X GET "http://demo.vscp.org:8884/vscp/rest? \
    vscpsession=d1c13eb83f52f319f14d167962048521 & \
    format=plain|csv|xml|json|jsonp& \
    op=2|close"
```

## Examples

##### example GET HTTP request

```css
    http://localhost:8884/vscp/rest?  
              vscpsession=d1c13eb83f52f319f14d167962048521&
              format=plain&
              op=2
```  

##  HTTP Request with POST

```css
curl -X POST "http://localhost:8884/vscp/rest" \
    -H "vscpsession: d1c13eb83f52f319f14d167962048521" \ 
    -d "op=close&format=plain"     
```

## Demo

There is a [demo app](https://github.com/grodansparadis/vscp-ux/tree/master/rest) in the source tree, that demonstrates this functionality using JavaScript.

### JavaScript Request with JSONP

```javascript
///////////////////////////////////////////////////////////////////
// do_close
//
		
var do_close = function() {
			
    if ( VscpSessionKey.length > 0 ) {	
        $.ajax({
            url: VscpServer + '/vscp/rest?vscpsession=' + VscpSessionKey + '&format=jsonp&op=2',
            type : "GET",
            jsonpCallback: 'handler',
            cache: true,
            dataType: 'jsonp',
            success: function(response) {
                // response will be a JavaScript
		// array of objects
		console.log("-----------------------------------------------------------");
		console.log("                         do_close");
		console.log("-----------------------------------------------------------");
		console.log("Success = " + response.success );
		console.log("Code = " + response.code );
		console.log("Message = " + response.message );
		console.log("Description = " + response.description );
		
	        if (  response.success ) {
                    console.log("Sessionkey cleared"  );
                    VscpSessionKey = "";
                    $("#sessionkey").html("`<b>`Sessionkey`</b>`: " );
                    $("#nevents").html(" " );
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
<vscp-rest success="true" code="1" message="Success" description="Success."/>
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
