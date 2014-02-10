// dos_can.h
//
// Copyright (C) 2005-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Version 0.0.1
//

#ifndef __DOSCANH__         
#define __DOSCANH__


#ifdef __cplusplus
  extern "C" {
#endif

// Types
typedef unsigned char _u8;
typedef unsigned short _u16;
typedef unsigned long _u32;

typedef long CANHANDLE;
typedef unsigned char CANDATA;

// Status bits
#define CANSTATUS_RECEIVE_FIFO_FULL		0x01
#define CANSTATUS_TRANSMIT_FIFO_FULL	0x02
#define CANSTATUS_ERROR_WARNING			0x04
#define CANSTATUS_DATA_OVERRUN			0x08
#define CANSTATUS_ERROR_PASSIVE			0x20
#define CANSTATUS_ARBITRATION_LOST		0x40
#define CANSTATUS_BUS_ERROR				0x80

// Filter mask settings
#define CAN232_ACCEPTANCE_CODE_ALL		0x00000000
#define CAN232_ACCEPTANCE_MASK_ALL		0xFFFFFFFF

// Message flags
#define CANMSG_EXTENDED		0x80 // Extended CAN id
#define CANMSG_RTR			0x40 // Remote frame

// Flush flags
#define FLUSH_WAIT			0x00
#define FLUSH_DONTWAIT		0x01
#define FLUSH_EMPTY_INQUEUE	0x02

// CAN Frame
typedef struct {
    _u32 id;        // Message id
    _u32 timestamp; // timestamp in milliseconds	
    _u8  flags;     // [extended_id|1][RTR:1][reserver:6]
    _u8  len;       // Frame size (0.8)
    _u8  data[ 8 ]; // Databytes 0..7
} CANMsg;


// Alternative CAN Frame
typedef struct {
	_u32 id;        // Message id
	_u32 timestamp; // timestamp in milliseconds
	_u8  flags;     // [extended_id|1][RTR:1][reserver:6]
	_u8  len;       // Frame size (0.8)
} CANMsgEx;

// Interface statistics
typedef struct structCAN232Statistics {
	_u32 cntReceiveFrames;			// # of receive frames
	_u32 cntTransmitFrames;			// # of transmitted frames
	_u32 cntReceiveData;			// # of received data bytes
	_u32 cntTransmitData;			// # of transmitted data bytes	
	_u32 cntOverruns;				// # of overruns
	_u32 cntBusWarnings;			// # of bys warnings
	_u32 cntBusOff;					// # of bus off's
} CAN232Statistics;


// Error return codes
#define ERROR_CAN232_OK					1
#define ERROR_CAN232_GENERAL			-1
#define ERROR_CAN232_OPEN_SUBSYSTEM		-2
#define ERROR_CAN232_COMMAND_SUBSYSTEM	-3
#define ERROR_CAN232_NOT_OPEN			-4
#define ERROR_CAN232_TX_FIFO_FULL		-5
#define ERROR_CAN232_INVALID_PARAM		-6
#define ERROR_CAN232_NO_MESSAGE			-7
#define ERROR_CAN232_MEMORY_ERROR		-8
#define ERROR_CAN232_NO_DEVICE			-9
#define ERROR_CAN232_TIMEOUT			-10
#define ERROR_CAN232_INVALID_HARDWARE	-11


// Open flags
#define CAN232_FLAG_TIMESTAMP			0x0001	// Timestamp messages
#define CAN232_FLAG_QUEUE_REPLACE		0x0002	// If input queue is full remove
												// oldest message and insert new
												// message.
#define CAN232_FLAG_BLOCK				0x0004	// Block receive/transmit
#define CAN232_FLAG_SLOW				0x0008	// Check ACK/NACK's
#define CAN232_FLAG_NO_LOCAL_SEND		0x0010	// Don't send transmited frames on
												// other local channels for the same
												// interface

// Prototypes

///////////////////////////////////////////////////////////////////////////////
// can232_Open
//
// Open CAN interface to device
//
// Returs handle to device if open was successfull or zero
// or negative error code on falure.
//
//
// szID
// ====
// Serial number for adapter or NULL to open the first found.
//
//
// szBitrate
// =========
// "10" for 10kbps  
// "20" for 20kbps
// "50" for 50kbps
// "100" for 100kbps
// "250" for 250kbps
// "500" for 500kbps
// "800" for 800kbps
// "1000" for 1Mbps
//
// or 
//
// btr0:btr1 pair  ex. "0x03:0x1c" or 3:28
//
// acceptance_code
// ===============
// Set to CAN232_ACCEPTANCE_CODE_ALL to  get all messages.
//
// acceptance_mask
// ===============
// Set to CAN232_ACCEPTANCE_MASk_ALL to  get all messages.
//
// flags
// =====
// CAN232_FLAG_TIMESTAMP - Timestamp will be set by adapter.
 
CANHANDLE WINAPI can232_Open( LPCSTR szPort,
								const int nBaudRate,
								LPCSTR szBitrate,
								_u32 acceptance_code,
								_u32 acceptance_mask,
								_u32 flags );



///////////////////////////////////////////////////////////////////////////////
// can232_Close
//
// Close channel with handle h.
//
// Returns <= 0 on failure. >0 on success.

int WINAPI can232_Close( CANHANDLE h );



///////////////////////////////////////////////////////////////////////////////
// can232_Read 
//
// Read message from channel with handle h. 
//
// Returns <= 0 on failure. >0 on success.
//

int WINAPI can232_Read( CANHANDLE h, CANMsg *msg );


///////////////////////////////////////////////////////////////////////////////
// can232_ReadEx
//
// Read message from channel with handle h.
//
// This is a version without a data-array in the structure to work with LabView
//
// Returns <= 0 on failure. >0 on success.
//

int WINAPI can232_ReadEx( CANHANDLE h, CANMsgEx *msg, CANDATA *pData );

///////////////////////////////////////////////////////////////////////////////
// can232_ReadFirst 
//
// Read message from channel with handle h and id "id" which satisfy flags. 
//
// Returns <= 0 on failure. >0 on success.
//

int WINAPI can232_ReadFirst( CANHANDLE h,
								_u32 id,
								_u32 flags,
								CANMsg *msg );


///////////////////////////////////////////////////////////////////////////////
// can232_ReadFirstEx
//
// Read message from channel with handle h and id "id" which satisfying flags.
//
// This is a version without a data-array in the structure to work with LabView
//
// Returns <= 0 on failure. >0 on success.
//

int WINAPI can232_ReadFirstEx( CANHANDLE h, 
									_u32 id, 
									_u32 flags, 
									CANMsgEx *msg, 
									CANDATA *pData );


///////////////////////////////////////////////////////////////////////////////
// can232_Write
//
// Write message to channel with handle h.
//
// Returns <= 0 on failure. >0 on success.
//

int WINAPI can232_Write( CANHANDLE h, CANMsg *msg );


///////////////////////////////////////////////////////////////////////////////
// can232_WriteEx
//
// Write message to channel with handle h. 
//
// This is a version without a data-array in the structure to work with LabView
//
// Returns <= 0 on failure. >0 on success.
//

int WINAPI can232_WriteEx( CANHANDLE h, CANMsgEx *msg, CANDATA *pData );



///////////////////////////////////////////////////////////////////////////////
// can232_Status
//
// Get Adaper status for channel with handle h. 

int WINAPI can232_Status( CANHANDLE h );



///////////////////////////////////////////////////////////////////////////////
// can232_VersionInfo
//
// Get hardware/fi4rmware and driver version for channel with handle h. 
//
// Returns <= 0 on failure. >0 on success.
//
// 

int WINAPI can232_VersionInfo( CANHANDLE h, LPSTR verinfo );


///////////////////////////////////////////////////////////////////////////////
// can232_Flush
//
// Flush output buffer on channel with handle h. 
//
// Returns <= 0 on failure. >0 on success.
//
// If flushflags is set to FLUSH_DONTWAIT the queue is just emptied and 
// there will be no wait for any frames in it to be sent 
//

int WINAPI can232_Flush( CANHANDLE h, _u8 flushflags );


///////////////////////////////////////////////////////////////////////////////
// can232_GetStatistics
//
// Get transmission statistics for channel with handle h. 
//
// Returns <= 0 on failure. >0 on success.
//
//

int WINAPI can232_GetStatistics( CANHANDLE h, CAN232Statistics *pStatistics );

///////////////////////////////////////////////////////////////////////////////
// can232_SetTimeouts
//
// Set timeouts used for blocking calls for channel with handle h. 
//
// Returns <= 0 on failure. >0 on success.
//
//

int WINAPI can232_SetTimeouts( CANHANDLE h, 
								_u32 receiveTimeout, 
								_u32 transmitTimeout );


#ifdef __cplusplus
}
#endif

#endif // __DOSCANH__
