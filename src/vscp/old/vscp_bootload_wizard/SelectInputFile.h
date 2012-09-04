///////////////////////////////////////////////////////////////////////////////
// SelectInputFile.h: interface for the CSelectInputFile class.
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
// $RCSfile: SelectInputFile.h,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SELECTINPUTFILE_H__AC4E0092_5C7C_4707_9D19_FB42E6C82135__INCLUDED_)
#define AFX_SELECTINPUTFILE_H__AC4E0092_5C7C_4707_9D19_FB42E6C82135__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma implementation "vscpboot.cpp"
    #pragma interface "vscpboot.cpp"
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



class wxFileSelectPage : public wxWizardPageSimple
{

public:

    wxFileSelectPage( wxWizard *parent );

	void OnWizardCancel( wxWizardEvent& event );

	void OnWizardPageChanging( wxWizardEvent& event );

	virtual bool TransferDataFromWindow();
							
private:
 
	/*!
		Path the hex file to work with
	*/
	wxString m_path;

	DECLARE_EVENT_TABLE()

};

#endif // !defined(AFX_SELECTINPUTFILE_H__AC4E0092_5C7C_4707_9D19_FB42E6C82135__INCLUDED_)
