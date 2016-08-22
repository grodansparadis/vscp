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
    
    m_bStock = false;   // This is not a stock variable

}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPVariable::~CVSCPVariable( void )
{
    ;
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
        else if ( 0 == str.Find( _("DMROW") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_DM_ROW;
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
            
        case VSCP_DAEMON_VARIABLE_CODE_DM_ROW:
            return "DMROW";    
            
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

bool CVSCPVariable::writeValueToString( wxString& strValueOut, bool bBase64 )
{
    wxString str = wxT("");
    switch ( m_type ) { 

        case VSCP_DAEMON_VARIABLE_CODE_STRING:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
        case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
        case VSCP_DAEMON_VARIABLE_CODE_LONG:  
        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
        case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:  
        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:  
        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
        case VSCP_DAEMON_VARIABLE_CODE_DATETIME: 
        case VSCP_DAEMON_VARIABLE_CODE_DATE:
        case VSCP_DAEMON_VARIABLE_CODE_TIME:    
            strValueOut = m_strValue;
            break;
  
        case VSCP_DAEMON_VARIABLE_CODE_BASE64:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;  

        case VSCP_DAEMON_VARIABLE_CODE_MIME:
            strValueOut = m_strValue;
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_HTML:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_JSON:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_XML:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_SQL:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LUA:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break; 
            
        case VSCP_DAEMON_VARIABLE_CODE_DM_ROW:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
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
// getValue
//

void CVSCPVariable::setValue(bool val)
{
    m_strValue.Printf(_("%s"), val ? "true" : "false" );
}

///////////////////////////////////////////////////////////////////////////////
// getValue
//

void CVSCPVariable::getValue(bool *pValue)
{
    if ( wxNOT_FOUND != m_strValue.Upper().Find(_("TRUE") ) ) {
        *pValue = true;
    }
    else {
        *pValue = false;
    }
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void CVSCPVariable::setValue(int val)
{
    m_strValue.Printf(_("%d"), val );
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//
    
void CVSCPVariable::getValue(int *pValue)
{
    long longValue = 0;
    m_strValue.ToLong( &longValue );
    *pValue = (int)longValue;
};
    
///////////////////////////////////////////////////////////////////////////////
// setValue
//
    
void CVSCPVariable::setValue(long val)
{
    m_strValue.Printf(_("%ld"), val );
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void CVSCPVariable::getValue(long *pValue)
{
    m_strValue.ToLong( pValue );
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void CVSCPVariable::setValue(double val)
{
    m_strValue.Printf(_("%f"), val );
}

///////////////////////////////////////////////////////////////////////////////
// getValue
//

void CVSCPVariable::getValue(double *pValue)
{
    m_strValue.ToDouble( pValue );
}



///////////////////////////////////////////////////////////////////////////////
// setValue
//
void CVSCPVariable::setValue(vscpEvent& event)
{
    vscp_writeVscpEventToString( &event, m_strValue );
};

///////////////////////////////////////////////////////////////////////////////
// getValue
//
void CVSCPVariable::getValue(vscpEvent *pEvent)
{
    if ( NULL != pEvent ) return;
    vscp_setVscpEventFromString( pEvent, m_strValue );
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//
void CVSCPVariable::setValue(vscpEventEx& eventex)
{
    vscp_writeVscpEventExToString( &eventex, m_strValue );
};

///////////////////////////////////////////////////////////////////////////////
// getValue
//
void CVSCPVariable::getValue(vscpEventEx *pEventEx)
{
    if ( NULL != pEventEx ) return;
    vscp_setVscpEventExFromString( pEventEx, m_strValue );
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void CVSCPVariable::getValue(wxDateTime *pValue)
{
    pValue->ParseDateTime( m_strValue );
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void CVSCPVariable::setValue(wxDateTime& val)
{
    if ( val.IsValid() ) {
        m_strValue = val.FormatISOCombined();
    }
    else {
        m_strValue = _("00-00-00T00:00:00");
    }
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
                    m_strValue = "true";
                }
                else {
                    m_strValue = "false";
                }
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            m_strValue = strValue;
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LONG:
            m_strValue = strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
            m_strValue = strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
            m_strValue = strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
            m_strValue = strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
            m_strValue = strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
            m_strValue = strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
            m_strValue = strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
            m_strValue = strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
            m_strValue = strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
            m_strValue = strValue;
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
            m_strValue = strValue;
            break;
            
         case VSCP_DAEMON_VARIABLE_CODE_TIME:
            m_strValue = strValue;
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
            
        case VSCP_DAEMON_VARIABLE_CODE_DM_ROW:
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
                m_strValue = _("false");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            case VSCP_DAEMON_VARIABLE_CODE_LONG:
                m_strValue = _("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                m_strValue = _("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                m_strValue = _("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                m_strValue = _("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                m_strValue = _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                m_strValue = _("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                m_strValue = _("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                m_strValue = _("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                m_strValue = _("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DATE:
                m_strValue = wxDateTime::Now().FormatISODate();
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_TIME:
                m_strValue = wxDateTime::Now().FormatISOTime();
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
                m_strValue = wxDateTime::Now().FormatISOCombined();
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
                
            case VSCP_DAEMON_VARIABLE_CODE_DM_ROW:
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
    if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType() && 
            ( wxNOT_FOUND != m_strValue.Upper().Find(_("TRUE") ) ) ) {
        return true;  
    }

    if ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType() && 
            atoi( m_strValue.mbc_str() ) ) {
        return true;  
    }

    if ( VSCP_DAEMON_VARIABLE_CODE_LONG == getType() && 
            atol( m_strValue.mbc_str() ) ) {
        return true;  
    }

    if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType()  ) {
        double d;
        getValue( &d );
        if ( d != 0 ) return true;  
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
        m_strValue = _("true");   
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType() ) {
        m_strValue = _("1");
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_LONG == getType() ) {
        m_strValue = _("1");
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType() ) {
        m_strValue = _("1.0");
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
        m_strValue = _("false"); 
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType() ) {
        m_strValue = _("0");
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_LONG == getType() ) {
        m_strValue = _("0"); 
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType() ) {
        m_strValue = _("0");
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

    // We just read variables  
    m_lastSaveTime = wxDateTime::Now();

    // Add stock variables
    m_StockVariable.Add( _("vscp.version.major") );
    m_StockVariable.Add( _("vscp.version.minor") );
    m_StockVariable.Add( _("vscp.version.build") );
    m_StockVariable.Add( _("vscp.version.str") );
    m_StockVariable.Add( _("vscp.version.wxwidgets.str") );
    m_StockVariable.Add( _("vscp.version.wxwidgets.major") );
    m_StockVariable.Add( _("vscp.version.wxwidgets.minor") );
    m_StockVariable.Add( _("vscp.version.wxwidgets.release") );
    m_StockVariable.Add( _("vscp.version.wxwidgets.sub") );
    m_StockVariable.Add( _("vscp.copyright.vscp") );
    m_StockVariable.Add( _("vscp.copyright.wxwidgets") );
    m_StockVariable.Add( _("vscp.copyright.mongoose") );
    m_StockVariable.Add( _("vscp.copyright.lua") );
    m_StockVariable.Add( _("vscp.version.lua.major") );
    m_StockVariable.Add( _("vscp.version.lua.minor") );
    m_StockVariable.Add( _("vscp.version.lua.release") );
    m_StockVariable.Add( _("vscp.version.lua.str") );
    m_StockVariable.Add( _("vscp.version.sqlite.major") );
    m_StockVariable.Add( _("vscp.version.sqlite.minor") );
    m_StockVariable.Add( _("vscp.version.sqlite.release") );
    m_StockVariable.Add( _("vscp.version.sqlite.build") );
    m_StockVariable.Add( _("vscp.version.sqllite.str") );
    m_StockVariable.Add( _("vscp.version.openssl.str") );
    m_StockVariable.Add( _("vscp.os.str") );
    m_StockVariable.Add( _("vscp.os.wordwidth") );
    m_StockVariable.Add( _("vscp.os.wordwidth.str") );
    m_StockVariable.Add( _("vscp.os.width.is64bit") );
    m_StockVariable.Add( _("vscp.os.width.is32bit") );
    m_StockVariable.Add( _("vscp.os.endiness.str") );
    m_StockVariable.Add( _("vscp.os.endiness.islittleendian") );
    m_StockVariable.Add( _("vscp.host.fullname") );
    m_StockVariable.Add( _("vscp.host.ip") );
    m_StockVariable.Add( _("vscp.host.mac") );
    m_StockVariable.Add( _("vscp.host.userid") );
    m_StockVariable.Add( _("vscp.host.username") );
    m_StockVariable.Add( _("vscp.host.guid") );
    m_StockVariable.Add( _("vscp.loglevel") );
    m_StockVariable.Add( _("vscp.loglevel.str") );
    m_StockVariable.Add( _("vscp.client.receivequeue.max") );
    m_StockVariable.Add( _("vscp.tcpip.address") );
    m_StockVariable.Add( _("vscp.udp.isenabled") );
    m_StockVariable.Add( _("vscp.udp.address") );
    m_StockVariable.Add( _("vscp.mqtt.broker.isenabled") );
    m_StockVariable.Add( _("vscp.mqtt.broker.address") );
    m_StockVariable.Add( _("vscp.coap.server.isenabled") );
    m_StockVariable.Add( _("vscp.coap.server.address") );
    m_StockVariable.Add( _("vscp.automation.heartbeat.isenabled") );
    m_StockVariable.Add( _("vscp.automation.heartbeat.period") );
    m_StockVariable.Add( _("vscp.automation.heartbeat.last") );
    m_StockVariable.Add( _("vscp.automation.segctrl-heartbeat.isEnabled") );
    m_StockVariable.Add( _("vscp.automation.segctrl.heartbeat.period") );
    m_StockVariable.Add( _("vscp.automation.segctrl.heartbeat.last") );
    m_StockVariable.Add( _("vscp.automation.longitude") );
    m_StockVariable.Add( _("vscp.automation.latitude") );
    m_StockVariable.Add( _("vscp.automation.issendtwilightsunriseevent") );
    m_StockVariable.Add( _("vscp.automation.issendtwilightsunriseevent") );
    m_StockVariable.Add( _("vscp.automation.issendsunsetevent") );
    m_StockVariable.Add( _("vscp.automation.issendtwilightsunsetevent") );
    m_StockVariable.Add( _("vscp.workingfolder") );
    m_StockVariable.Add( _("vscp.websrv.address") );
    m_StockVariable.Add( _("vscp.websrv.authenticationOn") );
    m_StockVariable.Add( _("vscp.websrv.root.path") );
    m_StockVariable.Add( _("vscp.websrv.authdomain") );
    m_StockVariable.Add( _("vscp.websrv.cert.path") );
    m_StockVariable.Add( _("vscp.websrv.extramimetypes") );
    m_StockVariable.Add( _("vscp.websrv.ssipatterns") );
    m_StockVariable.Add( _("vscp.websrv.ipacl") );
    m_StockVariable.Add( _("vscp.websrv.cgi.interpreter") );
    m_StockVariable.Add( _("vscp.websrv.cgi.pattern") );
    m_StockVariable.Add( _("vscp.websrv.directorylistings.enable") );
    m_StockVariable.Add( _("vscp.websrv.hidefile.pattern") );
    m_StockVariable.Add( _("vscp.websrv.indexfiles") );
    m_StockVariable.Add( _("vscp.websrv.urlrewrites") );
    m_StockVariable.Add( _("vscp.websrv.auth.file.directory") );
    m_StockVariable.Add( _("vscp.websrv.auth.file.global") );
    m_StockVariable.Add( _("vscp.websocket.auth.enable") );
    m_StockVariable.Add( _("vscp.dm.logging.enable") );
    m_StockVariable.Add( _("vscp.dm.logging.path") );
    m_StockVariable.Add( _("vscp.dm.static.path") );
    m_StockVariable.Add( _("vscp.variable.path") );
    m_StockVariable.Add( _("vscp.log.database.enable") );
    m_StockVariable.Add( _("vscp.log.general.enable") );
    m_StockVariable.Add( _("vscp.log.general.path") );
    m_StockVariable.Add( _("vscp.log.access.enable") );
    m_StockVariable.Add( _("vscp.log.access.path") );
    m_StockVariable.Add( _("vscp.log.security.enable") );
    m_StockVariable.Add( _("vscp.log.security.path") );
    m_StockVariable.Add( _("vscp.database.log.path") );
    m_StockVariable.Add( _("vscp.database.vscpdata.path") );
    m_StockVariable.Add( _("vscp.database.daemon.path") );
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVariableStorage::~CVariableStorage()
{    
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
    uint32_t id = 0;
    
    // Look for stock variables
    if ( name.StartsWith("VSCP.") ) {
        return findStockVariable( name, variable );
    }
    else {
        
        // Search for non-persistent
        if ( id = findNonPersistentVariable( name, variable ) ) {
            return id;
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

bool CVariableStorage::findStockVariable(const wxString& name, CVSCPVariable& var )
{
    wxString wxstr;
    
    // Make sure it starts with ".vscp"
    if ( !name.Lower().StartsWith("vscp.") ) return false;  
    
    if ( name.Lower() == _("vscp.version.major") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), VSCPD_MAJOR_VERSION ) );
    }
    else if ( name.Lower() == _("vscp.version.minor") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), VSCPD_MINOR_VERSION ) );
    }
    else if ( name.Lower() == _("vscp.version.build") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), VSCPD_BUILD_VERSION ) );
    }
    else if ( name.Lower() == _("vscp.version.str") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"), VSCPD_DISPLAY_VERSION ) );
    }
    else if ( name.Lower() == _("vscp.version.wxwidgets.str") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"), wxVERSION_STRING ) );
    }
    else if ( name.Lower() == _("vscp.version.wxwidgets.major") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), wxMAJOR_VERSION ) );
    }
    else if ( name.Lower() == _("vscp.version.wxwidgets.minor") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), wxMINOR_VERSION ) );
    }
    else if ( name.Lower() == _("vscp.version.wxwidgets.release") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), wxRELEASE_NUMBER ) );
    }
    else if ( name.Lower() == _("vscp.version.wxwidgets.sub") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), wxSUBRELEASE_NUMBER ) );
    }
    else if ( name.Lower() == _("vscp.copyright.vscp") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"), VSCPD_COPYRIGHT ) );
    }

// *****************************************************************************
//                               wxWidgets
// *****************************************************************************

    else if ( name.Lower() == _("vscp.copyright.wxwidgets") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"),
                    "Copyright (c) 1998-2005 Julian Smart, Robert Roebling et al" ) );
    }

// *****************************************************************************
//                               Mongoose
// *****************************************************************************

    else if ( name.Lower() == _("vscp.copyright.mongoose") ) {
        var.setStockVariable();
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

    else if ( name.Lower() == _("vscp.copyright.lua") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%s"), LUA_COPYRIGHT ) );
    }
    else if ( name.Lower() == _("vscp.version.lua.major") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%s"), LUA_VERSION_MAJOR ) );
    }
    else if ( name.Lower() == _("vscp.version.lua.minor") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%s"), LUA_VERSION_MINOR) );
    }
    else if ( name.Lower() == _("vscp.version.lua.release") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%s"), LUA_VERSION_RELEASE ) );
    }
    else if ( name.Lower() == _("vscp.version.lua.str") ) {
        var.setStockVariable();
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

    else if ( name.Lower() == _("vscp.version.sqlite.major") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), major ) );
    }
    else if ( name.Lower() == _("vscp.version.sqlite.minor") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), minor ) );
    }
    else if ( name.Lower() == _("vscp.version.sqlite.release") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), sub ) );
    }
    else if ( name.Lower() == _("vscp.version.sqlite.build") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d"), build ) );
    }
    else if ( name.Lower() == _("vscp.version.sqllite.str") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( _(SQLITE_VERSION) );
    }

// *****************************************************************************
//                                OPENSSL
// *****************************************************************************
    
#ifndef WIN32
    else if ( name.Lower() == _("vscp.version.openssl.str") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( _(OPENSSL_VERSION_TEXT) );
    }
#endif
    else if ( name.Lower() == _("vscp.os.str") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxGetOsDescription() );
    }
    else if ( name.Lower() == _("vscp.os.wordwidth") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxIsPlatform64Bit() ? _("64") : _("32") );
    }
    else if ( name.Lower() == _("vscp.os.wordwidth.str") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxIsPlatform64Bit() ? _("64") : _("32") );
    }
    else if ( name.Lower() == _("vscp.os.width.is64bit") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( wxIsPlatform64Bit() ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.os.width.is32bit") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( !wxIsPlatform64Bit() ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.os.endiness.str") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxIsPlatformLittleEndian() ? _("Little endian") : _("Big endian") );
    }
    else if ( name.Lower() == _("vscp.os.endiness.islittleendian") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( wxIsPlatformLittleEndian() ? _("true") : _("false") );
    }
   else if ( name.Lower() == _("vscp.host.fullname") ) {
       var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxGetFullHostName() );
    }
    else if ( name.Lower() == _("vscp.host.ip") ) {
        cguid guid;
 
        if ( !gpctrlObj->getIPAddress( guid ) ) {
            guid.clear();
        }
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::Format( _("%d.%d.%d.%d"),
                                                    guid.getAt( 11 ),
                                                    guid.getAt( 10 ),
                                                    guid.getAt( 9 ),
                                                    guid.getAt( 8 ) ) );
    }
    else if ( name.Lower() == _("vscp.host.mac") ) {
        cguid guid;
        
        if ( !gpctrlObj->getMacAddress( guid ) ) {
            guid.clear();
        }
        var.setStockVariable();
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
    else if ( name.Lower() == _("vscp.host.userid") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxGetUserId() );
    }
    else if ( name.Lower() ==  _("vscp.host.username") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxGetUserName() );
    }
    else if ( name.Lower() == _("vscp.host.guid") ) {
        gpctrlObj->m_guid.toString( wxstr );
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxstr );
    }
    else if ( name.Lower() == _("vscp.loglevel") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
        var.setValue( wxString::Format( _("%d "), gpctrlObj->m_logLevel ) );
    }
    else if ( name.Lower() == _("vscp.loglevel.str") ) {
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
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxstr );
    }
    else if ( name.Lower() == _("vscp.client.receivequeue.max") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setValue( wxString::Format( _("%d"), gpctrlObj->m_maxItemsInClientReceiveQueue ) );
    }
    else if ( name.Lower() == _("vscp.tcpip.address") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_strTcpInterfaceAddress );
    }
    else if ( name.Lower() == _("vscp.udp.isenabled") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bUDP ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.udp.address") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_strUDPInterfaceAddress );
    }
    else if ( name.Lower() == _("vscp.mqtt.broker.isenabled") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bMQTTBroker ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.mqtt.broker.address") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_strMQTTBrokerInterfaceAddress );
    }
    else if ( name.Lower() == _("vscp.coap.server.isenabled") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bCoAPServer ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.coap.server.address") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_strCoAPServerInterfaceAddress );
    }
    else if ( name.Lower() == _("vscp.automation.heartbeat.isenabled") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_automation.isSendHeartbeat() ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.automation.heartbeat.period") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setValue( wxString::Format( _("%ld"), gpctrlObj->m_automation.getIntervalHeartbeat() ) );
    }
    else if ( name.Lower() == _("vscp.automation.heartbeat.last") ) {
        wxstr = gpctrlObj->m_automation.getHeartbeatSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getHeartbeatSent().FormatISOTime();
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
        var.setValue( wxstr );
    }
    else if ( name.Lower() ==  _("vscp.automation.segctrl-heartbeat.isEnabled") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue(gpctrlObj->m_automation.isSendSegmentControllerHeartbeat() ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.automation.segctrl.heartbeat.period") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setValue( wxString::Format( _("%ld"), gpctrlObj->m_automation.getIntervalSegmentControllerHeartbeat() ) );
    }  
    else if ( name.Lower() == _("vscp.automation.segctrl.heartbeat.last") ) {
        wxstr = gpctrlObj->m_automation.getSegmentControllerHeartbeatSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getSegmentControllerHeartbeatSent().FormatISOTime();
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
        var.setValue( wxstr );
    }
    else if ( name.Lower() == _("vscp.automation.longitude") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
        var.setValue( wxString::Format( _("%f"), gpctrlObj->m_automation.getLongitude() ) );
    }
    else if ( name.Lower() == _("vscp.automation.latitude") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
        var.setValue( wxString::Format( _("%f"), gpctrlObj->m_automation.getLatitude() ) );
    }
    else if ( name.Lower() == _("vscp.automation.issendtwilightsunriseevent") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_automation.isSendSunriseTwilightEvent() ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.automation.issendtwilightsunriseevent") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_automation.isSendSunriseEvent() ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.automation.issendsunsetevent") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_automation.isSendSunsetEvent() ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.automation.issendtwilightsunsetevent") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_automation.isSendSunsetTwilightEvent() ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.workingfolder") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxGetCwd() );
    }
 

// *****************************************************************************
//                                WEB-Server
// *****************************************************************************


    else if ( name.Lower() == _("vscp.websrv.address") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_portWebServer );
    }
    else if ( name.Lower() == _("vscp.websrv.authenticationOn") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bDisableSecurityWebServer ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.websrv.root.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_pathWebRoot ) );
    }
    else if ( name.Lower() == _("vscp.websrv.authdomain") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( ( 0 == strlen( gpctrlObj->m_authDomain ) ) ?
                    wxString::FromAscii( "mydomain.com" ) :
                    wxString::FromUTF8( gpctrlObj->m_authDomain ) );
    }
    else if ( name.Lower() == _("vscp.websrv.cert.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_pathCert ) );
    }
    else if ( name.Lower() == _("vscp.websrv.extramimetypes") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_extraMimeTypes ) );
    }


    else if ( name.Lower() == _("vscp.websrv.ssipatterns") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_ssi_pattern ) );
    }

    else if ( name.Lower() == _("vscp.websrv.ipacl") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_ip_acl ) );
    }

    else if ( name.Lower() == _("vscp.websrv.cgi.interpreter") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_cgiInterpreter ) );
    }

    else if ( name.Lower() == _("vscp.websrv.cgi.pattern") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_cgiPattern ) );
    }
    else if ( name.Lower() == _("vscp.websrv.directorylistings.enable") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( ( NULL != strstr( gpctrlObj->m_EnableDirectoryListings,"yes" ) ) ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.websrv.hidefile.pattern") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_hideFilePatterns ) );
    }
    else if ( name.Lower() == _("vscp.websrv.indexfiles") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_indexFiles ) );
    }
    else if ( name.Lower() == _("vscp.websrv.urlrewrites") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_urlRewrites ) );
    }
    else if ( name.Lower() == _("vscp.websrv.auth.file.directory") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_per_directory_auth_file ) );
    }
    else if ( name.Lower() == _("vscp.websrv.auth.file.global") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxString::FromUTF8( gpctrlObj->m_per_directory_auth_file ) );
    }


// *****************************************************************************
//                            Websocket-Server
// *****************************************************************************

    else if ( name.Lower() == _("vscp.websocket.auth.enable") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bAuthWebsockets ? _("true") : _("false") );
    }


// *****************************************************************************
//                            Decision Matrix
// *****************************************************************************


    else if ( name.Lower() == _("vscp.dm.logging.enable") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_dm.m_bLogEnable ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.dm.logging.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_dm.m_logPath.GetFullPath() );
    }
    else if ( name.Lower() == _("vscp.dm.static.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_dm.m_configPath ); 
    }


// *****************************************************************************
//                             Variables
// *****************************************************************************


    else if ( name.Lower() == _("vscp.variable.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_VSCP_Variables.m_configPath );
    }


// *****************************************************************************
//                              Log files
// *****************************************************************************

    // Enable database logging
    else if ( name.Lower() == _("vscp.log.database.enable") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bLogToDatabase ? _("true") : _("false") );
     }
    
    // General
    else if ( name.Lower() == _("vscp.log.general.enable") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bLogGeneralEnable ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.log.general.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_logGeneralFileName.GetFullPath() );
    }

    // Access
    else if ( name.Lower() == _("vscp.log.access.enable") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bLogAccessEnable ? _("true") : _("false") );
    }
    else if ( name.Lower() == _("vscp.log.access.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_logAccessFileName.GetFullPath() );
    }

    // Security
    else if ( name.Lower() == _("vscp.log.security.enable") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
        var.setValue( gpctrlObj->m_bLogAccessEnable ? _("true") : _("false") );
    }   
    else if ( name.Lower() == _("vscp.log.security.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_logSecurityFileName.GetFullPath() );
    }


    
// *****************************************************************************
//                             Databases
// *****************************************************************************    
    
    
    else if ( name.Lower() ==  _("vscp.database.log.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_path_db_vscp_log.GetFullPath() );
    }    
    else if ( name.Lower() ==  _("vscp.database.vscpdata.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_path_db_vscp_data.GetFullPath() );
    } 
    else if ( name.Lower() ==  _("vscp.database.daemon.path") ) {
        var.setStockVariable();
        var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
        var.setPersistent( false );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( gpctrlObj->m_path_db_vscp_daemon.GetFullPath() );
    }
    
    
// *****************************************************************************
//                             Decision Matrix
// *****************************************************************************
    else if ( name.Lower() == _("vscp.dm.count") ) {
        var.setStockVariable();
        var.setPersistent( false );
        var.setAccessRights( PERMISSON_OWNER_READ );         
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setValue( (long)gpctrlObj->m_dm.getDatabaseRecordCount() );
    }
    else if ( name.Lower() == _("vscp.dm.count.active") ) {
        var.setStockVariable();
        var.setPersistent( false );
        var.setAccessRights( PERMISSON_OWNER_READ );;           
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setValue( gpctrlObj->m_dm.getMemoryElementCount() );      
    }
    else if ( wxNOT_FOUND != name.Lower().Find( _("vscp.dm.") ) ) {
        
        int pos;
        wxString wxstr;
        wxstr = name.Right( name.Length() - 8 );    // remove "vscp.dm."
        if ( wxNOT_FOUND != ( pos = wxstr.Lower().Find( _(".") ) ) ) {
            
            // this is a sub variable.
            wxString strID;
            unsigned long id;
            
            // Get id
            if ( !wxstr.Left( pos ).ToULong( &id ) ) return false;
            
            // Get the remaining sub string
            wxstr = wxstr.Right( wxstr.Length() - pos - 1 );
            
            dmElement *pElement;
            if ( !gpctrlObj->m_dm.getDatabaseRecord( id, pElement ) ) return false;
            
            var.setStockVariable();
            var.setPersistent( false );
            
            if ( wxstr == _("id") ) {
                var.setAccessRights( PERMISSON_OWNER_READ );;           
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("bEnable") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( pElement->m_bEnable );
            }
            else if ( wxstr == _("groupid") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pElement->m_strGroupID );
            }
            else if ( wxstr == _("mask.priority") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( pElement->m_vscpfilter.mask_priority );
            }
            else if ( wxstr == _("mask.class") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( pElement->m_vscpfilter.mask_class );
            }
            else if ( wxstr == _("mask.type") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( pElement->m_vscpfilter.mask_type );
            }
            else if ( wxstr == _("mask.guid") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pElement->m_vscpfilter.mask_GUID );
            }
            else if ( wxstr == _("filter.priority") ) {
                cguid guid;
                guid.getFromArray( pElement->m_vscpfilter.mask_GUID );
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( guid.getAsString() );
            }
            else if ( wxstr == _("filter.class") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( pElement->m_vscpfilter.filter_class );
            }
            else if ( wxstr == _("filter.type") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( pElement->m_vscpfilter.filter_type );
            }
            else if ( wxstr == _("filter.guid") ) {
                cguid guid;
                guid.getFromArray( pElement->m_vscpfilter.filter_GUID );
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( guid.getAsString() );
            }
            else if ( wxstr == _("allowed.start") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("allowed.end") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("allowed.monday") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("allowed.tuesday") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );          
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("allowed.wednessday") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );           
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("allowed.thursday") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );         
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("allowed.friday") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );           
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("allowed.saturday") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );           
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("allowed.sunday") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("allowed.time") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("bCheckIndex") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("index") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("bCheckZone") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("zone") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("bCheckSubZone") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("subzone") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("measurement.value") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("measurement.unit") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("measurement.compare") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("measurement.compare.string") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("measurement.comment") ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );    
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("count.trigger") ) {
                var.setAccessRights( PERMISSON_OWNER_READ );          
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr == _("count.error") ) {
                var.setAccessRights( PERMISSON_OWNER_READ );          
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( (long)pElement->m_id );
            }
            else if ( ( wxstr == _("error") ) || ( wxstr == _("error.string") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );           
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( (long)pElement->m_id );
            }
            
        }
        else {
            // A full DM row is requested ("vscp.dm.35" for row with id = 35)
            //???
        }
    }
    
// *****************************************************************************
//                              Drivers
// *****************************************************************************
    
    
// *****************************************************************************
//                             Interfaces
// ***************************************************************************** 
    
    
// *****************************************************************************
//                              Discovery
// ***************************************************************************** 
    
    
// *****************************************************************************
//                                 Log
// *****************************************************************************    
    
    
// *****************************************************************************
//                                Tables
// *****************************************************************************    
    
    
// *****************************************************************************
//                                Users
// *****************************************************************************    
    
    
// *****************************************************************************
//                               Groups
// *****************************************************************************    
    
    
// ----------------------------- Not Found -------------------------------------    
    
    else {
        return false;   // Not a stock variable
    }
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// writeStockVariable
//

bool CVariableStorage::writeStockVariable( CVSCPVariable& var )
{
    wxString wxstr;
    wxString name = var.getName();
    
    // Make sure it starts with ".vscp"
    if ( !name.Lower().StartsWith("vscp.") ) return false;  
    
    if ( name.Lower() == _("vscp.version.major") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.minor") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.build") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.str") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.wxwidgets.str") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.wxwidgets.major") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.wxwidgets.minor") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.wxwidgets.release") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.wxwidgets.sub") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.copyright.vscp") ) {
        return false;   // None writable
    }

// *****************************************************************************
//                               wxWidgets
// *****************************************************************************

    else if ( name.Lower() == _("vscp.copyright.wxwidgets") ) {
        return false;   // None writable
    }

// *****************************************************************************
//                               Mongoose
// *****************************************************************************

    else if ( name.Lower() == _("vscp.copyright.mongoose") ) {
        return false;   // None writable
    }

// *****************************************************************************
//                                 LUA
// *****************************************************************************

#ifndef VSCP_DISABLE_LUA

    else if ( name.Lower() == _("vscp.copyright.lua") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.lua.major") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.lua.minor") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.lua.release") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.lua.str") ) {
        return false;   // None writable
    }

#endif

// *****************************************************************************
//                                SQLite
// *****************************************************************************

    else if ( name.Lower() == _("vscp.version.sqlite.major") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.sqlite.minor") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.sqlite.release") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.sqlite.build") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.version.sqllite.str") ) {
        return false;   // None writable
    }

// *****************************************************************************
//                                OPENSSL
// *****************************************************************************
    
#ifndef WIN32
    else if ( name.Lower() == _("vscp.version.openssl.str") ) {
        return false;   // None writable
    }
#endif
    else if ( name.Lower() == _("vscp.os.str") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.os.wordwidth") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.os.wordwidth.str") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.os.width.is64bit") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.os.width.is32bit") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.os.endiness.str") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.os.endiness.islittleendian") ) {
        return false;   // None writable
    }
   else if ( name.Lower() == _("vscp.host.fullname") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.host.ip") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.host.mac") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.host.userid") ) {
        return false;   // None writable
    }
    else if ( name.Lower() ==  _("vscp.host.username") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.host.guid") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.loglevel") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.loglevel.str") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.client.receivequeue.max") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.tcpip.address") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.udp.isenabled") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.udp.address") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.mqtt.broker.isenabled") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.mqtt.broker.address") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.coap.server.isenabled") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.coap.server.address") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.automation.heartbeat.isenabled") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.automation.heartbeat.period") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.automation.heartbeat.last") ) {
        return false;   // None writable
    }
    else if ( name.Lower() ==  _("vscp.automation.segctrl-heartbeat.isEnabled") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.automation.segctrl.heartbeat.period") ) {
        //???
    }  
    else if ( name.Lower() == _("vscp.automation.segctrl.heartbeat.last") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.automation.longitude") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.automation.latitude") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.automation.issendtwilightsunriseevent") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.automation.issendtwilightsunriseevent") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.automation.issendsunsetevent") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.automation.issendtwilightsunsetevent") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.workingfolder") ) {
        //???
    }
 

// *****************************************************************************
//                                WEB-Server
// *****************************************************************************


    else if ( name.Lower() == _("vscp.websrv.address") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.authenticationOn") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.root.path") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.authdomain") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.cert.path") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.extramimetypes") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.ssipatterns") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.ipacl") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.cgi.interpreter") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.cgi.pattern") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.directorylistings.enable") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.hidefile.pattern") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.indexfiles") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.urlrewrites") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.auth.file.directory") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.websrv.auth.file.global") ) {
        //???
    }


// *****************************************************************************
//                            Websocket-Server
// *****************************************************************************

    else if ( name.Lower() == _("vscp.websocket.auth.enable") ) {
        //???
    }


// *****************************************************************************
//                            Decision Matrix
// *****************************************************************************


    else if ( name.Lower() == _("vscp.dm.logging.enable") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.dm.logging.path") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.dm.static.path") ) {
         //???
    }


// *****************************************************************************
//                             Variables
// *****************************************************************************


    else if ( name.Lower() == _("vscp.variable.path") ) {
        //???
    }


// *****************************************************************************
//                              Log files
// *****************************************************************************

    // Enable database logging
    else if ( name.Lower() == _("vscp.log.database.enable") ) {
        //???
    }
    
    // General
    else if ( name.Lower() == _("vscp.log.general.enable") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.log.general.path") ) {
        //???
    }

    // Access
    else if ( name.Lower() == _("vscp.log.access.enable") ) {
        //???
    }
    else if ( name.Lower() == _("vscp.log.access.path") ) {
        //???
    }

    // Security
    else if ( name.Lower() == _("vscp.log.security.enable") ) {
        //???
    }   
    else if ( name.Lower() == _("vscp.log.security.path") ) {
        //???
    }
    

    
// *****************************************************************************
//                             Databases
// *****************************************************************************    
    
    
    else if ( name.Lower() ==  _("vscp.database.log.path") ) {
        //???
    }    
    else if ( name.Lower() ==  _("vscp.database.vscpdata.path") ) {
        //???
    } 
    else if ( name.Lower() ==  _("vscp.database.daemon.path") ) {
        //???
    }
    
    
// *****************************************************************************
//                             Decision Matrix
// *****************************************************************************
    else if ( name.Lower() == _("vscp.dm.count") ) {
        return false;   // None writable
    }
    else if ( name.Lower() == _("vscp.dm.count.active") ) {
        return false;   // None writable      
    }
    else if ( wxNOT_FOUND != name.Lower().Find( _("vscp.dm.") ) ) {
        
        int pos;
        dmElement dmDB;
        wxString wxstr;
        wxstr = name.Right( name.Length() - 8 );    // remove "vscp.dm."
        if ( wxNOT_FOUND != ( pos = wxstr.Lower().Find( _(".") ) ) ) {
            
            // this is a sub variable.
            wxString strID;
            unsigned long id;
            
            // Get id
            if ( !wxstr.Left( pos ).ToULong( &id ) ) return false;
            
            // get the sub string
            wxstr = wxstr.Right( wxstr.Length() - pos - 1 );
            
            // Get a pointer to memory DM row if any
            dmElement *pdm = 
                    gpctrlObj->m_dm.getMemoryElementFromId( var.getID() ); 
            
            
            
            // Get the DM record
            if ( !gpctrlObj->m_dm.getDatabaseRecord( id, &dmDM ) ) return false;
                      
            if ( wxstr == _("id") ) {
                return false;   // None writable
            }
            else if ( wxstr == _("bEnable") ) {
                
                bool bVal;
                wxString strFieldName,strValue;
                
                strFieldName = _("bEnable");                
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );                  
            }
            else if ( wxstr == _("groupid") ) {
  
                wxString strFieldName,strValue;
                
                strFieldName = _("groupid");
                var.getValue( &strValue );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("mask.priority") ) {

                int val;
                wxString strFieldName,strValue;

                strFieldName = _("maskPriority");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("mask.class") ) {
                
                int val;
                wxString strFieldName,strValue;
                
                strFieldName = _("maskClass");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("mask.type") ) {
                
                int val;
                wxString strFieldName,strValue;
                
                strFieldName = _("maskType");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("mask.guid") ) {
                
                wxString strFieldName,strValue;
                strFieldName = _("maskGUID");
                var.getValue( &strValue );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("filter.priority") ) {
                
                int val;
                wxString strFieldName,strValue;
                
                strFieldName = _("filterPriority");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("filter.class") ) {
                
                int val;
                wxString strFieldName,strValue;
                
                strFieldName = _("filterClass");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("filter.type") ) {
                
                int val;
                wxString strFieldName,strValue;
                
                strFieldName = _("filterTYpe");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("filter.guid") ) {
                
                wxString strFieldName,strValue;
                
                strFieldName = _("filterGUID");
                var.getValue( &strValue );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.start") ) {
                
                wxString strFieldName,strValue;
                
                strFieldName = _("allowedStart");
                var.getValue( &strValue );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.end") ) {
                
                wxString strFieldName,strValue;
                
                strFieldName = _("allowedEnd");
                var.getValue( &strValue );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.monday") ) {
                
                bool bVal;
                wxString strFieldName,strValue;
                
                strFieldName = _("allowedMonday");                
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.tuesday") ) {
                
                bool bVal;
                wxString strFieldName,strValue;
                
                strFieldName = _("allowedTuesday");
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.wednessday") ) {
                
                bool bVal;
                wxString strFieldName,strValue;
                
                strFieldName = _("allowedWednessday");                
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.thursday") ) {
                
                bool bVal;
                wxString strFieldName,strValue;
                
                strFieldName = _("allowedThursday");                
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.friday") ) {
                
                bool bVal;
                wxString strFieldName,strValue;
                
                strFieldName = _("allowedFriday");                
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.saturday") ) {
                
                bool bVal;
                wxString strFieldName,strValue;
                
                strFieldName = _("allowedSaturday");                
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.sunday") ) {
                
                bool bVal;                
                wxString strFieldName,strValue;
                
                strFieldName = _("allowedSunday");                
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.time") ) {
                
                wxString strFieldName,strValue;
                
                strFieldName = _("allowedTime");
                var.getValue( &strValue );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("bCheckIndex") ) {
                
                bool bVal;                
                wxString strFieldName,strValue;
                
                strFieldName = _("bCheckIndex");                
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("index") ) {
                
                int val;
                
                wxString strFieldName,strValue;
                strFieldName = _("index");
                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("bCheckZone") ) {
                
                bool bVal;
                wxString strFieldName,strValue;
                
                strFieldName = _("bCheckZone");
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("zone") ) {
                
                int val;
                wxString strFieldName,strValue;
                
                strFieldName = _("zone");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("bCheckSubZone") ) {
                
                bool bVal;
                wxString strFieldName,strValue;
                
                strFieldName = _("bCheckSubZone");                
                var.getValue( &bVal );
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("subzone") ) {
                
                int val;
                wxString strFieldName,strValue;
                
                strFieldName = _("subzone");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("measurement.value") ) {
                
                double val;
                wxString strFieldName,strValue;
                
                strFieldName = _("measurementValue");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("measurement.unit") ) {
                
                int val;
                wxString strFieldName,strValue;
                
                strFieldName = _("measurementUnit");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("measurement.compare") ) {
                
                int val;
                wxString strFieldName,strValue;
                
                strFieldName = _("measurementCompare");                
                var.getValue( &val );
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("measurement.compare.string") ) {
                return false;
            }
            else if ( wxstr == _("comment") ) {
                
                wxString strFieldName,strValue;
                
                strFieldName = _("comment");
                var.getValue( &strValue );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    strFieldName,
                                                                    strValue );
            }
            else if ( wxstr == _("count.trigger") ) {
                dmElement *pdm;
                if ( NULL == 
                        ( pdm = gpctrlObj->m_dm.getMemoryElementFromId( var.getID() ) ) ) {
                    return false;
                }
                
                pdm->m_triggCounter = 0;
                
                /*
                CVSCPVariable stockvar;
                if ( ! findStockVariable( var.getName(), stockvar ) ) {
                    return false;   // None writable
                }
                */
            }
            else if ( wxstr == _("count.error") ) {
                dmElement *pdm;
                if ( NULL == 
                        ( pdm = gpctrlObj->m_dm.getMemoryElementFromId( var.getID() ) ) ) {
                    return false;
                }
                
                pdm->m_errorCounter = 0;
            }
            else if ( ( wxstr == _("error") ) || ( wxstr == _("error.string") ) ) {
                return false;   // None writable
            }
            
        }
        else {
            // A full DM row is written ("vscp.dm.35" for row with id = 35)
            // a new row have id set to zero
            
        }
    }
    
// *****************************************************************************
//                              Drivers
// *****************************************************************************
    
    
// *****************************************************************************
//                             Interfaces
// ***************************************************************************** 
    
    
// *****************************************************************************
//                              Discovery
// ***************************************************************************** 
    
    
// *****************************************************************************
//                                 Log
// *****************************************************************************    
    
    
// *****************************************************************************
//                                Tables
// *****************************************************************************    
    
    
// *****************************************************************************
//                                Users
// *****************************************************************************    
    
    
// *****************************************************************************
//                               Groups
// *****************************************************************************    
    
    
    
// ----------------------------- Not Found -------------------------------------    
    
    else {
        return false; // Not a stock variable
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// findNonPersistentVariable
//

uint32_t CVariableStorage::findNonPersistentVariable(const wxString& name, CVSCPVariable& variable )
{
    sqlite3_stmt *ppStmt;
    char psql[ 512 ];
    
    variable.setID( 0 );    // Invalid id
    
    sprintf( psql,
                "SELECT * FROM 'variableInt' WHERE name='%s'", 
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
        variable.setType( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_VALUE ) ) );
        variable.setPersistent( false );
        variable.setUserID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_LINK_TO_USER ) ); 
        variable.setGroupID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_LINK_TO_GROUP ) );
        variable.setAccessRights( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_NOTE ) ) );
    }
    
    sqlite3_finalize( ppStmt );
    
    return variable.getID();
}

///////////////////////////////////////////////////////////////////////////////
// findPersistentVariable
//

uint32_t CVariableStorage::findPersistentVariable(const wxString& name, CVSCPVariable& variable )
{
    sqlite3_stmt *ppStmt;
    char psql[ 512 ];
    
    variable.setID( 0 );    // Invalid id
    
    sprintf( psql,
                "SELECT * FROM 'variableEx' WHERE name='%s'", 
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
    
    return variable.getID();
}


varQuery *CVariableStorage::listPrepare( const uint8_t table, const wxString& search )
{
    varQuery *pq = NULL;
    
    
    if ( VARIABLE_STOCK == table ) {
        varQuery *pq = new varQuery;
        pq->table = VARIABLE_STOCK;
        pq->stockId = 0;
        pq->ppStmt = NULL;           
    }
    else if ( VARIABLE_INTERNAL == table ) {
        varQuery *pq = new varQuery;
        if ( pq != NULL ) return NULL;
        pq->table = VARIABLE_INTERNAL;
        pq->stockId = 0;
        pq->ppStmt = NULL;
        
        char *psql = sqlite3_mprintf( "SELECT * FROM \"variableint\" %s",
                                        (const char *)search.mbc_str() );
        
        if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_internal_variable,
                                            psql,
                                            -1,
                                            &pq->ppStmt,
                                            NULL ) ) {
            delete pq;
            return NULL;
        }
    }
    else if ( VARIABLE_EXTERNAL == table ) {
        varQuery *pq = new varQuery;
        if ( pq != NULL ) return NULL;
        pq->table = VARIABLE_EXTERNAL;
        pq->stockId = 0;
        pq->ppStmt = NULL;

        char *psql = sqlite3_mprintf( "SELECT * FROM \"variableext\" %s",
                                        (const char *)search.mbc_str() );
        
        if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_external_variable,
                                            psql,
                                            -1,
                                            &pq->ppStmt,
                                            NULL ) ) {
            delete pq;
            return NULL;
        }
    }
    
    return pq;
}

///////////////////////////////////////////////////////////////////////////////
// listItem
//

bool CVariableStorage::listItem( varQuery *pq, CVSCPVariable& variable )
{
    // Must be a valid pointer
    if ( NULL == pq ) return false;
    
    if ( VARIABLE_STOCK == pq->table  ) {
        if ( pq->stockId >= m_StockVariable.Count() ) return false;
        return findStockVariable( m_StockVariable[ pq->stockId ], variable );
    }
    else if ( VARIABLE_INTERNAL == pq->table  ) {
        if ( SQLITE_ROW != sqlite3_step( pq->ppStmt ) ) return false; 
        variable.setID( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_ID ) );
        variable.m_lastChanged.ParseDateTime( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_LASTCHANGE ) );
        variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_NAME ) ) );
        variable.setType( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_VALUE ) ) );
        variable.setPersistent( false );
        variable.setUserID( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_LINK_TO_USER ) ); 
        variable.setGroupID( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_LINK_TO_GROUP ) );
        variable.setAccessRights( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_INT_NOTE ) ) );
    }
    else if ( VARIABLE_EXTERNAL == pq->table  ) {
        if ( SQLITE_ROW != sqlite3_step( pq->ppStmt ) ) return false;
        variable.setID( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_ID ) );
        variable.m_lastChanged.ParseDateTime( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_LASTCHANGE ) );
        variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_NAME ) ) );
        variable.setType( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_VALUE ) ) );
        variable.setPersistent( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_PERSISTENT ) ? true : false );
        variable.setUserID( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_LINK_TO_USER ) );
        variable.setGroupID( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_LINK_TO_GROUP ) );
        variable.setAccessRights( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_EXT_NOTE ) ) );
    }
    else {
        return false;
    }
            
    return true;        
}

///////////////////////////////////////////////////////////////////////////////
// listFinalize
//

void CVariableStorage::listFinalize( varQuery *pq )
{
    // Must be a valid pointer
    if ( NULL == pq ) return;
    
    if ( VARIABLE_STOCK == pq->table  ) {
        // Nothing to do
    }
    else if ( VARIABLE_INTERNAL == pq->table  ) {
        if (NULL != pq->ppStmt ) sqlite3_finalize( pq->ppStmt );
    }
    else if ( VARIABLE_EXTERNAL == pq->table  ) {
        if (NULL != pq->ppStmt ) sqlite3_finalize( pq->ppStmt );
    }

    delete pq;
}

///////////////////////////////////////////////////////////////////////////////
// add
//

bool CVariableStorage::add( CVSCPVariable& var )
{
    uint32_t id = 0;
    char *zErrMsg = 0;
    
    // Stock variables can't be added.
    if ( var.getName().StartsWith("VSCP.") ) {
        return writeStockVariable( var );
    }

    // Update last changed timestamp
    var.setLastChangedToNow();

    if ( id = exist( var.getName() ) ) {
        char *sql = sqlite3_mprintf("UPDATE '%s' "
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
                (const char *) var.m_lastChanged.FormatISOCombined().mbc_str(),
                (const char *) var.getName().mbc_str(),
                var.getType(),
                (const char *) var.getValue().mbc_str(),
                var.isPersistent() ? 1 : 0,
                var.getUserID(),
                var.getGroupID(),
                var.getAccessRights(),
                (const char *) var.getNote().mbc_str(),
                id );

        if (SQLITE_OK != sqlite3_exec( var.isPersistent() ? m_db_vscp_external_variable : m_db_vscp_internal_variable, 
                                            sql, NULL, NULL, &zErrMsg)) {
            sqlite3_free(sql);
            return false;
        }

        sqlite3_free(sql);
    }
    else {
        char *sql = sqlite3_mprintf("INSERT INTO '%s' "
                "(lastchange,name,type,value,bPersistent,link_to_user,link_to_group,permission,note) "
                "VALUES ('%s','%s', '%d','%q','%d','%d','%d','%s','%q')",
                var.isPersistent() ? "variableEx" : "variableInt",
                (const char *) var.m_lastChanged.FormatISOCombined().mbc_str(),
                (const char *) var.getName().mbc_str(),
                var.getType(),
                (const char *) var.getValue().mbc_str(),
                var.isPersistent() ? 1 : 0,
                var.getUserID(),
                var.getGroupID(),
                var.getAccessRights(),
                (const char *) var.getNote().mbc_str() );

        if ( SQLITE_OK != sqlite3_exec( var.isPersistent() ? m_db_vscp_external_variable : m_db_vscp_internal_variable, 
                                            sql, NULL, NULL, &zErrMsg)) {
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

bool CVariableStorage::add( const wxString& name, 
                                    const wxString& value, 
                                    const uint8_t type,
                                    const uint32_t userid,
                                    const uint32_t groupid,
                                    const bool bPersistent,
                                    const uint32_t accessRights )
{
    CVSCPVariable variable;
            
    variable.setName( name );
    variable.setType( type );
    variable.setValue( value );
    variable.setPersistent( bPersistent );
    variable.setUserID( userid );
    variable.setGroupID( groupid );
    variable.setAccessRights( accessRights );

    return add( variable );
}

///////////////////////////////////////////////////////////////////////////////
// addWithStringType
//

bool CVariableStorage::add( const wxString& varName,
                                            const wxString& value,
                                            const wxString& strType,    
                                            const uint32_t userid,
                                            const uint32_t groupid,
                                            bool bPersistent,
                                            const uint32_t accessRights ) 
{
    uint8_t type = CVSCPVariable::getVariableTypeFromString( strType );
    return add( varName,
                    value,
                    type,
                    userid,
                    groupid,
                    bPersistent,
                    accessRights ); 
}

///////////////////////////////////////////////////////////////////////////////
// remove
//

bool CVariableStorage::remove( wxString& name ) 
{
    char *zErrMsg = 0;
    CVSCPVariable variable;
      
    if ( !find( name, variable ) ) {
        return false;
    }
    
    // If stock variable it can't be removed
    if ( variable.isStockVariable() ) {
        return false;
    }
    
    if ( variable.isPersistent() ) {
        char *sql = sqlite3_mprintf( "DELETE FROM \"idx_variableex\" "
                                        "WHERE idx_variableex='%d';",
                                    variable.getID() );
        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_external_variable, sql, NULL, NULL, &zErrMsg ) ) {
            sqlite3_free( sql );
            return false;
        }
        
        sqlite3_free( sql );                            
    }
    else {        
        char *sql = sqlite3_mprintf( "DELETE FROM \"idx_variableint\" "
                                        "WHERE idx_variableint='%d';",
                                    variable.getID() );
        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_internal_variable, sql, NULL, NULL, &zErrMsg ) ) {
            sqlite3_free( sql );
            return false;
        }
        
        sqlite3_free( sql ); 
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// remove
//

bool CVariableStorage::remove( CVSCPVariable& variable ) 
{
    return remove( variable.getName() );
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
    const char *psql = VSCPDB_VARIABLE_EXT_CREATE; 
    
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
    const char *psql = VSCPDB_VARIABLE_INT_CREATE;;
    
    // Check if database is open
    if ( NULL == m_db_vscp_internal_variable ) return false;
    
    if ( SQLITE_OK != sqlite3_exec(m_db_vscp_internal_variable, psql, NULL, NULL, NULL) ) {
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// load
//
// Read persistent variables from file
//

bool CVariableStorage::load( wxString& path  )
{
    wxString wxstr;
    unsigned long lval;
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
    
    // If a null path is supplied use the configured path
    if ( 0 == path.Length() ) {
        path = m_configPath;
    }
    
    if (!doc.Load( path ) ) {
        return false;
    }
    
    //::wxLogDebug ( _("Loading variables from: \n\t") + m_configPath );
    
    // start processing the XML file
    if ( !( doc.GetRoot()->GetName() != wxT("variables") ||
            doc.GetRoot()->GetName() != wxT("persistent") ) ) {
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {

        if (child->GetName() == wxT("variable")) {

            CVSCPVariable variable;

            // Mark last changed as now
            variable.setLastChangedToNow();

            // Get variable type - String is default           
            variable.setType( variable.getVariableTypeFromString( child->GetAttribute( wxT("type"), 
                                                                    wxT("string") ) ) ); 
            
            // Persistence
            wxstr = child->GetAttribute( wxT("persistent"), wxT("false") );
            wxstr.MakeUpper();
            if ( wxNOT_FOUND != wxstr.Find( _("TRUE") ) ) {
                variable.setPersistent( true ); 
            }
                    
            // userid
            if ( child->GetAttribute( wxT("userid"), wxT("0") ).ToULong( &lval ) ) {
                variable.setUserID( lval );
            }
            
            // groupid
            if ( child->GetAttribute( wxT("groupid"), wxT("0") ).ToULong( &lval ) ) {
                variable.setGroupID( lval );
            }
            
            // accessrights
            if ( child->GetAttribute( wxT("accessrights"), wxT("0") ).ToULong( &lval ) ) {
                variable.setAccessRights( lval );
            }
            
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
                    variable.setValue( subchild->GetNodeContent() );
                }
                else if (subchild->GetName() == wxT("note")) {
                    variable.setNote( subchild->GetNodeContent() );
                }

                subchild = subchild->GetNext();

            }

            // Add the variable
            add( variable );

        }

        child = child->GetNext();

    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// save
//
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

///////////////////////////////////////////////////////////////////////////////
// save
//
//

bool CVariableStorage::save( wxString& path, uint8_t whatToSave )
{   
    wxString str;

    if ( !wxFileName::IsFileWritable( path ) ) {
        return false;
    }

    wxFFileOutputStream *pFileStream = new wxFFileOutputStream( path );
    if ( NULL == pFileStream ) return false;
    if ( !pFileStream->IsOk() ) return false;

    pFileStream->Write("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n\n", 
                        strlen("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n\n") );

    // VSCP variables
    pFileStream->Write("<variables>\n\n", strlen("<variables>\n\n") );

    if ( ( NULL == m_db_vscp_internal_variable ) && 
            ( whatToSave & VARIABLE_INTERNAL ) ) {
        
        char *pErrMsg = 0;
        sqlite3_stmt *ppStmt;
        const char *psql = "SELECT * from variableint";        
    
        if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_internal_variable,
                                            psql,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
            return false;
        }
    
        while ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {
            
            CVSCPVariable variable;
            
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
            
            writeVariableToXmlFile( pFileStream, variable );
        }
        
        sqlite3_finalize( ppStmt );
    }
    
    if ( ( NULL == m_db_vscp_external_variable ) && 
            ( whatToSave & VARIABLE_EXTERNAL ) ) {
        
        char *pErrMsg = 0;
        sqlite3_stmt *ppStmt;
        const char *psql = "SELECT * from variableext";        
    
        if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_external_variable,
                                            psql,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
            return false;
        }
    
        while ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {
            
            CVSCPVariable variable;
            
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
            
            writeVariableToXmlFile( pFileStream, variable );
        }
        
        sqlite3_finalize( ppStmt );
    }

    // DM matrix information end
    pFileStream->Write("</variables>\n",strlen("</variables>\n"));

    // Close the file
    pFileStream->Close();

    // Variable saved
    m_lastSaveTime = wxDateTime::Now();

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// writeVariableToXmlFile
//
// Create the external variable database
//
//

bool CVariableStorage::writeVariableToXmlFile( wxFFileOutputStream *pFileStream,
                                                CVSCPVariable& variable ) 
{
    wxString str;
    wxString name = variable.getName();

    str.Printf(_("  <variable type=\"%d\" "), variable.getType());
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));

    if (variable.isPersistent()) {
        str.Printf(_(" persistent=\"true\" "));
    } 
    else {
        str.Printf(_(" persistent=\"false\" "));
    }
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));

    str.Printf(_(" userid=\"%d\" "), variable.getUserID());
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));

    str.Printf(_(" groupid=\"%d\" "), variable.getGroupID());
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));

    str.Printf(_(" accessrights=\"%d\" "), variable.getAccessRights());
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));

    str.Printf(_(">\n"), variable.getType());
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));

    // Write name
    pFileStream->Write("    <name>", strlen("    <name>"));
    str = variable.getName();
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
    pFileStream->Write("</name>\n", strlen("</name>\n"));

    // Write note
    pFileStream->Write("    <note>", strlen("    <note>"));
    str = variable.getNote();
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
    pFileStream->Write("</note>\n", strlen("</note>\n"));

    // Write value
    pFileStream->Write("    <value>", strlen("    <value>"));
    variable.writeValueToString(str);
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
    pFileStream->Write("</value>\n", strlen("</value>\n"));

    pFileStream->Write("  </variable>\n\n", strlen("  </variable>\n\n"));
  
    return true;
}


