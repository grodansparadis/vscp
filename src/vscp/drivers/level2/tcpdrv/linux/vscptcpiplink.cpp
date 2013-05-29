// vscptcpip.cpp: implementation of the CTcpipLink class.
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
#include "vscptcpiplink.h"


//////////////////////////////////////////////////////////////////////
// CTcpipLink
//

CTcpipLink::CTcpipLink()
{
	m_bQuit = false;
	m_pthreadWork = NULL;
	clearVSCPFilter(&m_vscpfilter); // Accept all events
	::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~CTcpipLink
//

CTcpipLink::~CTcpipLink()
{
	close();
	::wxUninitialize();
}


//////////////////////////////////////////////////////////////////////
// open
//
//

bool
CTcpipLink::open(const char *pUsername,
		const char *pPassword,
		const char *pHost,
		short port,
		const char *pPrefix,
		const char *pConfig)
{
	bool rv = true;
	wxString wxstr = wxString::FromAscii(pConfig);

	m_usernameLocal = wxString::FromAscii(pUsername);
	m_passwordLocal = wxString::FromAscii(pPassword);
	m_hostLocal = wxString::FromAscii(pHost);
	m_portLocal = port;
	m_prefix = wxString::FromAscii(pPrefix);

	// Parse the configuration string. It should
	// have the following form
	// path
	// 
	wxStringTokenizer tkz(wxString::FromAscii(pConfig), _(";\n"));

	// Check for remote host in configuration string
	if (tkz.HasMoreTokens()) {
		// Interface
		m_hostRemote = tkz.GetNextToken();
	}
	
	// Check for remote port in configuration string
	if (tkz.HasMoreTokens()) {
		// Interface
		m_portRemote = readStringValue(tkz.GetNextToken());
	}
	
	// Check for remote user in configuration string
	if (tkz.HasMoreTokens()) {
		// Interface
		m_usernameRemote = tkz.GetNextToken();
	}
	
	// Check for remote password in configuration string
	if (tkz.HasMoreTokens()) {
		// Interface
		m_passwordRemote = tkz.GetNextToken();
	}
	
	wxString strFilter;
	// Check for filter in configuration string
	if (tkz.HasMoreTokens()) {
		// Interface
		strFilter = tkz.GetNextToken();
		readFilterFromString(&m_vscpfilter, strFilter);
	}
	
	// Check for mask in configuration string
	wxString strMask;
	if (tkz.HasMoreTokens()) {
		// Interface
		strMask = tkz.GetNextToken();
		readMaskFromString(&m_vscpfilter, strMask);
	}
	
	// First log on to the host and get configuration 
	// variables

	if (m_srvLocal.doCmdOpen(m_hostLocal,
			m_portLocal,
			m_usernameLocal,
			m_passwordLocal) <= 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "Unable to connect to VSCP TCP/IP interface. Terminating!");
		return false;
	}

	// Find the channel id
	uint32_t ChannelID;
	m_srvLocal.doCmdGetChannelID(&ChannelID);

	// The server should hold configuration data for each sensor
	// we want to monitor.
	// 
	// We look for 
	//
	//	 _host_remote		- The remote host to which we should connect
	//
	//	 _port_remote		- The port to connect to at the remote host.
	//
	//	 _user_remote		- Username to login at remote host
	//
	//	 _password_remote	- Username to login at remote host
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
			wxString::FromAscii("_host_remote");
	m_srvLocal.getVariableString(strName, &m_hostRemote);
	
	strName = m_prefix +
			wxString::FromAscii("_port_remote");
	m_srvLocal.getVariableInt(strName, &m_portRemote);
	
	strName = m_prefix +
			wxString::FromAscii("_user_remote");
	m_srvLocal.getVariableString(strName, &m_usernameRemote);
	
	strName = m_prefix +
			wxString::FromAscii("_password_remote");
	m_srvLocal.getVariableString(strName, &m_passwordRemote);

	strName = m_prefix +
			wxString::FromAscii("_filter");
	if (m_srvLocal.getVariableString(strName, &str)) {
		readFilterFromString(&m_vscpfilter, str);
	}

	strName = m_prefix +
			wxString::FromAscii("_mask");
	if (m_srvLocal.getVariableString(strName, &str)) {
		readMaskFromString(&m_vscpfilter, str);
	}

	// start the workerthread
	m_pthreadWork = new CWrkTread();
	if (NULL != m_pthreadWork) {
		m_pthreadWork->m_pObj = this;
		m_pthreadWork->Create();
		m_pthreadWork->Run();
	} else {
		rv = false;
	}

	// Close the channel
	m_srvLocal.doCmdClose();

	return rv;
}


//////////////////////////////////////////////////////////////////////
// close
//

void
CTcpipLink::close(void)
{
	// Do nothing if already terminated
	if (m_bQuit) return;

	m_bQuit = true; // terminate the thread
	wxSleep(1); // Give the thread some time to terminate

}



//////////////////////////////////////////////////////////////////////
//                Workerthread - CWrkTread
//////////////////////////////////////////////////////////////////////

CWrkTread::CWrkTread()
{
	m_pObj = NULL;
}

CWrkTread::~CWrkTread()
{
	;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CWrkTread::Entry()
{
	bool bActivity;
	bool bRemoteConnectionLost = false;

	::wxInitialize();
			
	// Check pointers
	if (NULL == m_pObj) return NULL;

	// Open local interface
	if (m_srvLocal.doCmdOpen(m_pObj->m_hostLocal,
			m_pObj->m_portLocal,
			m_pObj->m_usernameLocal,
			m_pObj->m_passwordLocal) <= 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "Error while opening local VSCP TCP/IP interface. Terminating!");
		return NULL;
	}

	// Find the channel id
	uint32_t localChannelID;
	m_srvLocal.doCmdGetChannelID(&localChannelID);

	// Get GUID for this interface. 
	//m_srvLocal.doCmdGetGUID(localguid);
	
	
	// Open remote interface
	if (m_srvRemote.doCmdOpen(m_pObj->m_hostRemote,
			m_pObj->m_portRemote,
			m_pObj->m_usernameRemote,
			m_pObj->m_passwordRemote) <= 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "Error while opening remote VSCP TCP/IP interface. Terminating!");
		return NULL;
	}

	// Find the channel id
	uint32_t remoteChannelID;
	m_srvRemote.doCmdGetChannelID(&remoteChannelID);

	// Get GUID for this interface. 
	//m_srvRemote.doCmdGetGUID(localguid);


	vscpEventEx eventEx;
	while (!TestDestroy() && !m_pObj->m_bQuit) {

		// Make sure the remote connection is up
		if (!m_srvRemote.isConnected()) {

			if (!bRemoteConnectionLost) {
				bRemoteConnectionLost = true;
				m_srvRemote.doCmdClose();
				syslog(LOG_ERR,
						"%s",
						(const char *) "Lost connection to remote host.");
			}

			// Wait five seconds before we try to connect again
			::wxSleep(5);

			if (m_srvRemote.doCmdOpen(m_pObj->m_hostRemote,
					m_pObj->m_portRemote,
					m_pObj->m_usernameRemote,
					m_pObj->m_passwordRemote)) {
				syslog(LOG_ERR,
						"%s",
						(const char *) "Reconnected to remote host.");
				bRemoteConnectionLost = false;
			}

		}

		bActivity = false;
		
		// Check if local server has something to send
		if (m_srvLocal.doCmdDataAvailable()) {
			
			bActivity = true;
			
			// Yes there are data to send
			// So send it out on the CAN bus
				
			if (CANAL_ERROR_SUCCESS == m_srvLocal.doCmdReceiveEx(&eventEx)) {
				m_srvRemote.doCmdSendEx(&eventEx);
			}
			
		}
		
		// Check if remote server has something to send
		if (m_srvRemote.doCmdDataAvailable()) {
			
			bActivity = true;
			
			// Yes there are data to send if filter allows it
			if ( doLevel2FilterEx(&eventEx, &m_pObj->m_vscpfilter )) {						
				if (CANAL_ERROR_SUCCESS == m_srvLocal.doCmdReceiveEx(&eventEx)) {
					m_srvLocal.doCmdSendEx(&eventEx);
				}
			}
			
		}
		
		// If no activity we rest for a while
		if ( !bActivity ){
			::wxMilliSleep(100);
		}
		
	}

	// Close the channel
	m_srvLocal.doCmdClose();
	m_srvRemote.doCmdClose();

	return NULL;

}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
CWrkTread::OnExit()
{
	;
}


