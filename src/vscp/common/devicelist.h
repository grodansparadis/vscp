// DeviceList.h: interface for the CDeviceList class.
//
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


#if !defined(AFX_DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_)
#define AFX_DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_

#ifdef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif


#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"


#ifdef WIN32

#else

#include <pthread.h>
#include <semaphore.h>

#endif

#include "../common/canaldlldef.h"

//#include "controlobject.h"
#include "vscpdlldef.h"
#include "../../common/dllist.h"
#include "clientlist.h"
#include "devicethread.h"

enum _driver_levels {
    VSCP_DRIVER_LEVEL1 = 0,  
    VSCP_DRIVER_LEVEL2
};

class CClientItem;
class cguid;

WX_DECLARE_LIST ( canalMsg, CanalMsgOutList );



class CDeviceItem
{

public:

	/// Constructor
	CDeviceItem();

	/// Destructor
	virtual ~CDeviceItem();


    bool startDriver( CControlObject *pCtrlObject );


    bool stopDriver( void );


	/*!
		Name of device
	*/
    wxString m_strName;

	/*!
		Device configuration string
	*/
    wxString m_strParameter;

	/*!
		CANAL DLL/DL path
	*/
    wxString m_strPath;

	/*!
		Canal Driver Level
	*/
	uint32_t m_driverLevel;
	
	/// True if driver should be started.
	bool m_bEnable;

	/*!
		termination control
	*/
	bool m_bQuit;
	
	/*!
		GUID to use for driver interface if set
	 */
	cguid m_guid;

	/*!
		Worker thread for device
	*/
	deviceThread *m_pdeviceThread;

	wxMutex m_mutexdeviceThread;

	/*!
		Device flags for CANAL DLL open
	*/
	uint32_t m_DeviceFlags;

	/*!
		Client entry
	*/
	CClientItem *m_pClientItem;

	/*!
		Mutex handle that is used for sharing of the device.
	*/
	wxMutex m_deviceMutex;


	// Handle for dll/dl driver interface
	long m_openHandle;

	// Level I (CANAL) driver methods
	LPFNDLL_CANALOPEN			        m_proc_CanalOpen;
	LPFNDLL_CANALCLOSE				    m_proc_CanalClose;
	LPFNDLL_CANALGETLEVEL			    m_proc_CanalGetLevel;
	LPFNDLL_CANALSEND				    m_proc_CanalSend;
	LPFNDLL_CANALRECEIVE			    m_proc_CanalReceive;
	LPFNDLL_CANALDATAAVAILABLE		    m_proc_CanalDataAvailable;
	LPFNDLL_CANALGETSTATUS			    m_proc_CanalGetStatus;
	LPFNDLL_CANALGETSTATISTICS		    m_proc_CanalGetStatistics;
	LPFNDLL_CANALSETFILTER			    m_proc_CanalSetFilter;
	LPFNDLL_CANALSETMASK			    m_proc_CanalSetMask;
	LPFNDLL_CANALSETBAUDRATE		    m_proc_CanalSetBaudrate;
	LPFNDLL_CANALGETVERSION			    m_proc_CanalGetVersion;
	LPFNDLL_CANALGETDLLVERSION		    m_proc_CanalGetDllVersion;
	LPFNDLL_CANALGETVENDORSTRING	    m_proc_CanalGetVendorString;
	// Generation 2
	LPFNDLL_CANALBLOCKINGSEND		    m_proc_CanalBlockingSend;
	LPFNDLL_CANALBLOCKINGRECEIVE	    m_proc_CanalBlockingReceive;
	LPFNDLL_CANALGETDRIVERINFO		    m_proc_CanalGetdriverInfo;

    // Level II driver methods
    LPFNDLL_VSCPOPEN			        m_proc_VSCPOpen;
	LPFNDLL_VSCPCLOSE				    m_proc_VSCPClose;
	LPFNDLL_VSCPBLOCKINGSEND			m_proc_VSCPBlockingSend;
	LPFNDLL_VSCPBLOCKINGRECEIVE			m_proc_VSCPBlockingReceive;
    LPFNDLL_VSCPGETLEVEL				m_proc_VSCPGetLevel;
    LPFNDLL_VSCPGETVERSION			    m_proc_VSCPGetVersion;
	LPFNDLL_VSCPGETDLLVERSION		    m_proc_VSCPGetDllVersion;
	LPFNDLL_VSCPGETVENDORSTRING	        m_proc_VSCPGetVendorString;
    LPFNDLL_VSCPGETDRIVERINFO		    m_proc_VSCPGetdriverInfo;
	LPFNDLL_VSCPGETWEBPAGETEMPLATE		m_proc_VSCPGetWebPageTemplate;
	LPFNDLL_VSCPGETWEBPAGEINFO			m_proc_VSCPGetWebPageInfo;
	LPFNDLL_VSCPWEBPAGEUPDATE			m_proc_VSCPWebPageupdate;
};

// List with device items
WX_DECLARE_LIST( CDeviceItem, VSCPDEVICELIST );

class CDeviceList  
{
public:
	CDeviceList();
	virtual ~CDeviceList();

	/*!
		Add one driver item
	*/
	bool addItem( wxString strName,
					        wxString strParameters, 
					        wxString strPath, 
					        uint32_t flags,
					        uint8_t *pGUID,
                            uint8_t level = VSCP_DRIVER_LEVEL1,
							bool bEnable = true );

	/*!
		Remove a driver item
		@param id for the driver to removeItem
		@return True if driver was removed successfully 
						otherwise false.
	*/
 	bool removeItem( unsigned long id );

public:
	
  /*!
    Hash map with devices
  */
	VSCPDEVICELIST m_devItemList;


protected:

	/*!
		System assigned ID for device
	*/
	uint32_t m_deviceID;

};

#endif // !defined(AFX_DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_)


