// interfacelist.cpp
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (C) 2000-2020 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright ncat /sys/class/net/eth0/addressotice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifdef __GNUG__
    //#pragma implementation
#endif

#include <string>
#include <deque>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "interfacelist.h"


///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CInterfaceItem::CInterfaceItem( void )
{
    m_ipaddress = "127.0.0.1";
    m_macaddress = "";
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CInterfaceItem::~CInterfaceItem( void )
{

}


//*****************************************************************************
//                          CInterfaceList
//*****************************************************************************


///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CInterfaceList::CInterfaceList( void )
{
  m_tcpclientlist.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CInterfaceList::~CInterfaceList( void )
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// addInterface
//
bool CInterfaceList::addInterface( const std::string& ip, const std::string& mac, const std::string& guid )
{
    CInterfaceItem *pItem = new CInterfaceItem;
    if ( NULL == pItem ) return false;

    pItem->m_ipaddress = ip ;
    pItem->m_macaddress = mac;
    pItem->m_guid.getFromString( guid );

    // Add the user
    m_tcpclientlist.push_back( pItem );
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// addInterface
//
bool CInterfaceList::addInterface( const std::string& ip, const std::string& mac, const cguid& guid )
{
    CInterfaceItem *pItem = new CInterfaceItem;
    if ( NULL == pItem ) return false;

    pItem->m_ipaddress = ip ;
    pItem->m_macaddress = mac;
    pItem->m_guid = guid;

    // Add the user
    m_tcpclientlist.push_back( pItem );
    
    return true;
}