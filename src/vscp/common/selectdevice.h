///////////////////////////////////////////////////////////////////////////////
// SelectDevice.h: interface for the CSelectDevice class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2016 
// Grodans Paradis AB, <akhe@grodansparadis.com>
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
// $RCSfile: SelectDevice.h,v $                                       
// $Date: 2005/09/25 13:37:00 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SELECTDEVICE_H__FC69EC9B_C8E4_4EBD_AEE6_1E0C277D5820__INCLUDED_)
#define AFX_SELECTDEVICE_H__FC69EC9B_C8E4_4EBD_AEE6_1E0C277D5820__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma implementation "BootHexFileInfo.cpp"
    #pragma interface "BootHexFileInfo.cpp"
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

#include "wx/progdlg.h"
#include "wx/wizard.h"

#ifndef __WXMSW__
//    #include "wiztest.xpm"
//    #include "wiztest2.xpm"
#endif


#include "../common/canalsuperwrapper.h"

class CSelectDevice  
{
public:
    
    CSelectDevice( wxListBox* plist = NULL );

    virtual ~CSelectDevice();

    /*!
        Get a numerical data value (hex or decimal) from a string
        @param szData Strng containing value in string form
        @return The converted number
    */
    unsigned long getDataValue( const char *szData );


    /*!
        Fill the device listbox with availabe devices
    */
    void fillListBox( void );


    /*!
        Load device data from the registry
    */
    void loadRegistryDeviceData( void );


    /*!
        Get selected device
    */
    void getSelectedDevice( int idx, devItem** pItem );


    /*!
        Get device profile
        @param pDev Device item structure
        @return true on success.
    */
    bool getDeviceProfile( int idx, devItem* pDev );

    /*!
        Edit device
    */
    void OnButtonEditDevice();

    /*!
        Add device
    */
    void OnButtonAddDevice() ;

    /*!
        Remove device
    */
    void OnButtonRemoveDevice();


private:

    /*!
        List with device profiles from the registry
    */
    devItem *m_deviceList[ 256 ];

    /*!
        Listbox for the devices
    */
    wxListBox* m_plistBox;
};

#endif // !defined(AFX_SELECTDEVICE_H__FC69EC9B_C8E4_4EBD_AEE6_1E0C277D5820__INCLUDED_)
