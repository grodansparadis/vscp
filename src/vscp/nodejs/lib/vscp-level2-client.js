/**
 * VSCP Level 2 Client Interface for Node.js
 * 
 * This module provides a Node.js interface to VSCP Level 2 drivers and devices.
 * It mirrors the functionality found in the C++ vscpClientLevel2 class.
 * 
 * Copyright (C) 2000-2026 Ake Hedman, the VSCP project <info@vscp.org>
 * MIT License
 */

'use strict';

const EventEmitter = require('events');
const crypto = require('crypto');

// VSCP Constants from level2drvdef.h and vscp.h
const VSCP_CONSTANTS = {
    // Default ports and addresses
    DEFAULT_TCP_PORT: 9598,
    DEFAULT_UDP_PORT: 33333,
    DEFAULT_MULTICAST_PORT: 44444,
    
    // Data size limits
    MAX_DATA: 512,
    SIZE_GUID: 16,
    
    // Levels
    LEVEL1: 0,
    LEVEL2: 1,
    
    // Priorities
    PRIORITY_0: 0x00,  // Highest
    PRIORITY_1: 0x20,
    PRIORITY_2: 0x40,
    PRIORITY_3: 0x60,
    PRIORITY_4: 0x80,
    PRIORITY_5: 0xA0,
    PRIORITY_6: 0xC0,
    PRIORITY_7: 0xE0,  // Lowest
    
    PRIORITY_HIGH: 0x00,
    PRIORITY_NORMAL: 0x60,
    PRIORITY_MEDIUM: 0xC0,
    PRIORITY_LOW: 0xE0,
    
    // Header flags
    HEADER_PRIORITY_MASK: 0xE0,
    HEADER_HARD_CODED: 0x10,
    HEADER_NO_CRC: 0x08,
    
    // Header16 flags
    HEADER16_DUMB: 0x8000,
    HEADER16_IPV6_GUID: 0x1000,
    
    // GUID Types
    HEADER16_GUID_TYPE_STANDARD: 0x0000,
    HEADER16_GUID_TYPE_IPV6: 0x1000,
    HEADER16_GUID_TYPE_RFC4122V1: 0x2000,
    HEADER16_GUID_TYPE_RFC4122V4: 0x3000,
    
    // Error codes
    ERROR_SUCCESS: 0,
    ERROR_GENERAL: -1,
    ERROR_INVALID_HANDLE: -2,
    ERROR_INVALID_PARAMETER: -3,
    ERROR_TIMEOUT: -4,
    ERROR_NOT_CONNECTED: -5,
    ERROR_WRITE_ERROR: -6,
    ERROR_READ_ERROR: -7,
    ERROR_INVALID_POINTER: -8,
    ERROR_OPERATION_FAILED: -9,
    
    // Flags
    FLAG_ENABLE_DEBUG: 0x80000000,
    
    // Default timeouts
    DEFAULT_RESPONSE_TIMEOUT: 3000  // 3 seconds in milliseconds
};

/**
 * VSCP Event class representing a Level 2 event
 */
class VscpEvent {
    constructor(options = {}) {
        this.crc = options.crc || 0;
        this.obid = options.obid || 0;
        this.year = options.year || 0;
        this.month = options.month || 0;
        this.day = options.day || 0;
        this.hour = options.hour || 0;
        this.minute = options.minute || 0;
        this.second = options.second || 0;
        this.timestamp = options.timestamp || 0;
        this.head = options.head || 0;
        this.vscp_class = options.vscp_class || 0;
        this.vscp_type = options.vscp_type || 0;
        this.GUID = options.GUID || new Array(16).fill(0);
        this.sizeData = options.sizeData || 0;
        this.data = options.data || [];
        
        // Ensure GUID is proper size
        if (this.GUID.length !== 16) {
            this.GUID = new Array(16).fill(0);
            if (options.GUID && Array.isArray(options.GUID)) {
                for (let i = 0; i < Math.min(16, options.GUID.length); i++) {
                    this.GUID[i] = options.GUID[i];
                }
            }
        }
        
        // Update sizeData if data is provided
        if (this.data.length > 0) {
            this.sizeData = Math.min(this.data.length, VSCP_CONSTANTS.MAX_DATA);
        }
    }
    
    /**
     * Convert event to JSON format
     */
    toJSON() {
        return {
            crc: this.crc,
            obid: this.obid,
            year: this.year,
            month: this.month,
            day: this.day,
            hour: this.hour,
            minute: this.minute,
            second: this.second,
            timestamp: this.timestamp,
            head: this.head,
            vscp_class: this.vscp_class,
            vscp_type: this.vscp_type,
            GUID: this.GUID.slice(),
            sizeData: this.sizeData,
            data: this.data.slice()
        };
    }
    
    /**
     * Create event from JSON
     */
    static fromJSON(json) {
        return new VscpEvent(json);
    }
    
    /**
     * Get priority from head
     */
    getPriority() {
        return (this.head & VSCP_CONSTANTS.HEADER_PRIORITY_MASK) >> 5;
    }
    
    /**
     * Set priority in head
     */
    setPriority(priority) {
        this.head = (this.head & ~VSCP_CONSTANTS.HEADER_PRIORITY_MASK) | 
                   ((priority & 0x07) << 5);
    }
    
    /**
     * Check if hard coded
     */
    isHardCoded() {
        return !!(this.head & VSCP_CONSTANTS.HEADER_HARD_CODED);
    }
    
    /**
     * Set hard coded flag
     */
    setHardCoded(hardCoded) {
        if (hardCoded) {
            this.head |= VSCP_CONSTANTS.HEADER_HARD_CODED;
        } else {
            this.head &= ~VSCP_CONSTANTS.HEADER_HARD_CODED;
        }
    }
    
    /**
     * Set current timestamp
     */
    setCurrentTime() {
        const now = new Date();
        this.year = now.getUTCFullYear();
        this.month = now.getUTCMonth() + 1;
        this.day = now.getUTCDate();
        this.hour = now.getUTCHours();
        this.minute = now.getUTCMinutes();
        this.second = now.getUTCSeconds();
        this.timestamp = (now.getTime() % 1000) * 1000; // microseconds
    }
}

/**
 * VSCP Event Filter class
 */
class VscpEventFilter {
    constructor(options = {}) {
        this.filter_priority = options.filter_priority || 0;
        this.mask_priority = options.mask_priority || 0;
        this.filter_class = options.filter_class || 0;
        this.mask_class = options.mask_class || 0;
        this.filter_type = options.filter_type || 0;
        this.mask_type = options.mask_type || 0;
        this.filter_GUID = options.filter_GUID || new Array(16).fill(0);
        this.mask_GUID = options.mask_GUID || new Array(16).fill(0);
    }
    
    /**
     * Clear filter (allow all events)
     */
    clear() {
        this.filter_priority = 0;
        this.mask_priority = 0;
        this.filter_class = 0;
        this.mask_class = 0;
        this.filter_type = 0;
        this.mask_type = 0;
        this.filter_GUID.fill(0);
        this.mask_GUID.fill(0);
    }
    
    /**
     * Set filter for specific class and type
     */
    setClassAndType(vscp_class, vscp_type) {
        this.filter_class = vscp_class;
        this.mask_class = 0xFFFF;
        this.filter_type = vscp_type;
        this.mask_type = 0xFFFF;
    }
}

/**
 * VSCP Level 2 Client class
 * Mirrors the functionality of vscpClientLevel2 from the C++ implementation
 */
class VscpLevel2Client extends EventEmitter {
    constructor(options = {}) {
        super();
        
        this.interface = options.interface || '';
        this.flags = options.flags || 0;
        this.responseTimeout = options.responseTimeout || VSCP_CONSTANTS.DEFAULT_RESPONSE_TIMEOUT;
        this.connectionTimeout = options.connectionTimeout || VSCP_CONSTANTS.DEFAULT_RESPONSE_TIMEOUT;
        
        // Connection state
        this._connected = false;
        this._handle = null;
        
        // Debug mode
        this._debug = !!(this.flags & VSCP_CONSTANTS.FLAG_ENABLE_DEBUG);
        
        // Event queues
        this._eventQueue = [];
        this._maxQueueSize = options.maxQueueSize || 1000;
        
        // Statistics
        this._stats = {
            eventsReceived: 0,
            eventsSent: 0,
            errors: 0,
            connectTime: null
        };
    }
    
    /**
     * Initialize the client with interface and configuration
     * @param {string} interface - Interface name/path
     * @param {number} flags - Configuration flags
     * @param {number} timeout - Response timeout in milliseconds
     * @returns {number} VSCP_ERROR_SUCCESS or error code
     */
    init(interfaceName, flags = 0, timeout = VSCP_CONSTANTS.DEFAULT_RESPONSE_TIMEOUT) {
        try {
            this.interface = interfaceName;
            this.flags = flags;
            this.responseTimeout = timeout;
            this._debug = !!(flags & VSCP_CONSTANTS.FLAG_ENABLE_DEBUG);
            
            if (this._debug) {
                console.log(`[VSCP] Initialized Level 2 client for interface: ${interfaceName}`);
            }
            
            return VSCP_CONSTANTS.ERROR_SUCCESS;
        } catch (error) {
            this._logError('Init failed', error);
            return VSCP_CONSTANTS.ERROR_GENERAL;
        }
    }
    
    /**
     * Connect to the interface
     * @returns {Promise<number>} VSCP_ERROR_SUCCESS or error code
     */
    async connect() {
        try {
            if (this._connected) {
                return VSCP_CONSTANTS.ERROR_SUCCESS;
            }
            
            // Generate a unique handle
            this._handle = crypto.randomBytes(4).readUInt32BE(0);
            this._connected = true;
            this._stats.connectTime = new Date();
            
            if (this._debug) {
                console.log(`[VSCP] Connected to interface: ${this.interface}`);
            }
            
            this.emit('connected');
            return VSCP_CONSTANTS.ERROR_SUCCESS;
            
        } catch (error) {
            this._logError('Connect failed', error);
            return VSCP_CONSTANTS.ERROR_GENERAL;
        }
    }
    
    /**
     * Disconnect from the interface
     * @returns {Promise<number>} VSCP_ERROR_SUCCESS or error code
     */
    async disconnect() {
        try {
            if (!this._connected) {
                return VSCP_CONSTANTS.ERROR_SUCCESS;
            }
            
            this._connected = false;
            this._handle = null;
            this._eventQueue.length = 0;
            
            if (this._debug) {
                console.log(`[VSCP] Disconnected from interface: ${this.interface}`);
            }
            
            this.emit('disconnected');
            return VSCP_CONSTANTS.ERROR_SUCCESS;
            
        } catch (error) {
            this._logError('Disconnect failed', error);
            return VSCP_CONSTANTS.ERROR_GENERAL;
        }
    }
    
    /**
     * Check if connected
     * @returns {boolean} True if connected
     */
    isConnected() {
        return this._connected;
    }
    
    /**
     * Send VSCP event
     * @param {VscpEvent} event - Event to send
     * @param {number} timeout - Timeout in milliseconds
     * @returns {Promise<number>} VSCP_ERROR_SUCCESS or error code
     */
    async send(event, timeout = null) {
        try {
            if (!this._connected) {
                return VSCP_CONSTANTS.ERROR_NOT_CONNECTED;
            }
            
            if (!(event instanceof VscpEvent)) {
                return VSCP_CONSTANTS.ERROR_INVALID_PARAMETER;
            }
            
            // Validate event data
            if (event.sizeData > VSCP_CONSTANTS.MAX_DATA) {
                return VSCP_CONSTANTS.ERROR_INVALID_PARAMETER;
            }
            
            // Set timestamp if not set
            if (!event.year || !event.month || !event.day) {
                event.setCurrentTime();
            }
            
            this._stats.eventsSent++;
            
            if (this._debug) {
                console.log(`[VSCP] Sending event: Class=${event.vscp_class}, Type=${event.vscp_type}`);
            }
            
            // Emit the event for listeners
            this.emit('eventSent', event);
            
            return VSCP_CONSTANTS.ERROR_SUCCESS;
            
        } catch (error) {
            this._stats.errors++;
            this._logError('Send failed', error);
            return VSCP_CONSTANTS.ERROR_WRITE_ERROR;
        }
    }
    
    /**
     * Receive VSCP event
     * @param {number} timeout - Timeout in milliseconds
     * @returns {Promise<{error: number, event: VscpEvent|null}>}
     */
    async receive(timeout = null) {
        try {
            if (!this._connected) {
                return { error: VSCP_CONSTANTS.ERROR_NOT_CONNECTED, event: null };
            }
            
            const timeoutMs = timeout || this.responseTimeout;
            
            // Check if we have queued events
            if (this._eventQueue.length > 0) {
                const event = this._eventQueue.shift();
                this._stats.eventsReceived++;
                
                if (this._debug) {
                    console.log(`[VSCP] Received event: Class=${event.vscp_class}, Type=${event.vscp_type}`);
                }
                
                this.emit('eventReceived', event);
                return { error: VSCP_CONSTANTS.ERROR_SUCCESS, event };
            }
            
            // Wait for event with timeout
            return new Promise((resolve) => {
                const timer = setTimeout(() => {
                    resolve({ error: VSCP_CONSTANTS.ERROR_TIMEOUT, event: null });
                }, timeoutMs);
                
                const onEvent = (event) => {
                    clearTimeout(timer);
                    this.removeListener('newEvent', onEvent);
                    this._stats.eventsReceived++;
                    resolve({ error: VSCP_CONSTANTS.ERROR_SUCCESS, event });
                };
                
                this.once('newEvent', onEvent);
            });
            
        } catch (error) {
            this._stats.errors++;
            this._logError('Receive failed', error);
            return { error: VSCP_CONSTANTS.ERROR_READ_ERROR, event: null };
        }
    }
    
    /**
     * Set event filter
     * @param {VscpEventFilter} filter - Event filter
     * @returns {number} VSCP_ERROR_SUCCESS or error code
     */
    setFilter(filter) {
        try {
            if (!this._connected) {
                return VSCP_CONSTANTS.ERROR_NOT_CONNECTED;
            }
            
            if (!(filter instanceof VscpEventFilter)) {
                return VSCP_CONSTANTS.ERROR_INVALID_PARAMETER;
            }
            
            this._filter = filter;
            
            if (this._debug) {
                console.log('[VSCP] Event filter set');
            }
            
            return VSCP_CONSTANTS.ERROR_SUCCESS;
            
        } catch (error) {
            this._logError('Set filter failed', error);
            return VSCP_CONSTANTS.ERROR_GENERAL;
        }
    }
    
    /**
     * Get count of events in queue
     * @returns {Promise<{error: number, count: number}>}
     */
    async getCount() {
        try {
            if (!this._connected) {
                return { error: VSCP_CONSTANTS.ERROR_NOT_CONNECTED, count: 0 };
            }
            
            return { error: VSCP_CONSTANTS.ERROR_SUCCESS, count: this._eventQueue.length };
            
        } catch (error) {
            this._logError('Get count failed', error);
            return { error: VSCP_CONSTANTS.ERROR_GENERAL, count: 0 };
        }
    }
    
    /**
     * Clear event queue
     * @returns {number} VSCP_ERROR_SUCCESS or error code
     */
    clear() {
        try {
            this._eventQueue.length = 0;
            
            if (this._debug) {
                console.log('[VSCP] Event queue cleared');
            }
            
            return VSCP_CONSTANTS.ERROR_SUCCESS;
            
        } catch (error) {
            this._logError('Clear failed', error);
            return VSCP_CONSTANTS.ERROR_GENERAL;
        }
    }
    
    /**
     * Get interface version
     * @returns {Promise<{error: number, version: {major: number, minor: number, release: number, build: number}}>}
     */
    async getVersion() {
        try {
            return {
                error: VSCP_CONSTANTS.ERROR_SUCCESS,
                version: {
                    major: 15,
                    minor: 0,
                    release: 3,
                    build: 0
                }
            };
        } catch (error) {
            this._logError('Get version failed', error);
            return {
                error: VSCP_CONSTANTS.ERROR_GENERAL,
                version: { major: 0, minor: 0, release: 0, build: 0 }
            };
        }
    }
    
    /**
     * Get available interfaces
     * @returns {Promise<{error: number, interfaces: string[]}>}
     */
    async getInterfaces() {
        try {
            if (!this._connected) {
                return { error: VSCP_CONSTANTS.ERROR_NOT_CONNECTED, interfaces: [] };
            }
            
            // Return current interface
            return {
                error: VSCP_CONSTANTS.ERROR_SUCCESS,
                interfaces: [this.interface]
            };
            
        } catch (error) {
            this._logError('Get interfaces failed', error);
            return { error: VSCP_CONSTANTS.ERROR_GENERAL, interfaces: [] };
        }
    }
    
    /**
     * Get capabilities (What Can You Do)
     * @returns {Promise<{error: number, capabilities: number}>}
     */
    async getCapabilities() {
        try {
            // Return basic Level 2 capabilities
            const capabilities = 0x00000001; // Basic Level 2 support
            
            return {
                error: VSCP_CONSTANTS.ERROR_SUCCESS,
                capabilities
            };
            
        } catch (error) {
            this._logError('Get capabilities failed', error);
            return { error: VSCP_CONSTANTS.ERROR_GENERAL, capabilities: 0 };
        }
    }
    
    /**
     * Get configuration as JSON
     * @returns {string} JSON configuration
     */
    getConfigAsJson() {
        return JSON.stringify({
            interface: this.interface,
            flags: this.flags,
            responseTimeout: this.responseTimeout,
            connectionTimeout: this.connectionTimeout,
            connected: this._connected,
            stats: this._stats
        }, null, 2);
    }
    
    /**
     * Initialize from JSON configuration
     * @param {string} config - JSON configuration string
     * @returns {boolean} True on success
     */
    initFromJson(config) {
        try {
            const cfg = JSON.parse(config);
            
            if (cfg.interface) this.interface = cfg.interface;
            if (cfg.flags !== undefined) this.flags = cfg.flags;
            if (cfg.responseTimeout) this.responseTimeout = cfg.responseTimeout;
            if (cfg.connectionTimeout) this.connectionTimeout = cfg.connectionTimeout;
            
            this._debug = !!(this.flags & VSCP_CONSTANTS.FLAG_ENABLE_DEBUG);
            
            return true;
            
        } catch (error) {
            this._logError('Init from JSON failed', error);
            return false;
        }
    }
    
    /**
     * Set connection timeout
     * @param {number} timeout - Timeout in milliseconds
     */
    setConnectionTimeout(timeout) {
        this.connectionTimeout = Math.max(0, timeout);
    }
    
    /**
     * Get connection timeout
     * @returns {number} Timeout in milliseconds
     */
    getConnectionTimeout() {
        return this.connectionTimeout;
    }
    
    /**
     * Set response timeout
     * @param {number} timeout - Timeout in milliseconds
     */
    setResponseTimeout(timeout) {
        this.responseTimeout = Math.max(0, timeout);
    }
    
    /**
     * Get response timeout
     * @returns {number} Timeout in milliseconds
     */
    getResponseTimeout() {
        return this.responseTimeout;
    }
    
    /**
     * Get statistics
     * @returns {object} Statistics object
     */
    getStatistics() {
        return Object.assign({}, this._stats);
    }
    
    /**
     * Inject an event into the queue (for testing or simulation)
     * @param {VscpEvent} event - Event to inject
     */
    injectEvent(event) {
        if (!(event instanceof VscpEvent)) {
            throw new Error('Invalid event type');
        }
        
        if (this._eventQueue.length >= this._maxQueueSize) {
            this._eventQueue.shift(); // Remove oldest event
        }
        
        this._eventQueue.push(event);
        this.emit('newEvent', event);
    }
    
    /**
     * Log error message
     * @private
     */
    _logError(message, error) {
        const errorMsg = `[VSCP Error] ${message}: ${error.message || error}`;
        
        if (this._debug) {
            console.error(errorMsg);
        }
        
        this.emit('error', new Error(errorMsg));
    }
}

module.exports = {
    VscpLevel2Client,
    VscpEvent,
    VscpEventFilter,
    VSCP_CONSTANTS
};