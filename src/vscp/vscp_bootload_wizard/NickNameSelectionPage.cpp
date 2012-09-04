///////////////////////////////////////////////////////////////////////////////
// NickNameSelectionPage.cpp: implementation of the CNickNameSelectionPage class.
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
// $RCSfile: NickNameSelectionPage.cpp,v $                                       
// $Date: 2005/08/03 07:08:57 $                                  
// $Author: akhe $                                              
// $Revision: 1.5 $ 
///////////////////////////////////////////////////////////////////////////////

#include "NickNameSelectionPage.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( wxNickNameSelectionPage, wxWizardPageSimple )
    EVT_WIZARD_PAGE_CHANGING( -1, wxNickNameSelectionPage::OnWizardPageChanging )
    EVT_WIZARD_CANCEL( -1, wxNickNameSelectionPage::OnWizardCancel )
	EVT_BUTTON( 108, wxNickNameSelectionPage::OnAddButton )
	EVT_BUTTON( 109, wxNickNameSelectionPage::OnRemoveButton )
	EVT_BUTTON( 110, wxNickNameSelectionPage::OnTestButton )
END_EVENT_TABLE()


wxNickNameSelectionPage::wxNickNameSelectionPage( wxWizard *parent )
													: wxWizardPageSimple( parent )
{
	m_cntNodes = 0;	// No nodes yet

	for ( int i=0; i < 256; i++ ) {
		m_Node[ i ] = FALSE;
	}


	(void)new wxStaticText( this, 
				-1,
				_T("On this page you add the nickname-id for the device(s) you want\n")
				_T(" to update.\n")
                _T("The nickname-id is the lowest byte of the CAN id.\n") 
				); 
	
	
	m_pNnListBox = new wxListBox( this, 
									-1, 
									wxPoint( 5, 60 ),
									wxSize( 200, 300 ),
									0, NULL,
									wxLB_SORT | wxLB_MULTIPLE
									);

	m_pAddButton = new wxButton( this, 108, _T("Add Node"), wxPoint( 210, 60 ), wxSize( 100, 27 ) );
	m_pRemoveButton = new wxButton( this, 109, _T("Remove Node(s)"), wxPoint( 210, 90 ), wxSize( 100, 27 ) );
	m_pTestButton = new wxButton( this, 110, _T("Test Node(s)"), wxPoint( 210, 120 ), wxSize( 100, 27 ) );
}

 

// wizard event handlers

///////////////////////////////////////////////////////////////////////////////
// OnWizardCancel
//

void wxNickNameSelectionPage::OnWizardCancel( wxWizardEvent& event ) 
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

void wxNickNameSelectionPage::OnWizardPageChanging( wxWizardEvent& event )
{
	// Backwards always OK 
    if ( !event.GetDirection() ) {
		return;
	}

	// Forward OK if selected.
	if ( event.GetDirection() && m_cntNodes ) {
		return;
	}

    wxMessageBox( _T("Please select at least one node!"), 
								_T("Must select at least one node"),
								wxICON_WARNING | wxOK, this );

    event.Veto();

}

///////////////////////////////////////////////////////////////////////////////
// TransferDataFromWindow
//

bool wxNickNameSelectionPage::TransferDataToWindow( void )
{

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// TransferDataFromWindow
//

bool wxNickNameSelectionPage::TransferDataFromWindow( void )
{
	if ( 0 != m_cntNodes ) {

		// Test the nodes
		for ( int i = 0; i < 256; i++ ) {	
		
			if ( m_Node[ i ] ) {

				// This node should be bootloaded
				gBootCtrl.addClient( i );
			
			}

		}
	}

    return TRUE;

}


///////////////////////////////////////////////////////////////////////////////
// OnAddButton
//

void wxNickNameSelectionPage::OnAddButton( wxCommandEvent & event )
{
	long id;

	wxTextEntryDialog* pDlg = 
		new wxTextEntryDialog( this, 
								_T("Please enter node nickname ID between 1 and 255.\n\n")
								_T("The number can be entered as a decimal or \n")
								_T("or a hexadecimal integer. If hexadecimal use the form 0xnn\n"),
								_T("Enter Node Nickname ID"),
								_T(""),
								wxOK | wxCANCEL );

	if ( wxID_OK == pDlg->ShowModal() ) {
		
		wxString wx = pDlg->GetValue();
		if ( wx.ToLong( &id, 0 ) ) {

			id &= 0xff;	// Make sure its a byte

			// Check that the id is unique
			if ( !m_Node[ id ] ) {

				m_Node[ id ] = TRUE;
				m_cntNodes++;
				wx.Printf( _T("Node with nickname 0x%02X (%d)"), id, id );
				m_pNnListBox->Append( wx, ( m_Node + id ) );
				
			}
			else {

				wxMessageBox( _T("This node has already been added to the list!") );
			}

		}
		else {

			wxMessageBox( _T("Faild to convert node nickname id. Try again!") );

		}
	}

	pDlg->Destroy();

}


///////////////////////////////////////////////////////////////////////////////
// OnRemoveButton
//

void wxNickNameSelectionPage::OnRemoveButton( wxCommandEvent & event )
{
	wxArrayInt wxa;
	m_pNnListBox->GetSelections( wxa );

	if ( wxa.IsEmpty() ) {
		wxMessageBox( _T("No nodes has been selected for removal.") );
		return;
	}

	// Remove the nodes
	for ( size_t i = 0; i < wxa.GetCount(); i++ ) {	
		
		if ( NULL != m_pNnListBox->GetClientData( wxa.Item( i ) ) ) {
			*(unsigned char *)( m_pNnListBox->GetClientData( wxa.Item( i ) ) ) = FALSE;
		}

		m_pNnListBox->Delete( wxa.Item( i ) );
		m_cntNodes--;

	}
}


///////////////////////////////////////////////////////////////////////////////
// OnTestButton
//

void wxNickNameSelectionPage::OnTestButton( wxCommandEvent & event )
{
	wxArrayInt wxa;
	m_pNnListBox->GetSelections( wxa );

	if ( wxa.IsEmpty() ) {
		wxMessageBox( _T("No nodes has been selected for test.") );
		return;
	}

	// Test the nodes
	for ( size_t i = 0; i < wxa.GetCount(); i++ ) {	
		
		if ( ( m_Node + i ) == m_pNnListBox->GetClientData( wxa.Item( i ) ) ) {

			// This node should be tested
			
			// First test if this node is a VSCP node

			


		}

	}
}