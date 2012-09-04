// VSCPConfigWnd.cpp
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
// $RCSfile: vscpconfigwnd.cpp,v $
// $Date: 2005/09/08 16:27:40 $
// $Author: akhe $
// $Revision: 1.6 $

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
#include "vscpconfigwnd.h"


// Note that MDI_NEW_WINDOW and MDI_ABOUT commands get passed
// to the parent window for processing, so no need to
// duplicate event handlers here.
BEGIN_EVENT_TABLE( VSCPConfigChild, wxMDIChildFrame )

EVT_MENU( MDI_CHILD_QUIT, VSCPConfigChild::OnQuit )
EVT_MENU( MDI_VIEW_SEND_TOOLS, VSCPConfigChild::OnViewSendTools )
EVT_MENU( MDI_CHANGE_TITLE, VSCPConfigChild::OnChangeTitle )
EVT_MENU( MDI_CHANGE_POSITION, VSCPConfigChild::OnChangePosition )
EVT_MENU( MDI_CHANGE_SIZE, VSCPConfigChild::OnChangeSize )


EVT_BUTTON( RegButtonUpdate, VSCPConfigChild::OnButtonUpdate )

EVT_TIMER(  RegRefreshTimer, VSCPConfigChild::OnTimerRefresh )

EVT_GRID_EDITOR_HIDDEN( VSCPConfigChild::OnRegisterHidden )
EVT_GRID_CELL_CHANGE( VSCPConfigChild::OnRegisterEdited )

EVT_SIZE( VSCPConfigChild::OnSize )
EVT_MOVE( VSCPConfigChild::OnMove )

EVT_CHAR( VSCPConfigChild::OnKey )

EVT_CLOSE(VSCPConfigChild::OnClose)

END_EVENT_TABLE()


// For drawing lines in a canvas
static long xpos = -1;
static long ypos = -1;

// Globals

extern MyFrame *gframe;
extern wxList my_children;
extern int gs_nFrames;


///////////////////////////////////////////////////////////////////////////////
// VSCPConfigChild Constructor
//

VSCPConfigChild::VSCPConfigChild( wxMDIParentFrame *parent,
                                  const wxString& title,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  const long style,
                                  devItem *pdev )
                : wxMDIChildFrame( parent,
                                   RegGrid,
                                   title,
                                   pos,
                                   size,
                                   style  )
{
        int i;

        // Assign the device parameters
        m_pcsw = new CCanalSuperWrapper( pdev );
        if ( NULL == m_pcsw ) {
                wxMessageBox(_("Failed to create driver interface!"));
                return;
        }

        ::wxBeginBusyCursor();

        // Open i/f
        if ( !m_pcsw->doCmdOpen() ) {
                wxMessageBox(_("Unable to open channel to dll or daemon! Please start canal server/service or check that the driver is available."));
                ::wxEndBusyCursor();
                return;
        }

        ::wxEndBusyCursor();

        my_children.Append( this );

        m_pgrid = new wxGrid( this,
                              -1,
                              wxPoint( 0, 0 ),
                              wxSize( 700, 300 ) );

        m_pgridFont = new wxFont( 10, wxDEFAULT, wxNORMAL, wxBOLD );

        m_pgrid->CreateGrid( 0, 4 );
        m_pgrid->SetSelectionMode( wxGrid::wxGridSelectRows );
        m_pgrid->SetColLabelAlignment( wxALIGN_LEFT, wxALIGN_BOTTOM );
        m_pgrid->SetColLabelSize( 20 );

        m_pgrid->SetCellBackgroundColour( wxColour( 0xf0, 0xf0, 0xf0 ) );
        m_pgrid->SetGridLineColour( wxColour( 0x80, 0x80, 0x80 ) );

        m_pgrid->SetCellTextColour( *wxBLACK );
        m_pgrid->SetRowLabelSize( 50 );

        m_pgrid->SetRowLabelSize( 50 );

        m_pgrid->SetDefaultCellFont( *m_pgridFont );

        m_pgrid->SetColSize( 0, 60 );
        m_pgrid->SetColLabelValue( 0, _("Register") );

        m_pgrid->SetColSize( 1, 20 );
        m_pgrid->SetColLabelValue( 1, _(" ") );

        m_pgrid->SetColSize( 2, 60 );
        m_pgrid->SetColLabelValue( 2, _("Content") );

        m_pgrid->SetColSize( 3, 500 );
        m_pgrid->SetColLabelValue( 3, _("Description") );



        m_ptopSizer = new wxBoxSizer( wxVERTICAL );
        m_ptopSizer->Add( m_pgrid,
                          1,
                          wxEXPAND );

        m_pCmdWnd = new wxWindow( this,
                                  RegCmdWnd,
                                  wxPoint( 0, 0 ),
                                  wxSize( 700, 100 ) );


        m_pBtnLoadFile = new wxButton( m_pCmdWnd,
                                       RegButtonLoadFile,
                                       _("Load File"),
                                       wxPoint( 10, 10 ),
                                       wxSize( 90, 20 ) );

        m_pBtnSaveFile = new wxButton( m_pCmdWnd,
                                       RegButtonSaveFile,
                                       _("Save File"),
                                       wxPoint( 10, 35 ),
                                       wxSize( 90, 20 ) );


        m_pBtnUpdate = new wxButton( m_pCmdWnd,
                                     RegButtonUpdate,
                                     _("Update"),
                                     wxPoint( 610, 20 ),
                                     wxSize( 70, 20 ) );

        m_pBtnUndo = new wxButton( m_pCmdWnd,
                                   RegButtonUndo,
                                   _("Undo"),
                                   wxPoint( 610, 45 ),
                                   wxSize( 70, 20 ) );

        ( void ) new wxStaticText( m_pCmdWnd,
                                   -1,
                                   _( "GUID" ),
                                   wxPoint( 280, 5 ) );

        for ( i=0; i<8; i++ ) {

                m_pEditGUID[ i ] = new wxTextCtrl( m_pCmdWnd,
                                                   RegEditGUID + i,
                                                   _(""),
                                                   wxPoint( 280 + i*40, 20 ),
                                                   wxSize( 40, 20 ),
                                                   wxTE_RIGHT | wxTE_PROCESS_TAB,
                                                   wxTextValidator( wxFILTER_ALPHANUMERIC, &m_wxStrGUID[ i ] ) );
        }

        for ( i=8; i<16; i++ ) {

                m_pEditGUID[ i ] = new wxTextCtrl( m_pCmdWnd,
                                                   RegEditGUID + i,
                                                   _(""),
                                                   wxPoint( 280 + (i-8 )*40, 40 ),
                                                   wxSize( 40, 20 ),
                                                   wxTE_RIGHT | wxTE_PROCESS_TAB,
                                                   wxTextValidator( wxFILTER_ALPHANUMERIC, &m_wxStrGUID[ i ] ) );
        }

        ( void ) new wxStaticText( m_pCmdWnd,
                                   -1,
                                   _( "15 - 8" ),
                                   wxPoint( 245, 23 ),
                                   wxSize( 30, 20 ),
                                   wxALIGN_RIGHT );

        ( void ) new wxStaticText( m_pCmdWnd,
                                   -1,
                                   _( "7 - 0" ),
                                   wxPoint( 245, 43 ),
                                   wxSize( 30, 20 ),
                                   wxALIGN_RIGHT );

        m_pEditNickname = new wxTextCtrl( m_pCmdWnd,
                                          RegEditNickname,
                                          _("1"),
                                          wxPoint( 200, 20 ),
                                          wxSize( 40, 20 ),
                                          wxTE_RIGHT | wxTE_PROCESS_TAB,
                                          wxTextValidator( wxFILTER_ALPHANUMERIC, &m_wxStrNickname ) );

        ( void ) new wxStaticText( m_pCmdWnd,
                                   -1,
                                   _( "Nickname" ),
                                   wxPoint( 140, 23 ),
                                   wxSize( 50, 20 ),
                                   wxALIGN_RIGHT );

        m_pStaticMdfURL = new wxStaticText( m_pCmdWnd,
                                            RegStaticMdfURL,
                                            _( "http://www.somewhere.com/a123.xml" ),
                                            wxPoint( 280, 70 ),
                                            wxSize( 200, 20 ),
                                            wxALIGN_LEFT );

        m_pBtnLoadMDF = new wxButton( m_pCmdWnd,
                                      RegButtonLoadMDF,
                                      _("Load MDF data"),
                                      wxPoint( 180, 70 ),
                                      wxSize( 90, 20 ) );

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
        m_timer.SetOwner( this, RegRefreshTimer );
        m_timer.Start( 100 );

        wxString s_title, s_add;
        s_add.Printf(_(" - VSCP Configuration %d"), gs_nFrames );
        if ( NULL == pdev ) {
          // The daemon
          s_title = _("VSCP Configuration ");
        } else {
          s_title = pdev->strName;
        }
        s_title += s_add;
        SetTitle( s_title );

        // Init standard register info
        initStandardRegInfo();

        SendSizeEvent(); // Needed to calculate the children positions
}

///////////////////////////////////////////////////////////////////////////////
// VSCPConfigChild Destructor
//

VSCPConfigChild::~VSCPConfigChild()
{
	/*if ( m_plogActivate->GetValue() ) {
		m_plogActivate->SetValue( false );
    }*/

	my_children.DeleteObject( this );


    if ( NULL != m_pgridFont )
		delete m_pgridFont;

    // Delete CanalSuperWrapper object
    if ( NULL != m_pcsw )
		delete m_pcsw;
    m_pcsw = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// OnQuit
//

void VSCPConfigChild::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
        // Delete CanalSuperWrapper object
        if ( NULL != m_pcsw )
                delete m_pcsw;
        m_pcsw = NULL;

        Close( TRUE );
}

///////////////////////////////////////////////////////////////////////////////
// OnClose
//

void VSCPConfigChild::OnClose( wxCloseEvent& event )
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
        if ( NULL != m_pcsw )
                delete m_pcsw;
        m_pcsw = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// OnRefresh
//

void VSCPConfigChild::OnViewSendTools( wxCommandEvent& WXUNUSED( event ) )
{
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

void VSCPConfigChild::OnChangePosition( wxCommandEvent& WXUNUSED( event ) )
{
        Move(10, 10);
}

///////////////////////////////////////////////////////////////////////////////
// OnChangeSize
//

void VSCPConfigChild::OnChangeSize( wxCommandEvent& WXUNUSED( event ) )
{
        SetClientSize(100, 100);

}

///////////////////////////////////////////////////////////////////////////////
// OnChangeTitle
//

void VSCPConfigChild::OnChangeTitle( wxCommandEvent& WXUNUSED( event ) )
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

void VSCPConfigChild::OnActivate( wxActivateEvent& event )
{

        //if ( event.GetActive() && canvas ) {
        //    canvas->SetFocus();
        //}
}

///////////////////////////////////////////////////////////////////////////////
// OnMove
//

void VSCPConfigChild::OnMove( wxMoveEvent& event )
{
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

void VSCPConfigChild::OnSize( wxSizeEvent& event )
{
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

void VSCPConfigChild::OnKey( wxKeyEvent& event )
{

        event.Skip();
}






///////////////////////////////////////////////////////////////////////////////
//  OnTimerRefresh
//

void VSCPConfigChild::OnTimerRefresh( wxTimerEvent& event )
{
        /*if ( m_plogActivate->GetValue() ) {
          fillData();
        }*/
}



//////////////////////////////////////////////////////////////////////////////
// getCANData
//

bool VSCPConfigChild::getCanData( canalMsg *pMsg )
{
        int i;
        unsigned char nData = 0;

        // Must have somewhere to store data
        if ( NULL == pMsg )
                return false;

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
        /*
         // Get ID	
         wxstr = m_pMsgId->GetValue();
         pMsg->id = gframe->getDataValue( wxstr );



         // Get Data
         for ( i=7; i>=0; i-- ) {
           wxstr = m_pMsgData[ i ]->GetValue();
           pMsg->data[ i ] = gframe->getDataValue( wxstr );
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
        */
        return true;
}

//////////////////////////////////////////////////////////////////////////////
// writeDataToDialog
//

bool VSCPConfigChild::writeDataToDialog( canalMsg *pMsg )
{
        bool rv = true;

        return rv;
}


//////////////////////////////////////////////////////////////////////////////
// initStandardRegInfo
//

void VSCPConfigChild::initStandardRegInfo( void )
{
        int i;
        wxString strBuf;

        for ( i=0; i<128; i++ ) {
                m_pgrid->AppendRows( 1 );
                strBuf.Printf( _("0x%02lx"), i );
                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
                m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );

                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("--") );
                m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );

                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
                m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );

                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("") );
        }

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x80 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Alarm Status Register") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x81 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("VSCP Major version number") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x82 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("VSCP Minor version number") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf(  _("0x%02lx"), 0x83 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("rw") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Node Control Flags\r test") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x84 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("rw") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("User ID 0") );





        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x85 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("rw") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("User ID 1") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf(  _("0x%02lx"), 0x86 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("rw") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );


        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("User ID 2") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf(  _("0x%02lx"), 0x87 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("rw") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("User ID 3") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x88 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("rw") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("User ID 4") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x89 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Manufacturer device id 0") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x8a );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Manufacturer device id 1") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x8b );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Manufacturer device id 2") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x8c );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 3 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Manufacturer device id 3") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x8d );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Manufacturer sub device id 0") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x8e );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Manufacturer sub device id 1") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x8f );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Manufacturer sub device id 2") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x90 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Manufacturer sub device id 3") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x91 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Nickname id") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x92 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("rw") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Page select register MSB") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x93 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("rw") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Page select register LSB") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x94 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Firmware major version number") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x95 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Firmware minor version number") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x96 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Firmware sub minor version number") );


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0x97 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Boot loader algorithm") );


        // Reserved locations
        for ( i=0x98; i<0xd0; i++ ) {

                m_pgrid->AppendRows( 1 );
                strBuf.Printf( _("0x%02lx"), i );
                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
                m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
                m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("--") );
                m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
                m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
                m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
                m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("reserved") );

        }


        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xd0 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 15 MSB") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xd1 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 14") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xd2 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 13") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf(  _("0x%02lx"), 0xd3 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 12") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xd4 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 11") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xd5 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 10") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xd6 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 9") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xd7 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 8") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xd8 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 7") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xd9 );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1, _( "r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 6") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xda );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 5") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xdb );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 4") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xdc );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 3") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xdd );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 2") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xde );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 1") );

        m_pgrid->AppendRows( 1 );
        strBuf.Printf( _("0x%02lx"), 0xdf );
        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
        m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
        m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

        m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("GUID Byte 0 LSB") );


        for ( i=0; i<32; i++ ) {

                m_pgrid->AppendRows( 1 );
                strBuf.Printf( _("0x%02lx"), 0xe0 + i );
                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 0,  strBuf );
                m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 0 );
                m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 0 );

                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 1,  _("r-") );
                m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 1 );
                m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 1 );

                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 2,  _("??") );
                m_pgrid->SetCellAlignment( wxALIGN_CENTRE, m_pgrid->GetNumberRows()-1, 2 );
                m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );

                m_pgrid->SetCellValue( m_pgrid->GetNumberRows()-1, 3,  _("Module device file URL") );
        }
}


//////////////////////////////////////////////////////////////////////////////
// OnButtonUpdate
//

void VSCPConfigChild::OnButtonUpdate( wxCommandEvent& event )
{
	int errors = 0;
  wxString strBuf;
  int i;
  uint8_t nodeid = 1;
  canalMsg msg;
	bool bResend;

  wxString strtmp = m_pEditNickname->GetValue();
  nodeid = readStringValue( strtmp );

	if ( ( 0 == nodeid ) || ( nodeid > 254 )  ) {
		wxMessageBox( _("Invalid Node ID! Must be 1-254") );
		return;
	}

  ::wxBeginBusyCursor();

	// *********************************
	// Write data from changed registers
	// *********************************
	for ( i = 0; i < 256; i++ ) {

		if ( *wxRED == m_pgrid->GetCellTextColour( i, 2 ) ) {

		msg.flags = CANAL_IDFLAG_EXTENDED;
        msg.obid = 0;
        msg.id = 0x0B00;		// CLASS1.PROTOCOL Write register
        msg.sizeData = 3;
        msg.data[ 0 ] = nodeid;	// Node to read from
        msg.data[ 1 ] = i;		// Register to write

		unsigned long val;
		wxString wxstr = m_pgrid->GetCellValue( i, 2 );
		if ( wxstr.ToULong( &val, 16 ) ) {
			msg.data[ 2 ] = (unsigned char) val;	// Data to write
		}
		else {
			continue;
		}

		bResend = false;
		m_pcsw->doCmdSend( &msg );
 
    wxDateTime start = wxDateTime::Now();

    while ( true ) {

			if ( m_pcsw->doCmdDataAvailable() ) {					// Message available
				if ( m_pcsw->doCmdReceive( &msg ) ) {				// Valid message
					if ( (unsigned short)( msg.id & 0xffff ) ==
							( 0x0a00 + nodeid ) ) {					// Read reply?
						if ( msg.data[ 0 ] == i ) {					// Requested register?
							if ( msg.data[ 1 ] == val ) {			// value

								// Update display
								strBuf.Printf( _("0x%02lx"), msg.data[ 1 ] );
								m_pgrid->SetCellValue( i, 2,  strBuf );
								m_pgrid->SetCellAlignment( wxALIGN_CENTRE, i, 2 );
								m_pgrid->SetCellTextColour( i, 2, *wxBLUE );
								m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );
								m_pgrid->SelectRow( i );
								m_pgrid->MakeCellVisible( i, 2 );
								m_pgrid->Update();

								m_registers[ i ] = m_remote_registers[ i ] = msg.data[ 1 ];
								break;
							}

						}
					}
				}
			}

            if ( (wxDateTime::Now() - start).GetSeconds() > 2 ) {
				errors++;
                break;
            }
			else if ( (wxDateTime::Now() - start).GetSeconds() > 1 ) {
				// Send again
				if ( !bResend) m_pcsw->doCmdSend( &msg );
					bResend = true;
				}

		} // while

        if ( errors > 2 ) {
			wxMessageBox( _("No node present or problems when communicating with node. Aborting!") );
            break;
        }

		}

	}

		// *********************
		// Read register content
		// *********************
        for ( i = 0; i < 256; i++ ) {

			
				msg.flags = CANAL_IDFLAG_EXTENDED;
                msg.obid = 0;
                msg.id = 0x0900;		// CLASS1.PROTOCOL Read register
                msg.sizeData = 2;
                msg.data[ 0 ] = nodeid;	// Node to read from
                msg.data[ 1 ] = i;		// Register to read

				bResend = false;
				m_pcsw->doCmdSend( &msg );
         
                wxDateTime start = wxDateTime::Now();

                while ( true ) {

					if ( m_pcsw->doCmdDataAvailable() ) {							// Message available
						if ( m_pcsw->doCmdReceive( &msg ) ) {						// Valid message
							if ( (unsigned short)( msg.id & 0xffff ) ==
                                              ( 0x0a00 + nodeid ) ) {	// Read reply?
								if ( msg.data[ 0 ] == i ) {													// Requested register?

									// Update display
                                    strBuf.Printf( _("0x%02lx"), msg.data[ 1 ] );
                                    m_pgrid->SetCellValue( i, 2,  strBuf );
                                    m_pgrid->SetCellAlignment( wxALIGN_CENTRE, i, 2 );
                                    m_pgrid->SetReadOnly( m_pgrid->GetNumberRows()-1, 2 );
                                    m_pgrid->SelectRow( i );
                                    m_pgrid->MakeCellVisible( i, 2 );
                                    m_pgrid->Update();

                                    m_registers[ i ] = m_remote_registers[ i ] = msg.data[ 1 ];
                                    break;

								}
							}
						}
					}

                    if ( (wxDateTime::Now() - start).GetSeconds() > 2 ) {
						errors++;
                        break;
					}
					else if ( (wxDateTime::Now() - start).GetSeconds() > 1 ) {
						// Send again
						if ( !bResend) m_pcsw->doCmdSend( &msg );
						bResend = true;
					}

                } // while

                if ( errors > 2 ) {
                        wxMessageBox( _("No node present or problems when communicating with node. Aborting!") );
                        break;
                }
			

        }


        // Fill in GUID
        for ( i=0; i<16; i++ ) {

                strBuf.Printf( _("0x%02lx"), m_registers[ 0xd0 + i ] );
                m_pEditGUID[ i ]->SetValue( strBuf );

        }


        // Fill in MDF URL
        wxString str = _("http://");
        char urlbuf[ 33 ];	// 32 chars + terminating null

        memset( urlbuf, 0, sizeof( urlbuf ) );

        for ( i=0; i<32; i++ ) {
          urlbuf[ i ] = m_registers[ 0xe0 + i ];
        }

        str += (wxChar *)urlbuf;
        m_pStaticMdfURL->SetLabel( str );

        ::wxEndBusyCursor();

}

//////////////////////////////////////////////////////////////////////////////
// OnRegisterHidden
//

void VSCPConfigChild::OnRegisterHidden( wxGridEvent& event )
{}

//////////////////////////////////////////////////////////////////////////////
// OnRegisterEdited
//

void VSCPConfigChild::OnRegisterEdited( wxGridEvent& event )
{
        uint8_t val;
        wxString strBuf;
        wxString str;

        if  ( 2 == event.GetCol() ) {
                str = m_pgrid->GetCellValue( event.GetRow(), event.GetCol() );
				if ( str.First('x') || str.First('X') ) {
					long lval;
					if ( str.ToLong( &lval) ) {
						val = (unsigned char)lval;
					}
				}
                val = readStringValue( str );
                strBuf.Printf( _("0x%02lx"), val );
                m_pgrid->SetCellValue( event.GetRow(), event.GetCol(), strBuf );

                m_pgrid->SetCellTextColour( event.GetRow(),
                                            event.GetCol(),
                                            *wxRED );
        }
}



