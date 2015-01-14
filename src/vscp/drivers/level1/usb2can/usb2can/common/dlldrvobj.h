// dlldrvobj.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://can.sourceforge.net) 
//
// Copyright (C) 2000-2015 Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com
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

#if !defined(AFX_DLLDRVOBJ_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_)
#define AFX_DLLDRVOBJ_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_


#include "../common/usb2canobj.h"

// This is the version info for this DLL - Change to your own value
//#define DLL_VERSION		0x00010000

#define DLL_MAIN_VERSION					1
#define DLL_MINOR_VERSION					0
#define DLL_SUB_VERSION				        8	

// This is the vendor string - Change to your own value
#define CANAL_DLL_VENDOR "edevices, Lithuania, http://www.8devices.com"
							

// Max number of open connections
//#define CANAL_USB2CAN_DRIVER_MAX_OPEN	256
#define CANAL_USB2CAN_DRIVER_MAX_OPEN	1
//WARNING! CANAL_USB2CAN_DRIVER_MAX_OPEN will be a problem with using old version with smaller size and a new version with bigger size on the same pc at the same time

/////////////////////////////////////////////////////////////////////////////
// CDllDrvObj
// See loggerdll.cpp for the implementation of this class
//

class CDllDrvObj
{

public:

	/// Constructor
	CDllDrvObj();
	
	/// Destructor
	~CDllDrvObj();

	/*!
		Add a driver object

		@parm plog Object to add
		@return handle or 0 for error
	*/
	long addDriverObject( CUsb2canObj *pObj );

	/*!
		Get a driver object from its handle

		@param handle for object
		@return pointer to object or NULL if invalid
				handle.
	*/
	CUsb2canObj *getDriverObject( long h );

	/*!
		Remove a driver object

		@parm handle for object.
	*/
	void removeDriverObject( long h );


    /*
         GS
	*/
    BOOL CDllDrvObj::removeAllObjects();
	
	/*!
		The log file object
		This is the array with driver objects (max 256 objects
	*/
	CUsb2canObj *m_drvObjArray[ CANAL_USB2CAN_DRIVER_MAX_OPEN ];
	long * lpvMem;      // pointer to shared memory
	HANDLE hMapObject;  // handle to file mapping


	/// Mutex for open/close
#ifdef WIN32	
	HANDLE m_objMutex;
#else
	pthread_mutex_t m_objMutex;
#endif

	/// Counter for users of the interface
	unsigned long m_instanceCounter;

public:
	BOOL InitInstance();

};



#endif // !defined(AFX_DLLDRVOBJ_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_)
