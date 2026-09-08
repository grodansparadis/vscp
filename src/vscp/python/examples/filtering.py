"""
Event Filtering Example for VSCP Level 2 Client
Demonstrates how to set up and use event filters

Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project <info@vscp.org>
MIT License
"""

import asyncio
import sys
import os

# Add the parent directory to the path so we can import vscp_level2
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from vscp_level2 import (
    VSCPLevel2Client,
    VSCPEvent,
    VSCPEventFilter,
    VSCPError,
    VSCPPriority,
    VSCPConstants
)


async def test_filter(client: VSCPLevel2Client, events: list, description: str):
    """Helper function to test a filter with a set of events"""
    print(f'   Testing filter: {description}')
    
    matched_events = 0
    total_events = len(events)
    
    # Inject events into the client
    for event in events:
        event.set_current_time()
        client.inject_event(event)
    
    print(f'   Injected {total_events} test events')
    
    # In a real implementation, the filter would be applied at the driver level
    # For this example, we'll simulate by checking events manually
    for event in events:
        # The filter check is done in client.inject_event() via _check_filter()
        # We'll just report what would happen
        would_match = client._check_filter(event)
        
        print(f'   Event (Class: {event.vscp_class}, Type: {event.vscp_type}): {"✅ MATCH" if would_match else "❌ NO MATCH"}')
        
        if would_match:
            matched_events += 1
    
    print(f'   Result: {matched_events}/{total_events} events matched the filter')


async def filtering_example():
    """Demonstrate event filtering with the VSCP Level 2 client"""
    print('=== VSCP Level 2 Client - Event Filtering Example (Python) ===\n')
    
    try:
        # Create a new Level 2 client
        client = VSCPLevel2Client(
            interface='/dev/vscp0',
            flags=VSCPConstants.FLAG_ENABLE_DEBUG,
            response_timeout=2000
        )
        
        print('1. Created VSCP Level 2 client with debug enabled')
        
        # Connect to the interface
        await client.connect()
        print('2. Connected to interface')
        
        # Example 1: Filter for specific class and type
        print('\\n=== Example 1: Class and Type Filter ===')
        
        class_type_filter = VSCPEventFilter()
        class_type_filter.set_class_and_type(10, 6)  # MEASUREMENT class, TEMPERATURE type
        
        filter_result1 = client.set_filter(class_type_filter)
        if filter_result1 == VSCPError.SUCCESS:
            print('3. Set filter for temperature measurements (Class 10, Type 6)')
        
        # Create test events - some should match filter, some shouldn't
        test_events1 = [
            VSCPEvent(
                vscp_class=10,  # MEASUREMENT - should match
                vscp_type=6,    # TEMPERATURE - should match
                data=[0x89, 0x02, 0x00, 0x19]  # 25.0°C
            ),
            VSCPEvent(
                vscp_class=10,  # MEASUREMENT - wrong type
                vscp_type=7,    # HUMIDITY - should NOT match
                data=[0x89, 0x02, 0x00, 0x3C]  # 60.0% RH
            ),
            VSCPEvent(
                vscp_class=20,  # INFORMATION - wrong class
                vscp_type=6,    # should NOT match
                data=[0x01, 0x02, 0x03]
            ),
            VSCPEvent(
                vscp_class=10,  # MEASUREMENT - should match
                vscp_type=6,    # TEMPERATURE - should match
                data=[0x89, 0x02, 0x00, 0x1E]  # 30.0°C
            )
        ]
        
        print('4. Testing class and type filter with 4 events:')
        await test_filter(client, test_events1, 'Class 10, Type 6')
        
        # Example 2: Priority filter
        print('\\n=== Example 2: Priority Filter ===')
        
        priority_filter = VSCPEventFilter()
        priority_filter.filter_priority = VSCPPriority.HIGH
        priority_filter.mask_priority = 0xE0  # Mask all priority bits
        
        filter_result2 = client.set_filter(priority_filter)
        if filter_result2 == VSCPError.SUCCESS:
            print('5. Set filter for high priority events only')
        
        test_events2 = [
            VSCPEvent(
                vscp_class=10,
                vscp_type=6,
                head=VSCPPriority.HIGH,  # Should match
                data=[0x01]
            ),
            VSCPEvent(
                vscp_class=10,
                vscp_type=6,
                head=VSCPPriority.NORMAL,  # Should NOT match
                data=[0x02]
            ),
            VSCPEvent(
                vscp_class=10,
                vscp_type=6,
                head=VSCPPriority.LOW,  # Should NOT match
                data=[0x03]
            ),
            VSCPEvent(
                vscp_class=20,
                vscp_type=1,
                head=VSCPPriority.HIGH,  # Should match (different class but high priority)
                data=[0x04]
            )
        ]
        
        print('6. Testing priority filter with 4 events:')
        await test_filter(client, test_events2, 'High priority only')
        
        # Example 3: GUID filter
        print('\\n=== Example 3: GUID Filter ===')
        
        guid_filter = VSCPEventFilter()
        # Filter for specific device GUID pattern
        target_guid = [0xAA, 0xBB, 0xCC, 0xDD, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        guid_mask = [0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        guid_filter.filter_guid = target_guid.copy()
        guid_filter.mask_guid = guid_mask.copy()
        
        filter_result3 = client.set_filter(guid_filter)
        if filter_result3 == VSCPError.SUCCESS:
            print('7. Set GUID filter for devices with GUID pattern AA:BB:CC:DD:xx:xx...')
        
        test_events3 = [
            VSCPEvent(
                vscp_class=10,
                vscp_type=6,
                guid=[0xAA, 0xBB, 0xCC, 0xDD, 0x01, 0x02, 0x03, 0x04,  # Should match
                      0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C],
                data=[0x01]
            ),
            VSCPEvent(
                vscp_class=10,
                vscp_type=6,
                guid=[0xAA, 0xBB, 0xCC, 0xEE, 0x01, 0x02, 0x03, 0x04,  # Should NOT match
                      0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C],
                data=[0x02]
            ),
            VSCPEvent(
                vscp_class=10,
                vscp_type=6,
                guid=[0xAA, 0xBB, 0xCC, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF,  # Should match
                      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF],
                data=[0x03]
            )
        ]
        
        print('8. Testing GUID filter with 3 events:')
        await test_filter(client, test_events3, 'GUID pattern AA:BB:CC:DD')
        
        # Example 4: Clear filter (allow all events)
        print('\\n=== Example 4: Clear Filter ===')
        
        clear_filter = VSCPEventFilter()
        clear_filter.clear()
        
        filter_result4 = client.set_filter(clear_filter)
        if filter_result4 == VSCPError.SUCCESS:
            print('9. Cleared all filters (allow all events)')
        
        test_events4 = [
            VSCPEvent(vscp_class=10, vscp_type=6, data=[0x01]),  # Should match
            VSCPEvent(vscp_class=20, vscp_type=1, data=[0x02]),  # Should match
            VSCPEvent(vscp_class=30, vscp_type=5, data=[0x03])   # Should match
        ]
        
        print('10. Testing cleared filter with 3 events (all should match):')
        await test_filter(client, test_events4, 'No filter (all events)')
        
        # Example 5: Complex filter (multiple criteria)
        print('\\n=== Example 5: Complex Filter ===')
        
        complex_filter = VSCPEventFilter()
        complex_filter.set_class_and_type(10, 6)  # Temperature measurements
        complex_filter.filter_priority = VSCPPriority.NORMAL
        complex_filter.mask_priority = 0xE0
        
        filter_result5 = client.set_filter(complex_filter)
        if filter_result5 == VSCPError.SUCCESS:
            print('11. Set complex filter: Temperature measurements with normal priority')
        
        test_events5 = [
            VSCPEvent(
                vscp_class=10, vscp_type=6,
                head=VSCPPriority.NORMAL,  # Should match (class, type, and priority)
                data=[0x01]
            ),
            VSCPEvent(
                vscp_class=10, vscp_type=6,
                head=VSCPPriority.HIGH,  # Should NOT match (wrong priority)
                data=[0x02]
            ),
            VSCPEvent(
                vscp_class=10, vscp_type=7,
                head=VSCPPriority.NORMAL,  # Should NOT match (wrong type)
                data=[0x03]
            ),
            VSCPEvent(
                vscp_class=20, vscp_type=6,
                head=VSCPPriority.NORMAL,  # Should NOT match (wrong class)
                data=[0x04]
            )
        ]
        
        print('12. Testing complex filter with 4 events:')
        await test_filter(client, test_events5, 'Class 10, Type 6, Normal priority')
        
        # Demonstrate filter object methods
        print('\\n=== Filter Object Methods ===')
        
        demo_filter = VSCPEventFilter()
        print(f'13. New filter - Class filter: {demo_filter.filter_class}, mask: {demo_filter.mask_class}')
        
        demo_filter.set_class_and_type(15, 25)
        print(f'14. After set_class_and_type(15, 25) - Class filter: {demo_filter.filter_class}, mask: {demo_filter.mask_class}')
        print(f'    Type filter: {demo_filter.filter_type}, mask: {demo_filter.mask_type}')
        
        demo_filter.clear()
        print(f'15. After clear() - Class filter: {demo_filter.filter_class}, mask: {demo_filter.mask_class}')
        
        # Disconnect
        await client.disconnect()
        
        print('\\n=== Event Filtering Example Completed Successfully ===')
        
    except Exception as error:
        print(f'Error in filtering example: {error}')
        import traceback
        traceback.print_exc()
        return 1
    
    return 0


if __name__ == '__main__':
    exit_code = asyncio.run(filtering_example())
    sys.exit(exit_code)