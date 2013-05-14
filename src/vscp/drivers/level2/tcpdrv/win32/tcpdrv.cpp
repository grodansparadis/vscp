//
// tcpdrv.cpp : Defines the entry point for the DLL application.
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

#define __USE_W32_SOCKETS
#include <winsock2.h> 
#include <pcap.h>

//#define WIN32_LEAN_AND_MEAN
#include "wx/wxprec.h"
#include "wx/wx.h"
//#include <windows.h>
//#include <stdlib.h>

#include "../common/dlldrvobj.h"


#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
//#include "winsock.h"
//#include <winsock2.h>

#endif

#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include <wx/utils.h>



#include "../../../../common/vscphelper.h"
#include "../../../../common/vscptcpif.h"
#include "tcpdrv.h"

static HANDLE hThisInstDll = NULL;
static CDllDrvObj *theApp = NULL;

#pragma comment(lib, "Irprops.lib")



///////////////////////////////////////////////////////////////////////////////
// DllMain
//

BOOL APIENTRY DllMain( HANDLE hInstDll, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch( ul_reason_for_call ) {

	    case DLL_PROCESS_ATTACH:
            ::wxInitialize();
			hThisInstDll = hInstDll;
			theApp = new CDllDrvObj();
			theApp->InitInstance();
			break;

		case DLL_THREAD_ATTACH:
			if ( NULL == theApp ) delete theApp;
			break;

		case DLL_THREAD_DETACH:
 			break;

		case DLL_PROCESS_DETACH:
			::wxUninitialize();
 			break;
   }

	return TRUE;

}




///////////////////////////////////////////////////////////////////////////////
//                         V S C P   D R I V E R -  A P I
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// VSCPOpen
//

#ifdef WIN32
extern "C" long WINAPI EXPORT VSCPOpen( const char *pUsername,
                                            const char *pPassword,
                                            const char *pHost,
                                            short port,
                                            const char *pPrefix,
                                            const char *pParameter, 
                                            unsigned long flags )
#else
extern "C" long VSCPOpen( const char *pUsername,
                                            const char *pPassword,
                                            const char *pHost,
                                            short port,
                                            const char *pPrefix,
                                            const char *pParameter, 
                                            unsigned long flags )
#endif
{
	long h = 0;
	
	Ctcpdrv *pdrvObj = new Ctcpdrv();
	if ( NULL != pdrvObj ) {

		if ( pdrvObj->open( pUsername, pPassword, pHost, port, pPrefix, pParameter, flags ) ){

			if ( !( h = theApp->addDriverObject( pdrvObj ) ) ) {
				delete pdrvObj;
			}

		}
		else {
			delete pdrvObj;
		}

	}
 
	return h;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPClose
// 

#ifdef WIN32
extern "C" int  WINAPI EXPORT VSCPClose( long handle )
#else
extern "C" int VSCPClose( long handle )
#endif
{
	int rv = 0;

	Ctcpdrv *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	pdrvObj->close();
	theApp->removeDriverObject( handle );
	rv = 1;
	return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPGetLevel
// 

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT VSCPGetLevel( void )
#else
extern "C" unsigned long VSCPGetLevel( void )
#endif
{
	return CANAL_LEVEL_USES_TCPIP;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetDllVersion
//

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT VSCPGetDllVersion( void )
#else
extern "C" unsigned long VSCPGetDllVersion( void )
#endif
{
	return DLL_VERSION;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetVendorString
//

#ifdef WIN32
extern "C" const char * WINAPI EXPORT VSCPGetVendorString( void )
#else
extern "C" const char * VSCPGetVendorString( void )
#endif
{
	return VSCP_DLL_VENDOR;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetDriverInfo
//

#ifdef WIN32
extern "C" const char * WINAPI EXPORT VSCPGetDriverInfo( void )
#else
extern "C" const char * VSCPGetDriverInfo( void )
#endif
{
	return VSCP_LOGGER_DRIVERINFO;
}



///////////////////////////////////////////////////////////////////////////////
//                            T H E  C O D E
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// Constructor
//

Ctcpdrv::Ctcpdrv()
{
    m_bQuit = false;
    
    m_pthreadWorkTx = NULL;
    m_pthreadWorkRx = NULL;
    
    memset( m_localMac, 0, 16 );

    // Initialize tx channel GUID
    m_localGUIDtx.clear();
    m_localGUIDtx.setAt( 0, 0xff );
    m_localGUIDtx.setAt( 1, 0xff );
    m_localGUIDtx.setAt( 2, 0xff );
    m_localGUIDtx.setAt( 3, 0xff );
    m_localGUIDtx.setAt( 4, 0xff );
    m_localGUIDtx.setAt( 5, 0xff );
    m_localGUIDtx.setAt( 6, 0xff );
    m_localGUIDtx.setAt( 7, 0xfe );
    m_localGUIDtx.setAt( 14, 0x00 );
    m_localGUIDtx.setAt( 15, 0x00 );

    m_localGUIDrx.clear();
    m_localGUIDrx.setAt( 0, 0xff );
    m_localGUIDrx.setAt( 1, 0xff );
    m_localGUIDrx.setAt( 2, 0xff );
    m_localGUIDrx.setAt( 3, 0xff );
    m_localGUIDrx.setAt( 4, 0xff );
    m_localGUIDrx.setAt( 5, 0xff );
    m_localGUIDrx.setAt( 6, 0xff );
    m_localGUIDrx.setAt( 7, 0xfe );
    m_localGUIDrx.setAt( 14, 0x00 );
    m_localGUIDrx.setAt( 15, 0x01 );

    ::wxInitialize();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

Ctcpdrv::~Ctcpdrv()
{
    close();
    ::wxUninitialize();
}


///////////////////////////////////////////////////////////////////////////////
// open
//

bool Ctcpdrv::open( const char *pUsername,
                            const char *pPassword,
                            const char *pHost,
                            short port,
                            const char *pPrefix,
                            const char *pConfig, 
                            unsigned long flags )
{
    bool rv = true;
    m_flags = flags;
    wxString wxstr = pConfig;

    m_username = pUsername;
    m_password = pPassword;
    m_host = pHost;
    m_port = port;
    m_prefix = pPrefix;

    // Parse the configuration string. It should
    // have the following form
    // username;password;host;prefix;port;filename
    wxStringTokenizer tkz( pConfig, ";\n" );

    // Interface
    if ( tkz.HasMoreTokens() ) {
         m_interface = tkz.GetNextToken();     
    }

    // Local Mac
    wxString localMac;
    if ( tkz.HasMoreTokens() ) {
        localMac = tkz.GetNextToken();
        localMac.MakeUpper();
        wxStringTokenizer tkzmac( localMac, ":\n" );
        for ( int i = 0; i < 6; i++ ) {
            if ( !tkzmac.HasMoreTokens() ) break;
            wxString str = _("0X") + tkzmac.GetNextToken();
            m_localMac[ i ] = readStringValue( str );
            m_localGUIDtx.setAt( ( 9  + i ), m_localMac[ i ] );
            m_localGUIDrx.setAt( ( 9  + i ), m_localMac[ i ] );
        }
    }

    // start the workerthreads
    m_pthreadWorkTx = new CtcpdrvTxTread();
    if ( NULL != m_pthreadWorkTx ) {
        m_pthreadWorkTx->m_pobj = this;
        m_pthreadWorkTx->Create();
        m_pthreadWorkTx->Run();
    }
    else {
        rv = false;
    }

    wxSleep( 1 );

    m_pthreadWorkRx = new CtcpdrvRxTread();
    if ( NULL != m_pthreadWorkRx ) {
        m_pthreadWorkRx->m_pobj = this;
        m_pthreadWorkRx->Create();
        m_pthreadWorkRx->Run();
    }
    else {
        rv = false;
    }


	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// close
//

void Ctcpdrv::close( void )
{
	// Do nothing if already terminated
	if ( m_bQuit ) return;
	
	m_bQuit = true;     // terminate the thread
    wxSleep( 1 );       // Give the thread some time to terminate
}



///////////////////////////////////////////////////////////////////////////////
//                           W O R K E R   T H R E A D S
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//                            CtcpdrvTxTread
///////////////////////////////////////////////////////////////////////////////



CtcpdrvTxTread::CtcpdrvTxTread()
{
    
}


CtcpdrvTxTread::~CtcpdrvTxTread()
{

}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *CtcpdrvTxTread::Entry()
{
    pcap_t *fp;
	char errbuf[ PCAP_ERRBUF_SIZE ];
	//uint8_t packet[ 512 ];

    // First log on to the host and get configuration 
    // variables
    
    if ( m_srv.doCmdOpen( m_pobj->m_host,
                                m_pobj->m_port,
                                m_pobj->m_username,
                                m_pobj->m_password ) <= 0 ) {
        return NULL;
    }

    // Find the channel id
    m_srv.doCmdGetChannelID( &m_pobj->m_ChannelIDtx );

    // It is possible that there is configuration data the server holds 
    // that we need to read in. 
    // We look for 
    //      prefix_interface Communication interface to work on
    //      prefix_localmac MAC address to use for outgoing packets
    //      prefix_filter to find a filter. A string is expected.
    //      prefix_mask to find a mask. A string is expected.

    // Interface
    wxString varInterface;
    if ( m_srv.getVariableString( m_pobj->m_prefix + _T("_interface"), &varInterface ) ) {
        m_pobj->m_interface = varInterface;     
    }

    wxString varLocalMac;
    if ( m_srv.getVariableString( m_pobj->m_prefix + _T("_localmac"), &varLocalMac ) ) {
        varLocalMac.MakeUpper();
        wxStringTokenizer tkz( varLocalMac, ":\n" );
        for ( int i = 0; i < 6; i++ ) {
            if ( tkz.HasMoreTokens() ) break;
            wxString str = _("0X") + tkz.GetNextToken();
            m_pobj->m_localMac[ i ] = readStringValue( str );
            m_pobj->m_localGUIDtx.setAt( ( 9  + i ), m_pobj->m_localMac[ i ] );
            m_pobj->m_localGUIDrx.setAt( ( 9  + i ), m_pobj->m_localMac[ i ] );
        }
    }

    // We want to use our own Ethernet based GUID for this interface
    wxString strGUID;
    m_pobj->m_localGUIDtx.toString( strGUID );
    m_srv.doCmdSetGUID( (char *)strGUID.ToAscii() );

    // Open the adapter 
    if ( (fp = pcap_open_live( m_pobj->m_interface.ToAscii(),  // name of the device
							    65536,			// portion of the packet to capture. It doesn't matter in this case 
							    1,				// promiscuous mode (nonzero means promiscuous)
							    1000,			// read timeout
							    errbuf			// error buffer
							 ) ) == NULL ) {
		//fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", argv[1]);
		return NULL;
	}

    int rv;
    struct pcap_pkthdr *header;
    const u_char *pkt_data;

	while ( !TestDestroy() && 
                !m_pobj->m_bQuit &&
                ( rv = pcap_next_ex( fp, &header, &pkt_data ) ) >= 0 ) {

        // Check for timeout            
        if ( 0 == rv ) continue;

        // Check if this is VSCP
        if ( ( 0x25 == pkt_data[ 12 ] ) &&  
                ( 0x7e == pkt_data[ 13 ] ) ) {
            
            // We have a packet - send it as a VSCP event    
            vscpEventEx event;

            event.head = pkt_data[ 15 ] & 0xe0;     // Priority

            event.GUID[ 0 ] = 0xff;			        // Ethernet predefined  GUID
            event.GUID[ 1 ] = 0xff;
            event.GUID[ 2 ] = 0xff;
            event.GUID[ 3 ] = 0xff;
            event.GUID[ 4 ] = 0xff;
            event.GUID[ 5 ] = 0xff;
            event.GUID[ 6 ] = 0xff;
            event.GUID[ 7 ] = 0xfe;
            event.GUID[ 8 ] = pkt_data[ 6 ]; 	    // Source MAC address
            event.GUID[ 9 ] = pkt_data[ 7 ];
            event.GUID[ 10 ] = pkt_data[ 8 ];
            event.GUID[ 11 ] = pkt_data[ 9 ];
            event.GUID[ 12 ] = pkt_data[ 10 ];
            event.GUID[ 13 ] = pkt_data[ 11 ];
            event.GUID[ 14 ] = pkt_data[ 19 ];      // Device sub address
            event.GUID[ 15 ] = pkt_data[ 20 ];
           
            event.timestamp = ( pkt_data[ 21 ] << 24 ) + 
                              ( pkt_data[ 22 ] << 16 ) +
                              ( pkt_data[ 23 ] << 8 ) +
                                pkt_data[ 24 ] ;

            event.obid = ( pkt_data[ 25 ] << 24 ) + 
                         ( pkt_data[ 26 ] << 16 ) +
                         ( pkt_data[ 27 ] << 8 ) +
                           pkt_data[ 28 ];
            
            event.vscp_class = ( pkt_data[ 29 ] << 8 ) +
                                 pkt_data[ 30 ];

            event.vscp_type = ( pkt_data[ 31 ] << 8 ) +
                                pkt_data[ 32 ];

            event.sizeData = ( pkt_data[ 33 ] << 8 ) +
                                pkt_data[ 34 ];

            // If the packet is smaller then the set datasize just 
            // disregard it
            if ( ( event.sizeData + 35 ) > (uint16_t)header->len ) continue;

            for ( int i=0; i<event.sizeData; i++ ) {
                event.data[ i ] = pkt_data[ 35 + i ];
            }

            m_srv.doCmdSendEx( &event );    // Send the event

        }


    } // work loop   
 
    // Close listner
    pcap_close( fp ); 

    // Close the channel
    m_srv.doCmdClose();

    return NULL;  
}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void CtcpdrvTxTread::OnExit()
{
    
}




///////////////////////////////////////////////////////////////////////////////
//                            CtcpdrvRxTread
///////////////////////////////////////////////////////////////////////////////





CtcpdrvRxTread::CtcpdrvRxTread()
{
    
}


CtcpdrvRxTread::~CtcpdrvRxTread()
{

}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *CtcpdrvRxTread::Entry()
{
    pcap_t *fp;
	char errbuf[ PCAP_ERRBUF_SIZE ];
	uint8_t packet[ 512 ];

    // First log on to the host and get configuration 
    // variables
    
    if ( m_srv.doCmdOpen( m_pobj->m_host,
                                m_pobj->m_port,
                                m_pobj->m_username,
                                m_pobj->m_password ) <= 0 ) {
        return NULL;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srv.doCmdGetChannelID( &ChannelID );

    // We want to use our own Ethernet based  GUID for this interface
    wxString strGUID;
    m_pobj->m_localGUIDrx.toString( strGUID );
    m_srv.doCmdSetGUID( (char *)strGUID.ToAscii() );

    // Open the adapter 
    if ( (fp = pcap_open_live( m_pobj->m_interface.ToAscii(),  // name of the device
							    65536,			// portion of the packet to capture. It doesn't matter in this case 
							    1,				// promiscuous mode (nonzero means promiscuous)
							    1000,			// read timeout
							    errbuf			// error buffer
							 ) ) == NULL ) {
		//fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", argv[1]);
		return NULL;
	}


    // Enter receive loop to start to log events
	m_srv.doCmdEnterReceiveLoop();

    int rv;
    vscpEvent event;
	while ( !TestDestroy() && !m_pobj->m_bQuit ) {

        if ( CANAL_ERROR_SUCCESS == 
            ( rv = m_srv.doCmdBlockReceive( &event, 1000 ) ) ) {

            // As we are on a different VSCP interface we need to filter the events we sent out 
            // ourselves.
            if ( m_pobj->m_ChannelIDtx == event.obid ) {
                continue;
            }
          
            // Set mac destination to broadcast ff:ff:ff:ff:ff:ff 
	        packet[ 0 ] = 0xff;
	        packet[ 1 ] = 0xff;
	        packet[ 2 ] = 0xff;
	        packet[ 3 ] = 0xff;
	        packet[ 4 ] = 0xff;
	        packet[ 5 ] = 0xff;
	
	        // set mac source to configured value - 6..11
            memcpy( packet + 6, m_pobj->m_localMac, 6 );
	  
            // Set the type - always 0x2574 (9598)
            packet[ 12 ] = 0x25; 
            packet[ 13 ] = 0x7e;

            // rawEthernet frame version
            packet[ 14 ] = 0x00;

            // Head
            packet[ 15 ] = ( event.head & VSCP_HEADER_PRIORITY_MASK ); 
            packet[ 16 ] = 0x00;
            packet[ 17 ] = 0x00;
            packet[ 18 ] = 0x00;    // LSB

            // VSCP sub source address For this interface it's 0x0000
            packet[ 19 ] = 0x00;
            packet[ 20 ] = 0x00;

            // Timestamp
            uint32_t timestamp = wxINT32_SWAP_ON_LE( event.timestamp );
            packet[ 21 ] = ( timestamp & 0xff000000 ) >> 24;
            packet[ 22 ] = ( timestamp & 0x00ff0000 ) >> 16;
            packet[ 23 ] = ( timestamp & 0x0000ff00 ) >> 8;
            packet[ 24 ] = ( timestamp & 0x000000ff );

            // obid
            uint32_t obid = wxINT32_SWAP_ON_LE( event.obid );
            packet[ 25 ] = ( obid & 0xff000000 ) >> 24;
            packet[ 26 ] = ( obid & 0x00ff0000 ) >> 16;
            packet[ 27 ] = ( obid & 0x0000ff00 ) >> 8;
            packet[ 28 ] = ( obid & 0x000000ff );

            // VSCP Class
            uint16_t vscp_class = wxINT16_SWAP_ON_LE( event.vscp_class );
            packet[ 29 ] = ( vscp_class & 0xff00 ) >> 8;
            packet[ 30 ] = ( vscp_class & 0xff );

            // VSCP Type
            uint16_t vscp_type = wxINT16_SWAP_ON_LE( event.vscp_type );
            packet[ 31 ] = ( vscp_type & 0xff00 ) >> 8;
            packet[ 32 ] = ( vscp_class & 0xff );

            // Size
            packet[ 33 ] = event.sizeData >> 8;
            packet[ 34 ] = event.sizeData & 0xff; 

            // VSCP Data
            for ( int idx=0; idx < event.sizeData; idx++ ) {
                packet[ 35 + idx  ] = event.pdata[ idx ];
            }

            // Send the packet
	        if ( 0 != pcap_sendpacket( fp, packet, 32 + 1  + event.sizeData ) ) {
		        //fprintf(stderr,"\nError sending the packet: %s\n", pcap_geterr(fp));
		        // An error sending the frame - we do nothing
                // TODO: Send error frame back to daemon????
	        }

            // We are done with the event - remove data if any
            if ( NULL != event.pdata ) {
                delete [] event.pdata;
                event.pdata = NULL;
            }

        } // Event received

    } // work loop   
 
    // Close the ethernet interface
    pcap_close( fp );

    // Close the channel
    m_srv.doCmdClose();

    return NULL;  
}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void CtcpdrvRxTread::OnExit()
{
    
}
