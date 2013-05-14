// usb2canobj.cpp:  
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Copyright (C) 2009 Gediminas Simanskis , edevices, www.edevices.lt
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
// $RCSfile: usb2canobj.cpp,v $                                       
// $Date: 2005/06/09 06:47:44 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 

#include "stdio.h"
#include "usb2canobj.h"
#include "dlldrvobj.h"
#include "callback.h"

// Prototypes

void workThreadTransmit( void *pObject );
void workThreadReceiveData( void *pObject );
void workThreadReceiveCmd( void *pObject );


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// CUsb2canObj
//

CUsb2canObj::CUsb2canObj()
{ 
	m_initFlag = 0;
	
	// No filter mask
	m_filter = 0;
	m_mask = 0;

	m_bRun = false;

	hDataOut = NULL; 
	hDataIn = NULL;
	hCmdOut = NULL; 
	hCmdIn = NULL;
	hDev = NULL;

	
	m_hTreadReceive = 0;
	m_hTreadTransmit = 0;

	// Create the device AND LIST access mutexes
	m_usb2canMutex = CreateMutex( NULL, true, CANAL_DLL_USB2CANDRV_OBJ_MUTEX );
	m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_USB2CANDRV_RECEIVE_MUTEX );
	m_transmitMutex = CreateMutex( NULL, true, CANAL_DLL_USB2CANDRV_TRANSMIT_MUTEX );
	m_commandMutex = CreateMutex( NULL, true, CANAL_DLL_USB2CANDRV_COMMAND_MUTEX );
    m_responseDataMutex = CreateMutex( NULL, true, CANAL_DLL_USB2CANDRV_RESPONSE_DATA_MUTEX );

	//m_responseDataEvent = CreateEvent( NULL,TRUE,FALSE,NULL );

	dll_init( &m_transmitList, SORT_NONE );
	dll_init( &m_receiveList, SORT_NONE );
	dll_init( &m_responseCmdList, SORT_NONE );
	dll_init( &m_responseDataList, SORT_NONE );
}

//////////////////////////////////////////////////////////////////////
// ~CUsb2canObj
//

CUsb2canObj::~CUsb2canObj()
{		 
	close();
	
	LOCK_MUTEX( m_transmitMutex );
	dll_removeAllNodes( &m_transmitList );
	
	LOCK_MUTEX( m_receiveMutex );
	dll_removeAllNodes( &m_receiveList );

	LOCK_MUTEX( m_commandMutex );
	dll_removeAllNodes( &m_responseCmdList );

	LOCK_MUTEX( m_responseDataMutex );
	dll_removeAllNodes( &m_responseDataList );

	
	if ( NULL != m_usb2canMutex ) CloseHandle( m_usb2canMutex );	
	if ( NULL != m_receiveMutex ) CloseHandle( m_receiveMutex );
	if ( NULL != m_transmitMutex ) CloseHandle( m_transmitMutex );
	if ( NULL != m_commandMutex ) CloseHandle( m_commandMutex );
	if ( NULL != m_responseDataMutex ) CloseHandle( m_responseDataMutex );

    //if ( NULL != m_responseDataEvent ) CloseHandle( m_responseDataEvent );

	if( hDataOut != NULL) CloseHandle(hDataOut); 
	if( hDataIn != NULL ) CloseHandle(hDataIn);
	if( hCmdOut != NULL ) CloseHandle(hCmdOut);
	if( hCmdIn != NULL )  CloseHandle(hCmdIn);
	if( hDev != NULL )    CloseHandle(hDev);
  
}


//////////////////////////////////////////////////////////////////////
// open
//
//
// filename
//-----------------------------------------------------------------------------
// Parameters for the driver as a string on the following form
//		
// "serial;bus-speed"
//
// serial
// ======
// Serial number for channel
//
// bus-speed
// =========
// One of the predefined bitrates can be set here
// 
//   125 for  125 Kbs
//   250 for  250 Kbs
//   500 for  500 Kbs
//  1000 for 1000 Mbs
//
// 
// flags 
//-----------------------------------------------------------------------------
//   00000000 00000000 00000000 0000000 
//                                |||||__ SILENT
//                                ||||___ LOOPBACK     
//                                |||____ DAR (1) - DAR disable
//                                ||_____ ERROR (1) - ERROR frames sending enable
//                                |______ Data response enable (TODO:?)
//


bool CUsb2canObj::open( const char *szFileName, unsigned long flags )
{
	const char *pch = NULL;
//	unsigned char cnt,temp,tmp;
	m_emergencyInfo = 0;
	char szDrvParams[256];
	char completeDeviceName[MAX_LENGTH] = "";  //generated from the GUID registered by the driver itself
	char completeDeviceNameTmp[MAX_LENGTH] = "";
	m_initFlag = flags;
	m_RxMsgState = USB_IDLE;
//	BOOL success;

//    unsigned char TempBuff[256];


#ifdef DEBUG_USB2CAN_RECEIVE
	char dbgbuf[256];
#endif


//    int siz;//, nBytes;
//    char buf[256];
//    unsigned char ret;    
//	ULONG nBytesWrite;

	// save parameter string and convert to upper case
strncpy( szDrvParams, szFileName, 256 );
//strupr( szDrvParams );

	// Initiate statistics
	m_stat.cntReceiveData = 0;
	m_stat.cntReceiveFrames = 0;
	m_stat.cntTransmitData = 0;
	m_stat.cntTransmitFrames = 0;

	m_stat.cntBusOff = 0;
	m_stat.cntBusWarnings = 0;
	m_stat.cntOverruns = 0;

	// if open we have noting to do
	if ( m_bRun ) return true;	

#ifdef DEBUG_USB2CAN_RECEIVE
	m_flog = fopen( "c:\\usb2can.txt", "w" );
	//fputs(" bla bla bla ",m_flog);
#endif	


        // serial
	    pch = strtok (szDrvParams,";");

      if( (pch != NULL) && (strlen(szDrvParams) < 256 ) ) 		
	  {
          for(unsigned int y=0,x=0;x <= strlen(pch);x++)
		  {
            if(isalnum(pch[x]))
			{
              m_serial[y++] = pch[x];
			}

		    if( strlen(pch) == x)
			  m_serial[y] = 0;
		  }

		  strupr(m_serial);
		  
          if( (pch = strtok (NULL,";")) == NULL)		 
		    return false;

		  //  jei speed == 0
            m_speed = (uint16_t) strtoul( pch, 0, 0 );		          

			if( m_speed == 0 )
			{
              if( (pch = strtok (NULL,";")) == NULL)		 
		        return false;		 
		 
	            m_tseg1 = (uint8_t) strtoul( pch, 0, 0 );	

               if( (pch = strtok (NULL,";")) == NULL)
		        return false;		 
		 
	            m_tseg2 = (uint8_t) strtoul( pch, 0, 0 );	

               if( (pch = strtok (NULL,";")) == NULL)
		        return false;

			    m_sjw = (uint8_t) strtoul( pch, 0, 0 );

               if( (pch = strtok (NULL,";")) == NULL)
		        return false;

			    m_brp = (USHORT) strtoul( pch, 0, 0 );
			}	

          if( (pch = strtok (NULL,";")) == NULL)
		    goto end;
		 
		    m_hmask = strtoul( pch, 0, 0 );	

          if( (pch = strtok (NULL,";")) == NULL)
		    goto end;
		 
		    m_hfilter = strtoul( pch, 0, 0 );	

          if( (pch = strtok (NULL,";")) != NULL)
		    goto end;
	  }


end:

	// Handle busspeed
	uint8_t nSpeed = CAN_BAUD_125;

	switch ( m_speed ) {

		case 0:
			nSpeed = CAN_BAUD_MANUAL;
			break;

		case 125:
			nSpeed = CAN_BAUD_125;
			break;

		case 250:
			nSpeed = CAN_BAUD_250;
			break;

		case 500:
			nSpeed = CAN_BAUD_500;
			break;

		case 800:
			nSpeed = CAN_BAUD_800;
			break;

		case 1000:
			nSpeed = CAN_BAUD_1000;
			break;

		default:
			nSpeed = CAN_BAUD_125;
			break;

	}

	        memset(completeDeviceName,0,sizeof(completeDeviceName));

			if( !m_enum.GetDevicePath(
				                       (LPGUID) &GUID_CLASS_USB2CAN,			
						                completeDeviceName,
								        sizeof(completeDeviceName),
										m_serial))
			{			 
		     return  FALSE;
			}


/////////////////////////////////////////////////////////////////////////////////


			 //StringCchPrintf(dbgbuf,sizeof(dbgbuf),szDrvParams);
			 //fwrite( dbgbuf, 1, strlen( dbgbuf), m_flog );



    hDev = CreateFile (
                  completeDeviceName,
                  GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  0, // No special attributes
                  NULL); // No template file

    if (INVALID_HANDLE_VALUE == hDev) {

	           hDev = NULL;			
			   return FALSE;
	}

///////////////// DataIn init

    StringCchCopy(completeDeviceNameTmp,sizeof(completeDeviceNameTmp),completeDeviceName);
    StringCchCat (completeDeviceNameTmp, MAX_LENGTH, "\\" );                      
    StringCchCat (completeDeviceNameTmp, MAX_LENGTH, "PIPE00"); // ENDP1

           hDataIn = CreateFile (
                  completeDeviceNameTmp,
                  GENERIC_READ,
                  0,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  0,//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, //0, // No special attributes
                  NULL); // No template file

            if (INVALID_HANDLE_VALUE == hDataIn) {

				hDataIn = NULL;
				return FALSE;
			}

///////////////// DataOut

    StringCchCopy(completeDeviceNameTmp,sizeof(completeDeviceNameTmp),completeDeviceName);
    StringCchCat (completeDeviceNameTmp, MAX_LENGTH, "\\" );                      
    StringCchCat (completeDeviceNameTmp, MAX_LENGTH, "PIPE01"); // ENDP2


           hDataOut = CreateFile (
                  completeDeviceNameTmp,
                  GENERIC_WRITE,
                  0,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  0, // No special attributes
                  NULL); // No template file

            if (INVALID_HANDLE_VALUE == hDataOut) {

				hDataOut = NULL;
				return FALSE;
			}

    StringCchCopy(completeDeviceNameTmp,sizeof(completeDeviceNameTmp),completeDeviceName);
    StringCchCat (completeDeviceNameTmp, MAX_LENGTH, "\\" );                      
    StringCchCat (completeDeviceNameTmp, MAX_LENGTH, "PIPE02"); // ENDP3


           hCmdIn = CreateFile (
                  completeDeviceNameTmp,
                  GENERIC_READ,
                  0,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  0, // No special attributes
                  NULL); // No template file

            if (INVALID_HANDLE_VALUE == hCmdIn) {

				hCmdIn = NULL;
				return FALSE;
			}


    StringCchCopy(completeDeviceNameTmp,sizeof(completeDeviceNameTmp),completeDeviceName);
    StringCchCat (completeDeviceNameTmp, MAX_LENGTH, "\\" );                      
    StringCchCat (completeDeviceNameTmp, MAX_LENGTH, "PIPE03"); // ENDP4


           hCmdOut = CreateFile (
                  completeDeviceNameTmp,
                  GENERIC_WRITE,
                  0,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  0, // No special attributes
                  NULL); // No template file

            if (INVALID_HANDLE_VALUE == hCmdOut) {

				hCmdOut = NULL;
				return FALSE;
			}


#ifdef DEBUG_USB2CAN_RECEIVE
	fclose( m_flog );
#endif

    m_bRun = false;
    close();

	m_bRun = true;   
	
	// Start write thread 
	DWORD threadId;
	if ( NULL == 
			( m_hTreadTransmit = CreateThread(	NULL,
										0,
										(LPTHREAD_START_ROUTINE) workThreadTransmit,
										this,
										0,
										&threadId ) ) ) { 
		// Failure
		close();
		return false;
	}

	// Start read thread 
	if ( NULL == 
			( m_hTreadReceive = CreateThread(	NULL,
										0,
										(LPTHREAD_START_ROUTINE) workThreadReceiveData,
										this,
										0,
										&threadId ) ) ) { 
		// Failure
		close();
		return  false;
	}

	// Release the mutex
	UNLOCK_MUTEX( m_usb2canMutex );
	UNLOCK_MUTEX( m_receiveMutex );
	UNLOCK_MUTEX( m_transmitMutex );
	UNLOCK_MUTEX( m_commandMutex );
	UNLOCK_MUTEX( m_responseDataMutex );


	cmdMsg outmsg;
	cmdMsg inmsg;

    outmsg.channel = 0;
    outmsg.command = USB2CAN_OPEN;
	outmsg.opt1    = nSpeed;
	outmsg.opt2    = 0;
	outmsg.data[0] = m_tseg1;
	outmsg.data[1] = m_tseg2;
	outmsg.data[2] = m_sjw;

	// BRP
	outmsg.data[3] = (UCHAR) (m_brp >> 8);
    outmsg.data[4] = (UCHAR)  m_brp;

	//flags
	outmsg.data[5] = (UCHAR) (m_initFlag >> 24);
    outmsg.data[6] = (UCHAR) (m_initFlag >> 16);
	outmsg.data[7] = (UCHAR) (m_initFlag >> 8);
    outmsg.data[8] = (UCHAR)  m_initFlag;

    if(! sendCommandWait( &outmsg,&inmsg,500 ))
	{
     close();
	 return false;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////
// close
//

bool CUsb2canObj::close( void )
{	

//	ULONG nBytesWrite;


	cmdMsg outmsg;
	cmdMsg inmsg;

	outmsg.channel = 0;
    outmsg.command = USB2CAN_CLOSE;
	outmsg.opt1 = 1;
	outmsg.opt2 = 2;

    sendCommandWait( &outmsg,&inmsg,500 );

/*
    if(! sendCommandWait( &outmsg,&inmsg,500 ))
	{     
	  return false;
	}
*/

	// Do nothing if already terminated
	if ( !m_bRun ) return false;
	
	m_bRun = false;


#ifdef DEBUG_USB2CAN_RECEIVE
	fclose( m_flog );
#endif
 
	
	// terminate the worker thread 
	
	DWORD rv;


	// Wait for transmit thread to terminate
	while ( true ) {
		GetExitCodeThread( m_hTreadTransmit, &rv );
		if ( STILL_ACTIVE != rv ) break;
	}


	// Wait for receive thread to terminate
	while ( true ) {
		GetExitCodeThread( m_hTreadReceive, &rv );
		if ( STILL_ACTIVE != rv ) break;
	}	

/*
	// Wait for receive cmd thread to terminate
	while ( true ) {
		GetExitCodeThread( m_hTreadReceiveCmd, &rv );
		if ( STILL_ACTIVE != rv ) break;
	}	
*/	


	return true;
}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CUsb2canObj::doFilter( canalMsg *pcanalMsg )
{	
	unsigned long msgid = ( pcanalMsg->id & 0x1fffffff);
	if ( !m_mask ) return true;	// fast escape

	// Set bit 32 if extended message
	if ( pcanalMsg->flags | CANAL_IDFLAG_EXTENDED ) {
		msgid &= 0x1fffffff;
		msgid |= 80000000;	
	}
	else {
		// Standard message
		msgid &= 0x000007ff;
	}

	// Set bit 31 if RTR
	if ( pcanalMsg->flags | CANAL_IDFLAG_RTR ) { 
		msgid |= 40000000;	
	}

	return !( ( m_filter ^ msgid ) & m_mask );
}

//////////////////////////////////////////////////////////////////////
// setFilter
//

bool CUsb2canObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CUsb2canObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return true;
}



//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CUsb2canObj::writeMsg( canalMsg *pMsg )
{	
	bool rv = false;
	
	if ( NULL != pMsg ) {

		// Must be room for the message
		if ( m_transmitList.nCount < USB2CAN_MAX_SNDMSG ) {

			dllnode *pNode = new dllnode;
			
			if ( NULL != pNode ) {
			
				canalMsg *pcanalMsg = new canalMsg;

				pNode->pObject = pcanalMsg;
				pNode->pKey = NULL;
				pNode->pstrKey = NULL;

				if ( NULL != pcanalMsg ) {
					memcpy( pcanalMsg, pMsg, sizeof( canalMsg ) );
				}

				LOCK_MUTEX( m_transmitMutex );
				dll_addNode( &m_transmitList, pNode );
				UNLOCK_MUTEX( m_transmitMutex );
 
				rv = true;

			}
			else {

				delete pMsg;

			}
		}
	}
	
	return rv;		
}


//////////////////////////////////////////////////////////////////////
// readMsg
//

bool CUsb2canObj::readMsg( canalMsg *pMsg )
{
	bool rv = false;
	
	if ( ( NULL != m_receiveList.pHead ) && 
			( NULL != m_receiveList.pHead->pObject ) ) {
		
		memcpy( pMsg, m_receiveList.pHead->pObject, sizeof( canalMsg ) );
		LOCK_MUTEX( m_receiveMutex );
		dll_removeNode( &m_receiveList, m_receiveList.pHead );
		UNLOCK_MUTEX( m_receiveMutex );

		rv = true;
	}

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//	dataAvailable
//

int CUsb2canObj::dataAvailable( void )
{
	int cnt;
	
	LOCK_MUTEX( m_receiveMutex );
	cnt = dll_getNodeCount( &m_receiveList );	
	UNLOCK_MUTEX( m_receiveMutex );
 
	return cnt;
}


///////////////////////////////////////////////////////////////////////////////
//	getStatistics
//

bool CUsb2canObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{	
	// Must be a valid pointer
	if ( NULL == pCanalStatistics ) return false;

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

	return true;
}

/* STATUS
==================================

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

/*  ERROR messages
==========================================
Flag  value  Description  
===========================================
CANAL_STATUSMSG_OK        0\D700  Normal condition.  
CANAL_STATUSMSG_OVERRUN   0\D701  Overrun occured when sending data to CAN bus.  
CANAL_STATUSMSG_BUSLIGHT  0\D702  Error counter has reached 96.  
CANAL_STATUSMSG_BUSHEAVY  0\D703  Error counter has reached 128.  
CANAL_STATUSMSG_BUSOFF    0\D704  Device is in BUSOFF. CANAL_STATUSMSG_OK is sent when returning to operational mode.  
CANAL_STATUSMSG_STUFF     0\D720  Stuff Error.  
CANAL_STATUSMSG_FORM      0\D721  Form Error.  
CANAL_STATUSMSG_ACK       0\D723  Ack Error.  
CANAL_STATUSMSG_BIT0      0\D724  Bit1 Error.  
CANAL_STATUSMSG_BIT1      0\D725  Bit0 Error.  
CANAL_STATUSMSG_CRC       0\D726  CRC Error.  
*/

/* STATUS
============================================
CANAL_STATUS_NONE                 0x00000000
CANAL_STATUS_ACTIVE               0x10000000
CANAL_STATUS_PASSIVE              0x40000000
CANAL_STATUS_BUS_OFF              0x80000000
CANAL_STATUS_BUS_WARN             0x40000000
CANAL_STATUS_PHY_FAULT            0x08000000
CANAL_STATUS_PHY_H                0x04000000
CANAL_STATUS_PHY_L                0x02000000
CANAL_STATUS_SLEEPING             0x01000000
CANAL_STATUS_STOPPED              0x00800000
CANAL_STATUS_RECEIVE_FIFO_FULL    0x00400000
CANAL_STATUS_TRANSMIT_FIFO_FULL   0x00200000
Bits from 16-31 are reserved, bits from 0-15 are user defined and can be defined by the driver maker. 
*/

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CUsb2canObj::getStatus( PCANALSTATUS pCanalStatus )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatus ) return false;

	m_status.channel_status = ( m_emergencyInfo << 16 );

	if ( m_emergencyInfo & EMERGENCY_BUS_OFF ) {
		m_status.channel_status  |= 0x80000000;	
	}

	if ( ( m_emergencyInfo & EMERGENCY_TX_WARNING ) ||
			( m_emergencyInfo & EMERGENCY_RCV_WARNING ) ) {
		m_status.channel_status  |= 0x40000000;	
	}

	if ( ( m_emergencyInfo & EMERGENCY_TXBUS_PASSIVE ) ||
			( m_emergencyInfo & EMERGENCY_RXBUS_PASSIVE ) ) {
		m_status.channel_status  |= 0x20000000;	
	}

	// zero emergency info
	m_emergencyInfo = 0;

	// TX Error counter
	m_status.channel_status += getTxErrorCount();

	// RX Error counter
	m_status.channel_status += ( getRxErrorCount() << 8 );

	memcpy( pCanalStatus, &m_status, sizeof( canalStatus ) );

	return true;
}



/*
    COMMAND send format
	=========================================
	UCHAR  |  BeginFrame  |  1  |  0x11
	UCHAR  |  Channel     |  1  |
    UCHAR  |  Command	  |  1  |
	UCHAR  |  DATA        |  n  | <= 50
	UCHAR  |  EndFrame    |  1  |  0x22

    COMMAND response format
	=========================================
	UCHAR  |  BeginFrame  |  1  |  0x33
	UCHAR  |  Channel     |  1  |
	UCHAR  |  Command     |  1  | 
	UCHAR  |  DATA        |  n  | <= 50
    UCHAR  |  EndFrame    |  1  |  0x44
    

    COMMAND list
	====================================================
                              | CMD  | NoDat  |  DATA  |
    ====================================================
	USB2CAN_RESET             | 0x01 |  -     |
	USB2CAN_OPEN              | 0x02 |  1     | enum{standard=0,autoconnect=1,passive=2,loopback=3}
	USB2CAN_CLOSE             | 0x03 |  -     | 
	USB2CAN_SET_SPEED         | 0x04 |  3     | speed,reg1,reg2
	USB2CAN_SET_MASK_FILTER   | 0x05 |  9     | UCHAR pozition,ULONG mask,ULONG filetr
	USB2CAN_GET_STATUS        | 0x06 |  -
	USB2CAN_GET_STATISTICS    | 0x07 |  1     | enum{0..6}
	USB2CAN_GET_SERIAL        | 0x08 |  -
	USB2CAN_GET_SOFTWARE_VER  | 0x09 |  -
	USB2CAN_GET_HARDWARE_VER  | 0x0A |  -
	USB2CAN_ENABLE_TIMESTAMPS | 0x0B |  -
	USB2CAN_DISABLE_TIMESTAMPS| 0x0C |  -    

*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// CMD //GS
//
//
bool CUsb2canObj::Cmd(LONG OutCnt , UCHAR *DataOut, // gaunam 
						 LONG *InCnt , UCHAR *DataIn)  // grazinam atgal
{
	//uint8_t  status = FALSE;
    cmdMsg	 cmdOutMsg;
    cmdMsg	 cmdInMsg;
	uint8_t	 size = 0;	
//	UCHAR    x;

//  ULONG    nBytesRead;
//	ULONG    nBytesWrite;
//  uint8_t  RxDataBuf[256];


    if( OutCnt > 29 ) 
    return false;


	size=0;
    cmdOutMsg.channel = DataOut[size++];
	cmdOutMsg.command = DataOut[size++];
	cmdOutMsg.opt1    = DataOut[size++];    
    cmdOutMsg.opt2    = DataOut[size++]; 

	memcpy(&cmdOutMsg.data,&DataOut[size],24);

    if(!sendCommandWait(&cmdOutMsg,&cmdInMsg,500))	
    return false;
	
	size=0;	  
    DataIn[size++] = cmdInMsg.channel;
    DataIn[size++] = cmdInMsg.command;
    DataIn[size++] = cmdInMsg.opt1;
    DataIn[size++] = cmdInMsg.opt2;


    memcpy(&DataIn[size],&cmdInMsg.data,24);

	*InCnt = 4;


return true;	
}


///////////////////////////////////////////////////////////////////////////////
// sendCommandWait( )
//
//

bool CUsb2canObj::sendCommandWait( cmdMsg *cmdOutMsg,cmdMsg *cmdInMsg, uint32_t timeout )
{

	ULONG		nBytesWrite;
	ULONG		nBytesRead;

	uint8_t		buff[100];
	uint8_t		size = 0;
	cmdMsg		cmdmsg;


    cmdmsg = *cmdOutMsg;

    buff[size++] = 0x11;
	buff[size++] = cmdmsg.channel;
    buff[size++] = cmdmsg.command;
	buff[size++] = cmdmsg.opt1;
    buff[size++] = cmdmsg.opt2;

	memcpy(&buff[size],&cmdmsg.data,28);
	size += 28;

    buff[size++] = 0x22;

	if(!WriteFile(hCmdOut,buff,size,(PULONG) &nBytesWrite,0))
	{
    return false;
	}

    if(!ReadFile(hCmdIn, buff, sizeof(buff), (PULONG) &nBytesRead, NULL))
    {
      return false;
	}

    if( (buff[0] != 0x11) || (buff[33] != 0x22) || (nBytesRead != 34) )
	{
	  return false;
	}

	size = 0;
    size++;     // praleidziam usb msg pradzia (0x11)

	cmdmsg.channel = buff[size++];
	cmdmsg.command = buff[size++];
	cmdmsg.opt1    = buff[size++];
	cmdmsg.opt2    = buff[size++];

	memcpy(&cmdmsg.data,&buff[size],28);    

    *cmdInMsg = cmdmsg;

    return	true;
}


///////////////////////////////////////////////////////////////////////////////
// setBaudrate
//
//

bool CUsb2canObj::setBaudrate( uint8_t rate )
{
//	uint8_t command;
/*
	switch (rate)
	{
		case 0:
			command = SET_BAUD_1MEG;		
			break;

		case 1:
			command = SET_BAUD_500K;		
			break;

		case 2:
			command = SET_BAUD_250K;		
			break;

		case 3:
			command = SET_BAUD_125K;		
			break;
	}

	responseMsg msg;
	if ( !sendCommandWait( command, &msg, 1000 ) ) return false;

	if ( msg.command != ( command | 0x80 ) ) return false;
*/


	return true;
}


///////////////////////////////////////////////////////////////////////////////
// resetAdapter
//
//

bool CUsb2canObj::resetAdapter( void )
{
/*
	if ( USB_OK == sendCommand( RESET_MICRO ) ) {
		return true;
	}
	else {
		return false;
	}
*/
	return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// getTxErrorCount
//
//

short CUsb2canObj::getTxErrorCount( void )
{
/*
	responseMsg msg;
	if ( USB_OK != sendCommandWait( GET_TX_ERR_CNT, &msg, 1000 ) ) {
		return -1;
	}

	return msg.data[ 0 ];
*/
 return 0;
}


///////////////////////////////////////////////////////////////////////////////
// getRxErrorCount
//
//

short CUsb2canObj::getRxErrorCount( void )
{
/*
	responseMsg msg;
	if ( USB_OK != sendCommandWait( GET_RX_ERR_CNT, &msg, 1000 ) ) {
		return -1;
	}

	return msg.data[ 0 ];
*/
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// getAdapterMode
//
//

short CUsb2canObj::getAdapterCANSTAT( void )
{
/*
	responseMsg msg;
	if ( USB_OK != sendCommandWait( GET_CANSTAT, &msg, 1000 ) ) {
		return -1;
	}

	return msg.data[ 0 ];
*/
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// getAdapterStatus
//
//

short CUsb2canObj::getAdapterCOMSTAT( void )
{
/*
	responseMsg msg;
	if ( USB_OK != sendCommandWait( GET_COMSTAT, &msg, 1000 ) ) {
		return -1;
	}

	return msg.data[ 0 ];
*/
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// abortTransmission
//
//

bool CUsb2canObj::abortTransmission( void )
{
/*
	if ( USB_OK != sendCommand( ABORT_ALL_TX ) ) {
		return false;
	}
*/
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// setAdapterFilterMask
//
//

bool CUsb2canObj::setAdapterFilterMask( structFilterMask *pFilterMask )
{
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// workThreadTransmit
//
//

#ifdef WIN32
void workThreadTransmit( void *pObject )
#else
void *workThreadTransmit( void *pObject )
#endif
{
#ifdef WIN32
	DWORD errorCode = 0;
	ULONG	nBytesWrite;
    canalMsg *pRespMsg;
#else
	int rv = 0;
#endif


	CUsb2canObj * pobj = ( CUsb2canObj *)pObject;

	if ( NULL == pobj ) {

#ifdef WIN32	
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}
	
	while ( pobj->m_bRun )
	{		

begin:		// Noting to do if we should end...
		if ( !pobj->m_bRun ) continue;


		// Is there something to transmit
		 if ( ( NULL != pobj->m_transmitList.pHead ) && ( NULL != pobj->m_transmitList.pHead->pObject ) )
		 {
			 
          	uint8_t sendData[ 100 ];
	        short size = 0;

			canalMsg msg;
			memcpy( &msg, pobj->m_transmitList.pHead->pObject, sizeof( canalMsg ) ); 


			LOCK_MUTEX( pobj->m_transmitMutex );
			dll_removeNode( &pobj->m_transmitList, pobj->m_transmitList.pHead );
			UNLOCK_MUTEX( pobj->m_transmitMutex );						
          

			sendData[ size++ ] = 0x55; // FrameBegin
            sendData[ size++ ] = 0;    // Channel

            sendData[ size++ ] = ( uint8_t ) msg.flags & 0x03; // RTR and EXT_ID flags only

            sendData[ size++ ] = DATA_FRAME;    // Data frame type			
			sendData[ size++ ] = ( uint8_t )( msg.id >> 24 ) & 0x1f;
			sendData[ size++ ] = ( uint8_t )( msg.id >> 16 ) & 0xff;
			sendData[ size++ ] = ( uint8_t )( msg.id >> 8 )  & 0xff;
			sendData[ size++ ] = ( uint8_t )( msg.id ) & 0xff;       // Message ID
			
			memcpy( sendData + 8, msg.data, 8 ); // DATA
			size += 8;                        

            sendData[ size++ ] = msg.sizeData; // NoOfData
            sendData[ size++ ] = 0;    // SequenceNr
            sendData[ size++ ] = 0;    // CRC
            sendData[ size++ ] = 0xAA; // FrameEnd

			//LOCK_MUTEX( pobj->m_usb2canMutex );

			if( WriteFile(pobj->hDataOut,sendData,size,&nBytesWrite,0))
			{

	
			// Message sent successfully
			// Update statistics
			 pobj->m_stat.cntTransmitData += msg.sizeData;
   			 pobj->m_stat.cntTransmitFrames += 1;


			 /////////// response test              

			   LOCK_MUTEX( pobj->m_responseDataMutex );
               dll_removeAllNodes( &pobj->m_responseDataList );
               UNLOCK_MUTEX( pobj->m_responseDataMutex );

			   uint32_t start = GetTickCount();

	           while (  GetTickCount() < ( start + 2000 ) )
			   {
	             //if ( (NULL != pobj->m_responseDataList.pHead ) && (NULL != pobj->m_responseDataList.pHead->pObject ) )
			     if ( dll_getNodeCount(&pobj->m_responseDataList) > 0 )
				 {
			       //memcpy( pRespMsg, pobj->m_responseDataList.pHead->pObject, sizeof( canalMsg ) ); 

			       LOCK_MUTEX( pobj->m_responseDataMutex );
			       //dll_removeNode( &pobj->m_responseDataList, pobj->m_responseDataList.pHead );
				   dll_removeAllNodes( &pobj->m_responseDataList );
			       UNLOCK_MUTEX( pobj->m_responseDataMutex );
                   goto begin;
				   //continue;
				 }
		
			   }
			}



				// Failed - put message back in queue front
			/*	PCANALMSG pMsg	= new canalMsg;

				if ( NULL != pMsg )
				{
						
					// Copy in data
					memcpy ( pMsg, &msg, sizeof( canalMsg ) );										

					dllnode *pNode = new dllnode; 

					if ( NULL != pNode )
					{																
					  pNode->pObject = pMsg;

					  LOCK_MUTEX( pobj->m_transmitMutex );
					  dll_addNodeHead( &pobj->m_transmitList, pNode );
					  UNLOCK_MUTEX( pobj->m_transmitMutex );
					}
					else
					{
					 delete pMsg;
					}
				}*/

										
		} // if data

		// No data to write
		SLEEP( 1 );

	} // while 	 


#ifdef WIN32
	ExitThread( errorCode );
#else
	pthread_exit( &rv );
#endif

}


///////////////////////////////////////////////////////////////////////////////
// workThreadReceive
//
//

#ifdef WIN32
void workThreadReceiveData( void *pObject )
#else
void *workThreadReceiveData( void *pObject )
#endif
{
	//bool bData;
	BOOL	success;
	UCHAR	RxDataBuf[100];
	ULONG	nBytesRead;
//	ULONG   nBytesWrite;
	UCHAR	nChannel;
	UCHAR	nFlags;
	UCHAR	nOperation;
	UCHAR   nSequenceNr;
	UCHAR	nCRC;


#ifdef WIN32
	DWORD errorCode = 0;
#else
	int rv = 0;
#endif

	CUsb2canObj * pobj = ( CUsb2canObj *)pObject;
	if ( NULL == pobj ) {
#ifdef WIN32	
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}
	
	while ( pobj->m_bRun ) {
		
		// Noting to do if we should end...
		if ( !pobj->m_bRun ) continue;


		//LOCK_MUTEX( pobj->m_usb2canMutex );

		//bData = pobj->readUSBData();

	 success = ReadFile(pobj->hDataIn, RxDataBuf, sizeof(RxDataBuf), (PULONG) &nBytesRead, NULL);

	 if( success == TRUE )
	 {		
		 //if ( pobj->m_receiveList.nCount < USB2CAN_MAX_RCVMSG )
		 //{
            

			PCANALMSG pMsg	= new canalMsg;
			pMsg->flags = 0;

     		if ( NULL != pMsg )
			{

			 dllnode *pNode = new dllnode; 

				if ( NULL != pNode )
				{
                   pMsg->flags = 0;// GS

                   nChannel    = RxDataBuf[1];
				   nFlags      = RxDataBuf[2];
				   nOperation  = RxDataBuf[3];
				   nSequenceNr = RxDataBuf[21];
                   nCRC        = RxDataBuf[22];

					// If extended set extended flag
					if ( nFlags & 0x01 ) pMsg->flags |= CANAL_IDFLAG_EXTENDED;

					// Check for RTR package
					if ( nFlags & 0x02 ) pMsg->flags |= CANAL_IDFLAG_EXTENDED;

					pMsg->id = 
					(((DWORD)RxDataBuf[4]<<24) & 0x1f000000) |
					(((DWORD)RxDataBuf[5]<<16) & 0x00ff0000) |
					(((DWORD)RxDataBuf[6]<<8 ) & 0x0000ff00) |
					(((DWORD)RxDataBuf[7]    ) & 0x000000ff) ;                	
				

					pMsg->sizeData = RxDataBuf[16];	
					memcpy( (void *)pMsg->data, (RxDataBuf + 8 ), pMsg->sizeData );                 

			 		pMsg->timestamp = 
					(((DWORD)RxDataBuf[17]<<24 ) & 0xff000000) |
					(((DWORD)RxDataBuf[18]<<16 ) & 0x00ff0000) |
					(((DWORD)RxDataBuf[19]<<8  ) & 0x0000ff00) |
					(((DWORD)RxDataBuf[20]     ) & 0x000000ff) ;


					if(nOperation == DATA_FRAME ) // data
					{
					  //if ( pobj->m_receiveList.nCount < USB2CAN_MAX_RCVMSG )
					  {
						pNode->pObject = pMsg;

						LOCK_MUTEX( pobj->m_receiveMutex );
						dll_addNode( &pobj->m_receiveList, pNode );
						UNLOCK_MUTEX( pobj->m_receiveMutex );

						// Update statistics
						//pobj->m_stat.cntReceiveData += pMsg->sizeData;
						//pobj->m_stat.cntReceiveFrames += 1;

					  }

					}
					else if(DATA_FRAME_ACK) //data response
					{
			    	  //if ( pobj->m_responseDataList.nCount < USB2CAN_MAX_RESPONSEMSG )
					  {
					    pNode->pObject = pMsg;

						LOCK_MUTEX( pobj->m_responseDataMutex );
						dll_addNode( &pobj->m_responseDataList, pNode );
						UNLOCK_MUTEX( pobj->m_responseDataMutex );

						// Update statistics
						//pobj->m_stat.cntReceiveData += pMsg->sizeData;
						//pobj->m_stat.cntReceiveFrames += 1;

					  }
					}
					else 
					{
	   				  delete pMsg;						
                      delete pNode;
					}
								
				}//if ( NULL != pNode )
				 else
				   delete pMsg;

			}//if ( NULL != pMsg )

	 }//if( success == TRUE )


	//UNLOCK_MUTEX( pobj->m_usb2canMutex );

	
     SLEEP( 1 );

	} // while 	 


#ifdef WIN32
	ExitThread( errorCode );
#else
	pthread_exit( &rv );
#endif

}


/*
            DATA send frame format
            ================================
			UCHAR  |  FrameBegin  | 1 | 0x55
			UCHAR  |  Channel	  |	1 |
			UCHAR  |  Flags       | 1 | STD,EXT,RTR
			UCHAR  |  Operation   | 1 | 
			ULONG  |  ID		  |	4 |
			ULONG  |  DATA		  | 8 |
			UCHAR  |  NoOfData	  | 1 |
			UCHAR  |  SequenceNr  | 1 |
			UCHAR  |  CRC		  |	1 | XOR
			UCHAR  |  FrameEnd    | 1 | 0xAA

            DATA receive frame format
            ================================
			UCHAR  |  FrameBegin  | 1 | 0x55
			UCHAR  |  Channel	  |	1 |
			UCHAR  |  Flags       | 1 | STD,EXT,RTR
			UCHAR  |  Operation   | 1 | 
			ULONG  |  ID		  |	4 |
			ULONG  |  DATA		  | 8 |
			UCHAR  |  NoOfData	  | 1 |
			ULONG  |  Timestamp   | 4 |
			UCHAR  |  SequenceNr  | 1 |
			UCHAR  |  CRC		  |	1 | XOR
			UCHAR  |  FrameEnd    | 1 | 0xAA

            DATA response frame format
			================================
			UCHAR  |  FrameBegin  | 1 | 0x55
			UCHAR  |  Channel	  |	1 |
			UCHAR  |  Flags       | 1 | STD,EXT,RTR
			UCHAR  |  Operation   | 1 | 0x01 or 0x02
			UCHAR  |  SequenceNr  | 1 | 
			UCHAR  |  CRC         | 1 |
			UCHAR  |  FrameEnd    | 1 | 0xAA
            
            Flags
			===============================
            USBCAN_IDFLAG_STANDARD  | 0x00
            USBCAN_IDFLAG_EXTENDED  | 0x01
			USBCAN_IDFLAG_RTR       | 0x02

            Operation code list
	        ====================================
            DATA_FRAME                   | 0x00
			DATA_FRAME_ACK               | 0x01
		    DATA_FRAME_NACK              | 0x02
			------ ERROR frames -----
			EMERGENCY_OVERFLOW		     | 0x80
	        EMERGENCY_RCV_WARNING	     | 0x81
	        EMERGENCY_TX_WARNING	   	 | 0x82
	        EMERGENCY_TXBUS_PASSIVE	     | 0x83
	        EMERGENCY_RXBUS_PASSIVE	     | 0x84
	        EMERGENCY_BUS_OFF		     | 0x85

*/   
