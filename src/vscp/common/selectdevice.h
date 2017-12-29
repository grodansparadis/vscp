///////////////////////////////////////////////////////////////////////////////
// SelectDevice.h: interface for the CSelectDevice class.
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2017 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
