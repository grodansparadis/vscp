// vscpvariable.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
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


#ifdef WIN32
#include <winsock2.h>
#endif

#include "wx/wxprec.h"
//#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/app.h>
#include <wx/datetime.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/stdpaths.h>
#include <wx/base64.h>

#include <mongoose.h>

#include <wx/listimpl.cpp>

#ifndef VSCP_DISABLE_LUA
#include <lua.hpp>
#endif

#include <vscp.h>
#include <vscpdb.h>
#include <version.h>
#include <controlobject.h>
#include <vscphelper.h>
#include <guid.h>
#include <variablecodes.h>
#include <vscpvariable.h>

// The global control object
extern CControlObject *gpctrlObj;

WX_DEFINE_LIST( listVscpVariable );

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CVSCPVariable::CVSCPVariable( void )
{
    m_id = 0;
    m_name.Empty();
    m_type = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    m_bPersistent = false;                  // Not persistent by default
    m_accessRights = PERMISSON_ALL_RIGHTS;  // Read/Write/execute for all
    m_userid = USER_ADMIN;                  // Admin owns variable by default
    m_groupid = GROUP_ADMIN;                // Group is admin-group by default
    
    m_strValue.Empty();
    m_note.Empty();

    m_boolValue = false;
    m_event.pdata = NULL;
    m_longValue = 0;
    m_floatValue = 0;
    m_normIntSize = 0;
    memset( m_normInteger, 0, sizeof( m_normInteger ) );
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPVariable::~CVSCPVariable( void )
{
    // Clean up VSCP event data
    if ( NULL != m_event.pdata ) delete m_event.pdata;
}

///////////////////////////////////////////////////////////////////////////////
// fixName
//

void CVSCPVariable::fixName( void )
{
    m_name.MakeUpper();
    
    // Works only for ASCII names. Should be fixed so
    // UTF8 names can be used TODO
    for ( int i=0; i<m_name.Length(); i++ ) {
        switch ( (const char)m_name[ i ] ) {
            case ';':
            case '\'':
            case '\"':
            case ',':
            case ' ':
                m_name[ i ] = '_';
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// getVariableType
//

uint16_t CVSCPVariable::getVariableTypeFromString( const wxString& strVariableType )
{
    uint16_t type = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    wxString str = strVariableType;
    long val;

    if ( str.IsNumber() ) {
        str.ToLong( &val, 10 );
        type = val;
    }
    else {
        str.Trim();
        str.Trim( false );
        str = str.Upper();

        if ( 0 == str.Find( _("STRING") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_STRING;
        }
        else if ( 0 == str.Find( _("BOOL") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_BOOLEAN;
        }
        else if ( 0 == str.Find( _("BOOLEAN") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_BOOLEAN;
        }
        else if ( 0 == str.Find( _("INT") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_INTEGER;
        }
        else if ( 0 == str.Find( _("INTEGER") )  ) {
            type = VSCP_DAEMON_VARIABLE_CODE_INTEGER;
        }
        else if ( 0 == str.Find( _("LONG") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_LONG;
        }
        else if ( 0 == str.Find( _("FLOAT") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_DOUBLE;
        }
        else if ( 0 == str.Find( _("DOUBLE") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_DOUBLE;
        }
        else if ( 0 == str.Find( _("MEASUREMENT") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT;
        }         
        else if ( 0 == str.Find( _("EVENTGUID") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID;
        }
        else if ( 0 == str.Find( _("GUID") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID;
        }        
        else if ( 0 == str.Find( _("EVENTDATA") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA;
        }
        else if ( 0 == str.Find( _("DATA") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA;
        }
        else if ( 0 == str.Find( _("VSCPCLASS") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS;
        }
        else if ( 0 == str.Find( _("EVENTCLASS") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS;
        }
        else if ( 0 == str.Find( _("VSCPTYPE") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE;
        }
        else if ( 0 == str.Find( _("EVENTTYPE") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE;
        }
        else if ( 0 == str.Find( _("EVENTTIMESTAMP") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP;
        }
        else if ( 0 == str.Find( _("TIMESTAMP") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP;
        }		
        else if ( 0 == str.Find( _("DATETIME") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_DATETIME;
        }
        else if ( 0 == str.Find( _("DATE") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_DATE;
        }
        else if ( 0 == str.Find( _("TIME") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_TIME;
        }
        else if ( 0 == str.Find( _("BASE64") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_BASE64;
        }
        else if ( 0 == str.Find( _("MIME") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_MIME;
        }
        else if ( 0 == str.Find( _("HTML") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_HTML;
        }
        else if ( 0 == str.Find( _("JAVASCRIPT") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT;
        }
        else if ( 0 == str.Find( _("JSON") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_JSON;
        }
        else if ( 0 == str.Find( _("XML") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_XML;
        }
        else if ( 0 == str.Find( _("SQL") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_SQL;
        }
        else if ( 0 == str.Find( _("LUA") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_LUA;
        }
        else if ( 0 == str.Find( _("LUARES") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT;
        }
        else if ( 0 == str.Find( _("UX1") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1;
        }
        else {
            type = vscp_readStringValue( str );
        }
    }

    return type;

}

///////////////////////////////////////////////////////////////////////////////
// getVariableTypeAsString
//

const char * CVSCPVariable::getVariableTypeAsString( int type )
{
    switch ( type ) {

        case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
            return "Unassigned";

        case VSCP_DAEMON_VARIABLE_CODE_STRING:
            return "String";

        case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
            return "Boolean";

        case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            return "Integer";

        case VSCP_DAEMON_VARIABLE_CODE_LONG:
            return "Long";

        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
            return "Double";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
            return "Measurement";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
            return "VscpEvent";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
            return "VscpGuid";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
            return "VscpData";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
            return "VscpClass";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
            return "VscpType";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
            return "Timestamp";

        case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
            return "DateTime";
            
        case VSCP_DAEMON_VARIABLE_CODE_BASE64:
            return "BASE64";

        case VSCP_DAEMON_VARIABLE_CODE_DATE:
            return "Date";
            
        case VSCP_DAEMON_VARIABLE_CODE_TIME:
            return "Time";    
        
        case VSCP_DAEMON_VARIABLE_CODE_MIME:
            return "MIME";
            
        case VSCP_DAEMON_VARIABLE_CODE_HTML:
            return "HTML";

        case VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT:
            return "Javascript";
            
        case VSCP_DAEMON_VARIABLE_CODE_JSON:
            return "JSON";
            
        case VSCP_DAEMON_VARIABLE_CODE_XML:
            return "XML";
            
        case VSCP_DAEMON_VARIABLE_CODE_SQL:
            return "SQL";
            
        case VSCP_DAEMON_VARIABLE_CODE_LUA:
            return "LUA";
            
        case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
            return "LUARES";
            
        case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
            return "UX1";
            
        default:
            return "Unknown";
    }
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CVSCPVariable::setName( const wxString& strName )
{
    wxString str = strName;
    str.Trim( false );
    str.Trim( true );
    m_name = str= strName.Upper();

    fixName();

}


///////////////////////////////////////////////////////////////////////////////
// setRighs
//

bool CVSCPVariable::setRighs( wxString& strRights )
{
    unsigned long val;
    wxString nills = _("000000000");
    wxString dash = _("---------");
    strRights.Trim();
    strRights.Trim( false );
    strRights.MakeUpper();
    
    // Must have a valid length
    if ( strRights.Length() > 9 ) {
        strRights = strRights.Left( 9 );
    }
    
    if ( strRights.ToULong( &val ) ) {
        // This is a numerical version  "777777777"
        strRights = nills.Left( 9 - strRights.Length() ) + strRights;
        m_accessRights = val;
    }
    else {
        // This is a character version "rwxrwxrwx"
        val = 0;
        strRights = dash.Left( 9 - strRights.Length() ) + strRights;
        for ( int i=0; i<3; i++ ) {
            
            if ( 'R' == strRights[ i*3 ] ) {
                val+=4;
            }
            if ( 'W' == strRights[ i*3 + 1 ] ) {
                val+=2;
            }
            if ( 'X' == strRights[ i*3 + 2 ] ) {
                val+=1;
            }
            
            if ( 2 != i ) val *= 10;
        }
        
        m_accessRights = val;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setUser
//

bool CVSCPVariable::setUser( wxString& strUser )
{
    sqlite3 *db;
    sqlite3_stmt *ppStmt;
    char *pErrMsg = 0;
    char psql[ 512 ];
    
    sprintf( psql,
                "SELECT * FROM \"user\" where username='%s'", 
                (const char *)strUser.mbc_str() );
    
    if ( SQLITE_OK != sqlite3_open( gpctrlObj->m_path_db_vscp_daemon.GetFullPath().mbc_str(),
                                            &db ) ) {
        return false;
    }
    
    if ( SQLITE_OK != sqlite3_prepare( db,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        sqlite3_close( db );
        return false;
    }
    
    // Get the result
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        m_userid = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_USER_ID );
    }
    
    sqlite3_finalize( ppStmt );
    
    sqlite3_close( db );
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setGroup
//

bool CVSCPVariable::setGroup( wxString& strGroup )
{
    sqlite3 *db;
    sqlite3_stmt *ppStmt;
    char *pErrMsg = 0;
    char psql[ 512 ];
    
    sprintf( psql, 
                "SELECT * FROM \"group\" where groupname='%s'",
                (const char *)strGroup.mbc_str() );
    
    if ( SQLITE_OK != sqlite3_open( gpctrlObj->m_path_db_vscp_daemon.GetFullPath().mbc_str(),
                                            &db ) ) {
        return false;
    }
    
    if ( SQLITE_OK != sqlite3_prepare( db,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        sqlite3_close( db );
        return false;
    }
    
    // Get the result
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        m_groupid = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_GROUP_ID );
    }
    
    sqlite3_finalize( ppStmt );
    
    sqlite3_close( db );
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeValueToString
//

bool CVSCPVariable::writeValueToString( wxString& strValueOut )
{
    wxString str = wxT("");
    switch ( m_type ) { 

        case VSCP_DAEMON_VARIABLE_CODE_STRING:
            //strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            strValueOut = m_strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
            if ( m_boolValue ) {
                strValueOut.Printf(_("true"));
            }
            else {
                strValueOut.Printf(_("false"));
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            strValueOut.Printf(_("%d"), ((int)m_longValue) );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_LONG:
            strValueOut.Printf(_("%ld"), m_longValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
            strValueOut.Printf(_("%f"), m_floatValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
            vscp_writeVscpDataWithSizeToString( m_normIntSize, m_normInteger, strValueOut );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
            vscp_writeVscpEventToString( &m_event, strValueOut );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
            vscp_writeGuidToString( &m_event, strValueOut );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
            vscp_writeVscpDataToString( &m_event, strValueOut );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
            strValueOut.Printf(_("%d"), m_event.vscp_class );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
            strValueOut.Printf(_("%d"), m_event.vscp_type );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
            strValueOut.Printf(_("%lu"), m_event.timestamp );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
            if ( m_timestamp.IsValid() ) {
                strValueOut = m_timestamp.FormatISODate();
                strValueOut += wxT( "T" );
                strValueOut += m_timestamp.FormatISOTime();
            }
            else {
                strValueOut = _("00-00-00T00:00:00");
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_BASE64:
            strValueOut = m_strValue;
            break;  

        case VSCP_DAEMON_VARIABLE_CODE_DATE:
            if ( m_timestamp.IsValid() ) {
                strValueOut = m_timestamp.FormatISODate();
            }
            else {
                strValueOut = _( "00-00-00T00:00:00" );
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_TIME:
            if ( m_timestamp.IsValid() ) {
                strValueOut = m_timestamp.FormatISOTime();
            }
            else {
                strValueOut = _( "00-00-00T00:00:00" );
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_MIME:
            strValueOut = m_strValue;
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_HTML:
            strValueOut = m_strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT:
            strValueOut = m_strValue;
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_JSON:
            strValueOut = m_strValue;
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_XML:
            strValueOut = m_strValue;
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_SQL:
            strValueOut = m_strValue;
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LUA:
            strValueOut = m_strValue;
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
            strValueOut = m_strValue;
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
            strValueOut = m_strValue;
            break; 

        case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:

            // Fall through

        default:
            strValueOut.Printf(_("UNKNOWN"));
            break;
    }

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// setValueFromString
//

bool CVSCPVariable::setValueFromString( CVSCPVariable::vartype type, 
                                            const wxString& strValue, 
                                            bool bBase64 )
{
    return setValueFromString( (int)type, strValue );
}

///////////////////////////////////////////////////////////////////////////////
// setValueFromString
//

bool CVSCPVariable::setValueFromString( int type, const wxString& strValue, bool bBase64 )
{	
    // Convert to uppercase
    wxString strUpper;

    // Update last changed
    setLastChangedToNow();

    switch ( type ) { 

        case VSCP_DAEMON_VARIABLE_CODE_STRING:
            // Should we do a BASE64 conversion
            if ( bBase64 ) {
                m_strValue = wxT("");
                uint8_t *pbuf = new uint8_t[ wxBase64DecodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Decode( pbuf, 
                                                wxBase64DecodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str() );
                if ( wxCONV_FAILED == len ) {
                    delete [] pbuf;
                    return false;
                }
                m_strValue.FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
            {
                strUpper = strValue.Upper();
                if ( wxNOT_FOUND != strUpper.Find( _("TRUE") ) ) {
                    m_boolValue = true;
                }
                else {
                    m_boolValue = false;
                }
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            m_longValue = vscp_readStringValue( strValue );
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LONG:
            m_longValue = vscp_readStringValue( strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
            strValue.ToDouble( &m_floatValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
            {
                uint8_t data[ VSCP_MAX_DATA ];
                uint16_t sizeData = 0;
                vscp_setVscpDataArrayFromString( data, &sizeData, strValue );
                if ( sizeData > 8 ) sizeData = 8;
                if (sizeData) memcpy( m_normInteger, data, sizeData );
                m_normIntSize = sizeData;
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
            vscp_setVscpEventFromString( &m_event, strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
            vscp_getGuidFromString( &m_event, strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
            vscp_setVscpDataFromString( &m_event, strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
            m_event.vscp_class = vscp_readStringValue( strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
            m_event.vscp_type = vscp_readStringValue( strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
            m_event.timestamp = vscp_readStringValue( strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
            m_timestamp.ParseDateTime( strValue );
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_BASE64:
            // Should we do a BASE64 conversion
            if ( bBase64 ) {
                m_strValue = wxT("");
                uint8_t *pbuf = new uint8_t[ wxBase64DecodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Decode( pbuf, 
                                                wxBase64DecodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str() );
                if ( wxCONV_FAILED == len ) {
                    delete [] pbuf;
                    return false;
                }
                m_strValue.FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;   

        case VSCP_DAEMON_VARIABLE_CODE_DATE:
            m_timestamp.ParseDate( strValue );
            break;
            
         case VSCP_DAEMON_VARIABLE_CODE_TIME:
            m_timestamp.ParseTime( strValue );
            break;   
            
        case VSCP_DAEMON_VARIABLE_CODE_JSON:
            // Should we do a BASE64 conversion
            if ( bBase64 ) {
                m_strValue = wxT("");
                uint8_t *pbuf = new uint8_t[ wxBase64DecodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Decode( pbuf, 
                                                wxBase64DecodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str() );
                if ( wxCONV_FAILED == len ) {
                    delete [] pbuf;
                    return false;
                }
                m_strValue.FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_XML:
            // Should we do a BASE64 conversion
            if ( bBase64 ) {
                m_strValue = wxT("");
                uint8_t *pbuf = new uint8_t[ wxBase64DecodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Decode( pbuf, 
                                                wxBase64DecodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str() );
                if ( wxCONV_FAILED == len ) {
                    delete [] pbuf;
                    return false;
                }
                m_strValue.FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_SQL:
            // Should we do a BASE64 conversion
            if ( bBase64 ) {
                m_strValue = wxT("");
                uint8_t *pbuf = new uint8_t[ wxBase64DecodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Decode( pbuf, 
                                                wxBase64DecodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str() );
                if ( wxCONV_FAILED == len ) {
                    delete [] pbuf;
                    return false;
                }
                m_strValue.FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LUA:
            // Should we do a BASE64 conversion
            if ( bBase64 ) {
                m_strValue = wxT("");
                uint8_t *pbuf = new uint8_t[ wxBase64DecodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Decode( pbuf, 
                                                wxBase64DecodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str() );
                if ( wxCONV_FAILED == len ) {
                    delete [] pbuf;
                    return false;
                }
                m_strValue.FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
            // Should we do a BASE64 conversion
            if ( bBase64 ) {
                m_strValue = wxT("");
                uint8_t *pbuf = new uint8_t[ wxBase64DecodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Decode( pbuf, 
                                                wxBase64DecodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str() );
                if ( wxCONV_FAILED == len ) {
                    delete [] pbuf;
                    return false;
                }
                m_strValue.FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
            // Should we do a BASE64 conversion
            if ( bBase64 ) {
                m_strValue = wxT("");
                uint8_t *pbuf = new uint8_t[ wxBase64DecodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Decode( pbuf, 
                                                wxBase64DecodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str() );
                if ( wxCONV_FAILED == len ) {
                    delete [] pbuf;
                    return false;
                }
                m_strValue.FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
            
            // Fall through - Not allowed here

        default:
            return false;

    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableFromString
//
// Format is: "variable name",type,"persistence",rights,user,group,"value"

bool CVSCPVariable::getVariableFromString( const wxString& strVariable, bool bBase64 )
{
    wxString strName;               // Name of variable
    wxString strValue;              // Variable value
    wxString strRights;             // User rights for variable
    wxString strUser;               // Owner of variable
    wxString strGroup;              // Group for variable
    int      typeVariable;          // Type of variable;
    bool     bPersistent = false;   // Persistence of variable
    bool     brw = true;            // Writable    
    
    // Update last changed
    setLastChangedToNow();

    wxStringTokenizer tkz( strVariable, _(";") );

    // Get name of variable
    if ( tkz.HasMoreTokens() ) {
        strName = tkz.GetNextToken();
    }
    else {
        return false;	
    }

    // Get the type of the variable
    if ( tkz.HasMoreTokens() ) {
        typeVariable = CVSCPVariable::getVariableTypeFromString( tkz.GetNextToken() );
    }
    else {
        return false;
    }

    // Get the persistence of the variable
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str = str.Upper();
        if ( wxNOT_FOUND != str.Find( _("TRUE") ) ) {
            bPersistent = true;
        }
    }
    else {
        return false;
    }
    
    // Get rights for the variable
    // uuugggooo
    // uuu - owner RWX/numerical
    // ggg - group RWX/numerical
    // ooo - other RWX/numerical
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        strRights = str.Upper();
    }
    else {
        return false;
    }
    
    // Get owner of the variable
    // Numeric or by name
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        strUser = str.Upper();
    }
    else {
        return false;
    }
    
    // Get group for the variable
    // Numeric or by name
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        strGroup = str.Upper();
    }
    else {
        return false;
    }

    // Get the value of the variable
    if ( tkz.HasMoreTokens() ) {
        strValue = tkz.GetNextToken();
    }
    else {
        return false;
    }

    // Set values
    m_type = typeVariable;
    m_bPersistent = bPersistent;
    setName( strName );
    setRighs( strRights );
    setUser( strUser );
    setGroup( strGroup );
    setValueFromString( typeVariable, strValue, bBase64 );

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// Reset
//

void CVSCPVariable::Reset( void )
{
    // Update lastchanged
    setLastChangedToNow();

    if ( m_bPersistent ) {
        //setValueFromString( m_type, m_bPersistent );
    }
    else {
        switch ( m_type ) { 

            case VSCP_DAEMON_VARIABLE_CODE_STRING:
                m_strValue = _("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
                m_boolValue = false;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            case VSCP_DAEMON_VARIABLE_CODE_LONG:
                m_longValue = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                m_floatValue = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                // TODO
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                if ( NULL != m_event.pdata ) {
                    delete [] m_event.pdata;
                }
                m_event.pdata = NULL;
                m_event.crc = 0;
                memset( m_event.GUID, 0, 16 );
                m_event.head = 0;
                m_event.obid = 0;
                m_event.sizeData = 0;
                m_event.timestamp = 0;
                m_event.vscp_class = 0;
                m_event.vscp_type = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                memset( m_event.GUID, 0, 16 );
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                if ( NULL != m_event.pdata ) {
                    delete [] m_event.pdata;
                }
                m_event.pdata = NULL;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                m_event.vscp_class = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                m_event.vscp_type = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                m_event.timestamp = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DATE:
            case VSCP_DAEMON_VARIABLE_CODE_TIME:
            case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
                m_timestamp = wxDateTime::Now();
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_BASE64:
                m_strValue = _("");
                break; 
                
            case VSCP_DAEMON_VARIABLE_CODE_MIME:
                m_strValue = _("");
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_HTML:
                m_strValue = _("");
                break;    
                
            case VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT:
                m_strValue = _("");
                break;    
                
            case VSCP_DAEMON_VARIABLE_CODE_JSON:
                m_strValue = _("");
                break;    
                
            case VSCP_DAEMON_VARIABLE_CODE_XML:
                m_strValue = _("");
                break;    
                
            case VSCP_DAEMON_VARIABLE_CODE_SQL:
                m_strValue = _("");
                break;    
                
            case VSCP_DAEMON_VARIABLE_CODE_LUA:
                m_strValue = _("");
                break;    
                
            case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
                m_strValue = _("");
                break;    
                
            case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
                m_strValue = _("");
                break;    

            case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
            
                // Fall through - Not allowed here

            default:
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// isTrue
//

bool CVSCPVariable::isTrue( void )
{
    if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType() && m_boolValue ) {
        return true;  
    }

    if ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType() && m_longValue ) {
        return true;  
    }

    if ( VSCP_DAEMON_VARIABLE_CODE_LONG == getType() && m_longValue ) {
        return true;  
    }

    if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType() && m_floatValue ) {
        return true;  
    }

    return false;

}

///////////////////////////////////////////////////////////////////////////////
// setTrue
//

void CVSCPVariable::setTrue( void )
{
    // Update lastchanged
    setLastChangedToNow();

    if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType() ) {
        m_boolValue = true;   
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType() ) {
        m_longValue = -1;
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_LONG == getType() ) {
        m_longValue = -1; 
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType() ) {
        m_floatValue = -1;
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_STRING == getType() ) {
        m_strValue = wxT("true"); 
    }
  
}


///////////////////////////////////////////////////////////////////////////////
// setFalse
//

void CVSCPVariable::setFalse( void )
{
    // Update lastchanged
    setLastChangedToNow();

    if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType() ) {
        m_boolValue = false;   
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType() ) {
        m_longValue = 0;
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_LONG == getType() ) {
        m_longValue = 0; 
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType() ) {
        m_floatValue = 0;
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_STRING == getType() ) {
        m_strValue = wxT("false"); 
    }
  
}



//*****************************************************************************
//                            CVariableStorage
//*****************************************************************************



///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CVariableStorage::CVariableStorage()
{
    m_db_vscp_external_variable = NULL;
    m_db_vscp_internal_variable = NULL;
    
    // Set the default dm configuration path
#ifdef WIN32
    m_configPath = wxStandardPaths::Get().GetConfigDir();
    m_configPath += _("/vscp/variables.xml");
#else
    m_configPath = _("/srv/vscp/variables.xml");
#endif  
    
#ifdef WIN32
    m_path_db_vscp_variable.SetName( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp_variable.sqlite3") );
#else
    m_path_db_vscp_external_variable.SetName( _("/srv/vscp/vscp_variable.sqlite3") );
#endif

    // We just read varibles  
    m_lastSaveTime = wxDateTime::Now();

}


///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVariableStorage::~CVariableStorage()
{
    CVSCPVariable *pVariable;

    listVscpVariable::iterator it;
    for( it = m_listVariable.begin(); it != m_listVariable.end(); ++it ) {
        
        if ( NULL == ( pVariable = *it ) ) continue;
        remove( pVariable->getName() );
    }
    
    // Close the internal variable database
    if ( NULL != m_db_vscp_internal_variable ) {
        sqlite3_close( m_db_vscp_internal_variable );
    }
    
    // Close the external variable database
    if ( NULL != m_db_vscp_external_variable ) {
        sqlite3_close( m_db_vscp_external_variable );
    }
}

///////////////////////////////////////////////////////////////////////////////
// exist
//

uint32_t CVariableStorage::exist(const wxString& name )
{
    CVSCPVariable variable;
    
    return find( name, variable );
}

///////////////////////////////////////////////////////////////////////////////
// find
//

uint32_t CVariableStorage::find(const wxString& name, CVSCPVariable& variable )
{
    // Look for stock variables
    if ( name.StartsWith("VSCP.") ) {
        return findStockVariable( name, variable );
    }
    else {
        
        // Search for non-persistent
        if ( findNonPersistentVariable( name, variable ) ) {
            return true;
        }
        else {
            return findPersistentVariable( name, variable );
        }
        
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// findStockVariable
//

uint32_t CVariableStorage::findStockVariable(const wxString& name, CVSCPVariable& var )
{
    wxString wxstr;

    // Make sure it starts with ".vscp"
    if ( !name.StartsWith("VSCP.") ) return false;
    
    
    if ( name == _("vscp.version.major") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), VSCPD_MAJOR_VERSION ) );
    }
    else if ( name == _("vscp.version.minor") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), VSCPD_MINOR_VERSION ) );
    }
    else if ( name == _("vscp.version.build") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), VSCPD_BUILD_VERSION ) );
    }
    else if ( name == _("vscp.version.str") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"), VSCPD_DISPLAY_VERSION ) );
    }
    else if ( name == _("vscp.version.wxwidgets.str") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"), wxVERSION_STRING ) );
    }
    else if ( name == _("vscp.version.wxwidgets.major") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), wxMAJOR_VERSION ) );
    }
    else if ( name == _("vscp.version.wxwidgets.minor") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), wxMINOR_VERSION ) );
    }
    else if ( name == _("vscp.version.wxwidgets.release") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), wxRELEASE_NUMBER ) );
    }
    else if ( name == _("vscp.version.wxwidgets.sub") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), wxSUBRELEASE_NUMBER ) );
    }
    else if ( name == _("vscp.copyright.vscp") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"), VSCPD_COPYRIGHT ) );
    }

// *****************************************************************************
//                               wxWidgets
// *****************************************************************************

    else if ( name == _("vscp.copyright.wxwidgets") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"),
                    "Copyright (c) 1998-2005 Julian Smart, Robert Roebling et al" ) );
    }

// *****************************************************************************
//                               Mongoose
// *****************************************************************************

    else if ( name == _("vscp.copyright.mongoose") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"),
                    "Copyright (c) 2013-2015 Cesanta Software Limited" ) );
    }

// *****************************************************************************
//                                 LUA
// *****************************************************************************

#ifndef VSCP_DISABLE_LUA

    else if ( name == _("vscp.copyright.lua") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"), LUA_COPYRIGHT ) );
    }
    else if ( name == _("vscp.version.lua.major") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%s"), LUA_VERSION_MAJOR ) );
    }
    else if ( name == _("vscp.version.lua.minor") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%s"), LUA_VERSION_MINOR) );
    }
    else if ( name == _("vscp.version.lua.release") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%s"), LUA_VERSION_RELEASE ) );
    }
    else if ( name == _("vscp.version.lua.str") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s.%s.%s"),
                                    LUA_VERSION_MAJOR,
                                    LUA_VERSION_MINOR,
                                    LUA_VERSION_RELEASE ) );
    }

#endif

// *****************************************************************************
//                                SQLite
// *****************************************************************************

    else if ( name == _("vscp.version.sqlite.major") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), major ) );
    }
    else if ( name == _("vscp.version.sqlite.minor") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), minor ) );
    }
    else if ( name == _("vscp.version.sqlite.release") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), sub ) );
    }
    else if ( name == _("vscp.version.sqlite.build") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), build ) );
    }
    else if ( name == _("vscp.version.sqllite.str") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( _(SQLITE_VERSION) );
    }

// *****************************************************************************
//                                OPENSSL
// *****************************************************************************
    
#ifndef WIN32
    else if ( name == _("vscp.version.openssl.str") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( _(OPENSSL_VERSION_TEXT) );
    }
#endif
    else if ( name == _("vscp.os.str") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxGetOsDescription() );
    }
    else if ( name == _("vscp.os.wordwidth") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxIsPlatform64Bit() ? _("64") : _("32") );
    }
    else if ( name == _("vscp.os.wordwidth.str") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxIsPlatform64Bit() ? _("64") : _("32") );
    }
    else if ( name == _("vscp.os.width.is64bit") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( wxIsPlatform64Bit() ? _("true") : _("false") );
    }
    else if ( name == _("vscp.os.width.is32bit") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( !wxIsPlatform64Bit() ? _("true") : _("false") );
    }
    else if ( name == _("vscp.os.endiness.str") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxIsPlatformLittleEndian() ? _("Little endian") : _("Big endian") );
    }
    else if ( name == _("vscp.os.endiness.islittleendian") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( wxIsPlatformLittleEndian() ? _("true") : _("false") );
    }
   else if ( name == _("vscp.host.fullname") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxGetFullHostName() );
    }
    else if ( name == _("vscp.host.ip") ) {
        cguid guid;
 
        if ( !gpctrlObj->getIPAddress( guid ) ) {
            guid.clear();
        }
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%d.%d.%d.%d"),
                                                    guid.getAt( 11 ),
                                                    guid.getAt( 10 ),
                                                    guid.getAt( 9 ),
                                                    guid.getAt( 8 ) ) );
    }
    else if ( name == _("vscp.host.mac") ) {
        cguid guid;
        
        if ( !gpctrlObj->getMacAddress( guid ) ) {
            guid.clear();
        }
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%02X:%02X:%02X:%02X:%02X:%02X"),
                            guid.getAt( 13 ),
                            guid.getAt( 12 ),
                            guid.getAt( 11 ),
                            guid.getAt( 10 ),
                            guid.getAt( 9 ),
                            guid.getAt( 8 ) ) );
    }
    else if ( name == _("vscp.host.userid") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxGetUserId() );
    }
    else if ( name ==  _("vscp.host.username") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxGetUserName() );
    }
    else if ( name == _("vscp.host.guid") ) {
        gpctrlObj->m_guid.toString( wxstr );
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxstr );
    }
    else if ( name == _("vscp.loglevel") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d "), gpctrlObj->m_logLevel ) );
    }

    else if ( name == _("vscp.loglevel.str") ) {
        switch ( gpctrlObj->m_logLevel  ) {
            case DAEMON_LOGMSG_NONE:
                wxstr = _("None");
                break;
            case DAEMON_LOGMSG_NORMAL:
                wxstr = _("Normal");
                break;
            case DAEMON_LOGMSG_DEBUG:
                wxstr = _("Debug");
                break;
            default:
                wxstr = _("Unknown");
             break;
        }
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxstr );
    }
    else if ( name == _("vscp.client.receivequeue.max") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setValue( wxString::Format( _("%d"), gpctrlObj->m_maxItemsInClientReceiveQueue ) );
    }
    else if ( name == _("vscp.tcpip.address") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_strTcpInterfaceAddress );
    }
    else if ( name == _("vscp.udp.isenabled") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bUDP ? _("true") : _("false") );
    }
    else if ( name == _("vscp.udp.address") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_strUDPInterfaceAddress );
    }
    else if ( name == _("vscp.mqtt.broker.isenabled") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bMQTTBroker ? _("true") : _("false") );
    }
    else if ( name == _("vscp.mqtt.broker.address") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_strMQTTBrokerInterfaceAddress );
    }
    else if ( name == _("vscp.coap.server.isenabled") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bCoAPServer ? _("true") : _("false") );
    }
    else if ( name == _("vscp.coap.server.address") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_strCoAPServerInterfaceAddress );
    }
    else if ( name == _("vscp.automation.heartbeat.isenabled") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_automation.isSendHeartbeat() ? _("true") : _("false") );
    }
    else if ( name == _("vscp.automation.heartbeat.period") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setValue( wxString::Format( _("%ld"), gpctrlObj->m_automation.getIntervalHeartbeat() ) );
    }
    else if ( name == _("vscp.automation.heartbeat.last") ) {
        wxstr = gpctrlObj->m_automation.getHeartbeatSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getHeartbeatSent().FormatISOTime();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
        var.setValue( wxstr );
    }
    else if ( name ==  _("vscp.automation.segctrl-heartbeat.isEnabled") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue(gpctrlObj->m_automation.isSendSegmentControllerHeartbeat() ? _("true") : _("false") );
    }
    else if ( name == _("vscp.automation.segctrl.heartbeat.period") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setValue( wxString::Format( _("%ld"), gpctrlObj->m_automation.getIntervalSegmentControllerHeartbeat() ) );
    }  
    else if ( name == _("vscp.automation.segctrl.heartbeat.last") ) {
        wxstr = gpctrlObj->m_automation.getSegmentControllerHeartbeatSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getSegmentControllerHeartbeatSent().FormatISOTime();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
        var.setValue( wxstr );
    }
    else if ( name == _("vscp.automation.longitude") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
        var.setValue( wxString::Format( _("%f"), gpctrlObj->m_automation.getLongitude() ) );
    }
    else if ( name == _("vscp.automation.latitude") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
        var.setValue( wxString::Format( _("%f"), gpctrlObj->m_automation.getLatitude() ) );
    }
    else if ( name == _("vscp.automation.issendtwilightsunriseevent") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_automation.isSendSunriseTwilightEvent() ? _("true") : _("false") );
    }
    else if ( name == _("vscp.automation.issendsunriseevent") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_automation.isSendSunriseEvent() ? _("true") : _("false") );
    }
    else if ( name == _("vscp.automation.issendsunsetevent") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_automation.isSendSunsetEvent() ? _("true") : _("false") );
    }
    else if ( name == _("vscp.automation.issendtwilightsunsetevent") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_automation.isSendSunsetTwilightEvent() ? _("true") : _("false") );
    }
    else if ( name == _("vscp.workingfolder") ) {
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxGetCwd() );
    }
 

// *****************************************************************************
//                                WEB-Server
// *****************************************************************************


    else if ( name == _("vscp.websrv.address") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_portWebServer );
    }
    else if ( name == _("vscp.websrv.authenticationOn") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bDisableSecurityWebServer ? _("true") : _("false") );
    }
    else if ( name == _("vscp.websrv.root.path") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_pathWebRoot ) );
    }
    else if ( name == _("vscp.websrv.authdomain") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( ( 0 == strlen( gpctrlObj->m_authDomain ) ) ?
                    wxString::FromAscii( "mydomain.com" ) :
                    wxString::FromUTF8( gpctrlObj->m_authDomain ) );
    }
    else if ( name == _("vscp.websrv.cert.path") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_pathCert ) );
    }
    else if ( name == _("vscp.websrv.extramimetypes") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_extraMimeTypes ) );
    }


    else if ( name == _("vscp.websrv.ssipatterns") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_ssi_pattern ) );
    }

    else if ( name == _("vscp.websrv.ipacl") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_ip_acl ) );
    }

    else if ( name == _("vscp.websrv.cgi.interpreter") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_cgiInterpreter ) );
    }

    else if ( name == _("vscp.websrv.cgi.pattern") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_cgiPattern ) );
    }
    else if ( name == _("vscp.websrv.directorylistings.enable") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( ( NULL != strstr( gpctrlObj->m_EnableDirectoryListings,"yes" ) ) ? _("true") : _("false") );
    }
    else if ( name == _("vscp.websrv.hidefile.pattern") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_hideFilePatterns ) );
    }
    else if ( name == _("vscp.websrv.indexfiles") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_indexFiles ) );
    }
    else if ( name == _("vscp.websrv.urlrewrites") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_urlRewrites ) );
    }
    else if ( name == _("vscp.websrv.auth.file.directory") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_per_directory_auth_file ) );
    }
    else if ( name == _("vscp.websrv.auth.file.global") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_per_directory_auth_file ) );
    }


// *****************************************************************************
//                            Websocket-Server
// *****************************************************************************

    else if ( name == _("vscp.websocket.auth.enable") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bAuthWebsockets ? _("true") : _("false") );
    }


// *****************************************************************************
//                            Decision Matrix
// *****************************************************************************


    else if ( name == _("vscp.dm.logging.enable") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_dm.m_bLogEnable ? _("true") : _("false") );
    }
    else if ( name == _("vscp.dm.logging.path") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_dm.m_logPath.GetFullPath() );
    }
    else if ( name == _("vscp.dm.static.path") ) {
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_dm.m_configPath ); 
    }


// *****************************************************************************
//                             Variables
// *****************************************************************************


    else if ( name == _("vscp.variable.path") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_VSCP_Variables.m_configPath );
    }


// *****************************************************************************
//                              Log files
// *****************************************************************************

    // Enable database logging
     else if ( name == _("vscp.log.database.enable") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bLogToDatabase ? _("true") : _("false") );
     }
    
    // General
    else if ( name == _("vscp.log.general.enable") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bLogGeneralEnable ? _("true") : _("false") );
    }
    else if ( name == _("vscp.log.general.path") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_logGeneralFileName.GetFullPath() );
    }

    // Access
    else if ( name == _("vscp.log.access.enable") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bLogAccessEnable ? _("true") : _("false") );
    }
    else if ( name == _("vscp.log.access.path") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_logAccessFileName.GetFullPath() );
    }

    // Security
    else if ( name == _("vscp.log.security.enable") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bLogAccessEnable ? _("true") : _("false") );
    }   
    else if ( name == _("vscp.log.security.path") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_logSecurityFileName.GetFullPath() );
    }


    
// *****************************************************************************
//                             Databases
// *****************************************************************************    
    
    
    else if ( name ==  _("vscp.database.log.path") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_path_db_vscp_log.GetFullPath() );
    }    
    else if ( name ==  _("vscp.database.vscpdata.path") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_path_db_vscp_data.GetFullPath() );
    } 
    else if ( name ==  _("vscp.database.daemon.path") ) {
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );;
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_path_db_vscp_daemon.GetFullPath() );
    }
    
    
// *****************************************************************************
//                              Drivers
// *****************************************************************************
    
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// findNonPersistentVariable
//

uint32_t CVariableStorage::findNonPersistentVariable(const wxString& name, CVSCPVariable& variable )
{
    sqlite3_stmt *ppStmt;
    char psql[ 512 ];
    
    sprintf( psql,
                "SELECT * FROM \"variableint\" where name='%s'", 
                (const char *)name.mbc_str() );
    
    if ( NULL == m_db_vscp_internal_variable ) {
        return 0;
    }
    
    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_internal_variable,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        return 0;
    }
    
    // Get the data for the variable
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_ID ) );
        variable.m_lastChanged.ParseDateTime( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_LASTCHANGE ) );
        variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_NAME ) ) );
        variable.setType( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_VALUE ) ) );
        variable.setPersistent( false );
        variable.setUserID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_LINK_TO_USER ) ); 
        variable.setGroupID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_LINK_TO_GROUP ) );
        variable.setAccessRights( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_NOTE ) ) );
    }
    
    sqlite3_finalize( ppStmt );
    
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// findPersistentVariable
//

uint32_t CVariableStorage::findPersistentVariable(const wxString& name, CVSCPVariable& variable )
{
    sqlite3_stmt *ppStmt;
    char psql[ 512 ];
    
    sprintf( psql,
                "SELECT * FROM \"variableext\" where name='%s'", 
                (const char *)name.mbc_str() );
    
    if ( NULL == m_db_vscp_internal_variable ) {
        return 0;
    }
    
    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_external_variable,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        return 0;
    }
    
    // Get the result
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_ID ) );
        variable.m_lastChanged.ParseDateTime( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_LASTCHANGE ) );
        variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_NAME ) ) );
        variable.setType( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_VALUE ) ) );
        variable.setPersistent( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_PERSISTENT ) ? true : false );
        variable.setUserID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_LINK_TO_USER ) );
        variable.setGroupID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_LINK_TO_GROUP ) );
        variable.setAccessRights( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_NOTE ) ) );
    }
    
    sqlite3_finalize( ppStmt );
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// add
//

bool CVariableStorage::add( CVSCPVariable& var )
{
    uint32_t id = 0;
    char *zErrMsg = 0;
    
    // Make sure variable name is valid
    var.fixName();
    
    // Stock variables can't be added.
    if ( var.getName().StartsWith("VSCP.") ) {
        return false;
    }

    // Update last changed timestamp
    var.setLastChangedToNow();

    if ( id = exist( var.getName() ) ) {      
        char *sql = sqlite3_mprintf( "UPDATE TABLE \"%s\" "
                                    "SET lastchange='%s', "
                                    "name='%g', "
                                    "type='%d', "
                                    "value='%g', "
                                    "bPersistent='%d', "
                                    "link_to_user='%d', "
                                    "link_to_group='%d', "
                                    "permission='%d', "
                                    "note='%g' "
                                    "WHERE idx_variableex='%d';",
                    var.isPersistent() ? "variableEx" : "variableInt",
                    (const char *)var.m_lastChanged.FormatISOCombined().mbc_str(),
                    (const char *)var.getName().mbc_str(),
                    var.getType(),
                    (const char *)var.getValue().mbc_str(),
                    var.isPersistent() ? 1 : 0,
                    var.getUserID(),
                    var.getGroupID(),
                    var.getAccessRights(),
                    (const char *)var.getNote().mbc_str(),
                    id
        );

        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_external_variable, sql, NULL, NULL, &zErrMsg ) ) {
            sqlite3_free( sql );
            return false;
        }   
        
        sqlite3_free( sql );
    }
    else {
        char *sql = sqlite3_mprintf( "INSERT INTO \"%s\" "
                                    "(lastchange,name,type,value,bPersistent,link_to_user,link_to_group,permission,note) "
                                    "VALUES ('%s','%g','%s','%g','%s','%d','%d','%s','%g')",
                                    var.isPersistent() ? "variableEx" : "variableInt",
                                    (const char *)var.m_lastChanged.FormatISOCombined().mbc_str(),
                                    (const char *)var.getName().mbc_str(),
                                    var.getType(),
                                    (const char *)var.getValue().mbc_str(),
                                    var.isPersistent() ? 1 : 0,
                                    var.getUserID(),
                                    var.getGroupID(),
                                    var.getAccessRights(),
                                    (const char *)var.getNote().mbc_str()
                                );
    
        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_external_variable, sql, NULL, NULL, &zErrMsg ) ) {
            sqlite3_free( sql );
            return false;
        }
        
        sqlite3_free( sql );
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// add
//

bool CVariableStorage::add( const wxString& varName, 
                                    const wxString& value, 
                                    uint8_t type, 
                                    bool bPersistent,
                                    bool brw )
{
    // Name is always upper case
    wxString name = varName.Upper();
    name.Trim( true );
    name.Trim( false );

    // Name should not contain spaces so if it does
    // we replace them with 'underscore'
    size_t pos;
    while ( wxNOT_FOUND != ( pos = name.Find( ' ' ) ) ) {
        name[pos] = '_';
    }

    CVSCPVariable *pVar = new CVSCPVariable;
    if ( NULL == pVar ) return false;

    // Update lastchanged
    pVar->setLastChangedToNow();

    pVar->setType( type );          // Store the type

    // Store persistence
    if ( bPersistent ) {
        pVar->setPersistent( true );
    }
    else {
        pVar->setPersistent( false );
    }
    
    // Writable
    if ( brw ) {
        pVar->makeWritable( true );
    }
    else {
        pVar->makeWritable( false );
    }

    pVar->setName( name );

    // Store value
    if ( !pVar->setValueFromString( type, value ) ) return false;

    // If persistent store persistent value
    if ( bPersistent ) {
        // pVar->setPersistatValue( value );  // TODO
    }

    if ( exist( name ) ) {
        
        // The variable is there already - just change value
        //m_hashVariable[ name ]->setValueFromString( m_hashVariable[ name ]->getType(), value );

        //delete pVar;    // No use for the variable

    }
    else {
        // New variable
        //m_hashVariable[ name ] = pVar;
        //m_listVariable.Append( pVar );
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addWithStringType
//

bool CVariableStorage::addWithStringType( const wxString& varName,
                                            const wxString& value,
                                            const wxString& strType,
                                            bool bPersistent,
                                            bool brw ) 
{
    uint8_t type = CVSCPVariable::getVariableTypeFromString( strType );
    return add(varName,
                value,
                type,
                bPersistent,
                brw ); 
}


///////////////////////////////////////////////////////////////////////////////
// remove
//

bool CVariableStorage::remove( CVSCPVariable *pVariable ) 
{
    if ( NULL == pVariable ) return false;

    return remove( pVariable->getName() );
}


///////////////////////////////////////////////////////////////////////////////
// remove
//

bool CVariableStorage::remove( wxString& name ) 
{
    name.MakeUpper();
    m_listVariable.DeleteObject( m_hashVariable[ name ] );
    return m_hashVariable.erase( name ) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// load
//
// Read persistent variables from file
//

bool CVariableStorage::load( void )
{
    bool bArray;
    wxXmlDocument doc;

#ifdef BUILD_VSCPD_SERVICE
    //wxStandardPaths stdPath;

    // Set the default dm configuration path
#ifdef WIN32
    m_configPath = wxStandardPaths::Get().GetConfigDir();
    m_configPath += _("/vscp/variables.xml");
#else
    m_configPath = _("/srv/vscp/variables.xml");
#endif
#endif

    if (!doc.Load( m_configPath ) ) {
        return false;
    }

    //::wxLogDebug ( _("Loading variables from: \n\t") + m_configPath );

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != wxT("persistent") ) {
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {

        if (child->GetName() == wxT("variable")) {

            CVSCPVariable variable;
            variable.setPersistent( true );     // Loaded variables are persistent
            bArray = false;

            // Mark last changed as now
            variable.setLastChangedToNow();

            // Get variable type - String is default           
            variable.setType( variable.getVariableTypeFromString( child->GetAttribute( wxT("type"), 
                                                                wxT("string") ) ) );            

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName() == wxT("name")) {
                    wxString strName = subchild->GetNodeContent();
                    strName.Trim();
                    strName.Trim(false);
                    // Replace spaces in name with underscore
                    int pos;
                    while (wxNOT_FOUND != ( pos = strName.Find(_(" ")))){
                        strName.SetChar(pos,wxChar('_'));
                    }
                    variable.setName( strName );
                }
                else if (subchild->GetName() == wxT("value")) {
                    variable.setValueFromString( variable.getType(), 
                                                    subchild->GetNodeContent() );
                    //pVar->setPersistatValue( subchild->GetNodeContent() );  // TODO
                }
                else if (subchild->GetName() == wxT("note")) {
                    variable.setNote( subchild->GetNodeContent() );
                }

                subchild = subchild->GetNext();

            }

            // Add the variable (always persistent if from file storage)
            add( variable );

        }

        child = child->GetNext();

    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// save
//
// Write persistent variables to file 
//

bool CVariableStorage::save()
{
#ifdef BUILD_VSCPD_SERVICE
    //wxStandardPaths stdPath;

    // Set the default dm configuration path
    m_configPath = wxStandardPaths::Get().GetConfigDir();
    m_configPath += _("/vscp/variable.xml");
#endif

    // Make a copy before we save
    wxCopyFile( m_configPath, m_configPath + _("~") );

    return save( m_configPath );
}


bool CVariableStorage::save( wxString& path )
{
    CVSCPVariable *pVariable;
    wxString str;
#ifdef BUILD_VSCPD_SERVICE
    //wxStandardPaths stdPath;

    // Set the default variable configuration path
    m_configPath = wxStandardPaths::Get().GetConfigDir();
    m_configPath += _("/vscp/variable.xml");
#endif

    if ( !wxFileName::IsFileWritable( path ) ) {
        //wxString strLog = _("Variable save: File is not writable.\n");
        //logMsg( strLog );
        return false;
    }

    wxFFileOutputStream *pFileStream = new wxFFileOutputStream( path );
    if ( NULL == pFileStream ) return false;
    if ( !pFileStream->IsOk() ) return false;

    pFileStream->Write("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n\n", 
                        strlen("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n\n") );

    // VSCP variables
    pFileStream->Write("<persistent>\n\n", strlen("<persistent>\n\n") );

    listVscpVariable::iterator it;
    for( it = m_listVariable.begin(); it != m_listVariable.end(); ++it ) {
        
        if ( NULL == ( pVariable = *it ) ) continue;
        wxString name = pVariable->getName();

        if ( ( NULL != pVariable ) && pVariable->isPersistent() ) {
    
            switch ( pVariable->getType() ) {

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                    
                    str.Printf( _("  <variable type=\"event\">\n"), pVariable->getType() );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    //pFileStream->Write( pVariable->m_strValue, pVariable->m_strValue.Length() );
                    pVariable->writeValueToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_STRING:

                    str.Printf( _("  <variable type=\"string\">\n"), pVariable->getType() );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    pFileStream->Write( pVariable->getValue().mb_str(), 
                            strlen( pVariable->getValue().mb_str() ) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:

                    str.Printf( _("  <variable type=\"bool\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    if ( pVariable->m_boolValue ) {
                        pFileStream->Write( "true", strlen("true") );
                    }
                    else {
                        pFileStream->Write( "false", strlen("false") );
                    }
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_INTEGER:

                    str.Printf( _("  <variable type=\"integer\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    str.Printf( _("%ld"), pVariable->m_longValue );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_LONG:

                    str.Printf( _("  <variable type=\"long\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    str.Printf( _("%ld"), pVariable->m_longValue );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:

                    str.Printf( _("  <variable type=\"double\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    str.Printf( _("%f"), pVariable->m_floatValue );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:

                    str.Printf( _("  <variable type=\"measurement\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    //writeVscpDataWithSizeToString( pVariable->m_normIntSize, 
                    //                                pVariable->m_normInteger, 
                    //                                str,
                    //                                true );
                    //pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pVariable->writeValueToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;
                    
                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:

                    str.Printf( _("  <variable type=\"data\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    pVariable->writeValueToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;    

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:

                    str.Printf( _("  <variable type=\"class\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    str.Printf( _("%d"), pVariable->m_event.vscp_class );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:

                    str.Printf( _("  <variable type=\"type\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    str.Printf( _("%d"), pVariable->m_event.vscp_type );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;


                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:

                    str.Printf( _("  <variable type=\"timestamp\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    //str.Printf( _("%d"), pVariable->m_event.timestamp );
                    //pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pVariable->writeValueToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:

                    str.Printf( _("  <variable type=\"guid\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    pVariable->writeValueToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_DATETIME:

                    str.Printf( _("  <variable type=\"datetime\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    pVariable->writeValueToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;
                    
                case VSCP_DAEMON_VARIABLE_CODE_BASE64:

                    str.Printf( _("  <variable type=\"base64\">\n"), pVariable->getType() );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    pFileStream->Write( pVariable->getValue().mb_str(), 
                            strlen( pVariable->getValue().mb_str() ) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;
                    
                case VSCP_DAEMON_VARIABLE_CODE_DATE:

                    str.Printf( _("  <variable type=\"date\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    pVariable->writeValueToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_TIME:

                    str.Printf( _("  <variable type=\"time\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    pVariable->writeValueToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

            }

        }
        
    }

    // DM matrix information end
    pFileStream->Write("</persistent>\n",strlen("</persistent>\n"));

    // Close the file
    pFileStream->Close();

    // Variable saved
    m_lastSaveTime = wxDateTime::Now();

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// doCreateExternalVariableTable
//
// Create the external variable database
//
//

bool CVariableStorage::doCreateExternalVariableTable( void )
{
    char *pErrMsg = 0;
    const char *psql = "CREATE TABLE \"variableEx\" ("
	"`idx_variableex`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
	"`name`	TEXT NOT NULL UNIQUE,"
	"`type`	INTEGER NOT NULL DEFAULT 0,"
	"`value`	TEXT NOT NULL,"
	"`bPersistent`	INTEGER NOT NULL DEFAULT 0,"
	"`link_to_user`	INTEGER NOT NULL,"
	"`link_to_group`	INTEGER NOT NULL,"
	"`permission`	INTEGER NOT NULL,"
	"`note`	TEXT"
    ")";
    
    // Check if database is open
    if ( NULL == m_db_vscp_external_variable ) return false;
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_external_variable, psql, NULL, NULL, NULL) ) {
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateInternalVariableTable
//
// Create the internal variable database
//
//

bool CVariableStorage::doCreateInternalVariableTable( void )
{
    char *pErrMsg = 0;
    const char *psql = "CREATE TABLE \"variableInt\" ("
	"`idx_variableint`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
	"`name`	TEXT NOT NULL UNIQUE,"
	"`type`	INTEGER NOT NULL,"
	"`value`	TEXT NOT NULL,"
	"`bStock`	INTEGER NOT NULL,"
	"`link_to_user`	INTEGER NOT NULL,"
	"`link_to_group`	INTEGER NOT NULL,"
	"`permission`	INTEGER NOT NULL,"
	"`Note`	TEXT"
    ")";
    
    // Check if database is open
    if ( NULL == m_db_vscp_internal_variable ) return false;
    
    if ( SQLITE_OK != sqlite3_exec(m_db_vscp_internal_variable, psql, NULL, NULL, NULL) ) {
        return false;
    }
    
    return true;
}