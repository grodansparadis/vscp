// vscp2logger.h : main header file for the canallogger.dll
// Linux version
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2017 Ake Hedman, 
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

#if !defined(AFX_LOGGERDLL_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_)
#define AFX_LOGGERDLL_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_

#include "../common/log.h"

// This is the version info for this DLL - Change to your own value
#define VSCP_DLL_VERSION        0x000001

// This is the vendor string - Change to your own value
#define VSCP_DLL_VENDOR "Grodans Paradis AB, Sweden, http://www.grodansparadis.com"

// Driver information.
#define VSCP_LOGGER_DRIVERINFO "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>" \
"<!-- Version 0.0.1    2013-05-11   -->" \
"<config level=\"1|2\"blocking\"true|false\" description=\"bla bla bla bla\">" \
"   <item pos=\"0\" type=\"string\" description\"Serial number for Tellstick\"/>" \
"   <item pos=\"1\" type=\"path\" description\"Path to configuration file\"/>" \
"</config>"

// Max number of open connections
#define VSCP_LOGGER_DRIVER_MAX_OPEN	    256

/////////////////////////////////////////////////////////////////////////////
// CLoggerApp
// See loggerdll.cpp for the implementation of this class
//

class CLoggerApp {
public:

    /// Constructor
    CLoggerApp();

    /// Destructor
    ~CLoggerApp();

    /*!
        Add a driver object

        @parm plog Object to add
        @return handle or 0 for error
     */
    long addDriverObject(CVSCPLog *plog);

    /*!
        Get a driver object from its handle

        @param handle for object
        @return pointer to object or NULL if invalid
                handle.
     */
    CVSCPLog *getDriverObject(long h);

    /*!
        Remove a driver object

        @parm handle for object.
     */
    void removeDriverObject(long h);

    /*!
        The log file object
        This is the array with driver objects (max 256 objects)
     */
    CVSCPLog *m_logArray[ VSCP_LOGGER_DRIVER_MAX_OPEN ];

    /// Mutex for open/close
    pthread_mutex_t m_objMutex;

    /// Counter for users of the interface
    unsigned long m_instanceCounter;

public:
    BOOL InitInstance();

};

///////////////////////////////////////////////////////////////////////////////
// CreateObject
//

extern "C" {
    CLoggerApp *CreateObject(void);
}

#endif // !defined(AFX_LOGGERDLL_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_)
