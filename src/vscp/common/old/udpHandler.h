///////////////////////////////////////////////////////////////////////////////
// udphandler.h 
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $RCSfile: udpHandler.h,v $                                       
// $Date: 2005/01/05 12:16:10 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPHANDLER_H__798138BE_5902_4BB2_9B6D_E3F1FFF80CA5__INCLUDED_)
#define AFX_UDPHANDLER_H__798138BE_5902_4BB2_9B6D_E3F1FFF80CA5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//

#include "../common/dllist.h"

#define MAX_UDP_RCV_MESSAGES	1024
#define UDP_OBJ_MUTEX	"_____CANAL_UDP_MUTEX_______"
#define	CANAL_UDP_PORT	9999

class CUDPRecord : public CObject 
{

	//DECLARE_DYNAMIC( CUDPRecord )
	
public:
	CString m_IPAddress;
	UINT m_Port;
	int m_Size;
};


/////////////////////////////////////////////////////////////////////////////
// CudpHandler command target

class CudpHandler : public CAsyncSocket
{
// Attributes
public:

// Operations
public:
	CudpHandler( void );
	virtual ~CudpHandler( void );

	void Create( DoubleLinkedList * pList );

// Overrides
public:

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CudpHandler)
	public:
	virtual void OnReceive(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CudpHandler)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:

	/*!
		receive queue

		This is the receive queue for VSCP UDP messages
	*/
	DoubleLinkedList *m_pListUdpReceive;


	/*!
		Mutex for queue
	*/
	HANDLE m_udpReceiveMutex;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UDPHANDLER_H__798138BE_5902_4BB2_9B6D_E3F1FFF80CA5__INCLUDED_)
