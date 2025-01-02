// ChildFrm.cpp : implementation of the CChildFrame class
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (https://www.vscp.org) 
//
// Copyright (C) 2000-2025 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: ChildFrm.cpp,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#include <pch.h>
#include "CanalDiagnostic.h"

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	ON_WM_INITMENUPOPUP()
	ON_WM_MENUSELECT()
	ON_WM_CREATE()
	ON_COMMAND_EX(CG_ID_VIEW_CANALCHANNELCONTROL, OnBarCheck) 
	ON_UPDATE_COMMAND_UI(CG_ID_VIEW_CANALCHANNELCONTROL, OnUpdateControlBarMenu)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_MENU_DISPLAY_HEX, OnMenuDisplayHex)
	ON_COMMAND(ID_MENU_DISPLAY_RECEIVED, OnMenuDisplayReceived)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
	
}

///////////////////////////////////////////////////////////////////////////////
// CChildFrame
//

CChildFrame::~CChildFrame()
{
}

///////////////////////////////////////////////////////////////////////////////
// PreCreateWindow
//

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

///////////////////////////////////////////////////////////////////////////////
// Dump
//

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers


///////////////////////////////////////////////////////////////////////////////
// OnCreate
//

int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add a menu item that will toggle the visibility of the
	// dialog bar named "Canal channel control":
	//   1. In ResourceView, open the menu resource that is used by
	//      the CChildFrame class
	//   2. Select the View submenu
	//   3. Double-click on the blank item at the bottom of the submenu
	//   4. Assign the new item an ID: CG_ID_VIEW_CANALCHANNELCONTROL
	//   5. Assign the item a Caption: Canal channel control

	// TODO: Change the value of CG_ID_VIEW_CANALCHANNELCONTROL to an appropriate value:
	//   1. Open the file resource.h
	// CG: The following block was inserted by the 'Dialog Bar' component

	{
		// Initialize dialog bar m_wndCanalChannelControl
		if ( !m_wndCanalChannelControlBar.Create( this, 
												CG_IDD_CANALCHANNELCONTROL,
			CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_HIDE_INPLACE,
			CG_ID_VIEW_CANALCHANNELCONTROL))
		{
			TRACE0("Failed to create dialog bar m_wndCanalChannelControl\n");
			return -1;		// fail to create
		}

		m_wndCanalChannelControlBar.EnableDocking( CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM );
		EnableDocking( CBRS_ALIGN_ANY );
		DockControlBar( &m_wndCanalChannelControlBar );
		
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OnInitMenuPopup
//

void CChildFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
  	CMDIChildWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
  
	// CG: The following block was inserted by 'Status Bar' component.
	{
		GetParentFrame()->PostMessage(WM_INITMENUPOPUP,
			(WPARAM)pPopupMenu->GetSafeHmenu(), MAKELONG(nIndex, bSysMenu));
	}

}

///////////////////////////////////////////////////////////////////////////////
// OnMenuSelect
//

void CChildFrame::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
  	CMDIChildWnd::OnMenuSelect(nItemID, nFlags, hSysMenu);
  
	// CG: The following block was inserted by 'Status Bar' component.
	{
		static BOOL bMenuActive = FALSE;
		if (bMenuActive || hSysMenu != NULL)
		{
			GetParentFrame()->PostMessage(WM_MENUSELECT, 
				MAKELONG(nItemID, nFlags), (LPARAM)hSysMenu);
		}
		bMenuActive = hSysMenu != NULL;
	}

}

///////////////////////////////////////////////////////////////////////////////
// OnSize
//

void CChildFrame::OnSize(UINT nType, int cx, int cy) 
{	
	CMDIChildWnd::OnSize(nType, cx, cy);
}

///////////////////////////////////////////////////////////////////////////////
// ActivateFrame
//

void CChildFrame::ActivateFrame(int nCmdShow) 
{
	( ( CButton * )( m_wndCanalChannelControlBar.GetDlgItem( IDC_RADIO_STANDARD ) ) )->SetCheck( BST_CHECKED ); 	
	CMDIChildWnd::ActivateFrame(nCmdShow);
}

void CChildFrame::OnClose() 
{
	//CDocument::OnCloseDocument();
	
	CMDIChildWnd::OnClose();
}

void CChildFrame::OnMenuDisplayHex() 
{
	// TODO: Add your command handler code here
	
}

void CChildFrame::OnMenuDisplayReceived() 
{
	// TODO: Add your command handler code here
	
}
