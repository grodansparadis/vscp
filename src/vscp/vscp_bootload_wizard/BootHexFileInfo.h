///////////////////////////////////////////////////////////////////////////////
// BootHexFileInfo.h: interface for the CBootHexFileInfo class.
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
// $RCSfile: BootHexFileInfo.h,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOOTHEXFILEINFO_H__6FD6561C_DC19_45E1_BB1A_D668C3562F28__INCLUDED_)
#define AFX_BOOTHEXFILEINFO_H__6FD6561C_DC19_45E1_BB1A_D668C3562F28__INCLUDED_

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma implementation "BootHexFileInfo.cpp"
    #pragma interface "BootHexFileInfo.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/wizard.h"

#ifndef __WXMSW__
    #include "wiztest.xpm"
    #include "wiztest2.xpm"
#endif

#include "BootControl.h"

// -----------------------------------------------------------------------------
//                                     External stuff
// -----------------------------------------------------------------------------

extern CBootControl gBootCtrl;

/*!
	 
	   .
*/

class wxBootHexFileInfoPage : public wxWizardPageSimple
{

public:

    wxBootHexFileInfoPage( wxWizard *parent );

	void setBootHandler( CBootControl * pBootCtrl );

	void OnWizardCancel( wxWizardEvent& event );

	void OnWizardPageChanging( wxWizardEvent& event );

	virtual bool TransferDataToWindow();

	virtual bool TransferDataFromWindow();
							
private:
 
	/*!
		Path the hex file to work with
	*/
	wxString m_path;


	/*!
		Text field for start of Flash code
	*/
	wxTextCtrl* m_startFlashMemory;

	/*!
		Text field for end of Flash code
	*/
	wxTextCtrl* m_endFlashMemory;

	/*!
		A checkbox for selection of flash programming
	*/
	wxCheckBox* m_pFlashCheckBox;


	/*!
		Text field for start of config data
	*/
	wxTextCtrl* m_startConfigMemory;

	/*!
		Text field for end of config data
	*/
	wxTextCtrl* m_endConfigMemory;

	/*!
		A checkbox for selection of config programming
	*/
	wxCheckBox* m_pConfigCheckBox;

	/*!
		A checkbox for selection of EEPROM programing
	*/
	wxCheckBox* m_pEEPROMCheckBox;

	/*!
		Text field for start of EEPROM data
	*/
	wxTextCtrl* m_startEEPROMMemory;

	/*!
		Text field for end of EEPROM data
	*/
	wxTextCtrl* m_endEEPROMMemory;


	DECLARE_EVENT_TABLE()

};


#endif // !defined(AFX_BOOTHEXFILEINFO_H__6FD6561C_DC19_45E1_BB1A_D668C3562F28__INCLUDED_)
