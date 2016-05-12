// VSCP MDF javascript library
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

/** VSCP module description file handling functionality
 * @namespace vscp.mdf
 */
vscp._createNS( "vscp.mdf" );

/** VSCP response timeout in ms
 * @type {number}
 * @const
 */
vscp.mdf.timeout = 5000;

/** MDF specific constants
 * @namespace vscp.mdf.constants
 */
vscp._createNS( "vscp.mdf.constants" );

vscp.mdf.constants = {

    /** MDF type sizes in bytes
     * @enum {number}
     * @const
     */
    TYPE_SIZES: {
        "string": 0,
        "bitfield": 1,
        "bool": 1,
        "int8_t": 1,
        "uint8_t": 1,
        "int16_t": 2,
        "uint16_t": 2,
        "int32_t": 4,
        "uint32_t": 4,
        "int64_t": 8,
        "uint64_t": 8,
        "float": 4,
        "double": 8,
        "date": 4,
        "time": 4,
        "guid": 16,
        "char": 1,
        "byte": 1,
        "short": 2,
        "integer": 2,
        "long": 4
    },

    /** Conversion functions from decimal
     * @const
     */
    CONVERSION_FROM_DECIMAL: {
        "string": function( data ) {
            return String.fromCharCode.apply( null, data );
        },
        "bitfield": function( data ) {
            return data[ 0 ];
        },
        "bool": function( data ) {
            return ( "false" === data[ 0 ] ) ? false : true;
        },
        "int8_t": function( data ) {
            return ( 128 <= data[ 0 ] ) ? ( ( 256 - data[ 0 ] ) * -1 ) : data[ 0 ];
        },
        "uint8_t": function( data ) {
            return data[ 0 ];
        },
        "int16_t": function( data ) {
            var index   = 0;
            var value   = 0;
            var bSign   = false;

            if ( data[ 0 ] & 0x80 ) {
                bSign = true;

                for( index = 0; index < data.length; ++index ) {
                    data[ index ] ^= 0xFF;
                }
            }

            for( index = 0; index < data.length; ++index ) {
                value <<= 8;
                value += data[ index ];
            }

            if ( true === bSign ) {
                value = ( value + 1 ) * -1;
            }

            return value;
        },
        "uint16_t": function( data ) {
            var index = 0;
            var value = 0;

            for( index = 0; index < data.length; ++index ) {
                value <<= 8;
                value += data[ index ];
            }

            return value;
        },
        "int32_t": function( data ) {
            var index   = 0;
            var value   = 0;
            var bSign   = false;

            if ( data[ 0 ] & 0x80 ) {
                bSign = true;

                for( index = 0; index < data.length; ++index ) {
                    data[ index ] ^= 0xFF;
                }
            }

            for( index = 0; index < data.length; ++index ) {
                value <<= 8;
                value += data[ index ];
            }

            if ( true === bSign ) {
                value = ( value + 1 ) * -1;
            }

            return value;
        },
        "uint32_t": function( data ) {
            var index = 0;
            var value = 0;

            for( index = 0; index < data.length; ++index ) {
                value <<= 8;
                value += data[ index ];
            }

            return value;
        },
        "int64_t": function( data ) {
            // Javascript doesn't support signed 64 bit integer.
            // All numbers in javascript are 64 bit "double" precision IEE754 floating point.
            // The largest positive whole number that can therefore be accurately represented is 2^53
            return 0;
        },
        "uint64_t": function( data ) {
            // Javascript doesn't support signed 64 bit integer.
            // All numbers in javascript are 64 bit "double" precision IEE754 floating point.
            // The largest positive whole number that can therefore be accurately represented is 2^53
            return 0;
        },
        "float": function( data ) {
            var signValue       = 1;
            var exponent        = 0;
            var mantissa        = 0;
            var integral        = 0;
            var fractional      = 0;
            var value           = 0.0;
            var index           = 0;
            var mask            = 0;
            var powerOf2        = 1;
            var exponentSign    = false;

            //  1 bit sign
            //  8 bit exponent
            // 23 bit mantissa

            // Get sign
            if ( 0 < ( data[ 0 ] & 0x80 ) ) {
                signValue = -1;
            }

            exponent  = ( data[ 0 ] & 0x7F ) << 1;
            exponent += ( data[ 1 ] & 0x80 ) >> 7;
            exponent -= 127;    // Remove bias

            if ( 0 > exponent ) {
                exponent *= -1;
                exponentSign = true;
            }

            mantissa = data[ 1 ] & 0x7F;
            mantissa *= 256;
            mantissa += data[ 2 ];
            mantissa *= 256;
            mantissa += data[ 3 ];

            fractional = 0;
            mask = 0x00400000 >>> 0;
            powerOf2 = 1;

            if ( false === exponentSign ) {
                integral = 1;
            }
            else {
                integral = 0;
            }

            for( index = 0; index < exponent; ++index) {
                if ( false === exponentSign ) {
                    integral *= 2;  // integral <<= 1;

                    if ( 0 !== ( mantissa & mask ) ) {
                        integral += 1;
                    }

                    mask = Math.trunc( mask / 2 );  // mask >>= 1
                }
                else {
                    powerOf2 *= 2;  // powerOf2 <<= 1;
                }
            }

            if ( true === exponentSign ) {
                index = 0;
                fractional = 1.0 / powerOf2;
            }

            for( index = index; index < 23; ++index ) {
                powerOf2 *= 2;  // powerOf2 <<= 1;

                if ( 0 !== ( mantissa & mask ) ) {
                    fractional += 1.0 / powerOf2;
                }

                mask = Math.trunc( mask / 2 );  // mask >>= 1
            }

            value = signValue * ( integral + fractional );

            return value;
        },
        "double": function( data ) {
            var signValue       = 1;
            var exponent        = 0;
            var mantissaMsb     = 0;
            var mantissaLsb     = 0;
            var integral        = 0;
            var fractional      = 0;
            var value           = 0.0;
            var index           = 0;
            var maskMsb         = 0;
            var maskLsb         = 0;
            var powerOf2        = 1;
            var exponentSign    = false;

            //  1 bit sign
            // 11 bit exponent
            // 52 bit mantissa

            // Get sign
            if ( 0 < ( data[ 0 ] & 0x80 ) ) {
                signValue = -1;
            }

            exponent  = ( data[ 0 ] & 0x7F ) << 4;
            exponent += ( data[ 1 ] & 0xF0 ) >> 4;
            exponent -= 1023;   // Remove bias

            if ( 0 > exponent ) {
                exponent *= -1;
                exponentSign = true;
            }

            mantissaMsb = data[ 1 ] & 0x0F;
            mantissaMsb *= 256;
            mantissaMsb += data[ 2 ];
            mantissaMsb *= 256;
            mantissaMsb += data[ 3 ];

            mantissaLsb += data[ 4 ];
            mantissaLsb *= 256;
            mantissaLsb += data[ 5 ];
            mantissaLsb *= 256;
            mantissaLsb += data[ 6 ];
            mantissaLsb *= 256;
            mantissaLsb += data[ 7 ];

            fractional = 0;
            maskMsb = 0x00080000 >>> 0;
            maskLsb = 0x80000000 >>> 0;
            powerOf2 = 1;

            if ( false === exponentSign ) {
                integral = 1;
            }
            else {
                integral = 0;
            }

            for( index = 0; index < exponent; ++index) {
                if ( false === exponentSign ) {
                    integral *= 2;  // integral <<= 1;

                    if ( 0 < maskMsb ) {
                        if ( 0 !== ( mantissaMsb & maskMsb ) ) {
                            integral += 1;  // integral |= 1;
                        }

                        maskMsb = Math.trunc( maskMsb / 2 );  // maskMsb >>= 1
                    }
                    else {
                        if ( 0 !== ( mantissaLsb & maskLsb ) ) {
                            integral += 1;  // integral |= 1;
                        }

                        maskLsb = Math.trunc( maskLsb / 2 );  // maskLsb >>= 1
                    }
                }
                else {
                    powerOf2 *= 2;
                }
            }

            if ( true === exponentSign ) {
                index = 0;
                fractional = 1.0 / powerOf2;
            }

            for( index = index; index < 52; ++index ) {
                powerOf2 *= 2;  // powerOf2 <<= 1;

                if ( 0 < maskMsb ) {
                    if ( 0 !== ( mantissaMsb & maskMsb ) ) {
                        fractional += 1.0 / powerOf2;
                    }

                    maskMsb = Math.trunc( maskMsb / 2 );  // maskMsb >>= 1
                }
                else {
                    if ( 0 !== ( mantissaLsb & maskLsb ) ) {
                        fractional += 1.0 / powerOf2;
                    }

                    maskLsb = Math.trunc( maskLsb / 2 );  // maskLsb >>= 1
                }
            }

            value = signValue * ( integral + fractional );

            return value;
        },
        "date": function( data ) {
            // yy yy mm dd
            var day     = data[ 3 ];
            var month   = data[ 2 ];
            var year    = data[ 0 ] * 256 + data[ 1 ];

            return "" + day + "-" + month + "-" + year;
        },
        "time": function( data ) {
            // hh mm ss
            var hours   = data[ 0 ];
            var minutes = data[ 1 ];
            var seconds = data[ 2 ];

            return "" + hours + ":" + minutes + ":" + seconds;
        },
        "guid": function( data ) {
            return data;
        },
        "char": function( data ) {
            return vscp.mdf.constants.CONVERSION_FROM_DECIMAL[ "int8_t" ]( data );
        },
        "byte": function( data ) {
            return vscp.mdf.constants.CONVERSION_FROM_DECIMAL[ "int8_t" ]( data );
        },
        "short": function( data ) {
            return vscp.mdf.constants.CONVERSION_FROM_DECIMAL[ "int16_t" ]( data );
        },
        "integer": function( data ) {
            return vscp.mdf.constants.CONVERSION_FROM_DECIMAL[ "int16_t" ]( data );
        },
        "long": function( data ) {
            return vscp.mdf.constants.CONVERSION_FROM_DECIMAL[ "int32_t" ]( data );
        }
    },

    /** Conversion functions to decimal
     * @const
     */
    CONVERSION_TO_DECIMAL: {
        "string": function( data ) {
            var dataArray   = [];
            var index       = 0;

            for( index = 0; index < data.length; ++index ) {
                dataArray.push( data[ index ] );
            }

            return dataArray;
        },
        "bitfield": function( data ) {
            return [ data ];
        },
        "bool": function( data ) {
            var dataArray   = [];

            if ( false === data ) {
                dataArray.push( 0 );
            }
            else {
                dataArray.push( 1 );
            }

            return dataArray;
        },
        "int8_t": function( data ) {
            var dataArray   = [];

            // Check for limits
            if ( vscp.mdf.constants.RANGE[ "int8_t" ].MAX < data ) {
                data = vscp.mdf.constants.RANGE[ "int8_t" ].MAX;
            }
            else if ( vscp.mdf.constants.RANGE[ "int8_t" ].MIN > data ) {
                data = vscp.mdf.constants.RANGE[ "int8_t" ].MIN;
            }

            if ( 0 > data ) {
                dataArray.push( 256 - ( data * -1 ) );
            }
            else {
                dataArray.push( data );
            }

            return dataArray;
        },
        "uint8_t": function( data ) {
            var dataArray   = [];

            // Check for limits
            if ( vscp.mdf.constants.RANGE[ "uint8_t" ].MAX < data ) {
                data = vscp.mdf.constants.RANGE[ "uint8_t" ].MAX;
            }
            else if ( vscp.mdf.constants.RANGE[ "uint8_t" ].MIN > data ) {
                data = vscp.mdf.constants.RANGE[ "uint8_t" ].MIN;
            }

            dataArray.push( data );

            return dataArray;
        },
        "int16_t": function( data ) {
            var dataArray   = [];

            // Check for limits
            if ( vscp.mdf.constants.RANGE[ "int16_t" ].MAX < data ) {
                data = vscp.mdf.constants.RANGE[ "int16_t" ].MAX;
            }
            else if ( vscp.mdf.constants.RANGE[ "int16_t" ].MIN > data ) {
                data = vscp.mdf.constants.RANGE[ "int16_t" ].MIN;
            }

            if ( 0 > data ) {
                data = 65536 - ( data * -1 );
            }

            dataArray.push( ( data >> 8 ) & 0xFF );
            dataArray.push( ( data >> 0 ) & 0xFF );

            return dataArray;
        },
        "uint16_t": function( data ) {
            var dataArray   = [];

            // Check for limits
            if ( vscp.mdf.constants.RANGE[ "uint16_t" ].MAX < data ) {
                data = vscp.mdf.constants.RANGE[ "uint16_t" ].MAX;
            }
            else if ( vscp.mdf.constants.RANGE[ "uint16_t" ].MIN > data ) {
                data = vscp.mdf.constants.RANGE[ "uint16_t" ].MIN;
            }

            dataArray.push( ( data >> 8 ) & 0xFF );
            dataArray.push( ( data >> 0 ) & 0xFF );

            return dataArray;
        },
        "int32_t": function( data ) {
            var dataArray   = [];

            // Check for limits
            if ( vscp.mdf.constants.RANGE[ "int32_t" ].MAX < data ) {
                data = vscp.mdf.constants.RANGE[ "int32_t" ].MAX;
            }
            else if ( vscp.mdf.constants.RANGE[ "int32_t" ].MIN > data ) {
                data = vscp.mdf.constants.RANGE[ "int32_t" ].MIN;
            }

            if ( 0 > data ) {
                data = 4294967296 - ( data * -1 );
            }

            dataArray.push( ( data >> 24 ) & 0xFF );
            dataArray.push( ( data >> 16 ) & 0xFF );
            dataArray.push( ( data >>  8 ) & 0xFF );
            dataArray.push( ( data >>  0 ) & 0xFF );

            return dataArray;
        },
        "uint32_t": function( data ) {
            var dataArray   = [];

            // Check for limits
            if ( vscp.mdf.constants.RANGE[ "uint32_t" ].MAX < data ) {
                data = vscp.mdf.constants.RANGE[ "uint32_t" ].MAX;
            }
            else if ( vscp.mdf.constants.RANGE[ "uint32_t" ].MIN > data ) {
                data = vscp.mdf.constants.RANGE[ "uint32_t" ].MIN;
            }

            dataArray.push( ( data >> 24 ) & 0xFF );
            dataArray.push( ( data >> 16 ) & 0xFF );
            dataArray.push( ( data >>  8 ) & 0xFF );
            dataArray.push( ( data >>  0 ) & 0xFF );

            return dataArray;
        },
        "int64_t": function( data ) {
            // Javascript doesn't support signed 64 bit integer.
            // All numbers in javascript are 64 bit "double" precision IEE754 floating point.
            // The largest positive whole number that can therefore be accurately represented is 2^53
            return [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ];
        },
        "uint64_t": function( data ) {
            // Javascript doesn't support signed 64 bit integer.
            // All numbers in javascript are 64 bit "double" precision IEE754 floating point.
            // The largest positive whole number that can therefore be accurately represented is 2^53
            return [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ];
        },
        "float": function( data ) {

            //  1 bit sign
            //  8 bit exponent
            // 23 bit mantissa

            // TODO

            return [ 0, 0, 0, 0, 0, 0, 0, 0 ];
        },
        "double": function( data ) {

            //  1 bit sign
            // 11 bit exponent
            // 52 bit mantissa

            // TODO

            return [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ];
        },
        "date": function( data ) {
            var dataArray   = [];
            var items       = data.split( "-" );

            // yy yy mm dd
            var day     = parseInt( items[ 0 ] );
            var month   = parseInt( items[ 1 ] );
            var year    = parseInt( items[ 2 ] );

            dataArray.push( ( year >> 8 ) & 0xFF );
            dataArray.push( ( year >> 0 ) & 0xFF );
            dataArray.push( month & 0xFF );
            dataArray.push( day & 0xFF );

            return dataArray;
        },
        "time": function( data ) {
            var dataArray   = [];
            var items       = data.split( ":" );

            // hh mm ss
            var hours   = parseInt( items[ 0 ] );
            var minutes = parseInt( items[ 1 ] );
            var seconds = parseInt( items[ 2 ] );

            dataArray.push( hours & 0xFF );
            dataArray.push( minutes & 0xFF );
            dataArray.push( seconds & 0xFF );

            return dataArray;
        },
        "guid": function( data ) {
            return data;
        },
        "char": function( data ) {
            return vscp.mdf.constants.CONVERSION_TO_DECIMAL[ "int8_t" ]( data );
        },
        "byte": function( data ) {
            return vscp.mdf.constants.CONVERSION_TO_DECIMAL[ "int8_t" ]( data );
        },
        "short": function( data ) {
            return vscp.mdf.constants.CONVERSION_TO_DECIMAL[ "int16_t" ]( data );
        },
        "integer": function( data ) {
            return vscp.mdf.constants.CONVERSION_TO_DECIMAL[ "int16_t" ]( data );
        },
        "long": function( data ) {
            return vscp.mdf.constants.CONVERSION_TO_DECIMAL[ "int32_t" ]( data );
        }
    },

    /** Type ranges
     * @const
     */
    RANGE: {
        "int8_t": {
            MIN: -128,
            MAX: 127
        },
        "uint8_t": {
            MIN: 0,
            MAX: 255
        },
        "int16_t": {
            MIN: -32768,
            MAX: 32767
        },
        "uint16_t": {
            MIN: 0,
            MAX: 65535
        },
        "int32_t": {
            MIN: -2147483648,
            MAX: 2147483647
        },
        "uint32_t": {
            MIN: 0,
            MAX: 4294967295
        },
        // Not supported
        "int64_t": {
            MIN: 0,
            MAX: 0
        },
        // Not supported
        "uint64_t": {
            MIN: 0,
            MAX: 0
        },
        "float": {
            MIN: 0,
            MAX: 0
        },
        "double": {
            MIN: 0,
            MAX: 0
        },
        "char": {
            MIN: -128,
            MAX: 127
        },
        "byte": {
            MIN: -128,
            MAX: 127
        },
        "short": {
            MIN: -32767,
            MAX: 32766
        },
        "integer": {
            MIN: -32767,
            MAX: 32766
        },
        "long": {
            MIN: -2147483648,
            MAX: 2147483647
        }
    }

};

/**
 * Get the MDF as xml document from a URL.
 *
 * @param {object} options              - Options
 * @param {function} options.url        - URL to MDF file
 * @param {function} options.onSuccess  - Function which is called on a successful operation
 * @param {function} [options.onError]  - Function which is called on a failed operation
 */
vscp.mdf.load = function( options ) {

    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( "string" !== typeof options.url ) {
        console.error( vscp.utility.getTime() + " MDF URL is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Load MDF from URL " + options.url );

    $.ajax({

        // The 'type' property sets the HTTP method.
        // A value of 'PUT' or 'DELETE' will trigger a preflight request.
        type: "get",

        // The URL to make the request to.
        url: options.url,

        // Datatype is XML, which forces a $.parseXML for the received data.
        dataType: "xml",

        // Don't cache the transfered data.
        cache: false,

        // The 'contentType' property sets the 'Content-Type' header.
        // The jquery default for this property is
        // 'application/x-www-form-urlencoded; charset=UTF-8', which does not trigger
        // a preflight. If you set this value to anything other than
        // application/x-www-form-urlencoded, multipart/form-data, or text/plain,
        // you will trigger a preflight request.
        contentType: "text/plain",

        // The 'xhrFields' property sets additional fields on the XMLHttpRequest.
        // This can be used to set the 'withCredentials' property.
        // Set the value to 'true' if you'd like to pass cookies to the server.
        // If this is enabled, your server must respond with the header
        // 'Access-Control-Allow-Credentials: true'.
        xhrFields: {
            withCredentials: false
        },

        // Set any custom headers here.
        // If you set any non-simple headers, your server must include these
        // headers in the 'Access-Control-Allow-Headers' response header.
        headers: {
        },

        success: function( xmlDoc ) {
            options.onSuccess( xmlDoc );
        },

        error: function() {

            if ( null !== onError ) {
                onError();
            }
        }

    });

};

/**
 * Get the MDF as xml document from local file system.
 *
 * @param {object} options              - Options
 * @param {function} options.fileRef    - File reference to MDF file
 * @param {function} options.onSuccess  - If the xml file is successful loaded, this function will be called.
 * @param {function} [options.onError]  - If loading the xml file failed, this function will be called.
 */
vscp.mdf.loadLocal = function( options ) {

    var reader  = new FileReader();
    var onError = null;

    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing. " );
        return;
    }

    if ( "undefined" === typeof options.fileRef ) {
        console.error( vscp.utility.getTime() + " MDF file reference is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess function is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    console.info( vscp.utility.getTime() + " Load MDF from file " + options.fileRef.name );

    reader.readAsText( options.fileRef );

    reader.onload = function( e ) {
        var xml     = $.parseXML( reader.result );
        var xmlDoc  = $( xml );

        options.onSuccess( xmlDoc );
    };

    reader.onerror = function( e ) {

        if ( null !== onError ) {
            onError();
        }

    };

    return;
};

/**
 * Read a abstract value.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {object} options.mdf                  - MDF as xml jquery object
 * @param {number} options.id                   - Abstract value id
 * @param {function} options.onSuccess          - Function which is called on a successful operation
 * @param {function} [options.onError]          - Function which is called on a failed operation
 */
vscp.mdf.readAbstractValue = function( options ) {

    var onError         = null;
    var page            = 0;
    var offset          = 0;
    var type            = "";
    var size            = 0;
    var $abstraction    = null;
    var convFunc        = null;

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

    if ( "object" !== typeof options.mdf ) {
        console.error( vscp.utility.getTime() + " MDF object is missing." );
        return;
    }

    if ( "string" !== typeof options.id ) {
        console.error( vscp.utility.getTime() + " Abstraction id is missing." );
        return;
    }

    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess is missing." );
        return;
    }

    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }

    if ( "undefined" === typeof options.mdf.find( "vscp > module > abstractions > abstraction" ) ) {

        console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " is unknown." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }

    // Find abstract value by id
    options.mdf.find( "vscp > module > abstractions > abstraction" ).each( function() {

        if ( "undefined" !== typeof $( this ).attr( "id" ) ) {

            if ( options.id === $( this ).attr( "id" ) ) {

                $abstraction = $( this );

                // Break for each loop
                return false;
            }
        }

    });

    if ( null === $abstraction ) {
        console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " not found." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }

    if ( "undefined" === typeof $abstraction.attr( "page" ) ) {

        console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " page is missing." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }
    else {
        page = parseInt( $abstraction.attr( "page" ) );
    }

    if ( "undefined" === typeof $abstraction.attr( "offset" ) ) {

        console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " offset is missing." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }
    else {
        offset = parseInt( $abstraction.attr( "offset" ) );
    }

    if ( "undefined" === typeof $abstraction.attr( "type" ) ) {

        console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " type is missing." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }
    else {
        type = $abstraction.attr( "type" );
    }

    if ( "string" === type ) {

        if ( "undefined" === typeof $abstraction.attr( "size" ) ) {

            console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " size is missing." );

            if ( null !== onError ) {
                onError();
            }

            return;
        }

        size = parseInt( $abstraction.attr( "size" ).text() );
    }
    else {
        size = vscp.mdf.constants.TYPE_SIZES[ type ];
    }

    convFunc = vscp.mdf.constants.CONVERSION_FROM_DECIMAL[ type ];

    if ( "undefined" === typeof convFunc ) {
        console.error( vscp.utility.getTime() + " Conversion function for type " + type + " is unknown." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }

    console.info( vscp.utility.getTime() + " Read abstract \"" + options.id + "\" value." );

    // Read registers
    vscp.register.read({

        connection: options.connection,

        nodeId: options.nodeId,

        page: page,

        offset: offset,

        count: size,

        onSuccess: function( data ) {

            options.onSuccess(
                convFunc( data )
            );
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });

};

/**
 * Write a abstract value.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {object} options.mdf                  - MDF as xml jquery object
 * @param {number} options.id                   - Abstract value id
 * @param {string} options.value                - Abstract value
 * @param {function} options.onSuccess          - Function which is called on a successful operation
 * @param {function} [options.onError]          - Function which is called on a failed operation
 */
vscp.mdf.writeAbstractValue = function( options ) {

    var onError         = null;
    var page            = 0;
    var offset          = 0;
    var type            = "";
    var size            = 0;
    var $abstraction    = null;
    var convFunc        = null;

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

    if ( "object" !== typeof options.mdf ) {
        console.error( vscp.utility.getTime() + " MDF object is missing." );
        return;
    }

    if ( "string" !== typeof options.id ) {
        console.error( vscp.utility.getTime() + " Abstraction id is missing." );
        return;
    }

    if ( "undefined" === typeof options.value ) {
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

    if ( "undefined" === typeof options.mdf.find( "vscp > module > abstractions > abstraction" ) ) {

        console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " is unknown." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }

    // Find abstract value by id
    options.mdf.find( "vscp > module > abstractions > abstraction" ).each( function() {

        if ( "undefined" !== typeof $( this ).attr( "id" ) ) {

            if ( options.id === $( this ).attr( "id" ) ) {

                $abstraction = $( this );

                // Break for each loop
                return false;
            }
        }

    });

    if ( null === $abstraction ) {
        console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " not found." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }

    if ( "undefined" === typeof $abstraction.attr( "page" ) ) {

        console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " page is missing." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }
    else {
        page = parseInt( $abstraction.attr( "page" ) );
    }

    if ( "undefined" === typeof $abstraction.attr( "offset" ) ) {

        console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " offset is missing." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }
    else {
        offset = parseInt( $abstraction.attr( "offset" ) );
    }

    if ( "undefined" === typeof $abstraction.attr( "type" ) ) {

        console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " type is missing." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }
    else {
        type = $abstraction.attr( "type" );
    }

    if ( "string" === type ) {

        if ( "undefined" === typeof $abstraction.attr( "size" ) ) {

            console.error( vscp.utility.getTime() + " Abstract value id " + options.id + " size is missing." );

            if ( null !== onError ) {
                onError();
            }

            return;
        }

        size = parseInt( $abstraction.attr( "size" ).text() );
    }
    else {
        size = vscp.mdf.constants.TYPE_SIZES[ type ];
    }

    convFunc = vscp.mdf.constants.CONVERSION_TO_DECIMAL[ type ];

    if ( "undefined" === typeof convFunc ) {
        console.error( vscp.utility.getTime() + " Conversion function for type " + type + " is unknown." );

        if ( null !== onError ) {
            onError();
        }

        return;
    }

    console.info( vscp.utility.getTime() + " Write abstract \"" + options.id + "\" value " + options.value + "." );

    // Write registers
    vscp.register.write({

        connection: options.connection,

        nodeId: options.nodeId,

        page: page,

        offset: offset,

        count: size,

        data: convFunc( options.value ),

        onSuccess: function() {

            options.onSuccess();
        },

        onError: function() {
            if ( null !== onError ) {
                onError();
            }
        }
    });

};

/**
 * Change some bits in a abstract value.
 *
 * @param {object} options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number} options.nodeId               - Node id
 * @param {object} options.mdf                  - MDF as xml jquery object
 * @param {number} options.id                   - Abstract value id
 * @param {number} options.pos                  - Bit position
 * @param {number} options.width                - Bit width
 * @param {string} options.value                - Abstract value
 * @param {function} options.onSuccess          - Function which is called on a successful operation
 * @param {function} [options.onError]          - Function which is called on a failed operation
 */
vscp.mdf.writeAbstractBits = function( options ) {

    var onError     = null;
    var index       = 0;
    var width       = 1;
    var mask        = 0;

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

    if ( "object" !== typeof options.mdf ) {
        console.error( vscp.utility.getTime() + " MDF object is missing." );
        return;
    }

    if ( "string" !== typeof options.id ) {
        console.error( vscp.utility.getTime() + " Abstraction id is missing." );
        return;
    }

    if ( "number" !== typeof options.pos ) {
        console.error( vscp.utility.getTime() + " Bit position is missing." );
        return;
    }

    if ( "number" === typeof options.width ) {
        width = options.width;
    }

    if ( "undefined" !== typeof options.value ) {
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

    // Read value
    vscp.mdf.readAbstractValue({

        connection: options.connection,

        nodeId: options.nodeId,

        mdf: options.mdf,

        id: options.id,

        onSuccess: function( value ) {

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
            vscp.mdf.writeAbstractValue({

                connection: options.connection,

                nodeId: options.nodeId,

                mdf: options.mdf,

                id: options.id,

                value: value,

                onSuccess: options.onSuccess,

                onError: onError
            });

        },

        onError: onError

    });

};

/**
 * MDF register with all parameters.
 * @class
 */
vscp.mdf.Register = function() {

    /** Register name
     * @member {string}
     */
    this.name           = "";

    /** Register description
     * @member {string}
     */
    this.description    = "";

    /** Read access to the register allowed or not
     * @member {boolean}
     */
    this.readAccess     = false;

    /** Write access to the register allowed or not
     * @member {boolean}
     */
    this.writeAccess    = false;

    /** Page where the register is located
     * @member {number}
     */
    this.page           = 0;

    /** Offset where the register is located on the page
     * @member {number}
     */
    this.offset         = 0;

    /** Register value
     * @member {number}
     */
    this.value          = 0;

    /** Marks the register value dirty, which means the user changed it.
     * It can be used to detect which registers have to be written to the node.
     * @member {number}
     */
    this.dirty          = false;

    /** User specific data
     * @member {object}
     */
    this.userData       = null;
};

/**
 * This function set a register value and marks it dirty.
 *
 * @param {number} value - New value
 */
vscp.mdf.Register.prototype.setValue = function( value ) {

    // Out of range?
    if ( ( 0 > value ) ||
         ( 255 < value ) ) {

        // Abort
        console.error( vscp.utility.getTime() + " Value " + value + " out of range." );
        return;
    }

    if ( this.value !== value ) {

        this.value  = value;
        this.dirty  = true;
    }

    return;
};

/**
 * This function retrieves all register informations from a mdf.
 *
 * @param {object} options      - Options
 * @param {object} options.mdf  - MDF as jquery object
 *
 * @return {vscp.mdf.Register[]} Array of MDF registers
 */
vscp.mdf.getRegisters = function( options ) {

    var $registers  = null;
    var registers   = [];
    var register    = null;

    if ( "undefined" === typeof options ) {
        return;
    }

    if ( "undefined" === typeof options.mdf ) {
        return;
    }

    $registers = options.mdf.find( "vscp > module > registers" );

    if ( null === $registers ) {
        return;
    }

    $registers.children( "reg" ).each( function() {

        var $name           = $( this ).children( "name" );
        var $description    = $( this ).children( "description" );
        var $access         = $( this ).children( "access" );
        var page            = $( this ).attr( "page" );
        var offset          = $( this ).attr( "offset" );

        register = new vscp.mdf.Register();

        if ( "undefined" !== typeof $name ) {
            register.name = $name.text();
        }

        if ( "undefined" !== typeof $description ) {
            register.description = $description.text();
        }

        if ( "undefined" !== typeof $access ) {
            if ( "r" === $access.text() ) {
                register.readAccess     = true;
                register.writeAccess    = false;
            }
            else if ( "w" === $access.text() ) {
                register.readAccess     = false;
                register.writeAccess    = true;
            }
            else if ( "rw" === $access.text() ) {
                register.readAccess     = true;
                register.writeAccess    = true;
            }
        }

        if ( "undefined" !== typeof page ) {
            register.page = parseInt( page );
        }

        if ( "undefined" !== typeof offset ) {
            register.offset = parseInt( offset );
        }

        registers.push( register );
    });

    return registers;
};

/**
 * MDF abstraction with all parameters.
 * @class
 */
vscp.mdf.Abstraction = function() {

    /** Abstraction name
     * @member {string}
     */
    this.name           = "";

    /** Abstraction description
     * @member {string}
     */
    this.description    = "";

    /** Read access to the abstraction allowed or not
     * @member {boolean}
     */
    this.readAccess     = false;

    /** Write access to the abstraction allowed or not
     * @member {boolean}
     */
    this.writeAccess    = false;

    /** Abstraction id
     * @member {string}
     */
    this.id             = "";
    
    /** Page where the abstraction is located
     * @member {number}
     */
    this.page           = 0;

    /** Offset where the abstraction is located
     * @member {number}
     */
    this.offset         = 0;

    /** Abstraction value
     * @member {number}
     */
    this.value          = 0;

    /** Abstraction type
     * @member {string}
     */
    this.type           = "";

    /** Abstraction default value
     * @member {number}
     */
    this.defValue       = 0;

    /** Marks the abstraction value dirty, which means the user changed it.
     * It can be used to detect which abstractions have to be written to the node.
     * @member {number}
     */
    this.dirty          = false;

    /** User specific data
     * @member {object}
     */
    this.userData       = null;
};

/**
 * This function set a abstraction value and marks it dirty.
 *
 * @param {number} value - New value
 */
vscp.mdf.Abstraction.prototype.setValue = function( value ) {

    var min = 0;
    var max = 0;

    // Shall the value range be checked?
    if ( "number" === typeof value ) {

        if ( "undefined" !== typeof vscp.mdf.constants.RANGE[ this.type ] ) {

            // Get range
            min = vscp.mdf.constants.RANGE[ this.type ].MIN;
            max = vscp.mdf.constants.RANGE[ this.type ].MAX;

            // Range check supported?
            if ( min !== max ) {

                // Out of range?
                if ( ( min > value ) ||
                     ( max < value ) ) {

                    // Abort
                    console.error( vscp.utility.getTime() + " Value " + value + " out of range." );
                    return;
                }
            }
        }
    }

    if ( this.value !== value ) {

        this.value  = value;
        this.dirty  = true;
    }

    return;
};

/**
 * This function retrieves all abstraction informations from a mdf.
 *
 * @param {object} options      - Options
 * @param {object} options.mdf  - MDF as jquery object
 *
 * @return {vscp.mdf.Abstraction[]} Array of MDF abstractions
 */
vscp.mdf.getAbstractions = function( options ) {

    var $abstractions   = null;
    var abstractions    = [];
    var abstraction     = null;

    if ( "undefined" === typeof options ) {
        return;
    }

    if ( "undefined" === typeof options.mdf ) {
        return;
    }

    $abstractions = options.mdf.find( "vscp > module > abstractions" );

    if ( null === $abstractions ) {
        return;
    }

    $abstractions.children( "abstraction" ).each( function() {

        var $name           = $( this ).children( "name" );
        var $description    = $( this ).children( "description" );
        var $access         = $( this ).children( "access" );
        var id              = $( this ).attr( "id" );
        var page            = $( this ).attr( "page" );
        var offset          = $( this ).attr( "offset" );
        var type            = $( this ).attr( "type" );
        var defValue        = $( this ).attr( "default" );

        abstraction = new vscp.mdf.Abstraction();

        if ( "undefined" !== typeof $name ) {
            abstraction.name = $name.text();
        }

        if ( "undefined" !== typeof $description ) {
            abstraction.description = $description.text();
        }

        if ( "undefined" !== typeof $access ) {
            if ( "r" === $access.text() ) {
                abstraction.readAccess     = true;
                abstraction.writeAccess    = false;
            }
            else if ( "w" === $access.text() ) {
                abstraction.readAccess     = false;
                abstraction.writeAccess    = true;
            }
            else if ( "rw" === $access.text() ) {
                abstraction.readAccess     = true;
                abstraction.writeAccess    = true;
            }
        }

        if ( "undefined" !== typeof id ) {
            abstraction.id = id;
        }
        
        if ( "undefined" !== typeof page ) {
            abstraction.page = parseInt( page );
        }

        if ( "undefined" !== typeof offset ) {
            abstraction.offset = parseInt( offset );
        }

        if ( "undefined" !== typeof type ) {
            abstraction.type = type;
        }

        if ( "undefined" !== typeof defValue ) {
            abstraction.defValue = parseInt( defValue );
        }

        abstractions.push( abstraction );
    });

    return abstractions;
};
