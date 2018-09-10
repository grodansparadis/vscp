// can4vscpobj.h: 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2018
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

#if !defined(AFX_CAN4VSCPDRV_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
#define AFX_CAN4VSCPDRV_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_

#ifdef WIN32

#include <windows.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "com_win32.h"

#else

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <pthread.h>
#include <syslog.h>
#include <com.h>

#endif

#include <stdio.h>
#include <canal.h>
#include <vscp.h>
#include <vscp_serial.h>
#include <canal_macro.h>
#include <dllist.h>

// define to create a file c:/tmp/can4vscp.txt with input and
// stat machine data for debugging
//#define DEBUG_CAN4VSCP_RECEIVE

// Flags
#define CAN4VSCP_FLAG_ENABLE_NO_SWITCH_TO_NEW_MODE          0x0004
#define CAN4VSCP_FLAG_ENABLE_WAIT_FOR_ACK                   0x0008
#define CAN4VSCP_FLAG_ENABLE_TIMESTAMP                      0x0010
#define CAN4VSCP_FLAG_ENABLE_HARDWARE_HANDSHAKE             0x0020
#define CAN4VSCP_FLAG_ENABLE_REOPEN                         0x0040

// Mutexes
#define CANAL_DLL_CAN4VSCPDRV_OBJ_MUTEX	                    TEXT("___CANAL__DLL_CAN4VSCPDRV_OBJ_MUTEX____")
#define CANAL_DLL_CAN4VSCPDRV_RECEIVE_MUTEX                 TEXT("___CANAL__DLL_CAN4VSCPDRV_RECEIVE_MUTEX____")
#define CANAL_DLL_CAN4VSCPDRV_TRANSMIT_MUTEX                TEXT("___CANAL__DLL_CAN4VSCPDRV_TRANSMIT_MUTEX____")
#define CANAL_DLL_CAN4VSCPDRV_RESPONSE_MUTEX                TEXT("___CANAL__DLL_CAN4VSCPDRV_RESPONSE_MUTEX____")

// Max messages in input queue
#define CAN4VSCP_MAX_RCVMSG                                 4096

// Max messages in output queue
#define CAN4VSCP_MAX_SNDMSG                                 4096

// Max number of response messages in response queue
#define CAN4VSCP_MAX_RESPONSEMSG	                        32

// Byte stuffing start and end characters
#define DLE                                                 0x10
#define STX                                                 0x02
#define ETX                                                 0x03

// RX State machine
#define INCOMING_STATE_NONE                                 0	// Waiting for <STX>
#define INCOMING_STATE_STX                                  1	// Reading data
#define INCOMING_STATE_ETX                                  2	// <ETX> has been received
#define INCOMING_STATE_COMPLETE                             3	// Frame received

#define INCOMING_SUBSTATE_NONE                              0	// Idle
#define INCOMING_SUBSTATE_DLE                               1	// <DLE> received


// CAN4VSCP Commands
#define RESET_NOOP                                          0x00	// No Operation
#define	GET_TX_ERR_CNT                                      0x02	// Get TX error count
#define	GET_RX_ERR_CNT                                      0x03	// Get RX error count
#define	GET_CANSTAT                                         0x04	// Get CAN statistics
#define	GET_COMSTAT                                         0x05
#define	GET_MSGFILTER1                                      0x06	// Get message filter 1
#define	GET_MSGFILTER2                                      0x07	// Get message filter 2
#define	SET_MSGFILTER1                                      0x08	// Set message filter 1
#define	SET_MSGFILTER2                                      0x09	// Set message filter 2


// Emergency flags
#define EMERGENCY_OVERFLOW                                  0x01
#define EMERGENCY_RCV_WARNING                               0x02
#define EMERGENCY_TX_WARNING                                0x04
#define EMERGENCY_TXBUS_PASSIVE                             0x08
#define EMERGENCY_RXBUS_PASSIVE                             0x10
#define EMERGENCY_BUS_OFF                                   0x20

// VSCP Driver positions in frame
#define VSCP_CAN4VSCP_DRIVER_POS_FRAME_TYPE                 0
#define VSCP_CAN4VSCP_DRIVER_POS_FRAME_CHANNEL              1
#define VSCP_CAN4VSCP_DRIVER_POS_FRAME_SEQUENCY             2
#define VSCP_CAN4VSCP_DRIVER_POS_FRAME_SIZE_PAYLOAD_MSB     3
#define VSCP_CAN4VSCP_DRIVER_POS_FRAME_SIZE_PAYLOAD_LSB     4
#define VSCP_CAN4VSCP_DRIVER_POS_FRAME_PAYLOAD              5

// VSCP driver commands
#define VSCP_CAN4VSCP_DRIVER_COMMAND_NOOP                   0
#define VSCP_CAN4VSCP_DRIVER_COMMAND_OPEN                   1
#define VSCP_CAN4VSCP_DRIVER_COMMAND_LISTEN                 2
#define VSCP_CAN4VSCP_DRIVER_COMMAND_LOOPBACK               3
#define VSCP_CAN4VSCP_DRIVER_COMMAND_CLOSE                  4
#define VSCP_CAN4VSCP_DRIVER_COMMAND_SET_FILTER             5
#define VSCP_CAN4VSCP_DRIVER_COMMAND_SET_MASK               6

// VSCP driver configuration
#define VSCP_DRIVER_CONFIG_NOOP                             0
#define VSCP_DRIVER_CONFIG_MODE                             1
#define VSCP_DRIVER_CONFIG_TIMESTAMP                        2
#define VSCP_DRIVER_CONFIG_BAUDRATE                         3


// Capabilities for this driver
#define CAN4VSCP_DRIVER_MAX_VSCP_FRAMES                     2
#define CAN4VSCP_DRIVER_MAX_CANAL_FRAMES                    10

// Baudrate codes
#define SET_BAUDRATE_115200                                 0
#define SET_BAUDRATE_128000                                 1
#define SET_BAUDRATE_230400                                 2
#define SET_BAUDRATE_256000                                 3
#define SET_BAUDRATE_460800                                 4
#define SET_BAUDRATE_500000                                 5
#define SET_BAUDRATE_625000                                 6
#define SET_BAUDRATE_921600                                 7
#define SET_BAUDRATE_1000000                                8
#define SET_BAUDRATE_9600                                   9
#define SET_BAUDRATE_19200                                  10
#define SET_BAUDRATE_38400                                  11
#define SET_BAUDRATE_57600                                  12

#define SET_BAUDRATE_MAX                                    13

// Define soft open timeout in microsecons
// If enabled i/f inactivity more than tyhis timeout
// will reopen the i/f in a soft way and continue with
// that until activity is detected.
#define SOFT_OPEN_TIMOUT                                    120000  // Two minutes

// Driver info in BINHEX
#ifdef WIN32
#define DRIVERINFO   "PD94bWwgdmVyc2lvbiA9ICIxLjAiIGVuY29kaW5nID0gIlVURi04IiA/Pgo8Y29uZmlnPgogICAgPGRlc2NyaXB0aW9uPkNBTjRWU0NQIHN0YW5kYXJkIHNlcmlhbCBkcml2ZXIuPC9kZXNjcmlwdGlvbj4KICAgIDxsZXZlbD4xPC9sZXZlbD4KICAgIDxibG9ja2luZz55ZXM8L2Jsb2NraW5nPgogICAgPGluZm8+aHR0cDovL2h0dHA6Ly93d3cuZ3JvZGFuc3BhcmFkaXMuY29tL2ZyYW5rZnVydC9yczIzMi9tYW51YWwvZG9rdS5waHA/aWQ9dGhlX2NhbjR2c2NwX21vZGU8L2luZm8+CiAgICA8aXRlbXM+CiAgICAgICAgPGl0ZW0gcG9zPSIwIiB0eXBlPSJzdHJpbmciIGRlc2NyaXB0aW9uPSJTZXJpYWwgcG9ydCAoY29tMSwgY29tMi4uLikiIGluZm89IiIgLz4KICAgICAgICA8aXRlbSBwb3M9IjEiIHR5cGU9ImNob2ljZSIgb3B0aW9uYWw9InRydWUiIGRlc2NyaXB0aW9uPSJCYXVkcmF0ZSBjb2RlIiBpbmZvPSJodHRwOi8vd3d3LnZzY3Aub3JnL2RvY3MvdnNjcGQvZG9rdS5waHA/aWQ9bGV2ZWwxX2RyaXZlcl9jYW40dnNjcCNwYXJhbWV0ZXJfc3RyaW5nIj4KICAgICAgICAgICAgPGNob2ljZSB2YWx1ZT0iMCIgZGVzY3JpcHRpb24gPSAiMCAtIDExNTIwMCAoRGVmYXVsdCB2YWx1ZSkiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjEiIGRlc2NyaXB0aW9uID0gIjEgLSAxMjgwMDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjMiIGRlc2NyaXB0aW9uID0gIjIgLSAyMzA0MDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjQiIGRlc2NyaXB0aW9uID0gIjMgLSAyNTYwMDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjUiIGRlc2NyaXB0aW9uID0gIjQgLSA0NjA4MDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjYiIGRlc2NyaXB0aW9uID0gIjUgLSA1MDAwMDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjciIGRlc2NyaXB0aW9uID0gIjYgLSA2MjUwMDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjgiIGRlc2NyaXB0aW9uID0gIjcgLSA5MjE2MDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjkiIGRlc2NyaXB0aW9uID0gIjggLSAxMDAwMDAwIiAvPgogICAgICAgICAgICA8Y2hvaWNlIHZhbHVlPSIwIiBkZXNjcmlwdGlvbiA9ICI5IC0gOTYwMCIgLz4KICAgICAgICAgICAgPGNob2ljZSB2YWx1ZT0iMTAiIGRlc2NyaXB0aW9uID0gIjEwIC0gMTkyMDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjExIiBkZXNjcmlwdGlvbiA9ICIxMSAtIDM4NDAwIiAvPgogICAgICAgICAgICA8Y2hvaWNlIHZhbHVlPSIxMiIgZGVzY3JpcHRpb24gPSAiMTIgLSA1NzYwMCIgLz4KICAgICAgICA8L2l0ZW0+CiAgICA8L2l0ZW1zPgoJCiAgICA8ZmxhZ3M+CiAgICAgICAgPGJpdCBwb3M9IjAiIHdpZHRoPSIyIiB0eXBlPSJjaG9pY2UiIGRlc2NyaXB0aW9uPSIiIGluZm89Imh0dHA6Ly93d3cudnNjcC5vcmcvZG9jcy92c2NwZC9kb2t1LnBocD9pZD1sZXZlbDFfZHJpdmVyX2NhbjR2c2NwI2ZsYWdzIiA+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjAiIGRlc2NyaXB0aW9uPSJPcGVuIENBTjRWU0NQIGludGVyZmFjZSBpbiBub3JtYWwgbW9kZS4iIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjEiIGRlc2NyaXB0aW9uPSJPcGVuIENBTjRWU0NQIGludGVyZmFjZSBpbiBsaXN0ZW4gbW9kZS4iIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjIiIGRlc2NyaXB0aW9uPSJPcGVuIENBTjRWU0NQIGludGVyZmFjZSBpbiBub3JtYWwgbW9kZS4iIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjMiIGRlc2NyaXB0aW9uPSJPcGVuIENBTjRWU0NQIGludGVyZmFjZSBpbiBub3JtYWwgbW9kZS4iIC8+CiAgICAgICAgPC9iaXQ+CiAgICAgICAgPGJpdCBwb3M9IjIiIHdpZHRoPSIxIiB0eXBlPSJib29sIiBkZXNjcmlwdGlvbj0iSWYgc2V0IHRoZSBkcml2ZXIgd2lsbCBub3Qgc3dpdGNoIHRvIFZTQ1AgbW9kZS4gVGhhdCBpcyBpdCBtdXN0IGJlIGluIFZTQ1AgbW9kZS4gT3BlbiB3aWxsIGJlIGZhc3Rlci4iIGluZm89Imh0dHA6Ly93d3cudnNjcC5vcmcvZG9jcy92c2NwZC9kb2t1LnBocD9pZD1sZXZlbDFfZHJpdmVyX2NhbjR2c2NwI2ZsYWdzIiAvPgogICAgICAgIDxiaXQgcG9zPSIzIiB3aWR0aD0iMSIgdHlwZT0iYm9vbCIgZGVzY3JpcHRpb249IklmIHNldCB0aGUgZHJpdmVyIHdpbGwgd2FpdCBmb3IgYW4gQUNLIGZyb20gdGhlIHBoeXNpY2FsIGRldmljZSBmb3IgZXZlcnkgc2VudCBmcmFtZS4gVGhpcyB3aWxsIHNsb3cgZG93biBzZW5kaW5nIGJ1dCBtYWtlIHRyYW5zbWlzc2lvbiBpdCB2ZXJ5IHNlY3VyZS4iIGluZm89Imh0dHA6Ly93d3cudnNjcC5vcmcvZG9jcy92c2NwZC9kb2t1LnBocD9pZD1sZXZlbDFfZHJpdmVyX2NhbjR2c2NwI2ZsYWdzIiAvPgogICAgICAgIDxiaXQgcG9zPSI0IiB3aWR0aD0iMSIgdHlwZT0iYm9vbCIgZGVzY3JpcHRpb249IklmIHNldCBlbmFibGUgdGltZXN0YW1wLiBUaGUgdGltZXN0YW1wIHdpbGwgYmUgd3JpdHRlbiBieSB0aGUgaGFyZHdhcmUgaW5zdGVhZCBvZiB0aGUgZHJpdmVyLiIgaW5mbz0iaHR0cDovL3d3dy52c2NwLm9yZy9kb2NzL3ZzY3BkL2Rva3UucGhwP2lkPWxldmVsMV9kcml2ZXJfY2FuNHZzY3AjZmxhZ3MiIC8+CiAgICAgICAgPGJpdCBwb3M9IjUiIHdpZHRoPSIxIiB0eXBlPSJib29sIiBkZXNjcmlwdGlvbj0iSWYgc2V0IGVuYWJsZSBoYXJkd2FyZSBoYW5kc2hha2UuIiBpbmZvPSJodHRwOi8vd3d3LnZzY3Aub3JnL2RvY3MvdnNjcGQvZG9rdS5waHA/aWQ9bGV2ZWwxX2RyaXZlcl9jYW40dnNjcCNmbGFncyIgLz4KICAgICAgICA8Yml0IHBvcz0iNiIgd2lkdGg9IjEiIHR5cGU9ImJvb2wiIGRlc2NyaXB0aW9uPSJJZiBzZXQgZW5hYmxlIHNvZnQgT3Blbi4iIGluZm89Imh0dHA6Ly93d3cudnNjcC5vcmcvZG9jcy92c2NwZC9kb2t1LnBocD9pZD1sZXZlbDFfZHJpdmVyX2NhbjR2c2NwI2ZsYWdzIiAvPgogICAgPC9mbGFncz4KPC9jb25maWc+"
#else
#define DRIVERINFO  "PD94bWwgdmVyc2lvbiA9ICIxLjAiIGVuY29kaW5nID0gIlVURi04IiA/Pgo8Y29uZmlnPgogICAgPGRlc2NyaXB0aW9uPkNBTjRWU0NQIHN0YW5kYXJkIHNlcmlhbCBkcml2ZXIuPC9kZXNjcmlwdGlvbj4KICAgIDxsZXZlbD4xPC9sZXZlbD4KICAgIDxibG9ja2luZz55ZXM8L2Jsb2NraW5nPgogICAgPGluZm8+aHR0cDovL2h0dHA6Ly93d3cuZ3JvZGFuc3BhcmFkaXMuY29tL2ZyYW5rZnVydC9yczIzMi9tYW51YWwvZG9rdS5waHA/aWQ9dGhlX2NhbjR2c2NwX21vZGU8L2luZm8+CiAgICA8aXRlbXM+CiAgICAgICAgPGl0ZW0gcG9zPSIwIiB0eXBlPSJzdHJpbmciIGRlc2NyaXB0aW9uPSJTZXJpYWwgcG9ydCAoL2Rldi90dHlTMCwgL2Rldi90dHlTMS4uLikiIGluZm89IiIgLz4KICAgICAgICA8aXRlbSBwb3M9IjEiIHR5cGU9ImNob2ljZSIgb3B0aW9uYWw9InRydWUiIGRlc2NyaXB0aW9uPSJCYXVkcmF0ZSBjb2RlIiBpbmZvPSJodHRwOi8vd3d3LnZzY3Aub3JnL2RvY3MvdnNjcGQvZG9rdS5waHA/aWQ9bGV2ZWwxX2RyaXZlcl9jYW40dnNjcCNwYXJhbWV0ZXJfc3RyaW5nIj4KICAgICAgICAgICAgPGNob2ljZSB2YWx1ZT0iMCIgZGVzY3JpcHRpb24gPSAiMCAtIDExNTIwMCAoRGVmYXVsdCB2YWx1ZSkiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjEiIGRlc2NyaXB0aW9uID0gIjEgLSAxMjgwMDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjMiIGRlc2NyaXB0aW9uID0gIjIgLSAyMzA0MDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjQiIGRlc2NyaXB0aW9uID0gIjMgLSAyNTYwMDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjUiIGRlc2NyaXB0aW9uID0gIjQgLSA0NjA4MDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjYiIGRlc2NyaXB0aW9uID0gIjUgLSA1MDAwMDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjciIGRlc2NyaXB0aW9uID0gIjYgLSA2MjUwMDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjgiIGRlc2NyaXB0aW9uID0gIjcgLSA5MjE2MDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjkiIGRlc2NyaXB0aW9uID0gIjggLSAxMDAwMDAwIiAvPgogICAgICAgICAgICA8Y2hvaWNlIHZhbHVlPSIwIiBkZXNjcmlwdGlvbiA9ICI5IC0gOTYwMCIgLz4KICAgICAgICAgICAgPGNob2ljZSB2YWx1ZT0iMTAiIGRlc2NyaXB0aW9uID0gIjEwIC0gMTkyMDAiIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjExIiBkZXNjcmlwdGlvbiA9ICIxMSAtIDM4NDAwIiAvPgogICAgICAgICAgICA8Y2hvaWNlIHZhbHVlPSIxMiIgZGVzY3JpcHRpb24gPSAiMTIgLSA1NzYwMCIgLz4KICAgICAgICA8L2l0ZW0+CiAgICA8L2l0ZW1zPgoJCiAgICA8ZmxhZ3M+CiAgICAgICAgPGJpdCBwb3M9IjAiIHdpZHRoPSIyIiB0eXBlPSJjaG9pY2UiIGRlc2NyaXB0aW9uPSIiIGluZm89Imh0dHA6Ly93d3cudnNjcC5vcmcvZG9jcy92c2NwZC9kb2t1LnBocD9pZD1sZXZlbDFfZHJpdmVyX2NhbjR2c2NwI2ZsYWdzIiA+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjAiIGRlc2NyaXB0aW9uPSJPcGVuIENBTjRWU0NQIGludGVyZmFjZSBpbiBub3JtYWwgbW9kZS4iIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjEiIGRlc2NyaXB0aW9uPSJPcGVuIENBTjRWU0NQIGludGVyZmFjZSBpbiBsaXN0ZW4gbW9kZS4iIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjIiIGRlc2NyaXB0aW9uPSJPcGVuIENBTjRWU0NQIGludGVyZmFjZSBpbiBub3JtYWwgbW9kZS4iIC8+CiAgICAgICAgICAgIDxjaG9pY2UgdmFsdWU9IjMiIGRlc2NyaXB0aW9uPSJPcGVuIENBTjRWU0NQIGludGVyZmFjZSBpbiBub3JtYWwgbW9kZS4iIC8+CiAgICAgICAgPC9iaXQ+CiAgICAgICAgPGJpdCBwb3M9IjIiIHdpZHRoPSIxIiB0eXBlPSJib29sIiBkZXNjcmlwdGlvbj0iSWYgc2V0IHRoZSBkcml2ZXIgd2lsbCBub3Qgc3dpdGNoIHRvIFZTQ1AgbW9kZS4gVGhhdCBpcyBpdCBtdXN0IGJlIGluIFZTQ1AgbW9kZS4gT3BlbiB3aWxsIGJlIGZhc3Rlci4iIGluZm89Imh0dHA6Ly93d3cudnNjcC5vcmcvZG9jcy92c2NwZC9kb2t1LnBocD9pZD1sZXZlbDFfZHJpdmVyX2NhbjR2c2NwI2ZsYWdzIiAvPgogICAgICAgIDxiaXQgcG9zPSIzIiB3aWR0aD0iMSIgdHlwZT0iYm9vbCIgZGVzY3JpcHRpb249IklmIHNldCB0aGUgZHJpdmVyIHdpbGwgd2FpdCBmb3IgYW4gQUNLIGZyb20gdGhlIHBoeXNpY2FsIGRldmljZSBmb3IgZXZlcnkgc2VudCBmcmFtZS4gVGhpcyB3aWxsIHNsb3cgZG93biBzZW5kaW5nIGJ1dCBtYWtlIHRyYW5zbWlzc2lvbiBpdCB2ZXJ5IHNlY3VyZS4iIGluZm89Imh0dHA6Ly93d3cudnNjcC5vcmcvZG9jcy92c2NwZC9kb2t1LnBocD9pZD1sZXZlbDFfZHJpdmVyX2NhbjR2c2NwI2ZsYWdzIiAvPgogICAgICAgIDxiaXQgcG9zPSI0IiB3aWR0aD0iMSIgdHlwZT0iYm9vbCIgZGVzY3JpcHRpb249IklmIHNldCBlbmFibGUgdGltZXN0YW1wLiBUaGUgdGltZXN0YW1wIHdpbGwgYmUgd3JpdHRlbiBieSB0aGUgaGFyZHdhcmUgaW5zdGVhZCBvZiB0aGUgZHJpdmVyLiIgaW5mbz0iaHR0cDovL3d3dy52c2NwLm9yZy9kb2NzL3ZzY3BkL2Rva3UucGhwP2lkPWxldmVsMV9kcml2ZXJfY2FuNHZzY3AjZmxhZ3MiIC8+CiAgICAgICAgPGJpdCBwb3M9IjUiIHdpZHRoPSIxIiB0eXBlPSJib29sIiBkZXNjcmlwdGlvbj0iSWYgc2V0IGVuYWJsZSBoYXJkd2FyZSBoYW5kc2hha2UuIiBpbmZvPSJodHRwOi8vd3d3LnZzY3Aub3JnL2RvY3MvdnNjcGQvZG9rdS5waHA/aWQ9bGV2ZWwxX2RyaXZlcl9jYW40dnNjcCNmbGFncyIgLz4KICAgICAgICA8Yml0IHBvcz0iNiIgd2lkdGg9IjEiIHR5cGU9ImJvb2wiIGRlc2NyaXB0aW9uPSJJZiBzZXQgZW5hYmxlIHNvZnQgT3Blbi4iIGluZm89Imh0dHA6Ly93d3cudnNjcC5vcmcvZG9jcy92c2NwZC9kb2t1LnBocD9pZD1sZXZlbDFfZHJpdmVyX2NhbjR2c2NwI2ZsYWdzIiAvPgogICAgPC9mbGFncz4KPC9jb25maWc+"
#endif 

typedef struct {
    bool bWaitingForAckNack;    // True if message is sent and waiting for ACK or NACK
    bool bAck;                  // true if ACK is received, false if NACK
    uint8_t seq;                // Sequency number for frame
    uint8_t channel;            // Channel
} msgResponseInfoStruct;

//
// The command response structure
//

typedef struct {
    uint8_t op;                 // Operation == frametype
    uint8_t seq;                // Sequency number
    uint8_t channel;            // Channel
    uint16_t sizePayload;       // Size of payload
    uint8_t payload[ 512 ];     // Message payload
} cmdResponseMsg;

class CCan4VSCPObj {
public:

    /// Constructor
    CCan4VSCPObj();

    /// Destructor
    virtual ~CCan4VSCPObj();

    /*!
        Filter message

        @param pcanalMsg Pointer to CAN message
        @return True if message is accepted false if rejected
     */
    bool doFilter(canalMsg *pcanalMsg);


    /*!
        Set Filter
     */
    int setFilter(unsigned long filter);


    /*!
        Set Mask
     */
    int setMask(unsigned long mask);


#ifdef DEBUG_CAN4VSCP_RECEIVE
    /// The log file handle
    FILE *m_flog;
#endif

    /*! 
        Open/create the logfile

        @param pConfig	Configuration string
        @param flags 	bit 1 = 0 Append, bit 1 = 1 Rewrite
        @return True on success.
     */
    int open(const char *pConfig, unsigned long flags = 0);


    /*!
        Flush and close the log file
     */
    int close(void);

    /*!
        softOpen
        
        Reopen the channel in case of adapter being restarted while
        in operation.If power is briken for a can4vscp adapter it will be logically
        closed. From this side the serial channel will be open. softOpen
        do a logical open (in case no traffic is detected). This will keep 
        the connection open even in this case where a restart of the host 
        software otherwise was needed.
    */
    int softOpen();

    /*!
        Get Interface statistics
        @param pCanalStatistics Pointer to CANAL statistics structure
        @return True on success.
     */
    int getStatistics(PCANALSTATISTICS pCanalStatistics);


    /*!
        Write a message out to the device (non blocking)
        @param pcanalMsg Pointer to CAN message
        @return True on success.
     */
    int writeMsg(canalMsg *pMsg);

    /*!
        Write a message out to the device (blocking)
        @param pcanalMsg Pointer to CAN message
        @return CANAL return code. CANAL_ERROR_SUCCESS on success.
     */
    int writeMsgBlocking(canalMsg *pMsg, uint32_t Timeout);

    /*!
        Read a message fro the device
        @param pcanalMsg Pointer to CAN message
        @return True on success.
     */
    int readMsg(canalMsg *pMsg);


    /*!
        Read a message from the device (Blocking)
        @param pcanalMsg Pointer to CAN message
        @param timout Timout in milliseconds
        @return CANAL return code. CANAL_ERROR_SUCCESS on success.
     */
    int readMsgBlocking(canalMsg *pMsg, uint32_t timeout);


    /*!
        Check for data availability
        @return Number of packages in the queue
     */
    int dataAvailable(void);

    /*!
                Handle for receive event to know when to call readMsg
                @return Handle
     */
#ifdef WIN32	

    HANDLE getReceiveHandle(void) {
        return m_receiveDataEvent;
    }
#endif	

    /*!
        Get device status
        @param pCanalStatus Pointer to CANAL status structure
        @return True on success.
     */
    int getStatus( PCANALSTATUS pCanalStatus );

    /*!
        Get device capabilities
        Device capabilities are set in member variables
     */
    bool getDeviceCapabilities( void );

    /*!
        Send a command

        @param cmdcode Code for command
        @param data Pointer to data to send
        @param dataSize Size for datablock
        @return True on success.
     */
    bool sendCommand( uint8_t cmdcode,
                        uint8_t *pParam = NULL,
                        uint8_t size = 0 );


    /*!
        Wait for a command response message

        @param pMsg Pointer to response message
        @param cmdcode Command to wait for reply from
        @param saveseq Sequency number when frame was sent.
        @param timeout Timeout in milliseconds
        @return True on success
     */
    bool wait4CommandResponse( cmdResponseMsg *pMsg,
                                uint8_t cmdcode,
                                uint8_t saveseq,
                                uint32_t timeout );

    /*!
        Send command and wait for a response message

        @param cmdcode Code for command
        @param pMsg Pointer to response message
        @param timeout Timeout in milliseconds
        @return True on success
     */
    bool sendCommandWait( uint8_t cmdcode,
                            uint8_t *pParam,
                            uint8_t size,
                            cmdResponseMsg *pMsg,
                            uint32_t timeout );

    /*!
        Sent a configuration parameter in the device

        @param cmdcode Code for configuration parameter to set
        @param data Pointer to data to send
        @param dataSize Size for datablock
        @return True on success.
    */
    bool sendConfig( uint8_t codeConfig, uint8_t *pParam, uint8_t size );

    /*!
        Wait for a configuration response message

        @param pMsg Pointer to response message
        @param codeConfig Configuration code to wait for reply from
        @param saveseq Sequency number when frame was sent.
        @param timeout Timeout in milliseconds
        @return True on success
    */
    bool wait4ConfigResponse( cmdResponseMsg *pMsg, uint8_t codeConfig, uint8_t saveseq, uint32_t timeout );

    /*!
    Send command and wait for a response message

    @param codeConfig Code for configuration parameter
    @param pMsg Pointer to response message
    @param timeout Timeout in milliseconds
    @return True on success
    */
    bool sendConfigWait( uint8_t codeConfig,
                          uint8_t *pParam,
                          uint8_t size,
                          cmdResponseMsg *pMsg,
                          uint32_t timeout );

    /*!
        Send command on the serial channel

        @param buffer Contains data to send
        @param size Total number of bytes to send
        @return true on success
     */
    bool sendMsg(uint8_t *buffer, short size);

    /*!
        Check CRC for frame in buffer defined by
        content in m_bufferMsgRcv and with length
        m_lengthMsgRcv
     */
    bool checkCRC(void);

    /*!
        Send ACK
        \param seq Sequency number
     */
    void sendACK(uint8_t seq);

    /*!
        Send NACK
        \param seq Sequency number
     */
    void sendNACK(uint8_t seq);

    /*!
        Send NOOP frame
     */
    void sendNoopFrame(void);

    /*!
        Send Open Frame
        \mode is 0=Normal. 1=Listen, 2=Loopback, 3=Configuration
     */
    void sendOpenInterfaceFrame(uint8_t mode);

    /*!
        Send Close Interface frame
     */
    void sendCloseInterfaceFrame(void);

    /*!
        Add current frame in buffer to response queue
        \return true on success
     */
    bool addToResponseQueue(void);

    /*!
        Read serial data and feed to state machine
        @return true when a full frame is received.
     */
    bool serialData2StateMachine(void);

    /*!
        Do reading and interpret data
     */
    void readSerialData(void);

    // Endiness functions

    int little_endian() {
        int x = 1;
        return *(char*) &x;
    };

    int big_endian() {
        return !little_endian();
    };

public:

    /// Run flag
    bool m_bRun;

    // Open flag
    bool m_bOpen;

    // * * * Capabilities * * *
    vscp_serial_caps m_caps;

    /*! 
        Max number of VSCP frames in 
        multi frame payload
     */
    uint8_t m_capsMaxVscpFrames;

    /*!
        Max number of CANAL frames in
        multi frame payload.
     */
    uint8_t m_capsMaxCanalFrames;

    /*!
        Interface statistics
     */
    canalStatistics m_stat;


    /*!
        Interface status

        Bit 0  - TX Error Counter.
        Bit 1  - TX Error Counter.
        Bit 2  - TX Error Counter.
        Bit 3  - TX Error Counter.
        Bit 4  - TX Error Counter.
        Bit 5  - TX Error Counter.
        Bit 6  - TX Error Counter.
        Bit 7  - TX Error Counter.
        Bit 8  - RX Error Counter.
        Bit 9  - RX Error Counter.
        Bit 10 - RX Error Counter.
        Bit 11 - RX Error Counter.
        Bit 12 - RX Error Counter.
        Bit 13 - RX Error Counter.
        Bit 14 - RX Error Counter.
        Bit 15 - RX Error Counter.
        Bit 16 - Overflow.
        Bit 17 - RX Warning.
        Bit 18 - TX Warning.
        Bit 19 - TX bus passive.
        Bit 20 - RX bus passive..
        Bit 21 - Reserved.
        Bit 22 - Reserved.
        Bit 23 - Reserved.
        Bit 24 - Reserved.
        Bit 25 - Reserved.
        Bit 26 - Reserved.
        Bit 27 - Reserved.
        Bit 28 - Reserved.
        Bit 29 - Bus Passive.
        Bit 30 - Bus Warning status
        Bit 31 - Bus off status 

     */
    canalStatus m_status;


    /*! 
        Transmit queue

        This is the transmit queue for messages going out to the
        device
     */
    DoubleLinkedList m_transmitList;

    /*! 
        Receive queue

        This is the receive queue for messages going in to the
        device
     */
    DoubleLinkedList m_receiveList;


    /*! 
        Response queue

        This is the receive queue for command response messages
     */
    DoubleLinkedList m_responseList;


    /*!
        Holds info about the message that is in transmit and
        is awaiting a response from a remote board.
    */
    msgResponseInfoStruct msgResponseInfo;


    /*!
        Tread id Receive
     */
#ifdef WIN32
    HANDLE m_hTreadReceive;
#else
    pthread_t m_threadIdReceive;
#endif

    /*!
        Tread id Transmit
     */
#ifdef WIN32
    HANDLE m_hTreadTransmit;
#else
    pthread_t m_threadIdTransmit;
#endif

    /*!
        Mutex for device.
     */
#ifdef WIN32	
    HANDLE m_can4vscpMutex;
#else
    pthread_mutex_t m_can4vscpMutex;
#endif

#ifdef WIN32	
    HANDLE m_receiveDataEvent;      // GS
    HANDLE m_transmitDataPutEvent;  // GS
    HANDLE m_transmitDataGetEvent;  // GS
    HANDLE m_transmitAckNackEvent;  // Set when ACK/NACK is received for message
#else
    sem_t m_receiveDataSem;
    sem_t m_transmitDataPutSem;
    sem_t m_transmitDataGetSem;
    sem_t m_transmitAckNackSem;     // Set when ACK/NACK is received for message
#endif

    /*!
        Mutex that protect receive queue.
     */
#ifdef WIN32	
    HANDLE m_receiveMutex;
#else
    pthread_mutex_t m_receiveMutex;
#endif

    /*!
        Mutex that protect transmit queue.
     */
#ifdef WIN32	
    HANDLE m_transmitMutex;
#else
    pthread_mutex_t m_transmitMutex;
#endif

    /*!
        Mutex that protect command response queue.
     */
#ifdef WIN32	
    HANDLE m_responseMutex;
#else
    pthread_mutex_t m_responseMutex;
#endif

    /*!
    Filter for outgoing messages

    mask bit n | filter bit n | msg id bit | result
    ===========================================================
        0              X              X        Accept				
        1              0              0        Accept
        1              0              1        Reject
        1              1              0        Reject
        1              1              1        Accept

    Formula is !( ( filter \EEd ) & mask )
     */
    uint32_t m_filter;

    /*!
        Mask for outgoing messages	
     */
    uint32_t m_mask;


    ///////////////////////////////////////////////////////////////////////////
    //            * * * * *  CAN4VSCP specific * * * * *
    ///////////////////////////////////////////////////////////////////////////


    /*!
        flags 
     */
    uint32_t m_initFlag;

#ifdef WIN32
    CComm m_com;
#else
    Comm m_com;

    /*!
        Configured baudrate
    */
    uint8_t m_nBaud;

    /*!
        The can232 object MUTEX 	
     */
    pthread_mutex_t m_can4vscpObjMutex;


    /*!
        id for worker thread
     */
    pthread_t m_threadId;

#endif	


    /*!
        State for incoming frames
     */
    int m_RxMsgState;

    /*!
        Substate for incoming frames
     */
    int m_RxMsgSubState;


    /*!
        General receive buffer
     */
    uint8_t m_bufferRx[ 0x10000 ];

    /*!
        Message receive buffer
     */
    uint8_t m_bufferMsgRcv[ 512 ];

    /*!
        Current length for received message
     */
    uint16_t m_lengthMsgRcv;


    /*!
        Sequency number
        This number is increase for every frame sent.
     */
    uint8_t m_sequencyno;

    /*!
        Transmit queue handle
     */
    uint16_t m_hTxQue;

    /*!
        Receive queue handle
     */
    uint16_t m_hRxQue;

    /*!
        Activity timer
        Is zero if events received
    */
    uint32_t m_activity;
};

#endif // !defined(AFX_IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
