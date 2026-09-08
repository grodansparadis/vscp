/**
 * Event Handling Example for VSCP Level 2 Client
 * Demonstrates event sending, receiving, and handling
 * 
 * Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project <info@vscp.org>
 * MIT License
 */

'use strict';

const { VscpLevel2Client, VscpEvent, VscpEventFilter, VSCP_CONSTANTS } = require('../lib');

async function eventHandlingExample() {
    console.log('=== VSCP Level 2 Client - Event Handling Example ===\n');
    
    try {
        // Create a new Level 2 client with debug enabled
        const client = new VscpLevel2Client({
            interface: '/dev/vscp0',
            flags: VSCP_CONSTANTS.FLAG_ENABLE_DEBUG,
            responseTimeout: 3000
        });
        
        // Set up event listeners
        client.on('connected', () => {
            console.log('📡 Client connected');
        });
        
        client.on('disconnected', () => {
            console.log('📡 Client disconnected');
        });
        
        client.on('eventSent', (event) => {
            console.log(`📤 Event sent - Class: ${event.vscp_class}, Type: ${event.vscp_type}`);
        });
        
        client.on('eventReceived', (event) => {
            console.log(`📥 Event received - Class: ${event.vscp_class}, Type: ${event.vscp_type}`);
            console.log(`   Data: [${event.data.join(', ')}]`);
        });
        
        client.on('error', (error) => {
            console.error('❌ Client error:', error.message);
        });
        
        console.log('1. Set up event listeners');
        
        // Initialize and connect
        await client.connect();
        
        // Create and set a filter for measurement events
        const filter = new VscpEventFilter();
        filter.setClassAndType(10, 0); // MEASUREMENT class, any type
        
        const filterResult = client.setFilter(filter);
        if (filterResult === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log('2. Set event filter for measurement events');
        }
        
        // Create different types of measurement events
        const events = [
            // Temperature event
            new VscpEvent({
                vscp_class: 10,
                vscp_type: 6,
                GUID: [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F],
                data: [0x89, 0x02, 0x00, 0x19]  // 25.0°C
            }),
            
            // Humidity event
            new VscpEvent({
                vscp_class: 10,
                vscp_type: 7,
                GUID: [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x10],
                data: [0x89, 0x02, 0x00, 0x3C]  // 60.0% RH
            }),
            
            // Pressure event
            new VscpEvent({
                vscp_class: 10,
                vscp_type: 8,
                GUID: [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x11],
                data: [0x89, 0x02, 0x03, 0xE8]  // 1000 hPa
            })
        ];
        
        console.log('3. Created test measurement events');
        
        // Set timestamps and send events
        for (let i = 0; i < events.length; i++) {
            const event = events[i];
            event.setCurrentTime();
            event.setPriority(VSCP_CONSTANTS.PRIORITY_NORMAL);
            
            console.log(`\\n4.${i + 1}. Sending event ${i + 1}/3:`);
            console.log(`     Class: ${event.vscp_class}, Type: ${event.vscp_type}`);
            console.log(`     Priority: ${event.getPriority()}`);
            console.log(`     Hard coded: ${event.isHardCoded()}`);
            console.log(`     Data: [${event.data.join(', ')}]`);
            
            const sendResult = await client.send(event);
            if (sendResult === VSCP_CONSTANTS.ERROR_SUCCESS) {
                console.log('     ✅ Sent successfully');
            } else {
                console.log(`     ❌ Send failed: ${sendResult}`);
            }
            
            // Inject event into receive queue for demonstration
            client.injectEvent(event);
            
            // Wait a bit between events
            await new Promise(resolve => setTimeout(resolve, 500));
        }
        
        console.log('\\n5. Attempting to receive events:');
        
        // Try to receive events
        for (let i = 0; i < 3; i++) {
            console.log(`\\n5.${i + 1}. Receiving event ${i + 1}/3:`);
            
            const receiveResponse = await client.receive(1000);
            if (receiveResponse.error === VSCP_CONSTANTS.ERROR_SUCCESS && receiveResponse.event) {
                const event = receiveResponse.event;
                console.log(`     ✅ Received event:`);
                console.log(`       Class: ${event.vscp_class}, Type: ${event.vscp_type}`);
                console.log(`       Timestamp: ${event.year}-${String(event.month).padStart(2, '0')}-${String(event.day).padStart(2, '0')} ${String(event.hour).padStart(2, '0')}:${String(event.minute).padStart(2, '0')}:${String(event.second).padStart(2, '0')}`);
                console.log(`       Data: [${event.data.join(', ')}]`);
                console.log(`       GUID: [${event.GUID.map(b => b.toString(16).padStart(2, '0')).join(', ')}]`);
            } else if (receiveResponse.error === VSCP_CONSTANTS.ERROR_TIMEOUT) {
                console.log('     ⏰ Receive timeout');
            } else {
                console.log(`     ❌ Receive error: ${receiveResponse.error}`);
            }
        }
        
        // Get final statistics
        console.log('\\n6. Final statistics:');
        const stats = client.getStatistics();
        console.log(`   Events sent: ${stats.eventsSent}`);
        console.log(`   Events received: ${stats.eventsReceived}`);
        console.log(`   Errors: ${stats.errors}`);
        
        // Create an event with JSON serialization
        console.log('\\n7. Testing JSON serialization:');
        const jsonEvent = new VscpEvent({
            vscp_class: 10,
            vscp_type: 6,
            data: [0x89, 0x02, 0x00, 0x20]  // 32.0°C
        });
        
        const eventJson = jsonEvent.toJSON();
        console.log('   Original event as JSON:', JSON.stringify(eventJson, null, 2));
        
        const restoredEvent = VscpEvent.fromJSON(eventJson);
        console.log('   Restored event class:', restoredEvent.vscp_class);
        console.log('   Restored event type:', restoredEvent.vscp_type);
        console.log('   Restored event data:', restoredEvent.data);
        
        // Disconnect
        await client.disconnect();
        
        console.log('\\n=== Event Handling Example Completed Successfully ===');
        
    } catch (error) {
        console.error('Error in event handling example:', error.message);
        console.error(error.stack);
        process.exit(1);
    }
}

// Run the example
if (require.main === module) {
    eventHandlingExample().catch(console.error);
}

module.exports = eventHandlingExample;