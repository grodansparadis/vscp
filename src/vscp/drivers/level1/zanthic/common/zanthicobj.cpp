// zanthicobj.cpp:
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


#include "stdio.h"
#include <windows.h>
#include <winioctl.h>
#include "zanthicobj.h"
#include "dlldrvobj.h"

// Prototypes
#ifdef WIN32
void workThreadTransmit( void *pObject );
void workThreadReceive( void *pObject );
#else
void *workThreadTransmit( void *pObject );
void *workThreadReceive( void *pObject );
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CZanthicObj
//

CZanthicObj::CZanthicObj()
{
    // No device yet
    m_phDeviceHandle = NULL;

    // No filter mask
    m_filter = 0;
    m_mask = 0;

    m_bRun = false;
    m_drvIdx = 0;

#ifdef WIN32

    m_hTreadReceive = 0;
    m_hTreadTransmit = 0;

    // Create the device AND LIST access mutexes
    m_zanthicMutex = CreateMutex( NULL, true, CANAL_DLL_ZANTHICDRV_OBJ_MUTEX );
    m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_ZANTHICDRV_RECEIVE_MUTEX );
    m_transmitMutex = CreateMutex( NULL, true, CANAL_DLL_ZANTHICDRV_TRANSMIT_MUTEX );

#else

    m_flog = NULL;
    pthread_mutex_init( &m_ixxMutex, NULL );
    pthread_mutex_init( &m_receiveMutex, NULL );
    pthread_mutex_init( &m_transmitMutex, NULL );

#endif

    dll_init( &m_transmitList, SORT_NONE );
    dll_init( &m_receiveList, SORT_NONE );
}

//////////////////////////////////////////////////////////////////////
// ~CZanthicObj
//

CZanthicObj::~CZanthicObj()
{
    close();

    LOCK_MUTEX( m_transmitMutex );
    dll_removeAllNodes( &m_transmitList );

    LOCK_MUTEX( m_receiveMutex );
    dll_removeAllNodes( &m_receiveList );


#ifdef WIN32

    if ( NULL != m_zanthicMutex ) CloseHandle( m_zanthicMutex );
    if ( NULL != m_receiveMutex ) CloseHandle( m_receiveMutex );
    if ( NULL != m_transmitMutex ) CloseHandle( m_transmitMutex );

#else

    pthread_mutex_destroy( &m_ixxMutex );
    pthread_mutex_destroy( &m_receiveMutex );
    pthread_mutex_destroy( &m_transmitMutex );

#endif
}





//////////////////////////////////////////////////////////////////////
// open
//
//
// filename
//-----------------------------------------------------------------------------
// Parameters for the driver as a string on the following form
//
// "bus-speed"
//
//
// bus-speed
// =========
// One of the predefined bitrates can be set here
//
//  10 for 10 Kbs
//  20 for 20 Kbs
//	50 for 50 Kbs
//  100 for 100 Kbs
//  125 for 125 Kbs
//  250 for 250 Kbs
//  500 for 500 Kbs
//  800 for 800 Kbs
//  1000 for 1000 Mbs
//
//
//
// flags
//-----------------------------------------------------------------------------
//
// bit 0
// =====
//
//
// bit 2
// =====
//
//
// bit 2
// =====
//
//
// bit 3
// =====
//
//
// bit 4
// =====
//
//

bool CZanthicObj::open( const char *szFileName, unsigned long flags )
{
    const char *p;
    unsigned long busspeed = 125;
    char szDrvParams[ MAX_PATH ];
    char *next_token = NULL;

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



    // Bus-Speed

#ifdef WIN32
    strcpy_s( szDrvParams, MAX_PATH, szFileName );
    p = strtok_s( szDrvParams, ";", &next_token );
#else
    strcpy( szDrvParams, szFileName );
    p = strtok( szDrvParams, ";" );
#endif

    if ( NULL != p ) {
        if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
#ifdef WIN32
            sscanf_s( p + 2, "%x", &busspeed );
#else
            sscanf( p + 2, "%x", &busspeed );
#endif
        }
        else {
            busspeed = atol( p );
        }
    }

    // Try to open the driver i/f
    if ( !openDriver() ) return false;

    //Reset device
    resetDevice();

    // Set bus speed
    if ( ZANTECH_ACK != setDeviceSpeed( (short)busspeed ) ) {
        closeDriver();
        return false;
    }

    // Clear the buffer
    clearReceiveBuffer();

    // Set default filter and mask
    setMask( 0 );

    for ( int i=0; i<6; i++ ) {
        setDeviceFilter( i, 0x80000000 );
    }

    // Run run run .....
    m_bRun = true;

#ifdef WIN32

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
                                        (LPTHREAD_START_ROUTINE) workThreadReceive,
                                        this,
                                        0,
                                        &threadId ) ) ) {
        // Failure
        close();
        return  false;
    }

    // Release the mutex
    UNLOCK_MUTEX( m_zanthicMutex );
    UNLOCK_MUTEX( m_receiveMutex );
    UNLOCK_MUTEX( m_transmitMutex );


#else // LINUX


    pthread_attr_t thread_attr;
    pthread_attr_init( &thread_attr );

    // Create the log write thread.
    if ( pthread_create( &m_threadId,
                                &thread_attr,
                                workThreadTransmit,
                                this ) ) {

        syslog( LOG_ERR, "zanthicdrv: Unable to create zanthicdrv write thread.");
        rv = false;
            fclose( m_flog );
    }


    // Create the log write thread.
    if ( pthread_create( &m_threadId,
                            &thread_attr,
                            workThreadReceive,
                            this ) ) {

        syslog( LOG_ERR, "zanthicdrv: Unable to create zanthicdrv receive thread.");
        rv = false;
        fclose( m_flog );
    }

    // We are open
    m_bOpen = true;

    // Release the mutex
    pthread_mutex_unlock( &m_ixxMutex );

#endif

    return true;
}


//////////////////////////////////////////////////////////////////////
// close
//

bool CZanthicObj::close( void )
{
    // Do nothing if already terminated
    if ( !m_bRun ) return false;

    m_bRun = false;

    // Close the device
    closeDriver();

    UNLOCK_MUTEX( m_zanthicMutex );
        LOCK_MUTEX( m_zanthicMutex );


    // terminate the worker thread
#ifdef WIN32
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

#else
    int *trv;
    pthread_join( m_threadIdReceive, (void **)&trv );
    pthread_join( m_threadIdTransmit, (void **)&trv );
    pthread_mutex_destroy( &m_ixxMutex );

#endif

    return true;
}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CZanthicObj::doFilter( canalMsg *pcanalMsg )
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

bool CZanthicObj::setFilter( unsigned long filter )
{
    if ( ZANTECH_ACK != setDeviceFilter( 0, filter ) ) return false;
    return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CZanthicObj::setMask( unsigned long mask )
{
    if ( ZANTECH_ACK != setDeviceMask( mask, 7 ) ) return false;
    if ( ZANTECH_ACK != setDeviceMask( mask, 0x87 ) ) return false;
    return true;
}



//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CZanthicObj::writeMsg( canalMsg *pMsg )
{
    bool rv = false;

    if ( NULL != pMsg ) {

        // Filter
        //if ( !doFilter( pMsg ) ) return true;

        // Must be room for the message
        if ( m_transmitList.nCount < ZANTHIC_MAX_SNDMSG ) {

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

bool CZanthicObj::readMsg( canalMsg *pMsg )
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

int CZanthicObj::dataAvailable( void )
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

bool CZanthicObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{
    // Must be a valid pointer
    if ( NULL == pCanalStatistics ) return false;

    memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CZanthicObj::getStatus( PCANALSTATUS pCanalStatus )
{
    // Must be a valid pointer
    if ( NULL == pCanalStatus ) return false;

    // Nothing at the moment
    pCanalStatus->channel_status = 0;

    return true;
}


//-----------------------------------------------------------------------------
//
//							ZANTHEC Control Stuff
//
//-----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
//	resetDevice
//
short CZanthicObj::getDeviceInfo( uint8_t *Version,
                                    uint8_t *Feature,
                                    char *pManufact,
                                    uint8_t *CANCont )
{
    uint8_t outbuffer[ 3 ];
    uint8_t inbuffer[ 65 ];
    short result,pntr=4;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    outbuffer[ 0 ] = 0;
    outbuffer[ 1 ] = GETSTATSREQ;  // command

    result = doCommand( outbuffer, 2, inbuffer );
    if (result < 0 ) return result; // return error

    if ( GETSTATSRESP == inbuffer[0] )  {

        *Version++ = inbuffer[1];
        *Version++ = inbuffer[2];

        *Feature = inbuffer[3];

#ifdef WIN32
        strcpy_s( pManufact, sizeof( pManufact ), (char*)inbuffer + 4 );
#else
        strcpy( pManufact, (char*)inbuffer + 4 );
#endif

        *CANCont++ = 1;              // hardcoded for one device for now
        *CANCont++ = *(inbuffer + 4 + strlen( pManufact ) );
        return(1);
    }

    return ZANTHIC_ERROR_RESPONSE; //improper response from node
}


///////////////////////////////////////////////////////////////////////////////
//	resetDevice
//

short CZanthicObj::resetDevice( void )
{
    uint8_t outbuffer[3];
    uint8_t inbuffer[65];
    short result;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    outbuffer[0] = 0;
    outbuffer[1] = INITREQ;  // command

    result = doCommand( outbuffer, 2, inbuffer );
    if ( result < 0 ) return( result ); // return error

    if ( INITRESP == inbuffer[0] ) {
        return( 1 );
    }

    return ZANTHIC_ERROR_RESPONSE; //improper response from node
}


///////////////////////////////////////////////////////////////////////////////
//	setDeviceSpeed
//

short CZanthicObj::setDeviceSpeed( short speed )
{
    uint8_t outbuffer[ 4 ];
    uint8_t inbuffer[ 65 ];
    short result;
    uint8_t bval = 10;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    switch ( speed ) {

        case 10:
            bval = 0;
            break;

        case 20:
            bval = 1;
            break;

        case 100:
            bval = 3;
            break;

        case 125:
            bval = 4;
            break;

        case 250:
            bval = 5;
            break;

        case 500:
            bval = 6;
            break;

        case 800:
            bval = 7;
            break;

        case 1000:
            bval = 8;
            break;

        default:
            return ZANTHIC_ERROR_PARAM; // improper calling parameter
    }

    outbuffer[0] = 0;
    outbuffer[1] = SETBAUDREQ;  // command
    outbuffer[2] = bval;

    result = doCommand( outbuffer, 3, inbuffer );
    if (result<0) return(result); // return error

    if ( SETBAUDRESP != inbuffer[0] ) {
        return ZANTHIC_ERROR_RESPONSE; //improper response from node
    }

    return( ZANTECH_ACK );
}


///////////////////////////////////////////////////////////////////////////////
//	SendMsgToDevice
//

short CZanthicObj::sendMsgToDevice( uint32_t ID, uint8_t CB1, uint8_t CB2, uint8_t *databytes )
{
    uint8_t buffer[16];
    uint8_t inbuffer[65];
    uint8_t c,numbytes;
    short result;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    buffer[ 0 ] = 0;
    buffer[ 1 ] = SENDCANREQ;  // command
    buffer[ 2 ] = ( uint8_t )( ID >> 24 );
    buffer[ 3 ] = ( uint8_t )( ID >> 16 );
    buffer[ 4 ] = ( uint8_t )( ID >> 8 );
    buffer[ 5 ] = ( uint8_t )ID;
    buffer[ 6 ] = CB1;
    buffer[ 7 ] = CB2;
    numbytes = CB2 & 0x0F; // get number of bytes

    if (numbytes>8) return ZANTHIC_ERROR_PARAM;

    for ( c=0; c<numbytes; c++ ) {
        buffer[ 8 + c ] = *( databytes + c );
    }

    result = doCommand( buffer, numbytes + 8, inbuffer );
    if ( result < 0 ) return( result ); // return error

    if ( SENDCANRESP != inbuffer[0] ) return ZANTHIC_ERROR_RESPONSE; //improper response from node
    if ( ( SENDCANRESP == inbuffer[0] ) && (inbuffer[1] == ZANTECH_ACK ) ) return ZANTECH_ACK; // success

    // else an error occurred
    *databytes++ = inbuffer[ 2 ]; // first error code
    *databytes++ = inbuffer[ 3 ]; // second error code

    return ZANTECH_NACK;
}


///////////////////////////////////////////////////////////////////////////////
//	readReg
//

short CZanthicObj::readReg( uint8_t Address, uint8_t Numbytes, uint8_t *buffer)
{
    uint8_t outbuffer[4];
    uint8_t inbuffer[65];
    short int result,c;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    if ( Numbytes > 60 ) return ZANTHIC_ERROR_PARAM;  // too many

    outbuffer[0] = 0;
    outbuffer[1] = READREGREQ;  // command
    outbuffer[2] = Address;
    outbuffer[3] = Numbytes;

    result = doCommand( outbuffer, 4, inbuffer );
    if ( result < 0 ) return( result ); // return error

    if ( READREGRESP == inbuffer[0] )  {
        if ( inbuffer[ 1 ] == ZANTECH_ACK ) {
            for ( c=2; c<(2+Numbytes); c++ ) {
                *buffer++=inbuffer[c]; // load up data
            }
            return(Numbytes);
        }
        return ZANTECH_NACK;
    }
    else {
        return ZANTHIC_ERROR_RESPONSE; //improper response from node
    }
}


///////////////////////////////////////////////////////////////////////////////
//	writeReg
//

short CZanthicObj::writeReg( uint8_t Address, uint8_t Numbytes, uint8_t *buffer )
{
    uint8_t outbuffer[65];
    uint8_t inbuffer[65];
    short result,c;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    if (Numbytes>60) return ZANTHIC_ERROR_PARAM;  // too many
    outbuffer[0] = 0;
    outbuffer[1] = WRITEREGREQ;  // command
    outbuffer[2] = Address;
    outbuffer[3] = Numbytes;

    for (c=0;c<Numbytes;c++)
        outbuffer[4+c]=*buffer++;

    result = doCommand( outbuffer, 4 + Numbytes, inbuffer );
    if ( result < 0 ) return( result ); // return error
    if ( inbuffer[0] == WRITEREGRESP ) {
        if ( ZANTECH_ACK == inbuffer[1] ) return ZANTECH_ACK;
        return ZANTECH_NACK;
    }

    return ZANTHIC_ERROR_RESPONSE; //improper response from node
}


///////////////////////////////////////////////////////////////////////////////
//	getMsgFromDevice
//

short CZanthicObj::getMsgFromDevice( void )
{
    uint8_t outbuffer[3];
    uint8_t inbuffer[65];
    short result, cntMsg, sizeMsg;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    outbuffer[0] = 0;
    outbuffer[1] = GETCANMESREQ;  // command

    result = doCommand( outbuffer, 2, inbuffer );
    if ( result < 0 ) return( result );			// return error

    if ( GETCANMESRESP != inbuffer[ 0 ] ) return ZANTHIC_ERROR_RESPONSE; //improper response from node
    if ( inbuffer[ 1 ] == 0) return( 0 ); // no new messages (fix for proper form!)

    uint8_t *p = inbuffer + 3;	// Point at firts message
    cntMsg = inbuffer[ 2 ]; // number of packets in the buffer

    for ( int i=0; i<cntMsg; i++ ) {

        sizeMsg = *p; 	// Packetsize

        // [0] = number of bytes in message
        // [1] = Control Byte 1, upper 4 bits=message object, lower 4=number of bytes of data
        // [2] = Control Byte 2
        // bit 0 Filter 0
        // bit 1 Filter 1 (applies to Rec Buffer 1 only)
        // bit 2 Filter 2 (applies to Rec Buffer 1 only)
        // bit 3 RXRTR (taken from bit 3, RXBxCTRL)
        // bit 4 SRR (taken from bit 4, RXBnSIDL)
        // bit 5 Reserved
        // bit 6 RTR (taken from bit 6, RXBnDLC)
        // bit 7 IDE (taken from bit 3, RXBnSIDL)
        // [3] = CAN ID MSB
        // [4] = CAN ID
        // [5] = CAN ID
        // [6] = CAN ID LSB
        // [7+] = data bytes as required
        if (  m_receiveList.nCount < ZANTHIC_MAX_RCVMSG ) {

            PCANALMSG pMsg	= new canalMsg;
            pMsg->flags = 0;

            if ( NULL != pMsg ) {

                dllnode *pNode = new dllnode;
                if ( NULL != pNode ) {

                    pMsg->timestamp = 0;
                    pMsg->id = ( p[ 3 ] << 24 ) + ( p[ 4 ]<<16 ) + ( p[ 5 ]<<8 ) + p[ 6 ];

                    // Check for extended message
                    if ( 0x80 & p[ 2 ] ) {
                        pMsg->flags |= 	CANAL_IDFLAG_EXTENDED;
                    }

                    // Check for RTR message
                    if ( 0x40 & p[ 2 ] ) {
                        pMsg->flags |= 	CANAL_IDFLAG_RTR;
                    }

                    pMsg->sizeData = ( *(p+1) & 0x0f );
                    memcpy( pMsg->data, p + 7, pMsg->sizeData );

                    pNode->pObject = pMsg;
                    LOCK_MUTEX( m_receiveMutex );
                    dll_addNode( &m_receiveList, pNode );
                    UNLOCK_MUTEX( m_receiveMutex );

                    // Update statistics
                    m_stat.cntReceiveData += pMsg->sizeData;
                    m_stat.cntReceiveFrames += 1;

                }
                else {
                    delete pMsg;
                }
            }
        }
        else {

            // Full buffer
            m_stat.cntOverruns++;

        }

        p += sizeMsg;
    }

    return( 1 );
}


///////////////////////////////////////////////////////////////////////////////
// getMsgCntFromDevice
//
//

short CZanthicObj::getMsgCntFromDevice( void )
{
    uint8_t outbuffer[65];
    uint8_t inbuffer[65];
    short result;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    outbuffer[ 0 ] = 0;
    outbuffer[ 1 ] = GETNUMMESREQ;  // command

    result = doCommand( outbuffer, 2, inbuffer );
    if ( result < 0 ) return( result ); // return error

    if ( GETNUMMESRESP == inbuffer[ 0 ] ) {
        return ( ( inbuffer[ 1 ] * 0x100 ) + inbuffer[ 2 ] );
    }
    else {
        return ZANTHIC_ERROR_RESPONSE; //improper response from node
    }
}


///////////////////////////////////////////////////////////////////////////////
// ClearMsg
//
//

short CZanthicObj::clearReceiveBuffer( void )
{
    uint8_t outbuffer[ 3 ];
    uint8_t inbuffer[ 65 ];
    short result;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    outbuffer[0] = 0;
    outbuffer[1] = CLEARMESREQ;  // command

    result = doCommand( outbuffer, 2, inbuffer );
    if ( result < 0 )  return( result ); // return error

    if ( CLEARMESRESP == inbuffer[ 0 ] ) {
        return ( 1 );
    }
    else {
        return ZANTHIC_ERROR_RESPONSE; //improper response from node
    }
}


///////////////////////////////////////////////////////////////////////////////
// setDeviceFilter
//
//

short CZanthicObj::setDeviceFilter( uint8_t nFilter, uint32_t filter )
{
    uint8_t outbuffer[ 7 ];
    uint8_t inbuffer[ 65 ];
    short result;
    short mask = 1;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    if ( nFilter > 5 ) return ZANTHIC_ERROR_PARAM; //out of range

    outbuffer[0] = 0;
    outbuffer[1] = SETFILTERREQ;  // command

    outbuffer[2] = ( mask << nFilter ); // 0=1,1=2,2=4 etc
    outbuffer[3] = (unsigned char)( filter >> 24 );
    outbuffer[4] = (unsigned char)( filter >> 16 );
    outbuffer[5] = (unsigned char)( filter >> 8 );
    outbuffer[6] = (unsigned char)filter;

    result = doCommand( outbuffer, sizeof( outbuffer), inbuffer );
    if ( result < 0 ) return( result ); // return error

    if ( SETFILTERRESP == inbuffer[ 0 ] ) {
        return ZANTECH_ACK;
    }
    else {
        return ZANTHIC_ERROR_RESPONSE;	//improper response from node
    }
}


///////////////////////////////////////////////////////////////////////////////
//	setDeviceMask
//

short CZanthicObj::setDeviceMask( uint32_t mask, uint8_t cb1 )
{
    uint8_t outbuffer[8];
    uint8_t inbuffer[65];
    short result;

    // Must be open
    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    outbuffer[0] = 0;
    outbuffer[1] = RECCANREQ;  // command
    outbuffer[2] = ( uint8_t )( mask >> 24);
    outbuffer[3] = ( uint8_t )( mask >> 16);
    outbuffer[4] = ( uint8_t )( mask >> 8);
    outbuffer[5] = ( uint8_t )mask;
    outbuffer[6] = cb1;

    result = doCommand( outbuffer, 7, inbuffer );
    if ( result < 0 ) return( result );		// return error

    if ( RECCANRESP != inbuffer[ 0 ] ) return ZANTHIC_ERROR_RESPONSE; //improper response from node
    return ZANTECH_ACK;
}


///////////////////////////////////////////////////////////////////////////////
// Command
//
//

short CZanthicObj::doCommand( uint8_t *outbuffer, uint8_t numbytes, uint8_t *inbuffer)
{
    WORD inPacketSize = 64;
    DWORD nBytes = 0;
    BULK_TRANSFER_CONTROL bulkControl;
    BOOLEAN bResult= FALSE;
    WORD outPacketSize=numbytes;
    bulkControl.pipeNum = 1;  // hardcoded bulk out to pipe 1

    if ( NULL == m_phDeviceHandle ) return ZANTHIC_ERROR_GENERAL;

    // Perform the BULK OUT
    if ( !( bResult = DeviceIoControl( m_phDeviceHandle,
                                        IOCTL_EZUSB_BULK_WRITE,
                                        &bulkControl,
                                        sizeof( BULK_TRANSFER_CONTROL ),
                                        outbuffer,
                                        outPacketSize,
                                        &nBytes,
                                        NULL ) ) ) {
        CloseHandle( m_phDeviceHandle );
        return ZANTHIC_ERROR_BULK_WRITE;
    }


    bulkControl.pipeNum = 8;  // hardcoded bulk in to pipe 8

    // Perform the BULK IN
    if ( !( bResult = DeviceIoControl( m_phDeviceHandle,
                                IOCTL_EZUSB_BULK_READ,
                                &bulkControl,
                                sizeof( BULK_TRANSFER_CONTROL ),
                                inbuffer,
                                inPacketSize,
                                &nBytes,
                                NULL ) ) ) {

        //CloseHandle( m_phDeviceHandle );
        return ZANTHIC_ERROR_BULK_READ;
    }


    return( ( short )nBytes );
}


///////////////////////////////////////////////////////////////////////////////
// closeDriver
//
//

void CZanthicObj::closeDriver( void )
{
    CloseHandle( m_phDeviceHandle );
    m_phDeviceHandle = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// openDriver
//
//

bool CZanthicObj::openDriver( void )
{
    char completeDeviceName[ MAX_PATH ] = "";
    char pcMsg[64] = "";
    char strng[] ="Ezusb- ";

    // Check for a valid index
    if ( m_drvIdx > 9 ) return false;

    strng[ 6 ] = 0x30 + m_drvIdx;  // 0-9

#ifdef WIN32
    strcat_s( completeDeviceName, sizeof( completeDeviceName ), "\\\\.\\" );
    strcat_s( completeDeviceName, sizeof( completeDeviceName ), strng  );
#else
    strcat (completeDeviceName, "\\\\.\\" );
    strcat (completeDeviceName,	strng );
#endif

    if ( INVALID_HANDLE_VALUE ==
            ( m_phDeviceHandle = CreateFile( completeDeviceName,
                                                GENERIC_WRITE,
                                                FILE_SHARE_WRITE,
                                                NULL,
                                                OPEN_EXISTING,
                                                0,
                                                NULL ) ) ) {
        return false;
    }

   return true;
}



//-----------------------------------------------------------------------------
//
//							      Worker threads
//
//-----------------------------------------------------------------------------



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
#else
    int rv = 0;
#endif

    CZanthicObj * pobj = ( CZanthicObj *)pObject;
    if ( NULL == pobj ) {
#ifdef WIN32
        ExitThread( errorCode ); // Fail
#else
        pthread_exit( &rv );
#endif
    }

    uint32_t id;
    uint8_t cb1,cb2;	// Transmit control bytes

    while ( pobj->m_bRun ) {

        LOCK_MUTEX( pobj->m_zanthicMutex );

        // Noting to do if we should end...
        if ( !pobj->m_bRun ) continue;

        // Is there something to transmit
        while ( ( NULL != pobj->m_transmitList.pHead ) &&
                ( NULL != pobj->m_transmitList.pHead->pObject ) ) {

            canalMsg msg;
            memcpy( &msg, pobj->m_transmitList.pHead->pObject, sizeof( canalMsg ) );
            LOCK_MUTEX( pobj->m_transmitMutex );
            dll_removeNode( &pobj->m_transmitList, pobj->m_transmitList.pHead );
            UNLOCK_MUTEX( pobj->m_transmitMutex );

            id = msg.id;

            // Mark Extended if needed
            if ( msg.flags & CANAL_IDFLAG_EXTENDED ) {
                id |= 0x80000000;
            }

            // Mark RTS if needed
            if ( msg.flags & CANAL_IDFLAG_RTR ) {
                id |= 0x40000000;
            }

            cb1 = 0;
            cb2 = msg.sizeData;
            if ( ZANTECH_ACK == pobj->sendMsgToDevice( id, cb1, cb2, msg.data ) ) {

                // Message sent successfully
                // Update statistics
                pobj->m_stat.cntTransmitData += msg.sizeData;
                pobj->m_stat.cntTransmitFrames += 1;

            }
            else {

                // Failed - put message back in queue front
                PCANALMSG pMsg	= new canalMsg;
                if ( NULL != pMsg ) {

                    // Copy in data
                    memcpy ( pMsg, &msg, sizeof( canalMsg ) );

                    dllnode *pNode = new dllnode;
                    if ( NULL != pNode ) {

                        pNode->pObject = pMsg;
                        LOCK_MUTEX( pobj->m_transmitMutex );
                        dll_addNodeHead( &pobj->m_transmitList, pNode );
                        UNLOCK_MUTEX( pobj->m_transmitMutex );

                    }
                    else {
                        delete pMsg;
                    }
                }
            }
        } // while data

        // No data to write
        UNLOCK_MUTEX( pobj->m_zanthicMutex );
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
void workThreadReceive( void *pObject )
#else
void *workThreadReceive( void *pObject )
#endif
{
    int cntMsg;

#ifdef WIN32
    DWORD errorCode = 0;
#else
    int rv = 0;
#endif

    CZanthicObj * pobj = ( CZanthicObj *)pObject;
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

        LOCK_MUTEX( pobj->m_zanthicMutex );
        if ( ( cntMsg = pobj->getMsgCntFromDevice() ) > 0 ) {

            // There are messages to fetch
            pobj->getMsgFromDevice();

        }

        UNLOCK_MUTEX( pobj->m_zanthicMutex );
        SLEEP( 1 );

    } // while


#ifdef WIN32
    ExitThread( errorCode );
#else
    pthread_exit( &rv );
#endif

}
