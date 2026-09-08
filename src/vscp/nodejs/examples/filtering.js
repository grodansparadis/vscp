/**
 * Event Filtering Example for VSCP Level 2 Client
 * Demonstrates how to set up and use event filters
 * 
 * Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project <info@vscp.org>
 * MIT License
 */

'use strict';

const { VscpLevel2Client, VscpEvent, VscpEventFilter, VSCP_CONSTANTS } = require('../lib');

async function filteringExample() {
    console.log('=== VSCP Level 2 Client - Event Filtering Example ===\n');
    
    try {
        // Create a new Level 2 client
        const client = new VscpLevel2Client({
            interface: '/dev/vscp0',
            flags: VSCP_CONSTANTS.FLAG_ENABLE_DEBUG,
            responseTimeout: 2000
        });
        
        console.log('1. Created VSCP Level 2 client with debug enabled');
        
        // Connect to the interface
        await client.connect();
        console.log('2. Connected to interface');
        
        // Example 1: Filter for specific class and type
        console.log('\\n=== Example 1: Class and Type Filter ===');
        
        const classTypeFilter = new VscpEventFilter();
        classTypeFilter.setClassAndType(10, 6); // MEASUREMENT class, TEMPERATURE type
        
        const filterResult1 = client.setFilter(classTypeFilter);
        if (filterResult1 === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log('3. Set filter for temperature measurements (Class 10, Type 6)');
        }
        
        // Create test events - some should match filter, some shouldn't
        const testEvents1 = [
            new VscpEvent({
                vscp_class: 10,  // MEASUREMENT - should match
                vscp_type: 6,    // TEMPERATURE - should match
                data: [0x89, 0x02, 0x00, 0x19]  // 25.0°C
            }),
            new VscpEvent({
                vscp_class: 10,  // MEASUREMENT - wrong type
                vscp_type: 7,    // HUMIDITY - should NOT match
                data: [0x89, 0x02, 0x00, 0x3C]  // 60.0% RH
            }),
            new VscpEvent({
                vscp_class: 20,  // INFORMATION - wrong class
                vscp_type: 6,    // should NOT match
                data: [0x01, 0x02, 0x03]
            }),
            new VscpEvent({
                vscp_class: 10,  // MEASUREMENT - should match
                vscp_type: 6,    // TEMPERATURE - should match
                data: [0x89, 0x02, 0x00, 0x1E]  // 30.0°C
            })
        ];
        
        console.log('4. Testing class and type filter with 4 events:');
        await testFilter(client, testEvents1, 'Class 10, Type 6');
        
        // Example 2: Priority filter
        console.log('\\n=== Example 2: Priority Filter ===');
        
        const priorityFilter = new VscpEventFilter();
        priorityFilter.filter_priority = VSCP_CONSTANTS.PRIORITY_HIGH;
        priorityFilter.mask_priority = 0xE0;  // Mask all priority bits
        
        const filterResult2 = client.setFilter(priorityFilter);
        if (filterResult2 === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log('5. Set filter for high priority events only');
        }
        
        const testEvents2 = [
            new VscpEvent({
                vscp_class: 10,
                vscp_type: 6,
                head: VSCP_CONSTANTS.PRIORITY_HIGH,  // Should match
                data: [0x01]
            }),
            new VscpEvent({
                vscp_class: 10,
                vscp_type: 6,
                head: VSCP_CONSTANTS.PRIORITY_NORMAL,  // Should NOT match
                data: [0x02]
            }),
            new VscpEvent({
                vscp_class: 10,
                vscp_type: 6,
                head: VSCP_CONSTANTS.PRIORITY_LOW,  // Should NOT match
                data: [0x03]
            }),
            new VscpEvent({
                vscp_class: 20,
                vscp_type: 1,
                head: VSCP_CONSTANTS.PRIORITY_HIGH,  // Should match (different class but high priority)
                data: [0x04]
            })
        ];
        
        console.log('6. Testing priority filter with 4 events:');
        await testFilter(client, testEvents2, 'High priority only');
        
        // Example 3: GUID filter
        console.log('\\n=== Example 3: GUID Filter ===');
        
        const guidFilter = new VscpEventFilter();
        // Filter for specific device GUID pattern
        const targetGuid = [0xAA, 0xBB, 0xCC, 0xDD, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00];
        const guidMask = [0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00];
        
        guidFilter.filter_GUID = targetGuid.slice();
        guidFilter.mask_GUID = guidMask.slice();
        
        const filterResult3 = client.setFilter(guidFilter);
        if (filterResult3 === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log('7. Set GUID filter for devices with GUID pattern AA:BB:CC:DD:xx:xx...');
        }
        
        const testEvents3 = [
            new VscpEvent({
                vscp_class: 10,
                vscp_type: 6,
                GUID: [0xAA, 0xBB, 0xCC, 0xDD, 0x01, 0x02, 0x03, 0x04,  // Should match
                       0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C],
                data: [0x01]
            }),
            new VscpEvent({
                vscp_class: 10,
                vscp_type: 6,
                GUID: [0xAA, 0xBB, 0xCC, 0xEE, 0x01, 0x02, 0x03, 0x04,  // Should NOT match
                       0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C],
                data: [0x02]
            }),
            new VscpEvent({
                vscp_class: 10,
                vscp_type: 6,
                GUID: [0xAA, 0xBB, 0xCC, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF,  // Should match
                       0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF],
                data: [0x03]
            })
        ];
        
        console.log('8. Testing GUID filter with 3 events:');
        await testFilter(client, testEvents3, 'GUID pattern AA:BB:CC:DD');
        
        // Example 4: Clear filter (allow all events)
        console.log('\\n=== Example 4: Clear Filter ===');
        
        const clearFilter = new VscpEventFilter();
        clearFilter.clear();
        
        const filterResult4 = client.setFilter(clearFilter);
        if (filterResult4 === VSCP_CONSTANTS.ERROR_SUCCESS) {
            console.log('9. Cleared all filters (allow all events)');
        }
        
        const testEvents4 = [
            new VscpEvent({ vscp_class: 10, vscp_type: 6, data: [0x01] }),  // Should match
            new VscpEvent({ vscp_class: 20, vscp_type: 1, data: [0x02] }),  // Should match
            new VscpEvent({ vscp_class: 30, vscp_type: 5, data: [0x03] })   // Should match
        ];
        
        console.log('10. Testing cleared filter with 3 events (all should match):');
        await testFilter(client, testEvents4, 'No filter (all events)');
        
        // Disconnect
        await client.disconnect();
        
        console.log('\\n=== Event Filtering Example Completed Successfully ===');
        
    } catch (error) {
        console.error('Error in filtering example:', error.message);
        console.error(error.stack);
        process.exit(1);
    }
}

/**
 * Helper function to test a filter with a set of events
 */
async function testFilter(client, events, description) {
    console.log(`   Testing filter: ${description}`);
    
    let matchedEvents = 0;
    let totalEvents = events.length;
    
    // Inject events into the client
    for (const event of events) {
        event.setCurrentTime();
        client.injectEvent(event);
    }
    
    // Try to receive events (they would be filtered by a real implementation)
    console.log(`   Injected ${totalEvents} test events`);
    
    // In a real implementation, the filter would be applied at the driver level
    // For this example, we'll simulate by checking events manually
    for (const event of events) {
        // Simulate filter checking (in real implementation this would be done by the driver)
        const wouldMatch = checkEventAgainstFilter(event, client._filter);
        
        console.log(`   Event (Class: ${event.vscp_class}, Type: ${event.vscp_type}): ${wouldMatch ? '✅ MATCH' : '❌ NO MATCH'}`);
        
        if (wouldMatch) {
            matchedEvents++;
        }
    }
    
    console.log(`   Result: ${matchedEvents}/${totalEvents} events matched the filter`);
}

/**
 * Helper function to simulate filter checking (for demonstration)
 * In real implementation, this would be done at the driver/daemon level
 */
function checkEventAgainstFilter(event, filter) {
    if (!filter) return true;  // No filter means all events pass
    
    // Check class filter
    if (filter.mask_class !== 0) {
        if ((event.vscp_class & filter.mask_class) !== (filter.filter_class & filter.mask_class)) {
            return false;
        }
    }
    
    // Check type filter  
    if (filter.mask_type !== 0) {
        if ((event.vscp_type & filter.mask_type) !== (filter.filter_type & filter.mask_type)) {
            return false;
        }
    }
    
    // Check priority filter
    if (filter.mask_priority !== 0) {
        const eventPriority = event.head & VSCP_CONSTANTS.HEADER_PRIORITY_MASK;
        const filterPriority = filter.filter_priority & filter.mask_priority;
        if ((eventPriority & filter.mask_priority) !== filterPriority) {
            return false;
        }
    }
    
    // Check GUID filter
    for (let i = 0; i < 16; i++) {
        if (filter.mask_GUID[i] !== 0) {
            if ((event.GUID[i] & filter.mask_GUID[i]) !== (filter.filter_GUID[i] & filter.mask_GUID[i])) {
                return false;
            }
        }
    }
    
    return true;  // Event passes all filter criteria
}

// Run the example
if (require.main === module) {
    filteringExample().catch(console.error);
}

module.exports = filteringExample;