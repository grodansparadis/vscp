// vscpd_caps.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

//
// This file contains server capabilities for the VSCP server
//

#if !defined(VSCPDCAPS)
#define VSCPDCAPS

#include "vscp.h"


// Server capabilities for HIGH END SERVER PROBE

#define VSCP_DAEMON_SERVER_CAPABILITIES_0   0
#define VSCP_DAEMON_SERVER_CAPABILITIES_1   0
#define VSCP_DAEMON_SERVER_CAPABILITIES_2   0
#define VSCP_DAEMON_SERVER_CAPABILITIES_3   0
#define VSCP_DAEMON_SERVER_CAPABILITIES_4   0
#define VSCP_DAEMON_SERVER_CAPABILITIES_5   0

// Fill info in LSB of 16-bit code
#define VSCP_DAEMON_SERVER_CAPABILITIES_6  ( (uint8_t)(( VSCP_SERVER_CAPABILITY_TCPIP | \
                                        VSCP_SERVER_CAPABILITY_UDP | \
                                        VSCP_SERVER_CAPABILITY_MULTICAST_ANNOUNCE | \
                                        VSCP_SERVER_CAPABILITY_RAWETH | \
                                        VSCP_SERVER_CAPABILITY_WEB | \
                                        VSCP_SERVER_CAPABILITY_WEBSOCKET | \
                                        VSCP_SERVER_CAPABILITY_REST | \
                                        VSCP_SERVER_CAPABILITY_MULTICAST_CHANNEL ) >> 8 ) )
            
    // Fill info in LSB of 16-bit code
#define VSCP_DAEMON_SERVER_CAPABILITIES_7  VSCP_SERVER_CAPABILITY_IP6 | \
                                            VSCP_SERVER_CAPABILITY_IP4 | \
                                            VSCP_SERVER_CAPABILITY_SSL | \
                                            VSCP_SERVER_CAPABILITY_TWO_CONNECTIONS


#endif
