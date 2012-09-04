// pipecmd.h
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: canalpipecmd.h,v $                                       
// $Date: 2005/08/30 11:00:04 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 

// CANAL commands sent over the pipe interface
#define CANAL_PIPE_COMMAND_NOOP						  0		  // No command
#define CANAL_PIPE_COMMAND_OPEN						  1		  // Open channel 
#define CANAL_PIPE_COMMAND_CLOSE					  2		  // Close channel
#define CANAL_PIPE_COMMAND_SEND						  3		  // Send message
#define CANAL_PIPE_COMMAND_RECEIVE				  4		  // Receive message
#define CANAL_PIPE_COMMAND_CHECKDATA			  5		  // Check if data is available
#define CANAL_PIPE_COMMAND_BAUDRATE				  6		  // Set Baudrate
#define CANAL_PIPE_COMMAND_STATUS					  7		  // Get status
#define CANAL_PIPE_COMMAND_STATISTICS			  8		  // Get statistics
#define CANAL_PIPE_COMMAND_FILTER					  9		  // Set filter
#define CANAL_PIPE_COMMAND_MASK						  10		// Set mask
#define CANAL_PIPE_COMMAND_VERSION				  11		// CANAL version
#define CANAL_PIPE_COMMAND_DLL_VERSION		  12		// CANAL DLL version
#define CANAL_PIPE_COMMAND_VENDOR_STRING	  13		// CANAL vendor string
#define CANAL_PIPE_COMMAND_LEVEL					  14		// CANAL Level bitarray

// CANAL responses sent over the pipe interface
#define CANAL_PIPE_RESPONSE_NONE					  0		// 
#define CANAL_PIPE_RESPONSE_SUCCESS					1		// OK message
#define CANAL_PIPE_RESPONSE_ERROR					  2		// ERROR message
#define CANAL_PIPE_RESPONSE_MESSAGE					3		// Response to read

// CANAL error codes sent over the pipe interface 
// on error responses
#define CANAL_PIPE_ERROR_GENERAL					      128
#define CANAL_PIPE_ERROR_UNKNOWN_COMMAND			  129
#define CANAL_PIPE_ERROR_CHANNEL_OPEN				    130
#define CANAL_PIPE_ERROR_CHANNEL_CLOSED				  131
#define CANAL_PIPE_ERROR_SEND_SUCCESS				    132
#define CANAL_PIPE_ERROR_SEND_MSG_ALLOCATON			133
#define CANAL_PIPE_ERROR_BUFFER_EMPTY				    134
#define CANAL_PIPE_ERROR_BUFFER_FULL				    135
#define CANAL_PIPE_ERROR_READ_FAILURE				    136
#define CANAL_PIPE_ERROR_SEND_STORAGE				    137

