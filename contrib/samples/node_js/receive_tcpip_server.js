// Example
// =======
// Connect to a remote VSCP daemon and start to receive events from the
// server. We use the "rcvloop" command here to just sit and wait for
// event when they come in. And alternative is of course to use the
// event polling command "retr" to read an event in the input queue and
// the "chkdata" to check if there are events waiting to be read. You can
// read about the commands in the VCSP TCP/IP interface here
// https://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_tcp_ip_protocol_description
//
// Arguments is 'host' 'port' 'user' 'password' 'temperature'
// which defaults to '127.0.0.1' '9598' 'admin' 'secret' '25'
//
// Part of VSCP & Friends - https://www.vscp.org
// akhe@vscp.org, the VSCP project, http://www.paradiseofthefrog.com
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
    console.log("Unable to connect or timeout");
    client.destroy();
}, 6000);

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

    // OK we got data or "+OK" before timeout
    clearTimeout(timeoutConnection);

    var strReceiveData = data.toString();

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
            clearTimeout(timeoutConnection);

            // Send temperature event
            if (bDebug) console.log('Starting receive loop.');
            client.write('rcvloop' + '\r\n');
        }
        else {
            nState = STATE_NOT_CONNECTED;
        }
    }
    else if (STATE_CONNECTED == nState) {

        if (bDebug) console.log('---');

        if ((-1 != strReceiveData.search('\\+OK'))) {
            if (bDebug) console.log('+OK')
        }
        else {
            // This is a received event
            var offset;
            var vscpitems = strReceiveData.split(",");
            var vscphead = parseInt(vscpitems[0]);
            var vscpclass = parseInt(vscpitems[1]);
            var vscptype = parseInt(vscpitems[2]);
            var vscpobid = parseInt(vscpitems[3]);
            var vscpdatetime = vscpitems[4];
            var vscptimestamp = parseInt(vscpitems[5]);
            var vscpguid = vscpitems[6];

            // Check if we have Level I events over Level II
            offset = 0;
            if (vscpclass >= 512 && vscpclass < 1024) {
                offset = 16;    // Offset into data
                vscpclass -= 512;
            }

            // Get the data
            var vscpdata = new Array();
            console.log("length " + vscpitems.length);
            console.log("offset " + offset);
            // Data starts at pos 8 or 8 0 16 for VSCP level I events
            // over level II (offset = 16)
            for (i = 0; i < vscpitems.length - 7 - offset; i++) {
                vscpdata[i] = parseInt(vscpitems[offset + 7 + i]);
            }

            if (bDebug) {
                console.log("VSCP event received");
                console.log("===================");
                console.log("VSCP Class =" + vscpclass);
                console.log("VSCP Type =" + vscptype);
                console.log("VSCP UTC DateTime =" + vscpdatetime);
                console.log("VSCP Timestamp =" + vscptimestamp);
                console.log("VSCP GUID =" + vscpguid);
                console.log("VSCP Data =" + vscpdata);
            }
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
