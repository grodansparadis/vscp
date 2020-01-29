// vaxp_debug.h
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <vscpd.h>

extern uint32_t m_gdebugArray[8];

// * * * General debug flags
#define DBG_GENERAL            0
#define VSCP_DEBUG1_ALL        0xFFFFFFFF
#define VSCP_DEBUG1_EXTRA      (1 << 0) // Verbose info
#define VSCP_DEBUG1_AUTOMATION (1 << 1) // Automation debug info
#define VSCP_DEBUG1_CONFIG     (1 << 2) // Configuration

#define __VSCP_DEBUG_EXTRA (m_gdebugArray[DBG_GENERAL] | VSCP_DEBUG1_EXTRA)

#define __VSCP_DEBUG_AUTOMATION                                                \
    (m_gdebugArray[DBG_GENERAL] | VSCP_DEBUG1_AUTOMATION)

#define __VSCP_DEBUG_CONFIG                                                    \
    (m_gdebugArray[DBG_GENERAL] | VSCP_DEBUG1_CONFIG)

#define DEBUG_GENERAL_ALL m_gdebugArray[DBG_GENERAL] = 0xFFFFFFFF

// * * * tcp/ip
#define DBG_TCPIP          1
#define VSCP_DEBUG2_ALL    0xFFFFFFFF
#define VSCP_DEBUG2_TCP    (1 << 1) // TCP/IP client i/f debug info
#define VSCP_DEBUG2_TCP_RX (1 << 2)
#define VSCP_DEBUG2_TCP_TX (1 << 3)

#define __VSCP_DEBUG_TCP (m_gdebugArray[DBG_TCPIP] | VSCP_DEBUG2_TCP)

#define __VSCP_DEBUG_TCP_RX (m_gdebugArray[DBG_TCPIP] | VSCP_DEBUG2_TCP_RX)

#define __VSCP_DEBUG_TCP_TX (m_gdebugArray[DBG_TCPIP] | VSCP_DEBUG2_TCP_TX)

// * * * Web server

#define VSCP_DEBUG3_ALL           0xFFFFFFFF
#define DBG_WEBSRV                2
#define VSCP_DEBUG3_WEBSRV        (1 << 1) // Webserver i/f debug info
#define VSCP_DEBUG3_REST          (1 << 2) // REST client i/f debug info
#define VSCP_DEBUG3_WEBSRV_ACCESS (1 << 3) // Web server access

#define __VSCP_DEBUG_WEBSRV                                                    \
    (m_gdebugArray[DBG_WEBSRV] | VSCP_DEBUG3_WEBSRV)

#define __VSCP_DEBUG_REST (m_gdebugArray[DBG_WEBSRV] | VSCP_DEBUG3_REST)

#define __VSCP_DEBUG_WEBSRV_ACCESS                                             \
    (m_gdebugArray[DBG_WEBSRV] | VSCP_DEBUG3_WEBSRV_ACCESS)

//  * * * Web sockets
#define VSCP_DEBUG4_ALL          0xFFFFFFFF
#define DBG_WEBSOCK              3
#define VSCP_DEBUG4_WEBSOCKET    (1 << 1) // Websockets client i/f debug info
#define VSCP_DEBUG4_WEBSOCKET_RX (1 << 2) // Websocket receive
#define VSCP_DEBUG4_WEBSOCKET_TX (1 << 3) // Websocket send
#define VSCP_DEBUG4_WEBSOCKET_PING (1 << 4) // Websocket ping/pong

#define __VSCP_DEBUG_WEBSOCKET                                                 \
    (m_gdebugArray[DBG_WEBSOCK] | VSCP_DEBUG4_WEBSOCKET)

#define __VSCP_DEBUG_WEBSOCKET_RX                                              \
    (m_gdebugArray[DBG_WEBSOCK] | VSCP_DEBUG4_WEBSOCKET_RX)

#define __VSCP_DEBUG_WEBSOCKET_TX                                              \
    (m_gdebugArray[DBG_WEBSOCK] | VSCP_DEBUG4_WEBSOCKET_TX)

#define __VSCP_DEBUG_WEBSOCKET_PING                                              \
    (m_gdebugArray[DBG_WEBSOCK] | VSCP_DEBUG4_WEBSOCKET_PING)

// * * * Drivers
#define VSCP_DEBUG5_ALL        0xFFFFFFFF
#define DBG_DRV                4
#define VSCP_DEBUG5_DRIVER1    (1 << 1) // Driver debug info level I
#define VSCP_DEBUG5_DRIVER2    (1 << 2) // Driver debug info level II
#define VSCP_DEBUG5_DRIVER1_RX (1 << 3)
#define VSCP_DEBUG5_DRIVER1_TX (1 << 4)
#define VSCP_DEBUG5_DRIVER2_RX (1 << 5)
#define VSCP_DEBUG5_DRIVER2_TX (1 << 6)

#define __VSCP_DEBUG_DRIVER1 (m_gdebugArray[DBG_DRV] | VSCP_DEBUG5_DRIVER1)

#define __VSCP_DEBUG_DRIVER2 (m_gdebugArray[DBG_DRV] | VSCP_DEBUG5_DRIVER2)

#define __VSCP_DEBUG_DRIVER1_RX                                                \
    (m_gdebugArray[DBG_DRV] | VSCP_DEBUG5_DRIVER1_RX)

#define __VSCP_DEBUG_DRIVER1_TX                                                \
    (m_gdebugArray[DBG_DRV] | VSCP_DEBUG5_DRIVER1_TX)

#define __VSCP_DEBUG_DRIVER2_RX                                                \
    (m_gdebugArray[DBG_DRV] | VSCP_DEBUG5_DRIVER2_RX)

#define __VSCP_DEBUG_DRIVER2_TX                                                \
    (m_gdebugArray[DBG_DRV] | VSCP_DEBUG5_DRIVER2_TX)

#define VSCP_DEBUG6_ALL 0xFFFFFFFF

#define VSCP_DEBUG7_ALL 0xFFFFFFFF

#define VSCP_DEBUG8_ALL 0xFFFFFFFF

#define VSCP_DEBUG9_ALL 0xFFFFFFFF
