// selectifdlg.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (http://www.vscp.org) 
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
// $RCSfile: selectifdlg.cpp,v $                                       
// $Date: 2005/09/15 19:34:50 $                                  
// $Author: akhe $                                              
// $Revision: 1.12 $ 

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/colordlg.h"
#include "wx/filedlg.h"
#include "wx/dirdlg.h"
#include "wx/fontdlg.h"
#include "wx/choicdlg.h" 
#include "wx/tipdlg.h"
#include "wx/progdlg.h"
#include "wx/fdrepdlg.h"
#include "wx/busyinfo.h"
#include <wx/tokenzr.h>
#include <wx/stdpaths.h>

#define wxTEST_GENERIC_DIALOGS_IN_MSW 0

#if defined(__WXMSW__) && wxTEST_GENERIC_DIALOGS_IN_MSW
#include "wx/generic/colrdlgg.h"
#include "wx/generic/fontdlgg.h"
#endif

#define wxUSE_DIRDLGG 0

#if !defined(__WXMSW__) || defined(wxUSE_DIRDLGG) && wxUSE_DIRDLGG
#include "wx/generic/dirdlgg.h"
#endif

#include <wx/valgen.h>
#include <wx/config.h>
#include "wx/wfstream.h"
#include "wx/fileconf.h"
#include "wx/wxchar.h"
#include "wx/string.h"

#include "cw.h"
#include "../../common/canal.h"
#include "msglistwnd.h"
#include "selectifdlg.h"
#include "editifdlg.h"

// Configuration stuff
#include <wx/config.h>


#ifdef WIN32

#else
#include "stdio.h"
#include "string.h"
#endif


#ifndef MAX_PATH
#define MAX_PATH 255
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

	BEGIN_EVENT_TABLE(CSelectIfDlg, wxDialog)
	
	EVT_BUTTON( wxID_OK, CSelectIfDlg::OnOK )
	EVT_BUTTON( wxID_CANCEL, CSelectIfDlg::OnCancel )
	
	EVT_BUTTON( ButtonAddIf, CSelectIfDlg::OnAddIf )
	EVT_BUTTON( ButtonEditIf, CSelectIfDlg::OnEditIf )
	EVT_BUTTON( ButtonRemoveIf, CSelectIfDlg::OnRemoveIf )

	EVT_LISTBOX_DCLICK(IfList, CSelectIfDlg::OnOK)
	// We have to implement this to force closing
	// the dialog when the 'x' system button is pressed
	//EVT_CLOSE( CSelectIfDlg::OnCancel)
	END_EVENT_TABLE()


///////////////////////////////////////////////////////////////////////////////
// CSelectIfDlg
//

CSelectIfDlg::CSelectIfDlg( wxWindow *parent )
		: wxDialog( parent, 
					-1, 
					_("New CANAL interface"), 
					wxDefaultPosition,
					wxSize( 500, 300 ), 
					wxDEFAULT_DIALOG_STYLE )
{
	// Initialize device list
	for ( int i =0; i < MAX_DRIVERS ; i++ ) {
		m_deviceList[ i ] = NULL;
	}
	
	// No selected device
	m_sel_pdev = NULL;

	// Construct Buttons
	m_pOKButton = new wxButton( this, wxID_OK, _("OK"), wxPoint( 410, 10 ) );
	m_pCancelButton = new wxButton( this, wxID_CANCEL, _("Cancel"), wxPoint( 410, 40 ) );

	m_pButtonAddIf = new wxButton( this, ButtonAddIf, _("Add.."), wxPoint( 410, 100 ) );
	m_pButtonEditIf = new wxButton( this, ButtonEditIf, _("Edit..."), wxPoint( 410, 130 ) );
	m_pButtonRemoveIf = new wxButton( this, ButtonRemoveIf, _("Remove"), wxPoint( 410, 160 ) );

	// * * * Interface List box * * *
	m_pinterfaceList = new wxListBox( this,
										IfList,
										wxPoint( 5, 5 ),
										wxSize( 400, 265 ) );
							
	fillIfListBox();

}

///////////////////////////////////////////////////////////////////////////////
// ~CSelectIfDlg
//

CSelectIfDlg::~CSelectIfDlg()
{
	// Clean up list
	for ( int i =0; i < MAX_DRIVERS ; i++ ) {
		if ( NULL != m_deviceList[ i ] ) delete m_deviceList[ i ];
	}
}

///////////////////////////////////////////////////////////////////////////////
// OnSave
//

void CSelectIfDlg::OnOK( wxCommandEvent& event )
{
	if ( -1 == m_pinterfaceList->GetSelection() ) {
		wxMessageBox( _("Must make a selection.") );
		return;
	}
	
	if ( ( 0 == m_pinterfaceList->GetSelection() ) || 
				wxFileExists( getSelectedDevice()->strPath ) ) {
#if !wxCHECK_VERSION(2,8,0)
		wxDialog::OnOK( event );
#else
		wxDialog::EndModal( wxID_OK );
#endif
	}
	else {
		wxMessageBox( _("Warning the driver in the entered path does not exist") );
	}
}

///////////////////////////////////////////////////////////////////////////////
// OnQuit
//

void CSelectIfDlg::OnCancel( wxCommandEvent& event )
{
#if !wxCHECK_VERSION(2,8,0)
	wxDialog::OnCancel( event );
#else
	wxDialog::EndModal( 0 );
#endif
}


///////////////////////////////////////////////////////////////////////////////
// OnAddIf
//

void CSelectIfDlg::OnAddIf( wxCommandEvent& event )
{
	int FreeArrayIdx = 0;

	CEditIfDlg dlg( this );

	dlg.m_wxStrDriverName = _("");
	dlg.m_wxStrDriverPath = _("");
	dlg.m_wxStrDriverDeviceString = _("");
	dlg.m_wxStrDriverFlags = _("0");
	dlg.m_wxStrDriverFilter = _("0");
	dlg.m_wxStrDriverMask = _("0");
	dlg.m_wxStrDriverInBufSize = _("64");
	dlg.m_wxStrDriverOutBufSize = _("64");

	if ( wxID_OK == dlg.ShowModal() ) {

		// Find unused device devicelist entry
		// highLbIdx - free pos in deviceList
		// highRegIdx - next device id "device1"
		bool bfoundDevId = false;
		for ( int i=0; i<MAX_DRIVERS; i++ ) {
			if ( !bfoundDevId && ( NULL == m_deviceList[ i ] ) ) {
				FreeArrayIdx = i;
				bfoundDevId = true;
				break;
			}
		}

		// Check if a free index was found
		if ( !bfoundDevId ) {
			wxMessageBox( _("Failure! No room for another device. ") );	
			return;
		}

		devItem *pDev = new devItem;

		if ( NULL != pDev ) {
		

			pDev->id = FreeArrayIdx;	// Array position
			pDev->regid = -1;			    // Listbox index unknown yet
			m_deviceList[ FreeArrayIdx ] = pDev;
			
			if ( !wxFileExists( dlg.m_wxStrDriverPath ) ) {
				wxMessageBox( _("Warning! The driver in the entered path does not exist") );
			}			
			
			// Get Data from dlg
			pDev->strName = dlg.m_wxStrDriverName;
			pDev->strPath = dlg.m_wxStrDriverPath;
			pDev->strParameters = replaceBackslash( dlg.m_wxStrDriverDeviceString );

      if ( 0 == pDev->strParameters.Length() ) {
        pDev->strParameters = _("NULL");
      }
			
      unsigned long val;
			dlg.m_wxStrDriverFlags.ToULong( &val );
      pDev->flags = val;
			dlg.m_wxStrDriverFilter.ToULong( &val );
      pDev->filter = val;
			dlg.m_wxStrDriverMask.ToULong( &val );
      pDev->mask = val;

			// Fix listbox
			m_pinterfaceList->Append( pDev->strName, pDev );
			m_pinterfaceList->SetSelection( pDev->id + 1 );
 
			// Fix registry
			wxString strBuf;
      wxString strKey;

      strBuf = pDev->strName;
      strBuf += _(",");
      strBuf = pDev->strParameters;
      strBuf += _(",");
      strBuf = pDev->strPath;
      strBuf += _(",");
      wxString strBuild;
			strBuild.Printf(  _("%u,%u,%u"), 
						              pDev->flags,  
						              pDev->filter,
						              pDev->mask );
      strBuf += strBuild;
		
			strKey.Printf( _("device%i"), pDev->id );

			wxString str;
      wxStandardPaths strpath;
  
      wxString strcfgfile = strpath.GetUserDataDir();
      strcfgfile += _("/canalworks.conf");
			
			wxFFileInputStream cfgstream( strcfgfile );

			if ( !cfgstream.Ok() ) {
				wxMessageBox(_("Failed to open configuration file!"));
				return;
			}

			wxFileConfig *pconfig = new wxFileConfig( cfgstream );
 

			pconfig->SetPath(_("/DRIVERS"));
			pconfig->Write( strKey, strBuf );
			wxFFileOutputStream cfgOutstream( strcfgfile );
			if ( cfgOutstream.Ok() ) {
				// Save changes
				pconfig->Save( cfgOutstream );
			}
			
			if ( NULL!= pconfig ) delete pconfig;

		}
		else {
			wxMessageBox(_("Unable to add new data. Possible memory problem!"));
		}
	}		
}

///////////////////////////////////////////////////////////////////////////////
// OnEditIf
//

void CSelectIfDlg::OnEditIf( wxCommandEvent& event )
{
	devItem *pdev = NULL;

	int idx = m_pinterfaceList->GetSelection();

	if ( 0 == idx ) {
		wxMessageBox(_("Noting to edit for this built in interface!"));
	}
	else if ( -1 == idx ) {
		wxMessageBox(_("First select interface to edit!"));
	}
	else if ( NULL == ( pdev = (devItem *)m_pinterfaceList->GetClientData( idx ) ) ) {
		wxMessageBox(_("Invalid device object!"));
	}
	else {
	
		CEditIfDlg dlg( this );

		dlg.m_wxStrDriverName = pdev->strName;
		dlg.m_wxStrDriverPath = pdev->strPath;
		dlg.m_wxStrDriverDeviceString = pdev->strParameters;
		dlg.m_wxStrDriverFlags.Printf(_("%lu"), pdev->flags );
		dlg.m_wxStrDriverFilter.Printf(_("%lu"), pdev->filter );
		dlg.m_wxStrDriverMask.Printf(_("%lu"), pdev->mask );
	
		if ( wxID_OK == dlg.ShowModal() ) {
	
			if ( !wxFileExists( dlg.m_wxStrDriverPath ) ) {
				wxMessageBox( _("Warning the driver in the entered path does not exist") );
			}
			
			// Get Data from dlg
			pdev->strName = replaceBackslash( dlg.m_wxStrDriverName );
      pdev->strPath = replaceBackslash( dlg.m_wxStrDriverPath );
      pdev->strParameters = replaceBackslash( dlg.m_wxStrDriverDeviceString );
      
      if ( 0 == pdev->strParameters.Length() ) {
        pdev->strParameters = _("NULL");
      }
			
      unsigned long val;
			dlg.m_wxStrDriverFlags.ToULong( &val  );
      pdev->flags = val;
			dlg.m_wxStrDriverFilter.ToULong( &val );
      pdev->filter = val;
			dlg.m_wxStrDriverMask.ToULong( &val );
      pdev->mask = val;

			// Fix listbox
			m_pinterfaceList->Delete( idx );
			m_pinterfaceList->Insert( pdev->strName, idx, pdev );
			m_pinterfaceList->SetSelection( idx );

 
			// Fix configuration file content
			wxString strKey;
			wxString strBuf;
      strBuf = pdev->strName;
      strBuf += _(",");
      strBuf += pdev->strParameters;
      strBuf += _(",");
      strBuf += pdev->strPath;
      wxString strBuild;
			strBuild.Printf( _(",%u,%u,%u"), 
					              pdev->flags,
					              pdev->filter,
					              pdev->mask );  
      strBuf += strBuild;
			
			strKey.Printf( _("device%i"), pdev->id );
			wxString str;
      wxStandardPaths strpath;
  
      wxString strcfgfile = strpath.GetUserDataDir();
      strcfgfile += _("/canalworks.conf");
			
			wxFFileInputStream cfgstream( strcfgfile );

			if ( !cfgstream.Ok() ) {
				wxMessageBox(_("Failed to open configuration file!"));
				return;
			}

			wxFileConfig *pconfig = new wxFileConfig( cfgstream );

			pconfig->SetPath(_("/DRIVERS"));
			pconfig->Write( strKey, strBuf );
			wxFFileOutputStream cfgOutstream( strcfgfile );
			if ( cfgOutstream.Ok() ) {
				// Save changes
				pconfig->Save( cfgOutstream );
			}
			
			if ( NULL!= pconfig ) delete pconfig;

		}  
	}
}

///////////////////////////////////////////////////////////////////////////////
// OnRemoveIf
//

void CSelectIfDlg::OnRemoveIf( wxCommandEvent& event )
{
	devItem *pdev;
	int idx = m_pinterfaceList->GetSelection();

	if ( 0 == idx ) {
		wxMessageBox(_("This item can not be removed!"));
	}
	else if ( -1 == idx ) {
		wxMessageBox(_("First select interface to remove!"));
	}
	else if ( NULL == ( pdev = (devItem *)m_pinterfaceList->GetClientData( idx ) ) ) {
		wxMessageBox(_("Invalid device object!"));
	}
	else {
		
		if ( wxYES == wxMessageBox(_("Are you sure that this item should be deleted?"), 
									              _("Delete?"),
                                wxYES_NO ) ) {			
			
			m_deviceList[ pdev->id ] = NULL;	
			m_pinterfaceList->Delete( idx );

			if ( NULL != pdev ) {

				wxString strKey;
				
				strKey.Printf( _("device%i"), pdev->id );

        wxStandardPaths strpath;
  
        wxString strcfgfile = strpath.GetUserDataDir();
        strcfgfile += _("/canalworks.conf");
			
			  wxFFileInputStream cfgstream( strcfgfile );

				if ( !cfgstream.Ok() ) {
					wxMessageBox(_("Failed to open configuration file!"));
					return;
				}

				wxFileConfig *pconfig = new wxFileConfig( cfgstream );

				pconfig->SetPath(_("/DRIVERS"));
				pconfig->DeleteEntry( strKey );
				wxFFileOutputStream cfgOutstream( strcfgfile );
				if ( cfgOutstream.Ok() ) {
					// Save changes
					pconfig->Save( cfgOutstream );
				}

				if ( NULL!= pconfig ) delete pconfig;

			}

			delete pdev;

		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// fillIfListBox
//

void CSelectIfDlg::fillIfListBox( void )
{
	int i;
	for ( i = 0; i< MAX_DRIVERS; i++ ) {
		m_deviceList[ i ] = NULL;	
	}
  
	m_pinterfaceList->Append( _("canal daemon interface"), (void *)NULL );
	m_pinterfaceList->SetSelection( 0 );
  
	loadDeviceConfigurationData();
  
	for ( i = 0; i<MAX_DRIVERS; i++ ) {
		if ( NULL != m_deviceList[ i ] ) {
			m_deviceList[ i ]->regid = 
					m_pinterfaceList->Append( m_deviceList[ i ]->strName, m_deviceList[ i ] );	
		}
	}
  
}


/////////////////////////////////////////////////////////////////////////////
// loadDeviceConfigurationData
//
//

void CSelectIfDlg::loadDeviceConfigurationData()	
{
	int idx;
	wxString strDeviceKey;
	wxString strBuf;
	wxString str;

  wxStandardPaths strpath;
  
  wxString strcfgfile = strpath.GetUserDataDir();
  strcfgfile += _("/canalworks.conf");
			
  wxFFileInputStream cfgstream( strcfgfile );

	if ( !cfgstream.Ok() ) {
		wxMessageBox(_("Failed to open configuration file!"));
		return;
	}

	wxFileConfig *pconfig = new wxFileConfig( cfgstream );

	pconfig->SetPath(_("/DRIVERS"));
					
	// For all drivers
	for( idx=0; idx<MAX_DRIVERS; idx++ ) {

		strDeviceKey.Printf( _("device%i"), idx );
		if ( pconfig->Read( strDeviceKey, &str ) ) {

			unsigned long val;
			uint32_t inBufSize = 0;
			uint32_t outBufSize = 0;
			uint32_t flags = 0;
			uint32_t filter = 0;
			uint32_t mask = 0;

			//strBuf = str;

      wxStringTokenizer tkz( str, _(",") );

      // Get Devicename			
      wxString strName = tkz.GetNextToken();
			if ( strName.IsEmpty() ) continue;

			// Get device configuration
      wxString strParameters = tkz.GetNextToken();
			if ( 0 == strParameters.CmpNoCase(_("NULL")) ) {
        strParameters.Empty();
      }

			// Get Device Path
      wxString strPath = tkz.GetNextToken();
      if ( strPath.IsEmpty() ) continue;

			// inbufsize
      wxString strInBufSize = tkz.GetNextToken();
      if ( !strInBufSize.IsEmpty() ) {
        if ( strInBufSize.ToULong( &val ) ) {
          inBufSize = val;
        }
      } 
					
			// outbufsize
      wxString strOutBufSize = tkz.GetNextToken();
      if ( !strOutBufSize.IsEmpty() ) {
        if ( strOutBufSize.ToULong( &val ) ) {
          outBufSize = val;
        }
      }

					
			// flags
      wxString strFlags = tkz.GetNextToken();
      if ( !strFlags.IsEmpty() ) {
        if ( strFlags.ToULong( &val ) ) {
          flags = val;
        }
      }

			// filter
      wxString strFilter = tkz.GetNextToken();
      if ( !strFilter.IsEmpty() ) {
        if ( strFilter.ToULong( &val ) ) {
          filter = val;
        }
      }

			// mask
      wxString strMask = tkz.GetNextToken();
      if ( !strMask.IsEmpty() ) {
        if ( strMask.ToULong( &val ) ) {
          mask = val;
        }
      }

			// Add the device
			devItem *pDev = new devItem;
			if ( NULL != pDev ) {
						
				pDev->id = idx;		
				pDev->regid = idx; 
        pDev->strName = strName;
        pDev->strParameters = strParameters;
        pDev->strPath = strPath; 
				pDev->flags = flags;
				m_deviceList[ idx ] = pDev;
			}				
		}
	}

	if ( NULL!= pconfig ) delete pconfig;

}



/////////////////////////////////////////////////////////////////////////////
// getSelectedDevice
//

devItem *CSelectIfDlg::getSelectedDevice( void )
{
	if ( -1 == m_pinterfaceList->GetSelection() ) {
		return NULL;
	}
	else if ( 0 == m_pinterfaceList->GetSelection() ) {
		return NULL;		
	}
	else {
		return m_deviceList[ m_pinterfaceList->GetSelection() - 1 ];	
	}
}

/////////////////////////////////////////////////////////////////////////////
// getDeviceProfile

bool CSelectIfDlg::getDeviceProfile( devItem *pDev )
{
	bool rv = true;

	// Must be a valid device pointer
	if ( NULL == pDev ) return false;
	
	// Must be a selected item
	if ( -1 == m_pinterfaceList->GetSelection() ) {
		return false;
	}
	else if ( 0 == m_pinterfaceList->GetSelection() ) {
		pDev->id = 0;		
	}
	else {
		memcpy( pDev, 
					    m_deviceList[ m_pinterfaceList->GetSelection() ], 
					    sizeof( devItem ) );	
	}
	
	return rv;
}


//////////////////////////////////////////////////////////////////////////////
// getDataValue
//

uint32_t CSelectIfDlg::getDataValue( const char *szData )
{
	uint32_t val;
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

//////////////////////////////////////////////////////////////////////////////
// replaceBackslash 
//

wxString CSelectIfDlg::replaceBackslash( wxString& wxstr )
{
	int pos;
	while ( -1 != ( pos = wxstr.Find('\\') ) ) {
		wxstr[ pos ] = '/';
	}
	return wxstr;
}
