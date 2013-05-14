// serialdrv.cpp : Defines the entry point for the DLL application.
//
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "../../../../../common/com_win32.h"
#include "../../../../common/canal.h"
#include "vscp_projdefs.h"
#include "vscp_compiler.h"
#include "serialdrv.h"
#include "../../../../../../firmware/common/vscp_serial.h"

// Global handle for the com port
//		needed by the VSCP serial
//		write routine.
static HANDLE ghCommPort = 0;
static OVERLAPPED *gpOverlapped = NULL;

static HANDLE ghThisInstDll = NULL;
static DriverObj *gdrvObj = NULL;

static HANDLE gMemMapping = NULL;;	// Shared memory handle
static DWORD errorCode;


#ifdef WIN32
void workThreadTransmit( void *pObject );
void workThreadReceive( void *pObject );
#else
void *workThreadTransmit( void *pObject );
void *workThreadReceive( void *pObject );
#endif


HANDLE gObjMutex;	// Protector for driver object
char gszAdapterArray[64 * 32];
char gnAdapters = 0;
char gposAdapters = 0;


///////////////////////////////////////////////////////////////////////////////
// DllMain
//

BOOL APIENTRY DllMain( HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved )
{
    BOOL bInitialized = FALSE;

    switch( ul_reason_for_call ) {

        case DLL_PROCESS_ATTACH:

            // Create mutex for driver object
            gObjMutex = CreateMutex( NULL, TRUE, NULL );

            // Allocate storage for driver object
            if ( NULL != ( gdrvObj = (DriverObj *)malloc( sizeof( DriverObj ) ) ) ) {
                // Initialize driver object
                InitInstance( gdrvObj );
            }
            else {
                return FALSE;
            }
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:

            LOCK_MUTEX( gObjMutex );

            if ( NULL != gObjMutex ) CloseHandle( gObjMutex );

            // Free resources in driver stucture
            Detach( gdrvObj );

            // Free allocated memory
            free( gdrvObj );

            break;
    }

    return TRUE;

}

///////////////////////////////////////////////////////////////////////////////
// InitInstance
//

void InitInstance( DriverObj *pdrvObj )
{
    int i;

    // must be a valid driver object
    if ( NULL == pdrvObj ) return;


    // Init the driver array
    for ( i = 0; i<SERIAL_MAX_CANNELS; i++ ) {
        pdrvObj->m_pInterfaceObjArray[ i ].m_bInUse = FALSE;
    }

    pdrvObj->m_InstanceCount = 0;

    UNLOCK_MUTEX( gObjMutex );
}

///////////////////////////////////////////////////////////////////////////////
// Detach
//

void Detach( DriverObj *pdrvObj )
{
    int i;
    VirtualChannelObj *pvirtualChannelObj;

    // must be a valid driver object
    if ( NULL == pdrvObj ) return;

    LOCK_MUTEX( gObjMutex );

    for ( i = 0; i<SERIAL_MAX_CANNELS; i++ ) {

        if ( pdrvObj->m_pInterfaceObjArray[ i ].m_bInUse ) {

            ChannelObj *pChannelObj =  getDriverObject( pdrvObj, i );
            if ( NULL != pChannelObj ) { 

                if ( pChannelObj->m_bRun ) {
                    while( dll_getNodeCount( &pChannelObj->m_vChanneldllList ) ) { 
                        if ( NULL != pChannelObj->m_vChanneldllList.pHead ) {
                            if ( NULL != ( pvirtualChannelObj = 
                                (VirtualChannelObj *)pChannelObj->m_vChanneldllList.pHead->pObject ) ) {
                                    canclose( pChannelObj, (unsigned char)( pvirtualChannelObj->m_nChannel & 0xff ) );	
                            }
                        }
                    }	
                }

                pdrvObj->m_pInterfaceObjArray[ i ].m_bInUse = FALSE; 

            }

        }

    }

    UNLOCK_MUTEX( gObjMutex );

}

///////////////////////////////////////////////////////////////////////////////
// addChannelObject
//

long addChannelObject( DriverObj *pdrvObj, int idx )
{
    long h = 0;

    // must be a valid driver object
    if ( NULL == pdrvObj ) return 0;

    LOCK_MUTEX( gObjMutex);

    pdrvObj->m_pInterfaceObjArray[ idx ].m_bInUse = TRUE;
    h = pdrvObj->m_pInterfaceObjArray[ idx ].m_handle = (idx+1) << 8; 

    UNLOCK_MUTEX( gObjMutex );

    return h;
}

///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

ChannelObj *getDriverObject( DriverObj *pdrvObj, long h )
{
    long idx = ( ( h & 0xff00 ) >> 8 ) - 1;

    // must be a valid driver object
    if ( NULL == pdrvObj ) return NULL;

    // Check if valid handle
    if ( idx < 0 ) return NULL;
    if ( idx >= SERIAL_MAX_CANNELS ) return NULL;

    return &pdrvObj->m_pInterfaceObjArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// getFreeDriverObject
//

ChannelObj *getFreeDriverObject( DriverObj *pdrvObj, int *pIdx )
{
    int i;

    // must be a valid driver object
    if ( NULL == pdrvObj ) return NULL;

    // Must have a valid index pointer
    if ( NULL == pIdx ) return NULL;

    for ( i=0; i<SERIAL_MAX_CANNELS; i++ ) {

        if ( !pdrvObj->m_pInterfaceObjArray[ i ].m_bInUse ) {

            *pIdx = i;
            return &pdrvObj->m_pInterfaceObjArray[ i ];

        }

    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// getDriverObjectFromPort
//

ChannelObj *getDriverObjectFromPort( DriverObj *pdrvObj, const short port )
{
    int i;

    // must be a valid driver object
    if ( NULL == pdrvObj ) return NULL;

    for ( i=0; i<SERIAL_MAX_CANNELS; i++ ) {

        if ( ( pdrvObj->m_pInterfaceObjArray[i].m_bInUse ) && 
            ( pdrvObj->m_pInterfaceObjArray[i].m_com_port = port ) ) {

                return &pdrvObj->m_pInterfaceObjArray[i];

        }

    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void removeDriverObject( DriverObj *pdrvObj, long h )
{
    long idx = ( ( h & 0xff00 ) >> 8 ) - 1;

    // must be a valid driver object
    if ( NULL == pdrvObj ) return;

    // Check if valid handle
    if ( idx < 0 ) return;
    if ( idx >= SERIAL_MAX_CANNELS ) return;

    LOCK_MUTEX( gObjMutex );

    if ( pdrvObj->m_pInterfaceObjArray[ idx ].m_bInUse ) {
        pdrvObj->m_pInterfaceObjArray[ idx ].m_bInUse = FALSE;
    }


    UNLOCK_MUTEX( gObjMutex );
}

//////////////////////////////////////////////////////////////////////////////
// AssignNewVirtualHandel
//

long AssignNewVirtualHandel( DriverObj *pdrvObj, ChannelObj *pchannelObj )
{
    long k = 0;
    int i;

    for ( i=0; i<SERIAL_MAX_CANNELS; i++ ) {

        if ( NULL == dll_findNodeFromID( &pchannelObj->m_vChanneldllList, ( pchannelObj->m_handle + i ) ) ) {
            return i;
        }

    }

    return -1; // error
}

//////////////////////////////////////////////////////////////////////////////
// NewVirtualChannel
//

long NewVirtualChannel( DriverObj *pdrvObj, ChannelObj *pchannelObj )
{
    VirtualChannelObj *pvirtualChannelObj;	// Virtual channel object
    dllnode *pNode;							// Node to add
    long k = 0;								// index
    long h = 0;								// Handle
    //int cnt;

    if ( 0 <= ( k = AssignNewVirtualHandel( pdrvObj, pchannelObj ) ) ) {	

        if ( NULL != ( pvirtualChannelObj = 
            (VirtualChannelObj *)malloc( sizeof( VirtualChannelObj ) ) ) ) {

                // Assign channel
                pvirtualChannelObj->m_nChannel = h = pchannelObj->m_handle + k;

                // Create receive queue mutex
                pvirtualChannelObj->m_receiveMutex = CreateMutex( NULL, FALSE, NULL );

                // Initialize the receive queue
                dll_init( &pvirtualChannelObj->m_rxQueue, SORT_NONE );

                pvirtualChannelObj->m_receiveFrameEvent = CreateEvent( NULL,
                    TRUE,
                    FALSE,
                    NULL );

                LOCK_MUTEX( pchannelObj->m_virtualChannelsMutex );

                pNode = ( dllnode *)malloc( sizeof( dllnode ) );
                if ( NULL != pNode ) {

                    pNode->pObject = pvirtualChannelObj;
                    pNode->pKey = (unsigned long *)&pvirtualChannelObj->m_nChannel;	// Save so we can search on channel id
                    pNode->pstrKey = NULL;

                    //cnt = dll_getNodeCount( &pchannelObj->m_vChanneldllList );
                    dll_addNode( &pchannelObj->m_vChanneldllList, pNode );
                    //cnt = dll_getNodeCount( &pchannelObj->m_vChanneldllList );

                }
                else {

                    free( pvirtualChannelObj );
                    h = 0;	

                }

                UNLOCK_MUTEX( pchannelObj->m_virtualChannelsMutex );

        }

    }
    else {

        h = 0;

    }

    return h;

}

//////////////////////////////////////////////////////////////////////////////
// getVirtualChannel
//

VirtualChannelObj *getVirtualChannel( ChannelObj *pchannelObj, long handle )
{
    VirtualChannelObj *pvirtualChannelObj = NULL;	// Virtual channel object
    dllnode *pNode;	

    if ( NULL != ( pNode = dll_findNodeFromID( &pchannelObj->m_vChanneldllList, handle ) ) ){
        pvirtualChannelObj = ( VirtualChannelObj *)pNode->pObject;
    }

    return pvirtualChannelObj;
}

//////////////////////////////////////////////////////////////////////////////
// removeVirtualChannel
//

BOOL removeVirtualChannel( ChannelObj *pchannelObj, long handle )
{
    BOOL rv = TRUE;
    VirtualChannelObj *pvirtualChannelObj = NULL;	// Virtual channel object
    dllnode *pNode;	
    //int cnt;

    if ( NULL != ( pNode = dll_findNodeFromID( &pchannelObj->m_vChanneldllList, handle ) ) ){

        if ( NULL != ( pvirtualChannelObj = ( VirtualChannelObj *)pNode->pObject ) ) {

            LOCK_MUTEX( pchannelObj->m_virtualChannelsMutex );
            pNode->pObject = NULL;
            dll_removeNode( &pchannelObj->m_vChanneldllList, pNode );
            //cnt = dll_getNodeCount( &pchannelObj->m_vChanneldllList );
            UNLOCK_MUTEX( pchannelObj->m_virtualChannelsMutex );

            // Empty receive queue
            dll_removeAllNodes( &pvirtualChannelObj->m_rxQueue );
            if ( NULL != pvirtualChannelObj->m_receiveMutex ) CloseHandle( pvirtualChannelObj->m_receiveMutex );
            if ( NULL != pvirtualChannelObj->m_receiveFrameEvent ) CloseHandle( pvirtualChannelObj->m_receiveFrameEvent );
            free( pvirtualChannelObj );
        }
        else {
            rv = FALSE;
        }

    }
    else {
        rv = FALSE;
    }

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long getDataValue( const char *szData )
{
    unsigned long val;
    char *nstop;

    if ( ( NULL != strchr( szData, 'x' ) ) ||
        ( NULL != strchr( szData, 'X' ) ) ) {
            val = strtoul( szData, &nstop, 16 );
    }
    else {
        val = strtoul( szData, &nstop, 10 );
    }

    return val;	
}








////////////////////////////////////////////////////////////////////////////////
//											C A N A L  I N T E R F A C E
////////////////////////////////////////////////////////////////////////////////








///////////////////////////////////////////////////////////////////////////////
// CanalOpen
//
// Configuration string:
//		comport;channel;baudrate;bitrate;filter;mask;read-timeout;write-timeout
// 
// comport
//	A standard Windows Comm port number 1-255
//	0 == simulated interface.
//
// channel
//	An adapter channel. Used if adapter have several channels.
//
// baudrate
//	A standard Windows baudrate.
//
// bitrate 
//	can be set either as a standard value or as a register value a:b:c
//	where a is MSB, b is LSB and c is BRPR
//
// filter
//	CAN adapter filter.
//
// mask
//	CAN adapter mask.
// 
// read-timeout
//	used for bocking calls. Default is infinite. 
//
// write-timeout
//	used for bocking calls. Default is infinite. 
//

long WINAPI CanalOpen( const char *pDevice, unsigned long flags )
{
    char buf[ MAX_PATH ];
    char *p;

    // Standard driver parameters
    char szBitrate[ MAX_PATH ];
    uint32_t acceptance_filter = 0;
    uint32_t acceptance_mask = 0;
    uint32_t read_timeout = 0;
    uint32_t write_timeout = 0;

    ChannelObj *pchannelObj;		            // Channel object
    long h = 0;									// Handle
    int idxChannel = 0;

    short com_port;
    uint8_t com_channel;
    unsigned long com_baudrate;

    //VirtualChannelObj *pvirtualChannelObj;

    // Copy the devce string to a working buffer
    strcpy( buf, pDevice );

    // * * * Parse configuration string * * *

    //		Comm port
    p = strtok( buf, ";" );
    if ( NULL != p ) {
        com_port = atoi( p );	
    }
    else {
        com_port = 1;
    }

    //		Channel
    com_channel = 0;		// Set default
    p = strtok( NULL, ";" );
    if ( NULL != p ) {
        com_channel = (uint8_t)atol(p);
    }
    else {
        com_channel = 1;
    }

    //		Comm baudrate
    p = strtok( NULL, ";" );
    if ( NULL != p ) {
        com_baudrate = atol( p );	
    }
    else {
        com_baudrate = 115200;
    }

    //		Bitrate
    strcpy( szBitrate, "500" );	// set default
    p = strtok( NULL, ";" );
    if ( NULL != p ) {
        strcpy( szBitrate, p );
    }

    //		Acceptance Filter
    acceptance_filter = SERIAL_ACCEPTANCE_FILTER_ALL;	// Set default
    p = strtok( NULL, ";" );
    if ( NULL != p ) {
        acceptance_filter = atol(p);	
    }

    //		Acceptance Mask
    acceptance_mask = SERIAL_ACCEPTANCE_MASK_ALL;		// Set default
    p = strtok( NULL, ";" );
    if ( NULL != p ) {
        acceptance_mask = atol(p);
    }

    //		read-timeout
    read_timeout = INFINITE;		// Set default
    p = strtok( NULL, ";" );
    if ( NULL != p ) {
        read_timeout = atol(p);
    }

    //		write-timeout
    write_timeout = INFINITE;		// Set default
    p = strtok( NULL, ";" );
    if ( NULL != p ) {
        write_timeout = atol(p);
    }

    // Check if device has been opended yet
    if ( NULL == ( pchannelObj = getDriverObjectFromPort( gdrvObj, com_port ) ) ) {

        // No this is the first time - do full open
        pchannelObj = getFreeDriverObject( gdrvObj, &idxChannel );

        if ( NULL != pchannelObj ) {

            // Initialize the channel object
            pchannelObj->m_com_channel = com_channel;
            pchannelObj->m_com_port = com_port;
            pchannelObj->m_com_baudrate = com_baudrate;
            pchannelObj->m_handle = 0;
            pchannelObj->m_bRun = FALSE;
            pchannelObj->m_bUseProtocolSW = FALSE;
            pchannelObj->m_blockReciveTimeout = read_timeout;
            pchannelObj->m_blockTransmitTimeout = write_timeout;
            pchannelObj->acceptance_filter = acceptance_filter;
            pchannelObj->acceptance_mask = acceptance_mask;

            // Initialize virtual channel list
            dll_init( &pchannelObj->m_vChanneldllList, SORT_NUMERIC );

            if ( !( h = addChannelObject( gdrvObj, idxChannel ) ) ) {		
                // Failed - already available
                free( pchannelObj );
                return 0;
            }
            else {

                // Construct primary virtual channel
                if ( !( h = NewVirtualChannel( gdrvObj, pchannelObj ) ) ) {				
                    // Failed	
                    removeVirtualChannel( pchannelObj, h );
                    h = 0;
                    return 0;
                }

                // Open
                if ( CANAL_ERROR_SUCCESS ==  canopen( pchannelObj,
                    com_port,
                    com_baudrate,
                    szBitrate,
                    acceptance_filter,
                    acceptance_mask,
                    flags ) ) {
                        ;
                }
                else {
                    // Failure	
                    LOCK_MUTEX( gObjMutex );
                    removeVirtualChannel( pchannelObj, h );
                    removeDriverObject( gdrvObj, h );
                    UNLOCK_MUTEX( gObjMutex );
                    h = 0;
                }

            }

        }
    }
    else {

        // The user is trying to open a device that is already open in  
        // another process/thread.
        h = NewVirtualChannel( gdrvObj, pchannelObj );

    }

    return h;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalClose
// 

int WINAPI CanalClose( long handle )
{
    ChannelObj *pchannelObj;
    int rv = CANAL_ERROR_SUCCESS;
    int cnt;

    pchannelObj =  getDriverObject( gdrvObj, handle );
    if ( NULL == pchannelObj ) return CANAL_ERROR_GENERIC;

    LOCK_MUTEX( pchannelObj->m_transmitMutex );

    // The last virtual channel closes the physical device
    if ( ( cnt = dll_getNodeCount( &pchannelObj->m_vChanneldllList ) ) <= 1 ) {

        VirtualChannelObj *pvirtualChannelObj = 
            getVirtualChannel( pchannelObj, handle );
        if ( NULL != pvirtualChannelObj ) {
            pvirtualChannelObj->m_bRun = FALSE;  // Close the callback thread
        }

        rv = canclose( pchannelObj, (handle & 0xff) );
        removeVirtualChannel( pchannelObj, handle );
        removeDriverObject( gdrvObj, handle );

    }
    else {

        VirtualChannelObj *pvirtualChannelObj = 
            getVirtualChannel( pchannelObj, handle );
        pvirtualChannelObj->m_bRun = FALSE;  // Close the callback thread

        SLEEP( 500 ); // Give the worker threads some time to die

        removeVirtualChannel( pchannelObj, handle );

    }

    UNLOCK_MUTEX( pchannelObj->m_transmitMutex );


    return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetLevel
// 

unsigned long WINAPI CanalGetLevel( long handle )
{
    return CANAL_LEVEL_STANDARD;	// Supported Level == Standard
}



///////////////////////////////////////////////////////////////////////////////
//  CanalSend
// 

int WINAPI CanalSend( long handle, const PCANALMSG pCanalMsg )
{
    ChannelObj *pchannelObj =  getDriverObject( gdrvObj, handle );
    if ( NULL == pchannelObj ) return CANAL_ERROR_GENERIC;
    return canwriteMsg( pchannelObj, (handle & 0xff), pCanalMsg, FALSE );
}


///////////////////////////////////////////////////////////////////////////////
//  CanalBlockingSend
// 

int WINAPI CanalBlockingSend( long handle, PCANALMSG pCanMsg, unsigned long timeout )
{

    ChannelObj *pchannelObj =  getDriverObject( gdrvObj, handle );
    if ( NULL == pchannelObj ) return CANAL_ERROR_GENERIC;
    return canwriteMsg( pchannelObj, (handle & 0xff), pCanMsg, TRUE );

}


///////////////////////////////////////////////////////////////////////////////
//  CanalReceive
// 

int WINAPI CanalReceive( long handle, PCANALMSG pCanMsg )
{
    ChannelObj *pchannelObj =  getDriverObject( gdrvObj, handle );
    if ( NULL == pchannelObj ) return CANAL_ERROR_GENERIC;
    return canreadMsg( pchannelObj, (handle & 0xff), pCanMsg, FALSE );

}

///////////////////////////////////////////////////////////////////////////////
//  CanalBlockingReceive
// 

int WINAPI CanalBlockingReceive( long handle, PCANALMSG pCanMsg, unsigned long timeout )
{
    ChannelObj *pchannelObj =  getDriverObject( gdrvObj, handle );
    if ( NULL == pchannelObj ) return CANAL_ERROR_GENERIC;
    return  canreadMsg( pchannelObj, (handle & 0xff), pCanMsg, TRUE );
}


///////////////////////////////////////////////////////////////////////////////
//  CanalDataAvailable
// 

int WINAPI CanalDataAvailable( long handle )
{
    int nCount = 0;	

    ChannelObj *pchannelObj =  getDriverObject( gdrvObj, handle );
    if ( NULL == pchannelObj ) return CANAL_ERROR_GENERIC;

    return canDataAvailable( pchannelObj, (handle & 0xff) );
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetStatus
// 

int WINAPI CanalGetStatus( long handle, PCANALSTATUS pCanStatus )
{
    if ( NULL == pCanStatus ) return CANAL_ERROR_PARAMETER;
    /*
    //dosstatus = canusb_Status( handle );

    pCanStatus->channel_status = CANAL_STATUS_NONE;

    if ( dosstatus & CANSTATUS_RECEIVE_FIFO_FULL ) {
    pCanStatus->channel_status|= CANAL_STATUS_RECIVE_BUFFER_FULL;	
    }

    if ( dosstatus & CANSTATUS_TRANSMIT_FIFO_FULL ) {
    pCanStatus->channel_status |= CANAL_STATUS_TRANSMIT_BUFFER_FULL;
    }

    if ( dosstatus & CANSTATUS_ERROR_WARNING ) {
    pCanStatus->channel_status |= CANAL_STATUS_BUS_WARN;
    }

    if ( dosstatus & CANSTATUS_DATA_OVERRUN ) {
    // No way to report this at the moment
    }

    if ( dosstatus & CANSTATUS_ERROR_PASSIVE ) {
    pCanStatus->channel_status |= CANAL_STATUS_PASSIVE;
    }

    if ( dosstatus & CANSTATUS_ARBITRATION_LOST ) {
    // No way to report this at the moment
    }

    if ( dosstatus & CANSTATUS_BUS_ERROR ) {
    pCanStatus->channel_status |= CANAL_STATUS_BUS_OFF;
    }
    */	
    return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetStatistics
// 

int WINAPI CanalGetStatistics( long handle, PCANALSTATISTICS pCanalStatistics )
{
    //CANUsbStatistics Statistics;

    if ( NULL == pCanalStatistics ) return CANAL_ERROR_PARAMETER;

    /*
    if ( CANAL_ERROR_SUCCESS == canusb_GetStatistics( handle, &Statistics ) ) {
    pCanalStatistics->cntBusOff = Statistics.cntBusOff;
    pCanalStatistics->cntBusWarnings = Statistics.cntBusWarnings;
    pCanalStatistics->cntOverruns = Statistics.cntOverruns;
    pCanalStatistics->cntReceiveData = Statistics.cntReceiveData;
    pCanalStatistics->cntReceiveFrames = Statistics.cntReceiveFrames;
    pCanalStatistics->cntTransmitData = Statistics.cntTransmitData;
    pCanalStatistics->cntTransmitFrames =Statistics.cntTransmitFrames;
    }
    */

    return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalSetFilter
// 

int WINAPI CanalSetFilter( long handle, unsigned long filter )
{
    return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
//  CanalSetMask
// 

int WINAPI CanalSetMask( long handle, unsigned long mask )
{
    return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalSetBaudrate 
// 

int WINAPI CanalSetBaudrate ( long handle, unsigned long baudrate  )
{
    return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetVersion
// 

unsigned long WINAPI CanalGetVersion( void )
{
    return 0x010007;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetDllVersion 
// 

unsigned long WINAPI CanalGetDllVersion ( void )
{
    return NUM_DLL_VERSION;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetVendorString 
// 

const char * WINAPI CanalGetVendorString  ( void )
{
    return SERIAL_DLL_VENDOR;
}







///////////////////////////////////////////////////////////////////////////////
//                             I N T E R F A C E
//////////////////////////////////////////////////////////////////////////////








///////////////////////////////////////////////////////////////////////////////
// canopen
//

int canopen( ChannelObj *pchannelObj, 
            const short com_port,
            const unsigned long com_baudrate,
            const char *szBitrate,
            unsigned long acceptance_code,
            unsigned long acceptance_mask,
            uint32_t flags ) 
{
    //int i;
    char *p;
    char buf[ 64 ];
    unsigned char btr0, btr1;
    DWORD threadId;
    BOOL bUseBTRValues = FALSE;			// TRUE for bitrate set as btr0:btr1
    short bitrate;									// User supplied bitrate
    short nSpeed;										// bitrate code
    dllnode *pvirtualNode;
    VirtualChannelObj *pvirtualChannelObj;
    uint8_t cmddata[ 16 ];					// Datapart of command

    char szComPort[ 10 ];
    COMMTIMEOUTS ct;
    DCB dcbCommPort;	

    //vs_frame Frame;

    // Must be a channel to work on
    if ( NULL == pchannelObj) return CANAL_ERROR_PARAMETER;

    // Must be a baudrate
    if ( NULL == szBitrate) return CANAL_ERROR_PARAMETER;

    // Initialize statistic object
    pchannelObj->m_statistics.cntBusOff = 0;
    pchannelObj->m_statistics.cntBusWarnings = 0;

    pchannelObj->m_statistics.cntOverruns = 0;
    pchannelObj->m_statistics.cntReceiveData = 0;
    pchannelObj->m_statistics.cntReceiveFrames = 0;
    pchannelObj->m_statistics.cntTransmitData = 0;
    pchannelObj->m_statistics.cntTransmitFrames = 0;

    // Configure block timeouts
    pchannelObj->m_blockReciveTimeout = INFINITE;
    pchannelObj->m_blockTransmitTimeout = INFINITE;

    // Save flags argument for later use
    pchannelObj->m_flags = flags;

    // Default is real device
    pchannelObj->m_bSimulatedInterface = FALSE;

    // If com_port is zero we have a simulated device
    if ( 0 == com_port ) {

        // Mark as simulated channel
        pchannelObj->m_bSimulatedInterface = TRUE;

    }
    else {

        // Check bitrate
        if ( NULL == strchr( szBitrate, ':' ) ) {

            // Standard bitrate value
            bitrate = atoi( szBitrate );

            switch ( bitrate ) {

                case 10:
                    nSpeed = 0;
                    break;

                case 20:
                    nSpeed = 1;
                    break;

                case 50:
                    nSpeed = 2;
                    break;

                case 100:
                    nSpeed = 3;
                    break;

                case 125:
                    nSpeed = 4;
                    break;

                case 250:
                    nSpeed = 5;
                    break;

                case 500:
                    nSpeed = 6;
                    break;

                case 800:
                    nSpeed = 7;
                    break;

                case 1000:
                    nSpeed = 8;
                    break;

            }

        }

        // BTR-pair
        else {

            bUseBTRValues = TRUE;

            strcpy( buf, szBitrate );

            // BTR0
            p = strtok( buf, ":" );
            if ( NULL != p ) btr0 = ( unsigned char )getDataValue( p );

            // BTR1
            p = strtok( NULL, " \n" );
            if ( NULL != p ) btr1 = ( unsigned char )getDataValue( p );
        }

        // Open the serial channel

        // Check com-port data
        if ( com_port != 0 ) {
            sprintf( szComPort, "\\\\.\\COM%d", com_port );
        }
        else {
            return CANAL_ERROR_PARAMETER;	
        }

        if ( INVALID_HANDLE_VALUE == 
            ( pchannelObj->m_hCommPort = 
            CreateFile( szComPort, 
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, 
            NULL, 
            OPEN_EXISTING, 
            FILE_FLAG_SEQUENTIAL_SCAN, 
            NULL ) ) ) {
                return CANAL_ERROR_SUB_DRIVER;
        }

        // Set up the buffer sizes
        SetupComm( pchannelObj->m_hCommPort, 0x4000, 512 ); // set buffer sizes

        // Save a global for COM port handle
        ghCommPort = pchannelObj->m_hCommPort;

        memset( &ct, 0, sizeof( ct ) );

        // Set timeouts
        ct.ReadIntervalTimeout = MAXDWORD;			 
        ct.ReadTotalTimeoutMultiplier = MAXDWORD;	 
        ct.ReadTotalTimeoutConstant = 200;		 
        ct.WriteTotalTimeoutMultiplier = 0;		
        ct.WriteTotalTimeoutConstant = 200;	
        SetCommTimeouts( pchannelObj->m_hCommPort, &ct );

        // Comm state
        if ( !GetCommState( pchannelObj->m_hCommPort, &dcbCommPort ) ) {
            CloseHandle( pchannelObj->m_hCommPort );
            return CANAL_ERROR_SUB_DRIVER;
        }

        dcbCommPort.DCBlength = sizeof( DCB );
        dcbCommPort.BaudRate = com_baudrate;	
        dcbCommPort.Parity = NOPARITY;
        //dcbCommPort.fDtrControl = DTR_CONTROL_ENABLE;
        dcbCommPort.fDsrSensitivity = DTR_CONTROL_DISABLE;
        dcbCommPort.ByteSize = 8;
        dcbCommPort.StopBits = ONESTOPBIT;

        // Software flow control
        dcbCommPort.fOutX = FALSE;
        dcbCommPort.fInX = FALSE;

        // Hardware flow control
        dcbCommPort.fOutxCtsFlow = FALSE;
        dcbCommPort.fOutxDsrFlow = FALSE;

        if ( !SetCommState( pchannelObj->m_hCommPort, &dcbCommPort ) )  {
            CloseHandle( pchannelObj->m_hCommPort );
            return CANAL_ERROR_SUB_DRIVER;
        }

        SetCommMask( pchannelObj->m_hCommPort, EV_TXEMPTY | EV_RXCHAR | EV_RXFLAG );


        // Set adapter bitrate
        if ( bUseBTRValues ) {
            cmddata[ 0 ] = 1;
            cmddata[ 1 ] = btr0;
            cmddata[ 2 ] = btr1;
        }
        else {
            cmddata[ 0 ] = 0;
            cmddata[ 1 ] = nSpeed;
        }
        vs_sendCommandFrame( 1,	    // Channel
                                0,	// Sequency
                                SERIAL_COMMAND_SET_BITRATE,
                                3,	// datacount
                                cmddata );

        // Set adapter filter
        cmddata[ 3 ] = (uint8_t)(( pchannelObj->acceptance_filter >> 0 ) & 0xff);
        cmddata[ 2 ] = (uint8_t)(( pchannelObj->acceptance_filter >> 9 ) & 0xff);
        cmddata[ 1 ] = (uint8_t)(( pchannelObj->acceptance_filter >> 16 ) & 0xff);
        cmddata[ 0 ] = (uint8_t)(( pchannelObj->acceptance_filter >> 24 ) & 0xff);
        vs_sendCommandFrame( 1,	    // Channel
                                1,	// Sequency
                                SERIAL_COMMAND_SET_FILTER,
                                4,	// datacount
                                cmddata );

        // set adapter mask
        cmddata[ 3 ] = (uint8_t)(( pchannelObj->acceptance_mask >> 0 ) & 0xff);
        cmddata[ 2 ] = (uint8_t)(( pchannelObj->acceptance_mask >> 9 ) & 0xff);
        cmddata[ 1 ] = (uint8_t)(( pchannelObj->acceptance_mask >> 16 ) & 0xff);
        cmddata[ 0 ] = (uint8_t)(( pchannelObj->acceptance_mask >> 24 ) & 0xff);
        vs_sendCommandFrame( 1,	    // Channel
                                2,	// Sequency
                                SERIAL_COMMAND_SET_MASK,
                                4,	// datacount
                                cmddata );

        // open adapter
        vs_sendCommandFrame( 1,	    // Channel
                                3,	// Sequency
                                SERIAL_COMMAND_OPEN,
                                0,	// datacount
                                cmddata );

    } // real device

    // Initialize the queue's
    dll_init( &pchannelObj->m_txQueue, SORT_NONE );

    pchannelObj->m_bRun = TRUE;

    // Go out and do some real work...

    // Init sliding window struct
    vs_initSlidingWndFrames( &pchannelObj->m_swFrameStruct, 
                                SW_TIMEOUT,
                                SW_MAX_RESENDS );

    pchannelObj->m_comMutex = CreateMutex( NULL, TRUE, NULL );			// SERIAL_DLL_COMCHANNEL_MUTEX
    pchannelObj->m_transmitMutex = CreateMutex( NULL, TRUE, NULL ); // SERIAL_DLL_TRANSMIT_MUTEX
    pchannelObj->m_virtualChannelsMutex = CreateMutex( NULL, TRUE, NULL );
    pchannelObj->m_swMutex = CreateMutex( NULL, TRUE, NULL );				// Sliding window mutex

    pchannelObj->m_receiveCharEvent = CreateEvent( NULL,
        FALSE,
        FALSE,
        NULL );			// SERIAL_DLL_RECEIVE_CHAR_EVENT


    // Define event notification for received characters
    pchannelObj->m_transmitFrameEvent = CreateEvent( NULL,
        FALSE,
        FALSE,
        NULL );		// SERIAL_DLL_TRANSMIT_FRAME_EVENT


    // Define event notification for received characters
    pchannelObj->m_AckNackEvent = CreateEvent( NULL,
        FALSE,
        FALSE,
        NULL );				// SERIAL_DLL_ACKNACK_EVENT

    // transmit block
    pchannelObj->m_transmitRemoveEvent = CreateEvent( NULL,
        TRUE,
        TRUE,
        NULL );		// SERIAL_DLL_TRANSMIT_REMOVE_EVENT

    // We don't start worker threads for a simulated device
    if ( !pchannelObj->m_bSimulatedInterface ) {


        // Start read thread 
        if ( NULL == 
            ( pchannelObj->m_hTreadReceive = 
            CreateThread(	NULL,
            0,
            (LPTHREAD_START_ROUTINE) workThreadReceive,
            pchannelObj,
            0,
            &threadId ) ) ) { 
                // Failure
                canclose( pchannelObj, 0 );
                return  CANAL_ERROR_GENERIC;
        }

        SetThreadPriority( pchannelObj->m_hTreadReceive, THREAD_PRIORITY_TIME_CRITICAL );


        // Start transmit thread 
        if ( NULL == 
            ( pchannelObj->m_hTreadTransmit = 
            CreateThread(	NULL,
            0,
            (LPTHREAD_START_ROUTINE) workThreadTransmit,
            pchannelObj,
            0,
            &threadId ) ) ) { 
                // Failure
                canclose( pchannelObj, 0 );
                return  CANAL_ERROR_GENERIC;
        }

        SetThreadPriority( pchannelObj->m_hTreadTransmit, THREAD_PRIORITY_TIME_CRITICAL );

    }

    // Release the mutex
    UNLOCK_MUTEX( pchannelObj->m_comMutex );
    UNLOCK_MUTEX( pchannelObj->m_transmitMutex );
    UNLOCK_MUTEX( pchannelObj->m_virtualChannelsMutex );
    UNLOCK_MUTEX( pchannelObj->m_swMutex );

    // *** Unlock receive mutexes ***

    // Point at first virtual node
    pvirtualNode = pchannelObj->m_vChanneldllList.pHead;

    LOCK_MUTEX( pchannelObj->m_virtualChannelsMutex );

    while ( NULL != pvirtualNode  ) {

        // Noting to do if no object
        if ( NULL == ( pvirtualChannelObj = 
            (VirtualChannelObj *)pvirtualNode->pObject ) ) {
                pvirtualNode = pvirtualNode->pNext;
                continue;
        }
        UNLOCK_MUTEX( pvirtualChannelObj->m_receiveMutex );

        // Point at next virtual channel
        pvirtualNode = pvirtualNode->pNext;

    }

    UNLOCK_MUTEX( pchannelObj->m_virtualChannelsMutex );

    pchannelObj->m_bOpen = TRUE;	// We are open
    return CANAL_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// canclose
//

int canclose( ChannelObj *pchannelObj, CANHANDLE channel )
{
    uint8_t cmddata[ 16 ];

    // Must be a channel to work on
    if ( NULL == pchannelObj) return CANAL_ERROR_PARAMETER;

    pchannelObj->m_bRun = FALSE;

    // close adapter
    vs_sendCommandFrame( 0,
                            0,
                            SERIAL_COMMAND_CLOSE,
                            0,
                            cmddata );

    SLEEP( 500 ); // Give the worker threads some time to die

    LOCK_MUTEX( pchannelObj->m_comMutex );
    LOCK_MUTEX( pchannelObj->m_transmitMutex );
    LOCK_MUTEX( pchannelObj->m_virtualChannelsMutex );
    LOCK_MUTEX( pchannelObj->m_swMutex );

    if ( NULL != pchannelObj->m_comMutex) CloseHandle( pchannelObj->m_comMutex );
    if ( NULL != pchannelObj->m_transmitMutex ) CloseHandle( pchannelObj->m_transmitMutex );
    if ( NULL != pchannelObj->m_virtualChannelsMutex) CloseHandle( pchannelObj->m_virtualChannelsMutex );
    if ( NULL != pchannelObj->m_swMutex) CloseHandle( pchannelObj->m_swMutex );

    if ( NULL != pchannelObj->m_receiveCharEvent ) CloseHandle( pchannelObj->m_receiveCharEvent );
    if ( NULL != pchannelObj->m_transmitFrameEvent ) CloseHandle( pchannelObj->m_transmitFrameEvent );
    if ( NULL != pchannelObj->m_AckNackEvent ) CloseHandle( pchannelObj->m_AckNackEvent );
    if ( NULL != pchannelObj->m_transmitRemoveEvent ) CloseHandle( pchannelObj->m_transmitRemoveEvent );



    CloseHandle( pchannelObj->m_hCommPort );
    pchannelObj->m_hCommPort = NULL;
    ghCommPort = NULL;

    // Remove any data still in the queues
    dll_removeAllNodes( &pchannelObj->m_txQueue );

    pchannelObj->m_bOpen = FALSE; // We are closed
    return CANAL_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// canreadMsg
//

int canreadMsg( ChannelObj *pchannelObj, CANHANDLE channel, canalMsg *pMsg, BOOL bBlocking )
{
    int rv = CANAL_ERROR_SUCCESS;
    VirtualChannelObj *pvirtualChannelObj;

    // Must be a channel to work on
    if ( NULL == pchannelObj) return CANAL_ERROR_PARAMETER;

    pvirtualChannelObj = 
        getVirtualChannel( pchannelObj,  ( pchannelObj->m_handle + channel ) );

    // Must be a virtual channel
    if ( NULL == pvirtualChannelObj) return CANAL_ERROR_PARAMETER;

    // Must be a message pointer
    if ( NULL == pMsg) return CANAL_ERROR_PARAMETER;

    // Must be open
    if ( !pchannelObj->m_bOpen ) return CANAL_ERROR_NOT_OPEN;

    // Should we block
    if ( bBlocking /*pchannelObj->m_flags & SERIAL_FLAG_BLOCK*/ ) {
        // Yes we block if inqueue is empty
        if ( ( NULL == pvirtualChannelObj->m_rxQueue.pHead ) ) { 
            if ( WAIT_TIMEOUT == 
                WaitForSingleObject( pvirtualChannelObj->m_receiveFrameEvent, 
                pchannelObj->m_blockReciveTimeout ) ) {
                    // Timeout
                    return CANAL_ERROR_TIMEOUT;
            }
        }
    }

    LOCK_MUTEX( pvirtualChannelObj->m_receiveMutex );

    if ( ( NULL != pvirtualChannelObj->m_rxQueue.pHead ) && 
        ( NULL != pvirtualChannelObj->m_rxQueue.pHead->pObject ) ) {	

            memcpy( pMsg, pvirtualChannelObj->m_rxQueue.pHead->pObject, sizeof( canalMsg ) );
            dll_removeNode( &pvirtualChannelObj->m_rxQueue, pvirtualChannelObj->m_rxQueue.pHead );
            if ( 0 == dll_getNodeCount( &pvirtualChannelObj->m_rxQueue ) ) {
                ResetEvent( pvirtualChannelObj->m_receiveFrameEvent);	
            }
    }
    else {
        rv = CANAL_ERROR_FIFO_EMPTY;
    }

    UNLOCK_MUTEX( pvirtualChannelObj->m_receiveMutex );

    return rv;
}



///////////////////////////////////////////////////////////////////////////////
// canwriteMsg
//

int canwriteMsg( ChannelObj *pchannelObj, CANHANDLE channel, canalMsg *pMsg, BOOL bBlocking  )
{
    dllnode *pNode;
    canalMsg *pnewMsg;
    int rv = CANAL_ERROR_SUCCESS;

    VirtualChannelObj *pvirtualChannelObj;
    dllnode *pvirtualNode;

    // Must be open
    if ( !pchannelObj->m_bOpen ) return CANAL_ERROR_NOT_OPEN;

    // Must be a channel to work on
    if ( NULL == pchannelObj) return CANAL_ERROR_PARAMETER;

    // Must be a message pointer
    if ( NULL == pMsg) return CANAL_ERROR_PARAMETER;

    // If we should block we need to wait if the buffer is full
    if ( bBlocking /*pchannelObj->m_flags & SERIAL_FLAG_BLOCK*/ ) {
        if ( dll_getNodeCount( &pchannelObj->m_txQueue ) >= CANBUS_TXFIFO_SIZE ) {
            ResetEvent( pchannelObj->m_transmitRemoveEvent );
            if ( WAIT_TIMEOUT == WaitForSingleObject( pchannelObj->m_transmitRemoveEvent, 
                pchannelObj->m_blockTransmitTimeout ) ) {
                    return CANAL_ERROR_TIMEOUT;
            }
        }
    }

    // * * * Handle trasmit queue

    LOCK_MUTEX( pchannelObj->m_transmitMutex );

    // Must be room for the message
    if ( dll_getNodeCount( &pchannelObj->m_txQueue ) < CANBUS_TXFIFO_SIZE ) {

        if ( NULL != pMsg ) {

            pNode = (dllnode *)malloc( sizeof( dllnode ) );

            if ( NULL != pNode ) {

                pNode->pObject = NULL;
                pnewMsg = (canalMsg *)malloc( sizeof(canalMsg ) );

                if ( NULL != pnewMsg ) {

                    pNode->pObject = pnewMsg;
                    pNode->pKey = &pMsg->id;	// save id so we can search on it
                    pNode->pstrKey = NULL;

                    if ( NULL != pnewMsg ) {
                        memcpy( pnewMsg, pMsg, sizeof( canalMsg ) );
                    }

                    dll_addNode( &pchannelObj->m_txQueue, pNode );	
                    SetEvent( pchannelObj->m_transmitFrameEvent );	// Signal frame to send
                }
                else {
                    free( pnewMsg );
                    rv = CANAL_ERROR_MEMORY;
                }

            }
            else {
                rv = CANAL_ERROR_MEMORY;
            }
        }
        else {
            rv = CANAL_ERROR_PARAMETER;
        }
    }
    else {
        rv = CANAL_ERROR_FIFO_FULL;
    }

    UNLOCK_MUTEX( pchannelObj->m_transmitMutex );


    // * * * Handle Virtual channels


    if ( !( pchannelObj->m_flags & SERIAL_FLAG_NO_LOCAL_SEND ) ) {

        LOCK_MUTEX( pchannelObj->m_virtualChannelsMutex );

        // Point at first virtual node
        pvirtualNode = pchannelObj->m_vChanneldllList.pHead;

        while ( NULL != pvirtualNode  ) {

            // Noting to do if no object
            if ( NULL == ( pvirtualChannelObj = (VirtualChannelObj *)pvirtualNode->pObject ) ) {
                pvirtualNode = pvirtualNode->pNext;
                continue;
            }

            // Do nothing for same channel
            if ( channel == ( pvirtualChannelObj->m_nChannel & 0xff ) ) {
                pvirtualNode = pvirtualNode->pNext;
                continue;
            }

            LOCK_MUTEX( pvirtualChannelObj->m_receiveMutex );

            // If the input queue is full we remove the first message in
            // the queue if the QUEUE_REPLACE flag is set to be able to
            // add the new message at the end.
            if ( ( pchannelObj->m_flags & SERIAL_FLAG_QUEUE_REPLACE ) && 
                ( dll_getNodeCount( &pvirtualChannelObj->m_rxQueue ) >= CANBUS_RXFIFO_SIZE ) ) {

                    if ( NULL != pvirtualChannelObj->m_rxQueue.pHead ) { 

                        // Remove the object
                        if ( NULL != pvirtualChannelObj->m_rxQueue.pHead->pObject ) {
                            free( (canalMsg *)pvirtualChannelObj->m_rxQueue.pHead->pObject ); 
                            pvirtualChannelObj->m_rxQueue.pHead->pObject = NULL;
                        }

                        dll_removeNode( &pvirtualChannelObj->m_rxQueue, pvirtualChannelObj->m_rxQueue.pHead );	 
                    }

            }


            if ( dll_getNodeCount( &pvirtualChannelObj->m_rxQueue ) < CANBUS_RXFIFO_SIZE ) {				

                pnewMsg = (canalMsg *)malloc( sizeof( canalMsg ) );

                if ( NULL != pMsg ) {

                    memcpy( pnewMsg, pMsg, sizeof( canalMsg ) );

                    if ( !pchannelObj->m_bHardTimeStamp ) {
                        pnewMsg->timestamp = GetTickCount();
                    }	

                    pNode = (dllnode *)malloc( sizeof( dllnode ) );

                    if ( NULL != pNode ) {

                        pNode->pObject = pnewMsg;
                        pNode->pKey = &pnewMsg->id;	// Save so we can search on id
                        pNode->pstrKey = NULL;

                        dll_addNode( &pvirtualChannelObj->m_rxQueue, pNode );
                        SetEvent( pvirtualChannelObj->m_receiveFrameEvent ); // Signal frame in queue

                        pchannelObj->m_statistics.cntReceiveData += pnewMsg->sizeData;
                        pchannelObj->m_statistics.cntReceiveFrames++;

                    }
                }									
            }
            else {

                pchannelObj->m_statistics.cntOverruns++;

            }	

            UNLOCK_MUTEX( pvirtualChannelObj->m_receiveMutex );

            // Point at next virtual channel
            pvirtualNode = pvirtualNode->pNext;

        } // while

        UNLOCK_MUTEX( pchannelObj->m_virtualChannelsMutex );

    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// canGetStatistics
//

int canGetStatistics( ChannelObj *pchannelObj, 
                     CANHANDLE channel, 
                     PCANALSTATISTICS pStatistics )
{
    // Must be a channel to work on
    if ( NULL == pchannelObj) return CANAL_ERROR_PARAMETER;

    // Must have a pointer to a statistics structure
    if ( NULL == pStatistics) return CANAL_ERROR_PARAMETER;

    // Must be open
    if ( !pchannelObj->m_bOpen ) return CANAL_ERROR_NOT_OPEN;

    // Copy statistics struct
    memcpy( pStatistics, &pchannelObj->m_statistics, sizeof( canalStatistics ) );

    return CANAL_ERROR_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
// canDataAvailable
//

int canDataAvailable( ChannelObj *pchannelObj, CANHANDLE channel )
{
    VirtualChannelObj *pvirtualChannelObj;

    // Must be a channel to work on
    if ( NULL == pchannelObj) return CANAL_ERROR_PARAMETER;

    // Must be open
    if ( !pchannelObj->m_bOpen ) return CANAL_ERROR_NOT_OPEN;

    pvirtualChannelObj = 
        getVirtualChannel( pchannelObj, pchannelObj->m_handle );

    // Must be a virtual channel
    if ( NULL == pvirtualChannelObj) return CANAL_ERROR_PARAMETER;

    return dll_getNodeCount( &pvirtualChannelObj->m_rxQueue );

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
    //int i;
    uint32_t timestamp;
    canalMsg msg;
    vs_frame serialFrame;			// VSCP Serial frame

    int swCount = 0;					// Number of outstanding frames
    uint8_t seqnumber = 0;

#ifdef WIN32
    DWORD errorCode = 0;
#else
    int rv = 0;
#endif

    ChannelObj * pobj = ( ChannelObj *)pObject;
    if ( NULL == pobj ) {
#ifdef WIN32	
        ExitThread( -1 ); // Fail
#else
        pthread_exit( &rv );
#endif
    }

    ghCommPort = pobj->m_hCommPort;

    while ( pobj->m_bRun ) {

        if ( WAIT_OBJECT_0 == WaitForSingleObject( pobj->m_transmitFrameEvent, 100 ) ) {

            // * * * Send Frames from queue * * *

            // Any data to transmit
            while ( NULL != pobj->m_txQueue.pHead ) {

                if ( pobj->m_swFrameStruct.cntOutstanding >= VSCP_SERIAL_MAX_SLIDING_WINDOW_FRAMES ) {
                    if ( pobj->m_bUseProtocolSW ) { // If sw protocol is used
                        SLEEP( 1 );
                        continue;
                    }
                }

                if ( NULL != pobj->m_txQueue.pHead->pObject ) {

                    LOCK_MUTEX( pobj->m_transmitMutex );
                    if ( NULL != pobj->m_txQueue.pHead ) {
                        memcpy( &msg, pobj->m_txQueue.pHead->pObject, sizeof( canalMsg ) );
                    }
                    dll_removeNode( &pobj->m_txQueue, pobj->m_txQueue.pHead );
                    UNLOCK_MUTEX( pobj->m_transmitMutex );

                    SetEvent( pobj->m_transmitRemoveEvent ); // Signal frame removed

                    LOCK_MUTEX( pobj->m_comMutex );

                    serialFrame.channel = pobj->m_com_channel;
                    serialFrame.seqnumber = seqnumber++;
                    serialFrame.flags = 4 + 4 + msg.sizeData;
                    serialFrame.data[ 3 ] = (uint8_t)(( msg.id >> 0 ) & 0xff);
                    serialFrame.data[ 2 ] = (uint8_t)(( msg.id >> 9 ) & 0xff);
                    serialFrame.data[ 1 ] = (uint8_t)(( msg.id >> 16 ) & 0xff);
                    serialFrame.data[ 0 ] = (uint8_t)(( msg.id >> 24 ) & 0xff);
                    serialFrame.data[ 0 ] |= VSCP_SERIAL_CAN_DATA0_EXTENDED;
                    memcpy( ( serialFrame.data + 4 ), msg.data, msg.sizeData );

                    // Timestamp
                    timestamp = GetTickCount();
                    serialFrame.data[ 7 + msg.sizeData ] = (uint8_t)(( timestamp>> 0 ) & 0xff);
                    serialFrame.data[ 6 + msg.sizeData ] = (uint8_t)(( timestamp >> 9 ) & 0xff);
                    serialFrame.data[ 5 + msg.sizeData ] = (uint8_t)(( timestamp >> 16 ) & 0xff);
                    serialFrame.data[ 4 + msg.sizeData ] = (uint8_t)(( timestamp >> 24 ) & 0xff);

                    serialFrame.opcode = VSCP_SERIAL_OPCODE_CAN_FRAME;
                    serialFrame.vscpclass = 0;
                    serialFrame.vscptype = 0;

                    vs_sendFrame( &serialFrame );

                    // Add to sliding window storage
                    if ( pobj->m_bUseProtocolSW ) { // If sw protocol is used
                        vs_addSlidingWndFrame( &pobj->m_swFrameStruct,
                            &serialFrame,
                            GetTickCount() );
                    }

                    pobj->m_statistics.cntTransmitData += msg.sizeData - 4;
                    pobj->m_statistics.cntTransmitFrames++;

                    UNLOCK_MUTEX( pobj->m_comMutex );

                }
                else {
                    // object is null - just remove the message
                    LOCK_MUTEX( pobj->m_transmitMutex );
                    dll_removeNode( &pobj->m_txQueue, pobj->m_txQueue.pHead );
                    UNLOCK_MUTEX( pobj->m_transmitMutex );
                }

            } // while

            // Check if any sliding window retransmits are needed
            if ( pobj->m_bUseProtocolSW ) { // If sw protocol is used
                vs_timeoutSlidingWndFrame( &pobj->m_swFrameStruct, GetTickCount() );
            }

        } // wait object

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

void workThreadReceive( void *pObject )
{
    unsigned int i;
    BOOL bData = FALSE;
    DWORD errorCode = 0;
    int result;
    DWORD dwCount;	// Number of characters in receive queue
    char bufferRx[ RECEIVE_THREAD_BUFFER_SIZE ]; // USB receive buffer

    char state = SERIAL_STATE_NONE;

    vs_frame serialFrame;		// VSCP Serial frame
    canalMsg *pMsg;					// Message to add
    dllnode *pNode;					// Node to add

    VirtualChannelObj *pvirtualChannelObj;
    dllnode *pvirtualNode;

    //DWORD dwEvtMask;

    ChannelObj * pobj = ( ChannelObj *)pObject;
    if ( NULL == pobj ) {	
        ExitThread( -1 ); // Fail
    }

    //ExitThread( errorCode );

    while ( pobj->m_bRun ) {

        LOCK_MUTEX( pobj->m_swMutex );
        vs_timeoutSlidingWndFrame( &pobj->m_swFrameStruct, GetTickCount() );
        UNLOCK_MUTEX( pobj->m_swMutex );

        /*	
        WaitCommEvent( pobj->m_hCommPort,			// wait for data - but this
        &dwEvtMask,						// will return immediately because
        NULL );								// it is overlapped	

        if ( dwEvtMask & EV_RXCHAR ) {
        result = 1; 
        }

        if ( dwEvtMask & EV_RXFLAG ) {
        result = 2;
        }




        */
        //if ( WAIT_OBJECT_0 == WaitForSingleObject( pobj->m_receiveCharEvent, 100 ) ) {

        LOCK_MUTEX( pobj->m_comMutex );

        // Check if there is something to receive
        dwCount = 0;
        result = ReadFile( pobj->m_hCommPort, 
            bufferRx, 
            sizeof( bufferRx ), 
            &dwCount, 
            NULL );

        if ( result == 0) {	
            int err = GetLastError();
        }

        for ( i=0; i<dwCount; i++ ) {

            if ( vs_feed( bufferRx[ i ] ) ) {

                // We have a message
                if ( vs_getFrame( &serialFrame ) ) {

                    switch ( serialFrame.opcode ) {

                            case VSCP_SERIAL_OPCODE_NOOP:

                                // We don't do anything other then ACK the frame
                                vs_sendAck( serialFrame.channel, serialFrame.seqnumber );
                                break;

                            case VSCP_SERIAL_OPCODE_LEVEL1_EVENT:
                                break;

                            case VSCP_SERIAL_OPCODE_CAN_FRAME:

                                // Point at first virtual node
                                pvirtualNode = pobj->m_vChanneldllList.pHead;

                                while ( NULL != pvirtualNode  ) {

                                    // Noting to do if no object
                                    if ( NULL == ( pvirtualChannelObj = ( VirtualChannelObj *)pvirtualNode->pObject ) ) {
                                        pvirtualNode = pvirtualNode->pNext;
                                        continue;
                                    }

                                    LOCK_MUTEX( pvirtualChannelObj->m_receiveMutex );

                                    // If the input queue is full we remove the first message in
                                    // the queue if the QUEUE_REPLACE flag is set to be able to
                                    // add the new message at the end.
                                    if ( ( pobj->m_flags & SERIAL_FLAG_QUEUE_REPLACE ) && 
                                        ( dll_getNodeCount( &pvirtualChannelObj->m_rxQueue ) >= CANBUS_RXFIFO_SIZE ) ) {

                                            if ( NULL != pvirtualChannelObj->m_rxQueue.pHead ) { 

                                                // Remove the object
                                                if ( NULL != pvirtualChannelObj->m_rxQueue.pHead->pObject ) {
                                                    free( (canalMsg *)pvirtualChannelObj->m_rxQueue.pHead->pObject ); 
                                                    pvirtualChannelObj->m_rxQueue.pHead->pObject = NULL;
                                                }

                                                dll_removeNode( &pvirtualChannelObj->m_rxQueue, pvirtualChannelObj->m_rxQueue.pHead );	 
                                            }

                                    }

                                    if ( dll_getNodeCount( &pvirtualChannelObj->m_rxQueue ) < CANBUS_RXFIFO_SIZE ) {				

                                        pMsg = (canalMsg *)malloc( sizeof( canalMsg ) );

                                        pMsg->flags = CANAL_IDFLAG_EXTENDED;
                                        if ( serialFrame.data[0] & 0x40 ) {
                                            pMsg->flags |= CANAL_IDFLAG_RTR;   // Remote frame
                                        }

                                        // EXT + RTR removed om fata[ 3 ];
                                        pMsg->id = ( serialFrame.data[ 3 ] & 0x3f ) +  
                                            ( serialFrame.data[ 2 ] << 8 ) +
                                            ( serialFrame.data[ 1 ] << 16 ) +
                                            ( serialFrame.data[ 0 ] << 24 );
                                        pMsg->sizeData = ( serialFrame.flags & 0x1f ) - 8; // less timestamp + id
                                        memcpy( pMsg->data, ( serialFrame.data + 4 ), pMsg->sizeData );

                                        // Timestamp
                                        memcpy( &pMsg->timestamp, 
                                            ( serialFrame.data + 4 + pMsg->sizeData  ), 
                                            4 );

                                        if ( !pobj->m_bHardTimeStamp ) {
                                            pMsg->timestamp = GetTickCount();
                                        }	

                                        pNode = (dllnode *)malloc( sizeof( dllnode ) );

                                        if ( NULL != pNode ) {

                                            pNode->pObject = pMsg;
                                            pNode->pKey = &pMsg->id;	// Save so we can search on id
                                            pNode->pstrKey = NULL;

                                            dll_addNode( &pvirtualChannelObj->m_rxQueue, pNode );
                                            SetEvent( pvirtualChannelObj->m_receiveFrameEvent ); // Signal frame in queue

                                            pobj->m_statistics.cntReceiveData += pMsg->sizeData;
                                            pobj->m_statistics.cntReceiveFrames++;

                                        }

                                    }
                                    else {

                                        pobj->m_statistics.cntOverruns++;

                                    }	

                                    UNLOCK_MUTEX( pvirtualChannelObj->m_receiveMutex );

                                    // Point at next virtual channel
                                    pvirtualNode = pvirtualNode->pNext;

                                } // while

                                break;


                            case VSCP_SERIAL_OPCODE_LEVEL2_EVENT:

                                // We don't accept LEVEL II events
                                vs_sendNack( serialFrame.channel, serialFrame.seqnumber );
                                break;


                            case VSCP_SERIAL_OPCODE_POLL_EVENT: 
                                // We don't accept poll events
                                vs_sendNack( serialFrame.channel, serialFrame.seqnumber );
                                break;


                            case VSCP_SERIAL_OPCODE_NO_EVENT:

                                // We don't do anything
                                break;


                            case VSCP_SERIAL_OPCODE_ACK:
                                if ( pobj->m_bUseProtocolSW ) { // If sw protocol is used
                                    LOCK_MUTEX( pobj->m_swMutex );
                                    if ( NULL != vs_findFlyingFrame( &pobj->m_swFrameStruct,
                                        serialFrame.channel, 																	
                                        serialFrame.seqnumber ) ) {
                                            // Found  so remove it
                                            vs_removeSlidingWndFrame( &pobj->m_swFrameStruct,
                                                serialFrame.channel, 
                                                serialFrame.seqnumber );
                                    }
                                    UNLOCK_MUTEX( pobj->m_swMutex );
                                }
                                break;


                            case VSCP_SERIAL_OPCODE_NACK:

                                // NACK received
                                break;


                            case VSCP_SERIAL_OPCODE_ERROR:

                                // We don't do anything
                                break;


                            case VSCP_SERIAL_OPCODE_REPLY:
                                break;


                            case VSCP_SERIAL_OPCODE_COMMAND:
                                break;

                    } // Frame type

                } // valid frame

            } // frame

        } // for

        UNLOCK_MUTEX( pobj->m_comMutex );	

        // Release system for a while if noting to read
        if ( 0 == dwCount ) {
            SLEEP( 100 );
        }

        //}	// wait object

    } // while

    CloseHandle( pobj->m_hCommPort );
    pobj->m_hCommPort = NULL;
    ghCommPort = NULL;

    ExitThread( errorCode );

}



///////////////////////////////////////////////////////////////////////////////
// vs_sendBuffer
//
// Defined in vscp_serial.h and required to be defined in the project to send
// a character on the serial line
//

BOOL vs_sendBuffer( uint8_t *pbuf, uint16_t count )
{
    DWORD dwCount;

    // First check if port is open
    if ( NULL == ghCommPort ) return FALSE;

    return WriteFile( ghCommPort, pbuf, count, &dwCount, NULL );
}

///////////////////////////////////////////////////////////////////////////////
// vs_sendBuffer
//
// Defined in vscp_serial.h and required to be defined in the project to send
// a character on the serial line
//

BOOL vs_sendSerialByte( uint8_t b )
{
    DWORD dwCount;

    // First check if port is open
    if ( NULL == ghCommPort ) return FALSE;

    return WriteFile( ghCommPort, &b, 1, &dwCount, NULL );
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//                       S e r i a l   R o u t i n e s
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////








/////////////////////////////////////////////////////////////////////////////
// com_readChar
//

char com_readChar( HANDLE hCommPort, int* cnt )
{
    char c = 0;
    DWORD dwCount;

    int result = 0;

    result = ReadFile( hCommPort, &c, 1, &dwCount, gpOverlapped );
    if (result == 0) {	
        int err = GetLastError();
    }

    *cnt = dwCount;
    return c;
}


///////////////////////////////////////////////////////////////////////////
// com_readBuf
//

uint16_t com_readBuf( HANDLE hCommPort,
                     char *pBuf, 
                     unsigned short size )
{
    DWORD nRead;
    DWORD rv;

    if ( !ReadFile( hCommPort, pBuf, size, &nRead, NULL ) ) {
        rv = GetLastError();
    }
    return (uint16_t)nRead;
}


/////////////////////////////////////////////////////////////////////////////
// com_write
//

void com_write( HANDLE hCommPort, char *pstr, BOOL bCRLF, BOOL bNoLF )
{
    DWORD dwCount;
    char c;

    WriteFile( hCommPort, pstr, strlen( pstr ), &dwCount, NULL );

    if ( bCRLF ) {
        c = 13;
        WriteFile( hCommPort, &c, 1, &dwCount, NULL );

        if ( !bNoLF ) {
            c = 10;
            WriteFile( hCommPort, &c, 1, &dwCount, NULL );
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
// com_writebuf
//

BOOL com_writebuf( HANDLE hCommPort, unsigned char * p, unsigned short cnt )
{
    BOOL rv;
    DWORD dwCount;
    rv = WriteFile( hCommPort, p, cnt, &dwCount, NULL );
    return rv;
}










