// vaxp_debug.h
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
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

#define VSCP_DEBUG1_ALL 0xFFFFFFFF

// Debug flags 1
#define VSCP_DEBUG1_GENERAL (1 << 0)    // Automation debug info
#define VSCP_DEBUG1_AUTOMATION (1 << 1) // Automation debug info
#define VSCP_DEBUG1_TCP (1 << 2)        // TCP/IP client i/f debug info
#define VSCP_DEBUG1_DRIVER1 (1 << 3)    // Driver debug info level I
#define VSCP_DEBUG1_DRIVER2 (1 << 4)    // Driver debug info level II
#define VSCP_DEBUG1_DRIVER3 (1 << 5)    // Driver debug info level III
// the rest of the 32-bits...

#define VSCP_DEBUG2_ALL 0xFFFFFFFF

#define VSCP_DEBUG3_ALL 0xFFFFFFFF

#define VSCP_DEBUG4_ALL 0xFFFFFFFF

#define VSCP_DEBUG5_ALL 0xFFFFFFFF

#define VSCP_DEBUG6_ALL 0xFFFFFFFF

#define VSCP_DEBUG7_ALL 0xFFFFFFFF

#define VSCP_DEBUG8_ALL 0xFFFFFFFF

#define VSCP_DEBUG9_ALL 0xFFFFFFFF