// VSCP helper dll.h : main header file for the VSCP helper dll DLL
//

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

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "../../common/canalsuperwrapper.h"

/*!
	\class CVSCPhelperdllApp
	\brief Implements helper functionality from the VSCP helper
	class.
	See VSCP helper dll.cpp for the implementation of this class.
*/

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

    /*!
        VSCP Interface
    */
    CCanalSuperWrapper m_vscpif;
};
