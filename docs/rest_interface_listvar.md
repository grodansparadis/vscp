# List variables

```css
    op=12 or op=LISTVAR
```  
    
List variables selected with a regular expression. 

**Requires a valid session parameter**

**General format:**

```css
http://host:port/vscp/rest?
    vscpsession=session-key&
    format=plain|csv|xml|json|jsonp&
    op=12|listvar&
    [listlong=true|false]
    [regex=regular-expression]   
```

**Arguments:**


*  **op** - Set to 12|listvar

*  **format** - can be 2|xml, 3|json or 4|jsonp (0|plain and 1|csv returns error).

*  **vscpsession** - A valid session key received from the open method.

*  **listlong** - Optional setting that defaults to false. When false the variable list dos not include __value__ and __note__. If set to true __value__ and __note__ is included. The setting should be used with caution as both __value__ and __note__ can be very large. It is often a better alternative to list variables with **listvar** and then use **readvar** to retrieve full data for specific variables of interest.

*  **regex** - Optional regular expression to select variables to list. If not given or empty all variables will be listed.

## HTTP Request with GET

```css
   http://host:port/vscp/rest?vscpsession=sessionkey
      &format=plain|csv|xml|json|jsonp&op=7|listvar[&listlong=true][&regex=`<regular expression>`]   
```

## Examples

### Example HTTP GET request

```css
    http://localhost:8884/vscp/rest?  
              vscpsession=d1c13eb83f52f319f14d167962048521&
              format=plain&
              op=12&
              regex=SIM1
```  

### Example Curl HTTP Request with GET

```css
curl -X GET "http://localhost:8884/vscp/rest?vscpsession=d1c13eb83f52f319f14d167962048521&op=12&format=plain&regex=SIM1"     
```

### Example Curl HTTP Request with POST

```css
curl -X POST "http://localhost:8884/vscp/rest" \
    -H "vscpsession: d1c13eb83f52f319f14d167962048521" \ 
    -d "op=listvar&format=plain&regex=SIM"     
```

## Demo

There is a a [demo app.](https://github.com/grodansparadis/vscp-ux/tree/master/rest) in the source tree, that demonstrates this functionality using JavaScript.

### JavaScript Request with JSONP

```javascript
//////////////////////////////////////////////////////////////////
// printVariableData
//

var printVariableData = function( variable, bLong ) {

    var vartxt = "`<b>`Variable`</b>`: `<b>`Name`</b>`:" + variable.varname +	
       "  `<b>`Type`</b>`:" + variable.vartype + " `<b>`Typecode`</b>`:" + 
       variable.vartypecode + " `<b>`Persistence`</b>`:" + variable.varpersistence +
       " `<b>`User`</b>`:" + variable.varuser + " `<b>`Access-rights`</b>`:" + 
       variable.varaccessright + " `<b>`Last-change`</b>`:" + variable.varlastchange;

    if ( true === bLong ) {
        vartxt += "`<br>``<b>`Value`</b>`:" + variable.varvalue;
        vartxt += "`<br>``<b>`Note`</b>`:" + variable.varnote;
        vartxt += "`<br>`-----------------------------------------------------------------------------";	
    }

    return vartxt;
}

//////////////////////////////////////////////////////////////////
// do_listVariables
//

var do_listVariables = function() {

    if ( VscpSessionKey.length > 0 ) {	

        $("#events").html( "" );
	var txtRegEx = window.prompt("Regular expression (leave blank to select all):","");
        if ( null == txtRegEx ) txtRegEx = "(.*)";

            $.ajax({
                url: VscpServer + '/vscp/rest?vscpsession=' + VscpSessionKey + 
                  '&format=jsonp&op=listvar&listlong=false&regex=' + encodeURIComponent( txtRegEx ),                  
                type : "GET",
                jsonpCallback: 'handler',
                cache: true,
                dataType: 'jsonp',
                success: function(response) {
                    // response will be a JavaScript
                    // array of objects		
                    console.log("-----------------------------------------------------------");
                    console.log("                     do_listVariables");
                    console.log("-----------------------------------------------------------");
                    console.log("Success = " + response.success );
                    console.log("Code = " + response.code );
                    console.log("Message = " + response.message );
                    console.log("Description = " + response.description );
                    console.log("Info = " + response.info );
                    console.log("Count = " + response.count );
                    console.log("Errors = " + response.errors );
                    
                    if ( response.success ) {
                        if ( response.count > 0 ) {
                            var eventtxt = "";
                            for (var i=0; i<response.count; i++ ) {
                                eventtxt += printVariableData( response.variable[i], false ) + "`<br>`";
                            }
			    $("#events").html( eventtxt );
                         }
                         else {
                             $("#events").html( " " );
                             $("#nevents").html( "It looks like there is no variables to read." 
                         );
                     };
                 }
					
             },
             error: function( xhr, status, error ) {
                 console.log( "do_listVariable:" + error + " Status:" + status );
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
curl -X POST "http://localhost:8884/vscp/rest" -H "vscpsession:6c77f7bb681cf4c252984550f0dd3c22" -d "op=listvar&format=plain&regex=SIM"

1 1 Success 
name=SIM1_BLEVEL20 type=2 user=0 access-right=000 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_CODING0 type=3 user=0 access-right=000 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_DECISIONMATRIX0 type=1 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_GUID0 type=8 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_INDEX0 type=3 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_INTERVAL0 type=3 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_MEASUREMENTCLASS0 type=4 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_MEASUREMENTTYPE0 type=4 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_NUMBEROFNODES type=3 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_PATH0 type=1 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_SUBZONE0 type=3 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_UNIT0 type=3 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false
name=SIM1_ZONE0 type=3 user=0 access-right=700 last-change='2016-10-06T18:15:38' persistent=false 
```

##### Response for format=csv

```css
curl -X POST "http://localhost:8884/vscp/rest" -H "vscpsession: 6c77f7bb681cf4c252984550f0dd3c22" -d "op=listvar&format=1&regex=SIM"

success-code,error-code,message,description,Variable
1,1,Success,Success.,NULL
1,2,Info,13 variables found,NULL
1,5,Count,13,NULL
1,3,Data,Variable,SIM1_BLEVEL20;2;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_CODING0;3;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_DECISIONMATRIX0;1;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_GUID0;8;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_INDEX0;3;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_INTERVAL0;3;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_MEASUREMENTCLASS0;4;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_MEASUREMENTTYPE0;4;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_NUMBEROFNODES;3;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_PATH0;1;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_SUBZONE0;3;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_UNIT0;3;0;700;false;2016-10-06T18:15:38
1,3,Data,Variable,SIM1_ZONE0;3;0;700;false;2016-10-06T18:15:38
```

##### response for format=xml

```css
curl -X POST "http://localhost:8884/vscp/rest" -H "vscpsession: 6c77f7bb681cf4c252984550f0dd3c22" -d "op=listvar&format=2&regex=SIM"
```

```xml
<?xml version = "1.0" encoding = "UTF-8" ?>
<vscp-rest success = "true" code = "1" message = "Success" description = "Success." >
<variable name="SIM1_BLEVEL20" 
    typecode="2" 
    type="Boolean" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >

<variable name="SIM1_CODING0" 
    typecode="3" 
    type="Integer" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >

<variable name="SIM1_DECISIONMATRIX0" 
    typecode="1" 
    type="String" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >

<variable name="SIM1_GUID0" 
    typecode="8" 
    type="VscpGuid" 
    user="0" access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >
    
<variable name="SIM1_INDEX0" 
    typecode="3" 
    type="Integer" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >
    
<variable name="SIM1_INTERVAL0" 
    typecode="3" 
    type="Integer" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >
    
<variable name="SIM1_MEASUREMENTCLASS0" 
    typecode="4" 
    type="Long" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >
    
<variable name="SIM1_MEASUREMENTTYPE0" 
    typecode="4" 
    type="Long" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >
    
<variable name="SIM1_NUMBEROFNODES" 
    typecode="3" 
    type="Integer" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >
    
<variable name="SIM1_PATH0" 
    typecode="1" 
    type="String" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >
    
<variable name="SIM1_SUBZONE0" 
    typecode="3" 
    type="Integer" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >
    
<variable name="SIM1_UNIT0" 
    typecode="3" 
    type="Integer" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >
    
<variable name="SIM1_ZONE0" 
    typecode="3" 
    type="Integer" 
    user="0" 
    access-right="700" 
    persistent="false" 
    last-change="2016-10-06T18:15:38" >
    
</variable>

</vscp-rest>
```

##### response for format=json

```css
curl -X POST "http://localhost:8884/vscp/rest"3&regex=SIM"ion: 6c77f7bb681cf4c252984550f0dd3c22" -d "op=listvar&format=2

{"success":true,"code":1,"message":"success","description":"Success","info":"13 variables will be retrieved","variable":
[{"varname":"SIM1_BLEVEL20","vartype":"Boolean","vartypecode":2,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_CODING0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_DECISIONMATRIX0","vartype":"String","vartypecode":1,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_GUID0","vartype":"VscpGuid","vartypecode":8,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_INDEX0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_INTERVAL0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_MEASUREMENTCLASS0","vartype":"Long","vartypecode":4,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_MEASUREMENTTYPE0","vartype":"Long","vartypecode":4,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_NUMBEROFNODES","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_PATH0","vartype":"String","vartypecode":1,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_SUBZONE0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_UNIT0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_ZONE0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":700,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",}],"count":13,"errors":0}

```

##### response for format=jsonp

```javascript
curl -X POST "http://localhost:8884/vscp/rest" -H "vscpsession: 6c77f7bb681cf4c252984550f0dd34&regex=SIM"istvar&format=3

typeof handler === 'function' && 
handler({"success":true,"code":1,"message":"success","description":"Success","info":"13 variables will be retrieved","variable":
[{"varname":"SIM1_BLEVEL20","vartype":"Boolean","vartypecode":2,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_CODING0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_DECISIONMATRIX0","vartype":"String","vartypecode":1,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_GUID0","vartype":"VscpGuid","vartypecode":8,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_INDEX0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_INTERVAL0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_MEASUREMENTCLASS0","vartype":"Long","vartypecode":4,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_MEASUREMENTTYPE0","vartype":"Long","vartypecode":4,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_NUMBEROFNODES","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_PATH0","vartype":"String","vartypecode":1,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_SUBZONE0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_UNIT0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",},
{"varname":"SIM1_ZONE0","vartype":"Integer","vartypecode":3,"varuser":0,"varaccessright":0,"varpersistence":"false","varlastchange":"2016-10-06T18:15:38",}],"count":13,"errors":0});

```


[filename](./bottom_copyright.md ':include')
