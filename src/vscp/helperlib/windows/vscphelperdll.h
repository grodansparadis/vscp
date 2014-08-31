// VSCP helper dll.h : main header file for the VSCP helper dll DLL
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

// How to use wxGUI from dll
// http://svn.wxwidgets.org/viewvc/wx/wxWidgets/trunk/samples/dll/



/*!
	\file "VSCP helper dll.h"	
	\mainpage VSCP helper dll.h
	\brief VSCP helper dll with the tools for VSCP program development. 
	\details This is the documentation for the VSCP helper dll. This file includes
	most of the stuff that is needed by a programmer to interface CANAL drivers and 
	the talk to a VSCP daemon remotly over tcp/ip. All helper functionality and all 
	communication functionality is included in this dynamically loadable library. 
	Also ready to work with threads for receiving and transmitting events.
	\author Ake Hedman <akhe@grodansparadis.com>, Grodans Paradis AB, Sweden
		
*/

#pragma once

#include "resource.h"		// main symbols
#include "../../common/canalsuperwrapper.h"

/*!
	\class CVSCPhelperdllApp
	\brief Implements helper functionality from the VSCP helper
	class.
	See VSCP helper dll.cpp for the implementation of this class.
*/

/*
class CVSCPhelperdllApp : public CWinApp
{
public:
	CVSCPhelperdllApp();

// Overrides
public:
	/// Init this instance of the dll
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()

public:

    // VSCP Interface
    CCanalSuperWrapper m_vscpif;
};
*/