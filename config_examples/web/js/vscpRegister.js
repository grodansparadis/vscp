// VSCP register abstraction model javascript library
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
 * VSCP register access functions
 * @namespace vscp.register
 */
vscp._createNS( "vscp.register" );

/** VSCP response timeout in ms
 * @type {number}
 * @const
 */
vscp.register.timeout = 5000;

/** VSCP registers
 * @enum {number}
 */
vscp.register.constants = {

    ALARM_STATUS: 128,
    VSCP_VERSION_MAJOR: 129,
    VSCP_VERSION_MINOR: 130,
    NODE_CONTROL_FLAGS: 131,
    USER_ID_0: 132,
    USER_ID_1: 133,
    USER_ID_2: 134,
    USER_ID_3: 135,
    USER_ID_4: 136,
    MANUFACTURER_DEV_ID_0: 137,
    MANUFACTURER_DEV_ID_1: 138,
    MANUFACTURER_DEV_ID_2: 139,
    MANUFACTURER_DEV_ID_3: 140,
    MANUFACTURER_SUB_DEV_ID_0: 141,
    MANUFACTURER_SUB_DEV_ID_1: 142,
    MANUFACTURER_SUB_DEV_ID_2: 143,
    MANUFACTURER_SUB_DEV_ID_3: 144,
    NICKNAME_ID: 145,
    PAGE_SELECT_MSB: 146,
    PAGE_SELECT_LSB: 147,
    FIRMWARE_VERSION_MAJOR: 148,
    FIRMWARE_VERSION_MINOR: 149,
    FIRMWARE_VERSION_SUB_MINOR: 150,
    BOOT_LOADER_ALGORITHM: 151,
    BUFFER_SIZE: 152,
    PAGES_USED: 153,
    STD_DEV_FAMILY_CODE_3: 154,
    STD_DEV_FAMILY_CODE_2: 155,
    STD_DEV_FAMILY_CODE_1: 156,
    STD_DEV_FAMILY_CODE_0: 157,
    STD_DEV_TYPE_3: 158,
    STD_DEV_TYPE_2: 159,
    STD_DEV_TYPE_1: 160,
    STD_DEV_TYPE_0: 161,
    RESTORE_STD_CFG: 162,
    GUID: 208,
    MDF_URL: 224
};

/**
 * Read one or more register values.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {number} [options.page]               - Register page
 * @param {number} options.offset               - Register page offset
 * @param {number} options.count                - Number of registers to read
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.read = function( options ) {

    var page            = 0;
    var count           = 1;
    var onError         = null;
    var eventData       = [];
    var eventListener   = null;
    var timerHandle     = null;
    var responseData    = [];

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "number" !== typeof options.offset ) {
        console.error( vscp.utility.getTime() + " Register page offset is missing." );
        return;
    }

    if ( "number" === typeof options.offset ) {
        page = options.page;
    }

    if ( "number" === typeof options.count ) {
        if ( ( 0   > options.count ) ||
             ( 256 < options.count ) ) {
            console.error( vscp.utility.getTime() + " Invalid offset." );
            return;
        }

        if ( 256 === options.count ) {
            count = 0;
        }
        else {
            count = options.count;
        }
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    // Event listener to catch all CLASS1.PROTOCOL extended register read responses
    eventListener = function( conn, evt ) {

        var index   = 0;
        var data    = [];

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

        // Especially the extended register read/write responses
        if ( vscp.constants.types.VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE !== evt.vscpType ) {
            return;
        }

        // Clear timer
        clearTimeout( timerHandle );
        
        // More data received, than requested?
        if ( ( evt.vscpData.length - 4 ) > count ) {
            
            console.error( vscp.utility.getTime() + " More data received, than requested." );
            
            if ( null !== onError ) {
                onError();
            }
        }
        else {
        
            // Calculate how many bytes are left
            count = count - ( evt.vscpData.length - 4 );
        
            /* Store response data with index. The index is necessary to sort the responses later.
             * Without sorting the data could be in a wrong order.
             */
            responseData.push({
                index: evt.vscpData[ 0 ],
                data: evt.vscpData.slice( 4 )
            });
            
            // Is register read finished?
            if ( 0 === count ) {
            
                // Order all responses
                responseData.sort( function(a, b) {
                    return a.index - b.index;
                });
                
                // Create one single data container from all responses
                for( index = 0; index < responseData.length; ++index ) {
                    data = data.concat( responseData[ index ].data );
                }
            
                // Finished
                options.connection.removeEventListener( eventListener );
                options.onSuccess( data );
            }
        }
    };

    console.info( vscp.utility.getTime() + " Read " + count + " registers at page " + page + " and offset " + options.offset + " from node " + options.nodeId + "." );

    eventData = [
        options.nodeId,         // Node address
        (page >> 8) & 0x00FF,   // MSB of page where the register is located.
        (page >> 0) & 0x00FF,   // LSB of page where the register is located.
        options.offset,         // Register to read (offset into page).
        count                   // Number of registers to read.
    ];

    options.connection.sendEvent({

        event: new vscp.Event({
            vscpClass:      vscp.constants.classes.VSCP_CLASS1_PROTOCOL,
            vscpType:       vscp.constants.types.VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ,
            vscpPriority:   vscp.constants.priorities.PRIORITY_3_NORMAL,
            vscpData:       eventData
        }),

        onSuccess: function( conn ) {
            options.connection.addEventListener( eventListener );

            timerHandle = setTimeout(
                function() {
                    console.info( vscp.utility.getTime() + " Read register timeout." );

                    options.connection.removeEventListener( eventListener );

                    if ( null !== onError ) {
                        onError();
                    }
                },
                vscp.register.timeout
            );
        },

        onError: function( conn ) {
            console.error( vscp.utility.getTime() + " Reading register failed." );

            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Write one or more register values.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {number} options.page                 - Register page
 * @param {number} options.offset               - Register page offset
 * @param {number[]} options.data               - Data array
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.write = function( options ) {

    var page            = 0;
    var count           = 0;
    var onError         = null;
    var eventData       = [];
    var eventListener   = null;
    var timerHandle     = null;
    var index           = 0;
    var dataIndex       = 0;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "number" !== typeof options.offset ) {
        console.error( vscp.utility.getTime() + " Register page offset is missing." );
        return;
    }

    if ( "number" === typeof options.offset ) {
        page = options.page;
    }

    if ( false === ( options.data instanceof Array ) ) {
        console.error( vscp.utility.getTime() + " Data is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    // Event listener to catch all CLASS1.PROTOCOL extended register write responses
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

        // Especially the extended register read/write responses
        if ( vscp.constants.types.VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE !== evt.vscpType ) {
            return;
        }

        // Clear timer
        clearTimeout( timerHandle );
        timerHandle = null;

        // Is register write finished?
        if ( 0 === count ) {
            options.connection.removeEventListener( eventListener );
            options.onSuccess();
        }
        else {
            console.info( vscp.utility.getTime() + " Write register at page " + page + " and offset " + ( options.offset + dataIndex ) + " to node " + options.nodeId + "." );

            eventData = [
                options.nodeId,             // Node address
                (page >> 8) & 0x00FF,       // MSB of page where the register is located.
                (page >> 0) & 0x00FF,       // LSB of page where the register is located.
                options.offset + dataIndex  // Register to read (offset into page).
            ];

            for( index = 0; index < 4; ++index ) {
                eventData.push( options.data[ dataIndex ] );
                dataIndex++;
                --count;

                if ( 0 === count ) {
                    break;
                }
            }

            options.connection.sendEvent({

                event: new vscp.Event({
                    vscpClass:      vscp.constants.classes.VSCP_CLASS1_PROTOCOL,
                    vscpType:       vscp.constants.types.VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE,
                    vscpPriority:   vscp.constants.priorities.PRIORITY_3_NORMAL,
                    vscpData:       eventData
                }),

                onSuccess: function( conn ) {

                    timerHandle = setTimeout(
                        function() {
                            console.info( vscp.utility.getTime() + " Write register timeout." );

                            options.connection.removeEventListener( eventListener );

                            if ( null !== onError ) {
                                onError();
                            }
                        },
                        vscp.register.timeout
                    );
                },

                onError: function( conn ) {
                    console.error( vscp.utility.getTime() + " Writing register failed." );

                    options.connection.removeEventListener( eventListener );

                    if ( null !== onError ) {
                        onError();
                    }
                }
            });
        }
    };

    count = options.data.length;

    console.info( vscp.utility.getTime() + " Write register at page " + page + " and offset " + options.offset + " to node " + options.nodeId + "." );

    eventData = [
        options.nodeId,         // Node address
        (page >> 8) & 0x00FF,   // MSB of page where the register is located.
        (page >> 0) & 0x00FF,   // LSB of page where the register is located.
        options.offset          // Register to read (offset into page).
    ];

    for( index = 0; index < 4; ++index ) {
        eventData.push( options.data[ dataIndex ] );
        dataIndex++;
        --count;

        if ( 0 === count ) {
            break;
        }
    }

    options.connection.sendEvent({

        event: new vscp.Event({
            vscpClass:      vscp.constants.classes.VSCP_CLASS1_PROTOCOL,
            vscpType:       vscp.constants.types.VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE,
            vscpPriority:   vscp.constants.priorities.PRIORITY_3_NORMAL,
            vscpData:       eventData
        }),

        onSuccess: function( conn ) {
            options.connection.addEventListener( eventListener );

            timerHandle = setTimeout(
                function() {
                    console.info( vscp.utility.getTime() + " Write register timeout." );

                    options.connection.removeEventListener( eventListener );

                    if ( null !== onError ) {
                        onError();
                    }
                },
                vscp.register.timeout
            );
        },

        onError: function( conn ) {
            console.error( vscp.utility.getTime() + " Writing register failed." );

            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Change some bits of a register.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {number} options.page                 - Register page
 * @param {number} options.offset               - Register page offset
 * @param {number} options.pos                  - Bit position
 * @param {number} options.width                - Bit width
 * @param {number[]} options.data               - Data array
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.writeBits = function( options ) {

    var page    = 0;
    var onError = null;
    var index   = 0;
    var width   = 1;
    var value   = 0;
    var mask    = 0;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "number" !== typeof options.offset ) {
        console.error( vscp.utility.getTime() + " Register page offset is missing." );
        return;
    }

    if ( "number" === typeof options.offset ) {
        page = options.page;
    }

    if ( "number" !== typeof options.pos ) {
        console.error( vscp.utility.getTime() + " Bit position is missing." );
        return;
    }

    if ( "number" === typeof options.width ) {
        width = options.width;
    }

    if ( "number" !== typeof options.value ) {
        console.error( vscp.utility.getTime() + " Value is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    // Read register value
    vscp.register.read({
    
        connection: options.connection,

        nodeId: options.nodeId,

        page: page,

        offset: options.offset,

        count: 1,

        onSuccess: function( data ) {
            value = data[ 0 ];

            // Change the bits of the read register value
            mask = 1 << options.pos;
            for( index = 0; index < width; ++index ) {

                // Clear bit?
                if ( 0 === ( options.value & ( 1 << index ) ) ) {

                    value = value & ( ~mask );
                }
                // Set bit
                else {

                    value = value | mask;
                }

                mask = mask << 1;
            }

            // Write changed register value back
            vscp.register.write({
            
                connection: options.connection,

                nodeId: options.nodeId,

                page: page,

                offset: options.offset,

                count: 1,
                
                data: [ value ],

                onSuccess: function() {

                    options.onSuccess();

                }.bind( this ),

                onError: function() {

                    if ( null !== onError ) {
                        onError();
                    }
                }.bind( this )
            });

        }.bind( this ),

        onError: function() {

            if ( null !== onError ) {
                onError();
            }
        }.bind( this )
    });
};

/**
 * Read the alarm status from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readAlarmStatus = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read alarm status from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.ALARM_STATUS,

        count: 1,

        onSuccess: function( data ) {
            options.onSuccess( data );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the supported VSCP version from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readVscpVersion = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read VSCP version from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.VSCP_VERSION_MAJOR,

        count: 2,

        onSuccess: function( data ) {
            options.onSuccess({
                major: data[ 0 ],
                minor: data[ 1 ]
            });
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the node control flags from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readNodeControlFlags = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read node control flags from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.NODE_CONTROL_FLAGS,

        count: 1,

        onSuccess: function( data ) {
            options.onSuccess( data );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the user id from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readUserId = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read user id from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.USER_ID_0,

        count: 5,

        onSuccess: function( data ) {

            var index   = 0;
            var userId  = 0;

            for( index = 0; index < data.length; ++index ) {
                userId *= 256;
                userId += data[ data.length - index - 1 ];
            }

            options.onSuccess( userId );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the manufacturer device id from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readManufacturerDevId = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read manufacturer device id from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.MANUFACTURER_DEV_ID_0,

        count: 4,

        onSuccess: function( data ) {

            var index               = 0;
            var manufacturerDevId   = 0;

            for( index = 0; index < data.length; ++index ) {
                manufacturerDevId *= 256;
                manufacturerDevId += data[ data.length - index - 1 ];
            }

            options.onSuccess( manufacturerDevId );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the manufacturer sub device id from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readManufacturerSubDevId = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read manufacturer sub device id from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.MANUFACTURER_SUB_DEV_ID_0,

        count: 4,

        onSuccess: function( data ) {

            var index                   = 0;
            var manufacturerSubDevId    = 0;

            for( index = 0; index < data.length; ++index ) {
                manufacturerSubDevId *= 256;
                manufacturerSubDevId += data[ data.length - index - 1 ];
            }

            options.onSuccess( manufacturerSubDevId );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the nickname id from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readNicknameId = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read nickname id from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.NICKNAME_ID,

        count: 1,

        onSuccess: function( data ) {
            options.onSuccess( data );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the current selected page from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readSelectedPage = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read selected page from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.PAGE_SELECT_MSB,

        count: 2,

        onSuccess: function( data ) {

            var index   = 0;
            var page    = 0;

            for( index = 0; index < data.length; ++index ) {
                page *= 256;
                page += data[ data.length - index - 1 ];
            }

            options.onSuccess( page );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the firmware version from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readFirmwareVersion = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read firmware version from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.FIRMWARE_VERSION_MAJOR,

        count: 3,

        onSuccess: function( data ) {
            options.onSuccess({
                major: data[ 0 ],
                minor: data[ 1 ],
                subMinor: data[ 2 ]
            });
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the bootloader algorithm from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readBootloaderAlgorithm = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read bootloader algorithm from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.BOOT_LOADER_ALGORITHM,

        count: 1,

        onSuccess: function( data ) {
            options.onSuccess( data );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the number of used pages from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readUsedPages = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read used pages from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.PAGES_USED,

        count: 1,

        onSuccess: function( data ) {
            options.onSuccess( data );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the standard device family code from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readStdDevFamCode = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read standard device family code from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.STD_DEV_FAMILY_CODE_0,

        count: 4,

        onSuccess: function( data ) {
            var index               = 0;
            var stdDevFamilyCode    = 0;

            for( index = 0; index < data.length; ++index ) {
                stdDevFamilyCode *= 256;
                stdDevFamilyCode += data[ data.length - index - 1 ];
            }

            options.onSuccess( stdDevFamilyCode );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the standard device type from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readStdDevType = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read standard device type from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.STD_DEV_TYPE_0,

        count: 4,

        onSuccess: function( data ) {
            var index       = 0;
            var stdDevType  = 0;

            for( index = 0; index < data.length; ++index ) {
                stdDevType *= 256;
                stdDevType += data[ data.length - index - 1 ];
            }

            options.onSuccess( stdDevType );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the GUID from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readGUID = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read GUID from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.GUID,

        count: 16,

        onSuccess: function( data ) {
            options.onSuccess( data );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};

/**
 * Read the MDF URL from a node.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {function} options.onSuccess          - Callback which is called on successful operation
 * @param {function} [options.onError]          - Callback which is called on failed operation
 */
vscp.register.readMdfUrl = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }

    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Read MDF URL from node " + options.nodeId );

    // Read register
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: 0,

        offset: vscp.register.constants.MDF_URL,

        count: 32,

        onSuccess: function( data ) {

            var mdfUrl = "http://";

            // Remove all trailing zeros
            while( 0 === data[ data.length - 1] ) {
                data.pop();
            }

            mdfUrl += String.fromCharCode.apply( null, data );

            options.onSuccess( mdfUrl );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });
};
