# Read Event

```css
    op=4 or op=READEVENT
```  

Read one or more VSCP Event(s). 

**Requires a valid session parameter**

**General format:**

```css
http://server:port/rest?
    vscpsession=session-key&
    format=plain|csv|xml|json|jsonp&
    op=4|readevent
    [&count=n]
```

so to read a maximum of four events in plain format use

```css
http://server:port/rest?vscpsession=session-key&
    format=plain&
    op=4&
    count=4
```

**Arguments:**


*  **op** - Set to 4|readevent

*  **format** - Can be 0|plain, 1|csv, 2|xml, 3|json or 4|jsonp.

*  **vscpsession** - A valid session key received from the open method.

*  **count** is an optional parameter and will, if not set, default to one.

## HTTP Request with GET

```css
http://host:port/vscp/rest?vscpsession=d1c13eb83f52f319f14d167962048521 &format=plain|csv|xml|json|jsonp&op=4&readevent[&count=n]    
```

to test this with **curl** use the following format

```css
curl -X GET "http://host:port/vscp/rest? \
    vscpsession=d1c13eb83f52f319f14d167962048521 & \
    format=plain|csv|xml|json|jsonp& \
    op=4|readevent[&count=n]"
```

or against the demo server in your browser

```css
http://demo.vscp.org:8884/vscp/rest?vscpsession=98608c27b1ca33ceb6c7f003688c7095&format=plain&op=readevent&count=2
```

replacing the session key (98608c27b1ca33ceb6c7f003688c7095) with the key you received in the open call.

The result you get should be something like

    1 1 Success 
    2 events requested of 201 available (unfiltered) 2 will be retrieved
    - 96,10,6,1,2003-11-02T12:01:01,1917354301,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x81,0x00,0xE9/r/n
    - 96,10,6,1,Important note datetime is introduced in version 1.12.20.0,2003-11-02T12:01:01,1917354551,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x81,0x00,0xE7/r/n


## Examples

##### example GET HTTP request

```css
    http://localhost:8884/vscp/rest?  
              vscpsession=d1c13eb83f52f319f14d167962048521&
              format=plain&
              op=4
```  


## HTTP Request with POST

```css
curl -X POST "http://localhost:8884/vscp/rest" \
    -H "vscpsession: d1c13eb83f52f319f14d167962048521" \ 
    -d "op=readevent&format=plain&count=5"     
```

## Demo

There is a a [demo app.](https://github.com/grodansparadis/vscp-ux/tree/master/rest) in the source tree, that demonstrates this functionality using JavaScript.

### JavaScript Request with JSONP

```javascript
//////////////////////////////////////////////////////////////////
// do_readEventOne
//
		
var do_readEventOne = function() {
			
    if ( VscpSessionKey.length > 0 ) {	
        $.ajax({
            url: VscpServer + '/vscp/rest?vscpsession=' + VscpSessionKey + '&format=jsonp&op=readevent&count=1',
            type : "GET",
            jsonpCallback: 'handler',
            cache: true,
            dataType: 'jsonp',
            success: function(response) {
                // response will be a JavaScript
                // array of objects
                console.log("-----------------------------------------------------------");
                console.log("                         do_readEventOne");
                console.log("-----------------------------------------------------------");
                console.log("Success = " + response.success );
                console.log("Code = " + response.code );
                console.log("Message = " + response.message );
                console.log("Description = " + response.description );
                console.log("Info = " + response.info );
                console.log("Count = " + response.count );
                console.log("Filtered = " + response.filtered );
                console.log("Errors = " + response.errors );
					
                if (  response.success && ( 1 == response.count ) ) {
                    $("#events").html( printEventData( response.event[0] ) );
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

If you want to read all events in the queue just set count to a very (insanely) high value.

## Responses

**Important note:** The datetime field was introduced in version 1.12.20.0
### Plain

```css
1 1 Success 
10 events requested of 16 available (unfiltered) 10 will be retrieved
- 96,10,6,3,2003-11-02T12:01:01,55055109,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x81,0x01,0x32
- 96,10,6,3,2003-11-02T12:01:01,55057140,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xE5
- 96,10,6,3,2003-11-02T12:01:01,55059156,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xEE
- 96,10,6,3,2003-11-02T12:01:01,55061171,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xD9
- 96,10,6,3,2003-11-02T12:01:01,55063203,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xE4
- 96,10,6,3,2003-11-02T12:01:01,55065234,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xDF
- 96,10,6,3,2003-11-02T12:01:01,55067281,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x81,0x01,0x2E
- 96,20,9,3,2003-11-02T12:01:01,55068312,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x00,0x01,0x02
- 96,10,6,3,2003-11-02T12:01:01,55069343,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xB0
- 96,10,6,3,2003-11-02T12:01:01,55071343,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xAF
```

### CSV

```csv
success-code,error-code,message,description,Event
1,1,Success,Success.,NULL
1,2,Info,10 events requested of 14 available (unfiltered) 10 will be retrieved,NULL
1,4,Count,10,NULL
1,3,Data,Event,96,10,6,3,2003-11-02T12:01:01,55036906,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0A,0x4D
1,3,Data,Event,96,20,9,3,2003-11-02T12:01:01,55037906,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x00,0x01,0x02
1,3,Data,Event,96,10,6,3,2003-11-02T12:01:01,55038906,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0A,0xE3
1,3,Data,Event,96,10,6,3,2003-11-02T12:01:01,55040921,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0x81
1,3,Data,Event,96,10,6,3,2003-11-02T12:01:01,55042937,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0x8D
1,3,Data,Event,96,10,6,3,2003-11-02T12:01:01,55044953,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xAF
1,3,Data,Event,96,10,6,3,2003-11-02T12:01:01,55046968,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xCE
1,3,Data,Event,96,10,6,3,2003-11-02T12:01:01,55049000,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xD9
1,3,Data,Event,96,10,6,3,2003-11-02T12:01:01,55051015,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xEE
1,3,Data,Event,96,10,6,3,2003-11-02T12:01:01,55053062,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x82,0x0B,0xFB
```


*  Row starting with 1,1 is header

*  Row starting with 1,2 is info

*  Row starting with 1,3 is Event

*  Row starting with 1,4 is Event count returned.

### XML

```xml
<vscp-rest success="true" code="1" message="Success" description="Success.">
<info>
10 events requested of 5 available (unfiltered) 5 will be retrieved
</info>
<count>5</count>
<event>
    <head>96</head>
    <vscpclass>10</vscpclass>
    <vscptype>6</type>
    <timestamp>54997984</timestamp>
    <obid>3</obid>
    <guid>FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00</guid>
    <sizedata>4</sizedata>
    <data>0x8A,0x81,0x00,0xCE</data>
    <raw>
        96,10,6,3,2003-11-02T12:01:01,54997984,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x81,0x00,0xCE
    </raw>
</event>

<event>
    <head>96</head>
    <vscpclass>10</vscpclass>
    <vscptype>6</type>
    <obid>3</obid>
    <datetime>2003-11-02T12:01:01</datetime>
    <timestamp>55000015</timestamp>
    <obid>3</obid>
    <guid>FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00</guid>
    <sizedata>4</sizedata>
    <data>0x8A,0x81,0x00,0xD0</data>
    <raw>
        96,10,6,3,2003-11-02T12:01:01,55000015,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x81,0x00,0xD0
    </raw>
</event>

<event>
    <head>96</head>
    <vscpclass>10</vscpclass>
    <vscptype>6</type>
    <obid>3</obid>
    <datetime>2003-11-02T12:01:01</datetime>
    <timestamp>55002078</timestamp>
    <obid>3</obid>`
    <guid>FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00</guid>
    <sizedata>3</sizedata>
    <data0x8A,0x80,0x15</data>
    <raw>
        96,10,6,3,2003-11-02T12:01:01,55002078,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x80,0x15
    </raw>
</event>

<event>
    <head>96</head>
    <vscpclass>10</vscpclass>
    <vscptype>6</type>
    <obid>3</obid>
    <datetime>2003-11-02T12:01:01</datetime>
    <timestamp>55004109</timestamp>
    <obid>3</obid>
    <guid>FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00</guid>
    <sizedata>4</sizedata>
    <data>0x8A,0x81,0x00,0xD5</data>
    <raw>
        96,10,6,3,2003-11-02T12:01:01,55004109,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x81,0x00,0xD5
    </raw>
</event>

<event>
    <head96</head>
    <vscpclass>10</vcpclass>
    <vscptype>6</type>
    <obid>3</obid>
    <datetime>2003-11-02T12:01:01</datetime>
    <timestamp55006125</timestamp>
    <obid>3</obid>
    <guid>FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00</guid>
    <sizedata>4</sizedata>
    <data>0x8A,0x81,0x00,0xD8</data>
    <raw>
        96,10,6,3,2003-11-02T12:01:01,55006125,FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00,0x8A,0x81,0x00,0xD8
    </raw>
</event>

`<filtered>`0`</filtered>`
`<errors>`0`</errors>`

`</vscp-rest>`

```

###  JSON 

```javascript
{
    "success":true,
    "code":1,
    "message":"success",
    "description":"Success",
    "info":"10 events requested of 10 available (unfiltered) 10 will be retrieved",
    "event":   
    [
        { 
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01,"
            "timestamp":58712468,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,204]
        },
        {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58714468,            
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,206]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58716531,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,205]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58718546,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,206,]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58720625,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,208]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58722859,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":3,
            "data":[138,128,21]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58724906,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,213]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58726937,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,216]
      },
      {
           "head":96,
           "vscpclass":20,
           "vscptype":9,
           "datetime":"2003-11-02T12:01:01"
           "timestamp":58728000,
           "obid":3,
           "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
           "sizedata":3,
           "data":[0,1,2]
      },
      {
          "head":96,
          "vscpclass":10,
          "vscptype":6,
          "datetime":"2003-11-02T12:01:01"
          "timestamp":58729046,
          "obid":3,
          "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
          "sizedata":4,
          "data":[138,129,0,215]
       }
   ],
   "count":10,
   "filtered":0,
   "errors":0
}
```

###  JSONP 

```javascript
typeof handler === 'function' && handler(
{
    "success":true,
    "code":1,
    "message":"success",
    "description":"Success",
    "info":"10 events requested of 10 available (unfiltered) 10 will be retrieved",
    "event":   
    [
        { 
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58712468,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,204]
        },
        {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58714468,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,206]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58716531,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,205]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58718546,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,206,]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58720625,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,208]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58722859,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":3,
            "data":[138,128,21]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58724906,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,213]
       },
       {
            "head":96,
            "vscpclass":10,
            "vscptype":6,
            "datetime":"2003-11-02T12:01:01"
            "timestamp":58726937,
            "obid":3,
            "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
            "sizedata":4,
            "data":[138,129,0,216]
      },
      {
           "head":96,
           "vscpclass":20,
           "vscptype":9,
           "datetime":"2003-11-02T12:01:01"
           "timestamp":58728000,
           "obid":3,
           "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
           "sizedata":3,
           "data":[0,1,2]
      },
      {
          "head":96,
          "vscpclass":10,
          "vscptype":6,
          "datetime":"2003-11-02T12:01:01"
          "timestamp":58729046,
          "obid":3,
          "guid":"FF:FF:FF:FF:FF:FF:FF:F7:03:00:00:00:00:00:00:00",
          "sizedata":4,
          "data":[138,129,0,215]
       }
   ],
   "count":10
   "filtered":0,
   "errors":0
} );
```

[filename](./bottom_copyright.md ':include')
