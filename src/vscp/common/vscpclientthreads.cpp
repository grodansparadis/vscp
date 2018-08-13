/////////////////////////////////////////////////////////////////////////////
// Name:        vscpclientthreads.cpp
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

// HISTORY:
//		071218 - AKHE	Started this file
//

#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wx/defs.h"
#include "wx/app.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>
#include <wx/listimpl.cpp>
#include <wx/dynlib.h>

#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <sys/times.h>
#endif
#include "vscp.h"
#include "vscpclientthreads.h"


WX_DEFINE_LIST( eventOutQueue );
WX_DEFINE_LIST( TXLIST );

DEFINE_EVENT_TYPE(wxVSCP_IN_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_OUT_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_CTRL_LOST_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_RCV_LOST_EVENT)



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
//

ctrlObj::ctrlObj()
{
  m_bQuit = false;      // Don't even think of quitting yet...
  m_errorControl = 0;   // No error
  m_errorReceive = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
//

ctrlObj::~ctrlObj()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// startWorkerThreads
//

void ctrlObj::startWorkerThreads( wxWindow *pWnd, unsigned long id )
{
  wxBusyCursor wait;

    if ( INTERFACE_VSCP ==  m_interfaceType ) {

    /////////////////////////////////////////////////////////////////////////////
        // Load controlobject control handler
        /////////////////////////////////////////////////////////////////////////////
    m_pTXWorkerThread = new TXWorkerThread;

        if ( NULL != m_pTXWorkerThread )
        {
        m_CtrlObject.m_pVSCPSessionWnd = (frmVSCPSession *)pFrm;
        m_pTXWorkerThread->m_pCtrlObject = &m_CtrlObject;
            wxThreadError err;
            if ( wxTHREAD_NO_ERROR == ( err = m_pTXWorkerThread->Create() ) ) {
            m_pTXWorkerThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                if ( wxTHREAD_NO_ERROR != ( err = m_pTXWorkerThread->Run() ) ) {
                ::wxGetApp().logMsg ( _("Unable to run controlobject device thread."), DAEMON_LOGMSG_CRITICAL );
                }
            }
            else {
                ::wxGetApp().logMsg ( _("Unable to create controlobject device thread."), DAEMON_LOGMSG_CRITICAL );
            }
        }
        else
        {
            ::wxGetApp().logMsg ( _("Unable to allocate memory for controlobject device thread."), DAEMON_LOGMSG_CRITICAL );
        }


    /////////////////////////////////////////////////////////////////////////////
        // Load controlobject client message handler
        /////////////////////////////////////////////////////////////////////////////
    m_pRXWorkerThread = new RXWorkerThread;

        if ( NULL != m_pRXWorkerThread )
        {
        m_CtrlObject.m_pVSCPSessionWnd = (frmVSCPSession *)pFrm;
        m_pRXWorkerThread->m_pCtrlObject = &m_CtrlObject;
            wxThreadError err;
            if ( wxTHREAD_NO_ERROR == ( err = m_pRXWorkerThread->Create() ) ) {
            m_pRXWorkerThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                if ( wxTHREAD_NO_ERROR != ( err = m_pRXWorkerThread->Run() ) ) {
            ::wxGetApp().logMsg ( _("Unable to run controlobject client thread."), DAEMON_LOGMSG_CRITICAL );
                }
            }
            else {
                ::wxGetApp().logMsg ( _("Unable to create controlobject client thread."), DAEMON_LOGMSG_CRITICAL );
            }
        }
        else
        {
            ::wxGetApp().logMsg ( _("Unable to allocate memory for controlobject client thread."), DAEMON_LOGMSG_CRITICAL );
        }
    }
    else {
        /////////////////////////////////////////////////////////////////////////////
        // Load device handler
        /////////////////////////////////////////////////////////////////////////////
    m_pDeviceWorkerThread = new deviceThread;

        if ( NULL != m_pDeviceWorkerThread )
        {
        m_CtrlObject.m_pVSCPSessionWnd  = (frmVSCPSession *)pFrm;
        m_pDeviceWorkerThread->m_pCtrlObject = &m_CtrlObject;
            wxThreadError err;
            if ( wxTHREAD_NO_ERROR == ( err = m_pDeviceWorkerThread->Create() ) ) {
            m_pDeviceWorkerThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                if ( wxTHREAD_NO_ERROR != ( err = m_pDeviceWorkerThread->Run() ) ) {
                ::wxGetApp().logMsg ( _("Unable to run controlobject device thread."), DAEMON_LOGMSG_CRITICAL );
                }
            }
            else {
                ::wxGetApp().logMsg ( _("Unable to create controlobject device thread."), DAEMON_LOGMSG_CRITICAL );
            }
        }
        else
        {
            ::wxGetApp().logMsg ( _("Unable to allocate memory for controlobject device thread."), DAEMON_LOGMSG_CRITICAL );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stopWorkerThreads
//

void ctrlObj::stopWorkerThreads( void )
{
  wxBusyCursor wait;

  m_BtnActivateInterface->SetValue( false );
    m_BtnActivateInterface->SetLabel(_("Disconnected"));
    m_BtnActivateInterface->Update();

  m_CtrlObject.m_bQuit = true;

    if ( INTERFACE_VSCP ==  m_CtrlObject.m_interfaceType ) {
  
        if ( NULL != m_pTXWorkerThread ) {
        m_pTXWorkerThread->Wait();
      delete m_pTXWorkerThread;
      m_pTXWorkerThread = NULL;
        }

   if ( NULL != m_pRXWorkerThread ) {
     m_pRXWorkerThread->Wait();
     delete m_pRXWorkerThread;
      m_pRXWorkerThread = NULL;
   }

    }
    else {
        if ( NULL != m_pDeviceWorkerThread ) {
        m_pDeviceWorkerThread->Wait();
      delete m_pDeviceWorkerThread;
      m_pDeviceWorkerThread = NULL;
        }
    }
  
}


///////////////////////////////////////////////////////////////////////////////
// TXWorkerThread
//

TXWorkerThread::TXWorkerThread()
                : wxThread( wxTHREAD_JOINABLE )
{
    m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// transmitWorkerThread
//

TXWorkerThread::~TXWorkerThread()
{

}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to read from devices. Read it/them and send
// it/them to all other devices clients.

void *TXWorkerThread::Entry()
{
  eventOutQueue::compatibility_iterator node;
  vscpEvent *pEvent;

  // Must be a valid control object pointer
    if ( NULL == m_pCtrlObject ) return NULL;

    wxCommandEvent eventConnectionLost( wxVSCP_CTRL_LOST_EVENT, m_pCtrlObject->m_windowID );
  
  /// TCP/IP Control
  VscpTcpIf tcpifControl;
  


  
  // Connect to the server with the control interface
  if ( !tcpifControl.doCmdOpen( m_pCtrlObject->m_ifVSCP.m_strHost,
                                m_pCtrlObject->m_ifVSCP.m_port,
                                m_pCtrlObject->m_ifVSCP.m_strUser,
                                m_pCtrlObject->m_ifVSCP.m_strPassword ) ) {
    //::wxGetApp().logMsg ( _("VSCP TX thread - Unable to connect to server."), DAEMON_LOGMSG_CRITICAL );
    m_pCtrlObject->m_errorControl = VSCP_SESSION_ERROR_UNABLE_TO_CONNECT;
        wxPostEvent( m_pCtrlObject->m_pWnd, eventConnectionLost );
    return NULL;
  }
  
  // Get channel ID
  tcpifControl.doCmdGetChannelID( &m_pCtrlObject->m_txChannelID );
  
    while ( !TestDestroy() && !m_pCtrlObject->m_bQuit )
    {
    if ( wxSEMA_TIMEOUT == m_pCtrlObject->m_semOutQue.WaitTimeout( 500 ) ) continue;
    m_pCtrlObject->m_mutexOutQueue.Lock();
    node = m_pCtrlObject->m_outQueue.GetFirst();
    pEvent = node->GetData();
    tcpifControl.doCmdSend( pEvent );
    m_pCtrlObject->m_outQueue.DeleteNode( node );
    deleteVSCPevent( pEvent );
    m_pCtrlObject->m_mutexOutQueue.Unlock();
    } // while
  
  // Close the interface
  tcpifControl.doCmdClose();

    wxPostEvent( m_pCtrlObject->m_pWnd, eventConnectionLost );

    return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void TXWorkerThread::OnExit()
{

}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// receiveWorkerThread
//

RXWorkerThread::RXWorkerThread()
                  : wxThread( wxTHREAD_JOINABLE )
{
    m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// receiveWorkerThread
//

RXWorkerThread::~RXWorkerThread()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

void *RXWorkerThread::Entry()
{
  int rv;
  VscpTcpIf tcpifReceive;
  wxCommandEvent eventReceive( wxVSCP_IN_EVENT, m_pCtrlObject->m_windowID );
    wxCommandEvent eventConnectionLost( wxVSCP_RCV_LOST_EVENT, m_pCtrlObject->m_windowID );
  
  // Must be a valid control object pointer
    if ( NULL == m_pCtrlObject ) return NULL;
  
  // Connect to the server with the control interface
  if ( !tcpifReceive.doCmdOpen( m_pCtrlObject->m_ifVSCP.m_strHost,
                            m_pCtrlObject->m_ifVSCP.m_port,
                            m_pCtrlObject->m_ifVSCP.m_strUser,
                            m_pCtrlObject->m_ifVSCP.m_strPassword ) ) {
    //::wxGetApp().logMsg ( _("VSCP Receive thread - Unable to connect to server."), DAEMON_LOGMSG_CRITICAL );
    m_pCtrlObject->m_errorReceive = VSCP_SESSION_ERROR_UNABLE_TO_CONNECT;
        wxPostEvent( m_pCtrlObject->m_pWnd, eventConnectionLost );
    return NULL;
  }
  
  // Find the channel id
  tcpifReceive.doCmdGetChannelID( &m_pCtrlObject->m_rxChannelID );

  // Start Receive Loop
    tcpifReceive.doCmdEnterReceiveLoop();

  vscpEvent event;
    while ( !TestDestroy() && !m_pCtrlObject->m_bQuit )
    {

    if ( CANAL_ERROR_SUCCESS == 
            ( rv = tcpifReceive.doCmdBlockReceive( &event, 1000 ) ) ) {

      if ( NULL != m_pCtrlObject->m_pWnd ) {

        vscpEvent *pEvent = new vscpEvent;
        if ( NULL != pEvent ) {

          copyVSCPEvent( pEvent, &event );

          eventReceive.SetClientData( pEvent );
          wxPostEvent( m_pCtrlObject->m_pWnd, eventReceive );
        
        }

      } // Session window exist

    }
    else {
      if ( CANAL_ERROR_COMMUNICATION == rv ) m_pCtrlObject->m_bQuit = true;
    }

    } // while

    // Close the interface
    tcpifReceive.doCmdClose();

    wxPostEvent( m_pCtrlObject->m_pWnd, eventConnectionLost );

    return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void RXWorkerThread::OnExit()
{

}





///////////////////////////////////////////////////////////////////////////////
//                               D E V I C E
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// deviceThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//


deviceThread::deviceThread()
        : wxThread( wxTHREAD_JOINABLE )
{
    m_pCtrlObject = NULL;
    m_preceiveThread = NULL;
    m_pwriteThread = NULL;
}


deviceThread::~deviceThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceThread::Entry()
{
  wxCommandEvent eventReceive( wxVSCP_IN_EVENT, m_pCtrlObject->m_windowID );
    wxCommandEvent eventConnectionLost( wxVSCP_RCV_LOST_EVENT, m_pCtrlObject->m_windowID );

    // Must have a valid pointer to the control object
    if ( NULL == m_pCtrlObject ) return NULL;

    // Load dynamic library
    if ( ! m_wxdll.Load ( m_pCtrlObject->m_ifCANAL.m_strPath, wxDL_LAZY ) )
    {
        //::wxGetApp().logMsg ( _T ( "vscpd: Unable to load dynamic library." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // Now find methods in library

    // * * * * CANAL OPEN * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalOpen =
                       ( LPFNDLL_CANALOPEN ) m_wxdll.GetSymbol ( _T ( "CanalOpen" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalOpen." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL CLOSE * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalClose =
                       ( LPFNDLL_CANALCLOSE ) m_wxdll.GetSymbol ( _T ( "CanalClose" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalClose." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL GETLEVEL * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalGetLevel =
                       ( LPFNDLL_CANALGETLEVEL ) m_wxdll.GetSymbol ( _T ( "CanalGetLevel" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalGetLevel." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL SEND * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalSend =
                       ( LPFNDLL_CANALSEND ) m_wxdll.GetSymbol ( _T ( "CanalSend" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalSend." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL DATA AVAILABLE * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalDataAvailable =
                       ( LPFNDLL_CANALDATAAVAILABLE ) m_wxdll.GetSymbol ( _T ( "CanalDataAvailable" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalDataAvailable." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }


    // * * * * CANAL RECEIVE * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalReceive =
                       ( LPFNDLL_CANALRECEIVE ) m_wxdll.GetSymbol ( _T ( "CanalReceive" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalReceive." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL GET STATUS * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalGetStatus =
                       ( LPFNDLL_CANALGETSTATUS ) m_wxdll.GetSymbol ( _T ( "CanalGetStatus" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalGetStatus." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL GET STATISTICS * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalGetStatistics =
                       ( LPFNDLL_CANALGETSTATISTICS ) m_wxdll.GetSymbol ( _T ( "CanalGetStatistics" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalGetStatistics." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL SET FILTER * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalSetFilter =
                       ( LPFNDLL_CANALSETFILTER ) m_wxdll.GetSymbol ( _T ( "CanalSetFilter" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalSetFilter." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL SET MASK * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalSetMask =
                       ( LPFNDLL_CANALSETMASK ) m_wxdll.GetSymbol ( _T ( "CanalSetMask" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalSetMask." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL GET VERSION * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalGetVersion =
                       ( LPFNDLL_CANALGETVERSION ) m_wxdll.GetSymbol ( _T ( "CanalGetVersion" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalGetVersion." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL GET DLL VERSION * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalGetDllVersion =
                       ( LPFNDLL_CANALGETDLLVERSION ) m_wxdll.GetSymbol ( _T ( "CanalGetDllVersion" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalGetDllVersion." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // * * * * CANAL GET VENDOR STRING * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalGetVendorString =
                       ( LPFNDLL_CANALGETVENDORSTRING ) m_wxdll.GetSymbol ( _T ( "CanalGetVendorString" ) ) ) )
    {
        // Free the library
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalGetVendorString." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }


    wxLogNull logNo;


    // ******************************
    //     Generation 2 Methods
    // ******************************


    // * * * * CANAL BLOCKING SEND * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalBlockingSend =
                       ( LPFNDLL_CANALBLOCKINGSEND ) m_wxdll.GetSymbol ( _T ( "CanalBlockingSend" ) ) ) )
    {
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalBlockingSend. Probably Generation 1 driver." ),
        //					              DAEMON_LOGMSG_CRITICAL );
    m_pCtrlObject->m_proc_CanalBlockingSend = NULL;
    }

    // * * * * CANAL BLOCKING RECEIVE * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalBlockingReceive =
                       ( LPFNDLL_CANALBLOCKINGRECEIVE ) m_wxdll.GetSymbol ( _T ( "CanalBlockingReceive" ) ) ) )
    {
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalBlockingReceive. Probably Generation 1 driver." ),
        //					                DAEMON_LOGMSG_CRITICAL );
    m_pCtrlObject->m_proc_CanalBlockingReceive = NULL;
    }

    // * * * * CANAL GET DRIVER INFO * * * *
    if ( NULL == ( m_pCtrlObject->m_proc_CanalGetdriverInfo =
                       ( LPFNDLL_CANALGETDRIVERINFO ) m_wxdll.GetSymbol ( _T ( "CanalGetDriverInfo" ) ) ) )
    {
        //::wxGetApp().logMsg ( _T ( "Unable to get dl entry for CanalGetDriverInfo. Probably Generation 1 driver." ),
        //					                DAEMON_LOGMSG_CRITICAL );
    m_pCtrlObject->m_proc_CanalGetdriverInfo = NULL;
    }

    //
    // =====================================================================================
    //

    // Open the device
    m_pCtrlObject->m_openHandle =
        m_pCtrlObject->m_proc_CanalOpen ( ( const char * ) m_pCtrlObject->m_ifCANAL.m_strConfig.mb_str ( wxConvUTF8 ),
                                          m_pCtrlObject->m_ifCANAL.m_flags );



    // Check if the driver opened properly
    if ( 0 == m_pCtrlObject->m_openHandle )
    {
        //::wxGetApp().logMsg ( _T ( "Driver failed to open." ), DAEMON_LOGMSG_CRITICAL );
        return NULL;
    }

    // Get Driver Level
    m_pCtrlObject->m_driverLevel = m_pCtrlObject->m_proc_CanalGetLevel ( m_pCtrlObject->m_openHandle );


    // If this is a TCPIP level driver then nothing more then open and close should be done without
    // the driver. No messages should be put on any queues
    if ( CANAL_LEVEL_USES_TCPIP == m_pCtrlObject->m_driverLevel )
    {
        // Just sit and wait until the end
        while ( !TestDestroy() && !m_pCtrlObject->m_bQuit )
        {
            wxSleep ( 1 );
        }
    }
    else
    {
    if ( NULL != m_pCtrlObject->m_proc_CanalBlockingReceive )
        {

            // * * * * Blocking version * * * *

            /////////////////////////////////////////////////////////////////////////////
            // Device write worker thread
            /////////////////////////////////////////////////////////////////////////////
            m_pwriteThread = new deviceWriteThread;

            if ( m_pwriteThread )
            {
                m_pwriteThread->m_pMainThreadObj = this;
                wxThreadError err;
                if ( wxTHREAD_NO_ERROR == ( err = m_pwriteThread->Create() ) ) {
                    m_pwriteThread->SetPriority( WXTHREAD_MAX_PRIORITY );
                    if ( wxTHREAD_NO_ERROR != ( err = m_pwriteThread->Run() ) ) {
                        //::wxGetApp().logMsg ( _("Unable to run device write worker thread."), DAEMON_LOGMSG_CRITICAL );	
                    }
                }
                else {
                    //::wxGetApp().logMsg ( _("Unable to create device write worker thread."), DAEMON_LOGMSG_CRITICAL );	
                }
            }
            else
            {
                //::wxGetApp().logMsg ( _("Unable to allocate memory for device write worker thread."), DAEMON_LOGMSG_CRITICAL );
            }

            /////////////////////////////////////////////////////////////////////////////
            // Device read worker thread
            /////////////////////////////////////////////////////////////////////////////
            m_preceiveThread = new deviceReceiveThread;

            if ( m_preceiveThread )
            {
                m_preceiveThread->m_pMainThreadObj = this;
                wxThreadError err;
                if ( wxTHREAD_NO_ERROR == ( err = m_preceiveThread->Create() ) ) {
                    m_preceiveThread->SetPriority( WXTHREAD_MAX_PRIORITY );
                    if ( wxTHREAD_NO_ERROR != ( err = m_preceiveThread->Run() ) ) {
                        //::wxGetApp().logMsg ( _("Unable to run device receive worker thread."), DAEMON_LOGMSG_CRITICAL );	
                    }
                }
                else {
                    //::wxGetApp().logMsg ( _("Unable to create device receive worker thread."), DAEMON_LOGMSG_CRITICAL );	
                }
            }
            else
            {
                //::wxGetApp().logMsg ( _("Unable to allocate memory for device receive worker thread."), DAEMON_LOGMSG_CRITICAL );
            }

            // Just sit and wait until the end
            while ( !m_pCtrlObject->m_bQuit )
            {
                wxSleep ( 1 );
            }

      m_preceiveThread->m_bQuit = true;
      m_pwriteThread->m_bQuit = true;
      m_preceiveThread->Wait();
      m_pwriteThread->Wait();
        }
        else
        {

            // * * * * Non blocking version * * * *

      bool bActivity;
            while ( !TestDestroy() && !m_pCtrlObject->m_bQuit ) {

        bActivity = false;
                /////////////////////////////////////////////////////////////////////////////
                //                           Receive from device								   				 //
                /////////////////////////////////////////////////////////////////////////////
                canalMsg msg;
                if ( m_pCtrlObject->m_proc_CanalDataAvailable ( m_pCtrlObject->m_openHandle ) ) {

                        m_pCtrlObject->m_proc_CanalReceive ( m_pCtrlObject->m_openHandle, &msg );

            bActivity = true;
                            
                        vscpEvent *pEvent = new vscpEvent;
            if ( NULL != pEvent ) {

                          // Convert CANAL message to VSCP event
                          convertCanalToEvent ( pEvent,
                                                                        &msg,
                                                                    m_pCtrlObject->m_GUID );

              eventReceive.SetClientData( pEvent );
              wxPostEvent( m_pCtrlObject->m_pWnd, eventReceive );

            }

                }


                  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                  //             Send messages (if any) in the outqueue
                  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
          if ( m_pCtrlObject->m_outQueue.GetCount() ) {

            m_pCtrlObject->m_mutexOutQueue.Lock();

            eventOutQueue::compatibility_iterator node = 
                    m_pCtrlObject->m_outQueue.GetFirst();
            vscpEvent *pEvent = node->GetData();
    
            canalMsg canalMsg;
                convertEventToCanal ( &canalMsg, pEvent );
                if ( CANAL_ERROR_SUCCESS == 
                            m_pCtrlObject->m_proc_CanalBlockingSend ( m_pCtrlObject->m_openHandle, &canalMsg, 300 ) ) {

              eventReceive.SetClientData( pEvent );
              wxPostEvent( m_pCtrlObject->m_pWnd, eventReceive ); 

                  // Remove the node
                    m_pCtrlObject->m_outQueue.DeleteNode ( node );  
                }
                else {
                    // Give it another try
                    m_pCtrlObject->m_semOutQue.Post();
                }

            m_pCtrlObject->m_mutexOutQueue.Unlock();

          }

          if ( !bActivity ) {
            ::wxMilliSleep( 100 );
          }

          bActivity = false;

            } // while working - non blocking

        } // if blocking/non blocking

  } // not TCIP Driver

    // Close CANAL channel
    m_pCtrlObject->m_proc_CanalClose ( m_pCtrlObject->m_openHandle );

    // Library is unloaded in destructor

    if ( NULL != m_preceiveThread ) {
        m_preceiveThread->Wait();
        delete m_preceiveThread;
    }

    if ( NULL != m_pwriteThread ) {
        m_pwriteThread->Wait();
        delete m_pwriteThread;
    }

  wxPostEvent( m_pCtrlObject->m_pWnd, eventConnectionLost );

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
// deviceReceiveThread
//



deviceReceiveThread::deviceReceiveThread()
        : wxThread( wxTHREAD_JOINABLE )
{
    m_pMainThreadObj = NULL;
  m_bQuit = false;
}


deviceReceiveThread::~deviceReceiveThread()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceReceiveThread::Entry()
{
    // Must be a valid main object pointer
    if ( NULL == m_pMainThreadObj ) return NULL;

    wxCommandEvent eventReceive( wxVSCP_IN_EVENT, m_pMainThreadObj->m_pCtrlObject->m_windowID );
    canalMsg msg;

    // Blocking receive method must have been found
    if ( NULL == m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingReceive ) return NULL;

    while ( !TestDestroy() && !m_bQuit )
    {

        if ( CANAL_ERROR_SUCCESS == m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingReceive( m_pMainThreadObj->m_pCtrlObject->m_openHandle, &msg, 500 ) )
        {

            vscpEvent *pEvent = new vscpEvent;
            if ( NULL != pEvent ) {

                // Convert CANAL message to VSCP event
                convertCanalToEvent ( pEvent,
                                        &msg,
                                        m_pMainThreadObj->m_pCtrlObject->m_GUID );

                eventReceive.SetClientData( pEvent );
                wxPostEvent( m_pMainThreadObj->m_pCtrlObject->m_pWnd, eventReceive );

            }
        }
    }

    return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceReceiveThread::OnExit()
{
    ;
}





// ****************************************************************************





///////////////////////////////////////////////////////////////////////////////
// deviceWriteThread
//

deviceWriteThread::deviceWriteThread()
        : wxThread(wxTHREAD_JOINABLE)
{
    m_pMainThreadObj = NULL;
  m_bQuit = false;
}


deviceWriteThread::~deviceWriteThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceWriteThread::Entry()
{
  // Must be a valid main object pointer
    if ( NULL == m_pMainThreadObj ) return NULL;

  wxCommandEvent eventReceive( wxVSCP_IN_EVENT, m_pMainThreadObj->m_pCtrlObject->m_windowID );
  eventOutQueue::compatibility_iterator node;
  vscpEvent *pEvent;

    // Must be a valid main object pointer
    if ( NULL == m_pMainThreadObj ) return NULL;

   // Blocking send method must have been found
  if ( NULL == m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingSend ) return NULL;

    while ( !TestDestroy() && !m_bQuit )
    {
    if ( wxSEMA_TIMEOUT == m_pMainThreadObj->m_pCtrlObject->m_semOutQue.WaitTimeout( 500 ) ) continue;
    m_pMainThreadObj->m_pCtrlObject->m_mutexOutQueue.Lock();

    node = m_pMainThreadObj->m_pCtrlObject->m_outQueue.GetFirst();
    pEvent = node->GetData();
    
    canalMsg canalMsg;
        convertEventToCanal ( &canalMsg, pEvent );
        if ( CANAL_ERROR_SUCCESS == 
                            m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingSend ( m_pMainThreadObj->m_pCtrlObject->m_openHandle, &canalMsg, 300 ) ) {
          
      eventReceive.SetClientData( pEvent );

      // Remove the node
            m_pMainThreadObj->m_pCtrlObject->m_outQueue.DeleteNode ( node ); 
      
        }
        else {
            // Give it another try
            m_pMainThreadObj->m_pCtrlObject->m_semOutQue.Post();
        }

    m_pMainThreadObj->m_pCtrlObject->m_mutexOutQueue.Unlock();

    } // while

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceWriteThread::OnExit()
{

}















