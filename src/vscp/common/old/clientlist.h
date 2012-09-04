// ClientList.h: interface for the CClientList class.
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
// $RCSfile: clientlist.h,v $                                       
// $Date: 2005/01/05 12:50:53 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTLIST_H__B0190EE5_E0E8_497F_92A0_A8616296AF3E__INCLUDED_)
#define AFX_CLIENTLIST_H__B0190EE5_E0E8_497F_92A0_A8616296AF3E__INCLUDED_

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

#include "../../common/dllist.h"

class CClientItem
{

public:

	/// Constructor
	CClientItem( void );

	/// Destructor
	virtual ~CClientItem( void );

public:

	/**
		Mutex handle that is used for sharing of the client object
	*/
#ifdef WIN32	
	HANDLE m_hclientMutex;
#else
	pthread_mutex_t  m_clientMutex;
#endif

	/**
		Input Queue
	*/
	DoubleLinkedList m_inputQueue;

	/**
		Client ID
	*/
	unsigned long m_clientID;

	/**
		Flag for open/closed channel
	*/
	bool m_bOpen;

	/**
		Channel flags
	*/
	unsigned long m_flags;

	/**
		Channel filter
	*/
	unsigned long m_filter;

	/**
		Channel mask
	*/
	unsigned long m_mask;
 
};


class CClientList  
{
public:

	/// Constructor
	CClientList();

	/// Destructor
	virtual ~CClientList();

	/*!
		Add a client to the list
	*/
	bool addClient( CClientItem *pClientItem );

	/*!
		Remove a client from the list
	*/
	bool removeClient( CClientItem *pClientItem );
	
	/*!
		Remove data from packet list
	*/
	void removeData( DoubleLinkedList* pdll );

public:

	/**
		Active client list
	*/
	DoubleLinkedList m_clientList;

	// Counter for client id's
	unsigned long m_clientIDCounter;

protected:



	/**
		System assigned ID for device
	*/
	unsigned long m_clientID;

};

#endif // !defined(AFX_CLIENTLIST_H__B0190EE5_E0E8_497F_92A0_A8616296AF3E__INCLUDED_)
