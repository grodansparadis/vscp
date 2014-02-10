// Enum.h: interface for the CEnum class.
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#define MAX_USB_DEVICES		4

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
