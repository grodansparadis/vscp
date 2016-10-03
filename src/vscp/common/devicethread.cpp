// devicethread.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2016
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

#ifdef WIN32
#include <winsock2.h>
#endif

//#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"

#ifdef WIN32

#else

#define _POSIX

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#include "daemonvscp.h"
#include <canal_win32_ipc.h>
#include <canal_macro.h>
#include <vscp.h>
#include <vscpdlldef.h>
#include <vscphelper.h>
#include <dllist.h>
//#include <md5.h>
#include <controlobject.h>
#include "devicethread.h"


///////////////////////////////////////////////////////////////////////////////
// deviceThread
//

deviceThread::deviceThread() : wxThread(wxTHREAD_JOINABLE)
{
    m_pDeviceItem = NULL;
    m_pCtrlObject = NULL;
    m_preceiveThread = NULL;
    m_pwriteThread = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// stopDriver
//

deviceThread::~deviceThread()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceThread::Entry()
{
    // Must have a valid pointer to the device item
    if (NULL == m_pDeviceItem) return NULL;

    // Must have a valid pointer to the control object
    if (NULL == m_pCtrlObject) return NULL;

    // We need to create a clientobject and add this object to the list
    m_pDeviceItem->m_pClientItem = new CClientItem;
    if (NULL == m_pDeviceItem->m_pClientItem) {
        return NULL;
    }

    // This is now an active Client
    m_pDeviceItem->m_pClientItem->m_bOpen = true;
    if ( VSCP_DRIVER_LEVEL2 == m_pDeviceItem->m_driverLevel ) {
        m_pDeviceItem->m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL2;
    }
    else {
        m_pDeviceItem->m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1;
    }
    m_pDeviceItem->m_pClientItem->m_strDeviceName = m_pDeviceItem->m_strName;
    m_pDeviceItem->m_pClientItem->m_strDeviceName += _(" Started at ");
    wxDateTime now = wxDateTime::Now();
    m_pDeviceItem->m_pClientItem->m_strDeviceName += now.FormatISODate();
    m_pDeviceItem->m_pClientItem->m_strDeviceName += _(" ");
    m_pDeviceItem->m_pClientItem->m_strDeviceName += now.FormatISOTime();

    m_pCtrlObject->logMsg(m_pDeviceItem->m_pClientItem->m_strDeviceName + _("\n"),
                            DAEMON_LOGMSG_DEBUG);

    // Add the client to the Client List
    m_pCtrlObject->m_wxClientMutex.Lock();
    m_pCtrlObject->addClient(m_pDeviceItem->m_pClientItem);
    m_pCtrlObject->m_wxClientMutex.Unlock();

    // Load dynamic library
    if (!m_wxdll.Load(m_pDeviceItem->m_strPath, wxDL_LAZY)) {
        m_pCtrlObject->logMsg(_("Unable to load dynamic library.\n") );
        return NULL;
    }

    if (VSCP_DRIVER_LEVEL1 == m_pDeviceItem->m_driverLevel) {

        // Now find methods in library

        {
            wxString str;
            str = _("Loading level I driver: ");
            str += m_pDeviceItem->m_strName;
            str += _("\n");
            m_pCtrlObject->logMsg(str);
            wxLogDebug(str);
        }

        // * * * * CANAL OPEN * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalOpen =
            (LPFNDLL_CANALOPEN) m_wxdll.GetSymbol(_T("CanalOpen")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalOpen.\n") );
            return NULL;
        }

        // * * * * CANAL CLOSE * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalClose =
            (LPFNDLL_CANALCLOSE) m_wxdll.GetSymbol(_T("CanalClose")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalClose.\n") );
            return NULL;
        }

        // * * * * CANAL GETLEVEL * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetLevel =
            (LPFNDLL_CANALGETLEVEL) m_wxdll.GetSymbol(_T("CanalGetLevel")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalGetLevel.\n") );
            return NULL;
        }

        // * * * * CANAL SEND * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalSend =
            (LPFNDLL_CANALSEND) m_wxdll.GetSymbol(_T("CanalSend")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalSend.\n") );
            return NULL;
        }

        // * * * * CANAL DATA AVAILABLE * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalDataAvailable =
            (LPFNDLL_CANALDATAAVAILABLE) m_wxdll.GetSymbol(_T("CanalDataAvailable")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalDataAvailable.\n") );
            return NULL;
        }


        // * * * * CANAL RECEIVE * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalReceive =
            (LPFNDLL_CANALRECEIVE) m_wxdll.GetSymbol(_T("CanalReceive")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalReceive.\n") );
            return NULL;
        }

        // * * * * CANAL GET STATUS * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetStatus =
            (LPFNDLL_CANALGETSTATUS) m_wxdll.GetSymbol(_T("CanalGetStatus")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalGetStatus.\n") );
            return NULL;
        }

        // * * * * CANAL GET STATISTICS * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetStatistics =
            (LPFNDLL_CANALGETSTATISTICS) m_wxdll.GetSymbol(_T("CanalGetStatistics")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalGetStatistics.\n") );
            return NULL;
        }

        // * * * * CANAL SET FILTER * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalSetFilter =
            (LPFNDLL_CANALSETFILTER) m_wxdll.GetSymbol(_T("CanalSetFilter")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalSetFilter.\n") );
            return NULL;
        }

        // * * * * CANAL SET MASK * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalSetMask =
            (LPFNDLL_CANALSETMASK) m_wxdll.GetSymbol(_T("CanalSetMask")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalSetMask.\n") );
            return NULL;
        }

        // * * * * CANAL GET VERSION * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetVersion =
            (LPFNDLL_CANALGETVERSION) m_wxdll.GetSymbol(_T("CanalGetVersion")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalGetVersion.\n") );
            return NULL;
        }

        // * * * * CANAL GET DLL VERSION * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetDllVersion =
            (LPFNDLL_CANALGETDLLVERSION) m_wxdll.GetSymbol(_T("CanalGetDllVersion")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalGetDllVersion.\n") );
            return NULL;
        }

        // * * * * CANAL GET VENDOR STRING * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetVendorString =
            (LPFNDLL_CANALGETVENDORSTRING) m_wxdll.GetSymbol(_T("CanalGetVendorString")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalGetVendorString.\n") );
            return NULL;
        }


        // ******************************
        //     Generation 2 Methods
        // ******************************


        // * * * * CANAL BLOCKING SEND * * * *
        m_pDeviceItem->m_proc_CanalBlockingSend = NULL;
        if (m_wxdll.HasSymbol(_T("CanalBlockingSend"))) {
            if (NULL == (m_pDeviceItem->m_proc_CanalBlockingSend =
                (LPFNDLL_CANALBLOCKINGSEND) m_wxdll.GetSymbol(_T("CanalBlockingSend")))) {
                m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalBlockingSend. Probably Generation 1 driver.\n") );
                m_pDeviceItem->m_proc_CanalBlockingSend = NULL;
            }
        }
        else {
            m_pCtrlObject->logMsg(_T("CanalBlockingSend not available. \n\tNon blocking operations set.\n") );
        }

        // * * * * CANAL BLOCKING RECEIVE * * * *
        m_pDeviceItem->m_proc_CanalBlockingReceive = NULL;
        if (m_wxdll.HasSymbol(_T("CanalBlockingReceive"))) {
            if (NULL == (m_pDeviceItem->m_proc_CanalBlockingReceive =
                (LPFNDLL_CANALBLOCKINGRECEIVE) m_wxdll.GetSymbol(_T("CanalBlockingReceive")))) {
                m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalBlockingReceive. Probably Generation 1 driver.\n") );
                m_pDeviceItem->m_proc_CanalBlockingReceive = NULL;
            }
        }
        else {
            m_pCtrlObject->logMsg(_T("CanalBlockingReceive not available. \n\tNon blocking operations set.\n"));
        }

        // * * * * CANAL GET DRIVER INFO * * * *
        m_pDeviceItem->m_proc_CanalGetdriverInfo = NULL;
        if (m_wxdll.HasSymbol(_T("CanalGetDriverInfo"))) {
            if (NULL == (m_pDeviceItem->m_proc_CanalGetdriverInfo =
                (LPFNDLL_CANALGETDRIVERINFO) m_wxdll.GetSymbol(_T("CanalGetDriverInfo")))) {
                m_pCtrlObject->logMsg(_T("Unable to get dl entry for CanalGetDriverInfo. Probably Generation 1 driver.\n") );
                m_pDeviceItem->m_proc_CanalGetdriverInfo = NULL;
            }
        }

        // Open the device
        m_pDeviceItem->m_openHandle =
            m_pDeviceItem->m_proc_CanalOpen((const char *) m_pDeviceItem->m_strParameter.mb_str(wxConvUTF8),
                                                m_pDeviceItem->m_DeviceFlags);

        // Check if the driver opened properly
        if (m_pDeviceItem->m_openHandle <= 0) {
            wxString errMsg = _T("Failed to open driver. Will not use it! \n\t[ ")
                + m_pDeviceItem->m_strName + _T(" ]\n");
            m_pCtrlObject->logMsg( errMsg );
            return NULL;
        }
        else {
            wxString wxstr =
                wxString::Format(_("Driver %s opended.\n"),
                                    (const char *)m_pDeviceItem->m_strName.mbc_str() );
            m_pCtrlObject->logMsg( wxstr );
        }

        // Get Driver Level
        m_pDeviceItem->m_driverLevel =
                m_pDeviceItem->m_proc_CanalGetLevel(m_pDeviceItem->m_openHandle);


        //  * * * Level I Driver * * *

        // Check if blocking driver is available
        if (NULL != m_pDeviceItem->m_proc_CanalBlockingReceive) {

            // * * * * Blocking version * * * *

            /////////////////////////////////////////////////////////////////////////////
            // Device write worker thread
            /////////////////////////////////////////////////////////////////////////////
            m_pwriteThread = new deviceLevel1WriteThread;

            if (m_pwriteThread) {
                m_pwriteThread->m_pMainThreadObj = this;
                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = m_pwriteThread->Create())) {
                    m_pwriteThread->SetPriority(WXTHREAD_MAX_PRIORITY);
                    if (wxTHREAD_NO_ERROR != (err = m_pwriteThread->Run())) {
                        m_pCtrlObject->logMsg(_("Unable to run device write worker thread.\n") );
                    }
                }
                else {
                    m_pCtrlObject->logMsg(_("Unable to create device write worker thread.\n") );
                }
            }
            else {
                m_pCtrlObject->logMsg(_("Unable to allocate memory for device write worker thread.\n") );
            }

            /////////////////////////////////////////////////////////////////////////////
            // Device read worker thread
            /////////////////////////////////////////////////////////////////////////////
            m_preceiveThread = new deviceLevel1ReceiveThread;

            if (m_preceiveThread) {
                m_preceiveThread->m_pMainThreadObj = this;
                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = m_preceiveThread->Create())) {
                    m_preceiveThread->SetPriority(WXTHREAD_MAX_PRIORITY);
                    if (wxTHREAD_NO_ERROR != (err = m_preceiveThread->Run())) {
                        m_pCtrlObject->logMsg(_("Unable to run device receive worker thread.\n") );
                    }
                }
                else {
                    m_pCtrlObject->logMsg(_("Unable to create device receive worker thread.\n") );
                }
            }
            else {
                m_pCtrlObject->logMsg(_("Unable to allocate memory for device receive worker thread.\n") );
            }

            // Just sit and wait until the end of the world as we know it...
            while (!m_pDeviceItem->m_bQuit) {
                wxSleep(1);
            }

            m_preceiveThread->m_bQuit = true;
            m_pwriteThread->m_bQuit = true;
            m_preceiveThread->Wait();
            m_pwriteThread->Wait();
        }
        else {

            // * * * * Non blocking version * * * *

            bool bActivity;
            while (!TestDestroy() && !m_pDeviceItem->m_bQuit) {

                bActivity = false;
                /////////////////////////////////////////////////////////////////////////////
                //                           Receive from device						   //
                /////////////////////////////////////////////////////////////////////////////
                canalMsg msg;
                if (m_pDeviceItem->m_proc_CanalDataAvailable(m_pDeviceItem->m_openHandle)) {

                    if (CANAL_ERROR_SUCCESS ==
                        m_pDeviceItem->m_proc_CanalReceive(m_pDeviceItem->m_openHandle, &msg)) {

                        bActivity = true;

                        // There must be room in the receive queue
                        if (m_pCtrlObject->m_maxItemsInClientReceiveQueue >
                            m_pCtrlObject->m_clientOutputQueue.GetCount()) {

                            vscpEvent *pvscpEvent = new vscpEvent;
                            if (NULL != pvscpEvent) {

                                // Convert CANAL message to VSCP event
                                vscp_convertCanalToEvent( pvscpEvent,
                                                            &msg,
                                                            m_pDeviceItem->m_pClientItem->m_guid.m_id );

                                pvscpEvent->obid = m_pDeviceItem->m_pClientItem->m_clientID;

                                m_pCtrlObject->m_mutexClientOutputQueue.Lock();
                                m_pCtrlObject->m_clientOutputQueue.Append(pvscpEvent);
                                m_pCtrlObject->m_semClientOutputQueue.Post();
                                m_pCtrlObject->m_mutexClientOutputQueue.Unlock();

                            }
                        }
                    }
                } // data available


                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                //             Send messages (if any) in the outqueue
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                // Check if there is something to send
                if (m_pDeviceItem->m_pClientItem->m_clientInputQueue.GetCount()) {

                    bActivity = true;

                    CLIENTEVENTLIST::compatibility_iterator nodeClient;

                    m_pDeviceItem->m_pClientItem->m_mutexClientInputQueue.Lock();
                    nodeClient = m_pDeviceItem->m_pClientItem->m_clientInputQueue.GetFirst();
                    vscpEvent *pqueueEvent = nodeClient->GetData();
                    m_pDeviceItem->m_pClientItem->m_mutexClientInputQueue.Unlock();

                    canalMsg canalMsg;
                    vscp_convertEventToCanal(&canalMsg, pqueueEvent);
                    if (CANAL_ERROR_SUCCESS ==
                        m_pDeviceItem->m_proc_CanalSend(m_pDeviceItem->m_openHandle, &canalMsg)) {
                        // Remove the node
                        delete pqueueEvent;
                        m_pDeviceItem->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
                    }
                    else {
                        // Another try
                        //m_pCtrlObject->m_semClientOutputQueue.Post();
                        vscp_deleteVSCPevent(pqueueEvent);
                        m_pDeviceItem->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
                    }

                } // events

                if (!bActivity) {
                    ::wxMilliSleep(100);
                }

                bActivity = false;

            } // while working - non blocking

        } // if blocking/non blocking


        // Close CANAL channel
        m_pDeviceItem->m_proc_CanalClose(m_pDeviceItem->m_openHandle);

        // Library is unloaded in destructor

        // Remove messages in the client queues
        m_pCtrlObject->m_wxClientMutex.Lock();
        m_pCtrlObject->removeClient(m_pDeviceItem->m_pClientItem);
        m_pCtrlObject->m_wxClientMutex.Unlock();

        if (NULL != m_preceiveThread) {
            m_preceiveThread->Wait();
            delete m_preceiveThread;
        }

        if (NULL != m_pwriteThread) {
            m_pwriteThread->Wait();
            delete m_pwriteThread;
        }

    }
    else if (VSCP_DRIVER_LEVEL2 == m_pDeviceItem->m_driverLevel) {

        // Now find methods in library
        {
            wxString str;
            str = _("Loading level II driver: <");
            str += m_pDeviceItem->m_strName;
            str += _(">");
            str += _("\n");
            m_pCtrlObject->logMsg(str);
        }

        // * * * * VSCP OPEN * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPOpen =
            (LPFNDLL_VSCPOPEN) m_wxdll.GetSymbol(_T("VSCPOpen")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPOpen.\n"));
            return NULL;
        }

        // * * * * VSCP CLOSE * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPClose =
            (LPFNDLL_VSCPCLOSE) m_wxdll.GetSymbol(_T("VSCPClose")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPClose.\n"));
            return NULL;
        }

        // * * * * VSCP BLOCKINGSEND * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPBlockingSend =
            (LPFNDLL_VSCPBLOCKINGSEND) m_wxdll.GetSymbol(_T("VSCPBlockingSend")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPBlockingSend.\n"));
            return NULL;
        }

        // * * * * VSCP BLOCKINGRECEIVE * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPBlockingReceive =
            (LPFNDLL_VSCPBLOCKINGRECEIVE) m_wxdll.GetSymbol(_T("VSCPBlockingReceive")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPBlockingReceive.\n"));
            return NULL;
        }

        // * * * * VSCP GETLEVEL * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPGetLevel =
            (LPFNDLL_VSCPGETLEVEL) m_wxdll.GetSymbol(_T("VSCPGetLevel")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPGetLevel.\n"));
            return NULL;
        }

        // * * * * VSCP GET DLL VERSION * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPGetDllVersion =
            (LPFNDLL_VSCPGETDLLVERSION) m_wxdll.GetSymbol(_T("VSCPGetDllVersion")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPGetDllVersion.\n"));
            return NULL;
        }

        // * * * * VSCP GET VENDOR STRING * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPGetVendorString =
            (LPFNDLL_VSCPGETVENDORSTRING) m_wxdll.GetSymbol(_T("VSCPGetVendorString")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPGetVendorString.\n"));
            return NULL;
        }

        // * * * * VSCP GET DRIVER INFO * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetdriverInfo =
            (LPFNDLL_VSCPGETVENDORSTRING) m_wxdll.GetSymbol(_T("VSCPGetDriverInfo")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPGetDriverInfo.\n"));
            return NULL;
        }

        // * * * * VSCP GET WEB PAGE TEMPLATE * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPGetWebPageTemplate =
            (LPFNDLL_VSCPGETWEBPAGETEMPLATE) m_wxdll.GetSymbol(_T("VSCPGetWebPageTemplate")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPGetWebPageTemplate.\n"));
            return NULL;
        }

        // * * * * VSCP GET WEB PAGE INFO * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPGetWebPageInfo =
            (LPFNDLL_VSCPGETWEBPAGEINFO) m_wxdll.GetSymbol(_T("VSCPGetWebPageInfo")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPGetWebPageInfo.\n"));
            return NULL;
        }

        // * * * * VSCP WEB PAGE UPDATE * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPWebPageupdate =
            (LPFNDLL_VSCPWEBPAGEUPDATE) m_wxdll.GetSymbol(_T("VSCPWebPageupdate")))) {
            // Free the library
            m_pCtrlObject->logMsg(_T("Unable to get dl entry for VSCPWebPageupdate.\n"));
            return NULL;
        }

        m_pCtrlObject->logMsg(_("Discovered all methods\n"));

        // Username, password, host and port can be set in configuration file. Read in them here
        // if they are.
        wxString strHost(_("127.0.0.1:9598"));

        wxStringTokenizer tkz(m_pDeviceItem->m_strParameter, _(";"));
        if (tkz.HasMoreTokens()) {

            CVSCPVariable variable;

            // Get prefix
            wxString prefix = tkz.GetNextToken();

            // Check if username is specified in the configuration file            
            if ( m_pCtrlObject->m_VSCP_Variables.find(m_pDeviceItem->m_strName + _("_username"), variable ) ) {
                wxString str;
                if (VSCP_DAEMON_VARIABLE_CODE_STRING == variable.getType()) {
                    str = variable.getValue();
                    m_pCtrlObject->m_driverUsername = str;
                }
            }

            // Check if password is specified in the configuration file            
            if ( m_pCtrlObject->m_VSCP_Variables.find(m_pDeviceItem->m_strName + _("_password"), variable ) ) {
                wxString str;
                if (VSCP_DAEMON_VARIABLE_CODE_STRING == variable.getType()) {
                    str = variable.getValue();
                    m_pCtrlObject->m_driverPassword = str;
                }
            }

            // Check if host is specified in the configuration file       
            if ( m_pCtrlObject->m_VSCP_Variables.find(m_pDeviceItem->m_strName + _("_host"), variable ) ) {
                wxString str;
                if (VSCP_DAEMON_VARIABLE_CODE_STRING == variable.getType()) {
                    str = variable.getValue();
                    strHost = str;
                }
            }

        }

        // Open up the driver
        m_pDeviceItem->m_openHandle =
            m_pDeviceItem->m_proc_VSCPOpen( m_pCtrlObject->m_driverUsername.mbc_str(),
                                                ( const char * )m_pCtrlObject->m_driverPassword.mbc_str(),
                                                ( const char * )strHost.mbc_str(),
                                                0,
                                                ( const char * )m_pDeviceItem->m_strName.mbc_str(),
                                                ( const char * )m_pDeviceItem->m_strParameter.mbc_str() );

        if ( 0 == m_pDeviceItem->m_openHandle ) {
            // Free the library
            m_pCtrlObject->logMsg( _T( "Unable to open the library.\n" ) );
            return NULL;
        }

        /////////////////////////////////////////////////////////////////////////////
        // Device write worker thread
        /////////////////////////////////////////////////////////////////////////////

        m_pwriteLevel2Thread = new deviceLevel2WriteThread;

        if (m_pwriteLevel2Thread) {
            m_pwriteLevel2Thread->m_pMainThreadObj = this;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pwriteLevel2Thread->Create())) {
                m_pwriteLevel2Thread->SetPriority(WXTHREAD_MAX_PRIORITY);
                if (wxTHREAD_NO_ERROR != (err = m_pwriteLevel2Thread->Run())) {
                    m_pCtrlObject->logMsg(_("Unable to run device write worker thread."));
                }
            }
            else {
                m_pCtrlObject->logMsg(_("Unable to create device write worker thread."));
            }
        }
        else {
            m_pCtrlObject->logMsg(_("Unable to allocate memory for device write worker thread."));
        }

        /////////////////////////////////////////////////////////////////////////////
        // Device read worker thread
        /////////////////////////////////////////////////////////////////////////////

        m_preceiveLevel2Thread = new deviceLevel2ReceiveThread;

        if (m_preceiveLevel2Thread) {
            m_preceiveLevel2Thread->m_pMainThreadObj = this;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_preceiveLevel2Thread->Create())) {
                m_preceiveLevel2Thread->SetPriority(WXTHREAD_MAX_PRIORITY);
                if (wxTHREAD_NO_ERROR != (err = m_preceiveLevel2Thread->Run())) {
                    m_pCtrlObject->logMsg(_("Unable to run device receive worker thread."));
                }
            }
            else {
                m_pCtrlObject->logMsg(_("Unable to create device receive worker thread.") );
            }
        }
        else {
            m_pCtrlObject->logMsg(_("Unable to allocate memory for device receive worker thread.") );
        }

        // Just sit and wait until the end of the world as we know it...
        while (!TestDestroy() && !m_pDeviceItem->m_bQuit) {
            wxSleep(200);
        }

        m_preceiveLevel2Thread->m_bQuit = true;
        m_pwriteLevel2Thread->m_bQuit = true;

        // Close channel
        m_pDeviceItem->m_proc_VSCPClose(m_pDeviceItem->m_openHandle);

        // Library is unloaded in destructor

        // Remove messages in the client queues
        m_pCtrlObject->m_wxClientMutex.Lock();
        m_pCtrlObject->removeClient(m_pDeviceItem->m_pClientItem);
        m_pCtrlObject->m_wxClientMutex.Unlock();

        if (NULL != m_preceiveLevel2Thread) {
            m_preceiveLevel2Thread->Wait();
            delete m_preceiveLevel2Thread;
        }

        if (NULL != m_pwriteLevel2Thread) {
            m_pwriteLevel2Thread->Wait();
            delete m_pwriteLevel2Thread;
        }

    }




    //
    // =====================================================================================
    //




    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceThread::OnExit()
{

}


// ****************************************************************************


///////////////////////////////////////////////////////////////////////////////
// deviceCanalReceiveThread
//

deviceLevel1ReceiveThread::deviceLevel1ReceiveThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_pMainThreadObj = NULL;
    m_bQuit = false;
}

deviceLevel1ReceiveThread::~deviceLevel1ReceiveThread()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceLevel1ReceiveThread::Entry()
{
    canalMsg msg;
    Level1MsgOutList::compatibility_iterator nodeLevel1;

    // Must be a valid main object pointer
    if (NULL == m_pMainThreadObj) return NULL;

    // Blocking receive method must have been found
    if (NULL == m_pMainThreadObj->m_pDeviceItem->m_proc_CanalBlockingReceive) return NULL;

    while (!TestDestroy() && !m_bQuit) {

        if (CANAL_ERROR_SUCCESS ==
                m_pMainThreadObj->m_pDeviceItem->m_proc_CanalBlockingReceive(
                                                            m_pMainThreadObj->m_pDeviceItem->m_openHandle, &msg, 500 ) ) {

            // There must be room in the receive queue
            if (m_pMainThreadObj->m_pCtrlObject->m_maxItemsInClientReceiveQueue >
                    m_pMainThreadObj->m_pCtrlObject->m_clientOutputQueue.GetCount()) {

                vscpEvent *pvscpEvent = new vscpEvent;
                if (NULL != pvscpEvent) {

                    memset( pvscpEvent, 0, sizeof( vscpEvent ) );

                    // Convert CANAL message to VSCP event
                    vscp_convertCanalToEvent( pvscpEvent,
                                                &msg,
                                                m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_guid.m_id );

                    pvscpEvent->obid = m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_clientID;

                    // If no GUID is set,
                    //      - Set driver GUID if it is defined
                    //      - Set to interface GUID if not.

                    uint8_t ifguid[16];

                    // Save nickname
                    uint8_t nickname_lsb = pvscpEvent->GUID[15];

                    // Set if to use
                    memcpy( ifguid, pvscpEvent->GUID, 16 );
                    ifguid[14] = 0;
                    ifguid[15] = 0;

                    // If if is set to zero use interface id
                    if ( vscp_isGUIDEmpty( ifguid ) ) {

                        // Set driver GUID if set
                        if ( !m_pMainThreadObj->m_pDeviceItem->m_interface_guid.isNULL() ) {
                            m_pMainThreadObj->m_pDeviceItem->m_interface_guid.writeGUID( pvscpEvent->GUID );
                        }
                        else {
                            // If no driver GUID set use interface GUID
                            m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_guid.writeGUID( pvscpEvent->GUID );
                        }

                        // Preserve nickname
                        pvscpEvent->GUID[15] = nickname_lsb;

                    }
                    
                    // =========================================================
                    //                   Outgoing translations
                    // =========================================================
                    
                    // Level I measurement events to Level II measurement float
                    if ( m_pMainThreadObj->m_pDeviceItem->m_translation & 
                            VSCP_DRIVER_OUT_TR_M1M2F ) {
                        
                        double val64;
                        if ( vscp_getVSCPMeasurementAsDouble( pvscpEvent, &val64 ) ) {
                            
                            pvscpEvent->vscp_class = VSCP_CLASS2_MEASUREMENT_FLOAT;
                            uint8_t *p = new uint8_t[ 12 ];
                            if ( NULL != p ) {
                                
                                memset( p, 0, 12 );
/*                                
0 	Index for sensor, 0-255.
1 	Zone, 0-255.
2 	Sub zone, 0-255.
3 	Unit from measurements, 0-255.
4-11 	64-bit double precision floating point value stored MSB first. 
*/ 
                                if ( VSCP_CLASS1_MEASUREMENT == pvscpEvent->vscp_class ) {
                                    
                                    // Sensor index
                                    p[ 0 ] = pvscpEvent->pdata[ 0 ] & VSCP_MASK_DATACODING_INDEX;
                                    
                                    // Zone + Subzone
                                    p[ 1 ] = p[ 2 ] = 0x00;
                                    
                                    // unit
                                    p[ 3 ] = (pvscpEvent->pdata[ 0 ] & VSCP_MASK_DATACODING_UNIT) >> 3;
                                    
                                    // Floating point value
                                    val64 = wxUINT64_SWAP_ON_LE(val64);
                                    memcpy( p + 4, &val64, sizeof( val64 ) );
                                    
                                    delete [] pvscpEvent->pdata;
                                    
                                    pvscpEvent->pdata = p;
                                    
                                }
                                else if ( VSCP_CLASS1_MEASUREMENT64 == pvscpEvent->vscp_class ) {
                                    
                                    // Index = 0, Unit = 0, Zone = 0, Subzone = 0
                                    // Floating point value
                                    val64 = wxUINT64_SWAP_ON_LE(val64);
                                    memcpy( p + 4, &val64, sizeof( val64 ) );
                                    
                                    delete [] pvscpEvent->pdata;
                                    
                                    pvscpEvent->pdata = p;
                                    
                                }
                                else if ( VSCP_CLASS1_MEASUREMENT32 == pvscpEvent->vscp_class ) {
                                    
                                    // Index = 0, Unit = 0, Zone = 0, Subzone = 0
                                    // Floating point value
                                    val64 = wxUINT64_SWAP_ON_LE(val64);
                                    memcpy( p + 4, &val64, sizeof( val64 ) );
                                    
                                    delete [] pvscpEvent->pdata;
                                    
                                    pvscpEvent->pdata = p;
                                    
                                }
                                else if ( VSCP_CLASS1_MEASUREZONE == pvscpEvent->vscp_class ) {
                                    
                                    // Sensor index
                                    p[ 0 ] = pvscpEvent->pdata[ 0 ];
                                    
                                    // Zone
                                    p[ 1 ] = pvscpEvent->pdata[ 1 ];
                                    
                                    // Subzone
                                    p[ 2 ] = pvscpEvent->pdata[ 2 ];
                                            
                                    val64 = wxUINT64_SWAP_ON_LE(val64);
                                    memcpy( p + 4, &val64, sizeof( val64 ) );
                                    
                                    delete [] pvscpEvent->pdata;
                                    
                                    pvscpEvent->pdata = p;
                                    
                                }                                
                                else if ( VSCP_CLASS1_SETVALUEZONE == pvscpEvent->vscp_class ) {
                                    
                                    // Sensor index
                                    p[ 0 ] = pvscpEvent->pdata[ 0 ];
                                    
                                    // Zone
                                    p[ 1 ] = pvscpEvent->pdata[ 1 ];
                                    
                                    // Subzone
                                    p[ 2 ] = pvscpEvent->pdata[ 2 ];
                                            
                                    val64 = wxUINT64_SWAP_ON_LE(val64);
                                    memcpy( p + 4, &val64, sizeof( val64 ) );
                                    
                                    delete [] pvscpEvent->pdata;
                                    
                                    pvscpEvent->pdata = p;
                                    
                                }
                            }
                        }
                    }
                    
                    // Level I measurement events to Level II measurement string
                    if ( m_pMainThreadObj->m_pDeviceItem->m_translation & 
                            VSCP_DRIVER_OUT_TR_M1M2S ) {
                        
                        wxString strval;
                        if ( vscp_getVSCPMeasurementAsString( pvscpEvent, strval ) ) {
                            
                            pvscpEvent->vscp_class = VSCP_CLASS2_MEASUREMENT_STR;
                            
                            char *p = new char[ 4 + strval.Length() ];
                            if ( NULL != p ) {
                                
                                memset( p, 0, 4 + strval.Length() );

                                if ( VSCP_CLASS1_MEASUREMENT == pvscpEvent->vscp_class ) {
                                    
                                    // Sensor index
                                    p[ 0 ] = pvscpEvent->pdata[ 0 ] & VSCP_MASK_DATACODING_INDEX;
                                    
                                    // Zone + Subzone
                                    p[ 1 ] = p[ 2 ] = 0x00;
                                    
                                    // unit
                                    p[ 3 ] = (pvscpEvent->pdata[ 0 ] & VSCP_MASK_DATACODING_UNIT) >> 3;
                                    
                                    // Copy in the value string
                                    strcpy( p + 4, (const char *)strval.mbc_str() );
                                    
                                    delete [] pvscpEvent->pdata;
                                    
                                    pvscpEvent->pdata = (uint8_t *)p;
                                    
                                }
                                else if ( VSCP_CLASS1_MEASUREMENT64 == pvscpEvent->vscp_class ) {
                                    
                                    // Index = 0, Unit = 0, Zone = 0, Subzone = 0
                                    // Floating point value
                                    // Copy in the value string
                                    strcpy( p + 4, (const char *)strval.mbc_str() );
                                    
                                    delete [] pvscpEvent->pdata;
                                    
                                    pvscpEvent->pdata = (uint8_t *)p;
                                    
                                }
                                else if ( VSCP_CLASS1_MEASUREMENT32 == pvscpEvent->vscp_class ) {
                                    
                                    // Index = 0, Unit = 0, Zone = 0, Subzone = 0
                                    // Floating point value
                                    // Copy in the value string
                                    strcpy( p + 4, (const char *)strval.mbc_str() );
                                    
                                    delete [] pvscpEvent->pdata;
                                    
                                    pvscpEvent->pdata = (uint8_t *)p;
                                    
                                }
                                else if ( VSCP_CLASS1_MEASUREZONE == pvscpEvent->vscp_class ) {
                                    
                                    // Sensor index
                                    p[ 0 ] = pvscpEvent->pdata[ 0 ];
                                    
                                    // Zone
                                    p[ 1 ] = pvscpEvent->pdata[ 1 ];
                                    
                                    // Subzone
                                    p[ 2 ] = pvscpEvent->pdata[ 2 ];
                                            
                                    // Copy in the value string
                                    strcpy( p + 4, (const char *)strval.mbc_str() );
                                    
                                    delete [] pvscpEvent->pdata;
                                    
                                    pvscpEvent->pdata = (uint8_t *)p;
                                    
                                }                                
                                else if ( VSCP_CLASS1_SETVALUEZONE == pvscpEvent->vscp_class ) {
                                    
                                    // Sensor index
                                    p[ 0 ] = pvscpEvent->pdata[ 0 ];
                                    
                                    // Zone
                                    p[ 1 ] = pvscpEvent->pdata[ 1 ];
                                    
                                    // Subzone
                                    p[ 2 ] = pvscpEvent->pdata[ 2 ];
                                            
                                    // Copy in the value string
                                    strcpy( p + 4, (const char *)strval.mbc_str() );
                                    
                                    delete [] pvscpEvent->pdata;
                                    
                                    pvscpEvent->pdata = (uint8_t *)p;
                                    
                                }
                            }                            
                        }
                    }
                    
                    // Level I events to Level I over Level II events
                    if ( m_pMainThreadObj->m_pDeviceItem->m_translation & 
                            VSCP_DRIVER_OUT_TR_ALL512 ) {
                        pvscpEvent->vscp_class += 512;
                        uint8_t *p = new uint8_t[ 16 + pvscpEvent->sizeData ];
                        if ( NULL != p ) {
                            memset( p, 0, 16 + pvscpEvent->sizeData );
                            memcpy( p + 16, pvscpEvent->pdata, pvscpEvent->sizeData );
                            pvscpEvent->sizeData += 16;
                            delete [] pvscpEvent->pdata;
                            pvscpEvent->pdata = p;
                        }
                    }

                    m_pMainThreadObj->m_pCtrlObject->m_mutexClientOutputQueue.Lock();
                    m_pMainThreadObj->m_pCtrlObject->m_clientOutputQueue.Append( pvscpEvent );
                    m_pMainThreadObj->m_pCtrlObject->m_semClientOutputQueue.Post();
                    m_pMainThreadObj->m_pCtrlObject->m_mutexClientOutputQueue.Unlock();

                }
            }
        }
    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceLevel1ReceiveThread::OnExit()
{
    ;
}


// ****************************************************************************


///////////////////////////////////////////////////////////////////////////////
// deviceCanalWriteThread
//

deviceLevel1WriteThread::deviceLevel1WriteThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_pMainThreadObj = NULL;
    m_bQuit = false;
}

deviceLevel1WriteThread::~deviceLevel1WriteThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceLevel1WriteThread::Entry()
{

    Level1MsgOutList::compatibility_iterator nodeLevel1;

    // Must be a valid main object pointer
    if (NULL == m_pMainThreadObj) return NULL;

    // Blocking send method must have been found
    if (NULL == m_pMainThreadObj->m_pDeviceItem->m_proc_CanalBlockingSend) return NULL;

    while (!TestDestroy() && !m_bQuit) {

        // Wait until there is something to send
        if (wxSEMA_TIMEOUT ==
            m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_semClientInputQueue.WaitTimeout(500)) {
            continue;
        }

        CLIENTEVENTLIST::compatibility_iterator nodeClient;

        if (m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_clientInputQueue.GetCount()) {

            m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_mutexClientInputQueue.Lock();
            nodeClient = m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_clientInputQueue.GetFirst();
            vscpEvent *pqueueEvent = nodeClient->GetData();
            m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_mutexClientInputQueue.Unlock();

            canalMsg canalMsg;
            vscp_convertEventToCanal(&canalMsg, pqueueEvent);
            if (CANAL_ERROR_SUCCESS ==
                m_pMainThreadObj->m_pDeviceItem->m_proc_CanalBlockingSend( m_pMainThreadObj->m_pDeviceItem->m_openHandle,
                                                                            &canalMsg, 300)) {
                // Remove the node
                vscp_deleteVSCPevent(pqueueEvent);
                m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
            }
            else {
                // Give it another try
                m_pMainThreadObj->m_pCtrlObject->m_semClientOutputQueue.Post();
            }

        } // events in queue

    } // while

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceLevel1WriteThread::OnExit()
{

}

//-----------------------------------------------------------------------------
//                               L e v e l  I I
//-----------------------------------------------------------------------------


// ****************************************************************************


///////////////////////////////////////////////////////////////////////////////
// deviceLevel2ReceiveThread
//

deviceLevel2ReceiveThread::deviceLevel2ReceiveThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_pMainThreadObj = NULL;
    m_bQuit = false;
}

deviceLevel2ReceiveThread::~deviceLevel2ReceiveThread()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//  Read from device
//

void *deviceLevel2ReceiveThread::Entry()
{
    vscpEvent *pEvent;

    // Must be a valid main object pointer
    if (NULL == m_pMainThreadObj) return NULL;

    int rv;
    while (!TestDestroy() && !m_bQuit) {

        pEvent = new vscpEvent;
        if (NULL == pEvent) continue;
        rv = m_pMainThreadObj->m_pDeviceItem->m_proc_VSCPBlockingReceive(
                    m_pMainThreadObj->m_pDeviceItem->m_openHandle, pEvent, 500);

        if ((CANAL_ERROR_SUCCESS != rv) || (NULL == pEvent)) {
            delete pEvent;
            continue;
        }

        // Identify ourselves
        pEvent->obid = m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_clientID;

        // If timestamp is zero we set it here
        if ( 0 == pEvent->timestamp ) {
            pEvent->timestamp = vscp_makeTimeStamp();
        }

        // If no GUID is set,
        //      - Set driver GUID if define
        //      - Set interface GUID if no driver GUID defined.

        uint8_t ifguid[16];

        // Save nickname
        uint8_t nickname_msb = pEvent->GUID[14];
        uint8_t nickname_lsb = pEvent->GUID[15];

        // Set if to use
        memcpy( ifguid, pEvent->GUID, 16 );
        ifguid[14] = 0;
        ifguid[15] = 0;

        // If if is set to zero use interface id
        if ( vscp_isGUIDEmpty( ifguid ) ) {

            // Set driver GUID if set
            if ( !m_pMainThreadObj->m_pDeviceItem->m_interface_guid.isNULL() ) {
                m_pMainThreadObj->m_pDeviceItem->m_interface_guid.writeGUID( pEvent->GUID );
            }
            else {
                // If no driver GUID set use interface GUID
                m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_guid.writeGUID( pEvent->GUID );
            }

            // Preserve nickname
            pEvent->GUID[14] = nickname_msb;
            pEvent->GUID[15] = nickname_lsb;

        }

        // There must be room in the receive queue (even if rom (or whisky) has been better)
        if (m_pMainThreadObj->m_pCtrlObject->m_maxItemsInClientReceiveQueue >
                m_pMainThreadObj->m_pCtrlObject->m_clientOutputQueue.GetCount()) {

            m_pMainThreadObj->m_pCtrlObject->m_mutexClientOutputQueue.Lock();
            m_pMainThreadObj->m_pCtrlObject->m_clientOutputQueue.Append(pEvent);
            m_pMainThreadObj->m_pCtrlObject->m_semClientOutputQueue.Post();
            m_pMainThreadObj->m_pCtrlObject->m_mutexClientOutputQueue.Unlock();

        }
        else {
            if (NULL == pEvent) vscp_deleteVSCPevent(pEvent);
        }

    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceLevel2ReceiveThread::OnExit()
{
    ;
}


// ****************************************************************************


///////////////////////////////////////////////////////////////////////////////
// deviceLevel2WriteThread
//

deviceLevel2WriteThread::deviceLevel2WriteThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_pMainThreadObj = NULL;
    m_bQuit = false;
}

deviceLevel2WriteThread::~deviceLevel2WriteThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//  Write to device
//

void *deviceLevel2WriteThread::Entry()
{
    // Must be a valid main object pointer
    if (NULL == m_pMainThreadObj) return NULL;

    while (!TestDestroy() && !m_bQuit) {

        // Wait until there is something to send
        if (wxSEMA_TIMEOUT ==
            m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_semClientInputQueue.WaitTimeout(500)) {
            continue;
        }

        if (m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_clientInputQueue.GetCount()) {

            CLIENTEVENTLIST::compatibility_iterator nodeClient;

            m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_mutexClientInputQueue.Lock();
            nodeClient = m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_clientInputQueue.GetFirst();
            vscpEvent *pqueueEvent = nodeClient->GetData();

            m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_mutexClientInputQueue.Unlock();

            if (CANAL_ERROR_SUCCESS ==

                m_pMainThreadObj->m_pDeviceItem->m_proc_VSCPBlockingSend( m_pMainThreadObj->m_pDeviceItem->m_openHandle,
                                                                            pqueueEvent, 300)) {

                // Remove the node
                m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_mutexClientInputQueue.Lock();
                m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_clientInputQueue.DeleteNode( nodeClient );
                m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_mutexClientInputQueue.Unlock();
            }
            else {
                // Give it another try
                m_pMainThreadObj->m_pCtrlObject->m_semClientOutputQueue.Post();
            }

        } // events in queue

    } // while

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceLevel2WriteThread::OnExit()
{

}
