// DeviceList.cpp: 
//
// implementation of the CDeviceList class.
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 
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


#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/app.h>
#include <wx/listimpl.cpp>
#include <wx/xml/xml.h>
#include <wx/file.h>

#ifdef WIN32


#else

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#endif

#include "canal.h"
#include "vscp.h"
#include "../../common/dllist.h"
#include "clientlist.h"
#include "guid.h"
#include "devicelist.h"

WX_DEFINE_LIST(CanalMsgOutList);
WX_DEFINE_LIST(VSCPDEVICELIST);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction CDeviceList
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CDeviceItem -- Constructor 
//

CDeviceItem::CDeviceItem()
{
    m_bQuit = false;
    m_bEnable = false;  // Default is that driver should not be started

    m_strName.Empty();      // No Device Name
    m_strParameter.Empty(); // No Parameters
    m_strPath.Empty();      // No path
    m_DeviceFlags = 0;      // Default: No flags.
    m_driverLevel = 0;      // Standard Canal messages is the default

    m_pdeviceThread = NULL;

    // No proc. addresses assigned yet
    m_proc_CanalOpen = NULL;
    m_proc_CanalClose = NULL;
    m_proc_CanalGetLevel = NULL;
    m_proc_CanalSend = NULL;
    m_proc_CanalDataAvailable = NULL;
    m_proc_CanalReceive = NULL;
    m_proc_CanalGetStatus = NULL;
    m_proc_CanalGetStatistics = NULL;
    m_proc_CanalSetFilter = NULL;
    m_proc_CanalSetMask = NULL;
    m_proc_CanalSetBaudrate = NULL;
    m_proc_CanalGetVersion = NULL;
    m_proc_CanalGetDllVersion = NULL;
    m_proc_CanalGetVendorString = NULL;

    // Generation 2
    m_proc_CanalBlockingSend = NULL;
    m_proc_CanalBlockingReceive = NULL;
    m_proc_CanalGetdriverInfo = NULL;

    
    // VSCP Level II
    m_proc_VSCPOpen = NULL;
	m_proc_VSCPClose = NULL;
	m_proc_VSCPBlockingSend = NULL;
	m_proc_VSCPBlockingReceive = NULL;
    m_proc_VSCPGetLevel = NULL;
    m_proc_VSCPGetVersion = NULL;
	m_proc_VSCPGetDllVersion = NULL;
	m_proc_VSCPGetVendorString = NULL;
    m_proc_VSCPGetdriverInfo = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// ~CDeviceItem -- Destructor

CDeviceItem::~CDeviceItem(void)
{
    ;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction CDeviceList
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CDeviceList - Constructor
//

CDeviceList::CDeviceList(void)
{
    m_deviceID = 1; // First id == 1, unassigned == 0
}


///////////////////////////////////////////////////////////////////////////////
// ~CDeviceList Destructor

CDeviceList::~CDeviceList(void)
{
    VSCPDEVICELIST::iterator iter;
    for (iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter) {
        CDeviceItem *pItem = *iter;
        if (pItem) delete pItem;
    }

    m_devItemList.Clear();

}

///////////////////////////////////////////////////////////////////////////////
// addItem
//
// An one device to the list
//

bool CDeviceList::addItem(wxString strName,
                            wxString strParameter,
                            wxString strPath,
                            uint32_t flags,
                            uint8_t *pGUID,
                            uint8_t level,
                            bool bEnable )
{
    bool rv = true;
    CDeviceItem *pDeviceItem = new CDeviceItem();
    if (NULL == pDeviceItem) return false;

    if (NULL != pDeviceItem) {

        if (wxFile::Exists(strPath)) {

            m_devItemList.Append(pDeviceItem);

            pDeviceItem->m_bEnable = bEnable;
            
            pDeviceItem->m_driverLevel = level;
            pDeviceItem->m_strName = strName;
            pDeviceItem->m_strParameter = strParameter;
            pDeviceItem->m_strPath = strPath;
            pDeviceItem->m_guid.getFromArray(pGUID);

            // Set buffer sizes and flags
            pDeviceItem->m_DeviceFlags = flags;

        } else {
            //wxGetApp.logMsg(_("Driver does not exist."), DAEMON_LOGMSG_INFO );
            delete pDeviceItem;
            rv = false;
        }

    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// removeItem
//

bool CDeviceList::removeItem(unsigned long id)
{

    return true;
}
