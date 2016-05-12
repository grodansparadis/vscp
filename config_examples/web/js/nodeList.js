// Node list handling.
//
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

/**
 * Node list (singleton)
 *
 * Call the init() function before any other function is called.
 */
var nodeList = (function() {

    var connection = null;  // VSCP connection
    var container  = null;  // Container for the node list
    
    /**
     * Initializes the node list by
     * - Setup a container
     * - Reading the node list from the daemon
     * - If the node list variable not exists, it will create one
     *
     * @param {object} options                      - Options
     * @param {vscp.Connection} options.connection  - VSCP connection
     * @param {function} options.onSuccess          - Callback which is called on successful operation
     * @param {function} [options.onError]          - Callback which is called on failed operation
     */
    function init( options ) {

        if ( "undefined" === typeof options ) {
            return;
        }
        
        if ( true === ( options.connection instanceof vscp.Connection ) ) {
            connection = options.connection;
            
            // Create node list container
            container = new vscp.service.Container({
                connection: connection,
                name: "nodeList"
            });
            
            // Fill the node list container
            container.read({
            
                onSuccess: options.onSuccess,
                
                onError: function() {
                    // Node list doesn't exists, create a empty one
                    container.create({
                        onSuccess: options.onSuccess,
                        onError: options.onError
                    });
                }
            });
        }
    }
    
    /**
     * Get a list with current known nodes.
     *
     * @return {string[]} Node list (GUID string array)
     */
    function get() {
        return container.data;
    }
    
    /**
     * Set a new list of nodes.
     *
     * @param {object} options - Options
     * @param {string[]} guids - Array of GUID strings (one per node)
     */
    function set( options ) {
    
        if ( "undefined" === typeof options ) {
            return;
        }
        
        if ( false === ( options.guids instanceof Array ) ) {
            return;
        }
        
        container.data = options.guids;
    }
    
    /**
     * Append a node to the list.
     *
     * @param {object} options - Options
     * @param {string} guid - GUID string
     */
    function append( options ) {
        
        if ( "undefined" === typeof options ) {
            return;
        }
        
        if ( "string" !== typeof options.guid ) {
            return;
        }
        
        container.data.push( options.guid );
    }
    
    /**
     * Clears the node list.
     */
    function clear() {
        
        container.data = [];
    }
    
    /**
     * Write the node list persistent to a daemon variable.
     *
     * @param {object} options                      - Options
     * @param {function} options.onSuccess          - Callback which is called on successful operation
     * @param {function} [options.onError]          - Callback which is called on failed operation
     */
    function write( options ) {
        
        if ( "undefined" === typeof options ) {
            return;
        }
        
        container.write({
            onSuccess: options.onSuccess,
            onError: options.onError
        });
    }
    
    // Return the public interface
    return {
        init: init,
        get: get,
        set: set,
        append: append,
        clear: clear,
        write: write
    };
    
})();
