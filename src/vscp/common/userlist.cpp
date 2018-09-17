// userlist.cpp
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
//#include "wx/wxprec.h"

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

#include <sqlite3.h>

#include <vscpdb.h>
#include <controlobject.h>
#include <vscphelper.h>
#include "userlist.h"


// Forward declarations
void vscp_md5( char *digest, const unsigned char *buf, size_t len );


///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CUserItem::CUserItem(void)
{
    m_userID = VSCP_ADD_USER_UNINITIALISED;
    
    // Accept all events 
    vscp_clearVSCPFilter( &m_filterVSCP );
    
    // No user rights
    memset( m_userRights, 0, sizeof( m_userRights ) );
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CUserItem::~CUserItem(void)
{
    m_listAllowedIPV4Remotes.clear();
    m_listAllowedIPV6Remotes.clear();
    m_listAllowedEvents.clear();
}

///////////////////////////////////////////////////////////////////////////////
// fixName
//

void CUserItem::fixName( void ) 
{
    m_user.Trim( false );
    m_user.Trim( true );
    
    // Works only for ASCII names. Should be fixed so
    // UTF8 names can be used TODO
    for ( int i=0; i<m_user.Length(); i++ ) {
        switch ( (const char)m_user[ i ] ) {
            case ';':
            case '\'':
            case '\"':
            case ',':
            case ' ':
                m_user[ i ] = '_';
                break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// setFromString
//
// name;password;fullname;filtermask;rights;remotes;events;note
//

bool CUserItem::setFromString( wxString userSettings )
{
    wxString strToken;
    wxStringTokenizer tkz( userSettings, _(";") ); 
    
    // name
    if ( tkz.HasMoreTokens() ) {
        strToken = tkz.GetNextToken();
        strToken.Trim();
        strToken.Trim(false);
        if ( strToken.Length() ) {
            setUserName( strToken );
            fixName();
        }
    }
    
    // password
    if ( tkz.HasMoreTokens() ) {
        strToken = tkz.GetNextToken();
        strToken.Trim();
        strToken.Trim(false);
        if ( strToken.Length() ) {
            setPassword( strToken );
        }
    }
    
    // fullname
    if ( tkz.HasMoreTokens() ) {
        strToken = tkz.GetNextToken();
        strToken.Trim();
        strToken.Trim(false);
        if ( strToken.Length() ) {
            setFullname( strToken );
        }
    }
    
    // filter
    if ( tkz.HasMoreTokens() ) {
        strToken = tkz.GetNextToken();
        strToken.Trim();
        strToken.Trim(false);
        if ( strToken.Length() ) {
            setFilterFromString( strToken );
        }
    }
    
    // mask
    if ( tkz.HasMoreTokens() ) {
        strToken = tkz.GetNextToken();
        setFilterFromString( strToken );
    }
    
    // rights
    if ( tkz.HasMoreTokens() ) {
        strToken = tkz.GetNextToken();
        strToken.Trim();
        strToken.Trim(false);
        if ( strToken.Length() ) {
            setUserRightsFromString( strToken );
        }
    }
        
    // remotes
    if ( tkz.HasMoreTokens() ) {
        strToken = tkz.GetNextToken();
        setAllowedRemotesFromString( strToken );
    }
    
    // events
    if ( tkz.HasMoreTokens() ) {
        strToken = tkz.GetNextToken();
        strToken.Trim();
        strToken.Trim(false);
        if ( strToken.Length() ) {
            setAllowedEventsFromString( strToken );
        }
    }    
    
    // note
    if ( tkz.HasMoreTokens() ) {
        strToken = tkz.GetNextToken();        
        strToken.Trim();
        strToken.Trim(false);
        if ( strToken.Length() ) {
            vscp_base64_wxdecode( strToken );       
            setNote( strToken );
        }
    }
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getAsString
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool CUserItem::getAsString( wxString& strUser )
{
    wxString wxstr;
    strUser.Empty();
        
    strUser += wxString::Format( _("%ld;"), getUserID() );
    strUser += getUserName();
    strUser += _(";");
    // Protect password
    wxstr = getPassword();
    for ( int i=0; i<wxstr.Length(); i++ ) {
        strUser += _("*");
    }
    //strUser += getPassword();
    strUser += _(";");
    strUser += getFullname();
    strUser += _(";");
    vscp_writeFilterToString( getFilter(), wxstr );
    strUser += wxstr;
    strUser += _(";");
    vscp_writeMaskToString( getFilter(), wxstr );
    strUser += wxstr;
    strUser += _(";");
    strUser += getUserRightsAsString();
    strUser += _(";");
    strUser += getAllowedRemotesAsString();
    strUser += _(";");
    strUser += getAllowedEventsAsString();
    strUser += _(";");
    
    wxstr = getNote();
    vscp_base64_wxencode( wxstr );
    strUser += wxstr;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setUserRightsFromString
//

bool CUserItem::setUserRightsFromString( const wxString& strRights )
{
    // Privileges
    if ( strRights.Length() ) {
        
        wxStringTokenizer tkz( strRights, wxT("/") );
        
        int idx=0;
        do {
            
            wxString str = tkz.GetNextToken();
            if ( str.IsSameAs( _("admin"), false ) ) {
                // All rights
                //memset( m_userRights, 0xff, sizeof( m_userRights ) );
                // All rights
                for (int i= 0; i<USER_PRIVILEGE_BYTES; i++ ) {
                    setUserRights( i, 0xff );
                }
            } 
            else if ( str.IsSameAs(_("user"), false) ) {
                // A standard user
                setUserRights( 0, 0x06 );
            } 
            else if ( str.IsSameAs(_("driver"), false) ) {
                // A standard driver
                setUserRights( 0, 0x0f );
            } 
            else {
                // Numerical
                unsigned long lval;
                str.ToULong( &lval );
                setUserRights( idx++, (uint8_t)lval );
            }
            
        } while ( tkz.HasMoreTokens() && ( idx < sizeof( m_userRights ) ) );
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setAllowedEventsFromString
//

bool CUserItem::setAllowedEventsFromString( const wxString& strEvents )
{
    // Privileges
    if ( strEvents.Length() ) {
        
        m_listAllowedEvents.Clear();
        
        wxStringTokenizer tkz( strEvents, wxT("/") );
        
        do {
            
            wxString str = tkz.GetNextToken();
            m_listAllowedEvents.Add( str );
            
        } while ( tkz.HasMoreTokens() );
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setAllowedRemotesFromString
//

bool CUserItem::setAllowedRemotesFromString( const wxString& strConnect )
{
    // Privileges
    if ( strConnect.Length() ) {
        
        m_listAllowedIPV4Remotes.Clear();
        m_listAllowedIPV6Remotes.Clear();
        
        wxStringTokenizer tkz( strConnect, wxT(",") );
        
        while ( tkz.HasMoreTokens() ) {
            wxString remote = tkz.GetNextToken();
            remote.Trim();
            addAllowedRemote( remote );
        }
        
        // TODO handel Ipv6
        
        /*do {
            wxString remote = tkz.GetNextToken();
            if ( !remote.IsEmpty() ) {
                remote.Trim();
                remote.Trim( false );
                if ( ( _( "*" ) == remote ) || ( _( "*.*.*.*" ) == remote ) ) {
                    // All is allowed to connect
                    clearAllowedRemoteList();
                }
                else {
                    addAllowedRemote( remote );
                }
            }
        } while ( tkz.HasMoreTokens() );*/
        
    }
        
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getUserRightsAsString
//

wxString CUserItem::getUserRightsAsString( void  )
{
    wxString strRights;
    
    for ( int i=0; i<USER_PRIVILEGE_BYTES; i++ ) {
        strRights += wxString::Format( _("%d"), m_userRights[ i ] );
        if ( i != ( USER_PRIVILEGE_BYTES - 1 )  ) {
            strRights += _("/");
        }
    }
    
    return strRights;
}

///////////////////////////////////////////////////////////////////////////////
// getAllowedEventsAsString
//

wxString CUserItem::getAllowedEventsAsString( void )
{
    wxString strAllowedEvents;
    
    for ( int i=0; i<m_listAllowedEvents.Count(); i++ ) {
        strAllowedEvents += m_listAllowedEvents[ i ];
        if ( i != (m_listAllowedEvents.Count() - 1) ) strAllowedEvents += _("/");
    }
    
    return strAllowedEvents;
}

///////////////////////////////////////////////////////////////////////////////
// getAllowedRemotesAsString
//

wxString CUserItem::getAllowedRemotesAsString( void )
{
    int i;
    wxString strAllowedRemotes;
    
    for ( i=0; i<m_listAllowedIPV4Remotes.Count(); i++ ) {
        strAllowedRemotes += m_listAllowedIPV4Remotes[ i ];
        if ( i != (m_listAllowedIPV4Remotes.Count() - 1) ) strAllowedRemotes += _("/");
    }
    
    for ( i=0; i<m_listAllowedIPV6Remotes.Count(); i++ ) {
        strAllowedRemotes += m_listAllowedIPV6Remotes[ i ];
        if ( i != ( m_listAllowedIPV6Remotes.Count() - 1 )  ) strAllowedRemotes += _("/");
    }
    
    return strAllowedRemotes;
}

///////////////////////////////////////////////////////////////////////////////
// isUserInDB
//

bool CUserItem::isUserInDB( const unsigned long userid )  
{
    bool rv = false;
    sqlite3_stmt *ppStmt; 
    char *pErrMsg = 0;
    char psql[ 512 ];
    
    sprintf( psql,
                VSCPDB_USER_CHECK_USER_ID, 
                (unsigned long)(userid - VSCP_LOCAL_USER_OFFSET) );
    
    // Database must be open
    if ( NULL != gpobj->m_db_vscp_daemon ) {
        gpobj->logMsg( _("isUserInDB: VSCP daemon database is not open.") );
        return false;
    }
    
    gpobj->m_db_vscp_configMutex.Lock();
    
    if ( SQLITE_OK != sqlite3_prepare( gpobj->m_db_vscp_daemon,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        gpobj->m_db_vscp_configMutex.Unlock();
        return false;
    }
    
    // Get the result
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        rv = true;
    }
    
    sqlite3_finalize( ppStmt );
    
    gpobj->m_db_vscp_configMutex.Unlock();
    
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// isUserInDB
//

bool CUserItem::isUserInDB(const wxString& user, long *pid )  
{
    bool rv = false;
    char *zErrMsg = 0;
    sqlite3_stmt *ppStmt;
    
    // Database must be open
    if ( NULL == gpobj->m_db_vscp_daemon ) {
        gpobj->logMsg( _("isUserInDB: VSCP daemon database is not open.\n") );
        return false;
    }
    
    // Search username
    char *sql = sqlite3_mprintf( VSCPDB_USER_CHECK_USER,
                                    (const char *)user.mbc_str() );
    
    gpobj->m_db_vscp_configMutex.Lock();
    
    if ( SQLITE_OK != sqlite3_prepare( gpobj->m_db_vscp_daemon,
                                            sql,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        gpobj->logMsg( _("isUserInDB: Failed to read user database - prepare query.\n") );
        sqlite3_free( sql );
        gpobj->m_db_vscp_configMutex.Unlock();
        return false;
    }
    
    while ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {
        rv = true;
        if ( NULL != pid ) {
            *pid = sqlite3_column_int( ppStmt,VSCPDB_ORDINAL_USER_ID );
        }
    }
    
    sqlite3_free( sql );
    sqlite3_finalize( ppStmt );
    
    gpobj->m_db_vscp_configMutex.Unlock();
    
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// saveToDatabase
//

bool CUserItem::saveToDatabase( void )
{
    char *zErrMsg = 0;
    
    // Database must be open
    if ( NULL == gpobj->m_db_vscp_daemon ) return false;
    
    // Internal records can't be saved to db
    // driver users is an example on users that are only local
    if ( m_userID < 0 ) return false;
    
    wxString strFilter, strMask, strBoth;
    strBoth = strFilter + _(",") + strMask;
    vscp_writeFilterToString( &m_filterVSCP, strFilter );
    vscp_writeMaskToString( &m_filterVSCP, strMask );
    strBoth = strFilter + _(",") + strMask;
    
    if ( 0 == m_userID ) {
 
        // Add
        char *sql = sqlite3_mprintf( VSCPDB_USER_INSERT,
                (const char *)m_user.mbc_str(),
                (const char *)m_password.mbc_str(),
                (const char *)m_fullName.mbc_str(),
                (const char *)strBoth.mbc_str(),
                (const char *)getUserRightsAsString().mbc_str(),
                (const char *)getAllowedEventsAsString().mbc_str(),
                (const char *)getAllowedRemotesAsString().mbc_str(),
                (const char *)m_note.mbc_str() );
        
        gpobj->m_db_vscp_configMutex.Lock();
        
        if ( SQLITE_OK != sqlite3_exec( gpobj->m_db_vscp_daemon,
                                            sql, NULL, NULL, &zErrMsg)) {
            sqlite3_free( sql );
            gpobj->m_db_vscp_configMutex.Unlock();
            return false;
        }

        sqlite3_free( sql );
        
        gpobj->m_db_vscp_configMutex.Unlock();
        
        // The database item now have a id which we need to read back
        return readBackIndexFromDatabase();
        
    }
    else {
        
        // Update
        char *sql = sqlite3_mprintf( VSCPDB_USER_UPDATE,
                (const char *)m_user.mbc_str(),
                (const char *)m_password.mbc_str(),
                (const char *)m_fullName.mbc_str(),
                (const char *)strBoth.mbc_str(),
                (const char *)getUserRightsAsString().mbc_str(),
                (const char *)getAllowedEventsAsString().mbc_str(),
                (const char *)getAllowedRemotesAsString().mbc_str(),
                (const char *)m_note.mbc_str(),
                m_userID ); 
        
        gpobj->m_db_vscp_configMutex.Lock();
        
        if ( SQLITE_OK != sqlite3_exec( gpobj->m_db_vscp_daemon,
                                            sql, NULL, NULL, &zErrMsg)) { 
            gpobj->logMsg( wxString::Format( _("Failed to update user database. Err=%s  SQL=%s" ), zErrMsg, sql ) );
            sqlite3_free( sql );
            gpobj->m_db_vscp_configMutex.Unlock();
            return false;
        }

        sqlite3_free( sql );
        
        gpobj->m_db_vscp_configMutex.Unlock();
        
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readBackIndexFromDatabase
//

bool CUserItem::readBackIndexFromDatabase( void )
{
    char *zErrMsg = 0;
    sqlite3_stmt *ppStmt;
    char *sql = sqlite3_mprintf( VSCPDB_USER_CHECK_USER,
                                    (const char *)m_user.mbc_str() );
    
    gpobj->m_db_vscp_configMutex.Lock();
        
    if ( SQLITE_OK != sqlite3_prepare( gpobj->m_db_vscp_daemon,
                                        sql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        gpobj->logMsg( wxString::Format( _("Failed to get index for user. SQL=%s" ), sql ) );
        sqlite3_free( sql );
        gpobj->m_db_vscp_configMutex.Unlock();
        return false;
    }
    
    if ( SQLITE_ROW  != sqlite3_step( ppStmt ) ) {
        gpobj->logMsg( wxString::Format( _("Failed to get index result  for user. SQL=%s" ), sql ) );
        sqlite3_free( sql );       
        gpobj->m_db_vscp_configMutex.Unlock();
    }
    
    // Get index (offset to local user)
    m_userID = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_USER_ID ) + VSCP_LOCAL_USER_OFFSET;
    
    sqlite3_free( sql );       
    gpobj->m_db_vscp_configMutex.Unlock();
        
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// isAllowedToConnect
//

/*bool CUserItem::isAllowedToConnect(const wxString& remote)
{
    unsigned int i;
    wxString wxstr;
    wxIPV4address ipaddr;
    if ( !ipaddr.Hostname( remote ) ) return false;

    // If empty all host allowed, This is "*.*.*.*" or "*"
    if ( m_listAllowedIPV4Remotes.IsEmpty() ) return true;

    for (i = 0; i < m_listAllowedIPV4Remotes.GetCount(); i++) {
        wxLogDebug(m_listAllowedIPV4Remotes[ i ]);
        if (m_listAllowedIPV4Remotes[ i ].IsSameAs(remote)) return true;
    }

    wxStringTokenizer tkz(ipaddr.IPAddress(), wxT("."));
    wxString ip1 = tkz.GetNextToken();
    wxString ip2 = tkz.GetNextToken();
    wxString ip3 = tkz.GetNextToken();
    wxString ip4 = tkz.GetNextToken();

    // test wildcard a.b.c.*
    wxstr.Printf(_("%s.%s.%s.*"), ip1.c_str(), ip2.c_str(), ip3.c_str());
    for (i = 0; i < m_listAllowedIPV4Remotes.GetCount(); i++) {
        if (m_listAllowedIPV4Remotes[ i ].IsSameAs(wxstr)) return true;
    }

    // test wildcard a.b.*.*
    wxstr.Printf(_("%s.%s.*.*"), ip1.c_str(), ip2.c_str());
    for (i = 0; i < m_listAllowedIPV4Remotes.GetCount(); i++) {
        if (m_listAllowedIPV4Remotes[ i ].IsSameAs(wxstr)) return true;
    }

    // test wildcard a.*.*.*
    wxstr.Printf(_("%s.*.*.*"), ip1.c_str());
    for (i = 0; i < m_listAllowedIPV4Remotes.GetCount(); i++) {
        if (m_listAllowedIPV4Remotes[ i ].IsSameAs(wxstr)) return true;
    }

    return false;
}*/

////////////////////////////////////////////////////////////////////////////////
// check_acl
//
//

int 
CUserItem::isAllowedToConnect( uint32_t remote_ip )
{
    int allowed, flag;
    uint32_t net, mask;
            
    remote_ip = htonl( remote_ip );
    
    // If the list is empty - allow all
    allowed = (0 == m_listAllowedIPV4Remotes.Count() ) ? '+' : '-';

    for ( int i = 0; i < m_listAllowedIPV4Remotes.Count(); i++ ) {
            
        flag = m_listAllowedIPV4Remotes[i].GetChar( 0 );   //vec.ptr[0];
        if ( ( flag != '+' && flag != '-') ||                  
             ( 0 == vscp_parse_ipv4_addr( m_listAllowedIPV4Remotes[i].Right( m_listAllowedIPV4Remotes[i].Length() - 1 ), 
                                            &net, &mask ) ) ) {
                return -1;
        }

        if ( net == ( remote_ip & mask ) ) {
            allowed = flag;
        }
        
    }

    return ( allowed == '+' );

}


///////////////////////////////////////////////////////////////////////////////
// isUserAllowedToSendEvent
//

bool CUserItem::isUserAllowedToSendEvent( const uint32_t vscp_class,
                                            const uint32_t vscp_type )
{
    unsigned int i;
    wxString wxstr;

    // If empty all events allowed
    if ( m_listAllowedEvents.IsEmpty() ) return true;

    // test wildcard *.*
    wxstr.Printf(_("*:*"));
    for (i = 0; i < m_listAllowedEvents.GetCount(); i++) {
        if (m_listAllowedEvents[ i ].IsSameAs(wxstr)) return true;
    }

    wxstr.Printf(_("%04X:%04X"), vscp_class, vscp_type);
    for (i = 0; i < m_listAllowedEvents.GetCount(); i++) {
        if (m_listAllowedEvents[ i ].IsSameAs(wxstr)) return true;
    }

    // test wildcard class.*
    wxstr.Printf(_("%04X:*"), vscp_class);
    for (i = 0; i < m_listAllowedEvents.GetCount(); i++) {
        if (m_listAllowedEvents[ i ].IsSameAs(wxstr)) return true;
    }

    return false;
}


//*****************************************************************************
//                              CUserList
//*****************************************************************************


///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CUserList::CUserList( void )
{
    // First local user except the super user has id 1
    m_cntLocaluser = 1;
    
    //wxLogDebug( _("Read Configuration: VSCPEnable=%s"), 
    //              ( m_bVSCPEnable ? _("true") : _("false") )  );
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CUserList::~CUserList( void )
{
    {
        VSCPGROUPHASH::iterator it;
        for (it = m_grouphashmap.begin(); it != m_grouphashmap.end(); ++it) {
            CGroupItem *pItem = it->second;
            if (NULL != pItem) delete pItem;
        }
    }
    
    m_grouphashmap.clear();
    
    {
        VSCPUSERHASH::iterator it;
        for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {
            CUserItem *pItem = it->second;
            if (NULL != pItem) delete pItem;
        }
    }

    m_userhashmap.clear();
}

///////////////////////////////////////////////////////////////////////////////
// loadUsers
//

bool CUserList::loadUsers( void )
{
    // Check if user is already in the database
    char *pErrMsg = 0;
    sqlite3_stmt *ppStmt;
    const char *psql = VSCPDB_USER_ALL;

    // Check if database is open
    if ( NULL == gpobj->m_db_vscp_daemon ) {
        gpobj->logMsg( _("loadUsers: Failed to read VSCP settings database - database not open." ) );
        return false;
    }
    
    gpobj->m_db_vscp_configMutex.Lock();
        
    if ( SQLITE_OK != sqlite3_prepare( gpobj->m_db_vscp_daemon,
                                            psql,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        gpobj->logMsg( _("loadUsers: Failed to read VSCP settings database - prepare query.") );
        gpobj->m_db_vscp_configMutex.Unlock();
        return false;
    }
    
    while ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {
        
        // New user item
        CUserItem *pItem = new CUserItem;
        if ( NULL != pItem ) {

           const unsigned char *p;
            
            // id (offset from local users)
            pItem->setUserID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_USER_ID ) +
                                VSCP_LOCAL_USER_OFFSET );
                    
            // User
            p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_USER_USERNAME );
            if ( NULL != p ) {
                pItem->setUserName( wxString::FromUTF8( (const char *)p ) );
            }
                        
            // Password
            p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_USER_PASSWORD );
            if ( NULL != p ) {
                pItem->setPassword( wxString::FromUTF8( (const char *)p ) );
            }
            
            // Fullname
            p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_USER_FULLNAME );
            if ( NULL != p ) {
                pItem->setFullname( wxString::FromUTF8( (const char *)p ) );
            }         
            
            // Event filter
            p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_USER_FILTER );
            if ( NULL != p ) {
                wxString wxstr;
                wxstr.FromUTF8( (const char *)p );
                pItem->setFilterFromString( wxstr );
            }
                  
            // Rights
            p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_USER_RIGHTS );
            if ( NULL != p ) {
                pItem->setUserRightsFromString( (const char *)p );
            }
            
            // Allowed events
            p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_USER_ALLOWED_EVENTS );
            if ( NULL != p ) {
                pItem->setAllowedEventsFromString( (const char *)p );
            }
            
            // Allowed remotes
            p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_USER_ALLOWED_REMOTES );
            if ( NULL != p ) {
                pItem->setAllowedRemotesFromString( (const char *)p );
            }
            
            // Note
            p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_USER_NOTE );
            if ( NULL != p ) {                
                pItem->setNote( (const char *)p );
            }
            
            m_userhashmap[ pItem->getUserName() ] = pItem;
            
        }
        else {
            gpobj->logMsg( _("Unable to allocate memory for new user.\n") );
        }
    
        
    }
    
    sqlite3_finalize( ppStmt );    
    gpobj->m_db_vscp_configMutex.Unlock();    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// addUser
//

bool CUserList::addSuperUser( const wxString& user,
                                const wxString& password,
                                const wxString& allowedRemotes,
                                uint32_t bFlags )
{
    char buf[ 512 ];
    char *pErrMsg = 0;
    
    // Cant add user with name that is already defined.
    if ( NULL != m_userhashmap[ user ] ) {
        return false;
    }
    
    // New user item
    CUserItem *pItem = new CUserItem; 
    if (NULL == pItem) return false;
    
    pItem->setUserID( 0 );              // Super user is always at id = 0
    
    wxString driverhash = user;
    driverhash += _(":");
    driverhash += wxString::FromUTF8( gpobj->m_web_authentication_domain );
    driverhash += _(":");
    driverhash += password;
    
    memset( buf, 0, sizeof( buf ) );
    strncpy( buf, (const char *)driverhash.mbc_str(), driverhash.Length() );

    char digest[33];
    vscp_md5( digest, (const unsigned char *)buf, strlen( buf ) );

    pItem->setPasswordDomain( wxString::FromUTF8( digest ) );  

    pItem->setUserName( user );
    pItem->fixName();
    pItem->setPassword( password );
    pItem->setFullname( _("Admin user") );
    pItem->setNote( _("Admin user") );
    pItem->setFilter( NULL );
    pItem->setUserRightsFromString(_("admin"));
    pItem->setAllowedRemotesFromString( allowedRemotes );
    
    // Add to the map
    m_userhashmap[ user ] = pItem;
        
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addUser
//

bool CUserList::addUser( const wxString& user,
                            const wxString& password,
                            const wxString& fullname,
                            const wxString& strNote,
                            const vscpEventFilter *pFilter,
                            const wxString& userRights,
                            const wxString& allowedRemotes,
                            const wxString& allowedEvents,
                            uint32_t bFlags )
{
    char buf[ 512 ];
    char *pErrMsg = 0;
           
    // Cant add user with name that is already defined.
    if ( NULL != m_userhashmap[ user ] ) {
        return false;
    }

    // Check if database is open
    if ( !( bFlags & VSCP_ADD_USER_FLAG_LOCAL ) && 
          ( NULL == gpobj->m_db_vscp_daemon ) ) {
        gpobj->logMsg( _("addUser: Failed to read VSCP settings database - database not open.") );
        return false;
    }

    // New user item
    CUserItem *pItem = new CUserItem; 
    if (NULL == pItem) return false;
    
    // Local user
    if ( VSCP_ADD_USER_FLAG_LOCAL & bFlags ) {
    
    }
    
    pItem->setUserID( m_cntLocaluser );
    m_cntLocaluser++;   // Update local user id counter        
          
    // Check if user is defined already
    if ( !( bFlags & VSCP_ADD_USER_FLAG_LOCAL  ) && 
          pItem->isUserInDB( user ) ) {
        delete pItem;
        return false;
    }
    
    // MD5 Token
    wxString driverhash = user;
    driverhash += _(":");
    driverhash += gpobj->m_web_authentication_domain;
    driverhash += _(":");
    driverhash += password;
    
    memset( buf, 0, sizeof( buf ) );
    strncpy( buf, (const char *)driverhash.mbc_str(), driverhash.Length() );

    char digest[33];
    vscp_md5( digest, (const unsigned char *)buf, strlen( buf ) );

    pItem->setPasswordDomain( wxString::FromUTF8( digest ) );  

    pItem->setUserName( user );
    pItem->fixName();
    pItem->setPassword( password );
    pItem->setFullname( fullname );
    pItem->setNote( strNote );
    pItem->setFilter( pFilter );
    pItem->setUserRightsFromString( userRights );
    pItem->setAllowedRemotesFromString( allowedRemotes );
    pItem->setAllowedEventsFromString( allowedEvents );
    
    // Add to the map
    m_userhashmap[ user ] = pItem;
    
    // Set filter filter
    if (NULL != pFilter) {
        pItem->setFilter( pFilter );
    }
     
    // Save to database
    if ( !( VSCP_ADD_USER_FLAG_LOCAL & bFlags ) ) {
        pItem->saveToDatabase();
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addUser
//
// name;password;fullname;filter;mask;rights;remotes;events;note
//

bool CUserList::addUser( const wxString& strUser, bool bUnpackNote )
{
    wxString strToken;
    wxString user;
    wxString password;
    wxString fullname;
    wxString strNote;
    vscpEventFilter filter;
    wxString userRights;
    wxString allowedRemotes;
    wxString allowedEvents;
    
    wxStringTokenizer tkz( strUser, _(";") );
    
    // user
    if ( tkz.HasMoreTokens() ) {
        user = tkz.GetNextToken();
    }
    
    // password
    if ( tkz.HasMoreTokens() ) {
        password = tkz.GetNextToken();
    }
    
    // fullname
    if ( tkz.HasMoreTokens() ) {
        fullname = tkz.GetNextToken();
    }
    
    // filter
    if ( tkz.HasMoreTokens() ) {
        vscp_readFilterFromString( &filter, tkz.GetNextToken() );
    }
    
    // mask
    if ( tkz.HasMoreTokens() ) {
        vscp_readMaskFromString( &filter, tkz.GetNextToken() );
    }
    
    // user rights
    if ( tkz.HasMoreTokens() ) {
        userRights = tkz.GetNextToken();
    }
    
    // allowed remotes
    if ( tkz.HasMoreTokens() ) {
        allowedRemotes = tkz.GetNextToken();
    }
    
    // allowed events
    if ( tkz.HasMoreTokens() ) {
        allowedEvents = tkz.GetNextToken();
    }
    
    // note
    if ( tkz.HasMoreTokens() ) {
        
        if ( bUnpackNote ) {            
            strNote = tkz.GetNextToken();
            vscp_base64_wxdecode( strNote );
        }
        else {
            strNote = tkz.GetNextToken();
        }
        
    }
            
    return addUser( user,
                        password, 
                        fullname,
                        strNote,
                        &filter,
                        userRights,
                        allowedRemotes,
                        allowedEvents );
}

///////////////////////////////////////////////////////////////////////////////
// deleteUser
//

 bool CUserList::deleteUser( const wxString& user )
 {
     char *zErrMsg = 0;
     CUserItem *pUser = getUser( user );
     if ( NULL == pUser ) return false;          
     
     // Internal users can't be deleted
     if ( pUser->getUserID() < VSCP_LOCAL_USER_OFFSET ) return false;
     
     // Check if database is open
    if ( NULL == gpobj->m_db_vscp_daemon ) {
        gpobj->logMsg( _("deleteUser: Failed to read VSCP "
                        "settings database - database not open." ) );
        return false;
    }
    
    gpobj->m_db_vscp_configMutex.Lock();
     
    char *sql = sqlite3_mprintf( VSCPDB_USER_DELETE_USERNAME,
                                    (const char *)user.mbc_str() );
    if ( SQLITE_OK != sqlite3_exec( gpobj->m_db_vscp_daemon, 
                                        sql, 
                                        NULL, 
                                        NULL, 
                                        &zErrMsg ) ) {
        sqlite3_free( sql );
        gpobj->logMsg( wxString::Format( _("Delete user: Unable to delete "
                                            "user in db. [%s] Err=%s\n"), 
                                            sql, 
                                            zErrMsg ) );
        gpobj->m_db_vscp_configMutex.Unlock();
        return false;
    }
        
    gpobj->m_db_vscp_configMutex.Unlock(); 
    sqlite3_free( sql );
    
    // Remove also from internal table
    m_userhashmap.erase( user );
    
    return true;
 }

///////////////////////////////////////////////////////////////////////////////
// getUser
//

CUserItem * CUserList::getUser( const wxString& user )
{
    return m_userhashmap[ user ];
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

CUserItem *CUserList::getUser( const long userid )
{
    VSCPUSERHASH::iterator it;
    for( it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it ) {
        wxString key = it->first;
        CUserItem *pUserItem = it->second;
        if ( userid == pUserItem->getUserID() ) {
            return pUserItem;
        }
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// validateUser
//

CUserItem * CUserList::validateUser( const wxString& user, 
                                            const wxString& password ) 
{
    CUserItem *pUserItem;

    pUserItem = m_userhashmap[ user ];
    if ( NULL == pUserItem ) return NULL;
    
    if ( !vscp_isPasswordValid( pUserItem->getPassword(), password ) ) {
        return NULL;
    }

    return pUserItem;
}

///////////////////////////////////////////////////////////////////////////////
// validateUserDomain
//

CUserItem * CUserList::validateUserDomain( const wxString& user, 
                                            const wxString& md5password ) 
{
    CUserItem *pUserItem;

    pUserItem = m_userhashmap[ user ];
    if ( NULL == pUserItem ) return NULL;

    // Check password
    if (!pUserItem->getPasswordDomain().IsSameAs( md5password ) ) return NULL;

    return pUserItem;
}


///////////////////////////////////////////////////////////////////////////////
// getUserAsString
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool CUserList::getUserAsString( CUserItem *pUserItem, wxString& strUser )
{
    wxString wxstr;
    strUser.Empty();
    
    // Check pointer
    if ( NULL == pUserItem ) return false;
    
    return pUserItem->getAsString( strUser );    
}

///////////////////////////////////////////////////////////////////////////////
// getUserAsString
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool CUserList::getUserAsString( uint32_t idx, wxString& strUser )
{
    wxString wxstr;
    uint32_t i = 0;
    
    VSCPUSERHASH::iterator it;
    for( it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it ) {
        
        if ( i == idx ) {
            wxString key = it->first;
            CUserItem *pUserItem = it->second;
            if ( getUserAsString( pUserItem, strUser ) ) {
                return true;
            }
            else {
                return false;
            }
        }
        
        i++;
        
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// getAllUsers
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool CUserList::getAllUsers( wxString& strAllusers )
{
    wxString wxstr;
    strAllusers.Empty();
    
    VSCPUSERHASH::iterator it;
    for( it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it ) {
        wxString key = it->first;
        CUserItem *pUserItem = it->second;
        if ( getUserAsString( pUserItem, wxstr ) ) {
            strAllusers += wxstr;
            strAllusers += _("\r\n");
        }
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAllUsers
//
//

bool CUserList::getAllUsers( wxArrayString& arrayUsers )
{
    wxString wxstr;
    
    VSCPUSERHASH::iterator it;
    for( it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it ) {
        wxString key = it->first;
        CUserItem *pUserItem = it->second;
        arrayUsers.Add( key );
    }
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getUserItemFromOrdinal
//
//

CUserItem *CUserList::getUserItemFromOrdinal( uint32_t idx )
{
    wxString wxstr;
    uint32_t i = 0;
    
    VSCPUSERHASH::iterator it;
    for( it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it ) {
        
        if ( i == idx ) {
            wxString key = it->first;
            CUserItem *pUserItem = it->second;
            return pUserItem;
        }
        
        i++;
        
    }
    
    return NULL;
}