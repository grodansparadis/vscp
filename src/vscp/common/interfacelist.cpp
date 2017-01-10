// interfacelist.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://can.sourceforge.net) 
//
// Copyright (C) 2000-2017 
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
