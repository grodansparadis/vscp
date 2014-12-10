// FILE: vscp_serial.h 
//
// Copyright (C) 2000-2014 Ake Hedman akhe@grodansparadis.com 
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// This file contains defines for the VSCP Serial protocol which 
// is described here http://www.vscp.org/docs/vscpspec/doku.php?id=physical_level_lower_level_protocols#vscp_over_a_serial_channel_rs-232
//

#ifndef _VSCP_SERIAL_H_
#define _VSCP_SERIAL_H_

// Serial protocol defines

// VSCP Driver states
#define STATE_VSCP_DRIVER_WAIT_FOR_FRAME_START      0
#define STATE_VSCP_DRIVER_WAIT_FOR_FRAME_END        1
#define STATE_VSCP_DRIVER_FRAME_RECEIVED            2

// VSCP Driver positions in frame
#define VSCP_DRIVER_POS_FRAME_TYPE                  0
#define VSCP_DRIVER_POS_FRAME_CHANNEL               1
#define VSCP_DRIVER_POS_FRAME_SEQUENCY              2
#define VSCP_DRIVER_POS_FRAME_SIZE_PAYLOAD_MSB      3
#define VSCP_DRIVER_POS_FRAME_SIZE_PAYLOAD_LSB      4
#define VSCP_DRIVER_POS_FRAME_PAYLOAD               5

// VSCP Driver operations
#define VSCP_DRVER_OPERATION_NOOP                   0
#define VSCP_DRVER_OPERATION_VSCP_EVENT             1
#define VSCP_DRVER_OPERATION_CANAL                  2
#define VSCP_DRVER_OPERATION_CONFIGURE              3
#define VSCP_DRVER_OPERATION_POLL                   4
#define VSCP_DRVER_OPERATION_NO_EVENT               5
#define VSCP_DRVER_OPERATION_SENT_ACK               249
#define VSCP_DRVER_OPERATION_SENT_NACK              250
#define VSCP_DRVER_OPERATION_ACK                    251
#define VSCP_DRVER_OPERATION_NACK                   252
#define VSCP_DRVER_OPERATION_ERROR                  253
#define VSCP_DRVER_OPERATION_COMMAND_REPLY          254
#define VSCP_DRVER_OPERATION_COMMAND                255

// VSCP Driver errors
#define ERROR_VSCP_DRIVER_CHECKSUM                  1
#define ERROR_VSCP_DRIVER_UNKNOWN_OPERATION         2

// VSCP driver commands
#define VSCP_DRIVER_COMMAND_NOOP                    0
#define VSCP_DRIVER_COMMAND_OPEN                    1
#define VSCP_DRIVER_COMMAND_LISTEN                  2
#define VSCP_DRIVER_COMMAND_LOOPBACK                3
#define VSCP_DRIVER_COMMAND_CLOSE                   4
#define VSCP_DRIVER_COMMAND_SET_FILTER              5


#endif