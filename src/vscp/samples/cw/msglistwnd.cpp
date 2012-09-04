// msglistwnd.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL package (http://www.vscp.org) 
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
// $RCSfile: msglistwnd.cpp,v $                                       
// $Date: 2006/03/27 11:20:39 $                                  
// $Author: akhe $                                              
// $Revision: 1.11 $ 

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
    #include "wx/mdi.h"
#endif

#include "wx/toolbar.h"

#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__) || defined(__WXMAC__)
    #include "mondrian.xpm"
#endif

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/copy.xpm"
#include "bitmaps/cut.xpm"
#include "bitmaps/paste.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/help.xpm"

#include "wx/log.h"
#include "wx/file.h"
#include "wx/sizer.h"
#include "wx/menuitem.h"
#include "wx/checklst.h"
#include "wx/url.h"
#include "wx/html/htmlpars.h"
#include "wx/stream.h"
#include "wx/datetime.h"
#include "wx/grid.h"

#include "cw.h"
#include "msglistwnd.h"


DECLARE_APP( CanalWorksApp )


// Note that MDI_NEW_WINDOW and MDI_ABOUT commands get passed
// to the parent window for processing, so no need to
// duplicate event handlers here.
BEGIN_EVENT_TABLE( CanalChild, wxMDIChildFrame )

  EVT_BUTTON( CtrlButtonClearList, CanalChild::OnButtonClearList )
  EVT_BUTTON( CtrlButtonSendMsg, CanalChild::OnButtonSendMsg )
  EVT_BUTTON( CtrlButtonSendMsgBurst, CanalChild::OnButtonsendMsgBurst )
  EVT_TOGGLEBUTTON( CtrlButtonActivateLog, CanalChild::OnButtonActivateInterface )

  EVT_BUTTON( CtrlButtonPrevMsg, CanalChild::OnButtonPrevMsg )
  EVT_BUTTON( CtrlButtonNextMsg, CanalChild::OnButtonNextMsg )
  EVT_BUTTON( CtrlButtonLoadMsg, CanalChild::OnButtonLoadMsg )
  EVT_BUTTON( CtrlButtonSaveMsg, CanalChild::OnButtonSaveMsg )

  EVT_MENU( MDI_CHILD_QUIT, CanalChild::OnQuit )
  EVT_MENU( MDI_VIEW_SEND_TOOLS, CanalChild::OnViewSendTools )  
  EVT_MENU( MDI_ACTIVATE_INTERFACE, CanalChild::OnButtonMenuActivateInterface )
  EVT_MENU( MDI_CHANGE_TITLE, CanalChild::OnChangeTitle )
  EVT_MENU( MDI_CHANGE_POSITION, CanalChild::OnChangePosition )
  EVT_MENU( MDI_CHANGE_SIZE, CanalChild::OnChangeSize )
  
  EVT_TIMER( RefreshTimer, CanalChild::OnTimerRefresh )
  
  EVT_SIZE( CanalChild::OnSize )
  EVT_MOVE( CanalChild::OnMove )
  
  EVT_CHAR( CanalChild::OnKey )
  
  EVT_CLOSE(CanalChild::OnClose)
  END_EVENT_TABLE()


  // For drawing lines in a canvas
  static long xpos = -1;
static long ypos = -1;

// Globals

extern MyFrame *gframe;
extern wxList my_children;
extern int gs_nFrames;


///////////////////////////////////////////////////////////////////////////////
// CanalChild Constructor
// 

CanalChild::CanalChild( wxMDIParentFrame *parent, 
							            const wxString& title,
							            const wxPoint& pos, 
							            const wxSize& size,
							            const long style,
							            devItem *pdev )
			: wxMDIChildFrame( parent, 
								          CtrlMsgGrid, 
								          title, 
								          pos, 
								          size,
								          style  )
{
  m_pcsw = NULL;
    
  m_bConnected = false; // Not connected

	// Connect to localhost
	devItem device;

	if ( NULL == pdev ) {
		device.id = USE_TCPIP_INTERFACE;	
		device.strName = _("Default host");
		device.strPath = _("TCPIP");
		device.strParameters = ::wxGetApp().m_strUsername;
		device.strParameters += _(";");
		device.strParameters += ::wxGetApp().m_strPassword;
		device.strParameters += _(";");
		device.strParameters += ::wxGetApp().m_strHostname;
		device.strParameters += _(";");
		device.strParameters += wxString::Format( _("%d"), ::wxGetApp().m_port );
		device.flags = 0;
    device.filter = 0;
    device.mask = 0;
	}
	else {
		memcpy( &device, pdev, sizeof( devItem ) );
	}

  // Assign the device parameters
	m_pcsw = new CCanalSuperWrapper( &device );
	if ( NULL == m_pcsw ) {
		wxMessageBox( _("Failed to create driver interface!") );
		return;
	}

	my_children.Append( this );

	m_pgrid = new wxGrid( this,
							            -1,
							            wxPoint( 0, 0 ),
							            wxSize( 700, 300 ) );
#ifdef WIN32
	m_pgridFont = new wxFont( 8, wxDEFAULT, wxNORMAL, wxBOLD );
#else
	m_pgridFont = new wxFont( 6, wxDEFAULT, wxNORMAL, wxBOLD );
#endif

	m_pgrid->CreateGrid( 0, 8 );
	m_pgrid->SetSelectionMode( wxGrid::wxGridSelectRows );
	m_pgrid->SetColLabelAlignment( wxALIGN_LEFT, wxALIGN_BOTTOM );
	m_pgrid->SetColLabelSize( 20 );

	m_pgrid->SetCellBackgroundColour( wxColour( 240, 255, 255 ) );
	m_pgrid->SetGridLineColour( wxColour( 0xc0, 0xc0, 0xc0 ) );

	m_pgrid->SetCellTextColour( *wxBLUE );
	m_pgrid->SetRowLabelSize( 50 );

	m_pgrid->SetRowLabelSize( 50 );

	//m_pgrid->SetDefaultCellFont( *m_pgridFont );

	m_pgrid->SetColSize( 0, 30 );
	m_pgrid->SetColLabelValue( 0, _("Dir") );

	m_pgrid->SetColSize( 1, 125 );
	m_pgrid->SetColLabelValue( 1, _("Time") );
	
	m_pgrid->SetColSize( 2, 80 );
	m_pgrid->SetColLabelValue( 2, _("Timestamp") );
	
	m_pgrid->SetColSize( 3, 80 );
	m_pgrid->SetColLabelValue( 3, _("Flags") );
	
	m_pgrid->SetColSize( 4, 80 );
	m_pgrid->SetColLabelValue( 4, _("Id") );

	m_pgrid->SetColSize( 5, 30 );
	m_pgrid->SetColLabelValue( 5, _("Len") );
	
	m_pgrid->SetColSize( 6, 150 );
	m_pgrid->SetColLabelValue( 6, _("Data") );
	
	m_pgrid->SetColSize( 7, 500 );
	m_pgrid->SetColLabelValue( 7, _("Note") );

	m_ptopSizer = new wxBoxSizer( wxVERTICAL );
	m_ptopSizer->Add( m_pgrid,
						          1,
						          wxEXPAND );

	m_pCmdWnd = new wxWindow( this,
								              CtrlCmdWnd,
								              wxPoint( 0, 0 ),
								              wxSize( 700, 100 ) );

	( void ) new wxStaticText( m_pCmdWnd, 
								              -1, 
								              _( "Select View" ),
#ifdef WIN32								
								wxPoint( 5, 5 ) );
#else
								wxPoint( 5, 50 ) );
#endif	

	wxString wxChoices[ 3 ];
	wxChoices[ 0 ] = _("CAN Msg.List");
	wxChoices[ 1 ] = _("VSCP Msg.List");
	wxChoices[ 2 ] = _("Statistics");
	m_pViewSelectCombo = new wxComboBox( m_pCmdWnd,
										                    CtrlViewSelectCombo,	
										                    _("CAN Msg.List"),
#ifdef WIN32										
										wxPoint( 5, 20 ),
										wxSize( 100, 20 ),
#else
										wxPoint( 5, 70 ),
										wxSize( 130, 20 ),
#endif										
										3,
										wxChoices,
										wxCB_READONLY );

	m_poptionStandardMsgId = new wxRadioButton( m_pCmdWnd,
												                        CtrlOptionStandardMsgId,	
												                        _("Standard"),
												                        wxPoint( 115, 20 ),
												                        wxSize( 80, 20 ) );
	m_poptionStandardMsgId->SetValue( TRUE );

	m_poptionExtendedMsgId = new wxRadioButton( m_pCmdWnd,
												                        CtrlOptionExtendedMsgId,	
												                        _("Extended"),
												                        wxPoint( 115, 35 ),
												                         wxSize( 80, 20 ) );

  m_pRemoteFrame = new wxCheckBox( m_pCmdWnd,
				                            CtrlCheckRemoteFrame,	
				                            _("RTR"),
				                            wxPoint( 200, 40 ),
				                            wxSize( 100, 20 ) );

  ( void ) new wxStaticText( m_pCmdWnd, 
			                        -1, 
			                        _( "Id" ),
			                        wxPoint( 200, 5 ) );

  m_pMsgId = new wxTextCtrl( m_pCmdWnd,
			                        CtrlEditMsgId,	
			                        _("0"),
			                        wxPoint( 200, 20 ),
			                        wxSize( 60, 20 ),
			                        wxTE_RIGHT | wxTE_PROCESS_TAB,
			                        wxTextValidator( wxFILTER_ALPHANUMERIC, &m_wxStrMsgId ) );

	
  ( void ) new wxStaticText( m_pCmdWnd, 
			                        -1, 
			                        _( "Data" ),
			                         wxPoint( 280, 5 ) );
	
  for ( int i=0; i<8; i++ ) {

    m_pMsgData[ i ] = new wxTextCtrl( m_pCmdWnd,
				      CtrlData + i,	
				      _(""),
				      wxPoint( 280 + i*40, 20 ),
				      wxSize( 40, 20 ),
				      wxTE_RIGHT | wxTE_PROCESS_TAB,
				      wxTextValidator( wxFILTER_ALPHANUMERIC, &m_wxStrMsg[ i ] ) );
  }


  m_pPrevMsg = new wxButton( m_pCmdWnd, 
			     CtrlButtonPrevMsg, 
			     _("<"),
#ifdef WIN32
			    wxPoint( 120, 70 ),
			    wxSize( 40, 20 ) );
#else
				wxPoint( 155, 70 ),
			    wxSize( 40, 25 ) );
#endif

  m_pNextMsg = new wxButton( m_pCmdWnd, 
			     CtrlButtonNextMsg, 
			     _(">"),
#ifdef WIN32
			    wxPoint( 162, 70 ),
			    wxSize( 40, 20 ) );
#else
				wxPoint( 197, 70 ),
			    wxSize( 40, 25 ) );
#endif

  m_pLoadMsg = new wxButton( m_pCmdWnd, 
			     CtrlButtonLoadMsg, 
			     _("Load"),
#ifdef WIN32
			     wxPoint( 204, 70 ),
			     wxSize( 40, 20 ) );
#else
				wxPoint( 239, 70 ),
			     wxSize( 40, 25 ) );
#endif

  m_pSaveMsg = new wxButton( m_pCmdWnd, 
			     CtrlButtonSaveMsg, 
			     _("Save"),
#ifdef WIN32
			     wxPoint( 246, 70 ),
			     wxSize( 40, 20 ) );
#else
				wxPoint( 281, 70 ),
			     wxSize( 40, 25 ) );
#endif

  m_pSend = new wxButton( m_pCmdWnd, 
			  CtrlButtonSendMsg, 
			  _("Send"),
#ifdef WIN32			  
			  wxPoint( 610, 20 ),
			  wxSize( 70, 20 ) );
#else
			  wxPoint( 630, 20 ),
			  wxSize( 80, 25 ) );
#endif  


  m_pSendBurst = new wxButton( m_pCmdWnd, 
			       CtrlButtonSendMsgBurst, 
			       _("Send Burst"),
#ifdef WIN32
			       wxPoint( 610, 45 ),
			       wxSize( 70, 20 ) );
#else
				   wxPoint( 630,45 ),
				   wxSize( 80, 25 ) );
#endif


  m_pBurstCnt = new wxTextCtrl( m_pCmdWnd,
				CtrlEditBurstCnt,	
				_("0"),
#ifdef WIN32				
				wxPoint( 610,70 ),
				wxSize( 60, 20 ),
#else
				wxPoint( 630,70 ),
				wxSize( 80, 25 ),
#endif				
				wxTE_RIGHT | wxTE_PROCESS_TAB,
				wxTextValidator( wxFILTER_ALPHANUMERIC, &m_wxStrBurstCnt ) );


  m_pClrList = new wxButton( m_pCmdWnd, 
			     CtrlButtonClearList, 
			     _("Clear"),
#ifdef WIN32				 
			     wxPoint( 450, 70 ),
			     wxSize( 70, 20 ) );
#else
			     wxPoint( 450, 70 ),
			     wxSize( 80, 25 ) );
#endif  


  m_plogActivate = new wxToggleButton( m_pCmdWnd, 
				       CtrlButtonActivateLog, 
				       _("Activate i/f"),
#ifdef WIN32 					   
				        wxPoint( 530, 70 ),
				        wxSize( 70, 20 ) );
#else
					      wxPoint( 530, 70 ),
				        wxSize( 80, 25 ) );
#endif				       

  m_ptopSizer->Add( m_pCmdWnd,
		    0,
		    wxEXPAND );

  SetAutoLayout( TRUE );
  SetSizer( m_ptopSizer );

  // this should work for MDI frames as well as for normal ones
  //SetSizeHints(800, 600);

  m_ptopSizer->Fit( this );
  m_ptopSizer->SetSizeHints( this );

  // Initialize timeevents
  m_timer.SetOwner( this, RefreshTimer );
  m_timer.Start( 100 );   

  wxString s_title, s_add;
  s_add.Printf(_(" - CANAL Session %d"), gs_nFrames );
  if ( NULL == pdev ) {
    // The daemon
    s_title = _("daemon interface");
  }
  else {
      s_title = pdev->strName;
  }

  s_title += s_add;
  SetTitle( s_title );

  SendSizeEvent(); // Needed to calculate the children positions

  // Enable the interface
  m_plogActivate->SetValue( true );
  wxCommandEvent e;
  OnButtonActivateInterface( e );
}

///////////////////////////////////////////////////////////////////////////////
// CanalChild Destructor
//

CanalChild::~CanalChild()
{
  if ( m_plogActivate->GetValue() ) {
    m_plogActivate->SetValue( false );
  }

  my_children.DeleteObject( this );
	

  if ( NULL != m_pgridFont ) delete m_pgridFont;

  // Delete CanalSuperWrapper object
  if ( NULL != m_pcsw ) delete m_pcsw;
  m_pcsw = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// OnQuit
//

void CanalChild::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{	
  // Delete CanalSuperWrapper object
  if ( NULL != m_pcsw ) delete m_pcsw;
  m_pcsw = NULL;
	
  Close( TRUE );
}

///////////////////////////////////////////////////////////////////////////////
// OnClose
//

void CanalChild::OnClose( wxCloseEvent& event )
{
  /*
    if ( wxMessageBox( _T("Really close?"), 
    _T("Please confirm"),
    wxICON_QUESTION | wxYES_NO) != wxYES ) {
    event.Veto();
    return;
    }
  */
  gs_nFrames--;
  event.Skip();

  // Delete CanalSuperWrapper object
  if ( NULL != m_pcsw ) delete m_pcsw;
  m_pcsw = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// OnRefresh
//

void CanalChild::OnViewSendTools( wxCommandEvent& WXUNUSED( event ) )
{
  if ( gframe->m_poption_menu->IsChecked( MDI_VIEW_SEND_TOOLS ) ) {

    m_pCmdWnd->Show( true );
    m_ptopSizer->Add( m_pCmdWnd, 0, wxEXPAND );
    m_ptopSizer->RecalcSizes();
    m_ptopSizer->Layout();

  }
  else {

    m_ptopSizer->Remove( m_pCmdWnd );
    m_ptopSizer->RecalcSizes();
    m_pCmdWnd->Show( false );  
    m_ptopSizer->Layout();
		
  }
}

///////////////////////////////////////////////////////////////////////////////
// OnChangePosition
//

void CanalChild::OnChangePosition( wxCommandEvent& WXUNUSED( event ) )
{
  Move(10, 10);
}

///////////////////////////////////////////////////////////////////////////////
// OnChangeSize
//

void CanalChild::OnChangeSize( wxCommandEvent& WXUNUSED( event ) )
{
  SetClientSize(100, 100);
	
}

///////////////////////////////////////////////////////////////////////////////
// OnChangeTitle
//

void CanalChild::OnChangeTitle( wxCommandEvent& WXUNUSED( event ) )
{
  static wxString s_title = _("VSCP Project");

  wxString title = wxGetTextFromUser( _("Enter the new title for MDI child"),
				                                _("MDI sample question"),
				                                s_title,
				                                GetParent()->GetParent() );
  if ( !title ) {
    return;
  }

  s_title = title;
  SetTitle(s_title);

}

///////////////////////////////////////////////////////////////////////////////
// OnActivate
//

void CanalChild::OnActivate( wxActivateEvent& event )
{
    
  //if ( event.GetActive() && canvas ) {
  //    canvas->SetFocus();
  //}
}

///////////////////////////////////////////////////////////////////////////////
// OnMove
//

void CanalChild::OnMove( wxMoveEvent& event )
{
  // VZ: here everything is totally wrong under MSW, the positions are
  //     different and both wrong (pos2 is off by 2 pixels for me which seems
  //     to be the width of the MDI canvas border)
  wxPoint pos1 = event.GetPosition(),
  pos2 = GetPosition();

  event.Skip();
}

///////////////////////////////////////////////////////////////////////////////
// OnSize
//

void CanalChild::OnSize( wxSizeEvent& event )
{
  // VZ: under MSW the size event carries the client size (quite
  //     unexpectedly) *except* for the very first one which has the full
  //     size... what should it really be? TODO: check under wxGTK
  wxSize size1 = event.GetSize(),
    size2 = GetSize(),
    size3 = GetClientSize();
  /*
  wxLogStatus( wxT( "size from event: %dx%d, from frame %dx%d, client %dx%d"),
	       size1.x, 
	       size1.y, 
	       size2.x, 
	       size2.y, 
	       size3.x, 
	       size3.y);
  */
  event.Skip();

}


///////////////////////////////////////////////////////////////////////////////
// OnKey
//

void CanalChild::OnKey( wxKeyEvent& event )
{

  event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonClearList
//

void CanalChild::OnButtonClearList( wxCommandEvent& event )
{
	if ( m_pgrid->GetNumberRows() <= 0 ){
		wxMessageBox( _T("The list is empty. Nothing to delete.") );
		return;
	}

	if ( ::wxGetApp().m_bDisplayDeleteWarning ) {
		if ( wxMessageBox( _T("Are you sure that the list should be cleared?"), 
							          _T("Please confirm"),
							          wxICON_QUESTION | wxYES_NO ) == wxNO ) {
			return;
		}
	}

	::wxBeginBusyCursor();
        
    if ( m_plogActivate->GetValue() ) {
			
      m_plogActivate->SetValue( false );
      m_pgrid->ClearGrid();
      m_pgrid->DeleteRows( 0, m_pgrid->GetNumberRows() );
      m_plogActivate->SetValue( true );

    }
    else {
		
      m_pgrid->ClearGrid();
      m_pgrid->DeleteRows( 0, m_pgrid->GetNumberRows() );
		
    }


  ::wxEndBusyCursor();

}

///////////////////////////////////////////////////////////////////////////////
// CtrlButtonActivateInterface
//

void CanalChild::OnButtonActivateInterface( wxCommandEvent& event )
{
	::wxBeginBusyCursor();

	// Is it activated
	if ( m_plogActivate->GetValue()  ) {
    
		// Open i/f
		if ( 0 != m_pcsw->doCmdOpen() ) { 			
			
			if ( USE_DLL_INTERFACE == m_pcsw->getDeviceType() ) {
				
				// We use a dll.
				//
				// Verify that we have a connection with the interface
				// We do that by using the status command.
				canalStatus canalstatus;
				if ( CANAL_ERROR_SUCCESS != m_pcsw->doCmdStatus( &canalstatus ) ) {
					wxString strbuf;
					strbuf.Printf(
								_("Unable to open interface to driver. errorcode=%lu, suberrorcode=%lu, Description: %s"),
								canalstatus.lasterrorcode,
								canalstatus.lasterrorsubcode,
								canalstatus.lasterrorstr );
					wxLogStatus( strbuf );

					wxMessageBox( _("No response received from interface. May not work correctly!") );
	       
				}
									
			}
			else if ( USE_TCPIP_INTERFACE == m_pcsw->getDeviceType() ) {
				// TCP/IP interface
				
			}

      m_bConnected = true;

		}
		else {

			m_plogActivate->SetValue( false );
      m_bConnected = false;

			switch ( m_pcsw->getDeviceType() ) {
			
				case USE_DLL_INTERFACE:
					wxMessageBox( _("Failed to activate interface. Is hardware connected? Functional?") );
					break;
					
				case USE_TCPIP_INTERFACE:
					wxMessageBox( _("Failed to activate interface. Is VSCP Server started? Functional?") );
					break;

				default:
					wxMessageBox( _("Failed to acvtivate unknown interface. Possible internal program error?") );	
					break;
			}

		}

		wxLogStatus( _("Interface open") );
    
	}
	else {
    
		// Close i/f
		m_pcsw->doCmdClose();
    m_bConnected = false;
    
	}

  
	::wxEndBusyCursor();
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonMenuActivateInterface
//

void CanalChild::OnButtonMenuActivateInterface( wxCommandEvent& event )
{
	::wxBeginBusyCursor();
	if ( m_plogActivate->GetValue() ) {
		m_plogActivate->SetValue( false );	
	}
	else {
		m_plogActivate->SetValue( true );
	}
	::wxEndBusyCursor();
}

///////////////////////////////////////////////////////////////////////////////
// OnButtonPrevMsg
//

void CanalChild::OnButtonPrevMsg( wxCommandEvent& event )
{
	wxMessageBox( _("This function has not yet been implemented.") );
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonNextMsg
//

void CanalChild::OnButtonNextMsg( wxCommandEvent& event )
{
	wxMessageBox( _("This function has not yet been implemented.") ); 
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonLoadMsg
//

void CanalChild::OnButtonLoadMsg( wxCommandEvent& event )
{
	wxMessageBox( _("This function has not yet been implemented."));
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonSaveMsg
//

void CanalChild::OnButtonSaveMsg( wxCommandEvent& event )
{
	wxMessageBox(_("This function has not yet been implemented."));
}


///////////////////////////////////////////////////////////////////////////////
//  OnTimerRefresh
//

void CanalChild::OnTimerRefresh( wxTimerEvent& event ) 
{
	if ( m_bConnected && m_plogActivate->GetValue()  ) {
		fillData();
	}
}



///////////////////////////////////////////////////////////////////////////////
//  fillData
//

void CanalChild::fillData( void )
{
	int cnt;
	canalMsg Msg;

	if ( ( cnt = m_pcsw->doCmdDataAvailable() ) > 0 ) {
		
		// We don't want to lock up the UI
		if ( cnt > 10 ) cnt = 10;

		for ( int i=0; i<cnt; i++ ) {
			if ( CANAL_ERROR_SUCCESS == m_pcsw->doCmdReceive( &Msg ) ) {
				insertItem( &Msg );
			}
			else {
				// Failed to read event

			}
		}
	}
}



///////////////////////////////////////////////////////////////////////////////
//  insertItem
//

void CanalChild::insertItem( canalMsg *pMsg, bool bReceive )
{
	wxString strbuf;
	wxString strsmallbuf;
  
  
	// Add a row
	if ( m_pgrid->AppendRows( 1 ) ) {
    
		// Direction
		if ( bReceive ) {
			strbuf = _(" RX ");
		}
		else {
			strbuf = _(" TX ");	
		}
    
		m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strbuf );
    
		wxDateTime now = wxDateTime::Now();
		m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 
								1,  
								now.Format(_("%Y-%m-%d %H:%M:%S")) );
    
		// Timestamp
		strbuf.Printf( _("%08lX"), pMsg->timestamp );
		m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  strbuf );
    
		// Flags
		strbuf.Printf( _("%08lX"), pMsg->flags );
		m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  strbuf );
    
		// Id
		strbuf.Printf( _("%08lX"), pMsg->id );
		m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 4,  strbuf );
    
		// Len
		strbuf.Printf( _("%X"), pMsg->sizeData );
		m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 5,  strbuf );
    
		// Data
		strbuf.Empty();
		if ( pMsg->flags & CANAL_IDFLAG_RTR ) {
      
			strbuf.Printf( _("Remote Frame") );
      
		}
		else {
      
			for ( int i=0; i < 8; i++ ) {
				
				if ( i < pMsg->sizeData ) {
					strsmallbuf.Printf( _("%02X "), pMsg->data[ i ] ); 
					strbuf += strsmallbuf;
				}
			}
      
		}
    
		m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 6,  strbuf );
    
		m_pgrid->SetCellFont( m_pgrid->GetNumberRows()-1, 7, *wxSWISS_FONT );
    
		if ( !bReceive ) {
			for ( int i = 0; i < 8; i++ ) {
				m_pgrid->SetCellTextColour( m_pgrid->GetNumberRows()-1, 
											                i, 
											                wxColour( 0x80, 0x80, 0x80 ) );
			}
		}
    
		// Make sure the row is visible
		m_pgrid->MakeCellVisible( m_pgrid->GetNumberRows()-1, 0 );
    
	}
	else {
		wxMessageBox( _("Failed to add row!") );
	}
  
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonSendMsg
//

void CanalChild::OnButtonSendMsg( wxCommandEvent& event )
{
	OnSendMsg();
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonsendMsgBurst
//

void CanalChild::OnButtonsendMsgBurst( wxCommandEvent& event )
{
  unsigned long cnt;
	wxString wxstr;
	canalMsg Msg;


	if ( getCanData( &Msg ) ) {

		::wxBeginBusyCursor();

		// Get ID	
		wxstr = m_pBurstCnt->GetValue();
		cnt = readStringValue( wxstr );
		for ( unsigned long i = 0; i<cnt; i++ ) {
			
			if ( CANAL_ERROR_SUCCESS!= m_pcsw->doCmdSend( &Msg ) ) {
				wxMessageBox( _("Failed to send burst message") );
				break;
			}
			else {
				// Insert in list
				insertItem( &Msg, false );
			}
			
		}

		::wxEndBusyCursor();

	}
	else {
		wxMessageBox( _("Unable to read data for send burst message") );
	}
 
}


//////////////////////////////////////////////////////////////////////////////
// OnSendMsg
//


void CanalChild::OnSendMsg()
{
	canalMsg Msg;
	if ( getCanData( &Msg ) ) {
		if ( CANAL_ERROR_SUCCESS != m_pcsw->doCmdSend( &Msg ) ) {
			wxMessageBox( _("Failed to send message") );
		}
		else {
			// Insert in list
			insertItem( &Msg, false );
		}

	}
	else {
		wxMessageBox( _("Unable to read data for send message") );
	}
}


//////////////////////////////////////////////////////////////////////////////
// getCANData
//

bool CanalChild::getCanData( canalMsg *pMsg )
{
	int i;
	unsigned char nData = 0;

	// Must have somewhere to store data
	if ( NULL == pMsg ) return false;

	// Initialize message structure
	pMsg->flags = 0;
	pMsg->id = 0;
	pMsg->obid = 0;
	pMsg->sizeData = 0;
	pMsg->timestamp = 0;

	for ( i=0; i<8; i++ ) {
		pMsg->data[ i ] = 0;
	}

	wxString wxstr;
 	
	// Get ID	
	wxstr = m_pMsgId->GetValue();
	pMsg->id = readStringValue( wxstr );

	
	
	// Get Data
	for ( i=7; i>=0; i-- ) {
		wxstr = m_pMsgData[ i ]->GetValue();
		pMsg->data[ i ] = readStringValue( wxstr );
		if ( ( 0 == nData ) && ( wxstr.Len() > 0 ) ) nData = i+1;
	}

	// Set Data size
	pMsg->sizeData = nData;

	// Check for Extended message
	if ( m_poptionExtendedMsgId->GetValue() ) {
		pMsg->flags |= CANAL_IDFLAG_EXTENDED;
		
		// Fix ID 
		if ( pMsg->id > CAN_MAX_EXTENDED_ID ) {
			pMsg->id = CAN_MAX_EXTENDED_ID;	
		}
	}
	else {
		// Fix ID 
		if ( pMsg->id > CAN_MAX_STANDARD_ID ) {
			pMsg->id  = CAN_MAX_STANDARD_ID;
		}
	}

	// Check for Remote Frame
	if ( m_pRemoteFrame->GetValue() ) {
		pMsg->flags |= CANAL_IDFLAG_RTR;
	}
	
	// Write back data to dialog
	writeDataToDialog( pMsg );
	
	return true;
}	

//////////////////////////////////////////////////////////////////////////////
// writeDataToDialog
//

bool CanalChild::writeDataToDialog( canalMsg *pMsg )
{
	bool rv = true;

	return rv;
}
