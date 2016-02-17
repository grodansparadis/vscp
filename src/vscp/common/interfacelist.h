// interfacelist.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://can.sourceforge.net) 
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
// $RCSfile: dm.cpp,v $                                       
// $Date: 2005/08/30 11:00:04 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 



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


