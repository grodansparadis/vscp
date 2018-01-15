// interfacelist.cpp
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
//

#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wx/defs.h"
#include "wx/app.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>
#include <wx/listimpl.cpp>

#include "interfacelist.h"

WX_DEFINE_LIST(TCPClientList);

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CInterfaceItem::CInterfaceItem( void )
{
    m_ipaddress.Hostname(_("127.0.0.1"));
    m_macaddress = _("");
    memset( m_GUID, 0, sizeof( m_GUID ) );
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
  m_tcpclientlist.DeleteContents ( true );
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
bool CInterfaceList::addInterface( wxString ip, wxString mac, wxString guid )
{
    unsigned long var;

    CInterfaceItem *pItem = new CInterfaceItem;
    if ( NULL == pItem ) return false;

    pItem->m_ipaddress.Hostname( ip );
    pItem->m_macaddress = mac;
    
    wxStringTokenizer tkz( guid , wxT(":") );
    for ( int i=0; i<16; i++ ) {
        tkz.GetNextToken().ToULong( &var, 16 );
        pItem->m_GUID[ i ] = (uint8_t)var;
        // If no tokens left no use to continue
        if ( !tkz.HasMoreTokens() ) break;
    }

    // Add the user
    m_tcpclientlist.Append( pItem );
    
    return true;
}
