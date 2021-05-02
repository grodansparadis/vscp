// vscp_debug.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, the VSCP project
// <akhe@vscp.org>
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

// * * * VSCP daemon debug flags (64 flags)
#define VSCP_DEBUG_NONE             0x0000000000000000
#define VSCP_DEBUG_ALL              0xFFFFFFFFFFFFFFFF
#define VSCP_DEBUG_EXTRA            (1 << 0)    // 1 Verbose info
#define VSCP_DEBUG_CONFIG           (1 << 1)    // 2 Configuration

#define VSCP_DEBUG_MQTT_CONNECT     (1 << 2)    // 4 MQTT connect/disconnect debug info
#define VSCP_DEBUG_MQTT_MSG         (1 << 3)    // 8 MQTT message debug info
#define VSCP_DEBUG_MQTT_PUBLISH     (1 << 4)    // 16 MQTT publish debug info
#define VSCP_DEBUG_MQTT_LOG         (1 << 5)    // 32 MQTT log debug info

#define VSCP_DEBUG_DRIVERL1         (1 << 6)    // 64 Driver debug info level I
#define VSCP_DEBUG_DRIVERL1_MQTT    (1 << 7)    // 128 Driver MQTT debug info level I
#define VSCP_DEBUG_DRIVERL1_RX      (1 << 8)    // 256 Driver rx debug info level I
#define VSCP_DEBUG_DRIVERL1_TX      (1 << 9)    // 512 Driver tx debug info level I

#define VSCP_DEBUG_DRIVERL2         (1 << 10)   // 1024 Driver debug info level II
#define VSCP_DEBUG_DRIVERL2_MQTT    (1 << 11)   // 2048 Driver MQTT debug info level II
#define VSCP_DEBUG_DRIVERL2_RX      (1 << 12)   // 4096 Driver rx debug info level II
#define VSCP_DEBUG_DRIVERL2_TX      (1 << 13)   // 8192 Driver tx debug info level II

#define VSCP_DEBUG_EVENT_DATABASE   (1 << 14)   // 16384 Event database debug
#define VSCP_DEBUG_MAIN_DATABASE    (1 << 15)   // 32768 Discovery database debug


