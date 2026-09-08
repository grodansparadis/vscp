"""
VSCP Level 2 Client Interface for Python

This package provides a Python interface to VSCP Level 2 drivers and devices.
It mirrors the functionality found in the C++ vscpClientLevel2 class and
level2drvdef.h interface definitions.

Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project <info@vscp.org>
MIT License
"""

from .client import (
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
    create_filter,
    __version__,
    __author__,
    __email__,
    __license__
)

__all__ = [
    # Main classes
    'VSCPLevel2Client',
    'VSCPEvent', 
    'VSCPEventFilter',
    
    # Enums and constants
    'VSCPError',
    'VSCPPriority', 
    'VSCPConstants',
    
    # Data structures
    'VersionInfo',
    'Statistics',
    
    # Factory functions
    'create_client',
    'create_event',
    'create_filter',
    
    # Module info
    '__version__',
    '__author__',
    '__email__',
    '__license__'
]

# Convenience aliases
Client = VSCPLevel2Client
Event = VSCPEvent
Filter = VSCPEventFilter
Error = VSCPError
Priority = VSCPPriority
Constants = VSCPConstants