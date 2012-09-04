// usb2canobj.cpp:  
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
	m_bOpen = false;

	hDataOut = NULL; 
	hDataIn = NULL;
	hCmdOut = NULL; 
	hCmdIn = NULL;
	hDev = NULL;

    m_receiveDataEvent = NULL;

    m_transmitDataPutEvent = NULL;
	m_transmitDataGetEvent = NULL;
	
	m_hTreadReceive = NULL;
	m_hTreadTransmit = NULL;

	// Create the device AND LIST access mutexes

    // nenaudojamas
	m_usb2canMutex = CreateMutex( NULL, true, CANAL_DLL_USB2CANDRV_OBJ_MUTEX );

    // receive list mutex
	m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_USB2CANDRV_RECEIVE_MUTEX );

	// transmit list mutex
	m_transmitMutex = CreateMutex( NULL, true, CANAL_DLL_USB2CANDRV_TRANSMIT_MUTEX );

    // komandu mutex
	m_commandMutex = CreateMutex( NULL, true, CANAL_DLL_USB2CANDRV_COMMAND_MUTEX );

	// receive Event
	m_receiveDataEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// receive Event
	m_transmitDataPutEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_transmitDataGetEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	dll_init( &m_transmitList, SORT_NONE );
	dll_init( &m_receiveList, SORT_NONE );
}

//////////////////////////////////////////////////////////////////////
// ~CUsb2canObj
//

CUsb2canObj::~CUsb2canObj()
{		 
	//close();

	/*
	LOCK_MUTEX( m_transmitMutex );
	dll_removeAllNodes( &m_transmitList );
	
	LOCK_MUTEX( m_receiveMutex );
	dll_removeAllNodes( &m_receiveList );

*/

	// mutex
	if ( NULL != m_usb2canMutex ) CloseHandle( m_usb2canMutex );	
	if ( NULL != m_receiveMutex ) CloseHandle( m_receiveMutex );
	if ( NULL != m_transmitMutex ) CloseHandle( m_transmitMutex );
	if ( NULL != m_commandMutex ) CloseHandle( m_commandMutex );

	// events
    if ( NULL != m_receiveDataEvent ) CloseHandle( m_receiveDataEvent );
	if ( NULL != m_transmitDataPutEvent ) CloseHandle( m_transmitDataPutEvent );
	if ( NULL != m_transmitDataGetEvent ) CloseHandle( m_transmitDataGetEvent );

	// files
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
//                              |||||||__ SILENT
//                              ||||||___ LOOPBACK     
//                              |||||____ DAR (1) - DAR disable
//                              ||||_____ STATUS_FRAMES (1) - STTAUS frames sending enable
//                              |||______ 
//                              ||_______ 
//                              |________ 

bool CUsb2canObj::open( const char *szFileName, unsigned long flags )
{
	const char *pch = NULL;
	m_emergencyInfo = 0;
	char szDrvParams[256];
	char completeDeviceName[MAX_LENGTH] = "";  //generated from the GUID registered by the driver itself
	char completeDeviceNameTmp[MAX_LENGTH] = "";
	m_initFlag = flags;

	// Start write thread 
	DWORD threadId;

#ifdef DEBUG_USB2CAN_RECEIVE
	char dbgbuf[256];
#endif

	strncpy_s( szDrvParams, szFileName, 256 );
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
	//if ( m_bRun ) return true;

    if ( m_bOpen == true )
	  return false;

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

		  if( strlen (m_serial) != 8 )
            return false;
		  
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

    //  nSpeed 
	// =====================
	// CAN_BAUD_1000   - 0
    // CAN_BAUD_800    - 1
	// CAN_BAUD_500    - 2 
	// CAN_BAUD_250    - 3 
	// CAN_BAUD_125    - 4
	// CAN_BAUD_100    - 5
	// CAN_BAUD_50     - 6
	// CAN_BAUD_20     - 7
	// CAN_BAUD_10     - 8
    // CAN_BAUD_MANUAL - 9  

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


    hDev = CreateFile (
                  completeDeviceName,
                  GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  FILE_ATTRIBUTE_NORMAL,// | FILE_FLAG_OVERLAPPED,  //0, // No special attributes
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
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  FILE_ATTRIBUTE_NORMAL,// | FILE_FLAG_OVERLAPPED,//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, //0, // No special attributes
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
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  FILE_ATTRIBUTE_NORMAL,// | FILE_FLAG_OVERLAPPED, // No special attributes
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
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  FILE_ATTRIBUTE_NORMAL,// | FILE_FLAG_OVERLAPPED, // No special attributes
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
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  FILE_ATTRIBUTE_NORMAL,// | FILE_FLAG_OVERLAPPED, // No special attributes
                  NULL); // No template file

            if (INVALID_HANDLE_VALUE == hCmdOut) {

				hCmdOut = NULL;
				return FALSE;
			}


#ifdef DEBUG_USB2CAN_RECEIVE
//	close( m_flog );
#endif

    // leidziam thread'am suktis
	m_bRun = true;   

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
										&threadId ) ) )
	{ 
		// Failure
		close();
		return  false;
	}

	SetThreadPriority( m_hTreadReceive,THREAD_PRIORITY_TIME_CRITICAL );
	//SetThreadPriority( m_hTreadTransmit,THREAD_PRIORITY_TIME_CRITICAL );

	// Release the mutex
	UNLOCK_MUTEX( m_usb2canMutex );
	UNLOCK_MUTEX( m_receiveMutex );
	UNLOCK_MUTEX( m_transmitMutex );
	UNLOCK_MUTEX( m_commandMutex );

	// set USB2CAN baudrate
    if( USB2CAN_open( nSpeed,m_tseg1,m_tseg2,m_sjw,m_brp,m_initFlag ) == FALSE )
	  close();

/////////////////////////////////////////////////////////////

    m_bOpen = true;

	return true;
}

//////////////////////////////////////////////////////////////////////
// close
//

bool CUsb2canObj::close( void )
{	
    DWORD rv;

    USB2CAN_close();
 	
	SetEvent( m_receiveDataEvent );
	SetEvent( m_transmitDataPutEvent );
	SetEvent( m_transmitDataGetEvent );

	// terminate the worker thread 	

	m_bRun = false;    

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

	LOCK_MUTEX( m_transmitMutex );
	dll_removeAllNodes( &m_transmitList );
    UNLOCK_MUTEX( m_transmitMutex );
	
	LOCK_MUTEX( m_receiveMutex );
	dll_removeAllNodes( &m_receiveList );
	UNLOCK_MUTEX( m_receiveMutex );

	if( hDataOut != NULL) CloseHandle(hDataOut); 
    hDataOut = NULL; 
	if( hDataIn != NULL ) CloseHandle(hDataIn);
    hDataIn = NULL;
	if( hCmdOut != NULL ) CloseHandle(hCmdOut);
    hCmdOut = NULL; 
 	if( hCmdIn != NULL )  CloseHandle(hCmdIn);
    hCmdIn = NULL;
	if( hDev != NULL )    CloseHandle(hDev); 
    hDev = NULL;	

    m_bOpen = false;

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


char *CUsb2canObj::getVendorString(void)
	{
	 DWORD res;
	 const char prod[] = "www.edevices.lt\0";

      if (!USB2CAN_get_fver_hver(&res))
		  return NULL;

	 sprintf_s(m_USB2CAN_fver_hver,sizeof(m_USB2CAN_fver_hver),"%d.%d;%d.%d", (UCHAR)(res>>24),(UCHAR)(res>>16),
	    	                                    (UCHAR)(res>>8), (UCHAR)res );
   

      return m_USB2CAN_fver_hver;
	}

///////////////////////////////////////////////////////////////////////////////
// USB2CAN open
//
//

bool CUsb2canObj::USB2CAN_open( UCHAR speed,UCHAR tseg1,UCHAR tseg2,UCHAR sjw,USHORT brp, ULONG flags )
{
	cmdMsg outmsg;
	cmdMsg inmsg;

    outmsg.channel = 0;
    outmsg.command = USB2CAN_OPEN;
	outmsg.opt1    = speed;
	outmsg.opt2    = 0;
	outmsg.data[0] = tseg1;
	outmsg.data[1] = tseg2;
	outmsg.data[2] = sjw;

	// BRP
	outmsg.data[3] = (UCHAR) (brp >> 8);
    outmsg.data[4] = (UCHAR)  brp;

	//flags
	outmsg.data[5] = (UCHAR) (flags >> 24);
    outmsg.data[6] = (UCHAR) (flags >> 16);
	outmsg.data[7] = (UCHAR) (flags >> 8);
    outmsg.data[8] = (UCHAR)  flags;

    if(! sendCommandWait( &outmsg,&inmsg,500 ))		
	  return  false;	

	// opt1 from hardware : "0" - OK, "255" - ERROR
	if( inmsg.opt1 != 0 )
	  return  false;
	
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//    USB2CAN close
//

bool  CUsb2canObj::USB2CAN_close(void)
{
	cmdMsg outmsg;
	cmdMsg inmsg;

	outmsg.channel = 0;
    outmsg.command = USB2CAN_CLOSE;
	outmsg.opt1 = 0;
	outmsg.opt2 = 0;

   if(! sendCommandWait( &outmsg,&inmsg,500 ))		
	  return  false;	

   return true;
}

///////////////////////////////////////////////////////////////////////////////
//   USB2CAN reset
//

bool  CUsb2canObj::USB2CAN_reset(void)
{
	cmdMsg outmsg;
	cmdMsg inmsg;

	outmsg.channel = 0;
    outmsg.command = USB2CAN_RESET;
	outmsg.opt1 = 0;
	outmsg.opt2 = 0;

   if(! sendCommandWait( &outmsg,&inmsg,500 ))		
	  return  false;	

   return true;
}

///////////////////////////////////////////////////////////////////////////////
//   USB2CAN status
//

bool  CUsb2canObj::USB2CAN_status(canalStatus *pCanalStatus)
{
	cmdMsg outmsg;
	cmdMsg inmsg;
    canalStatus CanalStatus;

	outmsg.channel = 0;
    outmsg.command = USB2CAN_GET_STATUS;
	outmsg.opt1 = 0;
	outmsg.opt2 = 0;

   if(! sendCommandWait( &outmsg,&inmsg,500 ))		
	  return  false;	

	// opt1 from hardware : "0" - OK, "255" - ERROR
	if( inmsg.opt1 != 0 )
	  return  false;

   CanalStatus.channel_status = 
		(((DWORD)inmsg.data[0]<<24) & 0xff000000) |
		(((DWORD)inmsg.data[1]<<16) & 0x00ff0000) |
		(((DWORD)inmsg.data[2]<<8 ) & 0x0000ff00) |
		(((DWORD)inmsg.data[3]    ) & 0x000000ff) ;   

   *pCanalStatus = CanalStatus;

   return true;      
}


///////////////////////////////////////////////////////////////////////////////
//   USB2CAN status
//

bool  CUsb2canObj::USB2CAN_statistics( UCHAR num, DWORD *ret )
{
	cmdMsg outmsg;
	cmdMsg inmsg; 
	DWORD  res;

	outmsg.channel = 0;
    outmsg.command = USB2CAN_GET_STATISTICS;
	outmsg.opt1 = num;
	outmsg.opt2 = 0;

   if(! sendCommandWait( &outmsg,&inmsg,500 ))		
	  return  false;	

	// opt1 from hardware : "0" - OK, "255" - ERROR
	if( inmsg.opt1 != 0 )
	  return  false;

        res = 
		(((DWORD)inmsg.data[0]<<24) & 0xff000000) |
		(((DWORD)inmsg.data[1]<<16) & 0x00ff0000) |
		(((DWORD)inmsg.data[2]<<8 ) & 0x0000ff00) |
		(((DWORD)inmsg.data[3]    ) & 0x000000ff) ;   

   *ret = res;

   return true;      
}


bool CUsb2canObj::USB2CAN_get_fver_hver(DWORD *res)
	{
	cmdMsg outmsg;
	cmdMsg inmsg; 
	//DWORD  res;

	outmsg.channel = 0;
    outmsg.command = USB2CAN_GET_SOFTW_HARDW_VER;
	outmsg.opt1 = 0;
	outmsg.opt2 = 0;

   if(! sendCommandWait( &outmsg,&inmsg,500 ))		
	  return  false;	

	// opt1 from hardware : "0" - OK, "255" - ERROR
	if( inmsg.opt1 != 0 )
	  return  false;

    *res = 
		(((DWORD)inmsg.data[0]<<24) & 0xff000000) |
		(((DWORD)inmsg.data[1]<<16) & 0x00ff0000) |
		(((DWORD)inmsg.data[2]<<8 ) & 0x0000ff00) |
		(((DWORD)inmsg.data[3]    ) & 0x000000ff) ;  

	 return true;
	}

//////////////////////////////////////////////////////////////////////
// writeMsg
//

int CUsb2canObj::writeMsg( canalMsg *pMsg )
{
	// Must be a message pointer
	if ( NULL == pMsg)
		return CANAL_ERROR_PARAMETER;	

	// Must be open
    if ( !m_bOpen )
		return CANAL_ERROR_NOT_OPEN;


    if( m_transmitList.nCount > USB2CAN_MAX_FIFO )
       return CANAL_ERROR_FIFO_FULL;

			dllnode *pNode = new dllnode;			
			if ( NULL == pNode )
              return CANAL_ERROR_MEMORY;
						
			canalMsg *pcanalMsg = new canalMsg;
            if ( NULL == pcanalMsg )			
			{		 
			  delete pNode;            			
			  return CANAL_ERROR_MEMORY;
			}

  		    pNode->pObject = pcanalMsg;
		    pNode->pKey = NULL;
		    pNode->pstrKey = NULL;

    	    memcpy( pcanalMsg, pMsg, sizeof( canalMsg ));

            LOCK_MUTEX( m_transmitMutex );
		    dll_addNode( &m_transmitList, pNode );	
		    UNLOCK_MUTEX( m_transmitMutex );
			
			SetEvent( m_transmitDataGetEvent );
     		
 return CANAL_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// writeMsg blocking
//

int CUsb2canObj::writeMsgBlocking( canalMsg *pMsg, ULONG Timeout )
{
	DWORD res;

	// Must be a message pointer
	if ( NULL == pMsg) return CANAL_ERROR_PARAMETER;	

	// Must be open
    if ( !m_bOpen ) return CANAL_ERROR_NOT_OPEN;

        //if(dll_getNodeCount( &m_transmitList) > USB2CAN_MAX_FIFO )
	    if( m_transmitList.nCount >= USB2CAN_MAX_FIFO )
		{
		  ResetEvent( m_transmitDataPutEvent );

		  res = WaitForSingleObject( m_transmitDataPutEvent, Timeout );

   		  if( res == WAIT_TIMEOUT )
			  return CANAL_ERROR_TIMEOUT;			
		  else if( res == WAIT_ABANDONED )
			  return CANAL_ERROR_GENERIC;
		}
		   ///// ???
           if( m_transmitList.nCount >= USB2CAN_MAX_FIFO )
			   return CANAL_ERROR_FIFO_FULL;

			dllnode *pNode = new dllnode;			
			if ( NULL == pNode )
              return CANAL_ERROR_MEMORY;
						
			canalMsg *pcanalMsg = new canalMsg;
            if ( NULL == pcanalMsg )			
			{		 
			  delete pNode;            			
			  return CANAL_ERROR_MEMORY;
			}

  		    pNode->pObject = pcanalMsg;
		    pNode->pKey = NULL;
		    pNode->pstrKey = NULL;

    	    memcpy( pcanalMsg, pMsg, sizeof( canalMsg ));

            LOCK_MUTEX( m_transmitMutex );
		    dll_addNode( &m_transmitList, pNode );	
		    UNLOCK_MUTEX( m_transmitMutex );
			
			SetEvent( m_transmitDataGetEvent );
  							
	return CANAL_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// readMsg
// Non blocking

int CUsb2canObj::readMsg( canalMsg *pMsg )
{
  int rv = CANAL_ERROR_SUCCESS;

  // Must be a message pointer
  if ( NULL == pMsg)
	  return  CANAL_ERROR_PARAMETER;	

  // Must be open
  if ( !m_bOpen )
	  return  CANAL_ERROR_NOT_OPEN;

  if( m_receiveList.nCount == 0)
	  return  CANAL_ERROR_FIFO_EMPTY;

       memcpy( pMsg, m_receiveList.pHead->pObject, sizeof( canalMsg ) );

	   LOCK_MUTEX( m_receiveMutex );
	   dll_removeNode( &m_receiveList, m_receiveList.pHead );
       UNLOCK_MUTEX( m_receiveMutex );

	   ResetEvent( m_receiveDataEvent);		

       rv = CANAL_ERROR_SUCCESS;	

 return rv;
}

//////////////////////////////////////////////////////////////////////
// readMsg
// Blocking

int CUsb2canObj::readMsgBlocking( canalMsg *pMsg, ULONG	 Timeout )
{
	int rv = CANAL_ERROR_SUCCESS;
	DWORD res;

	// Must be a message pointer
	if ( NULL == pMsg)
		return CANAL_ERROR_PARAMETER;	

	// Must be open
    if ( !m_bOpen )
		return CANAL_ERROR_NOT_OPEN;

	  // Yes we block if inqueue is empty
      if( m_receiveList.nCount == 0)
	  { 
         res = WaitForSingleObject( m_receiveDataEvent, Timeout );
         ResetEvent( m_receiveDataEvent);	

   		  if( res == WAIT_TIMEOUT )
			  return CANAL_ERROR_TIMEOUT;			
		  else if( res == WAIT_ABANDONED )
			  return CANAL_ERROR_GENERIC;	 
	  }

      if( m_receiveList.nCount == 0)
            return  CANAL_ERROR_FIFO_EMPTY;

       memcpy( pMsg, m_receiveList.pHead->pObject, sizeof( canalMsg ) );

	   LOCK_MUTEX( m_receiveMutex );
	   dll_removeNode( &m_receiveList, m_receiveList.pHead );
       UNLOCK_MUTEX( m_receiveMutex );	   	

       rv = CANAL_ERROR_SUCCESS;		

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
//	dataAvailable
//

int CUsb2canObj::dataAvailable( void )
{
    if ( !m_bOpen ) 
		return  0;

	return   m_receiveList.nCount;
}


///////////////////////////////////////////////////////////////////////////////
//	getStatistics
//

int CUsb2canObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{
	DWORD  ret;

	// Must be a message pointer
	if ( NULL == pCanalStatistics)
	   return CANAL_ERROR_PARAMETER;	

	// Must be open
    if ( !m_bOpen )
	   return CANAL_ERROR_NOT_OPEN;

   //LOCK_MUTEX( m_commandMutex );

    // cntReceiveFrames
    if (! USB2CAN_statistics( 0, & ret ))
			  return  CANAL_ERROR_HARDWARE;
    
	m_stat.cntReceiveFrames = ret;
      
    // ReceivedDataBytes
    if (! USB2CAN_statistics( 1, & ret ))
			  return  CANAL_ERROR_HARDWARE;
    
    m_stat.cntReceiveData = ret;
    
	// TransmitedFrames
    if (! USB2CAN_statistics( 2, & ret ))
			  return  CANAL_ERROR_HARDWARE;
    
    m_stat.cntTransmitFrames = ret;


    // TransmitedDataBytes
    if (! USB2CAN_statistics( 3, & ret ))
			  return  CANAL_ERROR_HARDWARE;
    
	m_stat.cntTransmitData = ret;

    // Overruns
    if (! USB2CAN_statistics( 4, & ret ))
			  return  CANAL_ERROR_HARDWARE;
    
	m_stat.cntOverruns = ret;

    // Warnings
    if (! USB2CAN_statistics( 5, & ret ))
			  return  CANAL_ERROR_HARDWARE;
    
	m_stat.cntBusWarnings = ret;

    // cntBusOff
    if (! USB2CAN_statistics( 6, & ret ))
			  return  CANAL_ERROR_HARDWARE;

	m_stat.cntBusOff = ret;

    // reset statistics counters
    if (! USB2CAN_statistics( 7, & ret ))
			  return  CANAL_ERROR_HARDWARE;
    

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );
	memset( &m_stat,0,sizeof(m_stat) );

    //UNLOCK_MUTEX( m_commandMutex );

	return CANAL_ERROR_SUCCESS;	
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
		Bit 20 - RX bus passive.
		Bit 21 - Reserved.
		Bit 22 - Reserved.
		Bit 23 - Reserved.
		Bit 24 - Reserved.
		Bit 25 - Reserved.
		Bit 26 - Reserved.
		Bit 27 - Reserved.
		Bit 28 - Reserved.
		Bit 29 - Bus Passive.
		Bit 30 - Bus Warning status.
		Bit 31 - Bus off status.
*/

/*  ERROR messages
==========================================
Flag  value  Description  
===========================================
CANAL_STATUSMSG_OK        0×00  Normal condition.  
CANAL_STATUSMSG_OVERRUN   0×01  Overrun occured when sending data to CAN bus.  
CANAL_STATUSMSG_BUSLIGHT  0×02  Error counter has reached 96.  
CANAL_STATUSMSG_BUSHEAVY  0×03  Error counter has reached 128.  
CANAL_STATUSMSG_BUSOFF    0×04  Device is in BUSOFF. CANAL_STATUSMSG_OK is sent when returning to operational mode.  
CANAL_STATUSMSG_STUFF     0×20  Stuff Error.  
CANAL_STATUSMSG_FORM      0×21  Form Error.  
CANAL_STATUSMSG_ACK       0×23  Ack Error.  
CANAL_STATUSMSG_BIT0      0×24  Bit1 Error.  
CANAL_STATUSMSG_BIT1      0×25  Bit0 Error.  
CANAL_STATUSMSG_CRC       0×26  CRC Error.  
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

int CUsb2canObj::getStatus( PCANALSTATUS pCanalStatus )
{
   //canalStatus CanalStatus;

	// Must be a message pointer
	if ( NULL == pCanalStatus)
		return CANAL_ERROR_PARAMETER;	

	// Must be open
    if ( !m_bOpen )
		return CANAL_ERROR_NOT_OPEN;

	//LOCK_MUTEX( m_commandMutex );

    if(! USB2CAN_status(pCanalStatus/*&CanalStatus*/) )
        return  CANAL_ERROR_HARDWARE;

	//UNLOCK_MUTEX( m_commandMutex );

	return CANAL_ERROR_SUCCESS;
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

	memcpy(&buff[size],&cmdmsg.data,10);
	size += 10;

    buff[size++] = 0x22;

	if(!WriteFile(hCmdOut,buff,size,(PULONG) &nBytesWrite,0))	
      return false;
	

    if(!ReadFile(hCmdIn, buff, sizeof(buff), (PULONG) &nBytesRead, NULL))    
      return false;
	

    if( (buff[0] != 0x11) || (buff[15] != 0x22) || (nBytesRead != 16) )	
	  return false;
	
	size = 0;
    size++;     // praleidziam usb msg pradzia (0x11)

	cmdmsg.channel = buff[size++];
	cmdmsg.command = buff[size++];
	cmdmsg.opt1    = buff[size++];
	cmdmsg.opt2    = buff[size++];

	memcpy(&cmdmsg.data,&buff[size],10);    

    *cmdInMsg = cmdmsg;

    return	true;
}


///////////////////////////////////////////////////////////////////////////////
// workThreadTransmit
//
//

void workThreadTransmit( void *pObject )

{
	DWORD   errorCode = 0;
	ULONG	nBytesWrite;
    //bool    err = false;
//	unsigned int		cnt;
//	int     rcnt;


	CUsb2canObj * pobj = ( CUsb2canObj *)pObject;

	if ( NULL == pobj ) 
	{
	 ExitThread( errorCode ); // Fail
	}
	
	while ( pobj->m_bRun )
	{		
		// Noting to do if we should end...
		if ( !pobj->m_bRun ) continue;

          if( pobj->m_transmitList.nCount == 0)
		   {
		      ResetEvent( pobj->m_transmitDataGetEvent );
		      if ( WAIT_OBJECT_0 != WaitForSingleObject( pobj->m_transmitDataGetEvent, 100 ))		 
			     continue;
		   }

		//while ( ( NULL != pobj->m_transmitList.pHead ) && ( NULL != pobj->m_transmitList.pHead->pObject ) )
        //while ( dll_getNodeCount( &pobj->m_transmitList) > 0 )
		while( pobj->m_transmitList.nCount > 0)
		{	 
   	        uint8_t		sendData[ 65 ];
            short		size = 0;
	        canalMsg	msg;

			memcpy( &msg, pobj->m_transmitList.pHead->pObject, sizeof( canalMsg ) ); 			
					          
			sendData[ size++ ] = 0x55; // FrameBegin
            sendData[ size++ ] = ( uint8_t ) msg.flags & 0x03; // RTR and EXT_ID flags only			
			sendData[ size++ ] = ( uint8_t )( msg.id >> 24 ) & 0x1f;
			sendData[ size++ ] = ( uint8_t )( msg.id >> 16 ) & 0xff;
			sendData[ size++ ] = ( uint8_t )( msg.id >> 8 )  & 0xff;
			sendData[ size++ ] = ( uint8_t )( msg.id ) & 0xff;       // Message ID			
            sendData[ size++ ] = msg.sizeData; // NoOfData
			memcpy( &sendData[size], msg.data, 8 ); // DATA
			size += 8;                                    
            sendData[ size++ ] = 0xAA;   // FrameEnd		

		            if( WriteFile(pobj->hDataOut,sendData,size,&nBytesWrite,0))
			         {				
					  pobj->m_stat.cntTransmitFrames++;
					  pobj->m_stat.cntTransmitData += msg.sizeData;

			          LOCK_MUTEX( pobj->m_transmitMutex );
			          dll_removeNode( &pobj->m_transmitList, pobj->m_transmitList.pHead );
			          UNLOCK_MUTEX( pobj->m_transmitMutex );

                      SetEvent( pobj->m_transmitDataPutEvent ); // Signal frame in queue                      
			         }
					else
				 	 {
					  if ( !pobj->m_bRun ) break;
					  SLEEP(1);
				  	 }
  
		 }

	} // while(pobj->m_bRun) 


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

void workThreadReceiveData( void *pObject )
{
//    bool bData;
	DWORD  errorCode = 0;

	//bool bData;
	UCHAR	RxDataBuf[100];
	ULONG	nBytesRead;
	UCHAR   FramesCnt;
    UCHAR   x;
	UCHAR   index;
	UCHAR   BeginFrame;
	UCHAR   EndFrame;
	canalMsg *pMsg = NULL;
	dllnode  *pNode = NULL;


	CUsb2canObj * pobj = ( CUsb2canObj *)pObject;   

	if ( NULL == pobj )	
	{
	 ExitThread( errorCode ); // Fail
	}		    

	while ( pobj->m_bRun )
	{		
		// Noting to do if we should end...
		if ( !pobj->m_bRun )
		  continue;

	 if( ReadFile(pobj->hDataIn, RxDataBuf, sizeof(RxDataBuf), (PULONG) &nBytesRead, NULL))
	 {		           
		  
        if ( nBytesRead == 21 )
			FramesCnt = 1 ;
		else if( nBytesRead == 42 )
			FramesCnt = 2 ;
		else if( nBytesRead == 63 )
		    FramesCnt = 3 ;
		else			
			 FramesCnt = 0;
		             
         index = 0;

         for( x = 0; x < FramesCnt; x++ ) 
		 {            
            index = (x * 21);

            pMsg = new canalMsg;

     		if ( NULL != pMsg )
			{			 
			  pNode = new dllnode; 

				if ( NULL != pNode )
				{
                   pNode->pObject = pMsg;

                     BeginFrame = RxDataBuf[index+0];

 					 pMsg->flags =
                     (((USHORT) RxDataBuf[index+1] << 8 ) & 0xff00 ) |
    			     (((USHORT) RxDataBuf[index+2]      ) & 0x00ff ) ;	                    					 

					 pMsg->id = 
					 (((DWORD)RxDataBuf[index+3]<<24 ) & 0x1f000000) |
					 (((DWORD)RxDataBuf[index+4]<<16 ) & 0x00ff0000) |
					 (((DWORD)RxDataBuf[index+5]<<8  ) & 0x0000ff00) |
					 (((DWORD)RxDataBuf[index+6]     ) & 0x000000ff) ;                	

				     pMsg->sizeData = RxDataBuf[index+7];	

					 memcpy( (UCHAR*)pMsg->data, (&RxDataBuf[index+8] ), 8 );					 

			 		 pMsg->timestamp = 
					 (((DWORD)RxDataBuf[index+16]<<24 ) & 0xff000000) |
					 (((DWORD)RxDataBuf[index+17]<<16 ) & 0x00ff0000) |
					 (((DWORD)RxDataBuf[index+18]<<8  ) & 0x0000ff00) |
					 (((DWORD)RxDataBuf[index+19]     ) & 0x000000ff) ;

                     EndFrame = RxDataBuf[index+20];

				   if( (!(pMsg->flags & CANAL_IDFLAG_STATUS)) && ( pobj->m_receiveList.nCount < USB2CAN_MAX_FIFO) )
				   {
					 LOCK_MUTEX( pobj->m_receiveMutex );
					 dll_addNode( &pobj->m_receiveList, pNode );
					 UNLOCK_MUTEX( pobj->m_receiveMutex );

					 SetEvent( pobj->m_receiveDataEvent ); // Signal frame in queue

				   }
				   else if( (pobj->m_initFlag & 8) && (pobj->m_receiveList.nCount < USB2CAN_MAX_FIFO) )
				   {
					  LOCK_MUTEX( pobj->m_receiveMutex );
					  dll_addNode( &pobj->m_receiveList, pNode );
					  UNLOCK_MUTEX( pobj->m_receiveMutex );

					  SetEvent( pobj->m_receiveDataEvent ); // Signal frame in queue
				   }
				   else 
				   {
	   			    delete pMsg;						
                    delete pNode;
                    pMsg  = NULL;
                    pNode = NULL;
				   }
								
				}//if ( NULL != pNode )
				 else
					 {
				      delete pMsg;
                      pMsg = NULL;
					 }

			}//if ( NULL != pMsg )

          
		  }// for() 


	     }//if(Read file)

	  } // while 	    

	ExitThread( errorCode );
}
