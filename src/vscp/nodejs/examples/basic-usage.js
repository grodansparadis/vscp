/**
 * Basic Usage Example for VSCP Level 2 Client
 * Demonstrates the core functionality of the VSCP Level 2 interface
 * 
 * Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project <info@vscp.org>
 * MIT License
 */

'use strict';

const { VscpLevel2Client, VscpEvent, VSCP_CONSTANTS } = require('../lib');

async function basicUsageExample() {
    console.log('=== VSCP Level 2 Client - Basic Usage Example ===\n');
    
    try {
        // Create a new Level 2 client
        const client = new VscpLevel2Client({
            interface: '/dev/vscp0',  // Example interface path
            flags: VSCP_CONSTANTS.FLAG_ENABLE_DEBUG,
            responseTimeout: 5000
        });
        
        console.log('1. Created VSCP Level 2 client');
        
        // Initialize the client
        const initResult = client.init('/dev/vscp0', VSCP_CONSTANTS.FLAG_ENABLE_DEBUG, 5000);
        if (initResult !== VSCP_CONSTANTS.ERROR_SUCCESS) {
            throw new Error(`Init failed with error code: ${initResult}`);
        }
        console.log('2. Initialized client successfully');
        
        // Connect to the interface
        const connectResult = await client.connect();
        if (connectResult !== VSCP_CONSTANTS.ERROR_SUCCESS) {
            throw new Error(`Connect failed with error code: ${connectResult}`);
        }
        console.log('3. Connected to interface successfully');
        
        // Check if connected
        console.log(`4. Connection status: ${client.isConnected() ? 'Connected' : 'Not connected'}`);
        
        // Get version information
        const versionResponse = await client.getVersion();
        if (versionResponse.error === VSCP_CONSTANTS.ERROR_SUCCESS) {
            const v = versionResponse.version;
            console.log(`5. Interface version: ${v.major}.${v.minor}.${v.release}.${v.build}`);
        }
        
        // Get available interfaces
        const interfacesResponse = await client.getInterfaces();
        if (interfacesResponse.error === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log(`6. Available interfaces: ${interfacesResponse.interfaces.join(', ')}`);
        }
        
        // Get capabilities
        const capabilitiesResponse = await client.getCapabilities();
        if (capabilitiesResponse.error === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log(`7. Capabilities: 0x${capabilitiesResponse.capabilities.toString(16)}`);
        }
        
        // Create a test event (Temperature measurement)
        const testEvent = new VscpEvent({
            vscp_class: 10,  // MEASUREMENT
            vscp_type: 6,    // TEMPERATURE
            priority: VSCP_CONSTANTS.PRIORITY_NORMAL,
            GUID: [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F],
            data: [0x89, 0x02, 0x00, 0x19]  // 25.0 degrees Celsius in VSCP format
        });
        
        // Set current timestamp
        testEvent.setCurrentTime();
        
        console.log('8. Created test temperature event');
        console.log(`   Class: ${testEvent.vscp_class}, Type: ${testEvent.vscp_type}`);
        console.log(`   Data: [${testEvent.data.join(', ')}]`);
        
        // Send the event
        const sendResult = await client.send(testEvent);
        if (sendResult === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log('9. Test event sent successfully');
        } else {
            console.error(`9. Failed to send event: ${sendResult}`);
        }
        
        // Get event count
        const countResponse = await client.getCount();
        if (countResponse.error === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log(`10. Events in queue: ${countResponse.count}`);
        }
        
        // Clear the queue
        const clearResult = client.clear();
        if (clearResult === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log('11. Event queue cleared');
        }
        
        // Get statistics
        const stats = client.getStatistics();
        console.log('12. Client statistics:');
        console.log(`    Events sent: ${stats.eventsSent}`);
        console.log(`    Events received: ${stats.eventsReceived}`);
        console.log(`    Errors: ${stats.errors}`);
        console.log(`    Connect time: ${stats.connectTime}`);
        
        // Get configuration as JSON
        const config = client.getConfigAsJson();
        console.log('13. Current configuration:');
        console.log(config);
        
        // Disconnect
        const disconnectResult = await client.disconnect();
        if (disconnectResult === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log('14. Disconnected successfully');
        }
        
        console.log('\n=== Basic Usage Example Completed Successfully ===');
        
    } catch (error) {
        console.error('Error in basic usage example:', error.message);
        process.exit(1);
    }
}

// Run the example
if (require.main === module) {
    basicUsageExample().catch(console.error);
}

module.exports = basicUsageExample;