// DeviceList.cpp:
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifdef WIN32
#include <winsock2.h>
#endif

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

#include <canal.h>
#include <vscp.h>
#include <dllist.h>
#include <controlobject.h>
#include "clientlist.h"
#include <guid.h>
#include "devicethread.h"
#include "devicelist.h"

WX_DEFINE_LIST(Level1MsgOutList);
WX_DEFINE_LIST(VSCPDEVICELIST);

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

Driver3Process::Driver3Process( int flags ) 
                        : wxProcess( flags )
{
    ;
}

Driver3Process::~Driver3Process()
{
    ;
}

void Driver3Process::OnTerminate( int pid, int status )
{
    gpobj->logMsg(_("[Diver Level III] - Terminating.\n") );
}






//////////////////////////////////////////////////////////////////////
// Construction/Destruction CDeviceList
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CDeviceItem -- Constructor
//

CDeviceItem::CDeviceItem()
{
    m_bQuit = false;
    m_bEnable = false;      // Default is that driver should not be started

    m_translation = NO_TRANSLATION; // Default is no translation
    
    m_strName.Empty();      // No Device Name
    m_strParameter.Empty(); // No Parameters
    m_strPath.Empty();      // No path
    m_DeviceFlags = 0;      // Default: No flags.
    m_driverLevel = 0;      // Standard Canal messages is the default

    m_pdeviceThread = NULL; // No device thread started for this device

    // VSCP Level I
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
    m_proc_VSCPGetWebPageTemplate = NULL;
    m_proc_VSCPGetWebPageInfo = NULL;
    m_proc_VSCPWebPageupdate = NULL;
    
    // VSCP Level III
    m_pid = 0;
    m_pDriver3Process = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// ~CDeviceItem -- Destructor

CDeviceItem::~CDeviceItem(void)
{
    if ( NULL != m_pDriver3Process ) {
        m_pDriver3Process->Kill( m_pid );
        delete m_pDriver3Process;
        m_pDriver3Process = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
// startDriver
//
bool CDeviceItem::startDriver( CControlObject *pCtrlObject )
{
    // Must stop before we can start.
    if ( NULL != m_pdeviceThread ) {
        return false;
    }

    // Just start if enabled
    if ( !m_bEnable ) {
        pCtrlObject->logMsg(_("[Driver] - Disabled.\n") );
        return false;
    }

    // *****************************************
    //  Create the worker thread for the device
    // *****************************************

    m_pdeviceThread = new deviceThread();
    if ( NULL != m_pdeviceThread ) {

        m_pdeviceThread->m_pCtrlObject = pCtrlObject;
        m_pdeviceThread->m_pDeviceItem = this;

        wxThreadError err;
        if (wxTHREAD_NO_ERROR == (err = m_pdeviceThread->Create())) {
            if (wxTHREAD_NO_ERROR != (err = m_pdeviceThread->Run())) {
                pCtrlObject->logMsg(_("[Driver] - Unable to create DeviceThread.\n") );
            }
        }
        else {
            pCtrlObject->logMsg(_("[Driver] - Unable to run DeviceThread.\n") );
        }

    }
    else {
        pCtrlObject->logMsg(_("[Driver] - Unable to allocate memory "
                              "for DeviceThread.\n") );
    }
    
    pCtrlObject->logMsg(_("[Driver] - Started driver .") + m_strName + _("\n") );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// stopDriver
//

bool CDeviceItem::stopDriver()
{
    if ( NULL != m_pdeviceThread ) {
        
        fprintf( stderr, 
                 "CDeviceItem: Driver stop. [%s]\n",
                 (const char *)m_strName.mbc_str() );
        
        m_mutexdeviceThread.Lock();
        
        if ( NULL != m_pdeviceThread->m_preceiveThread ) {
            m_pdeviceThread->m_preceiveThread->m_bQuit = true;
            fprintf( stderr, 
                 "CDeviceItem: m_preceiveThread stop. [%s]\n",
                 (const char *)m_strName.mbc_str() );
            m_pdeviceThread->m_preceiveThread->Delete();
            m_pdeviceThread->m_preceiveThread = NULL;
        }
        
        if ( NULL != m_pdeviceThread->m_pwriteThread ) {
            m_pdeviceThread->m_pwriteThread->m_bQuit = true;
            fprintf( stderr, 
                 "CDeviceItem: m_pwriteThread stop. [%s]\n",
                 (const char *)m_strName.mbc_str() );
            m_pdeviceThread->m_pwriteThread->Delete(); 
            m_pdeviceThread->m_pwriteThread = NULL;
        }
        
        if ( NULL != m_pdeviceThread->m_preceiveLevel2Thread ) {
            m_pdeviceThread->m_preceiveLevel2Thread->m_bQuit = true;
            fprintf( stderr, 
                 "CDeviceItem: m_preceiveLevel2Thread stop. [%s]\n",
                 (const char *)m_strName.mbc_str() );
            m_pdeviceThread->m_preceiveLevel2Thread->Delete(); 
            m_pdeviceThread->m_preceiveLevel2Thread = NULL;
        }
        
        if ( NULL != m_pdeviceThread->m_pwriteLevel2Thread ) {
            m_pdeviceThread->m_pwriteLevel2Thread->m_bQuit = true;
            fprintf( stderr, 
                 "CDeviceItem: m_pwriteLevel2Thread stop. [%s]\n",
                 (const char *)m_strName.mbc_str() );
            m_pdeviceThread->m_pwriteLevel2Thread->Delete(); 
            m_pdeviceThread->m_pwriteLevel2Thread = NULL;
        }
               
        m_mutexdeviceThread.Unlock();
        
        m_bQuit = true;
        wxSleep( 1 );
        fprintf( stderr, 
                 "CDeviceItem: Driver asked to stop operation. [%s]\n",
                 (const char *)m_strName.mbc_str());        
        m_pdeviceThread->Delete();        
        m_pdeviceThread = NULL;
        
        fprintf( stderr, 
                 "CDeviceItem: Driver stopping. [%s]\n",
                 (const char *)m_strName.mbc_str());
        
        return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction CDeviceList
//////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// CDeviceList - Constructor
//

CDeviceList::CDeviceList(void)
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// ~CDeviceList Destructor

CDeviceList::~CDeviceList(void)
{
    VSCPDEVICELIST::iterator iter;
    for ( iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter ) {
        CDeviceItem *pItem = *iter;
        if ( pItem ) delete pItem;
    }

    m_devItemList.Clear();
}

///////////////////////////////////////////////////////////////////////////////
// addItem
//
// An one device to the list
//

bool CDeviceList::addItem( wxString strName,
                            wxString strParameter,
                            wxString strPath,
                            uint32_t flags,
                            cguid& guid,
                            uint8_t level,
                            bool bEnable,
                            uint32_t translation )
{
    bool rv = true;
    CDeviceItem *pDeviceItem = new CDeviceItem();
    if (NULL == pDeviceItem) return false;

    if ( NULL != pDeviceItem ) {

        if  ( wxFile::Exists( strPath ) ) {

            m_devItemList.Append( pDeviceItem );

            pDeviceItem->m_bEnable = bEnable;

            pDeviceItem->m_driverLevel = level;
            pDeviceItem->m_strName = strName;
            pDeviceItem->m_strParameter = strParameter;
            pDeviceItem->m_strPath = strPath;
            pDeviceItem->m_interface_guid = guid;

            // Set buffer sizes and flags
            pDeviceItem->m_DeviceFlags = flags;

        }
        else {
            // Driver does not exist at this path
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

///////////////////////////////////////////////////////////////////////////////
// getDeviceItemFromGUID
//

CDeviceItem *CDeviceList::getDeviceItemFromGUID( cguid& guid )
{
    CDeviceItem *returnItem = NULL;

    VSCPDEVICELIST::iterator iter;
    for ( iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter ) {
        CDeviceItem *pItem = *iter;
        if ( pItem->m_interface_guid == guid ) {
            returnItem = pItem;
            break;
        }
    }

    return returnItem;
}

///////////////////////////////////////////////////////////////////////////////
// getDeviceItemFromClientId
//

CDeviceItem *CDeviceList::getDeviceItemFromClientId( uint32_t id )
{
    CDeviceItem *returnItem = NULL;

    VSCPDEVICELIST::iterator iter;
    for ( iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter ) {
        CDeviceItem *pItem = *iter;
        if ( ( NULL != pItem->m_pClientItem  ) && 
             ( pItem->m_pClientItem->m_clientID == id ) ) {
            returnItem = pItem;
            break;
        }
    }

    return returnItem;
}