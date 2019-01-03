// zanthicobj.h: 
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
// Copyright (C) 2000 Steven D. Letkeman/Zanthic Technologies Inc.
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
// $RCSfile: zanthicobj.h,v $                                       
// $Date: 2005/01/05 12:16:19 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 

#if !defined(IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
#define IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_



#ifdef WIN32

#include <windows.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#else

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <syslog.h>

#endif

#include <canal.h>
#include <vscp.h>
#include <canal_macro.h>
#include <dllist.h>


#define CANAL_DLL_ZANTHICDRV_OBJ_MUTEX	"___CANAL__DLL_ZANTHICDRV_OBJ_MUTEX____"
#define CANAL_DLL_ZANTHICDRV_RECEIVE_MUTEX	"___CANAL__DLL_ZANTHICDRV_RECEIVE_MUTEX____"
#define CANAL_DLL_ZANTHICDRV_TRANSMIT_MUTEX "___CANAL__DLL_ZANTHICDRV_TRANSMIT_MUTEX____"

// Max messages in input queue
#define ZANTHIC_MAX_RCVMSG		512

// Max messages in output queue
#define ZANTHIC_MAX_SNDMSG		512

typedef struct _BULK_TRANSFER_CONTROL
{
    unsigned long pipeNum;
}   BULK_TRANSFER_CONTROL, *PBULK_TRANSFER_CONTROL;



//
// Set the base of the IOCTL control codes.  This is somewhat of an
// arbitrary base number, so you can change this if you want unique
// IOCTL codes.  You should consult the Windows NT DDK for valid ranges
// of IOCTL index codes before you choose a base index number.
//

#define Ezusb_IOCTL_INDEX  0x0800

//
// Perform an IN transfer over the specified bulk or interrupt pipe.
//
// lpInBuffer: BULK_TRANSFER_CONTROL stucture specifying the pipe number to read from
// nInBufferSize: sizeof(BULK_TRANSFER_CONTROL)
// lpOutBuffer: Buffer to hold data read from the device.  
// nOutputBufferSize: size of lpOutBuffer.  This parameter determines
//    the size of the USB transfer.
// lpBytesReturned: actual number of bytes read
// 
#define IOCTL_EZUSB_BULK_READ             CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+19,\
                                                   METHOD_OUT_DIRECT,  \
                                                   FILE_ANY_ACCESS)

//
// Perform an OUT transfer over the specified bulk or interrupt pipe.
//
// lpInBuffer: BULK_TRANSFER_CONTROL stucture specifying the pipe number to write to
// nInBufferSize: sizeof(BULK_TRANSFER_CONTROL)
// lpOutBuffer: Buffer of data to write to the device
// nOutputBufferSize: size of lpOutBuffer.  This parameter determines
//    the size of the USB transfer.
// lpBytesReturned: actual number of bytes written
// 
#define IOCTL_EZUSB_BULK_WRITE            CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+20,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)



#define ZANTECH_ACK         1
#define ZANTECH_NACK        0
#define GETSTATSREQ         2   // incoming packets have the second byte that contain a command code for desired action
#define GETSTATSRESP        3
#define INITREQ             4
#define INITRESP            5
#define SETBAUDREQ          6
#define SETBAUDRESP         7
#define SETFILTERREQ        8
#define SETFILTERRESP       9
#define SENDCANREQ          0x10
#define SENDCANRESP         0x11
#define RECCANREQ           0x12
#define RECCANRESP          0x13
#define GETCANMESREQ        0x14
#define GETCANMESRESP       0x15
#define READREGREQ          0x16
#define READREGRESP         0x17
#define WRITEREGREQ         0x18
#define WRITEREGRESP        0x19
// 0x1A is disable message object
#define GETNUMMESREQ        0x1C // get the number of messages in queue but don't do anything with them
#define GETNUMMESRESP       0x1D
#define CLEARMESREQ         0x1E
#define CLEARMESRESP        0x1F

// Device handling specific error codes
#define ZANTHIC_ERROR_GENERAL               -1  // for generic fail (no board present)
#define ZANTHIC_ERROR_NO_RESPONSE           -2  // for no response
#define ZANTHIC_ERROR_PARAM                 -3  // for improper calling parameter
#define ZANTHIC_ERROR_RESPONSE              -4  // for improper response
#define ZANTHIC_ERROR_DBL_RESPONSE          -5  // if more than one unit responded
#define ZANTHIC_ERROR_BUSY                  -6  // if system is busy at this time
#define ZANTHIC_ERROR_BULK_WRITE            -7  // for failed in bulk write
#define ZANTHIC_ERROR_BULK_READ             -8  // for failed in bulk read
#define ZANTHIC_ERROR_IF_NO_RESPONSE        -9  // for interface not responding
#define ZANTHIC_ERROR_CONFIG_LOAD           -10 // if interface config file not loaded properly
#define ZANTHIC_ERROR_START_DRIVER          -11 // start driver routine failed
#define ZANTHIC_ERROR_LOAD_DRIVER           -12 // load driver routine failed
#define ZANTHIC_ERROR_OPEN_DEVICE           -13 // could not open device
#define ZANTHIC_ERROR_OPEN_ISR              -14 // could not open ISR device
#define ZANTHIC_ERROR_OPEN_ISR_TREAD        -15 // could not open ISR thread
#define ZANTHIC_ERROR_LOADED                -16 // driver already loaded


class CZanthicObj
{

public:

    /// Constructor
    CZanthicObj();

    /// Destructor
    virtual ~CZanthicObj();


    /*!
        Filter message

        @param pcanalMsg Pointer to CAN message
        @return True if message is accepted false if rejected
    */
    bool doFilter( canalMsg *pcanalMsg );


    /*!
        Set Filter
    */
    bool setFilter( unsigned long filter );


    /*!
        Set Mask
    */
    bool setMask( unsigned long mask);


    /*!
        Open/create the logfile

        @param szFileName the name of the log file
        @param flags 	bit 1 = 0 Append, bit 1 = 1 Rewrite
        @return True on success.
    */
    bool open( const char *szFileName, unsigned long flags = 0 );

    /*!
        Flush and close the log file
    */
    bool close( void );


    /*!
        Get Interface statistics
        @param pCanalStatistics Pointer to CANAL statistics structure
        @return True on success.
    */
    bool getStatistics( PCANALSTATISTICS pCanalStatistics );


    /*!
        Write a message out to the file
        @param pcanalMsg Pointer to CAN message
        @return True on success.
    */
    bool writeMsg( canalMsg *pMsg );


    /*!
        Read a message fro the device
        @param pcanalMsg Pointer to CAN message
        @return True on success.
    */
    bool readMsg( canalMsg *pMsg );


    /*!
        Check for data availability
        @return Number of packages in the queue
    */
    int dataAvailable( void );

    /*!
        Get device status
        @param pCanalStatus Pointer to CANAL status structure
        @return True on success.
    */
    bool getStatus( PCANALSTATUS pCanalStatus );


    /*!
        Set Device speed

        @param speed = 10,20,50,100,125,250,500,800,1000 in kbps
        @return Result code. negative if error.
    */
    short setDeviceSpeed( short speed );

    /*!
        // This function will execute the internal reset command on the CAN controller

        @return Result code. negative if error.
    */
    short resetDevice( void );

    /*!
        Get Device information

        @param Version will get filled with two bytes for major and minor version
        @param Feature is an 8 bit flag byte to show which features are availble
        @param Manufact will be a string up to 20 characters of the board manufacturer name
        @param CANCont is a byte array with the first character containing the number of CAN
                controllers within this board.  The next bytes are a value to show what type of
                CAN controller this is
        @return Result code. negative if error.
    */
    short getDeviceInfo( uint8_t *Version,
                            uint8_t *Feature,
                            char *pManufact,
                            uint8_t *CANCont );

    /*!
        Send a message to the device

        @param ID is the CAN ID, right justified with bit 31=1 for extended, 
                30=1 for RTR
        @param CB1 = Command Byte 1. Bit 2=Txwait bit, bit 1&0 are priority
        @param CB2 = Command Byte 2. Upper 4 bits = Message object, 
                Lower 4 bits=Number of data bytes
        @param databytes is a pointer to type unsigned char of 0-8 data bytes of data
        @return Result code. negative if error.
    */
    short sendMsgToDevice( uint32_t ID, uint8_t CB1, uint8_t CB2, uint8_t* databytes );



    /*!
        Read register from the device

        @param Address is the start address within the CAN controller(0-255)
        @param Numbytes is the number of bytes to read (1-60)
        @param buffer is the user buffer that will get the values written to
        @return Result code. negative if error.
    */
    short readReg( uint8_t Address, uint8_t Numbytes, uint8_t *buffer);


    /*!
        Write data to a device register

        @param Address is the start address within the CAN controller(0-255)
        @param Numbytes is the number of bytes to write (1-60)
        @param buffer is the user buffer that contains the values to be written 
        @return Result code. negative if error.
    */
    short writeReg( uint8_t Address, uint8_t Numbytes, uint8_t *buffer );


    /*!
        Fetch messages from device

        The Get CAN Message function, when called from this function to the CAN-4-USB
        device, will return up to 7 packets.  

        @return Result code. negative if error.
    */
    short getMsgFromDevice( void );

    /*!
        Get messages from device. 

        @return Result code. negative if error.
    */
    short getMsgCntFromDevice( void );


    /*!
        Clear the receive buffer

        @return Result code. negative if error.
    */
    short clearReceiveBuffer(void );

    
    /*!
        Set adapter internal filter
        
        @param FilterNum is a value from 0-5 for the MCP2510
        @param FilterVal is a 4 byte long with the filter values
        The MSB filter value will determine if this is a 29 bit ID
        if not set then it is 11 bit ID and only the least sig 11 bits are used
        @return Result code. negative if error.
    */
    short setDeviceFilter( uint8_t nFilter, uint32_t filter );


    /*!
        @param ID is the CAN ID, right justified with bit 31 set to one for extended
        @param CB1 = Command Byte 1. upper 4 bits = Message object to use	
        @return Result code. negative if error.
    */
    short setDeviceMask( uint32_t mask, uint8_t cb1 );


    /*!
        closeDriver  
        Close the device driver
            
        @return true on success
    */                                
    void closeDriver( void );

    /*!
        openDriver  
        Opens the device driver  
            
        @return Boolean that indicates if the driver was
                successfully opened or not.
    */                                
    bool openDriver( void );

    /*!
        Send command to device

        @param outbuffer contains the packet being sent to the USB device
        @param  numbytes is the number of bytes to send
        @param  inbuffer is the returned packet from USB device
        @returns Result of command
                    -1 for failed to load driver
                    -7 for failed in Bulk write
                    -8 for failed in Bulk read
                    -3 for a parameter out of bounds
                    1+ for number of data bytes returned
    */
    short doCommand( uint8_t *outbuffer, uint8_t numbytes, uint8_t *inbuffer );


public:

    /// Run flag
    bool m_bRun;

    /// Driver index
    uint8_t m_drvIdx;


    /*!
        Interface statistics
    */
    canalStatistics m_stat;


    /*!
        Interface status ( 0 is returned at the moment ).

        Bit 0  - Reserved
        Bit 1  - Reserved.
        Bit 2  - Reserved.
        Bit 3  - Reserved.
        Bit 4  - Reserved.
        Bit 5  - Reserved.
        Bit 6  - Reserved.
        Bit 7  - Reserved.
        Bit 8  - Reserved.
        Bit 9  - Reserved.
        Bit 10 - Reserved.
        Bit 11 - Reserved.
        Bit 12 - Reserved.
        Bit 13 - Reserved.
        Bit 14 - Reserved.
        Bit 15 - Reserved.
        Bit 16 - Reserved.
        Bit 17 - Reserved.
        Bit 18 - Reserved.
        Bit 19 - Reserved
        Bit 20 - Reserved.
        Bit 21 - Reserved.
        Bit 22 - Reserved.
        Bit 23 - Reserved.
        Bit 24 - Reserved.
        Bit 25 - Reserved.
        Bit 26 - Reserved.
        Bit 27 - Reserved.
        Bit 28 - Reserved.
        Bit 29 - Reserved.
        Bit 30 - Reserved.
        Bit 31 - Reserved.
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
    HANDLE m_zanthicMutex;
#else
    pthread_mutex_t m_zanthicMutex;
#endif

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
        Filter for outgoing messages

        mask bit n	|	filter bit n	| msg id bit	|	result
        ===========================================================
            0               X                   X           Accept				
            1               0                   0           Accept
            1               0                   1           Reject
            1               1                   0           Reject
            1               1                   1           Accept

        Formula is !( ( filter \EEd ) & mask )
    */
    uint32_t m_filter;

    /*!
        Mask for outgoing messages	
    */
    uint32_t m_mask;

    
    /*!
        Adapter handle
    */
    HANDLE m_phDeviceHandle;
    
    /*!
        Transmit queue handle
    */
    uint16_t m_hTxQue;
    
    /*!
        Receive queue handle
    */
    uint16_t m_hRxQue;	
 
};

#endif // !defined(ZANTHIC_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
