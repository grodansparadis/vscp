// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// Copyright (C) 2005-2012 Gediminas Simanskis,8devices,<gediminas@8devices.com>
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



// Enum.h: interface for the CEnum class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_ENUM_H__58E62215_538E_463D_9E04_469803FC20D8__INCLUDED_)
#define AFX_ENUM_H__58E62215_538E_463D_9E04_469803FC20D8__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
#include <stdio.h>
#include <windows.h>
//#include "objbase.h"
#include <setupapi.h>
//#include <devguid.h>
//#include <initguid.h>
//#include <regstr.h>

#include "strsafe.h"
#include <winioctl.h>
//#include "Usbioctl.h"

#endif

// this should match CANAL_USB2CAN_DRIVER_MAX_OPEN in dlldrvobj.h!!
#define MAX_USB_DEVICES		32

///////////////////////////////////////////////////////////////////////////////////
//                           IOCTL
///////////////////////////////////////////////////////////////////////////////////

#define IOCTL_INDEX             0x0000


#define IOCTL_USBSAMP_GET_CONFIG_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     IOCTL_INDEX,     \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)
                                                   
#define IOCTL_USBSAMP_RESET_DEVICE          CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     IOCTL_INDEX + 1, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

#define IOCTL_USBSAMP_RESET_PIPE            CTL_CODE(FILE_DEVICE_UNKNOWN,     \
                                                     IOCTL_INDEX + 2, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)


#define IOCTL_USBSAMP_GET_SERIALNUMBER_STRING        CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                     IOCTL_INDEX + 3, \
                                                     METHOD_BUFFERED,         \
                                                     FILE_ANY_ACCESS)

////////////////////////////////////////////////////////////////////////////////////

class CEnum  
{
public:
	CEnum();
	virtual ~CEnum();

	BOOL GetDevicePath(
                 IN  LPGUID ,
                 __out_ecount(BufLen) PCHAR ,				 
                 __in size_t ,
				 __in_ecount(BufLen) PCHAR
                     );

private:

	HANDLE		hDev;
    CHAR		tmpBuf[253];
};

#endif // !defined(AFX_ENUM_H__58E62215_538E_463D_9E04_469803FC20D8__INCLUDED_)
