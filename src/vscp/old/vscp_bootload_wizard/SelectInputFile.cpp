///////////////////////////////////////////////////////////////////////////////
// SelectInputFile.cpp: implementation of the CSelectInputFile class.
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
// $RCSfile: SelectInputFile.cpp,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

#include "SelectInputFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(wxFileSelectPage, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(-1, wxFileSelectPage::OnWizardPageChanging)
    EVT_WIZARD_CANCEL(-1, wxFileSelectPage::OnWizardCancel)
END_EVENT_TABLE()


wxFileSelectPage::wxFileSelectPage( wxWizard *parent )
										: wxWizardPageSimple( parent )
{
	(void)new wxStaticText( this, 
			-1, 
			_T("When you select next you will be asked to choose an input file\n\n")
            _T("This file should have a valid Intel HEX file format with the proper mapping.\n\n") 
			_T("Program memory\n")
			_T("===============\n")
			_T("0x000200 - 0x1FFFFF ")
			_T("The bootloader is at 0x000200 - 0x1FF and this \n")
			_T("area is a protected area.\n\n")
			_T("Config memory\n")
			_T("=============\nn")
			_T("0x300000 - 0x3FFFFF\n\n")
			_T("EEPROM memory\n")
			_T("=============\n")
			_T("0xF00000 - 0xFFFFFF\n\n")
			_T("Note that locations that are not written with data from the loaded file \n")
			_T("will hold 0xff.\n")
			); 
}

 


// wizard event handlers



///////////////////////////////////////////////////////////////////////////////
// OnWizardCancel
//

void wxFileSelectPage::OnWizardCancel( wxWizardEvent& event ) 
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

void wxFileSelectPage::OnWizardPageChanging( wxWizardEvent& event )
{
	// Backwards always OK 
    if ( !event.GetDirection() ) {
		return;
	}

	// Forward OK if selected.
	if ( event.GetDirection() ) {
		wxFileDialog *pdlg = new wxFileDialog( this );
		
		if ( pdlg->ShowModal() == wxID_OK ) {
			
			if ( gBootCtrl.loadIntelHexFile( pdlg->GetPath().GetData() ) ) {
				
				delete pdlg;
				return;

			}
			else {
				
				wxMessageBox( _T("Failed to load input file.!"), 
								_T("ERROR"),
								wxICON_WARNING | wxOK, this );

			}

		}

		pdlg->Destroy(); 
		delete pdlg;

	}

    wxMessageBox( _T("Please select an input file!"), 
								_T("Must select file."),
								wxICON_WARNING | wxOK, this );

    event.Veto();

}

///////////////////////////////////////////////////////////////////////////////
// TransferDataFromWindow
//

bool wxFileSelectPage::TransferDataFromWindow( void )
{
	/*if ( -1 == m_pIfListBox->GetSelection() ) {

		       wxMessageBox( _T("Please select one of the interfaces!"), 
								_T("Must select interface"),
								wxICON_WARNING | wxOK, this );

		return FALSE;

	}*/

    return TRUE;

}
