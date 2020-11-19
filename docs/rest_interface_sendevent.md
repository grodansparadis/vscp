# Send Event

```css
    op=3 or op=SENDEVENT
```  
    
Send a VSCP Event. The event should be given in string form "head,vscp-class,vscp-type,obid,datetime,timestamp,guid,data1,data2,data2..." **Requires a valid session parameter**

**Important note** datetime is introduced in version 1.12.20.0

**General format:**

```css
http://server:port/rest?
    vscpsession=session-key&
    format=plain|csv|xml|json|jsonp&
    op=3|sendevent&
    vscpevent=event-in-string-form
```

Arguments:


*  **op** - Set to 3|sendevent

*  **format** - can be 0|plain, 1|csv, 2|xml, 3|json or 4|jsonp.

*  **vscpsession** - A valid session key received from the open method.

*  **vscpevent** - Event in string form. That is *head,class,type,obid,time-stamp,GUID,data1,data2,data3.... * for example *0,20,3,0,,0,0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15,0,1,35* to send [CLASS1.INFORMATION TYPE=3 ON event](http://docs.vscp.org/spec/latest/#/./class1.information?id=type3-0x03-on), for zone=1, sub-zone=35. Can also be sent as *0,20,3,0,0,-,0,1,35* which will use the GUID of the interface.

## HTTP Request with GET

```css
http://demo.vscp.org:8884/vscp/rest?vscpsession=d1c13eb83f52f319f14d167962048521 &format=plain|csv|xml|json|jsonp&op=3|sendevent&vscpevent=0,10,6,0,,,-,138,0,255    
```

to test this with **curl** use the following format

```css
curl -X GET "http://host:port/vscp/rest? \
    vscpsession=d1c13eb83f52f319f14d167962048521 & \
    format=plain|csv|xml|json|jsonp& \
    op=3|sendevent&vscpevent=0,10,6,0,,,-,138,0,255"
```


## Examples

##### example GET HTTP request

```css
    http://localhost:8884/vscp/rest?  
              vscpsession=d1c13eb83f52f319f14d167962048521&
              format=plain&
              op=3&
              vscpevent=event-in-string-form
```  


##  HTTP Request with POST

```css
curl -X POST "http://localhost:8884/vscp/rest" \
    -H "vscpsession: d1c13eb83f52f319f14d167962048521" \ 
    -d "op=sendevent&format=plain&vscpevent=0,10,6,0,,,-,138,0,255"     
```

## Demo

There is a a [demo app.](https://github.com/grodansparadis/vscp-ux/tree/master/rest) in the source tree, that demonstrates this functionality using JavaScript.

### JavaScript Request with JSONP

```javascript
//////////////////////////////////////////////////////////////////
// do_sendEvent
//
		
var do_sendEvent = function() {
			
    if ( VscpSessionKey.length > 0 ) {	
        $.ajax({
            url: VscpServer + '/vscp/rest?vscpsession=' + VscpSessionKey + '&format=jsonp&op=sendevent&vscpevent=' + txtSendEvent,
            type : "GET",
            jsonpCallback: 'handler',
            cache: true,
            dataType: 'jsonp',
            success: function(response) {
                // response will be a javascript
                // array of objects
                console.log("-----------------------------------------------------------");
                console.log("                         do_sendEvent");
                console.log("-----------------------------------------------------------");
                console.log("Success = " + response.success );
                console.log("Code = " + response.code );
                console.log("Message = " + response.message );
                console.log("Description = " + response.description);
                console.log("Info = " + response.info );
					
                if ( response.success ) {
                    $("#events").html( "event sent" );
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
