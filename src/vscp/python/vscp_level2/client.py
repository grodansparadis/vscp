"""
VSCP Level 2 Client Interface for Python

This module provides a Python interface to VSCP Level 2 drivers and devices.
It mirrors the functionality found in the C++ vscpClientLevel2 class and
level2drvdef.h interface definitions.

Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project <info@vscp.org>
MIT License
"""

import asyncio
import json
import time
from datetime import datetime, timezone
from typing import Optional, List, Dict, Any, Tuple, Union
from dataclasses import dataclass, field
from enum import IntEnum
import threading
import queue
import logging
import secrets


class VSCPError(IntEnum):
    """VSCP Error codes from the C++ implementation"""
    SUCCESS = 0
    GENERAL = -1
    INVALID_HANDLE = -2
    INVALID_PARAMETER = -3
    TIMEOUT = -4
    NOT_CONNECTED = -5
    WRITE_ERROR = -6
    READ_ERROR = -7
    INVALID_POINTER = -8
    OPERATION_FAILED = -9


class VSCPPriority(IntEnum):
    """VSCP Priority levels"""
    PRIORITY_0 = 0x00  # Highest
    PRIORITY_1 = 0x20
    PRIORITY_2 = 0x40
    PRIORITY_3 = 0x60
    PRIORITY_4 = 0x80
    PRIORITY_5 = 0xA0
    PRIORITY_6 = 0xC0
    PRIORITY_7 = 0xE0  # Lowest
    
    HIGH = 0x00
    NORMAL = 0x60
    MEDIUM = 0xC0
    LOW = 0xE0


class VSCPConstants:
    """VSCP Constants from vscp.h and level2drvdef.h"""
    
    # Default ports and addresses
    DEFAULT_TCP_PORT = 9598
    DEFAULT_UDP_PORT = 33333
    DEFAULT_MULTICAST_PORT = 44444
    
    # Data size limits
    MAX_DATA = 512
    SIZE_GUID = 16
    
    # Levels
    LEVEL1 = 0
    LEVEL2 = 1
    
    # Header flags
    HEADER_PRIORITY_MASK = 0xE0
    HEADER_HARD_CODED = 0x10
    HEADER_NO_CRC = 0x08
    
    # Header16 flags
    HEADER16_DUMB = 0x8000
    HEADER16_IPV6_GUID = 0x1000
    
    # GUID Types
    HEADER16_GUID_TYPE_STANDARD = 0x0000
    HEADER16_GUID_TYPE_IPV6 = 0x1000
    HEADER16_GUID_TYPE_RFC4122V1 = 0x2000
    HEADER16_GUID_TYPE_RFC4122V4 = 0x3000
    
    # Flags
    FLAG_ENABLE_DEBUG = 0x80000000
    
    # Default timeouts
    DEFAULT_RESPONSE_TIMEOUT = 3000  # 3 seconds in milliseconds


@dataclass
class VSCPEvent:
    """
    VSCP Event class representing a Level 2 event
    Mirrors the vscpEvent structure from vscp.h
    """
    crc: int = 0
    obid: int = 0
    year: int = 0
    month: int = 0
    day: int = 0
    hour: int = 0
    minute: int = 0
    second: int = 0
    timestamp: int = 0
    head: int = 0
    vscp_class: int = 0
    vscp_type: int = 0
    guid: List[int] = field(default_factory=lambda: [0] * 16)
    size_data: int = 0
    data: List[int] = field(default_factory=list)
    
    def __post_init__(self):
        """Ensure GUID is proper size and update size_data"""
        if len(self.guid) != VSCPConstants.SIZE_GUID:
            self.guid = [0] * VSCPConstants.SIZE_GUID
        
        # Update size_data if data is provided
        if self.data:
            self.size_data = min(len(self.data), VSCPConstants.MAX_DATA)
    
    def get_priority(self) -> int:
        """Get priority from head"""
        return (self.head & VSCPConstants.HEADER_PRIORITY_MASK) >> 5
    
    def set_priority(self, priority: int) -> None:
        """Set priority in head"""
        self.head = (self.head & ~VSCPConstants.HEADER_PRIORITY_MASK) | \
                   ((priority & 0x07) << 5)
    
    def is_hard_coded(self) -> bool:
        """Check if hard coded"""
        return bool(self.head & VSCPConstants.HEADER_HARD_CODED)
    
    def set_hard_coded(self, hard_coded: bool) -> None:
        """Set hard coded flag"""
        if hard_coded:
            self.head |= VSCPConstants.HEADER_HARD_CODED
        else:
            self.head &= ~VSCPConstants.HEADER_HARD_CODED
    
    def set_current_time(self) -> None:
        """Set current UTC timestamp"""
        now = datetime.now(timezone.utc)
        self.year = now.year
        self.month = now.month
        self.day = now.day
        self.hour = now.hour
        self.minute = now.minute
        self.second = now.second
        self.timestamp = (now.microsecond)  # microseconds
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert event to dictionary"""
        return {
            'crc': self.crc,
            'obid': self.obid,
            'year': self.year,
            'month': self.month,
            'day': self.day,
            'hour': self.hour,
            'minute': self.minute,
            'second': self.second,
            'timestamp': self.timestamp,
            'head': self.head,
            'vscp_class': self.vscp_class,
            'vscp_type': self.vscp_type,
            'guid': self.guid.copy(),
            'size_data': self.size_data,
            'data': self.data.copy()
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'VSCPEvent':
        """Create event from dictionary"""
        return cls(**data)
    
    def to_json(self) -> str:
        """Convert event to JSON string"""
        return json.dumps(self.to_dict())
    
    @classmethod
    def from_json(cls, json_str: str) -> 'VSCPEvent':
        """Create event from JSON string"""
        return cls.from_dict(json.loads(json_str))


@dataclass
class VSCPEventFilter:
    """
    VSCP Event Filter class for filtering events
    """
    filter_priority: int = 0
    mask_priority: int = 0
    filter_class: int = 0
    mask_class: int = 0
    filter_type: int = 0
    mask_type: int = 0
    filter_guid: List[int] = field(default_factory=lambda: [0] * 16)
    mask_guid: List[int] = field(default_factory=lambda: [0] * 16)
    
    def clear(self) -> None:
        """Clear filter (allow all events)"""
        self.filter_priority = 0
        self.mask_priority = 0
        self.filter_class = 0
        self.mask_class = 0
        self.filter_type = 0
        self.mask_type = 0
        self.filter_guid = [0] * 16
        self.mask_guid = [0] * 16
    
    def set_class_and_type(self, vscp_class: int, vscp_type: int) -> None:
        """Set filter for specific class and type"""
        self.filter_class = vscp_class
        self.mask_class = 0xFFFF
        self.filter_type = vscp_type
        self.mask_type = 0xFFFF


@dataclass
class VersionInfo:
    """Version information structure"""
    major: int = 0
    minor: int = 0
    release: int = 0
    build: int = 0


@dataclass
class Statistics:
    """Statistics tracking structure"""
    events_received: int = 0
    events_sent: int = 0
    errors: int = 0
    connect_time: Optional[datetime] = None


class VSCPLevel2Client:
    """
    VSCP Level 2 Client class
    Mirrors the functionality of vscpClientLevel2 from the C++ implementation
    """
    
    def __init__(self, 
                 interface: str = '',
                 flags: int = 0,
                 response_timeout: int = VSCPConstants.DEFAULT_RESPONSE_TIMEOUT,
                 connection_timeout: int = VSCPConstants.DEFAULT_RESPONSE_TIMEOUT,
                 max_queue_size: int = 1000):
        """
        Initialize VSCP Level 2 Client
        
        Args:
            interface: Interface name/path
            flags: Configuration flags
            response_timeout: Response timeout in milliseconds
            connection_timeout: Connection timeout in milliseconds
            max_queue_size: Maximum event queue size
        """
        self.interface = interface
        self.flags = flags
        self.response_timeout = response_timeout
        self.connection_timeout = connection_timeout
        self.max_queue_size = max_queue_size
        
        # Connection state
        self._connected = False
        self._handle: Optional[int] = None
        
        # Debug mode
        self._debug = bool(flags & VSCPConstants.FLAG_ENABLE_DEBUG)
        
        # Event queues and threading
        self._event_queue = queue.Queue(maxsize=max_queue_size)
        self._filter: Optional[VSCPEventFilter] = None
        
        # Statistics
        self._stats = Statistics()
        
        # Threading
        self._lock = threading.RLock()
        self._stop_event = threading.Event()
        
        # Logging setup
        self._logger = logging.getLogger(f'vscp.level2.{id(self)}')
        if self._debug:
            self._logger.setLevel(logging.DEBUG)
            if not self._logger.handlers:
                handler = logging.StreamHandler()
                formatter = logging.Formatter(
                    '[%(name)s] %(levelname)s: %(message)s'
                )
                handler.setFormatter(formatter)
                self._logger.addHandler(handler)
    
    def init(self, interface: str, flags: int = 0, 
             timeout: int = VSCPConstants.DEFAULT_RESPONSE_TIMEOUT) -> int:
        """
        Initialize the client with interface and configuration
        Maps to the Level 2 driver interface initialization
        
        Args:
            interface: Interface name/path
            flags: Configuration flags
            timeout: Response timeout in milliseconds
            
        Returns:
            VSCPError code
        """
        try:
            with self._lock:
                self.interface = interface
                self.flags = flags
                self.response_timeout = timeout
                self._debug = bool(flags & VSCPConstants.FLAG_ENABLE_DEBUG)
                
                if self._debug:
                    self._logger.debug(f'Initialized Level 2 client for interface: {interface}')
                
                return VSCPError.SUCCESS
                
        except Exception as e:
            self._log_error('Init failed', e)
            return VSCPError.GENERAL
    
    async def connect(self) -> int:
        """
        Connect to the interface
        Maps to LPFNDLL_VSCPOPEN from level2drvdef.h
        
        Returns:
            VSCPError code
        """
        try:
            with self._lock:
                if self._connected:
                    return VSCPError.SUCCESS
                
                # Generate a unique handle (simulated)
                self._handle = secrets.randbits(32)
                self._connected = True
                self._stats.connect_time = datetime.now(timezone.utc)
                
                if self._debug:
                    self._logger.debug(f'Connected to interface: {self.interface}')
                
                return VSCPError.SUCCESS
                
        except Exception as e:
            self._log_error('Connect failed', e)
            return VSCPError.GENERAL
    
    async def disconnect(self) -> int:
        """
        Disconnect from the interface
        Maps to LPFNDLL_VSCPCLOSE from level2drvdef.h
        
        Returns:
            VSCPError code
        """
        try:
            with self._lock:
                if not self._connected:
                    return VSCPError.SUCCESS
                
                self._connected = False
                self._handle = None
                
                # Clear event queue
                while not self._event_queue.empty():
                    try:
                        self._event_queue.get_nowait()
                    except queue.Empty:
                        break
                
                if self._debug:
                    self._logger.debug(f'Disconnected from interface: {self.interface}')
                
                return VSCPError.SUCCESS
                
        except Exception as e:
            self._log_error('Disconnect failed', e)
            return VSCPError.GENERAL
    
    def is_connected(self) -> bool:
        """
        Check if connected
        
        Returns:
            True if connected
        """
        return self._connected
    
    async def send(self, event: VSCPEvent, timeout: Optional[int] = None) -> int:
        """
        Send VSCP event
        Maps to LPFNDLL_VSCPWRITE from level2drvdef.h
        
        Args:
            event: Event to send
            timeout: Timeout in milliseconds
            
        Returns:
            VSCPError code
        """
        try:
            if not self._connected:
                return VSCPError.NOT_CONNECTED
            
            if not isinstance(event, VSCPEvent):
                return VSCPError.INVALID_PARAMETER
            
            # Validate event data
            if event.size_data > VSCPConstants.MAX_DATA:
                return VSCPError.INVALID_PARAMETER
            
            # Set timestamp if not set
            if not event.year or not event.month or not event.day:
                event.set_current_time()
            
            with self._lock:
                self._stats.events_sent += 1
            
            if self._debug:
                self._logger.debug(f'Sending event: Class={event.vscp_class}, Type={event.vscp_type}')
            
            return VSCPError.SUCCESS
            
        except Exception as e:
            with self._lock:
                self._stats.errors += 1
            self._log_error('Send failed', e)
            return VSCPError.WRITE_ERROR
    
    async def receive(self, timeout: Optional[int] = None) -> Tuple[int, Optional[VSCPEvent]]:
        """
        Receive VSCP event
        Maps to LPFNDLL_VSCPREAD from level2drvdef.h
        
        Args:
            timeout: Timeout in milliseconds
            
        Returns:
            Tuple of (error_code, event)
        """
        try:
            if not self._connected:
                return VSCPError.NOT_CONNECTED, None
            
            timeout_s = (timeout or self.response_timeout) / 1000.0
            
            # Try to get event from queue
            try:
                event = self._event_queue.get(timeout=timeout_s)
                with self._lock:
                    self._stats.events_received += 1
                
                if self._debug:
                    self._logger.debug(f'Received event: Class={event.vscp_class}, Type={event.vscp_type}')
                
                return VSCPError.SUCCESS, event
                
            except queue.Empty:
                return VSCPError.TIMEOUT, None
                
        except Exception as e:
            with self._lock:
                self._stats.errors += 1
            self._log_error('Receive failed', e)
            return VSCPError.READ_ERROR, None
    
    def set_filter(self, filter_obj: VSCPEventFilter) -> int:
        """
        Set event filter
        
        Args:
            filter_obj: Event filter
            
        Returns:
            VSCPError code
        """
        try:
            if not self._connected:
                return VSCPError.NOT_CONNECTED
            
            if not isinstance(filter_obj, VSCPEventFilter):
                return VSCPError.INVALID_PARAMETER
            
            with self._lock:
                self._filter = filter_obj
            
            if self._debug:
                self._logger.debug('Event filter set')
            
            return VSCPError.SUCCESS
            
        except Exception as e:
            self._log_error('Set filter failed', e)
            return VSCPError.GENERAL
    
    async def get_count(self) -> Tuple[int, int]:
        """
        Get count of events in queue
        
        Returns:
            Tuple of (error_code, count)
        """
        try:
            if not self._connected:
                return VSCPError.NOT_CONNECTED, 0
            
            return VSCPError.SUCCESS, self._event_queue.qsize()
            
        except Exception as e:
            self._log_error('Get count failed', e)
            return VSCPError.GENERAL, 0
    
    def clear(self) -> int:
        """
        Clear event queue
        
        Returns:
            VSCPError code
        """
        try:
            while not self._event_queue.empty():
                try:
                    self._event_queue.get_nowait()
                except queue.Empty:
                    break
            
            if self._debug:
                self._logger.debug('Event queue cleared')
            
            return VSCPError.SUCCESS
            
        except Exception as e:
            self._log_error('Clear failed', e)
            return VSCPError.GENERAL
    
    async def get_version(self) -> Tuple[int, VersionInfo]:
        """
        Get interface version
        Maps to LPFNDLL_VSCPGETVERSION from level2drvdef.h
        
        Returns:
            Tuple of (error_code, version_info)
        """
        try:
            version = VersionInfo(major=15, minor=0, release=3, build=0)
            return VSCPError.SUCCESS, version
            
        except Exception as e:
            self._log_error('Get version failed', e)
            return VSCPError.GENERAL, VersionInfo()
    
    async def get_interfaces(self) -> Tuple[int, List[str]]:
        """
        Get available interfaces
        
        Returns:
            Tuple of (error_code, interfaces_list)
        """
        try:
            if not self._connected:
                return VSCPError.NOT_CONNECTED, []
            
            # Return current interface
            return VSCPError.SUCCESS, [self.interface]
            
        except Exception as e:
            self._log_error('Get interfaces failed', e)
            return VSCPError.GENERAL, []
    
    async def get_capabilities(self) -> Tuple[int, int]:
        """
        Get capabilities (What Can You Do)
        
        Returns:
            Tuple of (error_code, capabilities)
        """
        try:
            # Return basic Level 2 capabilities
            capabilities = 0x00000001  # Basic Level 2 support
            return VSCPError.SUCCESS, capabilities
            
        except Exception as e:
            self._log_error('Get capabilities failed', e)
            return VSCPError.GENERAL, 0
    
    def get_config_as_json(self) -> str:
        """
        Get configuration as JSON
        
        Returns:
            JSON configuration string
        """
        config = {
            'interface': self.interface,
            'flags': self.flags,
            'response_timeout': self.response_timeout,
            'connection_timeout': self.connection_timeout,
            'connected': self._connected,
            'stats': {
                'events_sent': self._stats.events_sent,
                'events_received': self._stats.events_received,
                'errors': self._stats.errors,
                'connect_time': self._stats.connect_time.isoformat() if self._stats.connect_time else None
            }
        }
        return json.dumps(config, indent=2)
    
    def init_from_json(self, config: str) -> bool:
        """
        Initialize from JSON configuration
        
        Args:
            config: JSON configuration string
            
        Returns:
            True on success
        """
        try:
            cfg = json.loads(config)
            
            if 'interface' in cfg:
                self.interface = cfg['interface']
            if 'flags' in cfg:
                self.flags = cfg['flags']
            if 'response_timeout' in cfg:
                self.response_timeout = cfg['response_timeout']
            if 'connection_timeout' in cfg:
                self.connection_timeout = cfg['connection_timeout']
            
            self._debug = bool(self.flags & VSCPConstants.FLAG_ENABLE_DEBUG)
            
            return True
            
        except Exception as e:
            self._log_error('Init from JSON failed', e)
            return False
    
    def set_connection_timeout(self, timeout: int) -> None:
        """Set connection timeout in milliseconds"""
        self.connection_timeout = max(0, timeout)
    
    def get_connection_timeout(self) -> int:
        """Get connection timeout in milliseconds"""
        return self.connection_timeout
    
    def set_response_timeout(self, timeout: int) -> None:
        """Set response timeout in milliseconds"""
        self.response_timeout = max(0, timeout)
    
    def get_response_timeout(self) -> int:
        """Get response timeout in milliseconds"""
        return self.response_timeout
    
    def get_statistics(self) -> Statistics:
        """
        Get statistics
        
        Returns:
            Statistics object
        """
        with self._lock:
            return Statistics(
                events_received=self._stats.events_received,
                events_sent=self._stats.events_sent,
                errors=self._stats.errors,
                connect_time=self._stats.connect_time
            )
    
    def inject_event(self, event: VSCPEvent) -> None:
        """
        Inject an event into the queue (for testing or simulation)
        
        Args:
            event: Event to inject
        """
        if not isinstance(event, VSCPEvent):
            raise ValueError('Invalid event type')
        
        try:
            # Apply filter if set
            if self._filter and not self._check_filter(event):
                return
            
            self._event_queue.put_nowait(event)
            
        except queue.Full:
            # Remove oldest event and add new one
            try:
                self._event_queue.get_nowait()
                self._event_queue.put_nowait(event)
            except queue.Empty:
                pass
    
    def _check_filter(self, event: VSCPEvent) -> bool:
        """
        Check if event passes filter (for simulation)
        
        Args:
            event: Event to check
            
        Returns:
            True if event passes filter
        """
        if not self._filter:
            return True
        
        # Check class filter
        if self._filter.mask_class != 0:
            if (event.vscp_class & self._filter.mask_class) != (self._filter.filter_class & self._filter.mask_class):
                return False
        
        # Check type filter
        if self._filter.mask_type != 0:
            if (event.vscp_type & self._filter.mask_type) != (self._filter.filter_type & self._filter.mask_type):
                return False
        
        # Check priority filter
        if self._filter.mask_priority != 0:
            event_priority = event.head & VSCPConstants.HEADER_PRIORITY_MASK
            filter_priority = self._filter.filter_priority & self._filter.mask_priority
            if (event_priority & self._filter.mask_priority) != filter_priority:
                return False
        
        # Check GUID filter
        for i in range(16):
            if self._filter.mask_guid[i] != 0:
                if (event.guid[i] & self._filter.mask_guid[i]) != (self._filter.filter_guid[i] & self._filter.mask_guid[i]):
                    return False
        
        return True
    
    def _log_error(self, message: str, error: Exception) -> None:
        """Log error message"""
        error_msg = f'{message}: {str(error)}'
        
        if self._debug:
            self._logger.error(error_msg)
        
        with self._lock:
            self._stats.errors += 1


# Factory functions for convenience
def create_client(**kwargs) -> VSCPLevel2Client:
    """Create a new VSCP Level 2 client"""
    return VSCPLevel2Client(**kwargs)


def create_event(**kwargs) -> VSCPEvent:
    """Create a new VSCP event"""
    return VSCPEvent(**kwargs)


def create_filter(**kwargs) -> VSCPEventFilter:
    """Create a new VSCP event filter"""
    return VSCPEventFilter(**kwargs)


# Version information
__version__ = '15.0.3'
__author__ = 'Ake Hedman, the VSCP project'
__email__ = 'akhe@paradiseofthefrog.com'
__license__ = 'MIT'