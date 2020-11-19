# VSCP REST Protocol Description

This page describes the protocol and HTTP requests available for the VSCP REST protocol.

The format here uses some special characters that may need some extra explanation.


*  **'\'** is a sign that indicates that the current line continues on the next line.

*  **'|'** is **or**. It is often written on the form *type=option|option2|optionm3|option4* and means you can select one of the options.

## HTTP Request Parameters

 | Parameter | Example                                      | description |    
 | --------- | ------- | ----------- |    
 | **vscpuser**    | vscpuser=admin                               | Sets the username |  
 | **vscpsecret**  | vscpsecret=d50c3180375c27927c22e42a379c3f67  | Set password. The password is made up of the md5 hash of *username:auth-domain:realtextpassword*. The **mkpasswd** tool that comes with VSCP & Friends can be used to generate the hash or one can use any of the on-line resources available. |    
| **vscpsession** | vscpsession=d1c13eb83f52f319f14d167962048521 | This is a key that is received from the server after the **open** command. It is used as a parameter for all other parameters to identify the client session and reuse it |    
 | **format**      | format=plain \| csv \| xml \| json \| jsonp   | Sets the format responses should be delivered on. See format table below for possible values. |   
 | **op**          | op=open                                      | Operation to perform. op can have a token or a numerical value. |    

## Supported output formats 

The currently available formats are

 | Format    | Description                                    | 
 | ------    | -----------                                    | 
 | **plain** | Plain text format                              | 
 | **csv**   | Comma Separated Values                         | 
 | **xml**   | XML format                                     | 
 | **json**  | JSON format popular in Javascript              | 
 | **jsonp** | JSONP format popular in Javascript crossdomain | 


## Operations

The available operations currently are

 | operation token  | Code | Description                        | 
 | --------------- | :----: | -----------                        | 
 | [status](rest_interface_status.md)           | 0    | Gives status for session           | 
 | [open](rest_interface_open.md)               | 1    | Open a new session session         | 
 | [close](rest_interface_close.md)             | 2    | Close a session                    | 
 | [sendevent](rest_interface_sendevent.md)     | 3    | Send VSCP Event                    | 
 | [readevent](rest_interface_readevent.md)     | 4    | Read VSCP Event                    | 
 | [setfilter](rest_interface_setfilter.md)     | 5    | Set filter for this session        | 
 | [clearqueue](rest_interface_clearinqueue.md) | 6    | Clear input queue for this session | 
 | [listvar](rest_interface_listvar.md)         | 12   | List variables variable            | 
 | [readvar](rest_interface_readvar.md)         | 7    | Read the value of a variable       | 
 | [writevar](rest_interface_writevar.md)       | 8    | Write the value of a variable      | 
 | [createvar](rest_interface_createvar.md)      | 9    | Create a variable                  | 
 | [measurement](rest_interface_measurement.md) | 10   | Send a measurement                 | 
 | [table](rest_interface_table.md)             | 11   | Read a table                       | 

## Errors

### General - Error = -1

This error is received as a general error message.

### Invalid session - Error = -2

The login session is invalid or have timeout because of inactivity. An Open call is needed before most commands can be used.

##### Error code format=plain

```css
0 -2 Invalid session 

The session must be opened with 'open' before a session command can be used. It may also be possible that the session has timed out.
```

##### Error code format=csv

```css
success-code,error-code,message,description\r\n0,-2,Invalid session,The session must be opened with 'open' before a session command can be used. It may also be possible that the session has timed out.
```


##### Error code format=xml

```xml
<vscp-rest success="false" 
    code="-2" 
    message="Invalid session" 
    description="The session must be opened with 'open' before a session command can be used. It may also be possible that the session has timed out."/>
```

##### Error code format=json

```javascript
{\"success\":false,\"code\":-2,\"message\":\"Invalid session\",\"description\":\"The session must be opened with 'open' before a session command can be used. It may also be possible that the session has timed out.\"}
```

##### Error code format=jsonp

```javascript
typeof handler === 'function' && handler(("{\"success\":false,\"code\":-2,\"message\":\"Invalid session\",\"description\":\"The session must be opened with 'open' before a session command can be used. It may also be possible that the session has timed out.\"});
```

### Unsupported format - Error = -3

This error is received because the format of the call is not valid.

### Unable to create session - Error = -4

The system was unable to create the session.

### Missing data/parameter - Error = -5

Needed data or a parameter to a call is missing. 

### Input queue empty - Error = -6

There is no events to read from the input queue.

### Variable not found - Error = -7

A requested variable can't be found and is probably undefined.

### Variable could not be created - Error = -8

A variable could not be created. This can be a memory or a naming problem.

[filename](./bottom_copyright.md ':include')
