// rpigpio.h: interface for the gpio driver.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2018 Ake Hedman, 
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


#if !defined(____RPIGPIO__INCLUDED_)
#define ____RPIGPIO__INCLUDED_

#include <stdio.h>
#include <string.h>
#include <time.h>

#define _POSIX
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>

#include <wx/file.h>
#include <wx/wfstream.h>

#include "../../../../common/canal.h"
#include "../../../../common/vscp.h"
#include "../../../../common/canal_macro.h"
#include "../../../../../common/dllist.h"
#include "../../../../common/vscpremotetcpif.h"
#include "../../../../common/guid.h"

#include <list>
#include <string>

using namespace std;

#define VSCP_LEVEL2_DLL_RPIGPIO_OBJ_MUTEX   "___VSCP__DLL_L2GPIO_OBJ_MUTEX____"

#define VSCP_RPIGPIO_LIST_MAX_MSG		    2048
  
// Forward declarations
class RpiGpioWorkerTread;
class VscpRemoteTcpIf;
class wxFile;


class CRpiGpio {
public:

    /// Constructor
    CRpiGpio();

    /// Destructor
    virtual ~CRpiGpio();

    /*! 
        Open
        @return True on success.
     */
    bool open(const char *pUsername,
            const char *pPassword,
            const char *pHost,
            short port,
            const char *pPrefix,
            const char *pConfig);

    /*!
        Flush and close the log file
     */
    void close(void);

	/*!
		Add event to send queue 
	 */
	bool addEvent2SendQueue(const vscpEvent *pEvent);

public:

    /// Run flag
    bool m_bQuit;
	
    /// Server supplied username
    wxString m_username;

    /// Server supplied password
    wxString m_password;

    /// server supplied prefix
    wxString m_prefix;

    /// server supplied host
    wxString m_host;

    /// Server supplied port
    short m_port;
    
    /// socketcan interface to use
    wxString m_interface;
    
    /// Filter
    vscpEventFilter m_vscpfilter;
	
	/// Get GUID for this interface.
	//cguid m_ifguid;

    /// Pointer to worker threads
    RpiGpioWorkerTread *m_pthreadWorker;
    
     /// VSCP server interface
    VscpRemoteTcpIf m_srv;
	
	// Queue
	//VSCPEVENTLIST_SEND m_sendQueue;			// Things we should send
	//VSCPEVENTLIST_RECEIVE m_receiveQueue;		// Thing this driver receive
	
	std::list<vscpEvent *> m_sendList;
	std::list<vscpEvent *> m_receiveList;
	
	/*!
        Event object to indicate that there is an event in the output queue
     */
    wxSemaphore m_semSendQueue;			
	wxSemaphore m_semReceiveQueue;		
	
	// Mutex to protect the output queue
	wxMutex m_mutexSendQueue;		
	wxMutex m_mutexReceiveQueue;

};

///////////////////////////////////////////////////////////////////////////////
//				                Worker Treads
///////////////////////////////////////////////////////////////////////////////


class RpiGpioWorkerTread : public wxThread {
public:

    /// Constructor
    RpiGpioWorkerTread();

    /// Destructor
    ~RpiGpioWorkerTread();

    /*!
        Thread code entry point
     */
    virtual void *Entry();

    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
     */
    virtual void OnExit();

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    /// Sensor object
    CRpiGpio *m_pObj;

};



#endif // !defined(____RPIGPIO__INCLUDED_)
