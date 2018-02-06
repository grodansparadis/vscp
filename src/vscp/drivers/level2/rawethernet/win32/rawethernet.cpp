//
// rawethernet.cpp : Defines the entry point for the DLL application.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2017 Ake Hedman, Grodans Paradis AB,
// <akhe@grodansparadis.com>
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



#include <vscphelper.h>
#include <vscpremotetcpif.h>
#include "rawethernet.h"

static HANDLE hThisInstDll = NULL;
static CDllDrvObj *theApp = NULL;

#pragma comment(lib, "Irprops.lib")



///////////////////////////////////////////////////////////////////////////////
// DllMain
//

BOOL APIENTRY DllMain(HANDLE hInstDll,
		DWORD ul_reason_for_call,
		LPVOID lpReserved
		)
{
	switch (ul_reason_for_call) {

	case DLL_PROCESS_ATTACH:
		::wxInitialize();
		hThisInstDll = hInstDll;
		theApp = new CDllDrvObj();
		theApp->InitInstance();
		break;

	case DLL_THREAD_ATTACH:
		if (NULL == theApp) delete theApp;
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
extern "C" long WINAPI EXPORT VSCPOpen(const char *pUsername,
		const char *pPassword,
		const char *pHost,
		short port,
		const char *pPrefix,
		const char *pParameter,
		unsigned long flags)
#else

extern "C" long VSCPOpen(const char *pUsername,
		const char *pPassword,
		const char *pHost,
		short port,
		const char *pPrefix,
		const char *pParameter,
		unsigned long flags)
#endif
{
	long h = 0;

	CRawEthernet *pdrvObj = new CRawEthernet();
	if (NULL != pdrvObj) {

		if (pdrvObj->open(pUsername,
				pPassword,
				pHost,
				port,
				pPrefix,
				pParameter,
				flags)) {

			if (!(h = theApp->addDriverObject(pdrvObj))) {
				delete pdrvObj;
			}

		} else {
			delete pdrvObj;
		}

	}

	return h;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPClose
// 

#ifdef WIN32
extern "C" int WINAPI EXPORT VSCPClose(long handle)
#else

extern "C" int VSCPClose(long handle)
#endif
{
	int rv = 0;

	CRawEthernet *pdrvObj = theApp->getDriverObject(handle);
	if (NULL == pdrvObj) return 0;
	pdrvObj->close();
	theApp->removeDriverObject(handle);
	rv = 1;
	return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingSend
// 

extern "C" int
VSCPBlockingSend(long handle, const vscpEvent *pEvent, unsigned long timeout)
{
	int rv = 0;

	CRawEthernet *pdrvObj = theApp->getDriverObject(handle);
	if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;
    
    pdrvObj->addEvent2SendQueue( pEvent );
    
	return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingReceive
// 

extern "C" int
VSCPBlockingReceive(long handle, vscpEvent *pEvent, unsigned long timeout)
{
	int rv = 0;
 
    // Check pointer
    if ( NULL == pEvent) return CANAL_ERROR_PARAMETER;
    
	CRawEthernet *pdrvObj = theApp->getDriverObject(handle);
	if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;
    
    if ( wxSEMA_TIMEOUT == pdrvObj->m_semReceiveQueue.WaitTimeout( timeout ) ) {
        return CANAL_ERROR_TIMEOUT;
    }
    
	pdrvObj->m_mutexReceiveQueue.Lock();
    vscpEvent *pLocalEvent = pdrvObj->m_receiveList.front();
    pdrvObj->m_receiveList.pop_front();
	pdrvObj->m_mutexReceiveQueue.Unlock();
    if (NULL == pLocalEvent) return CANAL_ERROR_MEMORY;
    
    vscp_copyVSCPEvent( pEvent, pLocalEvent );
    vscp_deleteVSCPevent( pLocalEvent );
	
	return CANAL_ERROR_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
//  VSCPGetLevel
// 

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT VSCPGetLevel(void)
#else

extern "C" unsigned long VSCPGetLevel(void)
#endif
{
	return CANAL_LEVEL_USES_TCPIP;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetDllVersion
//

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT VSCPGetDllVersion(void)
#else

extern "C" unsigned long VSCPGetDllVersion(void)
#endif
{
	return VSCP_DLL_VERSION;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetVendorString
//

#ifdef WIN32
extern "C" const char * WINAPI EXPORT VSCPGetVendorString(void)
#else

extern "C" const char * VSCPGetVendorString(void)
#endif
{
	return VSCP_DLL_VENDOR;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetDriverInfo
//

#ifdef WIN32
extern "C" const char * WINAPI EXPORT VSCPGetDriverInfo(void)
#else

extern "C" const char * VSCPGetDriverInfo(void)
#endif
{
	return VSCP_RAWETH_DRIVERINFO;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPGetVSCPGetWebPageTemplate
// 

extern "C" long
VSCPGetWebPageTemplate( long handle, const char *url, char *page )
{
    page = NULL;

    // Not implemented
    return -1;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPGetVSCPWebPageInfo
// 

extern "C" int
VSCPGetWebPageInfo( long handle, const struct vscpextwebpageinfo *info )
{
    // Not implemented
    return -1;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPWebPageupdate
// 

extern "C" int
VSCPWebPageupdate( long handle, const char *url )
{
    // Not implemented
    return -1;
}



///////////////////////////////////////////////////////////////////////////////
//                            T H E  C O D E
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CRawEthernet::CRawEthernet()
{
	m_bQuit = false;

    m_pWrkReadTread = NULL;
    m_pWrkWriteTread = NULL;

	memset(m_localMac, 0, sizeof(m_localMac));

    m_subaddr = 0;

	// Initialize tx channel GUID
	m_localGUIDtx.clear();
	m_localGUIDtx.setAt(0, 0xff);
	m_localGUIDtx.setAt(1, 0xff);
	m_localGUIDtx.setAt(2, 0xff);
	m_localGUIDtx.setAt(3, 0xff);
	m_localGUIDtx.setAt(4, 0xff);
	m_localGUIDtx.setAt(5, 0xff);
	m_localGUIDtx.setAt(6, 0xff);
	m_localGUIDtx.setAt(7, 0xfe);
	m_localGUIDtx.setAt(14, 0x00);
	m_localGUIDtx.setAt(15, 0x00);

	// Initialize rx channel GUID
	m_localGUIDrx.clear();
	m_localGUIDrx.setAt(0, 0xff);
	m_localGUIDrx.setAt(1, 0xff);
	m_localGUIDrx.setAt(2, 0xff);
	m_localGUIDrx.setAt(3, 0xff);
	m_localGUIDrx.setAt(4, 0xff);
	m_localGUIDrx.setAt(5, 0xff);
	m_localGUIDrx.setAt(6, 0xff);
	m_localGUIDrx.setAt(7, 0xfe);
	m_localGUIDrx.setAt(14, 0x00);
	m_localGUIDrx.setAt(15, 0x01);

    vscp_clearVSCPFilter( &m_vscpfilter ); // Accept all events

	::wxInitialize();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CRawEthernet::~CRawEthernet()
{
	close();
	::wxUninitialize();
}


///////////////////////////////////////////////////////////////////////////////
// open
//

bool CRawEthernet::open( const char *pUsername,
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
	// interface;mac,filter;mask
	wxStringTokenizer tkz(pConfig, ";\n");

	// Interface
	if ( tkz.HasMoreTokens() ) {
		m_interface = tkz.GetNextToken();
	}

	// Local Mac
	wxString localMac;
	if ( tkz.HasMoreTokens() ) {
		localMac = tkz.GetNextToken();
		localMac.MakeUpper();
		wxStringTokenizer tkzmac(localMac, ":\n");
		for (int i = 0; i < 6; i++) {
			if (!tkzmac.HasMoreTokens()) break;
			wxString str = _("0X") + tkzmac.GetNextToken();
			m_localMac[ i ] = vscp_readStringValue(str);
			m_localGUIDtx.setAt((9 + i), m_localMac[ i ]);
			m_localGUIDrx.setAt((9 + i), m_localMac[ i ]);
		}
	}



    // First log on to the host and get configuration 
    // variables

    if ( VSCP_ERROR_SUCCESS != m_srv.doCmdOpen( m_host,
        m_username,
        m_password ) ) {
        return NULL;
    }

    // The server should hold configuration data for each sensor
    // we want to monitor.
    // 
    // We look for 
    //
    //	 _interface - The ethernet interface to use. Typically this 
    //					is �eth0, eth0, eth1...
    //
    //   _localmac - The MAC address for our outgoing frames.
    //					Typically on the form 00:26:55:CA:1F:DA
    //
    //   _filter - Standard VSCP filter in string form. 
    //				   1,0x0000,0x0006,
    //				   ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //				as priority,class,type,GUID
    //				Used to filter what events that is received from 
    //				the socketcan interface. If not give all events 
    //				are received.
    //	 _mask - Standard VSCP mask in string form.
    //				   1,0x0000,0x0006,
    //				   ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //				as priority,class,type,GUID
    //				Used to filter what events that is received from 
    //				the socketcan interface. If not give all events 
    //				are received. 
    //
    //	 _subaddr - Normally the subaddr of the computer the rawtherent
    //              driver is on is 0x0000 this can be changed with this vaeiable
    //

    // Interface
    wxString varInterface;
    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue( m_prefix + _T( "_interface" ), varInterface ) ) {
        m_interface = varInterface;
    }

    wxString varLocalMac;
    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue( m_prefix + _T( "_localmac" ), varLocalMac ) ) {
        varLocalMac.MakeUpper();
        wxStringTokenizer tkz( varLocalMac, ":\n" );
        for ( int i = 0; i < 6; i++ ) {
            if ( tkz.HasMoreTokens() ) break;
            wxString str = _( "0X" ) + tkz.GetNextToken();
            m_localMac[ i ] = vscp_readStringValue( str );
            m_localGUIDtx.setAt( ( 9 + i ), m_localMac[ i ] );
            m_localGUIDrx.setAt( ( 9 + i ), m_localMac[ i ] );
        }
    }

    wxString strFilter;
    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue( m_prefix + _T( "_filter" ), strFilter ) ) {
        vscp_readFilterFromString( &m_vscpfilter, strFilter );
    }

    wxString strMask;
    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue( m_prefix + _T( "_mask" ), strMask ) ) {
        vscp_readMaskFromString( &m_vscpfilter, strMask );
    }

    long subaddr;
    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableLong( m_prefix + _T( "_subaddr" ), &subaddr ) ) {
        m_subaddr = subaddr;
    }

    // We want to use our own Ethernet based GUID for this interface
    wxString strGUID;
    m_localGUIDtx.toString( strGUID );
    m_srv.doCmdSetGUID( (const unsigned char *)( (const char *)strGUID.mbc_str() ) );

    // Close the channel
    m_srv.doCmdClose();

	// start the workerthreads
    m_pWrkReadTread = new CWrkReadTread();
    if ( NULL != m_pWrkReadTread ) {
        m_pWrkReadTread->m_pObj = this;
        m_pWrkReadTread->Create();
        m_pWrkReadTread->Run();
	} 
    else {
		rv = false;
	}

	wxSleep(1);

    m_pWrkWriteTread = new CWrkWriteTread();
    if ( NULL != m_pWrkWriteTread ) {
        m_pWrkWriteTread->m_pObj = this;
        m_pWrkWriteTread->Create();
        m_pWrkWriteTread->Run();
	} 
    else {
		rv = false;
	}


	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// close
//

void CRawEthernet::close(void)
{
	// Do nothing if already terminated
	if (m_bQuit) return;

	m_bQuit = true; // terminate the thread
	wxSleep(1); // Give the thread some time to terminate
}



//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool 
CRawEthernet::addEvent2SendQueue(const vscpEvent *pEvent)
{
    m_mutexSendQueue.Lock();
    m_sendList.push_back((vscpEvent *)pEvent);
	m_semSendQueue.Post();
	m_mutexSendQueue.Unlock();
    return true;
}




///////////////////////////////////////////////////////////////////////////////
//                           W O R K E R   T H R E A D S
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//                            CWrkReadTread
///////////////////////////////////////////////////////////////////////////////

CWrkReadTread::CWrkReadTread()
{
    ;
}

CWrkReadTread::~CWrkReadTread()
{
    ;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *CWrkReadTread::Entry()
{
	pcap_t *fp;
	char errbuf[ PCAP_ERRBUF_SIZE ];

	// Open the adapter 
    if ( ( fp = pcap_open_live( (const char *)m_pObj->m_interface.mbc_str(), // name of the device
			                        65536,  // portion of the packet to capture. It doesn't matter in this case 
			                        1,      // promiscuous mode (nonzero means promiscuous)
			                        1000,   // read timeout
			                        errbuf  // error buffer
			                  ) ) == NULL) {
		//fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", argv[1]);
		return NULL;
	}

	int rv;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;

	while ( !TestDestroy() &&
            !m_pObj->m_bQuit &&
			( rv = pcap_next_ex(fp, &header, &pkt_data) ) >= 0) {

		// Check for timeout            
		if (0 == rv) continue;

        // If from our own interface we skip
        if ( 0 == memcmp( pkt_data + 6, m_pObj->m_localMac, 6 ) ) continue;

		// Check if this is VSCP
		if ( ( 0x25 == pkt_data[ 12 ] ) &&
				(0x7e == pkt_data[ 13 ] ) ) {

			// We have a packet - send it as a VSCP event    
			vscpEventEx eventex;

            eventex.head = pkt_data[ 15 ] & 0xe0; // Priority

			eventex.timestamp = vscp_makeTimeStamp();
            vscp_setEventExDateTimeBlockToNow( &eventex );

            eventex.GUID[ 0 ] = 0xff; // Ethernet predefined  GUID
            eventex.GUID[ 1 ] = 0xff;
            eventex.GUID[ 2 ] = 0xff;
            eventex.GUID[ 3 ] = 0xff;
            eventex.GUID[ 4 ] = 0xff;
            eventex.GUID[ 5 ] = 0xff;
            eventex.GUID[ 6 ] = 0xff;
            eventex.GUID[ 7 ] = 0xfe;
            eventex.GUID[ 8 ] = pkt_data[ 6 ]; // Source MAC address
            eventex.GUID[ 9 ] = pkt_data[ 7 ];
            eventex.GUID[ 10 ] = pkt_data[ 8 ];
            eventex.GUID[ 11 ] = pkt_data[ 9 ];
            eventex.GUID[ 12 ] = pkt_data[ 10 ];
            eventex.GUID[ 13 ] = pkt_data[ 11 ];
            eventex.GUID[ 14 ] = pkt_data[ 19 ]; // Device sub address
            eventex.GUID[ 15 ] = pkt_data[ 20 ];

            eventex.timestamp = ( pkt_data[ 21 ] << 24 ) +
					            (pkt_data[ 22 ] << 16) +
					            (pkt_data[ 23 ] << 8) +
					            pkt_data[ 24 ];

            eventex.obid = ( pkt_data[ 25 ] << 24 ) +
					            (pkt_data[ 26 ] << 16) +
					            (pkt_data[ 27 ] << 8) +
					            pkt_data[ 28 ];

            eventex.vscp_class = ( pkt_data[ 29 ] << 8 ) +
					                pkt_data[ 30 ];

            eventex.vscp_type = ( pkt_data[ 31 ] << 8 ) +
					                pkt_data[ 32 ];

			eventex.sizeData = (pkt_data[ 33 ] << 8) +
					                pkt_data[ 34 ];

            // Validate data size
            if ( eventex.vscp_class < 512 ) {
                if ( eventex.sizeData > 8 ) eventex.sizeData = 8;
            }
            else if ( eventex.vscp_class < 512 ) {
                if ( eventex.sizeData > ( 16 + 8 ) ) eventex.sizeData = 24;
            }
            else {
                if ( eventex.sizeData > VSCP_MAX_DATA ) eventex.sizeData = VSCP_MAX_DATA;
            }

			// If the packet is smaller then the set datasize just 
			// disregard it
            if ( ( eventex.sizeData + 35 ) > ( uint16_t )header->len ) continue;

            memcpy( eventex.data, pkt_data + 35, eventex.sizeData );

            vscpEvent *pEvent = new vscpEvent;
            if ( NULL != pEvent ) {

                vscp_convertVSCPfromEx( pEvent, &eventex );

                if ( vscp_doLevel2FilterEx( &eventex, &m_pObj->m_vscpfilter ) ) {
                    m_pObj->m_mutexReceiveQueue.Lock();
                    m_pObj->m_receiveList.push_back( pEvent );
                    m_pObj->m_semReceiveQueue.Post();
                    m_pObj->m_mutexReceiveQueue.Unlock();
                }
                else {
                    vscp_deleteVSCPevent( pEvent );
                }

            }

		}


	} // work loop   

	// Close listner
	pcap_close(fp);

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void CWrkReadTread::OnExit()
{

}




///////////////////////////////////////////////////////////////////////////////
//                            CWrkWriteTread
///////////////////////////////////////////////////////////////////////////////

CWrkWriteTread::CWrkWriteTread()
{

}

CWrkWriteTread::~CWrkWriteTread()
{

}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *CWrkWriteTread::Entry()
{
	pcap_t *fp;
	char errbuf[ PCAP_ERRBUF_SIZE ];
	uint8_t packet[ 512 ];

	// Open the adapter 
    if ( ( fp = pcap_open_live( (const char *)m_pObj->m_interface.mbc_str(), // name of the device
			                        65536, // portion of the packet to capture. It doesn't matter in this case 
			                        1, // promiscuous mode (nonzero means promiscuous)
			                        1000, // read timeout
			                        errbuf // error buffer
			                    ) ) == NULL ) {
		//fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", argv[1]);
		return NULL;
	}

    while ( !TestDestroy() && !m_pObj->m_bQuit ) {

        if ( wxSEMA_TIMEOUT == m_pObj->m_semSendQueue.WaitTimeout( 300 ) ) continue;

        // Check if there is event(s) to send
        if ( m_pObj->m_sendList.size() ) {

            // Yes there are data to send
            m_pObj->m_mutexSendQueue.Lock();
            vscpEvent *pEvent = m_pObj->m_sendList.front();
            m_pObj->m_sendList.pop_front();
            m_pObj->m_mutexSendQueue.Unlock();

            if ( NULL == pEvent ) continue;

			// Set mac destination to broadcast ff:ff:ff:ff:ff:ff 
			packet[ 0 ] = 0xff;
			packet[ 1 ] = 0xff;
			packet[ 2 ] = 0xff;
			packet[ 3 ] = 0xff;
			packet[ 4 ] = 0xff;
			packet[ 5 ] = 0xff;

			// set mac source to configured value - 6..11
            memcpy( packet + 6, m_pObj->m_localMac, 6 );

			// Set the type - always 0x2574 (9598)
			packet[ 12 ] = 0x25;
			packet[ 13 ] = 0x7e;

			// rawEthernet frame version
			packet[ 14 ] = RAW_ETHERNET_FRAME_VERSION;

			// Head
			packet[ 15 ] = (pEvent->head & VSCP_HEADER_PRIORITY_MASK);
			packet[ 16 ] = 0x00;
			packet[ 17 ] = 0x00;
			packet[ 18 ] = 0x00; // LSB

			// VSCP sub source address 
            packet[ 19 ] = ( m_pObj->m_subaddr >> 8 ) & 0xff;
            packet[ 20 ] = m_pObj->m_subaddr & 0xff;

			// Timestamp
            uint32_t timestamp = pEvent->timestamp;
			packet[ 21 ] = (timestamp & 0xff000000) >> 24;
			packet[ 22 ] = (timestamp & 0x00ff0000) >> 16;
			packet[ 23 ] = (timestamp & 0x0000ff00) >> 8;
			packet[ 24 ] = (timestamp & 0x000000ff);

			// obid
            uint32_t obid = pEvent->obid;
			packet[ 25 ] = (obid & 0xff000000) >> 24;
			packet[ 26 ] = (obid & 0x00ff0000) >> 16;
			packet[ 27 ] = (obid & 0x0000ff00) >> 8;
			packet[ 28 ] = (obid & 0x000000ff);

			// VSCP Class
            uint16_t vscp_class = pEvent->vscp_class;
			packet[ 29 ] = (vscp_class & 0xff00) >> 8;
			packet[ 30 ] = (vscp_class & 0xff);

			// VSCP Type
            uint16_t vscp_type = pEvent->vscp_type;
			packet[ 31 ] = (vscp_type & 0xff00) >> 8;
			packet[ 32 ] = (vscp_type & 0xff);

			// Size
            packet[ 33 ] = pEvent->sizeData >> 8;
            packet[ 34 ] = pEvent->sizeData & 0xff;

			// VSCP Data
            memcpy( packet + 35, pEvent->pdata, pEvent->sizeData );

			// Send the packet
            if ( 0 != pcap_sendpacket( fp, packet, 35 + pEvent->sizeData ) ) {
				//fprintf(stderr,"\nError sending the packet: %s\n", pcap_geterr(fp));
				// An error sending the frame - we do nothing
				// TODO: Send error frame back to daemon????
			}

			// We are done with the event - remove data if any
            if ( NULL != pEvent->pdata ) {
                delete[] pEvent->pdata;
                pEvent->pdata = NULL;
			}

		} // Event received

	} // work loop   

	// Close the ethernet interface
	pcap_close(fp);

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void CWrkWriteTread::OnExit()
{

}
