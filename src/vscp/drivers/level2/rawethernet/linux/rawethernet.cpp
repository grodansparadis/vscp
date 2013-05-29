// rawethernet.cpp: implementation of the CRawEthernet class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 Ake Hedman, 
// Grodans Paradis AB, <akhe@grodansparadis.com>
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

#include <stdio.h>
#include "unistd.h"
#include "stdlib.h"
#include <string.h>
#include "limits.h"
#include "syslog.h"
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <signal.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>
#include <pcap.h>

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>

#include "../../../../common/vscphelper.h"
#include "../../../../common/vscptcpif.h"
#include "../../../../common/vscp_type.h"
#include "../../../../common/vscp_class.h"
#include "rawethernet.h"


//////////////////////////////////////////////////////////////////////
// CRawEthernet
//

CRawEthernet::CRawEthernet()
{
	m_bQuit = false;
	m_preadWorkThread = NULL;
	m_pwriteWorkThread = NULL;
	m_interface = _("eth0");
	memset(m_localMac, 0, 16);

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

	clearVSCPFilter(&m_vscpfilter); // Accept all events
	::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~CRawEthernet
//

CRawEthernet::~CRawEthernet()
{
	close();
	::wxUninitialize();
}


//////////////////////////////////////////////////////////////////////
// open
//
//

bool
CRawEthernet::open(const char *pUsername,
		const char *pPassword,
		const char *pHost,
		short port,
		const char *pPrefix,
		const char *pConfig)
{
	bool rv = true;
	wxString wxstr = wxString::FromAscii(pConfig);

	m_username = wxString::FromAscii(pUsername);
	m_password = wxString::FromAscii(pPassword);
	m_host = wxString::FromAscii(pHost);
	m_port = port;
	m_prefix = wxString::FromAscii(pPrefix);

	// Parse the configuration string. It should
	// have the following form
	// path
	// 
	wxStringTokenizer tkz(wxString::FromAscii(pConfig), _(";\n"));

	// Look for rawethernet interface in configuration string
	if (tkz.HasMoreTokens()) {
		// Interface
		m_interface = tkz.GetNextToken();
	}

	// Local Mac
	wxString localMac;
	if (tkz.HasMoreTokens()) {
		localMac = tkz.GetNextToken();
		localMac.MakeUpper();
		wxStringTokenizer tkzmac(localMac, _(":\n"));
		for (int i = 0; i < 6; i++) {
			if (!tkzmac.HasMoreTokens()) break;
			wxString str = _("0X") + tkzmac.GetNextToken();
			m_localMac[ i ] = readStringValue(str);
			m_localGUIDtx.setAt((9 + i), m_localMac[ i ]);
			m_localGUIDrx.setAt((9 + i), m_localMac[ i ]);
		}
	}


	// First log on to the host and get configuration 
	// variables

	if (m_srv.doCmdOpen(m_host,
			m_port,
			m_username,
			m_password) <= 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "Unable to connect to VSCP TCP/IP interface. Terminating!");
		return false;
	}

	// Find the channel id
	uint32_t ChannelID;
	m_srv.doCmdGetChannelID(&ChannelID);

	// The server should hold configuration data for each sensor
	// we want to monitor.
	// 
	// We look for 
	//
	//	 _interface - The ethernet interface to use. Typically this 
	//					is “eth0, eth0, eth1...
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

	wxString str;
	wxString strName = m_prefix +
			wxString::FromAscii("_interface");
	m_srv.getVariableString(strName, &m_interface);

	// Local Mac
	if (tkz.HasMoreTokens()) {
		localMac = tkz.GetNextToken();
		localMac.MakeUpper();
		wxStringTokenizer tkzmac(localMac, _(":\n"));
		for (int i = 0; i < 6; i++) {
			if (!tkzmac.HasMoreTokens()) break;
			wxString str = _("0X") + tkzmac.GetNextToken();
			m_localMac[ i ] = readStringValue(str);
			m_localGUIDtx.setAt((9 + i), m_localMac[ i ]);
			m_localGUIDrx.setAt((9 + i), m_localMac[ i ]);
		}
	}

	strName = m_prefix +
			wxString::FromAscii("_filter");
	if (m_srv.getVariableString(strName, &str)) {
		readFilterFromString(&m_vscpfilter, str);
	}

	strName = m_prefix +
			wxString::FromAscii("_mask");
	if (m_srv.getVariableString(strName, &str)) {
		readMaskFromString(&m_vscpfilter, str);
	}

	// start the read workerthread
	m_preadWorkThread = new CWrkReadThread();
	if (NULL != m_preadWorkThread) {
		m_preadWorkThread->m_pObj = this;
		m_preadWorkThread->Create();
		m_preadWorkThread->Run();
	} else {
		rv = false;
	}

	// start the write workerthread
	m_pwriteWorkThread = new CWrkWriteThread();
	if (NULL != m_pwriteWorkThread) {
		m_pwriteWorkThread->m_pObj = this;
		m_pwriteWorkThread->Create();
		m_pwriteWorkThread->Run();
	} else {
		rv = false;
	}

	// Close the channel
	m_srv.doCmdClose();

	return rv;
}


//////////////////////////////////////////////////////////////////////
// close
//

void
CRawEthernet::close(void)
{
	// Do nothing if already terminated
	if (m_bQuit) return;

	close();
	::wxUninitialize();

	m_bQuit = true; // terminate the thread
	wxSleep(1); // Give the thread some time to terminate

}



//////////////////////////////////////////////////////////////////////
//                Workerthread - CWrkReadThread
//////////////////////////////////////////////////////////////////////

CWrkReadThread::CWrkReadThread()
{
	m_pObj = NULL;
}

CWrkReadThread::~CWrkReadThread()
{
	;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CWrkReadThread::Entry()
{
	pcap_t *fp;
	char errbuf[ PCAP_ERRBUF_SIZE ];
	//uint8_t packet[ 512 ];

	// First log on to the host and get configuration 
	// variables

	if (m_srv.doCmdOpen(m_pObj->m_host,
			m_pObj->m_port,
			m_pObj->m_username,
			m_pObj->m_password) <= 0) {
		return NULL;
	}

	// Find the channel id
	m_srv.doCmdGetChannelID(&m_pObj->m_ChannelIDtx);

	// It is possible that there is configuration data the server holds 
	// that we need to read in. 
	// We look for 
	//      prefix_interface Communication interface to work on
	//      prefix_localmac MAC address to use for outgoing packets
	//      prefix_filter to find a filter. A string is expected.
	//      prefix_mask to find a mask. A string is expected.

	/*	
		// Interface
		wxString varInterface;
		if (m_srv.getVariableString(m_pObj->m_prefix + _T("_interface"), &varInterface)) {
			m_pObj->m_interface = varInterface;
		}

		wxString varLocalMac;
		if (m_srv.getVariableString(m_pObj->m_prefix + _T("_localmac"), &varLocalMac)) {
			varLocalMac.MakeUpper();
			wxStringTokenizer tkz(varLocalMac, ":\n");
			for (int i = 0; i < 6; i++) {
				if (tkz.HasMoreTokens()) break;
				wxString str = _("0X") + tkz.GetNextToken();
				m_pObj->m_localMac[ i ] = readStringValue(str);
				m_pObj->m_localGUIDtx.setAt((9 + i), m_pObj->m_localMac[ i ]);
				m_pObj->m_localGUIDrx.setAt((9 + i), m_pObj->m_localMac[ i ]);
			}
		}

	 */
	// We want to use our own Ethernet based GUID for this interface
	wxString strGUID;
	m_pObj->m_localGUIDtx.toString(strGUID);
	m_srv.doCmdSetGUID((const char *) strGUID.ToAscii());

	// Open the adapter 
	if ((fp = pcap_open_live(m_pObj->m_interface.ToAscii(), // name of the device
			65536, // portion of the packet to capture. It doesn't matter in this case 
			1, // promiscuous mode (nonzero means promiscuous)
			1000, // read timeout
			errbuf // error buffer
			)) == NULL) {
		//fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", argv[1]);
		return NULL;
	}

	int rv;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;

	while (!TestDestroy() &&
			!m_pObj->m_bQuit &&
			(rv = pcap_next_ex(fp, &header, &pkt_data)) >= 0) {

		// Check for timeout            
		if (0 == rv) continue;

		// Check if this is VSCP
		if ((0x25 == pkt_data[ 12 ]) &&
				(0x7e == pkt_data[ 13 ])) {

			// We have a packet - send it as a VSCP event    
			vscpEventEx event;

			event.head = pkt_data[ 15 ] & 0xe0; // Priority

			event.GUID[ 0 ] = 0xff; // Ethernet predefined  GUID
			event.GUID[ 1 ] = 0xff;
			event.GUID[ 2 ] = 0xff;
			event.GUID[ 3 ] = 0xff;
			event.GUID[ 4 ] = 0xff;
			event.GUID[ 5 ] = 0xff;
			event.GUID[ 6 ] = 0xff;
			event.GUID[ 7 ] = 0xfe;
			event.GUID[ 8 ] = pkt_data[ 6 ]; // Source MAC address
			event.GUID[ 9 ] = pkt_data[ 7 ];
			event.GUID[ 10 ] = pkt_data[ 8 ];
			event.GUID[ 11 ] = pkt_data[ 9 ];
			event.GUID[ 12 ] = pkt_data[ 10 ];
			event.GUID[ 13 ] = pkt_data[ 11 ];
			event.GUID[ 14 ] = pkt_data[ 19 ]; // Device sub address
			event.GUID[ 15 ] = pkt_data[ 20 ];

			event.timestamp = (pkt_data[ 21 ] << 24) +
					(pkt_data[ 22 ] << 16) +
					(pkt_data[ 23 ] << 8) +
					pkt_data[ 24 ];

			event.obid = (pkt_data[ 25 ] << 24) +
					(pkt_data[ 26 ] << 16) +
					(pkt_data[ 27 ] << 8) +
					pkt_data[ 28 ];

			event.vscp_class = (pkt_data[ 29 ] << 8) +
					pkt_data[ 30 ];

			event.vscp_type = (pkt_data[ 31 ] << 8) +
					pkt_data[ 32 ];

			event.sizeData = (pkt_data[ 33 ] << 8) +
					pkt_data[ 34 ];

			// If the packet is smaller then the set datasize just 
			// disregard it
			if ((event.sizeData + 35) > (uint16_t) header->len) continue;

			for (int i = 0; i < event.sizeData; i++) {
				event.data[ i ] = pkt_data[ 35 + i ];
			}

			m_srv.doCmdSendEx(&event); // Send the event

		}


	} // work loop   

	// Close listner
	pcap_close(fp);

	// Close the channel
	m_srv.doCmdClose();

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
CWrkReadThread::OnExit()
{
	;
}



//////////////////////////////////////////////////////////////////////
//                Workerthread - CWrkWriteThread
//////////////////////////////////////////////////////////////////////

CWrkWriteThread::CWrkWriteThread()
{
	m_pObj = NULL;
}

CWrkWriteThread::~CWrkWriteThread()
{
	;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CWrkWriteThread::Entry()
{
	pcap_t *fp;
	char errbuf[ PCAP_ERRBUF_SIZE ];
	uint8_t packet[ 512 ];

	// First log on to the host and get configuration 
	// variables

	if (m_srv.doCmdOpen(m_pObj->m_host,
			m_pObj->m_port,
			m_pObj->m_username,
			m_pObj->m_password) <= 0) {
		return NULL;
	}

	// Find the channel id
	uint32_t ChannelID;
	m_srv.doCmdGetChannelID(&ChannelID);

	// We want to use our own Ethernet based  GUID for this interface
	wxString strGUID;
	m_pObj->m_localGUIDrx.toString(strGUID);
	m_srv.doCmdSetGUID((const char *) strGUID.ToAscii());

	// Open the adapter 
	if ((fp = pcap_open_live(m_pObj->m_interface.ToAscii(), // name of the device
			65536, // portion of the packet to capture. It doesn't matter in this case 
			1, // promiscuous mode (nonzero means promiscuous)
			1000, // read timeout
			errbuf // error buffer
			)) == NULL) {
		//fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", argv[1]);
		return NULL;
	}


	// Enter receive loop to start to log events
	m_srv.doCmdEnterReceiveLoop();

	int rv;
	vscpEvent event;
	while (!TestDestroy() && !m_pObj->m_bQuit) {

		if (CANAL_ERROR_SUCCESS ==
				(rv = m_srv.doCmdBlockReceive(&event, 10))) {

			// As we are on a different VSCP interface we need to filter the events we sent out 
			// ourselves.
			if (m_pObj->m_ChannelIDtx == event.obid) {
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
			memcpy(packet + 6, m_pObj->m_localMac, 6);

			// Set the type - always 0x2574 (9598)
			packet[ 12 ] = 0x25;
			packet[ 13 ] = 0x7e;

			// rawEthernet frame version
			packet[ 14 ] = 0x00;

			// Head
			packet[ 15 ] = (event.head & VSCP_HEADER_PRIORITY_MASK);
			packet[ 16 ] = 0x00;
			packet[ 17 ] = 0x00;
			packet[ 18 ] = 0x00; // LSB

			// VSCP sub source address For this interface it's 0x0000
			packet[ 19 ] = 0x00;
			packet[ 20 ] = 0x00;

			// Timestamp
			uint32_t timestamp = event.timestamp;
			packet[ 21 ] = (timestamp & 0xff000000) >> 24;
			packet[ 22 ] = (timestamp & 0x00ff0000) >> 16;
			packet[ 23 ] = (timestamp & 0x0000ff00) >> 8;
			packet[ 24 ] = (timestamp & 0x000000ff);

			// obid
			uint32_t obid = event.obid;
			packet[ 25 ] = (obid & 0xff000000) >> 24;
			packet[ 26 ] = (obid & 0x00ff0000) >> 16;
			packet[ 27 ] = (obid & 0x0000ff00) >> 8;
			packet[ 28 ] = (obid & 0x000000ff);

			// VSCP Class
			uint16_t vscp_class = event.vscp_class;
			packet[ 29 ] = (vscp_class & 0xff00) >> 8;
			packet[ 30 ] = (vscp_class & 0xff);

			// VSCP Type
			uint16_t vscp_type = event.vscp_type;
			packet[ 31 ] = (vscp_type & 0xff00) >> 8;
			packet[ 32 ] = (vscp_type & 0xff);

			// Size
			packet[ 33 ] = event.sizeData >> 8;
			packet[ 34 ] = event.sizeData & 0xff;

			// VSCP Data
			memcpy(packet + 35, event.pdata, event.sizeData);

			// Send the packet
			if (0 != pcap_sendpacket(fp, packet, 35 + event.sizeData)) {
				//fprintf(stderr,"\nError sending the packet: %s\n", pcap_geterr(fp));
				// An error sending the frame - we do nothing
				// TODO: Send error frame back to daemon????
			}

			// We are done with the event - remove data if any
			if (NULL != event.pdata) {
				delete [] event.pdata;
				event.pdata = NULL;
			}

		} // Event received

	} // work loop   

	// Close the ethernet interface
	pcap_close(fp);

	// Close the channel
	m_srv.doCmdClose();

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
CWrkWriteThread::OnExit()
{
	;
}


