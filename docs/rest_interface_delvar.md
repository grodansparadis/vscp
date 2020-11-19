# Create variable

```css
    op=10 or op=DELVAR
```  
    
Delete variable. You must own the variable (or be an admin user) to be able to delete it.

**Requires a valid session parameter**

**General form:**

```css
http://host:port/vscp/rest?
    vscpsession=session-key& 
    format=plain|csv|xml|json|jsonp&
    op=10|delvar&
    variable=name   
```

**Arguments:**


*  **op** - Set to 9|createvar

*  **format** - can be 2|xml, 3|json or 4|jsonp (0|plain and 1|csv returns error).

*  **vscpsession** - A valid session key received from the open method.

*  **variable** - Name of variable.
    
## HTTP Request with GET

```css
http://demo.vscp.org:8884/vscp/rest?vscpsession=d1c13eb83f52f319f14d167962048521& 
    format=plain|csv|xml|json|jsonp&
    op=10|delvar&
    variable=name  
```

to test this with **curl** use the following format

```css
curl -X GET "http://host:port/vscp/rest? \
    vscpsession=d1c13eb83f52f319f14d167962048521 & \
    format=plain|csv|xml|json|jsonp& \
    op=10|delvar"
```


## Examples

##### example GET HTTP request

```css
    http://localhost:8884/vscp/rest?  
              vscpsession=d1c13eb83f52f319f14d167962048521&
              format=plain&
              op=10&
              variable=test
```  

## HTTP Request with POST

```css
curl -X POST "http://localhost:8884/vscp/rest" \
    -H "vscpsession: d1c13eb83f52f319f14d167962048521" \ 
    -d "op=listvar&format=plain&variable=test"     
```

## Demo

There is a a [demo app.](https://github.com/grodansparadis/vscp-ux/tree/master/rest) in the source tree, that demonstrates this functionality using JavaScript.

### JavaScript Request with JSONP

```javascript
//////////////////////////////////////////////////////////////////
// do_deleteVariable
//

var do_deleteVariable = function() {
						
    if ( VscpSessionKey.length > 0 ) {

        var txtVariableName = window.prompt("Name of variable to delete:","test");
        if ( null == txtVariableName ) return;			

        $.ajax({
           url: VscpServer + '/vscp/rest?vscpsession=' + VscpSessionKey + 
                         '&format=jsonp&op=deletevar&variable=' + txtVariableName,
           type : "GET",
           jsonpCallback: 'handler',
           cache: true,
           dataType: 'jsonp',
           success: function(response) {
               // response will be a JavaScript
               // array of objects
               console.log("-----------------------------------------------------------");
               console.log("                     do_deleteVariable");
               console.log("-----------------------------------------------------------");
               console.log("Success = " + response.success ); 
               console.log("Code = " + response.code );
               console.log("Message = " + response.message );
               console.log("Description = " + response.description );
               console.log("Info = " + response.info );
                    		
               if ( response.success ) {
                   $("#events").html( "OK Variable deleted" );
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

curl -X POST "http://localhost:8884/vscp/rest" -H "vscpsession: 7fd06788e31bbeb5cd62708d1ecd7897 " -d "op=deletevar&format=plain&variable=test"

	
	1 1 Success 
	
	Everything is fine.


### CSV

curl -X POST "http://localhost:8884/vscp/rest" -H "vscpsession: 7fd06788e31bbeb5cd62708d1ecd7897 " -d "op=deletevar&format=csv&variable=test"

	
	success-code,error-code,message,description
	1,1,Success, Success.


### XML

curl -X POST "http://localhost:8884/vscp/rest" -H "vscpsession: 7fd06788e31bbeb5cd62708d1ecd7897 " at=xml&variable=test"

```xml
<?xml version = "1.0" encoding = "UTF-8" ?>
<vscp-rest success = "true" 
              code = "1" 
              message = "Success" 
              description = "Success." />
```

### JSON

curl -X POST "http://localhost:8884/vscp/rest" -H "vscpsession: 7fd06788e31bbeb5cd62708d1ecd7897 " at=xml&variable=test"

```css
{"success":true,"code":1,"message":"success","description":"Success"}
```

### JSONP

curl -X POST "http://localhost:8884/vscp/rest" -H "vscpsession: 7fd06788e31bbeb5cd62708d1ecd7897 " -d "op=deletevar&format=jsonp&variable=test"

```javascript
typeof handler === 'function' && handler( {"success":true,"code":1,"message":"success","description":"Success"} );
```



[filename](./bottom_copyright.md ':include')
