// dlldrvobj.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 
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

#if !defined(CHELPDLLOBJ__INCLUDED_)
#define CHELPDLLOBJ__INCLUDED_


// This is the version info for this DLL - Change to your own value
#define DLL_VERSION		0x000001

// This is the vendor string - Change to your own value
#define DLL_VENDOR "Paradise of the Frog, Sweden, http://www.paradiseofthefrog.com"
							
// Max number of open connections
#define VSCP_HELPER_MAX_OPEN	256

#include "../../common/canal_macro.h"
#include "../../common/canalsuperwrapper.h"

/////////////////////////////////////////////////////////////////////////////
// CHelpDllObj
// 
//

class CHelpDllObj
{

public:

	/// Constructor
	CHelpDllObj();
	
	/// Destructor
	~CHelpDllObj();

	/*!
		Add a driver object

		@parm plog Object to add
		@return handle or 0 for error
	*/
	long addDriverObject( VscpRemoteTcpIf *pObj );

	/*!
		Get a driver object from its handle

		@param handle for object
		@return pointer to object or NULL if invalid
				handle.
	*/
	VscpRemoteTcpIf *getDriverObject( long h );

	/*!
		Remove a driver object

		@parm handle for object.
	*/
	void removeDriverObject( long h );
	
	/*!
		The log file object
		This is the array with driver objects (max 256 objects
	*/
	VscpRemoteTcpIf *m_drvObjArray[ VSCP_HELPER_MAX_OPEN ];

	/// Mutex for open/close	
    wxMutex m_mutex;

	/// Counter for users of the interface
	unsigned long m_instanceCounter;

public:
	BOOL InitInstance();

};



#endif // !defined(AFX_DLLDRVOBJ_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_)
