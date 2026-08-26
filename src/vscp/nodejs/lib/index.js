/**
 * VSCP Node.js Level 2 Interface
 * Main entry point for the VSCP Level 2 client library
 * 
 * Copyright (C) 2000-2026 Ake Hedman, the VSCP project <info@vscp.org>
 * MIT License
 */

'use strict';

// Import the Level 2 client components
const {
    VscpLevel2Client,
    VscpEvent,
    VscpEventFilter,
    VSCP_CONSTANTS
} = require('./vscp-level2-client');
const { VscpGuid } = require('./vscp-guid');

// Re-export everything for easy access
module.exports = {
    // Main client class
    VscpLevel2Client,
    
    // Data structures
    VscpEvent,
    VscpEventFilter,
    VscpGuid,
    
    // Constants
    VSCP_CONSTANTS,
    
    // Convenience aliases
    Client: VscpLevel2Client,
    Event: VscpEvent,
    Filter: VscpEventFilter,
    Guid: VscpGuid,
    Constants: VSCP_CONSTANTS,
    
    // Factory functions
    createClient: function(options) {
        return new VscpLevel2Client(options);
    },
    
    createEvent: function(options) {
        return new VscpEvent(options);
    },
    
    createFilter: function(options) {
        return new VscpEventFilter(options);
    },

    createGuid: function(value) {
        return new VscpGuid(value);
    },
    
    // Version information
    version: '15.0.3',
    
    // Library information
    info: {
        name: 'node-vscp-level2',
        version: '15.0.3',
        description: 'VSCP Level 2 Client Interface for Node.js',
        author: 'Ake Hedman, the VSCP project',
        license: 'MIT',
        homepage: 'https://www.vscp.org'
    }
};