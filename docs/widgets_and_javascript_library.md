# VSCP Daemon Widgets and Javascript library


To make it easier to use the websocket interface of the daemon a number of widgets that makes it easier to create dynamic HTML5 web pages has been developed. Also a general JavaScript library is available that both help in developing and interfacing the VSCP websocket interface of the VSCP daemon.

When you create websocket applications using the internal web server of the VSCP daemon you should always disable the cache for the pages you create with

   `<!-- Don't cache the page -->`
   `<META HTTP-EQUIV="Pragma" CONTENT="no-cache">`
   `<META HTTP-EQUIV="Expires" CONTENT="-1">`

in the `<head>`..`</head>` section. 
### Samples

For samples check the testws folder of the [VSCP HTML5 project at GitHub](https://github.com/grodansparadis/vscp_html5). There is many samples here both with the vscpws library and with other libraries such as [Google Charts](https://developers.google.com/chart/).

 
[filename](./bottom_copyright.md ':include')
