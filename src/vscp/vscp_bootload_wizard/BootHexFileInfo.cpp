///////////////////////////////////////////////////////////////////////////////
// BootHexFileInfo.cpp: implementation of the CBootHexFileInfo class.
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
// $RCSfile: BootHexFileInfo.cpp,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

#include "BootHexFileInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(wxBootHexFileInfoPage, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(-1, wxBootHexFileInfoPage::OnWizardPageChanging)
    EVT_WIZARD_CANCEL(-1, wxBootHexFileInfoPage::OnWizardCancel)
END_EVENT_TABLE()


wxBootHexFileInfoPage::wxBootHexFileInfoPage( wxWizard *parent )
													: wxWizardPageSimple( parent )
{

	(void)new wxStaticText( this, 
				-1,
				_T("This page contains information about memory areas that will be \n")
                _T("loaded with data. You can edit the adress range to suit your needs.\n") 
				_T("Locations that are not loaded with data will contain 0xff.\n")
				); 
	
	

	// * * * Flash Memory * * *

	(void)new wxStaticText( this, 
				-1, _T("Flash Memory"), wxPoint( 5, 50 ) );

	(void)new wxStaticText( this, 
				-1, _T("Start :"), wxPoint( 50, 70 ), wxDefaultSize, wxALIGN_RIGHT );

	m_startFlashMemory = new wxTextCtrl( this, 101, _T("0"), wxPoint( 80, 65 ) );

	(void)new wxStaticText( this, 
				-1, _T("End :"), wxPoint( 50, 90 ), wxDefaultSize, wxALIGN_RIGHT );

	m_endFlashMemory = new wxTextCtrl( this, 102, _T("0"), wxPoint( 80, 85 ) );

	m_pFlashCheckBox = new wxCheckBox( this, 107, _T("Program"), wxPoint( 190, 65 ) );
	m_pFlashCheckBox->SetValue( FALSE );




	// * * * Config Memory * * *

	(void)new wxStaticText( this, 
				-1, _T("Config Memory"),wxPoint( 5, 120 ) );

	(void)new wxStaticText( this, 
				-1, _T("Start :"), wxPoint( 50, 140 ), wxDefaultSize, wxALIGN_RIGHT );

	m_startConfigMemory = new wxTextCtrl( this, 103, _T("0"), wxPoint( 80, 135 ) );

	(void)new wxStaticText( this, 
				-1, _T("End :"), wxPoint( 50, 160 ), wxDefaultSize, wxALIGN_RIGHT );

	m_startConfigMemory = new wxTextCtrl( this, 104, _T("0"), wxPoint( 80, 155 ) );

	m_pConfigCheckBox = new wxCheckBox( this, 107, _T("Program (not recommended)"), wxPoint( 190, 135 ) );
	m_pConfigCheckBox->SetValue( FALSE );




	// * * * EEPROM * * *

	(void)new wxStaticText( this, 
				-1, _T("EEPROM Memory"),wxPoint( 5, 190 ) );

	(void)new wxStaticText( this, 
				-1, _T("Start :"), wxPoint( 50, 210 ), wxDefaultSize, wxALIGN_RIGHT );

	m_startEEPROMMemory = new wxTextCtrl( this, 105, _T("0"), wxPoint( 80, 205 ) );

	(void)new wxStaticText( this, 
				-1, _T("End :"), wxPoint( 50, 230 ), wxDefaultSize, wxALIGN_RIGHT );

	m_endEEPROMMemory = new wxTextCtrl( this, 106, _T("0"), wxPoint( 80, 225 ) );

	m_pEEPROMCheckBox = new wxCheckBox( this, 107, _T("Program (not recomended)"), wxPoint( 190, 205 ) );
	m_pEEPROMCheckBox->SetValue( FALSE );
}


// wizard event handlers

///////////////////////////////////////////////////////////////////////////////
// OnWizardCancel
//

void wxBootHexFileInfoPage::OnWizardCancel( wxWizardEvent& event ) 
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

void wxBootHexFileInfoPage::OnWizardPageChanging( wxWizardEvent& event )
{
	// Backwards always OK 
    if ( !event.GetDirection() ) {
		return;
	}

	// Forward OK if selected.
	if ( event.GetDirection() ) {
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

bool wxBootHexFileInfoPage::TransferDataToWindow( void )
{
 
	
	wxString wxstr;

	// Flash

	if ( gBootCtrl.m_memImage.m_bPrgData ) {

		m_pFlashCheckBox->SetValue( TRUE );

		wxstr.Printf( _T("0x%08X"), gBootCtrl.m_memImage.m_minFlashAddr ); 
		m_startFlashMemory->SetValue( wxstr );

		wxstr.Printf( _T("0x%08X"), gBootCtrl.m_memImage.m_maxFlashAddr ); 
		m_endFlashMemory->SetValue( wxstr );

	}

	// Config

	if ( gBootCtrl.m_memImage.m_bConfigData ) {

		wxstr.Printf( _T("0x%08X"), gBootCtrl.m_memImage.m_minConfigAddr ); 
		m_startConfigMemory->SetValue( wxstr );

		wxstr.Printf( _T("0x%08X"), gBootCtrl.m_memImage.m_maxConfigAddr ); 
		m_endConfigMemory->SetValue( wxstr );

	}

	// EEPROM

	if ( gBootCtrl.m_memImage.m_bEEPROMData ) {

		wxstr.Printf( _T("0x%08X"), gBootCtrl.m_memImage.m_minEEPROMAddr ); 
		m_startEEPROMMemory->SetValue( wxstr );

		wxstr.Printf( _T("0x%08X"), gBootCtrl.m_memImage.m_maxEEPROMAddr ); 
		m_endEEPROMMemory->SetValue( wxstr );

	}
		
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// TransferDataFromWindow
//

bool wxBootHexFileInfoPage::TransferDataFromWindow( void )
{
	wxString wxstr;
	unsigned long value;


	// The program data area has a valid range in the region
	// 0x000000 - 0x2fffff

	if ( ( gBootCtrl.m_memImage.m_bPrgData = m_pFlashCheckBox->IsChecked() ) ) {

		wxstr = m_startFlashMemory->GetValue();

		if ( !wxstr.ToULong( &value, 0 ) ) {
			wxMessageBox(_T("ERROR: Unable to convert flash start address to a valid number.") );
			return FALSE;
		}
		
		if ( value > 0x2fffff ) {
			wxMessageBox(_T("ERROR: The flash start address must be in the range 0x000000 - 0x2fffff.") );
			return FALSE;
		}

		if ( value < 0x200 ) {
			wxMessageBox(_T("WARNING: The bootloader code may be overwritten if this flash start address is set!") );
		}

		gBootCtrl.m_memImage.m_minFlashAddr = value;

		// -----

		wxstr = m_endFlashMemory->GetValue();

		if ( !wxstr.ToULong( &value, 0 ) ) {
			wxMessageBox(_T("ERROR: Unable to convert flash end address to a valid number.") );
			return FALSE;
		}

		if ( value > 0x2fffff ) {
			wxMessageBox(_T("ERROR: The flash end address must be in the range 0x000000 - 0x2fffff.") );
			return FALSE;
		}

		gBootCtrl.m_memImage.m_maxFlashAddr = value;

	}


	// The config data area has a valid range in the region
	// 0x300000 - 0x3fffff

	if ( ( gBootCtrl.m_memImage.m_bConfigData = m_pConfigCheckBox->IsChecked() ) ) {

		wxstr = m_startConfigMemory->GetValue();

		if ( !wxstr.ToULong( &value, 0 ) ) {
			wxMessageBox(_T("ERROR: Unable to convert config start address to a valid number.") );
			return FALSE;
		}

		if ( value < 0x300000 ) {
			wxMessageBox(_T("ERROR: The config start address must be in the range 0x300000 - 0x3fffff.") );
			return FALSE;
		}

		if ( value > 0x3fffff ) {
			wxMessageBox(_T("ERROR: The config start address must be in the range 0x300000 - 0x3fffff.") );
			return FALSE;
		}

		gBootCtrl.m_memImage.m_minConfigAddr = value;

		// ------

		wxstr = m_endConfigMemory->GetValue();

		if ( !wxstr.ToULong( &value, 0 ) ) {
			wxMessageBox(_T("ERROR: Unable to convert config end address to a valid number.") );
			return FALSE;
		}

		if ( value < 0x300000 ) {
			wxMessageBox(_T("ERROR: The config end address must be in the range 0x300000 - 0x3fffff.") );
			return FALSE;
		}

		if ( value > 0x3fffff ) {
			wxMessageBox(_T("ERROR: The config end address must be in the range 0x300000 - 0x3fffff.") );
			return FALSE;
		}

		gBootCtrl.m_memImage.m_maxConfigAddr = value;

	}


	// The EEPROM data area has a valid range in the region
	// 0xf00000 - 0xffffff

	if ( ( gBootCtrl.m_memImage.m_bEEPROMData = m_pEEPROMCheckBox->IsChecked() ) ) {

		wxstr = m_startEEPROMMemory->GetValue();

		if ( !wxstr.ToULong( &value, 0 ) ) {
			wxMessageBox(_T("ERROR: Unable to convert EEPROM start address to a valid number.") );
			return FALSE;
		}

		if ( value < 0x300000 ) {
			wxMessageBox(_T("ERROR: The EEPROM start address must be in the range 0x300000 - 0x3fffff.") );
			return FALSE;
		}

		if ( value > 0x3fffff ) {
			wxMessageBox(_T("ERROR: The EEPROM start address must be in the range 0x300000 - 0x3fffff.") );
			return FALSE;
		}

		gBootCtrl.m_memImage.m_minEEPROMAddr = value;
	

		// ------

		wxstr = m_endEEPROMMemory->GetValue();

		if ( !wxstr.ToULong( &value, 0 ) ) {
			wxMessageBox(_T("ERROR: Unable to convert EEPROM end address to a valid number.") );
			return FALSE;
		}

		if ( value < 0x300000 ) {
			wxMessageBox(_T("ERROR: The EEPROM end address must be in the range 0x300000 - 0x3fffff.") );
			return FALSE;
		}

		if ( value > 0x3fffff ) {
			wxMessageBox(_T("ERROR: The EEPROM end address must be in the range 0x300000 - 0x3fffff.") );
			return FALSE;
		}

		gBootCtrl.m_memImage.m_maxEEPROMAddr = value;
	}

    return TRUE;

}