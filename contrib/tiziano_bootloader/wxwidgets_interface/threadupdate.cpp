/////////////////////////////////////////////////////////////////////////////
// Name:        threadupdate.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sat 30 Jun 2007 14:08:14 CEST
// RCS-ID:      
// Copyright:   (C) 2007 Ake Hedman, D Of Scandinavia, <ake@dofscandinavia.com>
// Licence:     
// This program is free software; you can redistribute it and/or  
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://can.sourceforge.net) 
//
// Copyright (C) 2000-2008 Ake Hedman, D Of Scandinavia, <ake@dofscandinavia.com>
// 
// This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.
// 
//  As a special exception, if other files instantiate templates or use macros
//  or inline functions from this file, or you compile this file and link it
//  with other works to produce a work based on this file, this file does not
//  by itself cause the resulting work to be covered by the GNU General Public
//  License. However the source code for this file must still be made available
//  in accordance with section (3) of the GNU General Public License.
// 
//  This exception does not invalidate any other reasons why a work based on
//  this file might be covered by the GNU General Public License.
// 
//  Alternative licenses for VSCP & Friends may be arranged by contacting 
//  D of Scandinavia at info@dofscandinavia.com, http://www.dofscandinavia.com
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "threadupdate.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/app.h>
#include <wx/listimpl.cpp>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/dynlib.h>
#include <wx/progdlg.h>

#include "../pronetapp.h"
#include "ctrlobj.h"
#include "../../common/canal.h"
#include "../../common/vscp.h"
#include "../../common/vscp_classes.h"
#include "../../common/vscp_type.h"
#include "../../common/vscphelper.h"
#include "../System/vscpregisterdef.h"
#include "ctrlupdate.h"
#include "threadupdate.h"

DEFINE_EVENT_TYPE(wxUPDATE_EVENT)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ThreadCtrl type definition
//

IMPLEMENT_CLASS( ThreadUpdate, wxEvtHandler )

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ThreadUpdate constructors
//

ThreadUpdate::ThreadUpdate()
{
  	Init();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ThreadUpdate destructor
//

ThreadUpdate::~ThreadUpdate()
{
  	// Stop the worker threads
  	//stopWorkerThreads();

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void ThreadUpdate::Init()
{
  	m_pCtrlObject = NULL;
	m_pWorkerThread = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void ThreadUpdate::Close( )
{
  	// Stop the worker threads
  	stopWorkerThreads();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// startWorkerThreads
//

int ThreadUpdate::startWorkerThreads( )
{
	int ret = 0;
	wxBusyCursor wait;

	if ( INTERFACE_VSCP ==  m_pCtrlObject->m_interfaceType ) {

  		/////////////////////////////////////////////////////////////////////////////
		// Load controlobject control handler
		/////////////////////////////////////////////////////////////////////////////
		
  		m_pWorkerThread = new UPDATEWorkerThread;

		if ( NULL != m_pWorkerThread )
		{
    		m_pWorkerThread->m_pCtrlObject = m_pCtrlObject;
			m_pWorkerThread->m_pCtrl = this;
			wxThreadError err;
			if ( wxTHREAD_NO_ERROR == ( err = m_pWorkerThread->Create() ) ) {
		  		m_pWorkerThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
				if ( wxTHREAD_NO_ERROR != ( err = m_pWorkerThread->Run() ) ) {
			  		::wxGetApp().logMsg ( _("Unable to run controlobject device thread."), DAEMON_LOGMSG_CRITICAL );
					ret = 1;
				}
			}
			else {
				::wxGetApp().logMsg ( _("Unable to create controlobject device thread."), DAEMON_LOGMSG_CRITICAL );
				ret = 2;
			}
		}
		else
		{
			::wxGetApp().logMsg ( _("Unable to allocate memory for controlobject device thread."), DAEMON_LOGMSG_CRITICAL );
			ret = 3;
		}
	}
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stopWorkerThreads
//

void ThreadUpdate::stopWorkerThreads( void )
{
  	wxBusyCursor wait;

  	m_pCtrlObject->m_bQuit = true;

	wxMilliSleep( 300 );


	if ( INTERFACE_VSCP ==  m_pCtrlObject->m_interfaceType ) {
 		if ( NULL != m_pWorkerThread ) {
    		m_pWorkerThread->Wait();
      		delete m_pWorkerThread;
      		m_pWorkerThread = NULL;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// UPDATEWorkerThread
//

UPDATEWorkerThread::UPDATEWorkerThread()
                : wxThread( wxTHREAD_JOINABLE )
{
	m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// transmitWorkerThread
//

UPDATEWorkerThread::~UPDATEWorkerThread()
{

}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *UPDATEWorkerThread::Entry()
{
  	bool f_loop = true;
	bool bResend = false;
	int i,size;
	eventOutQueue::compatibility_iterator node;
  	vscpEvent *pEvent;
	wxCommandEvent eventConnectionLost( wxVSCP_CTRL_LOST );
	wxCommandEvent eventMessage( wxUPDATE_EVENT );
  
  	/// TCP/IP Control
  	VscpTcpIf tcpifControl;
  
  	// Must be a valid control object pointer
	if ( NULL == m_pCtrlObject ) return NULL;

  
  	// Connect to the server with the control interface
  	if ( !tcpifControl.doCmdOpen( m_pCtrlObject->m_ifVSCP.m_strHost,
                                m_pCtrlObject->m_ifVSCP.m_port,
                                m_pCtrlObject->m_ifVSCP.m_strUser,
                                m_pCtrlObject->m_ifVSCP.m_strPassword ) ) {
    	::wxGetApp().logMsg ( _("VSCP TX thread - Unable to connect to server."), DAEMON_LOGMSG_CRITICAL );
    	m_pCtrlObject->m_errorControl = VSCP_SESSION_ERROR_UNABLE_TO_CONNECT;
		wxPostEvent( m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost );
    	return NULL;
  	}
  
  	// Get channel ID
  	tcpifControl.doCmdGetChannelID( &m_pCtrlObject->m_txChannelID );

	// Set filter
	if ( CANAL_ERROR_SUCCESS != 
		tcpifControl.doCmdFilter( &m_pCtrlObject->m_ifVSCP.m_vscpfilter ) ) {
		::wxGetApp().logMsg ( _("TCP/IP thread - Failed to set filter."), DAEMON_LOGMSG_INFO );
	}

	while ( !TestDestroy() && !m_pCtrlObject->m_bQuit )
	{
		if ( wxSEMA_TIMEOUT == m_pCtrlObject->m_semOutQue.WaitTimeout( 10 ) ) continue;

		vscpEvent* pSendEvent = new vscpEvent;
    	m_pCtrlObject->m_mutexOutQueue.Lock();
    	node = m_pCtrlObject->m_outQueue.GetFirst();
    	pEvent = node->GetData();
		copyVSCPEvent( pSendEvent, pEvent );
    	m_pCtrlObject->m_outQueue.DeleteNode( node );
    	deleteVSCPevent( pEvent );
    	m_pCtrlObject->m_mutexOutQueue.Unlock();

		tcpifControl.doCmdSend( pSendEvent );

		size = 0;
		switch(pSendEvent->vscp_type)
		{
			case VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER:
			case VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA:
			case VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE:
			case VSCP_TYPE_PROTOCOL_START_BLOCK:
				f_loop = true;
				break;
			case VSCP_TYPE_PROTOCOL_BLOCK_DATA:
				if(((pSendEvent->GUID[3] << 8) | (pSendEvent->GUID[4])) + 1 == BLOCKDATA_SIZE/8)
				{
					f_loop = true;
				}
				else
				{
					f_loop = false;
					wxMilliSleep( 2 );
					deleteVSCPevent( pSendEvent );
				}
				break;
		}

		wxDateTime start = wxDateTime::Now();

		bResend = false;
		
		/////////////////////////////////////////////////////////////////////////////
		// loop
		while(f_loop)
		{
			if ( tcpifControl.doCmdDataAvailable() ) {
				vscpEvent* pReceiveEvent = new vscpEvent;
				wxString strData;
				m_pCtrlObject->m_pEvent = pReceiveEvent;
				if ( CANAL_ERROR_SUCCESS == tcpifControl.doCmdReceive(  pReceiveEvent  ) ) {
					// Event received
					if ( VSCP_CLASS_PROTOCOL_FUNCTIONALITY == pReceiveEvent->vscp_class ) 
					{
						/////////////////////////////////////////////////////////////////////////
						switch(pReceiveEvent->vscp_type)
						{
							case VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER:
								if((pSendEvent->pdata[0] == pReceiveEvent->GUID[0]) && (pSendEvent->vscp_type == VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER))
								{
									// OK
									// i = type e id
									i = (VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER << 16) | pSendEvent->pdata[0]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(0);	// 0 = OK
									writeVscpDataToString( pReceiveEvent, strData );
									eventMessage.SetString(strData);
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								else
								{
									// ERROR
									i = (VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER << 16) | pSendEvent->pdata[0]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(1);
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								f_loop = false;
								break;
							case VSCP_TYPE_PROTOCOL_NACK_BOOT_LOADER:
								if((pSendEvent->pdata[0] == pReceiveEvent->GUID[0]) && (pSendEvent->vscp_type == VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER))
								{
									i = (VSCP_TYPE_PROTOCOL_NACK_BOOT_LOADER << 16) | pSendEvent->pdata[0]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(0);	// l' errore è gestito nell' handler dell' evento di risposta
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								else
								{
									i = (VSCP_TYPE_PROTOCOL_NACK_BOOT_LOADER << 16) | pSendEvent->pdata[0]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(1);	// error
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								f_loop = false;
								break;
							case VSCP_TYPE_PROTOCOL_START_BLOCK_ACK:
								if(pSendEvent->vscp_type == VSCP_TYPE_PROTOCOL_START_BLOCK)
								{
									i = (((VSCP_TYPE_PROTOCOL_START_BLOCK_ACK) << 16) | pSendEvent->pdata[2]); 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(0);
									writeVscpDataToString( pReceiveEvent, strData );
									eventMessage.SetString(strData);
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								else
								{
									i = (((VSCP_TYPE_PROTOCOL_START_BLOCK_ACK) << 16) | pSendEvent->pdata[2]); 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(1);	// error
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								f_loop = false;
								break;
							case VSCP_TYPE_PROTOCOL_START_BLOCK_NACK:
								if(pSendEvent->vscp_type == VSCP_TYPE_PROTOCOL_START_BLOCK)
								{
									i = (((VSCP_TYPE_PROTOCOL_START_BLOCK_NACK) << 16) | pSendEvent->pdata[2]); 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(0);	// l' errore è gestito nell' handler dell' evento di risposta
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								else
								{
									i = (((VSCP_TYPE_PROTOCOL_START_BLOCK_NACK) << 16) | pSendEvent->pdata[2]); 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(1);	// error
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								f_loop = false;
								break;
							case VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK:
								if(pSendEvent->vscp_type == VSCP_TYPE_PROTOCOL_BLOCK_DATA)
								{
									i = (VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK << 16) | pSendEvent->GUID[12]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(0);
									writeVscpDataToString( pReceiveEvent, strData );
									eventMessage.SetString(strData);
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								else
								{
									i = (VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK << 16) | pSendEvent->GUID[12]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(1);	// error
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								f_loop = false;
								break;
							case VSCP_TYPE_PROTOCOL_BLOCK_DATA_NACK:
								if(pSendEvent->vscp_type == VSCP_TYPE_PROTOCOL_BLOCK_DATA)
								{
									i = (VSCP_TYPE_PROTOCOL_BLOCK_DATA_NACK << 16) | pSendEvent->GUID[12]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(0);	// l' errore è gestito nell' handler dell' evento di risposta
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								else
								{
									i = (VSCP_TYPE_PROTOCOL_BLOCK_DATA_NACK << 16) | pSendEvent->GUID[12]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(1);	// error
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								f_loop = false;
								break;
							case VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK:
								if(pSendEvent->vscp_type == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA)
								{
									i = (VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK << 16) | pSendEvent->GUID[12]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(0);
									writeVscpDataToString( pReceiveEvent, strData );
									eventMessage.SetString(strData);
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								else
								{
									i = (VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK << 16) | pSendEvent->GUID[12]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(1);	// error
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								f_loop = false;
								break;
							case VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_NACK:
								if(pSendEvent->vscp_type == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA)
								{
									i = (VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_NACK << 16) | pSendEvent->GUID[12]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(0);	// l' errore è gestito nell' handler dell' evento di risposta
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								else
								{
									i = (VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_NACK << 16) | pSendEvent->GUID[12]; 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(1);	// error
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								f_loop = false;
								break;
							case VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_ACK:
								if(pSendEvent->vscp_type == VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE)
								{
									i = (((VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_ACK) << 16) | (pSendEvent->GUID[12])); 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(0);
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								else
								{
									i = (((VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_ACK) << 16) | (pSendEvent->GUID[12])); 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(1);	// error
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								f_loop = false;
								break;
							case VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_NACK:
								if(pSendEvent->vscp_type == VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE)
								{
									i = (((VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_NACK) << 16) | (pSendEvent->GUID[12])); 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(0);	// l' errore è gestito nell' handler dell' evento di risposta
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								else
								{
									i = (((VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_NACK) << 16) | (pSendEvent->GUID[12])); 
									eventMessage.SetInt(i);
									eventMessage.SetExtraLong(1);	// error
									wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
									deleteVSCPevent( pReceiveEvent );
									deleteVSCPevent( pSendEvent );
									wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
									m_pCtrlObject->m_pCondition->Signal();
								}
								f_loop = false;
								break;
							default:
								deleteVSCPevent( pReceiveEvent );
								continue;
								break;
						}
						//////////////////////////////////////////////////////////////////////////
					}
					else
					{
						deleteVSCPevent( pReceiveEvent );
						continue;
					}
				}
				else
				{
					deleteVSCPevent( pReceiveEvent );
					continue;
				}
			}
			else {
				wxMilliSleep( 100 );
			}
			if ( (wxDateTime::Now() - start).GetSeconds() > 10 ) {
				// TIMEOUT		
				eventMessage.SetInt(pSendEvent->GUID[12]);
				eventMessage.SetExtraLong(2);
				wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventMessage );
				deleteVSCPevent( pSendEvent );
				wxMutexLocker lock( *m_pCtrlObject->m_pMutex );
				m_pCtrlObject->m_pCondition->Signal();
				f_loop = false;
			}
			else if( (wxDateTime::Now() - start).GetSeconds() > 4 ) {
				// Send again
				if ( !bResend) {
					tcpifControl.doCmdSend( pSendEvent );
				}
				bResend = true;
			}
		}	
		/////////////////////////////////////////////////////////////////////////////
	} // while

	// Close the interface
  	tcpifControl.doCmdClose();

	return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void UPDATEWorkerThread::OnExit()
{

}


///////////////////////////////////////////////////////////////////////////////
