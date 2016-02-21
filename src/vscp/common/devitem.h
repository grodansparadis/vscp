// DeviceList.h: interface for the CDeviceList class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2016 
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

#if !defined(AFX_DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_)
#define AFX_DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_

#ifdef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif


#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"

#include "vscp.h"

/*!
  Structure for registry and config file
  interaction with device parameters
*/


typedef struct _devItem {
    int id;
    int regid;
    wxString strName;
    wxString strPath;
    wxString strParameters; 
    wxString strPrefix;
    uint32_t flags;
    uint32_t filter;
    uint32_t mask;
    wxString strUsername;
    wxString strPassword;
    short port;
} devItem;

typedef  devItem  *PDEVITEM;

#endif
