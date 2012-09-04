// CanalDiagnosticDoc.h : interface of the CCanalDiagnosticDoc class
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (http://www.vscp.org) 
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: CanalDiagnosticDoc.h,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CANALDIAGNOSTICDOC_H__73D0D1F3_987B_4A42_ADD5_AE5718C9D792__INCLUDED_)
#define AFX_CANALDIAGNOSTICDOC_H__73D0D1F3_987B_4A42_ADD5_AE5718C9D792__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewDocument.h"
#include "../../../../common/canalshmem_level1_win32.h"
#include "../../../../common/dllwrapper.h"
#include "../../../../../common/dllist.h"

#define SHT_INTERFACE		0
#define MAX_NUMBER_OF_MSG	10000

class CCanalDiagnosticDoc : public CDocument
{
protected: // create from serialization only
	CCanalDiagnosticDoc();
	DECLARE_DYNCREATE(CCanalDiagnosticDoc)

// Attributes
public:

// Operations
public:

	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCanalDiagnosticDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

// Implementation
public:

	/*!
		Send a CAN message on selected i/f
	*/
	bool sendMessage( canalMsg *pMsg );

	/*!
		Data check function
		Called from a timer event in the view
	*/
	bool checkForData( void );

	/*!
		Get next message from list
	*/
	bool getNextCanMsg( canalMsg *pMsg );

	/*!
		Set chain pointers to beginning
		of double linked list again.
	*/
	void resetMsgChain( void );

	// Destructor
	virtual ~CCanalDiagnosticDoc( void );

	/*!
		Timer for canal interaction
	*/
	UINT m_uTimerID;

	/*!
		Data for the selected device
		( from registry )
	*/
	devItem m_itemDevice;

	/*!
		The CANAL pipe interface
	*/
	CanalSharedMemLevel1 m_canalif;

	/*!
		The CANAL dll wrapper
	*/
	CDllWrapper m_canalDll;

	/*!
		Flag for open device
	*/
	bool m_bOpen;

	/*!
		Pointer at last node inserted in list
	*/
	dllnode *m_pNode;

	/*!
		Last node fetched
	*/
	dllnode *m_pLastNode;

	/*!
		Message list
	*/
	DoubleLinkedList m_msgList;


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
		
// Generated message map functions
protected:
	//{{AFX_MSG(CCanalDiagnosticDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CANALDIAGNOSTICDOC_H__73D0D1F3_987B_4A42_ADD5_AE5718C9D792__INCLUDED_)
