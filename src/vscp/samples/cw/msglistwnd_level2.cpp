// msglistwnd_level2.cpp
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
// $RCSfile: msglistwnd_level2.cpp,v $
// $Date: 2005/09/26 21:57:51 $
// $Author: akhe $
// $Revision: 1.7 $

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
#include "wx/spinctrl.h"
#include "wx/tokenzr.h"

#include "cw.h"
#include "../../common/vscp.h"
#include "../../common/vscphelper.h"
#include "../../common/vscptcpif.h"
#include "../../common/canalsuperwrapper.h"
#include "msglistwnd_level2.h"

DECLARE_APP( CanalWorksApp )

// Note that MDI_NEW_WINDOW and MDI_ABOUT commands get passed
// to the parent window for processing, so no need to
// duplicate event handlers here.
BEGIN_EVENT_TABLE( Level2Child, wxMDIChildFrame )

EVT_BUTTON( L2CtrlButtonClearList, Level2Child::OnButtonClearList )
EVT_BUTTON( L2CtrlButtonSendMsg, Level2Child::OnButtonSendMsg )
EVT_TOGGLEBUTTON( L2CtrlButtonActivateLog, Level2Child::OnButtonActivateInterface )

EVT_BUTTON( L2CtrlButtonPrevMsg, Level2Child::OnButtonPrevMsg )
EVT_BUTTON( L2CtrlButtonNextMsg, Level2Child::OnButtonNextMsg )
EVT_BUTTON( L2CtrlButtonLoadMsg, Level2Child::OnButtonLoadMsg )
EVT_BUTTON( L2CtrlButtonSaveMsg, Level2Child::OnButtonSaveMsg )

EVT_MENU( MDI_CHILD_QUIT, Level2Child::OnQuit )
EVT_MENU( MDI_VIEW_SEND_TOOLS, Level2Child::OnViewSendTools )
EVT_MENU( MDI_ACTIVATE_INTERFACE, Level2Child::OnButtonMenuActivateInterface )
EVT_MENU( MDI_CHANGE_TITLE, Level2Child::OnChangeTitle )
EVT_MENU( MDI_CHANGE_POSITION, Level2Child::OnChangePosition )
EVT_MENU( MDI_CHANGE_SIZE, Level2Child::OnChangeSize )

EVT_TIMER( L2RefreshTimer, Level2Child::OnTimerRefresh )

EVT_SIZE( Level2Child::OnSize )
EVT_MOVE( Level2Child::OnMove )

EVT_CHAR( Level2Child::OnKey )

EVT_CLOSE(Level2Child::OnClose)
END_EVENT_TABLE()


// For drawing lines in a canvas
static long xpos = -1;
static long ypos = -1;

// Globals

extern MyFrame *gframe;
extern wxList my_children;
extern int gs_nFrames;


///////////////////////////////////////////////////////////////////////////////
// Level2Child Constructor
//

Level2Child::Level2Child( wxMDIParentFrame *parent,
                          const wxString& title,
                          const wxPoint& pos,
                          const wxSize& size,
                          const long style,
                          devItem *pdev )
        : wxMDIChildFrame( parent,
                           L2CtrlMsgGrid,
                           title,
                           pos,
                           size,
                   style  ) {

	int i;

  m_bConnected = false;

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

    m_pgridFont = new wxFont( 8, wxDEFAULT, wxNORMAL, wxBOLD );

    m_pgrid->CreateGrid( 0, 9 );
    m_pgrid->SetSelectionMode( wxGrid::wxGridSelectRows );
    m_pgrid->SetColLabelAlignment( wxALIGN_LEFT, wxALIGN_BOTTOM );
    m_pgrid->SetColLabelSize( 20 );

    m_pgrid->SetCellBackgroundColour( wxColour( 255, 255, 230 ) );
    m_pgrid->SetGridLineColour( wxColour( 0xc0, 0xc0, 0xc0 ) );

    m_pgrid->SetCellTextColour( *wxBLACK );
    m_pgrid->SetRowLabelSize( 50 );

    m_pgrid->SetRowLabelSize( 50 );

    //m_pgrid->SetDefaultCellFont( *m_pgridFont );

    m_pgrid->SetColSize( 0, 30 );
    m_pgrid->SetColLabelValue( 0, _("Dir") );

    m_pgrid->SetColSize( 1, 110 );
    m_pgrid->SetColLabelValue( 1, _("Time") );

    m_pgrid->SetColSize( 2, 250 );
    m_pgrid->SetColLabelValue( 2, _("GUID") );

    m_pgrid->SetColSize( 3, 60 );
    m_pgrid->SetColLabelValue( 3, _("Class") );

    m_pgrid->SetColSize( 4, 60 );
    m_pgrid->SetColLabelValue( 4, _("Type") );

    m_pgrid->SetColSize( 5, 40 );
    m_pgrid->SetColLabelValue( 5, _("Head") );

    m_pgrid->SetColSize( 6, 40 );
    m_pgrid->SetColLabelValue( 6, _("Cnt") );

    m_pgrid->SetColSize( 7, 300 );
    m_pgrid->SetColLabelValue( 7, _("Data") );

    m_pgrid->SetColSize( 8, 80 );
    m_pgrid->SetColLabelValue( 8, _("Timestamp") );





    m_ptopSizer = new wxBoxSizer( wxVERTICAL );
    m_ptopSizer->Add( m_pgrid, 1, wxEXPAND );

    m_pCmdWnd = new wxWindow( this,
                              L2CtrlCmdWnd,
                              wxPoint( 0, 0 ),
                              wxSize( 700, 100 ) );


    ( void ) new wxStaticText( m_pCmdWnd,
                               -1,
                               _( "Class" ),
                               wxPoint( 130, 20 ) );

    m_pMsgClass = new wxTextCtrl( m_pCmdWnd,
                                  L2CtrlEditMsgClass,
                                  _("0"),
                                  wxPoint( 170, 20 ),
                                  wxSize( 60, 20 ),
                                  wxTE_RIGHT | wxTE_PROCESS_TAB,
                                  wxTextValidator( wxFILTER_ALPHANUMERIC, &m_wxStrMsgClass ) );

    ( void ) new wxStaticText( m_pCmdWnd,
                               -1,
                               _( "Type" ),
                               wxPoint( 130, 40 ) );

    m_pMsgType = new wxTextCtrl( m_pCmdWnd,
                                 L2CtrlEditMsgType,
                                 _("0"),
                                 wxPoint( 170, 40 ),
                                 wxSize( 60, 20 ),
                                 wxTE_RIGHT | wxTE_PROCESS_TAB,
                                 wxTextValidator( wxFILTER_ALPHANUMERIC, &m_wxStrMsgClass ) );


    ( void ) new wxStaticText( m_pCmdWnd,
                               -1,
                               _( "GUID" ),
                               wxPoint( 280, 5 ) );

    ( void ) new wxStaticText( m_pCmdWnd,
                               -1,
                               _( "15 - 8" ),
                               wxPoint( 243, 23 ),
                               wxSize( 30, 20 ),
                               wxALIGN_RIGHT );

    ( void ) new wxStaticText( m_pCmdWnd,
                               -1,
                               _( "7 - 0" ),
                               wxPoint( 243, 43 ),
                               wxSize( 30, 20 ),
                               wxALIGN_RIGHT );

    for ( i=0; i<8; i++ ) {

        m_pEditGUID[ i ] = new wxTextCtrl( m_pCmdWnd,
                                           L2EditGUID + i,
                                           _(""),
                                           wxPoint( 280 + i*40, 20 ),
                                           wxSize( 40, 20 ),
                                           wxTE_RIGHT | wxTE_PROCESS_TAB,
                                           wxTextValidator( wxFILTER_ALPHANUMERIC, &m_wxStrGUID[ i ] ) );
    }

    for ( i=8; i<16; i++ ) {

        m_pEditGUID[ i ] = new wxTextCtrl( m_pCmdWnd,
                                           L2EditGUID + i,
                                           _(""),
                                           wxPoint( 280 + (i-8 )*40, 40 ),
                                           wxSize( 40, 20 ),
                                           wxTE_RIGHT | wxTE_PROCESS_TAB,
                                           wxTextValidator( wxFILTER_ALPHANUMERIC, &m_wxStrGUID[ i ] ) );
    }

    ( void ) new wxStaticText( m_pCmdWnd,
                               -1,
                               _( "Data" ),
                               wxPoint( 243, 62 ),
                               wxSize( 30, 20 ),
                               wxALIGN_RIGHT );

    m_pMsgData = new wxTextCtrl( m_pCmdWnd,
                                 L2CtrlEditMsgData,
                                 wxT("0"),
                                 wxPoint( 280, 62 ),
                                 wxSize( 320, 35 ),
                                 wxTE_MULTILINE,
                                 wxTextValidator( wxFILTER_ASCII, &m_wxStrMsgClass ) );

    m_pPrevMsg = new wxButton( m_pCmdWnd,
                               L2CtrlButtonPrevMsg,
                               _("<"),
                               wxPoint( 10, 5 ),
                               wxSize( 40, 20 ) );

    m_pNextMsg = new wxButton( m_pCmdWnd,
                               L2CtrlButtonNextMsg,
                               _(">"),
                               wxPoint( 52, 5 ),
                               wxSize( 40, 20 ) );

    m_pLoadMsg = new wxButton( m_pCmdWnd,
                               L2CtrlButtonLoadMsg,
                               _("Load"),
                               wxPoint( 10, 27 ),
                               wxSize( 40, 20 ) );

    m_pSaveMsg = new wxButton( m_pCmdWnd,
                               L2CtrlButtonSaveMsg,
                               _("Save"),
                               wxPoint( 52, 27 ),
                               wxSize( 40, 20 ) );



    m_pSend = new wxButton( m_pCmdWnd,
                            L2CtrlButtonSendMsg,
                            _("Send"),
                            wxPoint( 610, 20 ),
                            wxSize( 70, 20 ) );

    m_plogActivate = new wxToggleButton( m_pCmdWnd,
                                         L2CtrlButtonActivateLog,
                                         _("Activate i/f"),
                                         wxPoint( 610, 50 ),
                                         wxSize( 70, 20 ) );

    m_pClrList = new wxButton( m_pCmdWnd,
                               L2CtrlButtonClearList,
                               _("Clear"),
                               wxPoint( 610, 72 ),
                               wxSize( 70, 20 ) );


    m_pSpinPriority = new wxSpinCtrl( m_pCmdWnd,
                                      L2CtrlSpinPriority,
                                      _("3"),
                                      wxPoint( 180, 72 ),
                                      wxSize( 50, 20 ),
                                      wxSP_ARROW_KEYS | wxSP_WRAP );
    m_pSpinPriority->SetRange( 0, 7 );

    ( void ) new wxStaticText( m_pCmdWnd,
                               -1,
                               _( "Priority" ),
                               wxPoint( 120, 72 ),
                               wxSize( 50, 30 ),
                               wxALIGN_RIGHT );

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
    m_timer.SetOwner( this, L2RefreshTimer );
    m_timer.Start( 100 );

    wxString s_title, s_add;
    s_add.Printf(_(" - VSCP client Session %d"), gs_nFrames );
    if ( NULL == pdev ) {
        // The daemon
        s_title = _("daemon interface");
    } else {
      s_title = pdev->strName;
    }
    s_title += s_add;
    SetTitle( s_title );

    SendSizeEvent(); // Needed to calculate the children positions
}

///////////////////////////////////////////////////////////////////////////////
// Level2Child Destructor
//

Level2Child::~Level2Child() {
    if ( m_plogActivate->GetValue() ) {
        m_plogActivate->SetValue( false );
    }

    my_children.DeleteObject( this );


    if ( NULL != m_pgridFont )
        delete m_pgridFont;

}

///////////////////////////////////////////////////////////////////////////////
// OnQuit
//

void Level2Child::OnQuit( wxCommandEvent& WXUNUSED( event ) ) {
    // Delete CanalSuperWrapper object
  if ( NULL != m_pcsw ) delete m_pcsw;
  m_pcsw = NULL;
	
  Close( TRUE );
}

///////////////////////////////////////////////////////////////////////////////
// OnClose
//

void Level2Child::OnClose( wxCloseEvent& event ) {
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

}

///////////////////////////////////////////////////////////////////////////////
// OnRefresh
//

void Level2Child::OnViewSendTools( wxCommandEvent& WXUNUSED( event ) ) {
    if ( gframe->m_poption_menu->IsChecked( MDI_VIEW_SEND_TOOLS ) ) {

        m_pCmdWnd->Show( true );
        m_ptopSizer->Add( m_pCmdWnd, 0, wxEXPAND );
        m_ptopSizer->RecalcSizes();
        m_ptopSizer->Layout();

    } else {

        m_ptopSizer->Remove( m_pCmdWnd );
        m_ptopSizer->RecalcSizes();
        m_pCmdWnd->Show( false );
        m_ptopSizer->Layout();

    }
}

///////////////////////////////////////////////////////////////////////////////
// OnChangePosition
//

void Level2Child::OnChangePosition( wxCommandEvent& WXUNUSED( event ) ) {
    Move(10, 10);
}

///////////////////////////////////////////////////////////////////////////////
// OnChangeSize
//

void Level2Child::OnChangeSize( wxCommandEvent& WXUNUSED( event ) ) {
    SetClientSize(100, 100);

}

///////////////////////////////////////////////////////////////////////////////
// OnChangeTitle
//

void Level2Child::OnChangeTitle( wxCommandEvent& WXUNUSED( event ) ) {
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

void Level2Child::OnActivate( wxActivateEvent& event ) {

    //if ( event.GetActive() && canvas ) {
    //    canvas->SetFocus();
    //}
}

///////////////////////////////////////////////////////////////////////////////
// OnMove
//

void Level2Child::OnMove( wxMoveEvent& event ) {
    // VZ: here everything is totally wrong under MSW, the positions are
    //     different and both wrong (pos2 is off by 2 pixels for me which seems
    //     to be the width of the MDI canvas border)
    wxPoint pos1 = event.GetPosition(),
                   pos2 = GetPosition();
    wxLogStatus( _("position from event: (%d, %d), from frame (%d, %d)"),
                  pos1.x, pos1.y, pos2.x, pos2.y);
    event.Skip();
}

///////////////////////////////////////////////////////////////////////////////
// OnSize
//

void Level2Child::OnSize( wxSizeEvent& event ) {
    // VZ: under MSW the size event carries the client size (quite
    //     unexpectedly) *except* for the very first one which has the full
    //     size... what should it really be? TODO: check under wxGTK
    wxSize size1 = event.GetSize(),
                   size2 = GetSize(),
                           size3 = GetClientSize();
    wxLogStatus( _( "size from event: %dx%d, from frame %dx%d, client %dx%d"),
                  size1.x,
                  size1.y,
                  size2.x,
                  size2.y,
                  size3.x,
                  size3.y);

    event.Skip();

}


///////////////////////////////////////////////////////////////////////////////
// OnKey
//

void Level2Child::OnKey( wxKeyEvent& event ) {

    event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonClearList
//

void Level2Child::OnButtonClearList( wxCommandEvent& event ) {

    if ( m_pgrid->GetNumberRows() <= 0 ) {
        wxMessageBox( _("The list is empty. Nothing to delete.") );
        return;
    }

    if ( ::wxGetApp().m_bDisplayDeleteWarning ) {
		if ( wxMessageBox( _("Are you sure that the list should be cleared?"), 
			    _("Please confirm"),
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

	} else {

		m_pgrid->ClearGrid();
        m_pgrid->DeleteRows( 0, m_pgrid->GetNumberRows() );

    }

    ::wxEndBusyCursor();

}

///////////////////////////////////////////////////////////////////////////////
// CtrlButtonActivateInterface
//

void Level2Child::OnButtonActivateInterface( wxCommandEvent& event ) {

    ::wxBeginBusyCursor();

    // Is it activated
    if ( m_plogActivate->GetValue() ) {

      if ( 0 != m_pcsw->doCmdOpen() ) { 				
        m_bConnected = true;      
      }
      else {
        wxMessageBox( _("Failed to open Level II interface.") );
        m_plogActivate->SetValue( false );
        
      }
      

    } else {

      // Close i/f
		  m_pcsw->doCmdClose();
      m_bConnected = false;

    }

    ::wxEndBusyCursor();
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonMenuActivateInterface
//

void Level2Child::OnButtonMenuActivateInterface( wxCommandEvent& event ) {
    ::wxBeginBusyCursor();
    if ( m_plogActivate->GetValue() ) {
        m_plogActivate->SetValue( false );
    } else {
        m_plogActivate->SetValue( true );
    }
    ::wxEndBusyCursor();
}

///////////////////////////////////////////////////////////////////////////////
// OnButtonPrevMsg
//

void Level2Child::OnButtonPrevMsg( wxCommandEvent& event ) {
    wxMessageBox(_("This function has not yet been implemented."));
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonNextMsg
//

void Level2Child::OnButtonNextMsg( wxCommandEvent& event ) {
    wxMessageBox(_("This function has not yet been implemented."));
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonLoadMsg
//

void Level2Child::OnButtonLoadMsg( wxCommandEvent& event ) {
    wxMessageBox(_("This function has not yet been implemented."));
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonSaveMsg
//

void Level2Child::OnButtonSaveMsg( wxCommandEvent& event ) {
    wxMessageBox(_("This function has not yet been implemented."));
}


///////////////////////////////////////////////////////////////////////////////
//  OnTimerRefresh
//

void Level2Child::OnTimerRefresh( wxTimerEvent& event ) 
{
 if ( m_bConnected && m_plogActivate->GetValue()  ) {
		fillData();
	}
}



///////////////////////////////////////////////////////////////////////////////
//  fillData
//

void Level2Child::fillData( void ) {
    int cnt;
    vscpEvent event;

    if ( ( cnt = m_pcsw->doCmdDataAvailable() ) ) {
        for ( int i=0; i<cnt; i++ ) {
            if ( CANAL_ERROR_SUCCESS == m_pcsw->doCmdReceive( &event ) ) {
                insertItem( &event );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  insertItem
//

void Level2Child::insertItem( vscpEvent *pEvent, bool bReceive ) {
    wxString strbuf;
    wxString strsmallbuf;


    // Add a row
    if ( m_pgrid->AppendRows( 1 ) ) {

        // Direction
        if ( bReceive ) {
            strbuf = _(" RX ");
        } else {
            strbuf = _(" TX ");
        }

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strbuf );

        wxDateTime now = wxDateTime::Now();
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1,
                               1,
                               now.Format(_("%Y-%m-%d %H:%M:%S")) );

        // Timestamp
        strbuf.Printf( _("%08lX"), pEvent->timestamp );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 8,  strbuf );

        // GUID
        strbuf.Printf( 
                 _("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X"),
                 pEvent->GUID[ 0 ], pEvent->GUID[ 1 ], pEvent->GUID[ 2 ], pEvent->GUID[ 3 ],
                 pEvent->GUID[ 4 ], pEvent->GUID[ 5 ], pEvent->GUID[ 6 ], pEvent->GUID[ 7 ],
                 pEvent->GUID[ 8 ], pEvent->GUID[ 9 ], pEvent->GUID[ 10 ], pEvent->GUID[ 11 ],
                 pEvent->GUID[ 12 ], pEvent->GUID[ 13 ], pEvent->GUID[ 14 ], pEvent->GUID[ 15 ]
               );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  strbuf );

        // Class
        strbuf.Printf( _("%04lX"), pEvent->vscp_class );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  strbuf );

        // Type
        strbuf.Printf( _("%04lX"), pEvent->vscp_type );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 4,  strbuf );

        // Head
        strbuf.Printf( _("%X"), pEvent->head );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 5,  strbuf );

        // sizeData
        strbuf.Printf( _("%X"), pEvent->sizeData );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 6,  strbuf );

        // Data
        strbuf.Empty();
        if ( NULL != pEvent->pdata ) {
            for ( uint16_t i=0; i < pEvent->sizeData; i++ ) {
                strsmallbuf.Printf( _("%02X "), pEvent->pdata[ i ] );
                strbuf = strsmallbuf;
                if ( !( (i+1) % 32 ) )
                    strbuf = _("\r\n");
            }
        }

        // Delete allocated data
        if ( NULL != pEvent->pdata )
            delete pEvent->pdata;

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 7,  strbuf );
        m_pgrid->SetCellFont( m_pgrid->GetNumberRows()-1, 7, *wxSWISS_FONT );

        if ( !bReceive ) {
            for ( int i = 0; i < 9; i++ ) {
                m_pgrid->SetCellTextColour( m_pgrid->GetNumberRows()-1,
                                            i,
                                            wxColour( 0x80, 0x80, 0x80 ) );
            }
        }

        // Make sure the row is visible
        m_pgrid->MakeCellVisible( m_pgrid->GetNumberRows()-1, 0 );

    } else {
        wxMessageBox( _("Failed to add row!") );
    }

}


///////////////////////////////////////////////////////////////////////////////
// OnButtonSendMsg
//

void Level2Child::OnButtonSendMsg( wxCommandEvent& event ) {
    OnSendMsg();
}


//////////////////////////////////////////////////////////////////////////////
// OnSendMsg
//


void Level2Child::OnSendMsg() {
    vscpEvent event;
    if ( getVSCPData( &event ) ) {
        if ( CANAL_ERROR_SUCCESS != m_pcsw->doCmdSend( &event ) ) {
            wxMessageBox( _("Failed to send VSCP event") );
        } else {
            // Insert in list
            insertItem( &event, false );
        }

    } else {
        wxMessageBox( _("Unable to read data for send message") );
    }

}


//////////////////////////////////////////////////////////////////////////////
// getVSCPData
//

bool Level2Child::getVSCPData( vscpEvent *pEvent ) {
    int i;
    uint8_t nData = 0;
    uint8_t msgdata[ 512 - 25 ];

    // Must have somewhere to store data
    if ( NULL == pEvent )
        return false;

    // Initialize message structure
    pEvent->head = 0;
    pEvent->vscp_class = 0;
    pEvent->vscp_type = 0;
    pEvent->sizeData = 0;
    pEvent->pdata = NULL;
    pEvent->timestamp = 0;

    wxString wxstr;

    // Set Head
    //		priority
    pEvent->head = ( m_pSpinPriority->GetValue()<< 5 );

    // Get Class
    wxstr = m_pMsgClass->GetValue();
    pEvent->vscp_class = readStringValue( wxstr );

    // Get Type
    wxstr = m_pMsgType->GetValue();
    pEvent->vscp_type= readStringValue( wxstr );


    // Get GUID
    for ( i=0; i<16; i++ ) {
        wxstr = m_pEditGUID[ i ]->GetValue();
        pEvent->GUID[ i ] = readStringValue( wxstr );
    }

    // Get Data
    wxstr  = m_pMsgData->GetValue();

    int idx = 0;
    wxStringTokenizer tok;
    tok.SetString( wxstr, _(",\r\n") );

    while( ( idx < (512-25) ) && tok.HasMoreTokens() ) {

        wxString tmp = tok.GetNextToken();
        msgdata[ idx ] = readStringValue( tmp );
        idx++;

    }

    // Set Data size
    pEvent->sizeData = idx;

    if ( 0 != idx ) {
        pEvent->pdata = new unsigned char [ idx ];
        if ( NULL != pEvent->pdata ) {
            memcpy( pEvent->pdata, msgdata, idx );
        }
    }

    // Write back data to dialog
    writeDataToDialog( pEvent );

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// writeDataToDialog
//

bool Level2Child::writeDataToDialog( vscpEvent *pEvent ) {
    bool rv = true;

    return rv;
}


