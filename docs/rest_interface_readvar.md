# Read variable

```css
    op=7 or op=READVAR
```  
    
Read the value of a server variable. **Requires a valid session parameter**

**General format:**

```css
http://host:port/vscp/rest?
    vscpsession=session-key&
    format=plain|csv|xml|json|jsonp&
    op=7|readvar&
    variable=variable-name   
```

**Arguments:**


*  **op** - Set to 7|readvar

*  **format** - can be 2|xml, 3|json or 4|jsonp (0|plain and 1|csv returns error).

*  **vscpsession** - A valid session key received from the open method.

*  **variable** - Name of variable.

## HTTP Request with GET

```css
http://host:port/vscp/rest?vscpsession=d1c13eb83f52f319f14d167962048521 &format=plain|csv|xml|json|jsonp&op=7|readvar &variable=`<variable_name>`   
```



to test this with **curl** use the following format

```css
curl -X GET "http://host:port/vscp/rest? \
    vscpsession=d1c13eb83f52f319f14d167962048521 & \
    format=plain|csv|xml|json|jsonp& \
    op=7|readvar \
    variable=`<variable_name>`"
```


## Examples

##### example GET HTTP request

```css
    http://localhost:8884/vscp/rest?  
              vscpsession=d1c13eb83f52f319f14d167962048521&
              format=plain&
              op=7&
              variable=SIM1_ZONE0
```  
## HTTP Request with POST

```css
curl -X POST "http://localhost:8884/vscp/rest" \
    -H "vscpsession: d1c13eb83f52f319f14d167962048521" \ 
    -d "op=readvar&format=plain"     
```
## Demo

There is a a [demo app.](https://github.com/grodansparadis/vscp-ux/tree/master/rest) in the source tree, that demonstrates this functionality using JavaScript.

### JavaScript Request with JSONP

```javascript
//////////////////////////////////////////////////////////////////
// do_readVariable
//
		
var do_readVariable = function() {
			
    if ( VscpSessionKey.length > 0 ) {

        var txtVariableName = window.prompt("Name of variable to read:","test");
				
        $.ajax({
            url: VscpServer + '/vscp/rest?vscpsession=' + VscpSessionKey + 
                 '&format=jsonp&op=readvar&variable=' + txtVariableName,
            type : "GET",
            jsonpCallback: 'handler',
            cache: true,
            dataType: 'jsonp',
            success: function(response) {
                // response will be a javaScript
                // array of objects
                console.log("-----------------------------------------------------------");
                console.log("                     do_readVariable");
                console.log("-----------------------------------------------------------");
                console.log("Success = " + response.success );
                console.log("Code = " + response.code );
                console.log("Message = " + response.message );
                console.log("Description = " + response.description );
                console.log("Info = " + response.info );
                console.log("Variable name = " + response.varname );                
                console.log("Variable type = " + response.vartype );
                console.log("Variable type code = " + response.vartypecode );
                console.log("Variable percistence = " + response.varpersistence );
                console.log("Variable user = " + response.varuser );
                console.log("Variable access-rights = " + response.varaccessright );
                console.log("Variable last-change = " + response.varlastchange );
                console.log("Variable value = " + response.varvalue );
                console.log("Variable note = " + response.varnote );
					
                if ( response.success ) {
                    $("#log").html( response.varname + " = " + response.varvalue + 
                           "`<br>`Type = " + response.vartype + 
                           "`<br>`Typecode = " + response.vartypecode + 
                           "`<br>`Peristence = " + response.varpersistence +
                           "`<br>`User = " + response.varuser +
                           "`<br>`Access-rights = " + response.varaccessright.toString(16) +
                           "`<br>`Last-change = " + response.varlastchange +
                           "`<br>`Note = " + response.varnote );
                }
                else {
                    $("#log").html( "Unable to read variable" );
                }
					
            },
            error: function( xhr, status, error ) {
                console.log( "do_readVariable:" + error + " Status:" + status );
            }
        });
    }
    else {
	alert("Interface is not open!");
    }
};
```

## Responses

**note** Response format has changed from version 1.12.14.12 and now include *user*, *access-rights*, *last-change*. Also all note data is always BASE64 encoded and this is true also for 'string-type' values.

With the format parameter you set the format your want the response represented as. The following shows the positive outcome of the Open HTTP request for all formats available.

##### Response for format=plain

```css
1 1 Success 
name=TEST type=1 user=0 access-rights=777 persistent=true last-change=2016-09-29T11:36:11 value='VGhpcyBpcyBhIHRlc3Q=' note='U3RheSBodW5ncnksIHN0YXkgZm9vbGlzaA=='
```

where 

```javascript
"VGhpcyBpcyBhIHRlc3Q=" = BASE64("This is a test")\\
"U3RheSBodW5ncnksIHN0YXkgZm9vbGlzaA==" = BASE64("Stay hungry, stay foolish") 
```
##### Response for format=csv

```css
success-code,error-code,message,description,Variable,Type,Persistent,Value,Note
1,1,Success,Success.,test,1,0,777,true,2016-09-29T11:36:11'VGhpcyBpcyBhIHRlc3Q=','U3RheSBodW5ncnksIHN0YXkgZm9vbGlzaA=='
```

##### response for format=xml

```xml
<vscp-rest success="true" 
            code="1" 
            message="Success"    
            description="Success.">
    <variable type="1(String)" persistent="true">
        <name>TEST</name>
        <value>VGhpcyBpcyBhIHRlc3Q</value>
        <note>U3RheSBodW5ncnksIHN0YXkgZm9vbGlzaA==</note>
    </variable>
</vscp-rest>
```

##### response for format=json

```css
{"success":true,"code":1,"message":"success","description":"Success","varname":"TEST","vartype":"String","vartypecode":1,"varpersistence":"true","varvalue":"VGhpcyBpcyBhIHRlc3Q","varnote":"U3RheSBodW5ncnksIHN0YXkgZm9vbGlzaA=="}
```

##### response for format=jsonp

```javascript
typeof handler === 'function' && handler({"success":true,"code":1,"message":"success","description":"Success","varname":"TEST","vartype":"String","vartypecode":1,"varpersistence":"true","varvalue":"VGhpcyBpcyBhIHRlc3Q","varnote":"U3RheSBodW5ncnksIHN0YXkgZm9vbGlzaA=="});
```


[filename](./bottom_copyright.md ':include')
