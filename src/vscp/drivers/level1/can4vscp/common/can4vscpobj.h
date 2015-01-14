// can4vscpobj.h: 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 
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
#include <string.h>
#include <pthread.h>
#include <syslog.h>

#endif

#include <stdio.h>
#include <canal.h>
#include <vscp.h>
#include <canal_macro.h>
#include <dllist.h>

// define to create a file c:\can4vscp.txt with input and
// stat machine data for debugging
//#define DEBUG_CAN4VSCP_RECEIVE

#define CANAL_DLL_CAN4VSCPDRV_OBJ_MUTEX			TEXT("___CANAL__DLL_CAN4VSCPDRV_OBJ_MUTEX____")
#define CANAL_DLL_CAN4VSCPDRV_RECEIVE_MUTEX		TEXT("___CANAL__DLL_CAN4VSCPDRV_RECEIVE_MUTEX____")
#define CANAL_DLL_CAN4VSCPDRV_TRANSMIT_MUTEX	TEXT("___CANAL__DLL_CAN4VSCPDRV_TRANSMIT_MUTEX____")
#define CANAL_DLL_CAN4VSCPDRV_RESPONSE_MUTEX	TEXT("___CANAL__DLL_CAN4VSCPDRV_RESPONSE_MUTEX____")

// Flags
#define CAN4VSCP_FLAG_NO_SWITCH_TO_NEW_MODE     4

// Max messages in input queue
#define CAN4VSCP_MAX_RCVMSG			            1024

// Max messages in output queue
#define CAN4VSCP_MAX_SNDMSG			            1024

// Max number of response messages in respnse queue
#define CAN4VSCP_MAX_RESPONSEMSG	            32

// Byte stuffing start and end characters
#define DLE						                0x10
#define STX						                0x02
#define ETX						                0x03

// RX State machine
#define INCOMING_STATE_NONE			            0	// Waiting for <STX>
#define INCOMING_STATE_STX			            1	// Reading data
#define INCOMING_STATE_ETX			            2	// <ETX> has been received
#define INCOMING_STATE_COMPLETE		            3	// Frame received

#define INCOMING_SUBSTATE_NONE		            0	// Idle
#define INCOMING_SUBSTATE_DLE		            1	// <DLE> received


// Can4VSCP Commands
#define RESET_NOOP			                    0x00	// No Operation
#define	GET_TX_ERR_CNT			                0x02	// Get TX error count
#define	GET_RX_ERR_CNT			                0x03	// Get RX error count
#define	GET_CANSTAT				                0x04	// Get CAN statistics
#define	GET_COMSTAT				                0x05
#define	GET_MSGFILTER1			                0x06	// Get message filter 1
#define	GET_MSGFILTER2			                0x07	// Get message filter 2
#define	SET_MSGFILTER1			                0x08	// Set message filter 1
#define	SET_MSGFILTER2			                0x09	// Set message filter 2



// Emergency flags
#define EMERGENCY_OVERFLOW		                0x01
#define EMERGENCY_RCV_WARNING	                0x02
#define EMERGENCY_TX_WARNING	                0x04
#define EMERGENCY_TXBUS_PASSIVE	                0x08
#define EMERGENCY_RXBUS_PASSIVE	                0x10
#define EMERGENCY_BUS_OFF		                0x20

// VSCP Driver positions in frame
#define VSCP_DRIVER_POS_FRAME_TYPE              0
#define VSCP_DRIVER_POS_FRAME_CHANNEL           1
#define VSCP_DRIVER_POS_FRAME_SEQUENCY          2
#define VSCP_DRIVER_POS_FRAME_SIZE_PAYLOAD_MSB  3
#define VSCP_DRIVER_POS_FRAME_SIZE_PAYLOAD_LSB  4
#define VSCP_DRIVER_POS_FRAME_PAYLOAD           5

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


//
// The command response structure
//
typedef struct {
    uint8_t op;             // Operation == framtype
    uint8_t	seq;            // Sequency number
    uint8_t	channel;        // Channel
    uint16_t sizePayload;   // Size of payload
    uint8_t payload[ 512 ];
} cmdResponseMsg;


class CCan4VSCPObj
{

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
    bool doFilter( canalMsg *pcanalMsg );


    /*!
        Set Filter
    */
    int setFilter( unsigned long filter );


    /*!
        Set Mask
    */
    int setMask( unsigned long mask);


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
    int open( const char *pConfig, unsigned long flags = 0 );


    /*!
        Flush and close the log file
    */
    int close( void );


    /*!
        Get Interface statistics
        @param pCanalStatistics Pointer to CANAL statistics structure
        @return True on success.
    */
    int getStatistics( PCANALSTATISTICS pCanalStatistics );


    /*!
        Write a message out to the device (non blocking)
        @param pcanalMsg Pointer to CAN message
        @return True on success.
    */
    int writeMsg( canalMsg *pMsg );

    /*!
        Write a message out to the device (blocking)
        @param pcanalMsg Pointer to CAN message
        @return CANAL return code. CANAL_ERROR_SUCCESS on success.
    */
    int writeMsgBlocking( canalMsg *pMsg, uint32_t Timeout );

    /*!
        Read a message fro the device
        @param pcanalMsg Pointer to CAN message
        @return True on success.
    */
    int readMsg( canalMsg *pMsg );


    /*!
        Read a message from the device (Blocking)
        @param pcanalMsg Pointer to CAN message
        @param timout Timout in millisconds
        @return CANAL return code. CANAL_ERROR_SUCCESS on success.
    */
    int readMsgBlocking( canalMsg *pMsg, uint32_t timeout );


    /*!
        Check for data availability
        @return Number of packages in the queue
    */
    int dataAvailable( void );

    /*!
		Handle for receive event to know when to call readMsg
		@return Handle
	*/
	HANDLE getReceiveHandle( void ) { return m_receiveDataEvent; }

    /*!
        Get device status
        @param pCanalStatus Pointer to CANAL status structure
        @return True on success.
    */
    int getStatus( PCANALSTATUS pCanalStatus );


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
        Wait for a response message

        @param pMsg Pointer to response message
        @param cmdcode Command to wait for reply from
        @param saveseq Squency number when frame was sent.
        @param timeout Timeout in milliseconds
        @return True on success
    */
    bool waitResponse( cmdResponseMsg *pMsg, 
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
        Send command on the serial channel

        @param buffer Contains data to send
        @param size Total number of bytes to send
        @return true on success
    */
    bool sendMsg( uint8_t *buffer, short size );

    /*!
        Check CRC for frame in buffer defined by
        content in m_bufferMsgRcv and with length
        m_lengthMsgRcv
    */
    bool checkCRC( void );

    /*!
        Send ACK
        \param seq Sequency number
    */
    void sendACK( uint8_t seq );

    /*!
        Send NACK
        \param seq Sequency number
    */
    void sendNACK( uint8_t seq );

    /*!
        Send NOOP frame
    */
    void sendNoopFrame( void );

    /*!
        Send Open Frame
        \mode is 0=Nomal. 1=Listen, 2=Loopback, 3=Configuration
    */
    void sendOpenInterfaceFrame( uint8_t mode );

    /*!
        Send Close Interface frame
    */
    void sendCloseInterfaceFrame( void );

    /*!
        Add current frame in buffer to response queue
        \return true on success
    */
    bool addToResponseQueue( void );

    /*!
        Read serial data and feed to state machine
        @return true when a full frame is received.
    */
    bool serialData2StateMachine( void );

    /*!
        Do reading and interpreet data
    */
    void readSerialData( void );

public:

    /// Run flag
    bool m_bRun;

    // Open flag
	bool m_bOpen;


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

    HANDLE      m_receiveDataEvent;         // GS
    HANDLE      m_transmitDataPutEvent;     // GS
    HANDLE      m_transmitDataGetEvent;     // GS


    /*!
        Mutex for receive queue.
    */
#ifdef WIN32	
    HANDLE m_receiveMutex;
#else
    pthread_mutex_t m_receiveMutex;
#endif

    /*!
        Mutex for transmit queue.
    */
#ifdef WIN32	
    HANDLE m_transmitMutex;
#else
    pthread_mutex_t m_transmitMutex;
#endif

    /*!
        Mutex for command response queue.
    */
#ifdef WIN32	
    HANDLE m_responseMutex;
#else
    pthread_mutex_t m_responseMutex;
#endif

    /*!
    Filter for outgoing messages

    mask bit n	|	filter bit n	| msg id bit	|	result
    ===========================================================
    0				X					X			Accept				
    1				0					0			Accept
    1				0					1			Reject
    1				1					0			Reject
    1				1					1			Accept

    Formula is !( ( filter \EEd ) & mask )
    */
    uint32_t m_filter;

    /*!
        Mask for outgoing messages	
    */
    uint32_t m_mask;


    ///////////////////////////////////////////////////////////////////////////
    //				 * * * * *  Can4VSCP specific * * * * *
    ///////////////////////////////////////////////////////////////////////////


    /*!
        flags 
    */
    uint32_t m_initFlag;


    CComm m_com;


    /*!
        State for incoming frames
    */
    int  m_RxMsgState;

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


};

#endif // !defined(AFX_IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
