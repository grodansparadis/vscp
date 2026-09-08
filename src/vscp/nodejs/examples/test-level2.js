/**
 * Test Suite for VSCP Level 2 Client
 * Basic tests to verify the Level 2 interface functionality
 * 
 * Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project <info@vscp.org>
 * MIT License
 */

'use strict';

const { VscpLevel2Client, VscpEvent, VscpEventFilter, VSCP_CONSTANTS } = require('../lib');

class TestRunner {
    constructor() {
        this.tests = [];
        this.passed = 0;
        this.failed = 0;
    }
    
    addTest(name, testFn) {
        this.tests.push({ name, testFn });
    }
    
    assert(condition, message) {
        if (!condition) {
            throw new Error(`Assertion failed: ${message}`);
        }
    }
    
    assertEqual(actual, expected, message) {
        if (actual !== expected) {
            throw new Error(`Assertion failed: ${message}. Expected: ${expected}, Got: ${actual}`);
        }
    }
    
    async run() {
        console.log(`Running ${this.tests.length} tests...\\n`);
        
        for (const test of this.tests) {
            try {
                console.log(`Running: ${test.name}`);
                await test.testFn.call(this);
                console.log(`✅ PASSED: ${test.name}`);
                this.passed++;
            } catch (error) {
                console.error(`❌ FAILED: ${test.name} - ${error.message}`);
                this.failed++;
            }
        }
        
        console.log(`\\nTest Results: ${this.passed} passed, ${this.failed} failed`);
        
        if (this.failed > 0) {
            process.exit(1);
        }
    }
}

async function runTests() {
    console.log('=== VSCP Level 2 Client - Test Suite ===\\n');
    
    const runner = new TestRunner();
    
    // Test 1: Client creation and initialization
    runner.addTest('Client Creation and Initialization', async function() {
        const client = new VscpLevel2Client({
            interface: '/dev/test',
            flags: VSCP_CONSTANTS.FLAG_ENABLE_DEBUG
        });
        
        this.assert(client instanceof VscpLevel2Client, 'Client should be instance of VscpLevel2Client');
        this.assertEqual(client.interface, '/dev/test', 'Interface should be set correctly');
        this.assertEqual(client.flags, VSCP_CONSTANTS.FLAG_ENABLE_DEBUG, 'Flags should be set correctly');
        this.assert(!client.isConnected(), 'Client should not be connected initially');
        
        const initResult = client.init('/dev/test2', 0x1234, 5000);
        this.assertEqual(initResult, VSCP_CONSTANTS.ERROR_SUCCESS, 'Init should succeed');
        this.assertEqual(client.interface, '/dev/test2', 'Interface should be updated after init');
        this.assertEqual(client.flags, 0x1234, 'Flags should be updated after init');
    });
    
    // Test 2: Connection and disconnection
    runner.addTest('Connection and Disconnection', async function() {
        const client = new VscpLevel2Client();
        
        const connectResult = await client.connect();
        this.assertEqual(connectResult, VSCP_CONSTANTS.ERROR_SUCCESS, 'Connect should succeed');
        this.assert(client.isConnected(), 'Client should be connected after connect()');
        
        const disconnectResult = await client.disconnect();
        this.assertEqual(disconnectResult, VSCP_CONSTANTS.ERROR_SUCCESS, 'Disconnect should succeed');
        this.assert(!client.isConnected(), 'Client should not be connected after disconnect()');
    });
    
    // Test 3: VSCP Event creation and manipulation
    runner.addTest('VSCP Event Creation and Manipulation', async function() {
        const event = new VscpEvent({
            vscp_class: 10,
            vscp_type: 6,
            data: [0x89, 0x02, 0x00, 0x19]
        });
        
        this.assertEqual(event.vscp_class, 10, 'Event class should be set correctly');
        this.assertEqual(event.vscp_type, 6, 'Event type should be set correctly');
        this.assertEqual(event.data.length, 4, 'Event data length should be correct');
        this.assertEqual(event.sizeData, 4, 'Event sizeData should be updated automatically');
        
        // Test priority methods
        event.setPriority(VSCP_CONSTANTS.PRIORITY_HIGH >> 5);
        this.assertEqual(event.getPriority(), 0, 'Priority should be set to high (0)');
        
        event.setPriority(VSCP_CONSTANTS.PRIORITY_LOW >> 5);
        this.assertEqual(event.getPriority(), 7, 'Priority should be set to low (7)');
        
        // Test hard coded flag
        this.assert(!event.isHardCoded(), 'Event should not be hard coded by default');
        event.setHardCoded(true);
        this.assert(event.isHardCoded(), 'Event should be hard coded after setting');
        
        // Test timestamp setting
        const beforeTime = Date.now();
        event.setCurrentTime();
        const afterTime = Date.now();
        
        this.assert(event.year >= new Date(beforeTime).getUTCFullYear(), 'Year should be reasonable');
        this.assert(event.month >= 1 && event.month <= 12, 'Month should be valid');
        this.assert(event.day >= 1 && event.day <= 31, 'Day should be valid');
    });
    
    // Test 4: Event JSON serialization
    runner.addTest('Event JSON Serialization', async function() {
        const originalEvent = new VscpEvent({
            vscp_class: 20,
            vscp_type: 3,
            GUID: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
            data: [0xAA, 0xBB, 0xCC]
        });
        
        const json = originalEvent.toJSON();
        this.assert(typeof json === 'object', 'toJSON should return an object');
        this.assertEqual(json.vscp_class, 20, 'JSON should contain correct class');
        this.assertEqual(json.vscp_type, 3, 'JSON should contain correct type');
        
        const restoredEvent = VscpEvent.fromJSON(json);
        this.assertEqual(restoredEvent.vscp_class, originalEvent.vscp_class, 'Restored class should match');
        this.assertEqual(restoredEvent.vscp_type, originalEvent.vscp_type, 'Restored type should match');
        this.assertEqual(restoredEvent.data.length, originalEvent.data.length, 'Restored data length should match');
        this.assertEqual(restoredEvent.GUID.length, 16, 'Restored GUID should have correct length');
    });
    
    // Test 5: Event Filter functionality
    runner.addTest('Event Filter Functionality', async function() {
        const filter = new VscpEventFilter();
        
        // Test initial state
        this.assertEqual(filter.filter_class, 0, 'Initial filter class should be 0');
        this.assertEqual(filter.mask_class, 0, 'Initial mask class should be 0');
        
        // Test setClassAndType
        filter.setClassAndType(10, 6);
        this.assertEqual(filter.filter_class, 10, 'Filter class should be set to 10');
        this.assertEqual(filter.mask_class, 0xFFFF, 'Mask class should be set to 0xFFFF');
        this.assertEqual(filter.filter_type, 6, 'Filter type should be set to 6');
        this.assertEqual(filter.mask_type, 0xFFFF, 'Mask type should be set to 0xFFFF');
        
        // Test clear
        filter.clear();
        this.assertEqual(filter.filter_class, 0, 'Filter class should be cleared');
        this.assertEqual(filter.mask_class, 0, 'Mask class should be cleared');
    });
    
    // Test 6: Client event operations
    runner.addTest('Client Event Operations', async function() {
        const client = new VscpLevel2Client();
        await client.connect();
        
        // Test send event
        const testEvent = new VscpEvent({
            vscp_class: 10,
            vscp_type: 6,
            data: [0x01, 0x02, 0x03]
        });
        
        const sendResult = await client.send(testEvent);
        this.assertEqual(sendResult, VSCP_CONSTANTS.ERROR_SUCCESS, 'Send should succeed');
        
        // Test queue operations
        let countResponse = await client.getCount();
        this.assertEqual(countResponse.error, VSCP_CONSTANTS.ERROR_SUCCESS, 'Get count should succeed');
        this.assert(countResponse.count >= 0, 'Count should be non-negative');
        
        const clearResult = client.clear();
        this.assertEqual(clearResult, VSCP_CONSTANTS.ERROR_SUCCESS, 'Clear should succeed');
        
        countResponse = await client.getCount();
        this.assertEqual(countResponse.count, 0, 'Count should be 0 after clear');
        
        await client.disconnect();
    });
    
    // Test 7: Client information methods
    runner.addTest('Client Information Methods', async function() {
        const client = new VscpLevel2Client();
        await client.connect();
        
        // Test version
        const versionResponse = await client.getVersion();
        this.assertEqual(versionResponse.error, VSCP_CONSTANTS.ERROR_SUCCESS, 'Get version should succeed');
        this.assert(typeof versionResponse.version.major === 'number', 'Version major should be a number');
        
        // Test interfaces
        const interfacesResponse = await client.getInterfaces();
        this.assertEqual(interfacesResponse.error, VSCP_CONSTANTS.ERROR_SUCCESS, 'Get interfaces should succeed');
        this.assert(Array.isArray(interfacesResponse.interfaces), 'Interfaces should be an array');
        
        // Test capabilities
        const capabilitiesResponse = await client.getCapabilities();
        this.assertEqual(capabilitiesResponse.error, VSCP_CONSTANTS.ERROR_SUCCESS, 'Get capabilities should succeed');
        this.assert(typeof capabilitiesResponse.capabilities === 'number', 'Capabilities should be a number');
        
        await client.disconnect();
    });
    
    // Test 8: Configuration management
    runner.addTest('Configuration Management', async function() {
        const client = new VscpLevel2Client({
            interface: '/dev/test',
            flags: 0x1234,
            responseTimeout: 5000
        });
        
        // Test getConfigAsJson
        const configJson = client.getConfigAsJson();
        this.assert(typeof configJson === 'string', 'Config should be a string');
        
        const config = JSON.parse(configJson);
        this.assertEqual(config.interface, '/dev/test', 'Config should contain interface');
        this.assertEqual(config.flags, 0x1234, 'Config should contain flags');
        this.assertEqual(config.responseTimeout, 5000, 'Config should contain response timeout');
        
        // Test initFromJson
        const newConfig = {
            interface: '/dev/new',
            flags: 0x5678,
            responseTimeout: 3000
        };
        
        const initResult = client.initFromJson(JSON.stringify(newConfig));
        this.assert(initResult, 'Init from JSON should succeed');
        this.assertEqual(client.interface, '/dev/new', 'Interface should be updated');
        this.assertEqual(client.flags, 0x5678, 'Flags should be updated');
        this.assertEqual(client.responseTimeout, 3000, 'Response timeout should be updated');
    });
    
    // Test 9: Timeout management
    runner.addTest('Timeout Management', async function() {
        const client = new VscpLevel2Client();
        
        // Test connection timeout
        client.setConnectionTimeout(2000);
        this.assertEqual(client.getConnectionTimeout(), 2000, 'Connection timeout should be set');
        
        // Test response timeout
        client.setResponseTimeout(4000);
        this.assertEqual(client.getResponseTimeout(), 4000, 'Response timeout should be set');
        
        // Test negative timeout handling
        client.setConnectionTimeout(-100);
        this.assertEqual(client.getConnectionTimeout(), 0, 'Negative timeout should be set to 0');
    });
    
    // Test 10: Statistics tracking
    runner.addTest('Statistics Tracking', async function() {
        const client = new VscpLevel2Client();
        await client.connect();
        
        let stats = client.getStatistics();
        this.assertEqual(stats.eventsSent, 0, 'Initial events sent should be 0');
        this.assertEqual(stats.eventsReceived, 0, 'Initial events received should be 0');
        
        // Send an event and check stats
        const event = new VscpEvent({ vscp_class: 10, vscp_type: 6 });
        await client.send(event);
        
        stats = client.getStatistics();
        this.assertEqual(stats.eventsSent, 1, 'Events sent should be 1');
        
        // Inject and receive an event
        client.injectEvent(event);
        const receiveResponse = await client.receive(100);
        
        if (receiveResponse.error === VSCP_CONSTANTS.ERROR_SUCCESS) {
            stats = client.getStatistics();
            this.assertEqual(stats.eventsReceived, 1, 'Events received should be 1');
        }
        
        await client.disconnect();
    });
    
    await runner.run();
}

// Run tests
if (require.main === module) {
    runTests().catch(console.error);
}

module.exports = runTests;