// vaxp_debug.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2017
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#define VSCP_DEBUG1_ALL             0xFFFFFFFF

// Debug flags
#define VSCP_DEBUG1_AUTOMATION      (1 << 0)    // Automation debug info
#define VSCP_DEBUG1_VARIABLE        (1 << 1)    // Variable handling debug info
#define VSCP_DEBUG1_MULTICAST       (1 << 2)    // Multicast i/f debug info
#define VSCP_DEBUG1_UDP             (1 << 3)    // UDP i/f debug info
#define VSCP_DEBUG1_TCP             (1 << 4)    // TCP/IP client i/f debug info
#define VSCP_DEBUG1_DM              (1 << 5)    // General DM debug info
#define VSCP_DEBUG1_DM_TIMERS       (1 << 6)    // DM timer debug info
#define VSCP_DEBUG1_DRIVER          (1 << 7)    // Driver debug info

#define VSCP_DEBUG2_ALL             0xFFFFFFFF

#define VSCP_DEBUG3_ALL             0xFFFFFFFF

#define VSCP_DEBUG4_ALL             0xFFFFFFFF

#define VSCP_DEBUG5_ALL             0xFFFFFFFF

#define VSCP_DEBUG6_ALL             0xFFFFFFFF

#define VSCP_DEBUG7_ALL             0xFFFFFFFF

#define VSCP_DEBUG8_ALL             0xFFFFFFFF

#define VSCP_DEBUG9_ALL             0xFFFFFFFF