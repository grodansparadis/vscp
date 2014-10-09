// userlist.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 
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

#include "vscphelper.h"
#include "userlist.h"


///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CUserItem::CUserItem(void)
{
    // Let all events thru
    vscp_clearVSCPFilter(&m_filterVSCP);
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CUserItem::~CUserItem(void)
{
    m_listAllowedRemotes.clear();
    m_listAllowedEvents.clear();
}

///////////////////////////////////////////////////////////////////////////////
// isAllowedToConnect
//

bool CUserItem::isAllowedToConnect(const wxString& remote)
{
    unsigned int i;
    wxString wxstr;
    wxIPV4address ipaddr;
    if (!ipaddr.Hostname(remote)) return false;

    // If empty all host allowed, This is "*.*.*.*"
    if ( m_listAllowedRemotes.IsEmpty() ) return true;

    for (i = 0; i < m_listAllowedRemotes.GetCount(); i++) {
        wxLogDebug(m_listAllowedRemotes[ i ]);
        if (m_listAllowedRemotes[ i ].IsSameAs(remote)) return true;
    }

    wxStringTokenizer tkz(ipaddr.IPAddress(), wxT("."));
    wxString ip1 = tkz.GetNextToken();
    wxString ip2 = tkz.GetNextToken();
    wxString ip3 = tkz.GetNextToken();
    wxString ip4 = tkz.GetNextToken();

    // test wildcard a.b.c.*
    wxstr.Printf(_("%s.%s.%s.*"), ip1.c_str(), ip2.c_str(), ip3.c_str());
    for (i = 0; i < m_listAllowedRemotes.GetCount(); i++) {
        if (m_listAllowedRemotes[ i ].IsSameAs(wxstr)) return true;
    }

    // test wildcard a.b.*.*
    wxstr.Printf(_("%s.%s.*.*"), ip1.c_str(), ip2.c_str());
    for (i = 0; i < m_listAllowedRemotes.GetCount(); i++) {
        if (m_listAllowedRemotes[ i ].IsSameAs(wxstr)) return true;
    }


    // test wildcard a.*.*.*
    wxstr.Printf(_("%s.*.*.*"), ip1.c_str());
    for (i = 0; i < m_listAllowedRemotes.GetCount(); i++) {
        if (m_listAllowedRemotes[ i ].IsSameAs(wxstr)) return true;
    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////
// isUserAllowedToSendEvent
//

bool CUserItem::isUserAllowedToSendEvent( const uint32_t vscp_class,
                                            const uint32_t vscp_type)
{
    unsigned int i;
    wxString wxstr;

    // If empty all events allowed
    if ( m_listAllowedEvents.IsEmpty() ) return true;

    // test wildcard *.*
    wxstr.Printf(_("*:*"), vscp_class);
    for (i = 0; i < m_listAllowedEvents.GetCount(); i++) {
        if (m_listAllowedEvents[ i ].IsSameAs(wxstr)) return true;
    }

    wxstr.Printf(_("%08X:%08X"), vscp_class, vscp_type);
    for (i = 0; i < m_listAllowedEvents.GetCount(); i++) {
        if (m_listAllowedEvents[ i ].IsSameAs(wxstr)) return true;
    }

    // test wildcard class.*
    wxstr.Printf(_("%08X:*"), vscp_class);
    for (i = 0; i < m_listAllowedEvents.GetCount(); i++) {
        if (m_listAllowedEvents[ i ].IsSameAs(wxstr)) return true;
    }

    return false;
}


//*****************************************************************************
//								CUserList
//*****************************************************************************


///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CUserList::CUserList(void)
{
    //wxLogDebug( _("Read Configuration: VSCPEnable=%s"), ( m_bVSCPEnable ? _("true") : _("false") )  );
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CUserList::~CUserList(void)
{
    VSCPUSERHASH::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {
        CUserItem *pItem = it->second;
        if (NULL != pItem) delete pItem;
    }

    m_userhashmap.clear();
}

///////////////////////////////////////////////////////////////////////////////
// addUser
//

bool CUserList::addUser(const wxString& user,
                            const wxString& md5,
                            const wxString& userRights,
                            const vscpEventFilter *pFilter,
                            const wxString& allowedRemotes,
                            const wxString& allowedEvents)
{
    CUserItem *pItem = new CUserItem;
    if (NULL == pItem) return false;

    pItem->m_user = user;
    pItem->m_md5Password = md5;

    wxLogDebug(_("Adding user: ") + user);

    m_userhashmap[ user ] = pItem;

    // Privileges
    if (userRights.Length()) {
        wxStringTokenizer tkz(userRights, wxT(","));
        do {
            wxString str = tkz.GetNextToken();
            if (str.IsSameAs(_("admin"), false)) {
                pItem->m_userRights = 0xffffffff;
            } else if (str.IsSameAs(_("user"), false)) {
                pItem->m_userRights = 0x00000006;
            } else if (str.IsSameAs(_("driver"), false)) {
                pItem->m_userRights = 0x0000000f;
            } else {
                // Numerical
                str.ToULong(&pItem->m_userRights);
            }
        } while (tkz.HasMoreTokens());
    }

    // Allow remotes
    if (allowedRemotes.Length()) {
        wxStringTokenizer tkz(allowedRemotes, wxT(","));
        do {
            wxString remote = tkz.GetNextToken();
            if (!remote.IsEmpty()) {
                pItem->m_listAllowedRemotes.Add(remote);

            }
        } while (tkz.HasMoreTokens());
    }

    // Allow Events
    if (allowedEvents.Length()) {
        wxStringTokenizer tkz(allowedEvents, wxT(","));
        do {
            wxString eventpair = tkz.GetNextToken();
            if (!eventpair.IsEmpty()) {
                eventpair.Trim();
                eventpair.Trim(false);
                pItem->m_listAllowedEvents.Add(eventpair);
            }
        } while (tkz.HasMoreTokens());
    } 

    // Clear filter
    if (NULL != pFilter) {
        memcpy(&pItem->m_filterVSCP, pFilter, sizeof( vscpEventFilter));
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

CUserItem * CUserList::getUser( const wxString& user )
{
    int i = 1;
    i = i + 2;

    VSCPUSERHASH::iterator it;

    if ((it = m_userhashmap.find(user)) ==
            m_userhashmap.end()) return NULL;

    return it->second;
}

///////////////////////////////////////////////////////////////////////////////
// checkUser
//

CUserItem * CUserList::checkUser(const wxString& user, const wxString& md5password)
{
    int i = 1;
    i = i + 2;

    VSCPUSERHASH::iterator it;


    if ((it = m_userhashmap.find(user)) ==
            m_userhashmap.end()) return NULL;

    // Check pointer to UserItem
    if (NULL == it->second) return NULL;

    // Check password
    if (!it->second->m_md5Password.IsSameAs(md5password)) return NULL;

    return it->second;
}




