// devicethread.cpp
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

#include "daemonworker.h"
#include <canal_win32_ipc.h>
#include <canal_macro.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscpdlldef.h>
#include <vscphelper.h>
#include <dllist.h>
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
    m_preceiveLevel2Thread = NULL;
    m_pwriteLevel2Thread = NULL;
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
    if ( NULL == m_pDeviceItem->m_pClientItem ) {
        return NULL;
    }

    // This is now an active Client
    m_pDeviceItem->m_pClientItem->m_bOpen = true;
    if ( VSCP_DRIVER_LEVEL1 == m_pDeviceItem->m_driverLevel ) {
        m_pDeviceItem->m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1;
    }
    else if ( VSCP_DRIVER_LEVEL2 == m_pDeviceItem->m_driverLevel ) {
        m_pDeviceItem->m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL2;
    }
    else if ( VSCP_DRIVER_LEVEL3 == m_pDeviceItem->m_driverLevel ) {
        m_pDeviceItem->m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL3;
    }
    m_pDeviceItem->m_pClientItem->m_strDeviceName = m_pDeviceItem->m_strName;
    m_pDeviceItem->m_pClientItem->m_strDeviceName += _("|Started at ");
    m_pDeviceItem->m_pClientItem->m_strDeviceName += wxDateTime::Now().FormatISODate();
    m_pDeviceItem->m_pClientItem->m_strDeviceName += _(" ");
    m_pDeviceItem->m_pClientItem->m_strDeviceName += wxDateTime::Now().FormatISOTime();

    m_pCtrlObject->logMsg( m_pDeviceItem->m_pClientItem->m_strDeviceName + _("\n"),
                            DAEMON_LOGMSG_DEBUG);

    // Add the client to the Client List
    m_pCtrlObject->m_wxClientMutex.Lock();
    if ( !m_pCtrlObject->addClient( m_pDeviceItem->m_pClientItem, 
                                    m_pDeviceItem->m_interface_guid.getClientID() ) ) {
        // Failed to add client
        delete m_pDeviceItem->m_pClientItem;
        m_pDeviceItem->m_pClientItem = NULL;
        m_pCtrlObject->m_wxClientMutex.Unlock();
        m_pCtrlObject->logMsg( _("Devicethread: Failed to add client. Terminating thread.") );
        return NULL;
    }
    m_pCtrlObject->m_wxClientMutex.Unlock();

    // Client now have GUID set to server GUID + channel id
    // If device has a non NULL GUID replace the client GUID preserving
    // the channel id with that GUID
    if ( !m_pDeviceItem->m_pClientItem->m_guid.isNULL() ) {
        memcpy( m_pDeviceItem->m_pClientItem->m_guid.m_id,                  
                    m_pDeviceItem->m_interface_guid.getGUID(),
                    12 ); 
    }

    if ( VSCP_DRIVER_LEVEL3 != m_pDeviceItem->m_driverLevel ) {
        // Load dynamic library
        if ( !m_wxdll.Load( m_pDeviceItem->m_strPath, wxDL_LAZY ) ) {
            m_pCtrlObject->logMsg(_("Unable to load dynamic library.\n") );
            return NULL;
        }
    }
    else {     // Level II driver
        
        //  Startup Level III driver
        /*wxString executable = m_pDeviceItem->m_strPath;

        if ( 0 == ( m_pDeviceItem->m_pid = wxExecute( executable.mbc_str() ) ) ) {
            wxString str;
            str = _("Failed to load level III driver: ");
            str += m_pDeviceItem->m_strName;
            str += _("\n");
            m_pCtrlObject->logMsg(str);
            wxLogDebug(str);
            return NULL;
        }*/
        
    }

    if ( VSCP_DRIVER_LEVEL1 == m_pDeviceItem->m_driverLevel ) {

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
            (LPFNDLL_CANALOPEN) m_wxdll.GetSymbol(_("CanalOpen")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalOpen.\n") );
            return NULL;
        }

        // * * * * CANAL CLOSE * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalClose =
            (LPFNDLL_CANALCLOSE) m_wxdll.GetSymbol(_("CanalClose")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalClose.\n") );
            return NULL;
        }

        // * * * * CANAL GETLEVEL * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetLevel =
            (LPFNDLL_CANALGETLEVEL) m_wxdll.GetSymbol(_("CanalGetLevel")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetLevel.\n") );
            return NULL;
        }

        // * * * * CANAL SEND * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalSend =
            (LPFNDLL_CANALSEND) m_wxdll.GetSymbol(_("CanalSend")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalSend.\n") );
            return NULL;
        }

        // * * * * CANAL DATA AVAILABLE * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalDataAvailable =
            (LPFNDLL_CANALDATAAVAILABLE) m_wxdll.GetSymbol(_("CanalDataAvailable")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalDataAvailable.\n") );
            return NULL;
        }


        // * * * * CANAL RECEIVE * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalReceive =
            (LPFNDLL_CANALRECEIVE) m_wxdll.GetSymbol(_("CanalReceive")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalReceive.\n") );
            return NULL;
        }

        // * * * * CANAL GET STATUS * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetStatus =
            (LPFNDLL_CANALGETSTATUS) m_wxdll.GetSymbol(_("CanalGetStatus")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetStatus.\n") );
            return NULL;
        }

        // * * * * CANAL GET STATISTICS * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetStatistics =
            (LPFNDLL_CANALGETSTATISTICS) m_wxdll.GetSymbol(_("CanalGetStatistics")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetStatistics.\n") );
            return NULL;
        }

        // * * * * CANAL SET FILTER * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalSetFilter =
            (LPFNDLL_CANALSETFILTER) m_wxdll.GetSymbol(_("CanalSetFilter")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalSetFilter.\n") );
            return NULL;
        }

        // * * * * CANAL SET MASK * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalSetMask =
            (LPFNDLL_CANALSETMASK) m_wxdll.GetSymbol(_("CanalSetMask")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalSetMask.\n") );
            return NULL;
        }

        // * * * * CANAL GET VERSION * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetVersion =
            (LPFNDLL_CANALGETVERSION) m_wxdll.GetSymbol(_("CanalGetVersion")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetVersion.\n") );
            return NULL;
        }

        // * * * * CANAL GET DLL VERSION * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetDllVersion =
            (LPFNDLL_CANALGETDLLVERSION) m_wxdll.GetSymbol(_("CanalGetDllVersion")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetDllVersion.\n") );
            return NULL;
        }

        // * * * * CANAL GET VENDOR STRING * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetVendorString =
            (LPFNDLL_CANALGETVENDORSTRING) m_wxdll.GetSymbol(_("CanalGetVendorString")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetVendorString.\n") );
            return NULL;
        }


        // ******************************
        //     Generation 2 Methods
        // ******************************


        // * * * * CANAL BLOCKING SEND * * * *
        m_pDeviceItem->m_proc_CanalBlockingSend = NULL;
        if (m_wxdll.HasSymbol(_("CanalBlockingSend"))) {
            if (NULL == (m_pDeviceItem->m_proc_CanalBlockingSend =
                (LPFNDLL_CANALBLOCKINGSEND) m_wxdll.GetSymbol(_("CanalBlockingSend")))) {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalBlockingSend. Probably Generation 1 driver.\n") );
                m_pDeviceItem->m_proc_CanalBlockingSend = NULL;
            }
        }
        else {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": CanalBlockingSend not available. \n\tNon blocking operations set.\n") );
        }

        // * * * * CANAL BLOCKING RECEIVE * * * *
        m_pDeviceItem->m_proc_CanalBlockingReceive = NULL;
        if (m_wxdll.HasSymbol(_("CanalBlockingReceive"))) {
            if (NULL == (m_pDeviceItem->m_proc_CanalBlockingReceive =
                (LPFNDLL_CANALBLOCKINGRECEIVE) m_wxdll.GetSymbol(_("CanalBlockingReceive")))) {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalBlockingReceive. Probably Generation 1 driver.\n") );
                m_pDeviceItem->m_proc_CanalBlockingReceive = NULL;
            }
        }
        else {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": CanalBlockingReceive not available. \n\tNon blocking operations set.\n"));
        }

        // * * * * CANAL GET DRIVER INFO * * * *
        m_pDeviceItem->m_proc_CanalGetdriverInfo = NULL;
        if (m_wxdll.HasSymbol(_("CanalGetDriverInfo"))) {
            if (NULL == (m_pDeviceItem->m_proc_CanalGetdriverInfo =
                (LPFNDLL_CANALGETDRIVERINFO) m_wxdll.GetSymbol(_("CanalGetDriverInfo")))) {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName + _(": Unable to get dl entry for CanalGetDriverInfo. Probably Generation 1 driver.\n") );
                m_pDeviceItem->m_proc_CanalGetdriverInfo = NULL;
            }
        }

        // Open the device
        m_pDeviceItem->m_openHandle =
            m_pDeviceItem->m_proc_CanalOpen((const char *) m_pDeviceItem->m_strParameter.mb_str(wxConvUTF8),
                                                m_pDeviceItem->m_DeviceFlags);

        // Check if the driver opened properly
        if (m_pDeviceItem->m_openHandle <= 0) {
            wxString errMsg = _("Failed to open driver. Will not use it! \n\t[ ")
                + m_pDeviceItem->m_strName + _(" ]\n");
            m_pCtrlObject->logMsg( errMsg );
            return NULL;
        }
        else {
            wxString wxstr =
                wxString::Format(_("Driver %s opended.\n"),
                                    (const char *)m_pDeviceItem->m_strName.mbc_str() );
            m_pCtrlObject->logMsg( wxstr );
        }

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg(  m_pDeviceItem->m_strName +  _(": [Device tread] Level I Driver open.")); 
        }

        // Get Driver Level
        m_pDeviceItem->m_driverLevel =
                m_pDeviceItem->m_proc_CanalGetLevel(m_pDeviceItem->m_openHandle);


        //  * * * Level I Driver * * *

        // Check if blocking driver is available
        if (NULL != m_pDeviceItem->m_proc_CanalBlockingReceive) {

            // * * * * Blocking version * * * *

            if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level I blocking version.")); 
            }

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
                        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to run device write worker thread.\n") );
                    }
                }
                else {
                    m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to create device write worker thread.\n") );
                }
            }
            else {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to allocate memory for device write worker thread.\n") );
            }

            if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level I write thread started.")); 
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
                        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to run device receive worker thread.\n") );
                    }
                }
                else {
                    m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to create device receive worker thread.\n") );
                }
            }
            else {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to allocate memory for device receive worker thread.\n") );
            }

            if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level I receive thread started.")); 
            }

            // Just sit and wait until the end of the world as we know it...
            while (!m_pDeviceItem->m_bQuit) {
                wxSleep(1);
            }

            if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level I work loop ended.")); 
            }

            m_preceiveThread->m_bQuit = true;
            m_pwriteThread->m_bQuit = true;
            m_preceiveThread->Wait();
            m_pwriteThread->Wait();
        }
        else {

            // * * * * Non blocking version * * * *

            if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level I NON Blocking version.")); 
            }

            bool bActivity;
            while ( !TestDestroy() && !m_pDeviceItem->m_bQuit ) {

                bActivity = false;
                /////////////////////////////////////////////////////////////////////////////
                //                           Receive from device						   //
                /////////////////////////////////////////////////////////////////////////////
                canalMsg msg;
                if ( m_pDeviceItem->m_proc_CanalDataAvailable( m_pDeviceItem->m_openHandle ) ) {

                    if ( CANAL_ERROR_SUCCESS ==
                        m_pDeviceItem->m_proc_CanalReceive(m_pDeviceItem->m_openHandle, &msg ) ) {

                        bActivity = true;

                        // There must be room in the receive queue
                        if (m_pCtrlObject->m_maxItemsInClientReceiveQueue >
                            m_pCtrlObject->m_clientOutputQueue.GetCount()) {

                            vscpEvent *pvscpEvent = new vscpEvent;
                            if (NULL != pvscpEvent) {

                                // Set driver GUID if set
                                /*if ( m_pDeviceItem->m_interface_guid.isNULL() ) {
                                    m_pDeviceItem->m_interface_guid.writeGUID( pvscpEvent->GUID );
                                }
                                else {
                                    // If no driver GUID set use interface GUID
                                    m_pDeviceItem->m_pClientItem->m_guid.writeGUID( pvscpEvent->GUID );
                                }*/

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
                //          Send messages (if any) in the output queue
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                // Check if there is something to send
                if ( m_pDeviceItem->m_pClientItem->m_clientInputQueue.GetCount() ) {

                    bActivity = true;

                    CLIENTEVENTLIST::compatibility_iterator nodeClient;

                    m_pDeviceItem->m_pClientItem->m_mutexClientInputQueue.Lock();
                    nodeClient = m_pDeviceItem->m_pClientItem->m_clientInputQueue.GetFirst();
                    vscpEvent *pqueueEvent = nodeClient->GetData();
                    m_pDeviceItem->m_pClientItem->m_mutexClientInputQueue.Unlock();
                    
                    // Trow away event if Level II and Level I interface
                    if ( ( CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1 == m_pDeviceItem->m_pClientItem->m_type ) &&
                            ( pqueueEvent->vscp_class > 512 ) ) {
                        // Remove the event and the node
                        delete pqueueEvent;
                        m_pDeviceItem->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
                        continue;
                    }

                    canalMsg canalMsg;
                    vscp_convertEventToCanal(&canalMsg, pqueueEvent);
                    if (CANAL_ERROR_SUCCESS ==
                        m_pDeviceItem->m_proc_CanalSend(m_pDeviceItem->m_openHandle, &canalMsg)) {
                        // Remove the event and the node
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

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level I Work loop ended.")); 
        }

        // Close CANAL channel
        m_pDeviceItem->m_proc_CanalClose(m_pDeviceItem->m_openHandle);

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level I Closed.")); 
        }

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
            (LPFNDLL_VSCPOPEN) m_wxdll.GetSymbol(_("VSCPOpen")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPOpen.\n"));
            return NULL;
        }

        // * * * * VSCP CLOSE * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPClose =
            (LPFNDLL_VSCPCLOSE) m_wxdll.GetSymbol(_("VSCPClose")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPClose.\n"));
            return NULL;
        }

        // * * * * VSCP BLOCKINGSEND * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPBlockingSend =
            (LPFNDLL_VSCPBLOCKINGSEND) m_wxdll.GetSymbol(_("VSCPBlockingSend")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPBlockingSend.\n"));
            return NULL;
        }

        // * * * * VSCP BLOCKINGRECEIVE * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPBlockingReceive =
            (LPFNDLL_VSCPBLOCKINGRECEIVE) m_wxdll.GetSymbol(_("VSCPBlockingReceive")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPBlockingReceive.\n"));
            return NULL;
        }

        // * * * * VSCP GETLEVEL * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPGetLevel =
            (LPFNDLL_VSCPGETLEVEL) m_wxdll.GetSymbol(_("VSCPGetLevel")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPGetLevel.\n"));
            return NULL;
        }

        // * * * * VSCP GET DLL VERSION * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPGetDllVersion =
            (LPFNDLL_VSCPGETDLLVERSION) m_wxdll.GetSymbol(_("VSCPGetDllVersion")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPGetDllVersion.\n"));
            return NULL;
        }

        // * * * * VSCP GET VENDOR STRING * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPGetVendorString =
            (LPFNDLL_VSCPGETVENDORSTRING) m_wxdll.GetSymbol(_("VSCPGetVendorString")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPGetVendorString.\n"));
            return NULL;
        }

        // * * * * VSCP GET DRIVER INFO * * * *
        if (NULL == (m_pDeviceItem->m_proc_CanalGetdriverInfo =
            (LPFNDLL_VSCPGETVENDORSTRING) m_wxdll.GetSymbol(_("VSCPGetDriverInfo")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPGetDriverInfo.\n"));
            return NULL;
        }

        // * * * * VSCP GET WEB PAGE TEMPLATE * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPGetWebPageTemplate =
            (LPFNDLL_VSCPGETWEBPAGETEMPLATE) m_wxdll.GetSymbol(_("VSCPGetWebPageTemplate")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPGetWebPageTemplate.\n"));
            return NULL;
        }

        // * * * * VSCP GET WEB PAGE INFO * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPGetWebPageInfo =
            (LPFNDLL_VSCPGETWEBPAGEINFO) m_wxdll.GetSymbol(_("VSCPGetWebPageInfo")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPGetWebPageInfo.\n"));
            return NULL;
        }

        // * * * * VSCP WEB PAGE UPDATE * * * *
        if (NULL == (m_pDeviceItem->m_proc_VSCPWebPageupdate =
            (LPFNDLL_VSCPWEBPAGEUPDATE) m_wxdll.GetSymbol(_("VSCPWebPageupdate")))) {
            // Free the library
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for VSCPWebPageupdate.\n"));
            return NULL;
        }

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) { 
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Discovered all methods\n"));
        }

        // Username, password, host and port can be set in configuration file. Read in them here
        // if they are.
        wxString strHost(_("127.0.0.1"));
        short port = 9598;

        wxStringTokenizer tkz(m_pDeviceItem->m_strParameter, _(";"));
        if ( tkz.HasMoreTokens() ) {

            CVSCPVariable variable;

            // Get prefix
            wxString prefix = tkz.GetNextToken();

            // Check if username is specified in the configuration file            
            if ( m_pCtrlObject->m_variables.find( m_pDeviceItem->m_strName +
                                                    _("_username"), variable ) ) {
                wxString str;
                if (VSCP_DAEMON_VARIABLE_CODE_STRING == variable.getType()) {
                    str = variable.getValue();
                    m_pCtrlObject->m_driverUsername = str;
                }
            }

            // Check if password is specified in the configuration file            
            if ( m_pCtrlObject->m_variables.find( m_pDeviceItem->m_strName +
                                                    _("_password"), variable ) ) {
                wxString str;
                if (VSCP_DAEMON_VARIABLE_CODE_STRING == variable.getType()) {
                    str = variable.getValue();
                    m_pCtrlObject->m_driverPassword = str;
                }
            }

            // Check if host is specified in the configuration file       
            if ( m_pCtrlObject->m_variables.find( m_pDeviceItem->m_strName + 
                                                    _("_host"), variable ) ) {
                wxString str;
                if (VSCP_DAEMON_VARIABLE_CODE_STRING == variable.getType()) {
                    str = variable.getValue();
                    strHost = str;
                }
            }

            // Check if host is specified in the configuration file
            if ( m_pCtrlObject->m_variables.find( m_pDeviceItem->m_strName + 
                                                    _("_port"), variable ) ) {
                wxString str;
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER == variable.getType()) {
                    str = variable.getValue();
                    port = vscp_readStringValue( str );
                }
            }

        }

        // Open up the driver
        m_pDeviceItem->m_openHandle =
            m_pDeviceItem->m_proc_VSCPOpen( m_pCtrlObject->m_driverUsername.mbc_str(),
                    ( const char * )m_pCtrlObject->m_driverPassword.mbc_str(),
                    ( const char * )strHost.mbc_str(),
                    port,
                    ( const char * )m_pDeviceItem->m_strName.mbc_str(),
                    ( const char * )m_pDeviceItem->m_strParameter.mbc_str() );

        if ( 0 == m_pDeviceItem->m_openHandle ) {
            // Free the library
            m_pCtrlObject->logMsg(  m_pDeviceItem->m_strName +  _(": [Device tread] Unable to open VSCP "
                                     "driver (check username/password/path/"
                                     "rights). Possible additional info from driver "
                                     "in syslog.\n" ) );
            return NULL;
        }
        else {
            wxString wxstr =
                wxString::Format(_("Driver %s opended.\n"),
                                    (const char *)m_pDeviceItem->m_strName.mbc_str() );
            m_pCtrlObject->logMsg( wxstr );
        }

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level II Open.")); 
        }

        /////////////////////////////////////////////////////////////////////////////
        // Device write worker thread
        /////////////////////////////////////////////////////////////////////////////

        m_pwriteLevel2Thread = new deviceLevel2WriteThread;

        if ( m_pwriteLevel2Thread ) {
            m_pwriteLevel2Thread->m_pMainThreadObj = this;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pwriteLevel2Thread->Create())) {
                m_pwriteLevel2Thread->SetPriority(WXTHREAD_MAX_PRIORITY);
                if (wxTHREAD_NO_ERROR != (err = m_pwriteLevel2Thread->Run())) {
                    m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to run device write worker thread."));
                }
            }
            else {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to create device write worker thread."));
            }
        }
        else {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to allocate memory for device write worker thread."));
        }

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level II Write thread created.")); 
        }

        /////////////////////////////////////////////////////////////////////////////
        // Device read worker thread
        /////////////////////////////////////////////////////////////////////////////

        m_preceiveLevel2Thread = new deviceLevel2ReceiveThread;

        if ( NULL != m_preceiveLevel2Thread ) {
            m_preceiveLevel2Thread->m_pMainThreadObj = this;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_preceiveLevel2Thread->Create())) {
                m_preceiveLevel2Thread->SetPriority(WXTHREAD_MAX_PRIORITY);
                if (wxTHREAD_NO_ERROR != (err = m_preceiveLevel2Thread->Run())) {
                    m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to run device "
                                            "receive worker thread."));
                }
            }
            else {
                m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to create device receive "
                                        "worker thread.") );
            }
        }
        else {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to allocate memory for device "
                                    "receive worker thread.") );
        }

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level II Read thread created.")); 
        }

        // Just sit and wait until the end of the world as we know it...
        while (!TestDestroy() && !m_pDeviceItem->m_bQuit) {
            wxSleep(1);
        }

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level II Cloing.")); 
        }

        //m_preceiveLevel2Thread->m_bQuit = true;
        //m_pwriteLevel2Thread->m_bQuit = true;

        // Close channel
        m_pDeviceItem->m_proc_VSCPClose( m_pDeviceItem->m_openHandle );

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level II Closed.")); 
        }

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

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level II Done waiting for threads.")); 
        }

    }
    else if (VSCP_DRIVER_LEVEL3 == m_pDeviceItem->m_driverLevel) {
        
        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level III Start server loop.")); 
        }

        // Just sit and wait until the end of the world as we know it...
        while ( !m_pDeviceItem->m_bQuit ) {
            wxSleep(1);
        }

        if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_DRIVER ) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": [Device tread] Level II End server loop.")); 
        }
        
        // Send stop to device
        //kill( m_pDeviceItem->m_pid, SIGKILL );
        //*** This happens in the item destructor now ***
        
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

                    // Set driver GUID if set
                    /*if ( m_pMainThreadObj->m_pDeviceItem->m_interface_guid.isNULL() ) {
                        m_pMainThreadObj->m_pDeviceItem->m_interface_guid.writeGUID( pvscpEvent->GUID );
                    }
                    else {
                        // If no driver GUID set use interface GUID
                        m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_guid.writeGUID( pvscpEvent->GUID );
                    }*/

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
                        vscp_convertLevel1MeasuremenToLevel2Double( pvscpEvent );                        
                    }                   
                    // Level I measurement events to Level II measurement string
                    else if ( m_pMainThreadObj->m_pDeviceItem->m_translation & 
                            VSCP_DRIVER_OUT_TR_M1M2S ) {                        
                        vscp_convertLevel1MeasuremenToLevel2String( pvscpEvent );
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
            
            // Trow away event if Level II and Level I interface
            if ( ( CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1 == m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_type ) &&
                            ( pqueueEvent->vscp_class > 512 ) ) {
                // Remove the event and the node
                delete pqueueEvent;
                m_pMainThreadObj->m_pDeviceItem->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
                continue;
            }

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
    while ( !TestDestroy() && !m_bQuit ) {

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

        // There must be room in the receive queue (even if room (or whisky) has been better)
        if ( m_pMainThreadObj->m_pCtrlObject->m_maxItemsInClientReceiveQueue >
                m_pMainThreadObj->m_pCtrlObject->m_clientOutputQueue.GetCount() ) {

            m_pMainThreadObj->m_pCtrlObject->m_mutexClientOutputQueue.Lock();
            m_pMainThreadObj->m_pCtrlObject->m_clientOutputQueue.Append( pEvent );
            m_pMainThreadObj->m_pCtrlObject->m_semClientOutputQueue.Post();
            m_pMainThreadObj->m_pCtrlObject->m_mutexClientOutputQueue.Unlock();

        }
        else {
            if (NULL == pEvent) vscp_deleteVSCPevent( pEvent );
            delete pEvent;
            pEvent = NULL;
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
    if ( NULL == m_pMainThreadObj ) return NULL;

    while ( !TestDestroy() && !m_bQuit ) {

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
