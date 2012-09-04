///////////////////////////////////////////////////////////////////////////////
// BootLoaderPage.cpp: implementation of the CBootLoaderPage class.
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
// $RCSfile: BootLoaderPage.cpp,v $                                       
// $Date: 2005/07/24 21:20:29 $                                  
// $Author: akhe $                                              
// $Revision: 1.5 $ 
///////////////////////////////////////////////////////////////////////////////

#include "BootLoaderPage.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( wxBootLoadPage, wxWizardPageSimple )
    EVT_WIZARD_PAGE_CHANGING( -1, wxBootLoadPage::OnWizardPageChanging )
	EVT_WIZARD_FINISHED( -1, wxBootLoadPage::OnWizardPageChanging )
    EVT_WIZARD_CANCEL( -1, wxBootLoadPage::OnWizardCancel )
END_EVENT_TABLE()


wxBootLoadPage::wxBootLoadPage( wxWizard *parent )
								 : wxWizardPageSimple( parent )
{
	(void)new wxStaticText( this, 
			-1, 
			_T("When you press the next button the bootloading process will begin.\n\n")
            _T("The bootloading process can be time consuming so please be patient.") );  
	
	m_FlashInfo = new wxStaticText( this, 
				-1, _T(""), wxPoint( 5, 70 ) );

	m_ConfigInfo = new wxStaticText( this, 
				-1, _T(""),wxPoint( 5, 100 ) );

	m_EEPROMInfo = new wxStaticText( this, 
				-1, _T(""),wxPoint( 5, 130 ) );
}
 

// wizard event handlers

///////////////////////////////////////////////////////////////////////////////
// OnWizardCancel
//

void wxBootLoadPage::OnWizardCancel( wxWizardEvent& event ) 
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

void wxBootLoadPage::OnWizardPageChanging( wxWizardEvent& event )
{
	// Backwards always OK 
    if ( !event.GetDirection() ) {
		return;
	}

	// Forward OK if selected.
	if ( event.GetDirection() ) {

		if ( doSetBootMode() && doBootLoad() ) {
			return;
		}

	}

    wxMessageBox( _T("There was a problem with the bootloading!"), 
								_T("Bootloader error"),
								wxICON_WARNING | wxOK, this );

    event.Veto();

}

///////////////////////////////////////////////////////////////////////////////
// TransferDataFromWindow
//

bool wxBootLoadPage::TransferDataToWindow( void )
{
	wxString wxstr;
	
	if ( gBootCtrl.m_memImage.m_bPrgData ) {

		wxstr.Printf( _T("* Flash will be written from 0x%08X to 0x%08X."),
						gBootCtrl.m_memImage.m_minFlashAddr,
						gBootCtrl.m_memImage.m_maxFlashAddr ); 
		m_FlashInfo->SetLabel( wxstr );

	}
	else {
		m_FlashInfo->SetLabel( _T("* No flash will be written.") );	
	}

	if ( gBootCtrl.m_memImage.m_bConfigData ) {
	
		wxstr.Printf( _T("* Config data will be written from 0x%08X to 0x%08X."),
						gBootCtrl.m_memImage.m_minConfigAddr,
						gBootCtrl.m_memImage.m_maxConfigAddr );
		m_ConfigInfo->SetLabel( wxstr );	

	}
	else {
		m_ConfigInfo->SetLabel( _T("* No config data will be written.") );	
	}

	if ( gBootCtrl.m_memImage.m_bEEPROMData ) {
	
		wxstr.Printf( _T("* EEPROM data will be written from 0x%08X to 0x%08X."),
						gBootCtrl.m_memImage.m_minEEPROMAddr,
						gBootCtrl.m_memImage.m_maxEEPROMAddr );
		m_EEPROMInfo->SetLabel( wxstr );

	}
	else {
		m_EEPROMInfo->SetLabel( _T("* No EEPROM data will be written.") );	
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// TransferDataFromWindow
//

bool wxBootLoadPage::TransferDataFromWindow( void )
{

    return TRUE;

}

///////////////////////////////////////////////////////////////////////////////
// doSetBootMode
//

bool wxBootLoadPage::doSetBootMode( void )
{
	bool rv = TRUE;

	// Must be a client to serve
	if ( 0 != gBootCtrl.getClientCnt() ) {
	
		for ( int i=0; i<gBootCtrl.getClientCnt(); i++ ) {

			if ( !gBootCtrl.setDeviceInBootMode( i ) ) {
				rv = FALSE;
				break;
			}
			
		}

	}

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doBootLoad
//

bool wxBootLoadPage::doBootLoad( void )
{
	bool bRun = TRUE;
	bool rv = TRUE;
	int nBootReady;
	unsigned long checksum = 0;
	unsigned long progress = 0;
	unsigned long addr;
	wxString wxStatusStr;

	unsigned long nFlashPackets = 0;
	unsigned long nConfigPackets = 0;
	unsigned long nEEPROMPackets = 0;
	
	// Flash memory
	if ( gBootCtrl.m_memImage.m_bPrgData ) {
		nFlashPackets = ( gBootCtrl.m_memImage.m_maxFlashAddr - gBootCtrl.m_memImage.m_minFlashAddr )/8;
	
		if ( 0 != ( ( gBootCtrl.m_memImage.m_maxFlashAddr - gBootCtrl.m_memImage.m_minFlashAddr ) % 8 ) ) {
			nFlashPackets++;		
		}
	}

	// Config
	if ( gBootCtrl.m_memImage.m_bConfigData ) {
		nConfigPackets = ( gBootCtrl.m_memImage.m_maxConfigAddr - gBootCtrl.m_memImage.m_minConfigAddr )/8;
	
		if ( 0 != ( ( gBootCtrl.m_memImage.m_maxConfigAddr - gBootCtrl.m_memImage.m_minConfigAddr ) % 8 ) ) {
			nConfigPackets++;		
		}
	}

	// EEPROM
	if ( gBootCtrl.m_memImage.m_bEEPROMData ) {
		nEEPROMPackets = ( gBootCtrl.m_memImage.m_maxEEPROMAddr - gBootCtrl.m_memImage.m_minEEPROMAddr )/8;
	
		if ( 0 != ( ( gBootCtrl.m_memImage.m_maxEEPROMAddr - gBootCtrl.m_memImage.m_minEEPROMAddr ) % 8 ) ) {
			nEEPROMPackets++;		
		}
	}

	long nTotalPackets = nFlashPackets + nConfigPackets + nEEPROMPackets;
	wxProgressDialog* pDlg = new wxProgressDialog( _T("Boot loading in progress..."),
														_T("test message"),
														nTotalPackets,
														NULL,
														wxPD_AUTO_HIDE | 
														wxPD_APP_MODAL | 
														wxPD_CAN_ABORT | 
														wxPD_ELAPSED_TIME |
														wxPD_REMAINING_TIME );

	// Must be a client to serve
	if ( 0 != gBootCtrl.getClientCnt() ) {
		
		// Initialize checksum
		addr = gBootCtrl.m_memImage.m_minFlashAddr;
		if ( !gBootCtrl.writeDeviceControlRegs( addr, 
												MODE_WRT_UNLCK | MODE_AUTO_ERASE | MODE_AUTO_INC | MODE_ACK,
												CMD_RST_CHKSM, 0 ) ) {
			wxMessageBox( _T("Failed to initialize checksum at node(s).") );
			rv = FALSE;
			bRun = false;
		}

		// * * * flash memory * * *

		if ( rv ) {
			
			// Send the start block
			addr = gBootCtrl.m_memImage.m_minFlashAddr;
			if ( gBootCtrl.writeDeviceControlRegs( addr ) ) {
				
				for ( unsigned long blk = 0; ( ( blk < nFlashPackets ) && bRun); blk++ ) {
				
					wxStatusStr.Printf("Loading flash... %0X", addr );
					if (  !( bRun = pDlg->Update( progress, wxStatusStr ) ) ) {
						wxMessageBox( _T("Aborted by user.") );
						rv = FALSE;
						bRun = false;
					}

					if ( !gBootCtrl.writeDeviceMemory( &checksum ) ) {
						wxMessageBox( _T("Failed to write flash data to node(s).") );
						rv = FALSE;
						bRun = false;
					}
				
					::wxUsleep( 100 );
					progress++;
					addr += 8;
				
				}
			}
			else {
				wxMessageBox( _T("Failed to send control info for flash data to node(s).") );	
				rv = FALSE;
			}
		}

		// * * * config memory * * *

		if ( rv && gBootCtrl.m_memImage.m_bConfigData ) {

			// Send the start block
			addr = gBootCtrl.m_memImage.m_minConfigAddr;
			if ( gBootCtrl.writeDeviceControlRegs( addr ) ) {
			
				for ( unsigned long blk = 0; ( ( blk < nConfigPackets ) && bRun); blk++ ) {
				
					wxStatusStr.Printf("Loading config memory... %0X", addr );
					if (  !( bRun = pDlg->Update( progress, wxStatusStr ) ) ) {
						wxMessageBox( _T("Aborted by user.") );
						rv = FALSE;
						bRun = false;
					}

					if ( !gBootCtrl.writeDeviceMemory( &checksum ) ) {
						wxMessageBox( _T("Failed to write config data to node(s).") );
						rv = FALSE;
						bRun = false;
					}
				
					::wxUsleep( 100 );
					progress++;
					addr += 8;
				
				}
			}
			else {
				wxMessageBox( _T("Failed to send control info for config data to node(s).") );	
				rv = FALSE;
			}
		}

		// * * * EEPROM memory * * *

		if ( rv && gBootCtrl.m_memImage.m_bEEPROMData ) {

			// Send the start block
			addr = gBootCtrl.m_memImage.m_minEEPROMAddr;
			if ( gBootCtrl.writeDeviceControlRegs( addr ) ) {
			
				for ( unsigned long blk = 0; ( ( blk < nConfigPackets ) && bRun); blk++ ) {
				
					wxStatusStr.Printf("Loading EEPROM memory... %0X", addr );
					if (  !( bRun = pDlg->Update( progress, wxStatusStr ) ) ) {
						wxMessageBox( _T("Aborted by user.") );
						rv = FALSE;
						bRun = false;
					}

					if ( !gBootCtrl.writeDeviceMemory( &checksum ) ) {
						wxMessageBox( _T("Failed to write EEPROM data to node(s).") );
						rv = FALSE;
						bRun = false;
					}
				
					::wxUsleep( 100 );
					progress++;
					addr += 8;
				
				}
			}
			else {
				wxMessageBox( _T("Failed to send control info for EEPROM data to node(s).") );	
				rv = FALSE;
			}
		}

		// Check if checksum is correct and reset device(s)
		addr = gBootCtrl.m_memImage.m_minFlashAddr;
		unsigned long calc_chksum  = ( 0x10000 - ( checksum & 0xffff ) );
		if ( !gBootCtrl.writeDeviceControlRegs( addr, 
												MODE_WRT_UNLCK | MODE_AUTO_ERASE,
												CMD_CHK_RUN, 
												( calc_chksum & 0xff ), 
												( (calc_chksum >> 8 ) & 0xff ) 
											) ) {
			wxMessageBox( _T("Failed to do finalizing and restart at node(s).Possible checksum error.") );
			rv = FALSE;
			bRun = false;
		}
		else {
			
			for ( int i=0; i<10; i++ ) {

				// Do the device RESET
				gBootCtrl.writeDeviceControlRegs( 0x0000, 
													0,
													CMD_RESET, 
													0, 
													0 );
				
				// Verify that all clients got out of boot mode.
				nBootReady = 0;
				for ( int i=0; i<gBootCtrl.getClientCnt(); i++ ) {

					unsigned char val;
					if ( !gBootCtrl.readRegister( gBootCtrl.m_clientList[ i ].m_nickname,
							0, 
							&val ) ) {
						Sleep( 100 );
						continue;
					}
					nBootReady++;
				}

				// Check if we are done
				if ( nBootReady == gBootCtrl.getClientCnt() ) {
					break;
				}
			}
		}

		// Done
		progress = nTotalPackets;
		pDlg->Update( progress, wxStatusStr  );
		
	}

	pDlg->Destroy();

	// Tell user if not all nodes got out of boot mode
	if ( nBootReady != gBootCtrl.getClientCnt() ) {
		wxMessageBox( _T("At least one node is still in bootmode. Try to run the program again to finalize the bootload process!") );
	}

    return rv;
}
