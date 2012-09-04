// CanalDiagnosticView.h : interface of the CCanalDiagnosticView class
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
// $RCSfile: CanalDiagnosticView.h,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CANALDIAGNOSTICVIEW_H__ACE1079A_3343_407F_A01A_9352F8A94EDC__INCLUDED_)
#define AFX_CANALDIAGNOSTICVIEW_H__ACE1079A_3343_407F_A01A_9352F8A94EDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/// List colors
#define LIST_FONT_COLOR			RGB( 0,0,255 )
#define LIST_BACK_COLOR			RGB( 240,255,255 )

/// Column sizes
#define LIST_WIDTH_DIR				25
#define LIST_WIDTH_TIME				150
#define LIST_WIDTH_TIMESTAMP		75
#define LIST_WIDTH_FLAGS			75
#define LIST_WIDTH_ID				75
#define LIST_WIDTH_DATA				150
#define LIST_WIDTH_NOTE				600

/// Max number of lits items
#define MAX_LISTITEMS				1000

#include "../../../../common/canalshmem_level1_win32.h"


class CCanalDiagnosticView : public CView
{
protected: // create from serialization only
	CCanalDiagnosticView();
	DECLARE_DYNCREATE(CCanalDiagnosticView)

	/*!
		
	*/
	void fillData( void );

	/*!
		
	*/
	void insertItem( canalMsg *pMsg, int idx );

	/*!
		Get datavalue from string. Accepts hex (0x...) or
		decimal data.
	*/
	unsigned long getDataValue( const char *szData );

	/*!
		Fill canalMsg structure from the dialog
	*/
	bool getCanData( canalMsg *pMsg );

	/*!
		Write CAN data to dialog
	*/
	bool writeDataToDialog( canalMsg *pMsg );

// Attributes
public:
	CCanalDiagnosticDoc* GetDocument();

	// The list window
	CListCtrl	m_ctrlList;

	// The canal pipe i/f
	CanalSharedMemLevel1 m_canalif;
	//CCanalSuperWrapper m_canalif;

	/// Index in display list
	int m_idx;

	/// Timer id
	UINT m_uTimerID;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCanalDiagnosticView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCanalDiagnosticView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	CButton *m_ctrlOptStandard;
	CButton *m_ctrlOptExtended;

	CButton *m_ctrlOptRtr;

	CEdit *m_ctrlEditID;

	CEdit *m_ctrlEditData0;
	CEdit *m_ctrlEditData1;
	CEdit *m_ctrlEditData2;
	CEdit *m_ctrlEditData3;
	CEdit *m_ctrlEditData4;
	CEdit *m_ctrlEditData5;
	CEdit *m_ctrlEditData6;
	CEdit *m_ctrlEditData7;

// Generated message map functions
protected:
	//{{AFX_MSG(CCanalDiagnosticView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnStandard();
	afx_msg void OnExtended();
	afx_msg void OnSendMsg();
	afx_msg void OnSendBurst();
	afx_msg void OnPrevID();
	afx_msg void OnNextID();
	afx_msg void OnClearData();
	
	afx_msg void OnEditData0LostFocus();
	afx_msg void OnEditData1LostFocus();
	afx_msg void OnEditData2LostFocus();
	afx_msg void OnEditData3LostFocus();
	afx_msg void OnEditData4LostFocus();
	afx_msg void OnEditData5LostFocus();
	afx_msg void OnEditData6LostFocus();
	afx_msg void OnEditData7LostFocus();
	
	afx_msg void OnEditIDLostFocus();
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CanalDiagnosticView.cpp
inline CCanalDiagnosticDoc* CCanalDiagnosticView::GetDocument()
   { return (CCanalDiagnosticDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CANALDIAGNOSTICVIEW_H__ACE1079A_3343_407F_A01A_9352F8A94EDC__INCLUDED_)
