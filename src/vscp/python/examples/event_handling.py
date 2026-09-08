"""
Event Handling Example for VSCP Level 2 Client
Demonstrates event sending, receiving, and handling

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


async def event_handling_example():
    """Demonstrate event handling with the VSCP Level 2 client"""
    print('=== VSCP Level 2 Client - Event Handling Example (Python) ===\n')
    
    try:
        # Create a new Level 2 client with debug enabled
        client = VSCPLevel2Client(
            interface='/dev/vscp0',
            flags=VSCPConstants.FLAG_ENABLE_DEBUG,
            response_timeout=3000
        )
        
        print('1. Created VSCP Level 2 client with debug enabled')
        
        # Initialize and connect
        await client.connect()
        print('2. Connected to interface')
        
        # Create and set a filter for measurement events
        filter_obj = VSCPEventFilter()
        filter_obj.set_class_and_type(10, 0)  # MEASUREMENT class, any type
        
        filter_result = client.set_filter(filter_obj)
        if filter_result == VSCPError.SUCCESS:
            print('3. Set event filter for measurement events')
        
        # Create different types of measurement events
        events = [
            # Temperature event
            VSCPEvent(
                vscp_class=10,
                vscp_type=6,
                guid=[0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F],
                data=[0x89, 0x02, 0x00, 0x19]  # 25.0°C
            ),
            
            # Humidity event
            VSCPEvent(
                vscp_class=10,
                vscp_type=7,
                guid=[0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x10],
                data=[0x89, 0x02, 0x00, 0x3C]  # 60.0% RH
            ),
            
            # Pressure event
            VSCPEvent(
                vscp_class=10,
                vscp_type=8,
                guid=[0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x11],
                data=[0x89, 0x02, 0x03, 0xE8]  # 1000 hPa
            )
        ]
        
        print('4. Created test measurement events')
        
        # Set timestamps and send events
        for i, event in enumerate(events):
            event.set_current_time()
            event.set_priority(VSCPPriority.NORMAL >> 5)
            
            print(f'\\n5.{i + 1}. Sending event {i + 1}/3:')
            print(f'     Class: {event.vscp_class}, Type: {event.vscp_type}')
            print(f'     Priority: {event.get_priority()}')
            print(f'     Hard coded: {event.is_hard_coded()}')
            print(f'     Data: {event.data}')
            
            send_result = await client.send(event)
            if send_result == VSCPError.SUCCESS:
                print('     ✅ Sent successfully')
            else:
                print(f'     ❌ Send failed: {send_result}')
            
            # Inject event into receive queue for demonstration
            client.inject_event(event)
            
            # Wait a bit between events
            await asyncio.sleep(0.5)
        
        print('\\n6. Attempting to receive events:')
        
        # Try to receive events
        for i in range(3):
            print(f'\\n6.{i + 1}. Receiving event {i + 1}/3:')
            
            receive_error, event = await client.receive(1000)
            if receive_error == VSCPError.SUCCESS and event:
                print(f'     ✅ Received event:')
                print(f'       Class: {event.vscp_class}, Type: {event.vscp_type}')
                print(f'       Timestamp: {event.year}-{event.month:02d}-{event.day:02d} {event.hour:02d}:{event.minute:02d}:{event.second:02d}')
                print(f'       Data: {event.data}')
                print(f'       GUID: {[f"{b:02x}" for b in event.guid]}')
            elif receive_error == VSCPError.TIMEOUT:
                print('     ⏰ Receive timeout')
            else:
                print(f'     ❌ Receive error: {receive_error}')
        
        # Get final statistics
        print('\\n7. Final statistics:')
        stats = client.get_statistics()
        print(f'   Events sent: {stats.events_sent}')
        print(f'   Events received: {stats.events_received}')
        print(f'   Errors: {stats.errors}')
        
        # Create an event with different serialization methods
        print('\\n8. Testing serialization methods:')
        json_event = VSCPEvent(
            vscp_class=10,
            vscp_type=6,
            data=[0x89, 0x02, 0x00, 0x20]  # 32.0°C
        )
        
        # Test dictionary conversion
        event_dict = json_event.to_dict()
        print(f'   Original event as dict: class={event_dict["vscp_class"]}, type={event_dict["vscp_type"]}')
        
        restored_from_dict = VSCPEvent.from_dict(event_dict)
        print(f'   Restored from dict: class={restored_from_dict.vscp_class}, type={restored_from_dict.vscp_type}')
        
        # Test JSON conversion
        event_json = json_event.to_json()
        print(f'   Event as JSON length: {len(event_json)} characters')
        
        restored_from_json = VSCPEvent.from_json(event_json)
        print(f'   Restored from JSON: class={restored_from_json.vscp_class}, type={restored_from_json.vscp_type}')
        print(f'   Restored data: {restored_from_json.data}')
        
        # Test priority and flags
        print('\\n9. Testing priority and flag methods:')
        test_event = VSCPEvent(vscp_class=10, vscp_type=6)
        
        priorities = [
            ('HIGH', VSCPPriority.HIGH),
            ('NORMAL', VSCPPriority.NORMAL), 
            ('MEDIUM', VSCPPriority.MEDIUM),
            ('LOW', VSCPPriority.LOW)
        ]
        
        for name, priority in priorities:
            test_event.set_priority(priority >> 5)
            print(f'   Priority {name}: {test_event.get_priority()} (head: 0x{test_event.head:04x})')
        
        # Test hard coded flag
        print(f'   Initially hard coded: {test_event.is_hard_coded()}')
        test_event.set_hard_coded(True)
        print(f'   After setting hard coded: {test_event.is_hard_coded()} (head: 0x{test_event.head:04x})')
        test_event.set_hard_coded(False)
        print(f'   After clearing hard coded: {test_event.is_hard_coded()} (head: 0x{test_event.head:04x})')
        
        # Disconnect
        await client.disconnect()
        
        print('\\n=== Event Handling Example Completed Successfully ===')
        
    except Exception as error:
        print(f'Error in event handling example: {error}')
        import traceback
        traceback.print_exc()
        return 1
    
    return 0


if __name__ == '__main__':
    exit_code = asyncio.run(event_handling_example())
    sys.exit(exit_code)