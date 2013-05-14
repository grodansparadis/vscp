// serialdrv.h
//
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// cw debug test: C:\development\vscp\src\vscp\cw\Debug\cw.exe

#ifndef __SERIALDRVH__         
#define __SERIALDRVH__

#include "../../../../common/canal.h"
#include "../../../../common/canal_macro.h"
#include "../../../../common/vscp_class.h"
#include "../../../../common/vscp_type.h"
#include "../../../../common/vscphelper.h"
#include "../../../../../common/dllist.h"
#include "../../../../../../firmware/common/vscp_serial.h"

// This is the version info for this DLL - Change to your own value
#define DLL_VERSION						"0.0.1"
#define NUM_DLL_VERSION					0x000001

// Open flags
#define SERIAL_FLAG_QUEUE_REPLACE		0x00000001	// If input queue is full remove
// oldest message and insert new
// message.
#define SERIAL_FLAG_BLOCK				0x00000002	// Block receive/transmit
#define SERIAL_FLAG_ERROR				0x00000004	// Errors will be received from the adapter.
#define SERIAL_FLAG_USE_SW				0x00000008	// Activate sliding windows protocol	
#define SERIAL_FLAG_NO_LOCAL_SEND		0x00000010	// Don't send transmited frames on
// other local channels for the same
// interface
#define SERIAL_FLAG_SILENT				0x00000020	// Just listen to CAN bus traffic
#define SERIAL_FLAG_LOOPBACK			0x00000040	// Set loopback. Can be combined with
// SERIAL_FLAG_SILENT for HOT loopback

// This is the vendor string - Change to your own value
#define SERIAL_DLL_VENDOR "eurosource, Sweden, http://www.eurosource.se"

#define SERIAL_MAX_CANNELS		16			// Max # of simultainus open channels
#define MAX_VIRTUAL_CHANNELS	64			// Max # of virtual channels for each device

#define CANBUS_RXFIFO_SIZE		0x2000	// Max # of messages in receive fifo
#define CANBUS_TXFIFO_SIZE		0x2000	// Max # of messages in transmit fifo

// Large buffer for receive thread makes 
// it possible to receive large message bursts
#define RECEIVE_THREAD_BUFFER_SIZE		0x400

// Receive state machine states
#define SERIAL_STATE_NONE									0			// 
#define SERIAL_STATE_MSG									1			// Message is beeing read
#define SERIAL_CONTROL_RESPONSE								2			// control response is beeing read
#define SERIAL_POSTITIVE_RESPONSE							3			// Positive response
#define SERIAL_NEGATIVE_RESPONSE							4			// Positive response

// SERIAL commands
#define SERIAL_COMMAND_NOOP									0			// No operation
#define SERIAL_COMMAND_SET_BITRATE						    1			// Set CAN bitrate
#define SERIAL_COMMAND_SET_FILTER							2			// Set acceptance filter
#define SERIAL_COMMAND_SET_MASK								3			// Set acceptance mask
#define SERIAL_COMMAND_OPEN									4			// Open interface
#define SERIAL_COMMAND_CLOSE								5			// Close interface
#define SERIAL_COMMAND_GET_STATUS							6			// Get status
#define SERIAL_COMMAND_GET_VERSION_FIRMWARE		            7			// Get version for firmware
#define SERIAL_COMMAND_GET_VERSION_HARWARE		            8			// Get version for hardware
#define SERIAL_COMMAND_GET_SERIALNO						    9			// Get serial number
#define SERIAL_COMMAND_FIND_BITRATE						    10		    // Find bitrate
#define SERIAL_COMMAND_GET_STAT_FRAME					    11		    // Get statistics frame data
#define SERIAL_COMMAND_GET_STAT_DATA					    12		    // Get statistics data data
#define SERIAL_COMMAND_GET_STAT_ERROR					    13		    // Get statistics error data
#define SERIAL_COMMAND_CLR_STAT								14		    // Clear statistics
#define SERIAL_COMMAND_FLUSH_TX								15		    // Flush transmission queue
#define SERIAL_COMMAND_FLUSH_RX								16		    // Flush receive queue

#define SERIAL_ACCEPTANCE_FILTER_ALL                        0xffffffff;
#define SERIAL_ACCEPTANCE_MASK_ALL                          0xffffffff;

#define LOCK_MUTEX( x )		( WaitForSingleObject( x, INFINITE ) )
#define UNLOCK_MUTEX( x )	( ReleaseMutex( x ) )
#define SLEEP( x )			( Sleep( x ) )
#define SYSLOG( a, b )		( wxLogError( b ) )
#define BZERO( a )			( memset( ( _u8* )&a, 0, sizeof( a ) ) )	

#define MAX( a, b )	( ( (a) > (b) ) ? (a) : (b) )
#define MIN( a, b )	( ( (a) < (b) ) ? (a) : (b) )
#define ABS( a )	(( (int) (a) < 0 ) ? ((a) ^ 0xffffffff) + 1 : (a) )

#define bigtolittle(A) ((((uint32_t)(A) & 0xff000000) >> 24) | \
    (((uint32_t)(A) & 0x00ff0000) >> 8)  | \
    (((uint32_t)(A) & 0x0000ff00) << 8)  | \
    (((uint32_t)(A) & 0x000000ff) << 24))

// Protect driver object
#define SERIAL_DLL_OBJ_MUTEX		"___SERIAL__DOS_OBJ_MUTEX____"

// protect transmit queue
#define SERIAL_DLL_TRANSMIT_MUTEX	"___SERIAL__DOS_TRANSMIT_MUTEX____"

// Receive char event
#define SERIAL_DLL_RECEIVE_CHAR_EVENT	"___SERIAL__DOS_RECEIVE_CHAR_EVENT____"

// Transmit event
#define SERIAL_DLL_TRANSMIT_FRAME_EVENT	"___SERIAL__DOS_TRANSMIT_FRAME_EVENT____"

// Transmit block
#define SERIAL_DLL_TRANSMIT_REMOVE_EVENT	"___SERIAL__DOS_TRANSMIT_REMOVE_EVENT____"



//
// Structure for each simulated channel
//

typedef struct {
    uint32_t m_nChannel;									// Channel this object represents		
    DoubleLinkedList m_rxQueue;								// Receive queue
    HANDLE m_receiveFrameEvent;							
    // Signaled when a frame is available
    HANDLE	m_receiveMutex;									// Mutex for receive queue

    // Callback thread
    BOOL m_bRun;											// TRUE as long as the callback thread should run

} VirtualChannelObj;


// Structure for each channel
typedef struct {

    BOOL m_bInUse;											// TRUE if in use FALSE if free

    uint32_t m_handle;										// Handle for channel 0
    BOOL m_bOpen;											// TRUE if channel is open
    BOOL m_bRun;											// TRUE as long threads should run

    BOOL m_bHardTimeStamp;									// If set timestamp from hardware
    unsigned long m_flags;									// Copy of flags filed in open

    canalStatistics m_statistics;							// Interface statistics

    // Configurable block timeouts
    unsigned long m_blockReciveTimeout;
    unsigned long m_blockTransmitTimeout;

    BOOL m_bSimulatedInterface;						// Marks simulated channel

    DoubleLinkedList m_txQueue;						// Transmit queue

    // Sliding windows
    BOOL m_bUseProtocolSW;							// Use sliding window protocol
    /*volatile*/ vs_swStruct m_swFrameStruct;			// SW Struct
    HANDLE m_swMutex;								// Mutex for sliding window data

    // Serial channel
    HANDLE m_hCommPort;								// Handle for COM port
    uint8_t m_com_channel;							// Sub channel of serial interface
    short m_com_port;								// Interface name that was opened.
    unsigned long m_com_baudrate;					// comm interface baudrate

    // CAN Filter/mask
    uint32_t acceptance_filter;
    uint32_t acceptance_mask;

    // Virtual channels
    DoubleLinkedList m_vChanneldllList;				// A list with virtual channels	

    HANDLE m_comMutex;								// Mutex for the COM channel
    HANDLE m_transmitMutex;							// Mutex for transmit queue
    HANDLE m_virtualChannelsMutex;					// Mutex for virtual channels

    HANDLE m_hTreadReceive;							// Tread id Receive
    HANDLE m_hTreadTransmit;						// Tread id Transmit

    HANDLE m_receiveCharEvent;						// Signaled when data available
    HANDLE m_transmitFrameEvent;					// Signaled when frame to send
    HANDLE m_AckNackEvent;							// Signaled when a ACK/NACK response received
    HANDLE m_transmitRemoveEvent;					// Signale when frame is removed from send queue

} ChannelObj;



// Structure for dll
typedef struct {

    /// Counter for instances using the object
    int m_InstanceCount;

    /// Channel information
    ChannelObj m_pInterfaceObjArray[ SERIAL_MAX_CANNELS ];

} DriverObj;




// Prototypes





/*!

*/
void InitInstance( DriverObj *pdrvObj );

/*!

*/
void Detach( DriverObj *pdrvObj );

/*!

*/
long addChannelObject( DriverObj *pdrvObj, int idx );

/*!

*/
ChannelObj *getDriverObject( DriverObj *pdrvObj, long handle );

/*!

*/
ChannelObj *getFreeDriverObject( DriverObj *pdrvObj, int *pIdx );

/*!

*/
ChannelObj *getDriverObjectFromName( DriverObj *pdrvObj, const char *pName );

/*!

*/
void removeDriverObject( DriverObj *pdrvObj, long handle );

/*!

*/
long AssignNewVirtualHandel( DriverObj *pdrvObj, ChannelObj *pchannelObj );

/*!

*/
long NewVirtualChannel( DriverObj *pdrvObj, ChannelObj *pchannelObj );

/*!

*/
VirtualChannelObj *getVirtualChannel( ChannelObj *pchannelObj, long handle );

/*!

*/
BOOL removeVirtualChannel( ChannelObj *pchannelObj, long handle );

/*!

*/
unsigned long getDataValue( const char *szData );

/*!

*/
BOOL canopen( ChannelObj *pchannelObj, 
             const short com_port,
             const unsigned long com_baudrate,
             const char *szBitrate,
             uint32_t acceptance_code,
             uint32_t acceptance_mask,
             uint32_t flags ) ;

/*!

*/
BOOL canclose( ChannelObj *pchannelObj, CANHANDLE channel );

/*!

*/
int canreadMsg( ChannelObj *pchannelObj, 
               CANHANDLE channel, 
               canalMsg *pMsg, 
               BOOL bBlocking  );

/*!

*/
int canwriteMsg( ChannelObj *pchannelObj, 
                CANHANDLE channel, 
                canalMsg *pMsg, 
                BOOL bBlocking  );

/*!

*/
int cangetStatus( ChannelObj *pchannelObj, CANHANDLE channel );

/*!

*/
int cangetVersion( ChannelObj *pchannelObj, CANHANDLE channel, char *szVersion );

/*!

*/
int cansetMask( ChannelObj *pchannelObj, CANHANDLE channel, uint32_t mask );

/*!

*/
int cansetAcceptanceCode( ChannelObj *pchannelObj, CANHANDLE channel, uint32_t mask );


/*!

*/
int canDataAvailable( ChannelObj *pchannelObj, CANHANDLE channel );

/*!

*/
char com_readChar( HANDLE hCommPort, int* cnt );

/*!

*/
uint16_t com_readBuf( HANDLE hCommPort,
                     char *pBuf, 
                     uint16_t size );

/*!

*/
void com_write( HANDLE hCommPort, char *pstr, BOOL bCRLF, BOOL bNoLF );

/*!

*/
BOOL com_writebuf( HANDLE hCommPort, unsigned char *p, unsigned short cnt );





#endif // __SERIALDRVH__
