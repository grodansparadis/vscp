///////////////////////////////////////////////////////////////////////////////
// SelectInterfacePage.cpp: implementation of the CSelectInterfacePage class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: SelectInterfacePage.cpp,v $                                       
// $Date: 2005/07/22 05:25:55 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 
///////////////////////////////////////////////////////////////////////////////

#include "SelectInterfacePage.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(wxInterfaceSelectPage, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(-1, wxInterfaceSelectPage::OnWizardPageChanging)
    EVT_WIZARD_CANCEL(-1, wxInterfaceSelectPage::OnWizardCancel)
END_EVENT_TABLE()


wxInterfaceSelectPage::wxInterfaceSelectPage( wxWizard *parent )
													: wxWizardPageSimple( parent )
{
	(void)new wxStaticText( this, -1, 
									_T("Select the interface you want to use. \n\n")
                                    _T("If the VSCP daemon is started you probably should use it. Using a direct\n")
									_T("connection to a CANAL driver instead will probably speed up the bootloading\n")	 
									_T("process a lot. Make sure the VSCP daemon is stoped before trying this.\n\n")
								);

    m_pIfListBox = new wxListBox( this, 
									-1, 
									wxPoint( 5, 80 ),
									wxSize( 200, 300 ) );

	m_pSelectDevice = new CSelectDevice(  m_pIfListBox );

}

///////////////////////////////////////////////////////////////////////////////
// ~wxInterfaceSelectPage
//

wxInterfaceSelectPage::~wxInterfaceSelectPage()
{
	if ( NULL != m_pSelectDevice ) delete m_pSelectDevice;	
}

// wizard event handlers

///////////////////////////////////////////////////////////////////////////////
// OnWizardCancel
//

void wxInterfaceSelectPage::OnWizardCancel( wxWizardEvent& event ) 
{
	if ( wxMessageBox( _T("Do you really want to cancel?"), _T("Question" ),
                          wxICON_QUESTION | wxYES_NO, this ) != wxYES ) {
		
		// not confirmed
		event.Veto();

    }
}

///////////////////////////////////////////////////////////////////////////////
// OnWizardPageChanging
//

void wxInterfaceSelectPage::OnWizardPageChanging( wxWizardEvent& event )
{
	// Backwards always OK 
    if ( !event.GetDirection() ) {
		return;
	}

	// Forward OK if selected.
	if ( event.GetDirection() && ( -1 != m_pIfListBox->GetSelection() ) ) {
		return;
	}

    wxMessageBox( _T("Please select one of the interfaces!"), 
					_T("Must select interface"),
					wxICON_WARNING | wxOK, this );

    event.Veto();

}

///////////////////////////////////////////////////////////////////////////////
// TransferDataFromWindow
//

bool wxInterfaceSelectPage::TransferDataFromWindow( void )
{
	wxBusyCursor wait;

	if ( -1 == m_pIfListBox->GetSelection() ) {

		       wxMessageBox( _T("Please select one of the interfaces!"), 
								_T("Must select interface"),
								wxICON_WARNING | wxOK, this );

		return FALSE;

	}



	// Get selected device data
	devItem* pItem = NULL;
	m_pSelectDevice->getSelectedDevice( m_pIfListBox->GetSelection(), &pItem );
	gBootCtrl.setCanalDevice( pItem );

    return TRUE;

}