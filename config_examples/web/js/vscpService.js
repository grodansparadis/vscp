// VSCP service javascript library
// Copyright (c) 2015 Andreas Merkle
// <vscp@blue-andi.de>
//
// Licence:
// The MIT License (MIT)
// [OSI Approved License]
//
// The MIT License (MIT)
//
// Copyright (c) 2012-2016 Grodans Paradis AB (Paradise of the Frog)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Alternative licenses for VSCP & Friends may be arranged by contacting
// Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
//

/*jshint bitwise: false */

/* Create the root namespace and making sure we're not overwriting it */
var vscp = vscp || {};

/* ---------------------------------------------------------------------- */

/**
 * VSCP service supporting functions
 * @namespace vscp.service
 */
vscp._createNS( "vscp.service" );

/** VSCP response timeout in ms
 * @type {number}
 * @const
 */
vscp.service.timeout = 5000;

/**
 * Request a response from all nodes on the communication bus and returns
 * their GUID and MDF URL.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.service.whoIsThere = function( options ) {

    var onError         = null;
    var eventData       = [];
    var eventListener   = null;
    var timerHandle     = null;
    var nodeData        = [];

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    // Event listener to catch all CLASS1.PROTOCOL who is there responses
    eventListener = function( conn, evt ) {

        var index           = 0;
        var seqId           = 0;
        var nodeDataIndex   = 0;
        var nodes           = [];
        var current         = "";
        var nodeId          = 0;
        var guid            = [];
        var mdfUrl          = [];

        if ( "undefined" === typeof evt ) {
            return;
        }

        if ( false === ( evt instanceof vscp.Event ) ) {
            return;
        }

        // Only CLASS1.PROTOCOL events are interesting
        if ( vscp.constants.classes.VSCP_CLASS1_PROTOCOL !== evt.vscpClass ) {
            return;
        }

        // Especially the who is there responses
        if ( vscp.constants.types.VSCP_TYPE_PROTOCOL_WHO_IS_THERE_RESPONSE !== evt.vscpType ) {
            return;
        }

        // Clear timer
        clearTimeout( timerHandle );

        // Store node data
        nodeData.push({
            guid: evt.vscpGuid,
            data: evt.vscpData
        });

        timerHandle = setTimeout(
            function() {
                var interfaceGuid = [];
            
                options.connection.removeEventListener( eventListener );

                // Sort data array for GUID and sequence number
                nodeData.sort( function( a, b ) {

                    // 1. Sort after GUID
                    if ( a.guid < b.guid ) {
                        return -1;
                    }

                    if ( a.guid > b.guid ) {
                        return 1;
                    }

                    // 2. Sort after sequence number
                    if ( a.data[ 0 ] < b.data[ 0 ] ) {
                        return -1;
                    }

                    if ( a.data[ 0 ] > b.data[ 0 ] ) {
                        return 1;
                    }

                    return 0;
                });

                for( nodeDataIndex = 0; nodeDataIndex < nodeData.length; ++nodeDataIndex ) {

                    if ( current !== nodeData[ nodeDataIndex ].guid ) {

                        if ( 0 < nodeDataIndex ) {
                            if ( 7 !== seqId ) {
                                console.warn( vscp.utility.getTime() + " Missing who is there response detected." );
                            }
                        }

                        // Reset
                        seqId = 0;

                        // Next node
                        current = nodeData[ nodeDataIndex ].guid;

                        // Interface GUID
                        interfaceGuid = vscp.utility.strToGuid( nodeData[ nodeDataIndex ].guid );
                        
                        // Get node id
                        nodeId = vscp.utility.getNodeId( nodeData[ nodeDataIndex ].guid );
                    }

                    // Event missing?
                    if ( seqId !== nodeData[ nodeDataIndex ].data[ 0 ] ) {
                        // Throw all other events from this node away until the event comes from another node
                        seqId = -1;
                    }
                    else if ( 0 === nodeData[ nodeDataIndex ].data[ 0 ] ) {
                        guid = [];
                        for( index = 1; index < 8; ++index ) {
                            guid.push( nodeData[ nodeDataIndex ].data[ index ] );
                        }
                    }
                    else if ( 1 === nodeData[ nodeDataIndex ].data[ 0 ] ) {
                        for( index = 1; index < 8; ++index ) {
                            guid.push( nodeData[ nodeDataIndex ].data[ index ] );
                        }
                    }
                    else if ( 2 === nodeData[ nodeDataIndex ].data[ 0 ] ) {
                        for( index = 1; index < 3; ++index ) {
                            guid.push( nodeData[ nodeDataIndex ].data[ index ] );
                        }
                        mdfUrl = [];
                        for( index = 3; index < 8; ++index ) {
                            mdfUrl.push( nodeData[ nodeDataIndex ].data[ index ] );
                        }
                    }
                    else if ( 3 === nodeData[ nodeDataIndex ].data[ 0 ] ) {
                        for( index = 1; index < 8; ++index ) {
                            mdfUrl.push( nodeData[ nodeDataIndex ].data[ index ] );
                        }
                    }
                    else if ( 4 === nodeData[ nodeDataIndex ].data[ 0 ] ) {
                        for( index = 1; index < 8; ++index ) {
                            mdfUrl.push( nodeData[ nodeDataIndex ].data[ index ] );
                        }
                    }
                    else if ( 5 === nodeData[ nodeDataIndex ].data[ 0 ] ) {
                        for( index = 1; index < 8; ++index ) {
                            mdfUrl.push( nodeData[ nodeDataIndex ].data[ index ] );
                        }
                    }
                    else if ( 6 === nodeData[ nodeDataIndex ].data[ 0 ] ) {
                        for( index = 1; index < 7; ++index ) {
                            mdfUrl.push( nodeData[ nodeDataIndex ].data[ index ] );
                        }

                        // Remove all trailing zeros
                        while( 0 === mdfUrl[ mdfUrl.length - 1] ) {
                            mdfUrl.pop();
                        }

                        nodes.push({
                            nodeId: nodeId,
                            interfaceGuid: interfaceGuid,
                            guid: guid,
                            mdfUrl: "http://" + String.fromCharCode.apply( null, mdfUrl )
                        });
                    }

                    if ( 0 <= seqId ) {
                        ++seqId;
                    }
                }

                console.info( vscp.utility.getTime() + " Found " + nodes.length + " nodes." );

                options.onSuccess( nodes );
            },
            vscp.service.timeout
        );
    };

    console.info( vscp.utility.getTime() + " Who is there?" );

    eventData = [
        0xFF    // All nodes shall respond
    ];

    options.connection.sendEvent({

        event: new vscp.Event({
            vscpClass:      vscp.constants.classes.VSCP_CLASS1_PROTOCOL,
            vscpType:       vscp.constants.types.VSCP_TYPE_PROTOCOL_WHO_IS_THERE,
            vscpPriority:   vscp.constants.priorities.PRIORITY_3_NORMAL,
            vscpData:       eventData
        }),

        onSuccess: function( conn ) {
            options.connection.addEventListener( eventListener );

            timerHandle = setTimeout(
                function() {
                    console.info( vscp.utility.getTime() + " Who is there timeout." );

                    options.connection.removeEventListener( eventListener );

                    if ( null !== onError ) {
                        onError();
                    }
                },
                vscp.service.timeout
            );
        },

        onError: function( conn ) {
            console.error( vscp.utility.getTime() + " Who is there failed." );

            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Scan for nodes.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.begin                - Node id where to start scanning
 * @param {number} options.end                  - Node id where to stop scanning
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.service.scan = function( options ) {

    var onError         = null;
    var eventListener   = null;
    var fnProbe         = null;
    var timerHandle     = null;
    var nodes           = [];
    var currentNodeId   = 0;
    var scanTimeout     = 1000;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.begin ) {
        console.error( vscp.utility.getTime() + " begin is missing." );
        return;
    }

    if ( "number" !== typeof options.end ) {
        console.error( vscp.utility.getTime() + " end is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    // Event listener to catch all CLASS1.PROTOCOL probe responses
    eventListener = function( conn, evt ) {

        if ( "undefined" === typeof evt ) {
            return;
        }

        if ( false === ( evt instanceof vscp.Event ) ) {
            return;
        }

        // Only CLASS1.PROTOCOL events are interesting
        if ( vscp.constants.classes.VSCP_CLASS1_PROTOCOL !== evt.vscpClass ) {
            return;
        }

        // Especially the probe acknowledge
        if ( vscp.constants.types.VSCP_TYPE_PROTOCOL_PROBE_ACK !== evt.vscpType ) {
            return;
        }

        // Clear timer
        clearTimeout( timerHandle );

        // Store node GUID
        nodes.push( evt.vscpGuid );

        options.connection.removeEventListener( eventListener );

        if ( options.end > currentNodeId ) {

            ++currentNodeId;

            fnProbe( currentNodeId );
        }
        else {

            // Sort data array for GUID
            nodes.sort();

            console.info( vscp.utility.getTime() + " Found " + nodes.length + " nodes." );

            options.onSuccess( nodes );
        }
    };

    fnProbe = function( nodeId ) {

        options.connection.sendEvent({

            event: new vscp.Event({
                vscpClass:      vscp.constants.classes.VSCP_CLASS1_PROTOCOL,
                vscpType:       vscp.constants.types.VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE,
                vscpPriority:   vscp.constants.priorities.PRIORITY_3_NORMAL,
                vscpData:       [ nodeId ]
            }),

            onSuccess: function( conn ) {

                options.connection.addEventListener( eventListener );

                timerHandle = setTimeout(
                    function() {

                        options.connection.removeEventListener( eventListener );

                        if ( options.end === currentNodeId ) {

                            // Sort data array for GUID
                            nodes.sort();

                            console.info( vscp.utility.getTime() + " Found " + nodes.length + " nodes." );

                            options.onSuccess( nodes );
                        }
                        else {
                            ++currentNodeId;

                            fnProbe( currentNodeId );
                        }

                    },
                    scanTimeout
                );

            },

            onError: function( conn ) {
                console.error( vscp.utility.getTime() + " Scan failed." );

                if ( null !== onError ) {
                    onError();
                }
            }
        });
    };

    console.info( vscp.utility.getTime() + " Scanning starts ..." );

    currentNodeId = options.begin;

    fnProbe( currentNodeId );
};

/**
 * The container is used to store javascript objects in a daemon variable as string.
 * It supports one or more objects in a single variable!
 * @class
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {string} options.name                 - Container name
 */
vscp.service.Container = function( options ) {

    /** VSCP connection
     * @member {vscp.Connection}
     */
    this.connection = null;
    /** Complete container name (prefix + user defined name)
     * @member {string}
     */
    this.name       = "";
    /** Data container itself
     * @member {object[]}
     */
    this.data       = [];
    /** Data element separator
     * @member {object[]}
     */
    this.separator  = ",";

    if ( "undefined" !== typeof options ) {

        if ( true === ( options.connection instanceof vscp.Connection ) ) {
            this.connection = options.connection;
        }

        if ( "string" === typeof options.name ) {
            this.name = "Container_" + options.name;
        }
    }

};

/**
 * Create a container at the daemon.
 *
 * @param {object} options              - Options
 * @param {function} options.onSuccess  - Callback which is called on successful operation
 * @param {function} [options.onError]  - Callback which is called on failed operation
 */
vscp.service.Container.prototype.create = function( options ) {

    var onError     = null;
    var index       = 0;
    var container   = "";

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }
    
    // Build container, which contains all data in string form, separated by semicolon.
    for( index = 0; index < this.data.length; ++index ) {

        // Convert object to string
        container += JSON.stringify( this.data[ index ] );

        if ( this.data.length > ( index + 1 ) ) {
            container += this.separator;
        }
    }

    // Store the container in a variable
    this.connection.createVar({

        name: this.name,

        type: vscp.constants.varTypes.STRING,

        value: container,

        persistency: true,

        onSuccess: function( conn, variable ) {

            options.onSuccess();
        },

        onError: function( conn ) {

            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Write the container to a daemon variable.
 * The container must exist at the daemon!
 *
 * @param {object} options              - Options
 * @param {function} options.onSuccess  - Callback which is called on successful operation
 * @param {function} [options.onError]  - Callback which is called on failed operation
 */
vscp.service.Container.prototype.write = function( options ) {

    var onError     = null;
    var index       = 0;
    var container   = "";

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    // Build container, which contains all data in string form, separated by semicolon.
    for( index = 0; index < this.data.length; ++index ) {

        // Convert object to string
        container += JSON.stringify( this.data[ index ] );

        if ( this.data.length > ( index + 1 ) ) {
            container += this.separator;
        }
    }

    // Store the container in a variable
    this.connection.writeVar({

        name: this.name,

        value: container,

        onSuccess: function( conn, variable ) {

            options.onSuccess();
        },

        onError: function( conn ) {

            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the container from the daemon variable.
 *
 * @param {object} options              - Options
 * @param {function} options.onSuccess  - Callback which is called on successful operation
 * @param {function} [options.onError]  - Callback which is called on failed operation
 */
vscp.service.Container.prototype.read = function( options ) {

    var onError     = null;
    var elements    = null;

    if ( "undefined" !== typeof options ) {

        if ( "function" !== typeof options.onSuccess ) {
            return;
        }

        if ( "function" === typeof options.onError ) {
            onError = options.onError;
        }
    }

    // Read container from variable
    this.connection.readVar({

        name: this.name,

        onSuccess: function( conn, variable ) {

            // Clear data container
            this.data = [];
            
            // Container not empty?
            if ( "" !== variable.value ) {
            
                // Separate data elements
                elements = variable.value.split( this.separator );
                               
                // Convert the strings to objects
                for( index = 0; index < elements.length; ++index ) {
                    this.data.push( JSON.parse( elements[ index ] ) );
                }
            }

            options.onSuccess();
        }.bind( this ),

        onError: function( conn ) {

            if ( null !== onError ) {
                onError();
            }
        }
    });

};
