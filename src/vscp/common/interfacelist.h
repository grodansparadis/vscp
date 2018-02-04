// interfacelist.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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



#if !defined(INTERFACELIST__INCLUDED_)
#define INTERFACELIST__INCLUDED_

#include <wx/socket.h>

#include <vscp.h>
#include <dllist.h>

class CInterfaceItem
{

public:

    /// Constructor
    CInterfaceItem( void );

    /// Destructor
    virtual ~CInterfaceItem( void );

    /// IP Address for interface
    wxIPV4address m_ipaddress;

    /// MAC address for interface
    wxString m_macaddress;

    /// GUID for interface
    uint8_t m_GUID[16];

protected:

    /*!
        System assigned ID for interface
    */
    uint32_t m_interfaceID;

};


WX_DECLARE_LIST ( CInterfaceItem, TCPClientList );


class CInterfaceList  
{
public:

    /// Constructor
    CInterfaceList( void );

    /// Destructor
    virtual ~CInterfaceList( void );

    /*!
        Add interface
        @param ip IP address for interface.
        @param mac MAC address for interface.
        @param guid GUID for interface.
        @return true on sucess, false on failure.
    */
    bool addInterface( wxString ip, wxString mac, wxString guid );

protected:

    /*!
        List with interface items
    */
    TCPClientList m_tcpclientlist;

};



#endif


