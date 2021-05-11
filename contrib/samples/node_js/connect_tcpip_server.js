// Example
// =======
// Connect to a remote VSCP daemon and send a temperature event that defaults to
// 25 degrees Celsius but can be set between -128 and 127.
//
// You can read about all the commands in the VCSP TCP/IP interface here
// https://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_tcp_ip_protocol_description
//
// Arguments is 'host' 'port' 'user' 'password' 'temperature'
// which defaults to '127.0.0.1' '9598' 'admin' 'secret' '25'
//
// Part of VSCP & Friends - https://www.vscp.org
// info@vscp.org, the VSCP project, http://www.paradiseofthefrog.com
//
var net = require('net');

// State machine
// 0 = Not connected
// 1 = Connected - Username sent
// 2 = Connected - Password sent
// 3 = Connected - Credentials OK

var STATE_NOT_CONNECTED = 0;
var STATE_USERNAME = 1;
var STATE_PASSWORD = 2;
var STATE_CONNECTED = 3;
var nState = STATE_NOT_CONNECTED;

// Get arguments
var HOST = (typeof process.argv[2] !== 'undefined') ? process.argv[2] : '127.0.0.1';
var PORT = (typeof process.argv[3] !== 'undefined') ? process.argv[3] : 9598;
var USER = (typeof process.argv[4] !== 'undefined') ? process.argv[4] : "admin";
var PASSWORD = (typeof process.argv[5] !== 'undefined') ? process.argv[5] : "secret";
var TEMPERATURE = (typeof process.argv[6] !== 'undefined') ? process.argv[6] : 25;

var bDebug = true;  // Set to true to get some console logging

// Create a socket
var client = new net.Socket();

// Set timeout
var timeoutConnection = setTimeout(() => {
    console.log("Unable to connect");
    client.destroy();
}, 3000);

// Connect to host
client.connect(PORT, HOST);

///////////////////////////////////////////////////////////////////////////////
// Receive data
//

client.on('connect', function () {
    console.log('Connected to "' + HOST + '" on port "' + PORT + '"');
});


///////////////////////////////////////////////////////////////////////////////
// Receive data
//

client.on('data', function (data) {
    var strReceiveData = data.toString();
    var reg1 = "\+OK Welcome to the VSCP daemon";

    if (bDebug) console.log('DATA: ' + data); // Log received data

    if (STATE_NOT_CONNECTED == nState) {
        if ((-1 != strReceiveData.search("Welcome to the VSCP daemon.")) &&
            (-1 != strReceiveData.search("\\+OK - Success"))) {

            if (bDebug) console.log('Welcome message received.');

            // OK we have a successful login message
            client.write('user ' + USER + "\r\n");

            nState = STATE_USERNAME;
        }
    }
    else if (STATE_USERNAME == nState) {
        if ((-1 != strReceiveData.search('\\+OK'))) {
            if (bDebug) console.log('Username accepted.');
            nState = STATE_PASSWORD;
            client.write('pass ' + PASSWORD + "\r\n");
        }
        else {
            nState = STATE_NOT_CONNECTED;
        }
    }
    else if (STATE_PASSWORD == nState) {
        if ((-1 != strReceiveData.search('\\+OK'))) {

            if (bDebug) console.log('Password accepted - we can do work.');

            // OK We are connected
            nState = STATE_CONNECTED;

            // OK we managed before timeout
            clearTimeout(global.timeoutConnection);

            // Send temperature event
            if (bDebug) console.log('Sending temperature event to VSCP daemon.');
            client.write('send 0,10,6,,,,-,138,0,' + TEMPERATURE + '\r\n');
        }
        else {
            nState = STATE_NOT_CONNECTED;
        }
    }
    else if (STATE_CONNECTED == nState) {
        if ((-1 != strReceiveData.search('\\+OK'))) {
            console.log('Command OK');
            client.destroy();
        }
        else {
            console.log('Command Error');
            client.destroy();
        }
    }
});

///////////////////////////////////////////////////////////////////////////////
// Close
//

client.on('close', function () {
    console.log('Connection closed');
});

///////////////////////////////////////////////////////////////////////////////
// Close
//

client.on('error', function () {
    console.log('Connection error');
});
