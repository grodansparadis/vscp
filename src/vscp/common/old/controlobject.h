// ControlObject.h: interface for the CControlObject class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia, <akhe@eurosource.se>
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
// $RCSfile: controlobject.h,v $                                       
// $Date: 2005/01/05 12:50:53 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#if !defined(AFX_CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define AFX_CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#ifdef __WXMSW__
    #include  "wx/ownerdrw.h"
#endif

#include "../../canal/common/canalpipe.h"
#include "clientlist.h"
//#include "udphandler.h"
#include "vscp.h"

#ifdef WIN32
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

#define MAX_ITEMS_RECEIVE_QUEUE				1024
#define MAX_ITEMS_SEND_QUEUE				1024
#define MAX_ITEMS_CLIENT_RECEIVE_QUEUE		1024
#define MAX_ITEMS_UDP_RECEIVE_QUEUE			1024

class CControlObject  
{

public:

	/*!
		Constructor
	*/
	CControlObject( void );
	
	/*!
		Destructor
	*/
	virtual ~CControlObject( void );

	/*!
		General initialization
	*/
	bool init( void );

	/*!
		Clean up used resources
	*/
	bool cleanup( void );

	/*! 
		The main worker thread
	*/
	bool run( void );

	/*!
		Work...
	*/
	bool doWork( bool *bTraffic );


	/*! 
		Load persistent data
	*/
	void loadRegistryData( void );

	/*!
		Save persistent data
	*/
	void saveRegistryData( void );


	/*!
		Add a new client to the clinet list

		@param Pointer to client that should be added.
	*/
	void addClient( CClientItem *pClientItem );

	/*!
		Remove a new client from the clinet list

		@param Pointer to client that should be added.
	*/
	void removeClient( CClientItem *pClientItem );


	/*!
		Send message as UDP datagram
     */
	bool sendUdpMessage( vscpMsg2 * pMsg );

	/*!
		Send message as CANAL message
     */
	bool sendCanalMsg( vscpMsg2 * pMsg );


public:

	/*!
		true if we should quit
	*/
	bool m_bQuit;		

	/*! 
		Send queue

		This is the send queue for all clients attached to the system
	*/
	DoubleLinkedList m_sendQueue;


	/*! 
		Receive queue

		This is the receive queue for TCP
	*/
	DoubleLinkedList m_udpReceiveQueue;

#ifdef WIN32
#ifndef BUILD_VSCPD_SERVICE

	/*!
		Pointer to main window
	*/
	//CWnd *m_pwnd;

#endif
#endif

	/*!
		The CANAL pipe interface
	*/
	CCanalPipe m_canalPipe;


	/*!
		Variable to indicate if the canal pipe i/f
		is open
	*/
	bool m_bCanalOpen;
  

private:

	/*!
		UDP sending socket
	*/
#ifdef WIN32
	SOCKET m_sendsock;
#else
	int m_sendsock;
#endif


	/*!
		The list with active clients.
	*/
	CClientList m_clientList;	

#ifdef WIN32
	HICON m_hIcon;
#endif

	/// Client List Mutex
#ifdef WIN32	
	HANDLE m_clientMutex;
#endif
	
	/// Send Mutex
#ifdef WIN32	
	HANDLE m_sendMutex;
#endif

	/// UDP Receive Mutex
#ifdef WIN32	
	HANDLE m_udpReceiveMutex;
#endif

};


 /// Structure used to pass to clientThread from the listenThread
struct _clientstruct
{
	CControlObject *pctrlObject;
#ifdef WIN32
	HANDLE hPipe;
#else
	int pipe;
#endif
};


#endif // !defined(AFX_CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)

