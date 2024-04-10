// vscp_bootdevice_defs.h.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:  (C) 2007-2024
// Ake Hedman, the VSCP project, <info@vscp.org>
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
//

#define MEMREG_PRG_START_COMMON 0x000000
#define MEMREG_PRG_END_COMMON 0x2fffff

#define BUFFER_SIZE_PROGRAM_COMMON 0x20000

#define INTEL_LINETYPE_DATA 0             // Data record.
#define INTEL_LINETYPE_EOF 1              // End Of File record.
#define INTEL_LINETYPE_EXTENDED_SEGMENT 2 // Extended segment address	record.
#define INTEL_LINETYPE_RESERVED 3         // Not used
#define INTEL_LINETYPE_EXTENDED_LINEAR 4  // Extended linear address record.

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

// Timeout for response
#define BOOT_COMMAND_RESPONSE_TIMEOUT 5

// Memory Type
#define MEM_TYPE_PROGRAM 0x00

#define VSCP_ENTER_BOOTLODER_MODE                                              \
    0x0C // VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER  = 12

#define VSCP_PRIORITY_LOW_COMMON 0x07

// Used VSCP registers
#define VSCP_REG_PAGE_SELECT_MSB 0x92
#define VSCP_REG_PAGE_SELECT_LSB 0x93
#define VSCP_REG_GUID0 0xD0
#define VSCP_REG_GUID3 0xD3
#define VSCP_REG_GUID5 0xD5
#define VSCP_REG_GUID7 0xD7
