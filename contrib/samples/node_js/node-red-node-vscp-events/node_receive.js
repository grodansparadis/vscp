/**
 * Copyright 2014 Grodans Paradis AB
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

module.exports = function(RED) {
    "use strict";
    
    // State machine
    // 0 = Not connected
    // 1 = Connected - Username sent
    // 2 = Connected - Password sent
    // 3 = Connected - Credentials OK

    var STATE_NOT_CONNECTED = 0;
    var STATE_USERNAME = 1;
    var STATE_PASSWORD = 2;
    var STATE_CONNECTED = 3;

    var reconnectTime = RED.settings.socketReconnectTime || 10000;
    var socketTimeout = RED.settings.socketTimeout||null;
    
    var net = require('net');

    var connectionPool = {};

    function receiveVSCP(config) {
        RED.nodes.createNode(this,config);
        var nState = STATE_NOT_CONNECTED;
        this.host = config.host;
        this.port = config.port * 1;
        this.filter = config.filter;
        this.topic = config.topic;
        this.closing = false;
        var node = this;
        var count = 0;
        var client;
        var reconnectTimeout;
        var end = false;
            
        var setupTcpClient = function() {
            node.log("connecting to " + node.host + ":" + node.port );
            node.status({fill:"grey",shape:"dot",text:"connecting"});
            var id = (1+Math.random()*4294967295).toString(16);
            
            var timeoutConnection = setTimeout( fnTimeout, 3000 );
            
            // Connect to VSCP Daemon
            client = net.connect(node.port, node.host, function() {    
                node.log("connected to " + node.host+  ":" + node.port);
                node.status({fill:"yellow",shape:"dot",text:"connected"});
            });
            connectionPool[id] = client;

            // Data received
            client.on('data', function (data) {
                data = data.toString(node.datatype);
                buffer = buffer + data;
                var parts = buffer.split("\n");
                for (var i = 0;i<parts.length-1;i+=1) {
                    
                    if ( STATE_NOT_CONNECTED == nState ) {
                        if ( ( -1 != strReceiveData.search("\\+OK Welcome to the VSCP daemon") ) &&
                                ( -1 != strReceiveData.search("\\+OK - Success") ) ) {
            
                            if ( bDebug ) node.log('Welcome message received.');
            
                            // OK we have a successful login message
                            client.write('user ' + this.credentials.username + "\r\n" );
            
                            nState = STATE_USERNAME;
                        }
                    }
                    else if ( STATE_USERNAME == nState ) {
                        if ( ( -1 != strReceiveData.search('\\+OK') ) ) {
        
                        if ( bDebug ) node.log('Username accepted.');
        
                        nState = STATE_PASSWORD;
                        client.write('pass ' + this.credentials.password + "\r\n" );
                        }
                        else {
                            nState = STATE_NOT_CONNECTED;
                        }
                    }
                    else if ( STATE_PASSWORD == nState ) {
                        if ( ( -1 != strReceiveData.search('\\+OK') ) ) {
            
                            if ( bDebug ) node.log('Password accepted - we can do work.');
            
                            // OK We are connected
                            nState = STATE_CONNECTED;
                            
                            node.log("Logged in to " + node.host+  ":" + node.port);
                            node.status({fill:"green",shape:"dot",text:"connected"});
            
                            // OK we managed before timeout
                            clearTimeout( global.timeoutConnection );

                            // Send temperature event 
                            if ( bDebug ) node.log('Starting receive loop.');
                            client.write('rcvloop'  + '\r\n');
                        }
                        else {
                            nState = STATE_NOT_CONNECTED;
                        }
                    }
                    else if ( STATE_CONNECTED == nState ) {
                        if ( ( -1 != strReceiveData.search('\\+OK') ) ) {
                            if ( bDebug ) node.log('Command OK');;
                        }
                        else {
                            // This is a received event
                            var offset;
                            var vscpitems = strReceiveData.split(",");
                            var vscphead = parseInt(vscpitems[0]);
                            var vscpclass = parseInt(vscpitems[1]);
                            var vscptype = parseInt(vscpitems[2]);
                            var vscpobid = parseInt(vscpitems[3]);
                            var vscptimestamp = parseInt(vscpitems[4]);
                            var vscpguid = vscpitems[5];
                
                            // Check if we have Level I events over Level II
                            if ( vscpclass >= 512 && vscpclass < 1024 ) {
                                offset = 16;    // Offset into data
                                vscpclass -= 512;
                            }   
        
                            // Get the data
                            var vscpdata = new Array();
                            for (i=0;i<vscpitems.length-6-offset;i++){
                                vscpdata[i] = parseInt(vscpitems[offset+6+i]);
                            }
            
                            if ( bDebug ) {
                                node.log("VSCP event received");
                                node.log("===================");
                                node.log("VSCP Class =" + vscpclass );
                                node.log("VSCP Type =" + vscptype );
                                node.log("VSCP GUID =" + vscpguid );
                                node.log("VSCP Data =" + vscpdata );
                            }
                            
                            var msg = {topic:node.topic, 
                                            payload:{vscp_class:vscpclass,
                                                vscp_type:vscptype,
                                                vscp_guid:vscpguid,
                                                vscp_data:vscpdata,
                                                vscp_head:vscphead,
                                                vscp_obid:vscpobid,
                                                vscp_timestamp:vscptimestamp },
                                                ip:socket.remoteAddress,port:socket.remotePort};
                            msg._session = {type:"vscp",id:id};
                            node.send(msg);
                            
                        }
                    }   
                }            
                buffer = parts[parts.length-1];
            }) 

        };
        
        client.on('end', function() {
            if ( buffer.length > 0) {
                var msg = {topic:node.topic,payload:{}};
                msg._session = {type:"vscp",id:id};
                if (buffer.length !== 0) {
                    end = true; // only ask for fast re-connect if we actually got something
                    node.send(msg);
                }
                buffer = null;
            }
        });
                
        client.on('close', function() {
            delete connectionPool[id];
            node.status({fill:"red",shape:"ring",text:"disconnected"});
            if (!node.closing) {
                // if we were asked to close then try to reconnect once very quick.
                if (end) { 
                    end = false;
                    reconnectTimeout = setTimeout( setupVSCPReceiveClient, 20 );
                }
                else {
                    node.log("connection lost to " + node.host + ":" + node.port);
                    reconnectTimeout = setTimeout( setupVSCPReceiveClient, reconnectTime );
                }
            }
        });
        
        setupVSCPReceiveClient();
        
        client.on('error', function(err) {
            node.log(err);
        });
        
        var fnTimeout = function(){ 
            node.log("Unable to connect/login");
            //client.destroy();
        }
        
    }
    /*
    RED.nodes.registerType("vscp-receive", receiveVSCP() {
        credentials: {
            username: {type:"text"},
            password: {type:"password"}
        }
    });
    */
}
