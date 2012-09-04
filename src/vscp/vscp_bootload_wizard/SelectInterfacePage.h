///////////////////////////////////////////////////////////////////////////////
// SelectInterfacePage.h: interface for the CSelectInterfacePage class.
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
// $RCSfile: SelectInterfacePage.h,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SELECTINTERFACEPAGE_H__853E6017_9324_4308_8425_288B6A631BFA__INCLUDED_)
#define AFX_SELECTINTERFACEPAGE_H__853E6017_9324_4308_8425_288B6A631BFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma implementation "SelectInterfacePage.cpp"
    #pragma interface "SelectInterfacePage.cpp"
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
#include "../common/SelectDevice.h"

// -----------------------------------------------------------------------------
//                                     External stuff
// -----------------------------------------------------------------------------

extern CBootControl gBootCtrl;

/*!
	Select interface
	================

	Interface to use: CANAL or direct talk to driver.
*/
class wxInterfaceSelectPage : public wxWizardPageSimple
{

public:

    wxInterfaceSelectPage( wxWizard *parent );
	~wxInterfaceSelectPage();

	void OnWizardCancel( wxWizardEvent& event );
	void OnWizardPageChanging( wxWizardEvent& event );

	virtual bool TransferDataFromWindow();
							
private:
 
 
	/*!
		Interface selection lisbox
	*/
	wxListBox *m_pIfListBox;


	CSelectDevice* m_pSelectDevice;

	DECLARE_EVENT_TABLE()

};

#endif // !defined(AFX_SELECTINTERFACEPAGE_H__853E6017_9324_4308_8425_288B6A631BFA__INCLUDED_)
