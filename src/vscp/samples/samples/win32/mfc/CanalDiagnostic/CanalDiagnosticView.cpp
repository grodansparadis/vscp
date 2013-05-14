// CanalDiagnosticView.cpp : implementation of the CCanalDiagnosticView class
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
// $RCSfile: CanalDiagnosticView.cpp,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#include "stdafx.h"
#include "CanalDiagnostic.h"

#include "CanalDiagnosticDoc.h"
#include "CanalDiagnosticView.h"
#include "childfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticView

IMPLEMENT_DYNCREATE(CCanalDiagnosticView, CView)

BEGIN_MESSAGE_MAP(CCanalDiagnosticView, CView)
	//{{AFX_MSG_MAP(CCanalDiagnosticView)
	ON_BN_CLICKED(IDC_RADIO_EXTENDED, OnExtended)
	ON_BN_CLICKED(IDC_RADIO_STANDARD, OnStandard)
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnSendMsg)
	ON_BN_CLICKED(IDC_BUTTON_SENDBURST, OnSendBurst)
	ON_BN_CLICKED(IDC_BUTTON_PREV, OnPrevID)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnNextID)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_DATA, OnClearData)
	ON_EN_KILLFOCUS(IDC_EDIT_DATA0, OnEditData0LostFocus )
	ON_EN_KILLFOCUS(IDC_EDIT_DATA1, OnEditData1LostFocus )
	ON_EN_KILLFOCUS(IDC_EDIT_DATA2, OnEditData2LostFocus )
	ON_EN_KILLFOCUS(IDC_EDIT_DATA3, OnEditData3LostFocus )
	ON_EN_KILLFOCUS(IDC_EDIT_DATA4, OnEditData4LostFocus )
	ON_EN_KILLFOCUS(IDC_EDIT_DATA5, OnEditData5LostFocus )
	ON_EN_KILLFOCUS(IDC_EDIT_DATA6, OnEditData6LostFocus )
	ON_EN_KILLFOCUS(IDC_EDIT_DATA7, OnEditData7LostFocus )
	ON_EN_SETFOCUS(IDC_EDIT_DATA0, OnEditData0LostFocus )
	ON_EN_SETFOCUS(IDC_EDIT_DATA1, OnEditData1LostFocus )
	ON_EN_SETFOCUS(IDC_EDIT_DATA2, OnEditData2LostFocus )
	ON_EN_SETFOCUS(IDC_EDIT_DATA3, OnEditData3LostFocus )
	ON_EN_SETFOCUS(IDC_EDIT_DATA4, OnEditData4LostFocus )
	ON_EN_SETFOCUS(IDC_EDIT_DATA5, OnEditData5LostFocus )
	ON_EN_SETFOCUS(IDC_EDIT_DATA6, OnEditData6LostFocus )
	ON_EN_SETFOCUS(IDC_EDIT_DATA7, OnEditData7LostFocus )
	ON_EN_KILLFOCUS(IDC_EDIT_ID, OnEditIDLostFocus )
	ON_EN_SETFOCUS(IDC_EDIT_ID, OnEditIDLostFocus )
	ON_WM_SIZE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticView construction/destruction

CCanalDiagnosticView::CCanalDiagnosticView()
{
	m_idx = 0;
}

CCanalDiagnosticView::~CCanalDiagnosticView()
{
	int i;

	i = 1;
}

BOOL CCanalDiagnosticView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticView drawing

void CCanalDiagnosticView::OnDraw(CDC* pDC)
{
	CCanalDiagnosticDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticView printing

BOOL CCanalDiagnosticView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCanalDiagnosticView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCanalDiagnosticView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticView diagnostics

#ifdef _DEBUG
void CCanalDiagnosticView::AssertValid() const
{
	CView::AssertValid();
}

void CCanalDiagnosticView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCanalDiagnosticDoc* CCanalDiagnosticView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCanalDiagnosticDoc)));
	return (CCanalDiagnosticDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticView message handlers

void CCanalDiagnosticView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	if ( bActivate ) {		
		
	}
	
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

///////////////////////////////////////////////////////////////////////////////
// OnInitialUpdate
//

void CCanalDiagnosticView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	DWORD dwStyle = LVS_AUTOARRANGE | LVS_SINGLESEL | LVS_ALIGNTOP | LVS_REPORT;

	CRect rc;
	GetWindowRect( &rc );

	m_ctrlList.Create( dwStyle,
							rc,
							this,
							IDC_LIST );

	GetWindowRect( &rc );		
	ScreenToClient( &rc );	
	m_ctrlList.MoveWindow( &rc );

	m_ctrlList.SetBkColor( LIST_BACK_COLOR );
	m_ctrlList.SetTextBkColor( LIST_BACK_COLOR );
	m_ctrlList.SetTextColor( LIST_FONT_COLOR );
	m_ctrlList.SetExtendedStyle( LVS_EX_FULLROWSELECT );

	m_ctrlList.InsertColumn( 0, _T("Dir"), LVCFMT_LEFT, LIST_WIDTH_DIR );
	m_ctrlList.InsertColumn( 1, _T("Time"), LVCFMT_LEFT, LIST_WIDTH_TIME );
	m_ctrlList.InsertColumn( 2, _T("Timestamp"), LVCFMT_LEFT, LIST_WIDTH_TIMESTAMP );
	m_ctrlList.InsertColumn( 3, _T("Flags"), LVCFMT_LEFT, LIST_WIDTH_FLAGS );
	m_ctrlList.InsertColumn( 4, _T("Id"), LVCFMT_LEFT, LIST_WIDTH_ID );
	m_ctrlList.InsertColumn( 5, _T("OBID"), LVCFMT_LEFT, LIST_WIDTH_ID );
	m_ctrlList.InsertColumn( 6, _T("Data"), LVCFMT_LEFT, LIST_WIDTH_DATA );
	m_ctrlList.InsertColumn( 7, _T("Note"), LVCFMT_LEFT, LIST_WIDTH_NOTE /*rc.Width() - 
													LIST_WIDTH_TIME - 
													LIST_WIDTH_TIMESTAMP - 
													LIST_WIDTH_FLAGS - 
													LIST_WIDTH_DATA -
													LIST_WIDTH_ID*/ );
	m_ctrlList.ShowWindow( SW_SHOW );
	m_ctrlList.SetExtendedStyle( LVS_REPORT | LVS_EX_FULLROWSELECT | 
										LVS_EX_ONECLICKACTIVATE | 
										LVS_EX_UNDERLINEHOT | 
										LVS_EX_GRIDLINES );

	m_uTimerID = SetTimer( ID_MAIN_TIMER, 100, NULL );
}

///////////////////////////////////////////////////////////////////////////////
// fillData
//

void CCanalDiagnosticView::fillData( void )
{
	int cnt;
	canalMsg Msg;

	AfxMessageBox("tttt");
	if ( cnt = m_canalif.doCmdDataAvailable() ) {
		for ( int i=0; i<cnt; i++ ) {
			if ( m_canalif.doCmdReceive( &Msg ) ) {
				insertItem( &Msg, m_idx++ );
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// insertItem
//

void CCanalDiagnosticView::insertItem( canalMsg *pMsg, int idx )
{
	LVITEM lvi;
	char buf[ 1024 ];
	char smallbuf[ 32 ];
	char tempbuf[ 32 ];
	time_t ltime;

	idx = 0;

	// Get UNIX-style time and display as number and string. 
    time( &ltime );
	strcpy( tempbuf, ctime( &ltime ) );
	tempbuf[ strlen( tempbuf ) - 1 ] = 0; // Remove /n

	m_ctrlList.InsertItem( idx, "" );
	
	// Direction
	if ( pMsg->flags & CANAL_IDFLAG_SEND ) { 
		strcpy( buf, "TX" );
	}
	else {
		strcpy( buf, "RX" );
	}
	lvi.mask = LVIF_TEXT;
	lvi.iItem = idx;
	lvi.iSubItem = 0;
	lvi.lParam = 1;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );

	// Time
	sprintf( buf, "%s", tempbuf );
	lvi.mask = LVIF_TEXT;
	lvi.iItem = idx;
	lvi.iSubItem = 1;
	lvi.lParam = 1;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );

	// Timestamp
	sprintf( buf, "%08X", pMsg->timestamp );
	lvi.iItem = idx;
	lvi.iSubItem = 2;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );

	// Flags
	sprintf( buf, "%08X", pMsg->flags );
	lvi.iItem = idx;
	lvi.iSubItem = 3;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );

	// Id
	sprintf( buf, "%08X", pMsg->id );
	lvi.iItem = idx;
	lvi.iSubItem = 4;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );

	// OBID
	sprintf( buf, "%08X", pMsg->obid );
	lvi.iItem = idx;
	lvi.iSubItem = 5;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );

	// Data
	sprintf( buf, "(%i) - " , pMsg->sizeData ); 
	for ( int i=0; i < 8; i++ ) {
		if ( i < pMsg->sizeData ) {
			sprintf( smallbuf, "%02X ", pMsg->data[ i ] ); 
			strcat( buf, smallbuf );
		}
	}
	lvi.iItem = idx;
	lvi.iSubItem = 6;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );	


	if ( m_ctrlList.GetItemCount( ) > MAX_LISTITEMS ) {
		m_ctrlList.DeleteItem( MAX_LISTITEMS );
	}
}


///////////////////////////////////////////////////////////////////////////////
// OnSize
//

void CCanalDiagnosticView::OnSize(UINT nType, int cx, int cy) 
{
	CRect rc;

	CView::OnSize(nType, cx, cy);
	
	GetWindowRect( &rc );		
	ScreenToClient( &rc );
	if ( NULL != m_ctrlList.GetSafeHwnd() ) {
		m_ctrlList.MoveWindow( &rc );	
	}
}


///////////////////////////////////////////////////////////////////////////////
// OnStandard
//

void CCanalDiagnosticView::OnStandard()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// OnExtended
//

void CCanalDiagnosticView::OnExtended()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// OnSendMsg
//

void CCanalDiagnosticView::OnSendMsg()
{
	canalMsg Msg;
	getCanData( &Msg );	 
	if ( !GetDocument()->sendMessage( &Msg ) ) {
		AfxMessageBox("Failed to send message!");
	}	
}

///////////////////////////////////////////////////////////////////////////////
// OnPrevID
//

void CCanalDiagnosticView::OnPrevID()
{
	AfxMessageBox("Prev ID is not implemented yet.");
}

///////////////////////////////////////////////////////////////////////////////
// OnNextID
//

void CCanalDiagnosticView::OnNextID()
{
	AfxMessageBox("Next ID is not implemented yet.");
}

///////////////////////////////////////////////////////////////////////////////
// OnSendBurst
//

void CCanalDiagnosticView::OnSendBurst()
{
	AfxMessageBox("Send Burst is not implemented yet.");
}

///////////////////////////////////////////////////////////////////////////////
// OnClearData
//

void CCanalDiagnosticView::OnClearData()
{
	AfxMessageBox("Clear Data is not implemented yet.");
}

 
///////////////////////////////////////////////////////////////////////////////
// OnEditData0LostFocus
//

void CCanalDiagnosticView::OnEditData0LostFocus()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// OnEditData1LostFocus
//

void CCanalDiagnosticView::OnEditData1LostFocus()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// OnEditData2LostFocus
//

void CCanalDiagnosticView::OnEditData2LostFocus()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// OnEditData3LostFocus
//

void CCanalDiagnosticView::OnEditData3LostFocus()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// OnEditData4LostFocus
//

void CCanalDiagnosticView::OnEditData4LostFocus()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// OnEditData5LostFocus
//

void CCanalDiagnosticView::OnEditData5LostFocus()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// OnEditData6LostFocus
//

void CCanalDiagnosticView::OnEditData6LostFocus()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// OnEditData7LostFocus
//

void CCanalDiagnosticView::OnEditData7LostFocus()
{
	;
}


///////////////////////////////////////////////////////////////////////////////
// OnEditIDLostFocus
//

void CCanalDiagnosticView::OnEditIDLostFocus()
{
	;
}


//////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long CCanalDiagnosticView::getDataValue( const char *szData )
{
	unsigned long val;
	char *nstop;

	if ( ( NULL != strchr( szData, 'x' ) ) ||
			( NULL != strchr( szData, 'X' ) ) ) {
		val = strtoul( szData, &nstop, 16 );
	}
	else {
		val = strtoul( szData, &nstop, 10 );
	}

	return val;	
}


//////////////////////////////////////////////////////////////////////////////
// getCANData
//

bool CCanalDiagnosticView::getCanData( canalMsg *pMsg )
{
	bool rv = false;
	unsigned char nData = 0;

	// Must have somewhere to store data
	if ( NULL == pMsg ) return rv;

	// Initialize message structure
	pMsg->flags = 0;
	pMsg->id = 0;
	pMsg->obid = 0;
	pMsg->sizeData = 0;
	pMsg->timestamp = 0;

	for ( int i=0; i<8; i++ ) {
		pMsg->data[ i ] = 0;
	}

	char buf[ 256 ];

	CChildFrame *pWnd = ( CChildFrame * )GetParent();
	ASSERT ( NULL != pWnd );
	
	m_ctrlOptStandard  = ( CButton * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_RADIO_STANDARD );
	ASSERT ( NULL != m_ctrlOptStandard );
	
	m_ctrlOptExtended = ( CButton * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_RADIO_EXTENDED );
	ASSERT ( NULL != m_ctrlOptExtended );

	m_ctrlOptRtr = ( CButton * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_CHECK_RTR );
	ASSERT ( NULL != m_ctrlOptRtr );

	m_ctrlEditID = ( CEdit * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_EDIT_ID );
	ASSERT ( NULL != m_ctrlEditID );		

	m_ctrlEditData0 = ( CEdit * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_EDIT_DATA0 );
	ASSERT ( NULL != m_ctrlEditData0 );
	
	m_ctrlEditData1 = ( CEdit * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_EDIT_DATA1 );
	ASSERT ( NULL != m_ctrlEditData1 );
	
	m_ctrlEditData2 = ( CEdit * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_EDIT_DATA2 );
	ASSERT ( NULL != m_ctrlEditData2 );
	
	m_ctrlEditData3 = ( CEdit * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_EDIT_DATA3 );
	ASSERT ( NULL != m_ctrlEditData3 );
	
	m_ctrlEditData4 = ( CEdit * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_EDIT_DATA4 );
	ASSERT ( NULL != m_ctrlEditData4 );
	
	m_ctrlEditData5 = ( CEdit * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_EDIT_DATA5 );
	ASSERT ( NULL != m_ctrlEditData5 );
	
	m_ctrlEditData6 = ( CEdit * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_EDIT_DATA6 );
	ASSERT ( NULL != m_ctrlEditData6 );
	
	m_ctrlEditData7 = ( CEdit * )pWnd->m_wndCanalChannelControlBar.GetDlgItem( IDC_EDIT_DATA7 );
	ASSERT ( NULL != m_ctrlEditData7 );

	// Get ID
	m_ctrlEditID->GetWindowText( buf, sizeof( buf ) );
	pMsg->id = getDataValue( buf ); 

	// Is the ID valid?
	if ( pMsg->id  > 0x1FFFFFFF ) return false;
	
	// Get Data 7
	m_ctrlEditData7->GetLine( 0, buf, sizeof( buf ) );
	pMsg->data[ 7 ] = (unsigned char)getDataValue( buf );
	if ( strlen( buf ) > 0 ) nData = 8;

	// Get Data 6
	m_ctrlEditData6->GetLine( 0, buf, sizeof( buf ) );
	pMsg->data[ 6 ] = (unsigned char)getDataValue( buf );
	if ( ( 0 == nData ) && ( strlen( buf ) > 0  ) ) nData = 7;
	
	// Get Data 5
	m_ctrlEditData5->GetLine( 0, buf, sizeof( buf ) );
	pMsg->data[ 5 ] = (unsigned char)getDataValue( buf );
	if ( ( 0 == nData ) && ( strlen( buf ) > 0  ) ) nData = 6;
	
	// Get Data 4
	m_ctrlEditData4->GetLine( 0, buf, sizeof( buf ) );
	pMsg->data[ 4 ] = (unsigned char)getDataValue( buf );
	if ( ( 0 == nData ) && ( strlen( buf ) > 0  ) ) nData = 5;
	
	// Get Data 3
	m_ctrlEditData3->GetLine( 0, buf, sizeof( buf ) );
	pMsg->data[ 3 ] = (unsigned char)getDataValue( buf );
	if ( ( 0 == nData ) && ( strlen( buf ) > 0  ) ) nData = 4;
	
	// Get Data 2
	m_ctrlEditData2->GetLine( 0, buf, sizeof( buf ) );
	pMsg->data[ 2 ] = (unsigned char)getDataValue( buf );
	if ( ( 0 == nData ) && ( strlen( buf ) > 0  ) ) nData = 3;
	
	// Get Data 1
	m_ctrlEditData1->GetLine( 0, buf, sizeof( buf ) );
	pMsg->data[ 1 ] = (unsigned char)getDataValue( buf );
	if ( ( 0 == nData ) && ( strlen( buf ) > 0  ) ) nData = 2;
	
	// Get Data 0
	m_ctrlEditData0->GetLine( 0, buf, sizeof( buf ) );
	pMsg->data[ 0 ] = (unsigned char)getDataValue( buf );
	if ( ( 0 == nData ) && ( strlen( buf ) > 0  ) ) nData = 1;
	
	pMsg->sizeData = nData;

	// Check for Extended message
	if ( m_ctrlOptExtended->GetCheck() ) {
		pMsg->flags |= CANAL_IDFLAG_EXTENDED;
	}

	// Check for Remote Frame
	if ( m_ctrlOptRtr->GetCheck() ) {
		pMsg->flags |= CANAL_IDFLAG_RTR;
	}
	
	// Write back data to dialog
	writeDataToDialog( pMsg );
	
	return rv;
}	

//////////////////////////////////////////////////////////////////////////////
// writeDataToDialog
//

bool CCanalDiagnosticView::writeDataToDialog( canalMsg *pMsg )
{
	bool rv = true;

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// OnUpdate
//

void CCanalDiagnosticView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	canalMsg Msg;

	while ( GetDocument()->getNextCanMsg( &Msg ) ) {
		insertItem( &Msg, m_idx );
	}		
}

///////////////////////////////////////////////////////////////////////////////
// OnTimer
//

void CCanalDiagnosticView::OnTimer(UINT nIDEvent) 
{
	if ( NULL != GetDocument() ) {
		if ( GetDocument()->checkForData() ) {
			GetDocument()->UpdateAllViews( NULL );
		}
	}

	CView::OnTimer(nIDEvent);
}

