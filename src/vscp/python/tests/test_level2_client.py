"""
Test Suite for VSCP Level 2 Client
Comprehensive tests to verify the Level 2 interface functionality

Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project <info@vscp.org>
MIT License
"""

import asyncio
import pytest
import json
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
    VSCPConstants,
    VersionInfo,
    Statistics,
    create_client,
    create_event,
    create_filter
)


class TestVSCPEvent:
    """Test the VSCPEvent class"""
    
    def test_event_creation(self):
        """Test basic event creation"""
        event = VSCPEvent(
            vscp_class=10,
            vscp_type=6,
            data=[0x89, 0x02, 0x00, 0x19]
        )
        
        assert event.vscp_class == 10
        assert event.vscp_type == 6
        assert event.data == [0x89, 0x02, 0x00, 0x19]
        assert event.size_data == 4
        assert len(event.guid) == 16
    
    def test_event_priority_methods(self):
        """Test priority getter/setter methods"""
        event = VSCPEvent()
        
        # Test setting priority
        event.set_priority(VSCPPriority.HIGH >> 5)
        assert event.get_priority() == 0
        
        event.set_priority(VSCPPriority.LOW >> 5)
        assert event.get_priority() == 7
        
        event.set_priority(VSCPPriority.NORMAL >> 5)
        assert event.get_priority() == 3
    
    def test_event_hard_coded_flag(self):
        """Test hard coded flag methods"""
        event = VSCPEvent()
        
        # Initially should not be hard coded
        assert not event.is_hard_coded()
        
        # Set hard coded
        event.set_hard_coded(True)
        assert event.is_hard_coded()
        
        # Clear hard coded
        event.set_hard_coded(False)
        assert not event.is_hard_coded()
    
    def test_event_timestamp(self):
        """Test timestamp methods"""
        event = VSCPEvent()
        
        # Initially should have no timestamp
        assert event.year == 0
        
        # Set current time
        event.set_current_time()
        
        # Should now have valid timestamp
        assert event.year >= 2025
        assert 1 <= event.month <= 12
        assert 1 <= event.day <= 31
        assert 0 <= event.hour <= 23
        assert 0 <= event.minute <= 59
        assert 0 <= event.second <= 59
    
    def test_event_serialization(self):
        """Test event serialization to/from dict and JSON"""
        original_event = VSCPEvent(
            vscp_class=20,
            vscp_type=3,
            guid=[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
            data=[0xAA, 0xBB, 0xCC]
        )
        
        # Test dictionary conversion
        event_dict = original_event.to_dict()
        assert isinstance(event_dict, dict)
        assert event_dict['vscp_class'] == 20
        assert event_dict['vscp_type'] == 3
        assert len(event_dict['guid']) == 16
        
        restored_from_dict = VSCPEvent.from_dict(event_dict)
        assert restored_from_dict.vscp_class == original_event.vscp_class
        assert restored_from_dict.vscp_type == original_event.vscp_type
        assert restored_from_dict.data == original_event.data
        
        # Test JSON conversion
        event_json = original_event.to_json()
        assert isinstance(event_json, str)
        
        restored_from_json = VSCPEvent.from_json(event_json)
        assert restored_from_json.vscp_class == original_event.vscp_class
        assert restored_from_json.vscp_type == original_event.vscp_type
        assert restored_from_json.data == original_event.data


class TestVSCPEventFilter:
    """Test the VSCPEventFilter class"""
    
    def test_filter_creation(self):
        """Test basic filter creation"""
        filter_obj = VSCPEventFilter()
        
        # Initially should be clear
        assert filter_obj.filter_class == 0
        assert filter_obj.mask_class == 0
        assert filter_obj.filter_type == 0
        assert filter_obj.mask_type == 0
        assert len(filter_obj.filter_guid) == 16
        assert len(filter_obj.mask_guid) == 16
    
    def test_filter_class_and_type(self):
        """Test setting class and type filter"""
        filter_obj = VSCPEventFilter()
        
        filter_obj.set_class_and_type(10, 6)
        
        assert filter_obj.filter_class == 10
        assert filter_obj.mask_class == 0xFFFF
        assert filter_obj.filter_type == 6
        assert filter_obj.mask_type == 0xFFFF
    
    def test_filter_clear(self):
        """Test clearing filter"""
        filter_obj = VSCPEventFilter()
        
        # Set some values
        filter_obj.set_class_and_type(10, 6)
        filter_obj.filter_priority = 0x20
        filter_obj.mask_priority = 0xE0
        
        # Clear filter
        filter_obj.clear()
        
        # Should be back to defaults
        assert filter_obj.filter_class == 0
        assert filter_obj.mask_class == 0
        assert filter_obj.filter_type == 0
        assert filter_obj.mask_type == 0
        assert filter_obj.filter_priority == 0
        assert filter_obj.mask_priority == 0


class TestVSCPLevel2Client:
    """Test the VSCPLevel2Client class"""
    
    def test_client_creation(self):
        """Test basic client creation"""
        client = VSCPLevel2Client(
            interface='/dev/test',
            flags=VSCPConstants.FLAG_ENABLE_DEBUG
        )
        
        assert client.interface == '/dev/test'
        assert client.flags == VSCPConstants.FLAG_ENABLE_DEBUG
        assert not client.is_connected()
    
    def test_client_initialization(self):
        """Test client initialization"""
        client = VSCPLevel2Client()
        
        result = client.init('/dev/test2', 0x1234, 5000)
        assert result == VSCPError.SUCCESS
        assert client.interface == '/dev/test2'
        assert client.flags == 0x1234
        assert client.response_timeout == 5000
    
    @pytest.mark.asyncio
    async def test_client_connection(self):
        """Test client connection and disconnection"""
        client = VSCPLevel2Client()
        
        # Connect
        connect_result = await client.connect()
        assert connect_result == VSCPError.SUCCESS
        assert client.is_connected()
        
        # Disconnect
        disconnect_result = await client.disconnect()
        assert disconnect_result == VSCPError.SUCCESS
        assert not client.is_connected()
    
    @pytest.mark.asyncio
    async def test_client_send_event(self):
        """Test sending events"""
        client = VSCPLevel2Client()
        await client.connect()
        
        # Create test event
        event = VSCPEvent(
            vscp_class=10,
            vscp_type=6,
            data=[0x01, 0x02, 0x03]
        )
        
        # Send event
        send_result = await client.send(event)
        assert send_result == VSCPError.SUCCESS
        
        # Check statistics
        stats = client.get_statistics()
        assert stats.events_sent == 1
        
        await client.disconnect()
    
    @pytest.mark.asyncio
    async def test_client_receive_event(self):
        """Test receiving events"""
        client = VSCPLevel2Client()
        await client.connect()
        
        # Create and inject test event
        event = VSCPEvent(
            vscp_class=10,
            vscp_type=6,
            data=[0x01, 0x02, 0x03]
        )
        client.inject_event(event)
        
        # Receive event
        receive_error, received_event = await client.receive(1000)
        assert receive_error == VSCPError.SUCCESS
        assert received_event is not None
        assert received_event.vscp_class == 10
        assert received_event.vscp_type == 6
        
        # Check statistics
        stats = client.get_statistics()
        assert stats.events_received == 1
        
        await client.disconnect()
    
    @pytest.mark.asyncio
    async def test_client_receive_timeout(self):
        """Test receive timeout"""
        client = VSCPLevel2Client()
        await client.connect()
        
        # Try to receive without any events
        receive_error, received_event = await client.receive(100)
        assert receive_error == VSCPError.TIMEOUT
        assert received_event is None
        
        await client.disconnect()
    
    def test_client_filter(self):
        """Test setting event filter"""
        client = VSCPLevel2Client()
        
        filter_obj = VSCPEventFilter()
        filter_obj.set_class_and_type(10, 6)
        
        # Should fail when not connected
        result = client.set_filter(filter_obj)
        assert result == VSCPError.NOT_CONNECTED
    
    @pytest.mark.asyncio
    async def test_client_filter_connected(self):
        """Test setting event filter when connected"""
        client = VSCPLevel2Client()
        await client.connect()
        
        filter_obj = VSCPEventFilter()
        filter_obj.set_class_and_type(10, 6)
        
        result = client.set_filter(filter_obj)
        assert result == VSCPError.SUCCESS
        
        await client.disconnect()
    
    @pytest.mark.asyncio
    async def test_client_queue_operations(self):
        """Test queue operations"""
        client = VSCPLevel2Client()
        await client.connect()
        
        # Initially queue should be empty
        count_error, count = await client.get_count()
        assert count_error == VSCPError.SUCCESS
        assert count == 0
        
        # Inject some events
        for i in range(3):
            event = VSCPEvent(vscp_class=10, vscp_type=i, data=[i])
            client.inject_event(event)
        
        # Check count
        count_error, count = await client.get_count()
        assert count_error == VSCPError.SUCCESS
        assert count == 3
        
        # Clear queue
        clear_result = client.clear()
        assert clear_result == VSCPError.SUCCESS
        
        # Should be empty again
        count_error, count = await client.get_count()
        assert count_error == VSCPError.SUCCESS
        assert count == 0
        
        await client.disconnect()
    
    @pytest.mark.asyncio
    async def test_client_version_info(self):
        """Test getting version information"""
        client = VSCPLevel2Client()
        await client.connect()
        
        version_error, version_info = await client.get_version()
        assert version_error == VSCPError.SUCCESS
        assert isinstance(version_info, VersionInfo)
        assert version_info.major == 15
        assert version_info.minor == 0
        assert version_info.release == 3
        assert version_info.build == 0
        
        await client.disconnect()
    
    @pytest.mark.asyncio
    async def test_client_interfaces(self):
        """Test getting interface list"""
        client = VSCPLevel2Client(interface='/dev/test')
        await client.connect()
        
        interfaces_error, interfaces = await client.get_interfaces()
        assert interfaces_error == VSCPError.SUCCESS
        assert isinstance(interfaces, list)
        assert '/dev/test' in interfaces
        
        await client.disconnect()
    
    @pytest.mark.asyncio
    async def test_client_capabilities(self):
        """Test getting capabilities"""
        client = VSCPLevel2Client()
        await client.connect()
        
        capabilities_error, capabilities = await client.get_capabilities()
        assert capabilities_error == VSCPError.SUCCESS
        assert isinstance(capabilities, int)
        assert capabilities == 0x00000001
        
        await client.disconnect()
    
    def test_client_configuration(self):
        """Test configuration management"""
        client = VSCPLevel2Client(
            interface='/dev/test',
            flags=0x1234,
            response_timeout=5000
        )
        
        # Test getConfigAsJson
        config_json = client.get_config_as_json()
        assert isinstance(config_json, str)
        
        config = json.loads(config_json)
        assert config['interface'] == '/dev/test'
        assert config['flags'] == 0x1234
        assert config['response_timeout'] == 5000
        
        # Test initFromJson
        new_config = {
            'interface': '/dev/new',
            'flags': 0x5678,
            'response_timeout': 3000
        }
        
        init_result = client.init_from_json(json.dumps(new_config))
        assert init_result
        assert client.interface == '/dev/new'
        assert client.flags == 0x5678
        assert client.response_timeout == 3000
    
    def test_client_timeouts(self):
        """Test timeout management"""
        client = VSCPLevel2Client()
        
        # Test connection timeout
        client.set_connection_timeout(2000)
        assert client.get_connection_timeout() == 2000
        
        # Test response timeout
        client.set_response_timeout(4000)
        assert client.get_response_timeout() == 4000
        
        # Test negative timeout handling
        client.set_connection_timeout(-100)
        assert client.get_connection_timeout() == 0
    
    @pytest.mark.asyncio
    async def test_client_statistics(self):
        """Test statistics tracking"""
        client = VSCPLevel2Client()
        await client.connect()
        
        stats = client.get_statistics()
        assert stats.events_sent == 0
        assert stats.events_received == 0
        assert stats.errors == 0
        assert stats.connect_time is not None
        
        # Send an event
        event = VSCPEvent(vscp_class=10, vscp_type=6)
        await client.send(event)
        
        stats = client.get_statistics()
        assert stats.events_sent == 1
        
        # Inject and receive an event
        client.inject_event(event)
        receive_error, received_event = await client.receive(100)
        
        if receive_error == VSCPError.SUCCESS:
            stats = client.get_statistics()
            assert stats.events_received == 1
        
        await client.disconnect()


class TestFactoryFunctions:
    """Test factory functions"""
    
    def test_create_client(self):
        """Test create_client factory function"""
        client = create_client(interface='/dev/factory')
        assert isinstance(client, VSCPLevel2Client)
        assert client.interface == '/dev/factory'
    
    def test_create_event(self):
        """Test create_event factory function"""
        event = create_event(vscp_class=15, vscp_type=25)
        assert isinstance(event, VSCPEvent)
        assert event.vscp_class == 15
        assert event.vscp_type == 25
    
    def test_create_filter(self):
        """Test create_filter factory function"""
        filter_obj = create_filter()
        assert isinstance(filter_obj, VSCPEventFilter)


class TestConstants:
    """Test VSCP constants and enums"""
    
    def test_error_codes(self):
        """Test VSCPError enum values"""
        assert VSCPError.SUCCESS == 0
        assert VSCPError.GENERAL == -1
        assert VSCPError.INVALID_HANDLE == -2
        assert VSCPError.TIMEOUT == -4
        assert VSCPError.NOT_CONNECTED == -5
    
    def test_priority_values(self):
        """Test VSCPPriority enum values"""
        assert VSCPPriority.PRIORITY_0 == 0x00
        assert VSCPPriority.PRIORITY_7 == 0xE0
        assert VSCPPriority.HIGH == 0x00
        assert VSCPPriority.LOW == 0xE0
    
    def test_constants_values(self):
        """Test VSCPConstants values"""
        assert VSCPConstants.DEFAULT_TCP_PORT == 9598
        assert VSCPConstants.MAX_DATA == 512
        assert VSCPConstants.SIZE_GUID == 16
        assert VSCPConstants.FLAG_ENABLE_DEBUG == 0x80000000


# Simple test runner for when pytest is not available
async def run_simple_tests():
    """Run simple tests without pytest"""
    print('=== VSCP Level 2 Client - Test Suite (Python) ===\n')
    
    tests_passed = 0
    tests_failed = 0
    
    def assert_equal(actual, expected, message=""):
        nonlocal tests_passed, tests_failed
        if actual == expected:
            tests_passed += 1
            print(f"✅ PASS: {message}")
        else:
            tests_failed += 1
            print(f"❌ FAIL: {message} - Expected: {expected}, Got: {actual}")
    
    def assert_true(condition, message=""):
        assert_equal(condition, True, message)
    
    # Test 1: Event creation
    print("Test 1: Event creation")
    event = VSCPEvent(vscp_class=10, vscp_type=6, data=[1, 2, 3])
    assert_equal(event.vscp_class, 10, "Event class should be 10")
    assert_equal(event.vscp_type, 6, "Event type should be 6")
    assert_equal(len(event.data), 3, "Event data length should be 3")
    
    # Test 2: Priority methods
    print("\\nTest 2: Priority methods")
    event.set_priority(VSCPPriority.HIGH >> 5)
    assert_equal(event.get_priority(), 0, "Priority should be 0 (HIGH)")
    
    # Test 3: Client creation
    print("\\nTest 3: Client creation")
    client = VSCPLevel2Client(interface='/dev/test')
    assert_equal(client.interface, '/dev/test', "Interface should be set")
    assert_true(not client.is_connected(), "Should not be connected initially")
    
    # Test 4: Connection
    print("\\nTest 4: Connection")
    connect_result = await client.connect()
    assert_equal(connect_result, VSCPError.SUCCESS, "Connect should succeed")
    assert_true(client.is_connected(), "Should be connected after connect()")
    
    # Test 5: Send event
    print("\\nTest 5: Send event")
    send_result = await client.send(event)
    assert_equal(send_result, VSCPError.SUCCESS, "Send should succeed")
    
    # Test 6: Statistics
    print("\\nTest 6: Statistics")
    stats = client.get_statistics()
    assert_equal(stats.events_sent, 1, "Should have sent 1 event")
    
    # Test 7: Version
    print("\\nTest 7: Version")
    version_error, version_info = await client.get_version()
    assert_equal(version_error, VSCPError.SUCCESS, "Get version should succeed")
    assert_equal(version_info.major, 15, "Major version should be 15")
    
    # Test 8: Filter
    print("\\nTest 8: Filter")
    filter_obj = VSCPEventFilter()
    filter_obj.set_class_and_type(10, 6)
    assert_equal(filter_obj.filter_class, 10, "Filter class should be 10")
    assert_equal(filter_obj.mask_class, 0xFFFF, "Mask class should be 0xFFFF")
    
    # Test 9: Event injection and receive
    print("\\nTest 9: Event injection and receive")
    client.inject_event(event)
    receive_error, received_event = await client.receive(100)
    assert_equal(receive_error, VSCPError.SUCCESS, "Receive should succeed")
    assert_true(received_event is not None, "Should receive an event")
    
    # Test 10: Disconnect
    print("\\nTest 10: Disconnect")
    disconnect_result = await client.disconnect()
    assert_equal(disconnect_result, VSCPError.SUCCESS, "Disconnect should succeed")
    assert_true(not client.is_connected(), "Should not be connected after disconnect")
    
    print(f"\\n=== Test Results: {tests_passed} passed, {tests_failed} failed ===")
    
    return tests_failed == 0


if __name__ == '__main__':
    # Check if pytest is available
    try:
        import pytest
        print("Running tests with pytest...")
        pytest.main([__file__, "-v"])
    except ImportError:
        print("pytest not available, running simple tests...")
        success = asyncio.run(run_simple_tests())
        sys.exit(0 if success else 1)