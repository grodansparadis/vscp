// pipecmd.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org 
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia, <akhe@eurosource.se>
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: pipecmd.h,v $                                       
// $Date: 2005/01/05 12:50:53 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

// Data package
#define VSCP_POS_COMMAND					0
#define VSCP_POS_FLAGS						1
#define VSCP_POS_ID							5
#define VSCP_POS_OBID						9
#define VSCP_POS_TIMESTAMP					13
#define POS_DATASIZE						17
#define VSCP_POS_DATA0						18

// Response package
#define VSCP_POS_RESPONSE					0
#define VSCP_POS_RESPONSE_CODE				1

// Data availability
#define VSCP_POS_NUMBER_OF_MSG				1

// State
#define VSCP_POS_STATE						1

// Statistics
#define	VSCP_POS_CNT_RECEIVE_FRAMES			1
#define VSCP_POS_CNT_TRANSMIT_FRAMES		5
#define VSCP_POS_CNT_RECEIVE_DATA			9
#define VSCP_POS_CNT_TRANSMIT_DATA			13
#define VSCP_POS_CNT_OVERRUNS				17
#define VSCP_POS_CNT_BUS_WARNINGS			21
#define VSCP_POS_CNT_BUS_OFF				25

// Filter
#define	VSCP_POS_FILTER						1

// Mask
#define POS_MASK							1

// Baudrate
#define VSCP_POS_BAUDRATE					1

// Canal version information
#define VSCP_POS_VSCP_MAIN_VERSION			1	
#define VSCP_POS_VSCP_MINOR_VERSION			0
#define VSCP_POS_VSCP_SUB_VERSION			0



// CANAL commands sent over the pipe interface
#define VSCP_PIPE_COMMAND_NONE						0		// No command
#define VSCP_PIPE_COMMAND_OPEN						1		// Open channel 
#define VSCP_PIPE_COMMAND_CLOSE						2		// Close channel
#define VSCP_PIPE_COMMAND_SEND						3		// Send message
#define VSCP_PIPE_COMMAND_RECEIVE					4		// Receive message
#define VSCP_PIPE_COMMAND_CHECKDATA					5		// Check if data is available
#define VSCP_PIPE_COMMAND_BAUDRATE					6		// Set Baudrate
#define VSCP_PIPE_COMMAND_STATUS					7		// Get status
#define VSCP_PIPE_COMMAND_STATISTICS				8		// Get statistics
#define VSCP_PIPE_COMMAND_FILTER					9		// Set filter
#define VSCP_PIPE_COMMAND_MASK						10		// Set mask
#define VSCP_PIPE_COMMAND_VERSION					11		// VSCP version
#define VSCP_PIPE_COMMAND_DLL_VERSION			    12		// VSCP DLL version
#define VSCP_PIPE_COMMAND_VENDOR_STRING				13		// VSCP vendor string
#define VSCP_PIPE_COMMAND_LEVEL						14		// VSCP Level bitarray

// CANAL responses sent over the pipe interface
#define VSCP_PIPE_RESPONSE_NONE						0		// 
#define VSCP_PIPE_RESPONSE_SUCCESS					1		// OK message
#define VSCP_PIPE_RESPONSE_ERROR					2		// ERROR message
#define VSCP_PIPE_RESPONSE_MESSAGE					3		// Response to read

// CANAL error codes sent over the pipe interface 
// on error responses
#define VSCP_PIPE_ERROR_GENERAL						128
#define VSCP_PIPE_ERROR_UNKNOWN_COMMAND				129
#define VSCP_PIPE_ERROR_CHANNEL_OPEN				130
#define VSCP_PIPE_ERROR_CHANNEL_CLOSED				131
#define VSCP_PIPE_ERROR_SEND_SUCCESS				132
#define VSCP_PIPE_ERROR_SEND_MSG_ALLOCATON			133
#define VSCP_PIPE_ERROR_BUFFER_EMPTY				134
#define VSCP_PIPE_ERROR_BUFFER_FULL					135
#define VSCP_PIPE_ERROR_READ_FAILURE				136
#define VSCP_PIPE_ERROR_SEND_STORAGE				137

