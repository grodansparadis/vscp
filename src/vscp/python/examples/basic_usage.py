"""
Basic Usage Example for VSCP Level 2 Client
Demonstrates the core functionality of the VSCP Level 2 interface

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
    VSCPError,
    VSCPPriority,
    VSCPConstants
)


async def basic_usage_example():
    """Demonstrate basic usage of the VSCP Level 2 client"""
    print('=== VSCP Level 2 Client - Basic Usage Example (Python) ===\n')
    
    try:
        # Create a new Level 2 client
        client = VSCPLevel2Client(
            interface='/dev/vscp0',  # Example interface path
            flags=VSCPConstants.FLAG_ENABLE_DEBUG,
            response_timeout=5000
        )
        
        print('1. Created VSCP Level 2 client')
        
        # Initialize the client
        init_result = client.init('/dev/vscp0', VSCPConstants.FLAG_ENABLE_DEBUG, 5000)
        if init_result != VSCPError.SUCCESS:
            raise Exception(f'Init failed with error code: {init_result}')
        print('2. Initialized client successfully')
        
        # Connect to the interface
        connect_result = await client.connect()
        if connect_result != VSCPError.SUCCESS:
            raise Exception(f'Connect failed with error code: {connect_result}')
        print('3. Connected to interface successfully')
        
        # Check if connected
        print(f'4. Connection status: {"Connected" if client.is_connected() else "Not connected"}')
        
        # Get version information
        version_error, version_info = await client.get_version()
        if version_error == VSCPError.SUCCESS:
            print(f'5. Interface version: {version_info.major}.{version_info.minor}.{version_info.release}.{version_info.build}')
        
        # Get available interfaces
        interfaces_error, interfaces = await client.get_interfaces()
        if interfaces_error == VSCPError.SUCCESS:
            print(f'6. Available interfaces: {", ".join(interfaces)}')
        
        # Get capabilities
        capabilities_error, capabilities = await client.get_capabilities()
        if capabilities_error == VSCPError.SUCCESS:
            print(f'7. Capabilities: 0x{capabilities:x}')
        
        # Create a test event (Temperature measurement)
        test_event = VSCPEvent(
            vscp_class=10,  # MEASUREMENT
            vscp_type=6,    # TEMPERATURE
            guid=[0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F],
            data=[0x89, 0x02, 0x00, 0x19]  # 25.0 degrees Celsius in VSCP format
        )
        
        # Set current timestamp
        test_event.set_current_time()
        
        print('8. Created test temperature event')
        print(f'   Class: {test_event.vscp_class}, Type: {test_event.vscp_type}')
        print(f'   Data: {test_event.data}')
        
        # Send the event
        send_result = await client.send(test_event)
        if send_result == VSCPError.SUCCESS:
            print('9. Test event sent successfully')
        else:
            print(f'9. Failed to send event: {send_result}')
        
        # Get event count
        count_error, count = await client.get_count()
        if count_error == VSCPError.SUCCESS:
            print(f'10. Events in queue: {count}')
        
        # Clear the queue
        clear_result = client.clear()
        if clear_result == VSCPError.SUCCESS:
            print('11. Event queue cleared')
        
        # Get statistics
        stats = client.get_statistics()
        print('12. Client statistics:')
        print(f'    Events sent: {stats.events_sent}')
        print(f'    Events received: {stats.events_received}')
        print(f'    Errors: {stats.errors}')
        print(f'    Connect time: {stats.connect_time}')
        
        # Get configuration as JSON
        config = client.get_config_as_json()
        print('13. Current configuration:')
        print(config)
        
        # Test event serialization
        print('\\n14. Testing event serialization:')
        event_dict = test_event.to_dict()
        print(f'    Event as dict: vscp_class={event_dict["vscp_class"]}, vscp_type={event_dict["vscp_type"]}')
        
        event_json = test_event.to_json()
        restored_event = VSCPEvent.from_json(event_json)
        print(f'    Restored from JSON: vscp_class={restored_event.vscp_class}, vscp_type={restored_event.vscp_type}')
        
        # Test priority methods
        print('\\n15. Testing priority methods:')
        test_event.set_priority(VSCPPriority.HIGH >> 5)
        print(f'    Set priority to HIGH: {test_event.get_priority()}')
        
        test_event.set_priority(VSCPPriority.LOW >> 5)
        print(f'    Set priority to LOW: {test_event.get_priority()}')
        
        # Test hard coded flag
        print('\\n16. Testing hard coded flag:')
        print(f'    Initially hard coded: {test_event.is_hard_coded()}')
        test_event.set_hard_coded(True)
        print(f'    After setting to True: {test_event.is_hard_coded()}')
        
        # Disconnect
        disconnect_result = await client.disconnect()
        if disconnect_result == VSCPError.SUCCESS:
            print('\\n17. Disconnected successfully')
        
        print('\\n=== Basic Usage Example Completed Successfully ===')
        
    except Exception as error:
        print(f'Error in basic usage example: {error}')
        return 1
    
    return 0


if __name__ == '__main__':
    exit_code = asyncio.run(basic_usage_example())
    sys.exit(exit_code)