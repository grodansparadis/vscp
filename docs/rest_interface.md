# VSCP Daemon REST Interface

The REST interface is popular among many developers today as it makes it easy to communicate using JavaScript and other web oriented languages over the Internet, and using standard web techniques. The VSCP Daemon has a REST interface that has much of the same functionality of the TCP/IP interface.

The commands that are available are described on this page.

## Where to find the REST interface

The web interface is available at

    http://host:port/vscp/rest

or for a SSL connection

    https://host:port/vscp/rest

for simplicity we only show the non SSL URL's in this documentation.

## Format for HTTP Requests

You can use **GET** or **POST** HTTP requests. They differ a little in format as the **POST** version send authentication information in the header, but in all other senses they are the same. 

### Example of GET HTTP REQUEST

    http://localhost:8884/vscp/rest?vscpuser=admin&vscpsecret=secret&op=open&format=plain

### Example of POST HTTP REQUEST

    curl -X POST "http://host:port/vscp/rest" \
    -H "vscpuser: user" \
    -H "vscpsecret: password" \
    -H "vscpsession: session" \
    -d "vscpuser=admin&vscpsecret=secret&\
        op=open&format=plain"
        
## Life of a REST session

A REST session normally lives between **open** and **close** HTTP requests. If for some reason a rest session is not closed and not accessed within ten minutes the system will regard it as an orphan and close it. To issue a *status* HTTP request from time to time is a good way to hold a session open. But any other of the HTTP requests that need the session value also will do.

## JSONP

If you are using JavaScript in a web browser to retrieve data, then you might be interested in using the JSONP format. JSONP allows you to make to requests from a server from a different domain, which is normally not possible because of the same-origin policy.

Unlike all of the other methods, JSONP responses will always be sent with the HTTP 200 success code. We respond this way for the JSONP format because browsers will not parse the response body when the server replies with a HTTP error code.


[filename](./bottom_copyright.md ':include')
