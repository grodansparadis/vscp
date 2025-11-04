/*!
  Example code for node.js and the VSCP daemon ws2 websocket interface
  send 0,30,5,,,0,-,15,14,13,12,11,10,9,8,7,6,5,4,3,2,0,0,1,35
  Code from: https://github.com/websockets/ws

  Copyright 2020-2025 Ake Hedman, the VSCP project - MIT license
*/

WebSocket = require('ws');

const ws = new WebSocket('ws://localhost:8884/ws1', {
  perMessageDeflate: false
});

ws.on('open', function open() {

  var cmdauth = "C;AUTH;5a475c082c80dcdf7f2dfbd976253b24;69b1180d2f4809d39be34e19c750107f";

  console.log("\n* * * Logging in as admin user.");
  console.log(cmdauth);
  ws.send(cmdauth);


  var cmdnoop = "C;NOOP";

  console.log("\n* * * Sending NOOP command.");
  console.log(cmdnoop);
  ws.send(cmdnoop);


  var cmdopen = "C;OPEN";

  console.log("\n* * * Open communication channel.");
  console.log(cmdopen);
  ws.send(cmdopen);

  // 'E';head,vscp_class,vscp_type,obid,datetime,timestamp,GUID,data
  var sendEvent = "E;0,30,5,0,2020-01-29T23:05:59Z,0,FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:00:02:00:00,1,2,3,4,5,6";

  console.log("\n* * * Send event.");
  console.log(sendEvent);
  ws.send(sendEvent);

  // Receive only CLASS1.CONTROL, TurnOn

  // C;SF;filter-priority, filter-class, 
  //  filter-type, filter-GUID;mask-priority, 
  //  mask-class, mask-type, mask-GUID”
  var cmdFilter = "C;SF;0,30,5,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00;0,0xffff,0xffff,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";

  console.log("\n* * * Set filter.");
  console.log(cmdFilter);
  ws.send(cmdFilter);

  console.log("---------------------------------------------------------");
  console.log("Waiting for events (Abort with ctrl+c)")

  console.log("Will only receive CLASS1.CONTROL, TurnOn now after filter");
  console.log("is set");
  console.log("---------------------------------------------------------");

});
  
ws.on('message', function incoming(data) {
  console.log("\n* * * Response:");  
  console.log(data);
});
