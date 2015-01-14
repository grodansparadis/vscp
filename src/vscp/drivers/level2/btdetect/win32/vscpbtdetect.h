///////////////////////////////////////////////////////////////////////////////
// vscpbtdetect.h:
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://can.sourceforge.net)
//
// Copyright (C) 2000-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef vscpbtdetect_h
#define vscpbtdetect_h

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef WIN32

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#else

#define _POSIX
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>

#endif

#include <wx/file.h>
#include <wx/wfstream.h>

#include "../../../../common/canal.h"
#include "../../../../common/vscp.h"
#include "../../../../common/canal_macro.h"
#include "../../../../../common/dllist.h"
#include "../../../../common/vscpremotetcpif.h"

// Forward declarations
class CVSCPBTDetectWrkTread;
class VscpTcpIf;


class CDetectedDevice
{

public:

    /*!
        Constructor
    */
    CDetectedDevice();

    /*!
        Destructor
    */
    ~CDetectedDevice();

    uint16_t    m_radio_id;     // Radio id
    uint16_t    m_device_id;    // Device id for device
    uint8_t     m_address[ 6 ]; // Address of device
    wxString    m_name;         // Name of device
    uint32_t    m_class;        // Device class
    uint16_t    m_manufacturer; // Maker of the device
    SYSTEMTIME  m_lastSeen;     // Last time the system detected this device
    bool        m_bfound;       // True if the device is found in search
};


class CVSCPBTDetect  
{

public:

	/// Constructor
	CVSCPBTDetect();
	
	/// Destructor
	virtual ~CVSCPBTDetect();

    	/*! 
		Open the Bluetooth detecter

		@param Configuration string
		@param flags 	Not used
		@return True on success.
	*/
	bool open( const char *pUsername,
                const char *pPassword,
                const char *pHost,
                short port,
                const char *pPrefix,
                const char *pConfig, 
                unsigned long flags = 0 );

	/*!
		Flush and close the log file
	*/
	void close( void );

public:

	/// Run flag
	bool m_bQuit;

    /// Driver flags
    unsigned long m_flags;

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

    /// Pointer to worker thread
    CVSCPBTDetectWrkTread *m_pthreadWork;

};



///////////////////////////////////////////////////////////////////////////////
//						       Worker Tread
///////////////////////////////////////////////////////////////////////////////


class CVSCPBTDetectWrkTread : public wxThread 
{

public:

	/// Constructor
	CVSCPBTDetectWrkTread();

	/// Destructor
	~CVSCPBTDetectWrkTread();

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

    /// Mama pointer
    CVSCPBTDetect *m_pobj;
};


#endif
