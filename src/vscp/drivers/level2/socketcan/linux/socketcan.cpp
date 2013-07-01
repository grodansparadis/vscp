// socketcan.cpp: implementation of the Csocketcan class.
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

// Different on Kernel 2.6 and cansocket examples
// currently using locally from can-utils
// TODO remove include form makefile when they are in sync
#include <linux/can.h>
#include <linux/can/raw.h>

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
#include "socketcan.h"


//////////////////////////////////////////////////////////////////////
// Csocketcan
//

Csocketcan::Csocketcan()
{
	m_bQuit = false;
	m_pthreadWork = NULL;
	m_interface = _("vcan0");
	clearVSCPFilter(&m_vscpfilter); // Accept all events
	::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~Csocketcan
//

Csocketcan::~Csocketcan()
{
	close();
	::wxUninitialize();
}


//////////////////////////////////////////////////////////////////////
// open
//
//

bool
Csocketcan::open(const char *pUsername,
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

	// Check for socketcan interface in configuration string
	if (tkz.HasMoreTokens()) {
		// Interface
		m_interface = tkz.GetNextToken();
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
	//	 _interface - The socketcan interface to use. Typically this 
	//	 is “can0, can0, can1...
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

	// start the workerthread
	m_pthreadWork = new CReadSocketCanTread();
	if (NULL != m_pthreadWork) {
		m_pthreadWork->m_pObj = this;
		m_pthreadWork->Create();
		m_pthreadWork->Run();
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
Csocketcan::close(void)
{
	// Do nothing if already terminated
	if (m_bQuit) return;

	m_bQuit = true; // terminate the thread
	wxSleep(1); // Give the thread some time to terminate

}



//////////////////////////////////////////////////////////////////////
//                Workerthread - CReadSocketCanTread
//////////////////////////////////////////////////////////////////////

CReadSocketCanTread::CReadSocketCanTread()
{
	m_pObj = NULL;
}

CReadSocketCanTread::~CReadSocketCanTread()
{
	;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CReadSocketCanTread::Entry()
{
	int sock;
	char devname[IFNAMSIZ + 1];
	fd_set rdfs;
	struct timeval tv;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct cmsghdr *cmsg;
	struct canfd_frame frame;
	char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
	const int canfd_on = 1;
	cguid ifguid; // Interface GUID on VSCP server.

	::wxInitialize();
			
	// Check pointers
	if (NULL == m_pObj) return NULL;

	strncpy(devname, m_pObj->m_interface.ToAscii(), sizeof(devname) - 1);
#if DEBUG	
	syslog(LOG_ERR, "CWriteSocketCanTread: Interface: %s\n", ifname);
#endif	

	if (m_srv.doCmdOpen(m_pObj->m_host,
			m_pObj->m_port,
			m_pObj->m_username,
			m_pObj->m_password) <= 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "CReadSocketCanTread: Error while opening VSCP TCP/IP interface. Terminating!");
		return NULL;
	}

	// Find the channel id
	uint32_t ChannelID;
	m_srv.doCmdGetChannelID(&ChannelID);

	// Get GUID for this interface. 
	m_srv.doCmdGetGUID(ifguid);
	
	sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (sock < 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "CReadSocketCanTread: Error while opening socket. Terminating!");
		return NULL;
	}

	strcpy(ifr.ifr_name, devname);
	ioctl(sock, SIOCGIFINDEX, &ifr);

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

#ifdef DEBUG
	printf("using interface name '%s'.\n", ifr.ifr_name);
#endif

	// try to switch the socket into CAN FD mode 
	setsockopt(sock,
			SOL_CAN_RAW,
			CAN_RAW_FD_FRAMES,
			&canfd_on,
			sizeof(canfd_on));

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "CReadSocketCanTread: Error in socket bind. Terminating!");
		return NULL;
	}

	while (!TestDestroy() && !m_pObj->m_bQuit) {

		FD_ZERO(&rdfs);
		FD_SET(sock, &rdfs);

		tv.tv_sec = 0;
		tv.tv_usec = 5000; // 50ms timeout 

		int ret;
		if ((ret = select(sock+1, &rdfs, NULL, NULL, &tv)) < 0) {
			// Error
			m_pObj->m_bQuit = true;
			continue;
		}

		if (ret) {

			// There is data to read

			ret = read(sock, &frame, sizeof(struct can_frame));
			if (ret < 0) {
				m_pObj->m_bQuit = true;
				continue;
			}

			// Must be Extended
			if (!(frame.can_id & CAN_EFF_FLAG)) continue;

			// Mask of control bits
			frame.can_id &= CAN_EFF_MASK;

			vscpEvent *pEvent = new vscpEvent();
			if (NULL != pEvent) {
				
				pEvent->pdata = new uint8_t[16 + frame.len];
				if ( NULL == pEvent->pdata ) {
					delete pEvent;
					continue;
				}

				// Set VSCP class + 512
				pEvent->vscp_class = getVSCPclassFromCANid(frame.can_id) + 512;

				// Set VSCP type
				pEvent->vscp_type = getVSCPtypeFromCANid(frame.can_id);

				// Set node id + guid
				ifguid.setLSB(frame.can_id & 0xff);
				ifguid.setGUID(pEvent->pdata);

				// Copy data if any
				pEvent->sizeData = frame.len + 16;
				if (frame.len) {	
					memcpy(pEvent->pdata + 16, frame.data, frame.len);
				}

				if ( doLevel2Filter(pEvent, &m_pObj->m_vscpfilter )) {						
					// Send the event
					m_srv.doCmdSend(pEvent);
				}
				else {
					deleteVSCPevent(pEvent);
				}
			}

		} else {
	
			// Check if there is event(s) to send
			if (m_srv.doCmdDataAvailable()) {

				// Yes there are data to send
				// So send it out on the CAN bus
				
				vscpEventEx *pEventEx = new vscpEventEx();
				if (NULL != pEventEx && 
						CANAL_ERROR_SUCCESS == m_srv.doCmdReceiveEx(pEventEx)) {
                    
					// Class must be a Level I class or a Level II
					// mirror class
					if (pEventEx->vscp_class < 512) {
						frame.can_id = getCANidFromVSCPeventEx(pEventEx);
						frame.can_id |= CAN_EFF_FLAG; // Always extended
						if (0 != pEventEx->sizeData) {
							frame.len = (pEventEx->sizeData > 8 ? 8 : pEventEx->sizeData);
							memcpy(frame.data, pEventEx->data, frame.len);
						}
					} else if (pEventEx->vscp_class < 1024) {
						pEventEx->vscp_class -= 512;
                        frame.can_id = getCANidFromVSCPeventEx(pEventEx);
						frame.can_id |= CAN_EFF_FLAG; // Always extended
						if (0 != pEventEx->sizeData) {
							frame.len = ((pEventEx->sizeData - 16) > 8 ? 8 : pEventEx->sizeData - 16);
							memcpy(frame.data, pEventEx->data + 16, frame.len);
						}
					}

					// Remove the event
					deleteVSCPeventEx(pEventEx);

					// Write the data
					int nbytes = write(sock, &frame, sizeof(struct can_frame));
				}
			}
		}
	}

	// Close the socket
	close(sock);

	// Close the channel
	m_srv.doCmdClose();

	return NULL;

}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
CReadSocketCanTread::OnExit()
{
	;
}


