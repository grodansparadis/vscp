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
    var reconnectTime = RED.settings.socketReconnectTime||10000;
    var socketTimeout = RED.settings.socketTimeout||null;
    var net = require('net');

    var connectionPool = {};

    function receiveVSCP(config) {
        RED.nodes.createNode(this,config);
        this.host = n.host;
        this.port = n.port * 1;
        this.topic = n.topic;
        this.closing = false;
        var node = this;
        var count = 0;
        var client;
        var reconnectTimeout;
        var end = false;
            
        var setupTcpClient = function() {
            node.log("connecting to "+node.host+":"+node.port);
            node.status({fill:"grey",shape:"dot",text:"connecting"});
            var id = (1+Math.random()*4294967295).toString(16);
            
            client = net.connect(node.port, node.host, function() {    
                node.log("connected to "+node.host+":"+node.port);
                node.status({fill:"green",shape:"dot",text:"connected"});
            });
            connectionPool[id] = client;

            // Data received
            client.on('data', function (data) {
                data = data.toString(node.datatype);
                buffer = buffer + data;
                var parts = buffer.split("\n");
                for (var i = 0;i<parts.length-1;i+=1) {
                    var msg = {topic:node.topic, payload:parts[i]};
                    msg._session = {type:"tcp",id:id};
                    node.send(msg);
                }
                
                buffer = parts[parts.length-1];
            } 
            else {
                var msg = {topic:node.topic, payload:data};
                msg._session = {type:"tcp",id:id};
                node.send(msg);
            }

        });
        
        client.on('end', function() {
            if ( buffer.length > 0) ) {
                var msg = {topic:node.topic,payload:buffer};
                msg._session = {type:"tcp",id:id};
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
                    reconnectTimeout = setTimeout( setupTcpClient, 20 );
                }
                else {
                    node.log("connection lost to " + node.host + ":" + node.port);
                    reconnectTimeout = setTimeout( setupTcpClient, reconnectTime );
                }
            }
        });
        
        client.on('error', function(err) {
            node.log(err);
        });
    }
    setupTcpClient();

    this.on('close', function() {
        this.closing = true;
        client.end();
        clearTimeout(reconnectTimeout);
    });
    RED.nodes.registerType("vscp-receive",receiveVSCP);

    function transmitVSCP(n) {
        RED.nodes.createNode(this,n);
        this.host = n.host;
        this.port = n.port * 1;
        this.base64 = n.base64;
        this.doend = n.end || false;
        this.beserver = n.beserver;
        this.name = n.name;
        this.closing = false;
        var node = this;

        if (!node.beserver||node.beserver=="client") {
            var reconnectTimeout;
            var client = null;
            var connected = false;
            var end = false;

            var setupTcpClient = function() {
                node.log("connecting to "+node.host+":"+node.port);
                node.status({fill:"grey",shape:"dot",text:"connecting"});
                client = net.connect(node.port, node.host, function() {
                    connected = true;
                    node.log("connected to "+node.host+":"+node.port);
                    node.status({fill:"green",shape:"dot",text:"connected"});
                });
                client.on('error', function (err) {
                    node.log('error : '+err);
                });
                client.on('end', function (err) {
                });
                client.on('close', function() {
                    node.status({fill:"red",shape:"ring",text:"disconnected"});
                    connected = false;
                    client.destroy();
                    if (!node.closing) {
                        if (end) {
                            end = false;
                            reconnectTimeout = setTimeout(setupTcpClient,20);
                        }
                        else {
                            node.log("connection lost to "+node.host+":"+node.port);
                            reconnectTimeout = setTimeout(setupTcpClient,reconnectTime);
                        }
                    }
                });
            }
            setupTcpClient();

            node.on("input", function(msg) {
                if (connected && msg.payload != null) {
                    if (Buffer.isBuffer(msg.payload)) {
                        client.write(msg.payload);
                    } else if (typeof msg.payload === "string" && node.base64) {
                        client.write(new Buffer(msg.payload,'base64'));
                    } else {
                        client.write(new Buffer(""+msg.payload));
                    }
                    if (node.doend === true) {
                        end = true;
                        client.end();
                    }
                }
            });

            node.on("close", function() {
                this.closing = true;
                client.end();
                clearTimeout(reconnectTimeout);
            });

        } else if (node.beserver == "reply") {
            node.on("input",function(msg) {
                if (msg._session && msg._session.type == "tcp") {
                    var client = connectionPool[msg._session.id];
                    if (client) {
                        if (Buffer.isBuffer(msg.payload)) {
                            client.write(msg.payload);
                        } else if (typeof msg.payload === "string" && node.base64) {
                            client.write(new Buffer(msg.payload,'base64'));
                        } else {
                            client.write(new Buffer(""+msg.payload));
                        }
                    }
                }
            });
        } else {
            var connectedSockets = [];
            node.status({text:"0 connections"});
            var server = net.createServer(function (socket) {
                if (socketTimeout !== null) { socket.setTimeout(socketTimeout); }
                var remoteDetails = socket.remoteAddress+":"+socket.remotePort;
                node.log("connection from "+remoteDetails);
                connectedSockets.push(socket);
                node.status({text:connectedSockets.length+" connections"});
                socket.on('timeout', function() {
                    node.log('timeout closed socket port '+node.port);
                    socket.end();
                });
                socket.on('close',function() {
                    node.log("connection closed from "+remoteDetails);
                    connectedSockets.splice(connectedSockets.indexOf(socket),1);
                    node.status({text:connectedSockets.length+" connections"});
                });
                socket.on('error',function() {
                    node.log("socket error from "+remoteDetails);
                    connectedSockets.splice(connectedSockets.indexOf(socket),1);
                    node.status({text:connectedSockets.length+" connections"});
                });
            });

            node.on("input", function(msg) {
                if (msg.payload != null) {
                    var buffer;
                    if (Buffer.isBuffer(msg.payload)) {
                        buffer = msg.payload;
                    } else if (typeof msg.payload === "string" && node.base64) {
                        buffer = new Buffer(msg.payload,'base64');
                    } else {
                        buffer = new Buffer(""+msg.payload);
                    }
                    for (var i = 0; i<connectedSockets.length;i+=1) {
                        if (node.doend === true) { connectedSockets[i].end(buffer); }
                        else { connectedSockets[i].write(buffer); }
                    }
                }
            });

            server.on('error', function(err) {
                if (err) {
                    node.error('unable to listen on port '+node.port+' : '+err);
                }
            });

            server.listen(node.port, function(err) {
                if (err) {
                    node.error('unable to listen on port '+node.port+' : '+err);
                } else {
                    node.log('listening on port '+node.port);
                    node.on('close', function() {
                        server.close();
                        node.log('stopped listening on port '+node.port);
                    });
                }
            });
        }
    }
    RED.nodes.registerType("vscp-receive",transmitVSCP);

}
