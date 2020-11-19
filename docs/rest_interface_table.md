
**note** The table functionality is broken in the head version. This is because it is being rewritten at the moment. 


# Read Table

Tables is a feature of the daemon that makes it easy to collect time + value data, typically measurements from a sensor. It is a feature of the decision matrix of the VSCP daemon and is described [here](./decision_matrix.md#write_table). 

```css
    op=11 or op=TABLE
```  
    
Read data from a server defined table. **Requires a valid session parameter**

**General Format**
```css
http://host:port/vscp/rest?
    vscpsession=session-key&
    format=plain|csv|xml|json|jsonp&
    op=11|table&
    name=name-of-table&
    from=from-date-time&
    to=from-date-time  
```

**Arguments:**


*  **op** - Set to 11|gettable

*  **format** - can be 0|plain, 1|csv, 2|xml, 3|json or 4|jsonp.

*  **vscpsession** - A valid session key received from the open method.

*  **name** - Name of table to fetch data from.

*  **from** - Time on the form YY-MM-DD HH:MM:SS to start show values from.

*  **to** - Time on the form YY-MM-DD HH:MM:SS to show values to.

## HTTP Request with GET

```css
http://host:port/vscp/rest?vscpsession=d1c13eb83f52f319f14d167962048521 &format=plain|csv|xml|json|jsonp&op=11|table    
```

to test this with **curl** use the following format

```css
curl -X GET "http://host:port/vscp/rest? \
    vscpsession=d1c13eb83f52f319f14d167962048521 & \
    format=plain|csv|xml|json|jsonp& \
    op=11|table"
```

##### example GET HTTP request

```css
    http://localhost:8884/vscp/rest?  
              vscpsession=d1c13eb83f52f319f14d167962048521&
              format=plain&
              op=11
```  

## HTTP Request with POST

```css
curl -X POST "http://localhost:8884/vscp/rest" \
    -H "vscpsession: d1c13eb83f52f319f14d167962048521" \ 
    -d "op=table&format=plain"     
```

## Demo

There is a a [demo app](https://github.com/grodansparadis/vscp-ux/tree/master/rest) in the source tree, that demonstrates this functionality using JavaScript.

### Javascript Request with JSONP

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
