///////////////////////////////////////////////////////////////////////////////
// BootLoaderPage.h: interface for the CBootLoaderPage class.
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
// $RCSfile: BootLoaderPage.h,v $                                       
// $Date: 2005/07/22 05:25:55 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOOTLOADERPAGE_H__3175D341_4CDE_40A3_8C7B_EFB4103F68FD__INCLUDED_)
#define AFX_BOOTLOADERPAGE_H__3175D341_4CDE_40A3_8C7B_EFB4103F68FD__INCLUDED_

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

#include "wx/progdlg.h"
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
	Select nickname(s) for	node(s) that should be
	bootloaded.
	   .
*/

class wxBootLoadPage : public wxWizardPageSimple
{

public:

    wxBootLoadPage( wxWizard *parent );

	void OnWizardCancel( wxWizardEvent& event );

	void OnWizardPageChanging( wxWizardEvent& event );

	virtual bool TransferDataToWindow();

	virtual bool TransferDataFromWindow();

	/*!
		Set selected devices in bootmode
	*/
	bool doSetBootMode( void );

	/*!
		Do the actual bootloading
	*/
	bool doBootLoad( void );

							
private:

	/*!
		Flash programming information
	*/
	wxStaticText* m_FlashInfo;

	/*!
		Config programming information
	*/
	wxStaticText* m_ConfigInfo;

	/*!
		EEPROM programming information
	*/
	wxStaticText* m_EEPROMInfo;

	DECLARE_EVENT_TABLE()

};
#endif // !defined(AFX_BOOTLOADERPAGE_H__3175D341_4CDE_40A3_8C7B_EFB4103F68FD__INCLUDED_)
