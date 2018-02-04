// vaxp_debug.h
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

#define VSCP_DEBUG1_ALL             0xFFFFFFFF

// Debug flags 1
#define VSCP_DEBUG1_AUTOMATION      (1 << 0)    // Automation debug info
#define VSCP_DEBUG1_VARIABLE        (1 << 1)    // Variable handling debug info
#define VSCP_DEBUG1_MULTICAST       (1 << 2)    // Multicast i/f debug info
#define VSCP_DEBUG1_UDP             (1 << 3)    // UDP i/f debug info
#define VSCP_DEBUG1_TCP             (1 << 4)    // TCP/IP client i/f debug info
#define VSCP_DEBUG1_DM              (1 << 5)    // General DM debug info
#define VSCP_DEBUG1_DM_TIMERS       (1 << 6)    // DM timer debug info
#define VSCP_DEBUG1_DRIVER          (1 << 7)    // Driver debug info
// the rest of the 32-bits...

#define VSCP_DEBUG2_ALL             0xFFFFFFFF

#define VSCP_DEBUG3_ALL             0xFFFFFFFF

#define VSCP_DEBUG4_ALL             0xFFFFFFFF

#define VSCP_DEBUG5_ALL             0xFFFFFFFF

#define VSCP_DEBUG6_ALL             0xFFFFFFFF

#define VSCP_DEBUG7_ALL             0xFFFFFFFF

#define VSCP_DEBUG8_ALL             0xFFFFFFFF

#define VSCP_DEBUG9_ALL             0xFFFFFFFF