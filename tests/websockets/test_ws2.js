/*!
  Example code for node.js and the VSCP daemon ws2 websocket interface
  send 0,30,5,,,0,-,15,14,13,12,11,10,9,8,7,6,5,4,3,2,0,0,1,35
  Code from: https://github.com/websockets/ws

  Copyright 2020 Ake Hedman, Grodans Paradis AB - MIT license
*/

const WebSocket = require('ws');

const ws = new WebSocket('ws://localhost:8884/ws2', {
  perMessageDeflate: false
});

ws.on('open', function open() {

  var cmdauth = {
    "type": "cmd",
    "command": "auth",
    "args": {
       "iv":"5a475c082c80dcdf7f2dfbd976253b24",
       "crypto": "69b1180d2f4809d39be34e19c750107f"
    }
  }

  console.log("\n* * * Logging in as admin user.");
  console.log(JSON.stringify(cmdauth, null, 2 ));
  ws.send(JSON.stringify(cmdauth));


  var cmdnoop = {
    "type": "cmd",
    "command": "noop",
    "args": null
  }

  console.log("\n* * * Sending NOOP command.");
  console.log(JSON.stringify(cmdnoop, null, 2 ));
  ws.send(JSON.stringify(cmdnoop));


  var cmdver = {
    "type": "cmd",
    "command": "version",
    "args": null
  }

  console.log("\n* * * Sending VERSION command.");
  console.log(JSON.stringify(cmdver, null, 2 ));
  ws.send(JSON.stringify(cmdver));


  var cmdcopy = {
    "type": "cmd",
    "command": "copyright",
    "args": null
  }

  console.log("\n* * * Sending COPYRIGHT command.");
  console.log(JSON.stringify(cmdcopy, null, 2 ));
  ws.send(JSON.stringify(cmdcopy));

  var cmdopen = {
    "type": "cmd",
    "command": "open",
    "args": null
  }

  console.log("\n* * * Open communication channel.");
  console.log(JSON.stringify(cmdopen, null, 2 ));
  ws.send(JSON.stringify(cmdopen));

  var sendEvent = {
    "type": "event",
    "event" : {
        "head" : 0,
        "obid": 0,
        "datetime": "2020-01-29T23:05:59Z",
        "timestamp": 0,
        "class": 30,
        "type": 5,
        "guid": "FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:00:02:00:00",
        "data": [1,2,3,4,5,6]
    }
  }

  console.log("\n* * * Send event.");
  console.log(JSON.stringify(sendEvent, null, 2 ));
  ws.send(JSON.stringify(sendEvent));

  // Receive only CLASS1.CONTROL, TurnOn
  var cmdFilter = {
    "type": "cmd",
    "command": "setfilter",
    "args": {                                                                  
        "mask_priority": 0,
        "mask_class": 65535,
        "mask_type": 65535,                                                     
        "mask_guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",                                               
        "filter_priority": 0,                                             
        "filter_class": 30,                                                  
        "filter_type": 5,                                                   
        "filter_guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    }
  }

  console.log("\n* * * Set filter.");
  console.log(JSON.stringify(cmdFilter, null, 2 ));
  ws.send(JSON.stringify(cmdFilter));

  console.log("---------------------------------------------------------");
  console.log("Waiting for events (Abort with ctrl+c)")

  console.log("Will only receive CLASS1.CONTROL, TurnOn now after filter");
  console.log("is set");
  console.log("---------------------------------------------------------");

});
  
ws.on('message', function incoming(data) {
  var reply = JSON.parse(data);
  console.log("\n* * * Response:");  
  console.log(JSON.stringify(reply,null,2) );
});