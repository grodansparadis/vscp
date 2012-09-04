///////////////////////////////////////////////////////////////////////////////
// ByeByePage.h: interface for the CByeByePage class.
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
// $RCSfile: ByeByePage.h,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BYEBYEPAGE_H__D1AA6C72_A75B_4D14_B4A1_BE5EAC9D2D5D__INCLUDED_)
#define AFX_BYEBYEPAGE_H__D1AA6C72_A75B_4D14_B4A1_BE5EAC9D2D5D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma implementation "welcome.cpp"
    #pragma interface "welcome.cpp"
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

/*!
	Bootloader goodby page
	=======================

	Just some text...
*/
class wxGoodbyPage : public wxWizardPageSimple
{

public:

    wxGoodbyPage( wxWizard *parent );
							
private:
    
 
 

};



#endif // !defined(AFX_BYEBYEPAGE_H__D1AA6C72_A75B_4D14_B4A1_BE5EAC9D2D5D__INCLUDED_)
