// vscpd.h : main header file for the VSCPD application
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2009 Ake Hedman, eurosource, <akhe@eurosource.se>
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

#if !defined(AFX_VSCPD_H__3D1CDB8C_8027_46D5_9284_67750BEA4B7E__INCLUDED_)
#define AFX_VSCPD_H__3D1CDB8C_8027_46D5_9284_67750BEA4B7E__INCLUDED_

#include "../../common/controlobject.h"

// Inlcude the CANAL definitions
#include "../../common/canal.h"

/// Nams of mutex's

#define CANALD_CLIENT_OBJ_MUTEX "____CANAL_CLIENT_OBJ_MUTEX____"
#define CANALD_DEVICE_OBJ_MUTEX "____CANAL_DEVICE_OBJ_MUTEX____"
#define CANALD_SEND_OBJ_MUTEX "____CANAL_SEND_OBJ_MUTEX____"
#define CANALD_RECEIVE_OBJ_MUTEX "____CANAL_RECEIVE_OBJ_MUTEX____"
#define CANALD_CLIENT_MUTEX "__CANAL_CLIENT_MUTEX__"

/////////////////////////////////////////////////////////////////////////////
// VSCPApp:
// See vscpd.cpp for the implementation of this class
//

class VSCPApp
{
public:
	VSCPApp();

public:
	
	/*!
		Initialize the system
	*/
	BOOL init( wxString& strcfgfile );
	

// Implementation
	
private:

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSCPD_H__3D1CDB8C_8027_46D5_9284_67750BEA4B7E__INCLUDED_)
