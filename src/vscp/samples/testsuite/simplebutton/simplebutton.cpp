///////////////////////////////////////////////////////////////////////////////
// Name:        simplebutton.cpp
// Purpose:     A simple test application for VSCP
// Author:      Ake Hedman
// Modified by:
// Created:     2004-09-28
// RCS-ID:      $Id: simplebutton.cpp,v 1.20 2005/08/26 07:49:53 akhe Exp $
// Copyright:   (c) 2004-2011 Ake Hedman <akhe@eurosource.se>
// Version 0.0.3
// 
// Copyright (C) 2002 Ake Hedman akhe@eurosource.se 
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: simplebutton.cpp,v $                                       
// $Date: 2005/08/26 07:49:53 $                                  
// $Author: akhe $                                              
// $Revision: 1.20 $ 

#define VERSION  "0.0.3"

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

#include "wx/sizer.h"
#include "wx/menuitem.h"
#include "wx/cmdline.h"
#include "wx/tokenzr.h"
#include "wx/tglbtn.h"
#include "wx/spinctrl.h"

#include "../../../common/vscp_class.h"
#include "../../../common/vscp_type.h"

#include "optiondialog.h"
#include "simplebutton.h"

// For MAC address 

#ifdef WIN32

typedef struct _ASTAT_
{
	ADAPTER_STATUS adapt;
    NAME_BUFFER    NameBuff [30];

}ASTAT, * PASTAT;

ASTAT Adapter;

#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// * * * Event handling * * *

BEGIN_EVENT_TABLE(appFrame, wxFrame)
    EVT_MENU( MenuAbout, appFrame::OnAbout)
    EVT_MENU( MenuQuit, appFrame::OnQuit)
	EVT_MENU( MenuOptions, appFrame::OnDialogSettings )

    EVT_BUTTON( BtnCmd, appFrame::OnButton )

	EVT_TIMER( RefreshTimer, appFrame::OnTimerRefresh )
END_EVENT_TABLE()

IMPLEMENT_APP( SimpleButtonApp );

static const wxCmdLineEntryDesc cmdLineDesc[] = { 
{ wxCMD_LINE_OPTION, "a", "address", "Set LSB of node address (default=1)", wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL }, 
{ wxCMD_LINE_OPTION, "d", "data", "Data part of message", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
{ wxCMD_LINE_OPTION, "c", "class", "Set class", wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL }, 
{ wxCMD_LINE_OPTION, "t", "type", "set type", wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },   
{ wxCMD_LINE_SWITCH, "x", "toggle", "set toggle type button", wxCMD_LINE_VAL_NONE,  wxCMD_LINE_PARAM_OPTIONAL },
{ wxCMD_LINE_NONE } };

///////////////////////////////////////////////////////////////////////////////
//  OnInit
//
// init our app: create windows
//

bool SimpleButtonApp::OnInit( void )
{
	wxCmdLineParser *pparser = new wxCmdLineParser( argc, argv );

    appFrame *pFrame = new appFrame( NULL,
										wxT("VSCP - Simple Button"),
										400, 100, 680, 520 );
    
	if ( NULL != pparser ) {
		
		pparser->SetDesc(  cmdLineDesc );
		pparser->Parse();

		long opt;

		if ( pparser->Found( "address", &opt ) ) {
			pFrame->m_nodeAddr[  0 ] = ( unsigned char )opt;	
		}

		if ( pparser->Found( "class", &opt ) ) {
			pFrame->m_class = ( unsigned char )opt;	
		}

		if ( pparser->Found( "type", &opt ) ) {
			pFrame->m_type = ( unsigned char )opt;	
		}

		wxString wxstr;
		if ( pparser->Found( "data", &wxstr ) ) {

			int idx = 0;
			wxStringTokenizer tok;
			tok.SetString( wxstr, wxT(",\r\n") );
			
			while( ( idx < 8 ) && tok.HasMoreTokens() ) {

				pFrame->m_msgdata[ idx ] = getDataValue( tok.GetNextToken() );
				idx++;

			}

			pFrame->m_datasize = idx;

		}

		if ( pparser->Found( "toggle", &opt ) ) {
			pFrame->m_bToggle = true;	
		}

		delete pparser;
	}
	
	SetTopWindow( pFrame );

    return TRUE;
}


unsigned long SimpleButtonApp::getDataValue( const char *szData )
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


///////////////////////////////////////////////////////////////////////////////
// appFrame
//
// main frame constructor
//

appFrame::appFrame( wxFrame *frame,
					const wxChar *title,
					int x, int y, int w, int h)
                 : wxFrame( frame, -1, title, wxPoint( x, y ), wxSize( w, h ) )
{
	// Button is standard button
	m_bToggle = false;

	// Set the default node address	
	// Note that MSB is in byte 0 beacuse it should
	// be copied as the first byte in the datagram 
	m_nodeAddr[ 15 ] = 1; // User value LSB
	m_nodeAddr[ 14 ] = 0; // User Value MSB
	m_nodeAddr[ 13 ] = 0; // Ethernet MAC LSB
	m_nodeAddr[ 12 ] = 0;
	m_nodeAddr[ 11 ] = 0;
	m_nodeAddr[ 10 ] = 0;
	m_nodeAddr[  9 ] = 0;
	m_nodeAddr[  8 ] = 0; // Ethernet MAC MSB
	m_nodeAddr[  7 ] = 0xfe;
	m_nodeAddr[  6 ] = 0xff;
	m_nodeAddr[  5 ] = 0xff;
	m_nodeAddr[  4 ] = 0xff;
	m_nodeAddr[  3 ] = 0xff;
	m_nodeAddr[  2 ] = 0xff;
	m_nodeAddr[  1 ] = 0xff;
	m_nodeAddr[  0 ] = 0xff;

	// Default class/type/msgdata
	m_class = VSCP_CLASS1_CONTROL;
	m_type = VSCP_TYPE_CONTROL_ALL_LAMPS;
	
	m_msgdata[ 0 ] = 1;	// Default: ON
	m_msgdata[ 1 ] = 0; // Default: Zone = 0
	m_msgdata[ 2 ] = 0;
	m_msgdata[ 3 ] = 0;
	m_msgdata[ 4 ] = 0;
	m_msgdata[ 5 ] = 0;
	m_msgdata[ 6 ] = 0;
	m_msgdata[ 7 ] = 0;

	m_datasize = 2;

	getMacAddress();
		
	// Start count 
	m_secondCounter = 0;

	// Set Button state
	m_state = false;

	// create the status line
	const int widths[] = { -1, 60, 60 };
	CreateStatusBar( 3 );
	SetStatusWidths( 3, widths );
	wxLogStatus( this, wxT("Button") );
	
	///////////////////////////////////////////////////////////////////////////
	//            Persistent storage first time initialization
	///////////////////////////////////////////////////////////////////////////

    // Make a menubar
    // --------------

    // file submenu
    wxMenu *menuFile = new wxMenu;
    menuFile->Append( MenuAbout, wxT("&About...\tF1") );
    menuFile->AppendSeparator();
    menuFile->Append( MenuQuit, wxT("E&xit\tAlt-X") );

    // tools submenu
    //wxMenu *menuTools = new wxMenu;
    //menuTools->Append( MenuTest, wxT("Testa OLE System\tCtrl-M") );

	  // options submenu
    wxMenu *menuOptions = new wxMenu;
	  menuOptions->Append( MenuOptions, wxT("&General Settings...\tF10") );

    // put it all together
    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append( menuFile, wxT("&File") );
    menu_bar->Append( menuOptions, wxT("&Settings") );
    SetMenuBar( menu_bar );

    // make a panel with some controls
    m_panel = new wxPanel( this, -1, wxPoint( 0, 0 ), wxSize(400, 200), wxTAB_TRAVERSAL);

    // create buttons for moving the items around
    m_button = new wxButton( m_panel, 
									BtnCmd, 
									wxT("VSCP"), 
									wxPoint( 420, 60  ),
									wxSize( 200,200 ) );

	  // Sizer for the listbox
    wxBoxSizer *mainsizer = new wxBoxSizer( wxVERTICAL );

	  mainsizer->Add( m_button, 0, wxGROW | wxALL, 10 );
	
    // tell frame to make use of sizer (or constraints, if any)
    m_panel->SetAutoLayout( TRUE );
    m_panel->SetSizer( mainsizer );

    // don't allow frame to get smaller than what the sizers tell ye
    mainsizer->SetSizeHints( this );


	if ( !m_vscpif.doCmdOpen() ) {
		wxMessageBox( _T("Failed to contact the VSCP daemon") );
		Close( TRUE );
	}

	// Initialize timeevents
	m_timer.SetOwner( this, RefreshTimer );
	m_timer.Start( 200 );   // set timer interval

    Show( TRUE );

}


///////////////////////////////////////////////////////////////////////////////
//  ~appFrame
//


appFrame::~appFrame()
{
	// Close pipe
	m_vscpif.doCmdClose();
}


///////////////////////////////////////////////////////////////////////////////
//  OnQuit
//

void appFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
	Close( TRUE );
}


///////////////////////////////////////////////////////////////////////////////
//  wxCommandEvent
//

void appFrame::OnAbout( wxCommandEvent& WXUNUSED( event)  )
{
    wxMessageBox(wxT("VSCP Simple Button\n© Ake Hedman, eurosource 2004 - 2007\nhttp://www.vscp.org"),
						wxT("About pd"),
						wxICON_INFORMATION, this );
}



///////////////////////////////////////////////////////////////////////////////
//  OnTimerRefresh
//

void appFrame::OnTimerRefresh( wxTimerEvent& event ) 
{
	int rv;
	m_secondCounter++;

	// Check if there is data available
	while ( ( rv = m_vscpif.doCmdDataAvailable() ) > 0 ) {
		
		// Yep! there is data
		vscpEvent vscpevent;
		if ( m_vscpif.doCmdReceive( &vscpevent ) ) {

			// Dimmer command
			if ( ( VSCP_CLASS1_CONTROL == vscpevent.vscp_class ) &&
				( VSCP_TYPE_CONTROL_DIM_LAMPS == vscpevent.vscp_type ) ) {

				if ( ( NULL != vscpevent.pdata ) ) {
				
					// Set new dimmer value
					//m_pGauge->SetValue( *(vscpmsg.pdata + 1) );

				}
				
			}

			// on/off command
			else if ( ( VSCP_CLASS1_CONTROL == vscpevent.vscp_class ) &&
				( VSCP_TYPE_CONTROL_ALL_LAMPS == vscpevent.vscp_type ) ) {
			
				if ( ( NULL != vscpevent.pdata ) && vscpevent.pdata ) {
					
					// All lamps on	
					//m_pGauge->SetValue( 100 );

				}
				else {

					// All lamps off
					//m_pGauge->SetValue( 0 );

				}

			}


			// Must remove VSCP data
			if ( NULL != vscpevent.pdata ) {
				delete vscpevent.pdata;
			}

		}
	
	}
}


///////////////////////////////////////////////////////////////////////////////
//  OnDialogSettings
//

void appFrame::OnDialogSettings( wxCommandEvent& event )
{
    int i;
	COptionDialog dlg( this );

	// transfer data

	// Full address
	memcpy( dlg.m_vscpaddr, m_nodeAddr, sizeof( m_nodeAddr ) );

	dlg.m_pSpinCtrlClass->SetValue( m_class );
	dlg.m_pSpinCtrlType->SetValue( m_type );
	dlg.m_pSpinCtrlLsbAddress->SetValue( m_nodeAddr[ 0 ] );
	dlg.m_pSpinCtrlDataSize->SetValue( m_datasize );

	for ( i=0; i<8; i++ ) {
		dlg.m_pSpinCtrlData[ i ]->SetValue( m_msgdata[ i ] );
	}

	dlg.printVSCPAddress();
	
	if ( wxID_OK == dlg.ShowModal() ) {

		// Read back data		
		m_class = dlg.m_pSpinCtrlClass->GetValue();
		m_type = dlg.m_pSpinCtrlType->GetValue();
		m_nodeAddr[  0 ] = dlg.m_pSpinCtrlLsbAddress->GetValue();
		m_datasize = dlg.m_pSpinCtrlDataSize->GetValue();

		for ( i=0; i<8; i++ ) {
			m_msgdata[ i ] = dlg.m_pSpinCtrlData[ i ]->GetValue();
		}


	}
}
 

///////////////////////////////////////////////////////////////////////////////
//  OnButton
//

void appFrame::OnButton( wxCommandEvent& WXUNUSED( event ) )
{
	// Response message

	vscpEvent vscpevent;
	memset( &vscpevent, 0, sizeof( vscpevent ) );

	memcpy( vscpevent.GUID, m_nodeAddr, sizeof( vscpevent.GUID ) );

	vscpevent.vscp_class = m_class;
	vscpevent.vscp_type = m_type;

	vscpevent.sizeData = m_datasize;
	vscpevent.pdata = new unsigned char[ vscpevent.sizeData ];

	if ( NULL != vscpevent.pdata ) {	
		
		memcpy( vscpevent.pdata, m_msgdata, m_datasize );

		if ( !m_vscpif.doCmdSend( &vscpevent ) ) {
			;
		}

		if ( NULL != vscpevent.pdata ) delete vscpevent.pdata;
	 
	}	
}


///////////////////////////////////////////////////////////////////////////////
//  getMacAddress
//

bool appFrame::getMacAddress( void )
{

#ifdef WIN32
	bool rv = false;
	NCB Ncb;
	UCHAR uRetCode;
	//char NetName[ 50 ];
	LANA_ENUM lenum;
	int i;
	//char buf[ MAX_PATH ];

	memset( &Ncb, 0, sizeof(Ncb) );
    Ncb.ncb_command = NCBENUM;
    Ncb.ncb_buffer = (UCHAR *)&lenum;
    Ncb.ncb_length = sizeof(lenum);
    uRetCode = Netbios( &Ncb );
    printf( "The NCBENUM return code is: 0x%x \n", uRetCode );

    for( i=0; i < lenum.length ;i++ ) {
		memset( &Ncb, 0, sizeof(Ncb) );
        Ncb.ncb_command = NCBRESET;
        Ncb.ncb_lana_num = lenum.lana[i];

        uRetCode = Netbios( &Ncb );
        //sprintf( buf, 
		//			"The NCBRESET on LANA %d return code is: 0x%x \n",
		//			lenum.lana[i], uRetCode );
		//wxMessageBox( buf );

        memset( &Ncb, 0, sizeof (Ncb) );
        Ncb.ncb_command = NCBASTAT;
        Ncb.ncb_lana_num = lenum.lana[i];

        strcpy( (char *)Ncb.ncb_callname,  "*               " );
        Ncb.ncb_buffer = (unsigned char *) &Adapter;
        Ncb.ncb_length = sizeof(Adapter);

        uRetCode = Netbios( &Ncb );
        //sprintf( buf, 
		//			"The NCBASTAT on LANA %d return code is: 0x%x \n",
		//			lenum.lana[i], uRetCode );
		//wxMessageBox( buf );
        if ( uRetCode == 0 ) {
			//sprintf( buf, "The Ethernet Number on LANA %d is: %02x %02x %02x %02x %02x %02x\n",
			//				lenum.lana[i],
			//				Adapter.adapt.adapter_address[0],
			//				Adapter.adapt.adapter_address[1],
			//				Adapter.adapt.adapter_address[2],
			//				Adapter.adapt.adapter_address[3],
			//				Adapter.adapt.adapter_address[4],
			//				Adapter.adapt.adapter_address[5] );
			//wxMessageBox( buf );
			m_nodeAddr[ 8 ] = Adapter.adapt.adapter_address[0];
			m_nodeAddr[ 9 ] = Adapter.adapt.adapter_address[1];
			m_nodeAddr[ 10 ] = Adapter.adapt.adapter_address[2];
			m_nodeAddr[ 11 ] = Adapter.adapt.adapter_address[3];
			m_nodeAddr[ 12 ] = Adapter.adapt.adapter_address[4];
			m_nodeAddr[ 13 ] = Adapter.adapt.adapter_address[5];

			rv = true;
		}
	}
	
	return rv;

#endif

}

