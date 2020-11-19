# VSCP Daemon VSCP Websocket Interface

The daemon exports a HTML5 websocket interface from version 0.3.3. This interface makes it possible to have web widgets that are self contained and entirely written in JavaScript which can send and receive VSCP events. This means that you can create a simple web page, place your widgets on it and with or without a stand alone web server have a lightweight user interface. As phone, tablets and other devices generally also support HTML5. That is you have a general way for user interface creation. If you prefer apps. you can compile your interface code using **phonegap** or similar tools.

The websocket server can be reached on port 8080(default) just as the internal web server and will use SSL if the web server is configured to use it. The VSCP & Friends package comes with a few simple test pages.

One important design goal when this interface was designed was to create an interface that also could be implemented on low end devices which needs a user interface. With just a few commands and some simple rules we have managed to do that. This means that we can expect user interfaces interacting with both small devices and larger software units as the VSCP daemon.

If you want to test this there is a simple [walkthrough](./new_system_install_test_ride.md#the_websocket_interface).  The Javascript library and samples can be found on [GitHub](https://github.com/grodansparadis/vscp_html5) 

[filename](./bottom_copyright.md ':include')


