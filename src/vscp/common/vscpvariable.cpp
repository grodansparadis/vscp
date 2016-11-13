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
#include <wx/regex.h>
#include <wx/base64.h>

#include <openssl/opensslv.h>
#include <openssl/crypto.h>

#include <slre.h>
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
    init();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPVariable::~CVSCPVariable( void )
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

void CVSCPVariable::init( void )
{
    m_id = 0;
    m_name.Empty();
    m_type = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    m_bPersistent = false;                  // Not persistent by default
    m_accessRights = PERMISSON_OWNER_ALL;   // Owner can do everything. 
    m_userid = USER_ID_ADMIN;               // Admin owns variable by default
    m_lastChanged = wxDateTime::Now();
    
    m_strValue.Empty();
    m_note.Empty();
    
    m_bStock = false;   // This is not a stock variable
}


///////////////////////////////////////////////////////////////////////////////
// fixName
//

void CVSCPVariable::fixName( void )
{
    m_name.Trim( false );
    m_name.Trim( true );
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
        else if ( 0 == str.Find( _("BLOB") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_BLOB;
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
        else if ( 0 == str.Find( _("DRIVER") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_DRIVER;
        }
        else if ( 0 == str.Find( _("USER") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_USER;
        }
        else if ( 0 == str.Find( _("FILTER") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_FILTER;
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
            
        case VSCP_DAEMON_VARIABLE_CODE_BLOB:
            return "BLOB";

        case VSCP_DAEMON_VARIABLE_CODE_DATE:
            return "Date";
            
        case VSCP_DAEMON_VARIABLE_CODE_TIME:
            return "Time";    
        
        case VSCP_DAEMON_VARIABLE_CODE_MIME:
            return "Mime";
            
        case VSCP_DAEMON_VARIABLE_CODE_HTML:
            return "Html";

        case VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT:
            return "Javascript";
            
        case VSCP_DAEMON_VARIABLE_CODE_JSON:
            return "Json";
            
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
            return "DMrow";   
            
        case VSCP_DAEMON_VARIABLE_CODE_DRIVER:
            return "Driver";
            
        case VSCP_DAEMON_VARIABLE_CODE_USER:
            return "User"; 
            
        case VSCP_DAEMON_VARIABLE_CODE_FILTER:
            return "Filter";    
            
        default:
            return "Unknown";
    }
}

///////////////////////////////////////////////////////////////////////////////
// isValueBase64Encoded
//

bool CVSCPVariable::isValueBase64Encoded( int type ) 
{
    if ( VSCP_DAEMON_VARIABLE_CODE_STRING == type || 
                VSCP_DAEMON_VARIABLE_CODE_BLOB == type || 
                VSCP_DAEMON_VARIABLE_CODE_MIME ==  type || 
                VSCP_DAEMON_VARIABLE_CODE_HTML == type || 
                VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT == type ||  
                VSCP_DAEMON_VARIABLE_CODE_JSON == type || 
                VSCP_DAEMON_VARIABLE_CODE_XML == type || 
                VSCP_DAEMON_VARIABLE_CODE_SQL == type || 
                VSCP_DAEMON_VARIABLE_CODE_LUA == type || 
                VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT == type || 
                VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1 == type || 
                VSCP_DAEMON_VARIABLE_CODE_DM_ROW == type || 
                VSCP_DAEMON_VARIABLE_CODE_DRIVER == type || 
                VSCP_DAEMON_VARIABLE_CODE_USER == type || 
                VSCP_DAEMON_VARIABLE_CODE_FILTER == type ) {
        
        return true;        
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// setType
//

bool CVSCPVariable::setType( const wxString& strType )
{
    unsigned long lval;
    
    if ( strType.ToULong( &lval ) ) {
        
        if ( lval > 0xffff ) return false;
        
        
    }
    else {
        
        if ( VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED == 
                ( lval = getVariableTypeFromString( strType ) ) ) {
            return false;
        }
            
    }
    
    // Set as numeric
    m_type = (uint16_t)lval;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAsString
//

wxString CVSCPVariable::getAsString( bool bShort )
{
    wxString wxstr;
    
    wxstr = m_name;
    wxstr += _(";");
    wxstr += wxString::Format(_("%u"), (unsigned short)m_type );
    wxstr += _(";");
    wxstr += wxString::Format(_("%lu"), (unsigned long)m_userid );
    wxstr += _(";");
    wxstr += wxString::Format(_("%lu"), (unsigned long)m_accessRights );
    wxstr += _(";");
    wxstr += m_bPersistent ? _("true") : _("false");
    wxstr += _(";");
    wxstr += m_lastChanged.FormatISOCombined();
    
    // Long format can be dangerous as it can give **VERY** long lines.
    if ( !bShort ) { 
        if ( isValueBase64Encoded( m_type ) ) {
            wxstr += _(";");
            wxstr += wxBase64Encode( m_strValue, m_strValue.Length() );
            wxstr += _(";");
            wxstr += wxBase64Encode( m_note, m_note.Length() );
        }
        else {
            wxstr += _(";");
            wxstr += m_strValue;
            wxstr += _(";");
            wxstr += wxBase64Encode( m_note, m_note.Length() );
        }
    }
    
    return wxstr;
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

bool CVSCPVariable::setName( const wxString& strName )
{
    wxString str = strName;
    str.Trim();
    
    if ( str.IsEmpty() ) {
        // Name can't be null
        gpctrlObj->logMsg( wxString::Format( _("Setting variable name: Variable name can't be empty.\n") ) );
        return false;
    }
    
    m_name = str;
    fixName();

    return true;
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
// setUserId
//

bool CVSCPVariable::setUserId( uint32_t userid )
{
    // Check for admin user
    if ( 0 == userid ) {
        m_userid = USER_ID_ADMIN;
        return true;
    }
    
    // Check if id is defined
    if ( !CUserItem::isUserInDB( userid ) ) return false;
    
    // Assign userid    
    m_userid = userid;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setUserIdFromUserName
//

bool CVSCPVariable::setUserIdFromUserName( wxString& strUser )
{
    long userid;
    
    // Check for admin user
    if ( gpctrlObj->m_admin_user == strUser.Upper() ) {
        m_userid = USER_ID_ADMIN;
        return true;
    }
    
    if ( !CUserItem::isUserInDB( strUser, &userid ) ) return false;
    
    m_userid = userid;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeValueToString
//

void CVSCPVariable::writeValueToString( wxString& strValueOut, bool bBase64 )
{
    wxString str;
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
  
        case VSCP_DAEMON_VARIABLE_CODE_BLOB:
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
            
        case VSCP_DAEMON_VARIABLE_CODE_DRIVER:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_USER:
            if ( bBase64 ) {
                strValueOut = wxBase64Encode( m_strValue.ToUTF8(), strlen( m_strValue.ToUTF8() ) );
            }
            else {
                strValueOut = m_strValue;
            }
            break;   
            
        case VSCP_DAEMON_VARIABLE_CODE_FILTER:
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
    pValue->ParseISOCombined( m_strValue );
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
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
            {
                long lval;
                if ( strValue.ToLong( &lval ) ) {
                    m_strValue = wxString::Format(_("%d"), (int)lval );
                }
                else {
                    m_strValue = _("0");
                }
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LONG:
            long lval;
            if ( strValue.ToLong( &lval ) ) {
                m_strValue = wxString::Format(_("%ld"), lval );
            }
            else {
                m_strValue = _("0");
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
            double dval;
            if ( strValue.ToDouble( &dval ) ) {
                m_strValue = wxString::Format(_("%f"), dval );
            }
            else {
                m_strValue = _("0");
            }
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
            
        case VSCP_DAEMON_VARIABLE_CODE_BLOB:
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;    
            
        case VSCP_DAEMON_VARIABLE_CODE_DRIVER:
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;    
            
        case VSCP_DAEMON_VARIABLE_CODE_USER:
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
            }
            else {
                m_strValue = strValue;
            }
            break;   
            
        case VSCP_DAEMON_VARIABLE_CODE_FILTER:
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
                m_strValue = wxString::FromUTF8( (const char *)pbuf, len );
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
// Format is: "variable name","type","persistence","user","rights","value","note"
//

bool CVSCPVariable::getVariableFromString( const wxString& strVariable, bool bBase64 )
{
    wxString strRights;             // User rights for variable               
    int      typeVariable;          // Type of variable;
    bool     bPersistent = false;   // Persistence of variable
    bool     brw = true;            // Writable    
    
    // Update last changed
    setLastChangedToNow();

    wxStringTokenizer tkz( strVariable, _(";") );

    // Name
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str.Trim();
        if ( str.Length() ) setName( tkz.GetNextToken() );
    }
    else {
        return false;	
    }

    // Type
    if ( tkz.HasMoreTokens() ) {
        getVariableTypeFromString( tkz.GetNextToken() );
    }
    else {
        return false;
    }

    // Persistence
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str = str.Upper();
        str.Trim();
        if ( str.Length() ) {
            if ( wxNOT_FOUND != str.Find( _("TRUE") ) ) {
                setPersistent( true );
            }
            else {
                setPersistent( false );
            }
        }
    }
    else {
        return false;
    }
    
    // User - Numeric or by name
    if ( tkz.HasMoreTokens() ) {
        unsigned long userid = 0;
        wxString str = tkz.GetNextToken();
        str.Trim();
        if ( str.Length() ) {
            if ( str.ToULong( &userid ) ) {
                setUserId( userid );
            }
            else {
                setUserIdFromUserName( str );
            }
        }
    }
    else {
        return false;
    }
    
    // Access rights
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
    
    
    
    // Get the value of the variable
    if ( tkz.HasMoreTokens() ) {
        setValueFromString( typeVariable, tkz.GetNextToken(), bBase64 );
    }
    else {
        return false;
    }
    
    // Get the note (if any)
    // Get the value of the variable
    if ( tkz.HasMoreTokens() ) {
        setNote( tkz.GetNextToken() );
    }
    
    return true;

}

///////////////////////////////////////////////////////////////////////////////
// setNote
//

void CVSCPVariable::setNote( const wxString& strNote, bool bBase64 )
{
    wxString wxstr = strNote;
    if ( bBase64 ) {
        wxstr = wxBase64Encode( strNote, strNote.Length() );
    }
    
    m_note = wxstr;
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
                
            case VSCP_DAEMON_VARIABLE_CODE_BLOB:
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
                
            case VSCP_DAEMON_VARIABLE_CODE_DRIVER:
                m_strValue = _("");
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_USER:
                m_strValue = _("");
                break; 
                
            case VSCP_DAEMON_VARIABLE_CODE_FILTER:
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
//                       V A R I A B L E   S T O R A G E
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
    m_xmlPath = wxStandardPaths::Get().GetConfigDir();
    m_xmlPath += _("/vscp/variable.xml");
#else
    m_xmlPath = _("/srv/vscp/variable.xml");
#endif  
    
#ifdef WIN32
    m_path_db_vscp_variable.Assign( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/variable.sqlite3") );
#else
    m_dbFilename.Assign( _("/srv/vscp/variable.sqlite3") );
#endif

    // We just read variables  
    m_lastSaveTime = wxDateTime::Now();

    
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
// init
//

bool CVariableStorage::init( void )
{
    wxString wxstr;
    CVSCPVariable variable;
    
    // * * * VSCP Daemon external variable database * * *
    
    wxstr.Printf( _("Path=%s\n"),
                            (const char *)m_dbFilename.GetFullPath().mbc_str()  );
            fprintf( stderr, wxstr.mbc_str() );
        
    // Check filename
    if ( m_dbFilename.IsOk() && m_dbFilename.FileExists() ) {

        if ( SQLITE_OK != sqlite3_open( (const char *)m_dbFilename.GetFullPath().mbc_str(),
                                            &m_db_vscp_external_variable ) ) {

            // Failed to open/create the database file
            fprintf( stderr, "VSCP Daemon external variable database could not be opened. - Will not be used.\n" );
            wxstr.Printf( _("Path=%s error=%s\n"),
                            (const char *)m_dbFilename.GetFullPath().mbc_str(),
                            sqlite3_errmsg( m_db_vscp_external_variable ) );
            fprintf( stderr, wxstr.mbc_str() );

        }

    }
    else {
        if ( m_dbFilename.IsOk() ) {
            
            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            fprintf( stderr, "VSCP Daemon external variable database does not exist - will be created.\n" );
            wxstr.Printf( _("Path=%s\n"), (const char *)m_dbFilename.GetFullPath().mbc_str() );
            fprintf( stderr, wxstr.mbc_str() );
            
            gpctrlObj->logMsg( wxString::Format( _("VSCP Daemon external variable database open  Err=%s\n"), 
                                                    sqlite3_errmsg( m_db_vscp_external_variable ) ) );
            
            if ( SQLITE_OK == sqlite3_open( (const char *)m_dbFilename.GetFullPath().mbc_str(),
                                            &m_db_vscp_external_variable ) ) {            
                // create the table.
                doCreateExternalVariableTable();
                
            }
            else {
                
                // Failed to create the variable database
                gpctrlObj->logMsg( wxString::Format( _("VSCP Daemon external variable database open  Err=%s\n"), sqlite3_errmsg( m_db_vscp_external_variable ) ) );
            
                
            }
            
        }
        else {
            fprintf( stderr, "VSCP Daemon external variable database path invalid - will not be used.\n" );
            wxstr.Printf(_("Path=%s\n"), m_dbFilename.GetFullPath().mbc_str() );
            fprintf( stderr, wxstr.mbc_str() );
        }

    }
    
    // * * * VSCP Daemon internal variable database - Always created in-memory * * *
    
    if ( SQLITE_OK == sqlite3_open( NULL, &m_db_vscp_internal_variable ) ) {
        // Should always be created
        doCreateInternalVariableTable();
    }
    else {
        // Failed to open/create the database file
        fprintf( stderr, "VSCP Daemon internal variable database could not be opened - Will not be used.\n" );
        wxstr.Printf( _("Error=%s\n"),
                            sqlite3_errmsg( m_db_vscp_internal_variable ) );
        fprintf( stderr, wxstr.mbc_str() );
        if ( NULL != m_db_vscp_internal_variable ) sqlite3_close( m_db_vscp_internal_variable );
        m_db_vscp_internal_variable = NULL;
    }
    
    //**************************************************************************        
    // Stock variables is added to the internal variable database for 
    // sorting/listing. !!!! Values are not read or written here !!!!
    //**************************************************************************        
    
    // Set common values 
    variable.setID( ID_NON_PERSISTENT );
    variable.setStockVariable( true );
    variable.setPersistent( false );
    variable.setUserId( USER_ID_ADMIN );
    variable.setLastChangedToNow();
    variable.setNote(_(""));
    
    // *************************************************************************
    //                             Version
    // *************************************************************************
        
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.major") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("VSCP major version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.minor") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("VSCP minor version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.sub") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("VSCP sub version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.build") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("VSCP build version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("VSCP version string."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.wxwidgets.major") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("wxWidgets major version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.wxwidgets.minor") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("wxWidgets minor version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.wxwidgets.release") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("wxWidgets release version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.wxwidgets.sub") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("wxWidgets sub version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.wxwidgets.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("wxWidgets version string."), false );
    addStockVariable( variable  );
    
    
    
#ifndef VSCP_DISABLE_LUA
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.lua.major") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("LUA major version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.lua.minor") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("LUA minor version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.lua.release") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("LUA release version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.lua.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("LUA version string."), false );
    addStockVariable( variable  );
   
#endif    
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.sqlite.major") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("SQLite3 major version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.sqlite.minor") );
    variable.setNote( _("SQLite3 minor version number."), false );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.sqlite.release") );
    variable.setNote( _("SQLite3 release version number."), false );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.sqlite.build") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("SQLite3 build version number."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.sqllite.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("SQLite3 version string."), false );
    addStockVariable( variable  );
      
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.openssl.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Open SSL version string."), false );
    addStockVariable( variable  );
    
    

    
    // *************************************************************************
    //                              Copyright
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.copyright.vscp") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("VSCP copyright."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.copyright.wxwidgets") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("wxWidgets copyright."), false );
    addStockVariable( variable  );
      
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.copyright.mongoose") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Mongoose copyright."), false );
    addStockVariable( variable  );

#ifndef VSCP_DISABLE_LUA
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.copyright.lua") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("LUA copyright."), false );
    addStockVariable( variable  );
    
#endif
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.copyright.sqlite") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("SQLite3 copyright."), false );
    addStockVariable( variable  );

    
    // *************************************************************************
    //                               System
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Operating system information string."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.width") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Operating system CPU word width (numerical)."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.width.is64bit") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("True if operating system CPU word width is 64-bit."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.width.is32bit") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("True if operating system CPU word width is 32-bit."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.width.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Operating system CPU word width (text form)."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.endiness.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Operating system endiness."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.islittleendian") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("True if Operating system endiness is little endian."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.name") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Name of host machine."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.ip") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("IP address for host machine."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.mac") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("MAC address for host machine."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.userid") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("The user the VSCP daemon is running as."), false ); 
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.username") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Full name for the user the VSCP daemon is running as."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.guid") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID );
    variable.setNote( _("GUID for host."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.loglevel") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Active log level (numerical form)."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.loglevel.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Active log level (text form)."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.client.receivequeue.max") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Maximum number of events in client receive queue."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.root.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Root path for VSCP daemon."), false );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                             TCP/IP
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.tcpip.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Address for VSCP daemon TCP/IP interface."), false );
    addStockVariable( variable  );
    

    
    // *************************************************************************
    //                             Multicast
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.multicast.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Address for VSCP daemon multicast TCP/IP interface (Default: 224.0.23.158)."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.multicast.ttl") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("TTL for VSCP daemon multicast TCP/IP interface (Default: 224.0.23.158)."), false );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                                UDP
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.udp.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon simple UDP interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.udp.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Address for VSCP daemon simple UDP interface."), false );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                                 MQTT
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.mqtt.broker.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon MQTT interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.mqtt.broker.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Address for VSCP daemon simple UDP interface."), false );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                               CoAP
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.coap.server.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon CoAP interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.coap.server.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Address for VSCP daemon CoAP interface."), false );
    addStockVariable( variable  );
    
        // *************************************************************************
    //                              Discovery
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.discovery.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon discovery."), false );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                              AUTOMATION
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.calc.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last VSCP daemon automation calculation."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.longitude") );
    variable.setNote( _("VSCP daemon automation longitude setting."), false );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.latitude") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
    variable.setNote( _("VSCP daemon automation latitude setting."), false );
    addStockVariable( variable  );
    
    
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.heartbeat.enable") );
    variable.setNote( _("Enable flag for VSCP daemon automation heartbeat event."), false );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.heartbeat.period") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Period in seconds for VSCP daemon automation heartbeat event."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.heartbeat.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation heart beat event."), false );
    addStockVariable( variable  );
    
    
    
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.segctrl-heartbeat.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation segment controller heartbeat event."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.segctrl-heartbeat.period") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Period in seconds for VSCP daemon automation heart beat event."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.segctrl-heartbeat.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation heart beat event."), false );
    addStockVariable( variable  );
    
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.daylength") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("VSCP daemon automation calculated daylength."), false );
    addStockVariable( variable  );
    
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.declination") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
    variable.setNote( _("VSCP daemon automation calculated sun declination."), false );
    addStockVariable( variable  );  
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.sun.max.altitude") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
    variable.setNote( _("VSCP daemon automation calculated max sun altitude."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.twilightsunriseevent.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation twilight sunrise event."), false );
    addStockVariable( variable  );
       
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.twilightsunriseevent.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation twilight sunrise event."), false );
    addStockVariable( variable  );
    
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.sunriseevent.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation sunrise event."), false );
    addStockVariable( variable  );
    
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.sunriseevent.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation sunrise event."), false );
    addStockVariable( variable  );
    
    
    
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.sunsetevent.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation sunset event."), false );
    addStockVariable( variable  );
    
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.sunsetevent.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation sunset event."), false );
    addStockVariable( variable  );
           
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.twilightsunsetevent.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation twilight sunset event."), false );
    addStockVariable( variable  );
    
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.twilightsunsetevent.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation twilight sunset event."), false );
    addStockVariable( variable  );
       
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.calculatednoonevent.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation calculated noon event."), false );
    addStockVariable( variable  );
    
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.calculatednoonevent.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation calculated noon event."), false );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                            Websocket-Server
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websocket.auth.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable/disable authentication on VSCP Daemon websocket interface."), false );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                                WEB-Server
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Address for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.authentication.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable authentication for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.root.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Root folder for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.authdomain") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Auth domain for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.cert.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Certification path for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.extramimetypes") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Extra mime types for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.ssipatterns") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("SSI patterns for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.ipacl") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("IP access control list for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.cgi.interpreter") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to CGI interpreter for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.cgi.pattern") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("CGI pattern for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.directorylistings.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable directory listings for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.hidefile.pattern") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Hide file pattern for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.indexfiles") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("List of indexfiles for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.urlrewrites") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("URL rewrites for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.auth.file.directory") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Authentication file directory for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.auth.file.global") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to global authentication file for VSCP daemon web interface."), false );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                            Decision Matrix
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.logging.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag for decision matrix logging (true for activated)."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.logging.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to decision matrix logging,"), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.db.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to decision matrix database."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.xml.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to decision matrix XML file to load at startup."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Total number of decision matrix rows."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.count.active") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Number of active decision matrix rows."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.dm") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Full decision matrix (all rows)."), false );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                             Variables
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.variable.xml.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to XML file to load variables from on startup."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.variable.db.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to variable database."), false );
    addStockVariable( variable  );    
    
    // *************************************************************************
    //                              Log files
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.syslog.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable syslog."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.database.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable logging to database."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.database.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.database.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to logging database."), false );
    addStockVariable( variable  );
    
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.general.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable general logging to a text file."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.general.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to general logging text file."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.access.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable access logging to a text file."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.access.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to access logging text file."), false );
    addStockVariable( variable  );
    
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.security.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable security logging to a text file."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.security.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to security logging text file."), false );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                             Databases
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.database.vscpdata.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to VSCP data database."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.database.vscpdconfig.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to VSCP daemon main database."), false );
    addStockVariable( variable  );
    
    
    
    // *************************************************************************
    //                              Drivers
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of loaded drivers."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.level1.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of Level I drivers."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.level2.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of level II drivers."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("All driver info."), false );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                             Interfaces
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.interface.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of active interfaces."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.interface") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All interfaces."), false );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                              Discovery
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.discovery.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of discovered units."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.discovery") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All discovered units."), false );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                                Users
    // *************************************************************************
    
    variable.setAccessRights( PERMISSON_ALL_READ );
    variable.setUserID( 0 );
    variable.setName( _("vscp.user.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of defined users."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_OWNER_ALL );
    variable.setUserID( 0 );
    variable.setName( _("vscp.user") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All defined users."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );
    variable.setUserID( 0 );
    variable.setName( _("vscp.user.names") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All defined users names."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_OWNER_WRITE );
    variable.setUserID( 0 );
    variable.setName( _("vscp.user.add") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Add a new user."), false );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                                Tables
    // *************************************************************************   
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.table.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of defined user tables."), false );
    addStockVariable( variable  );
    
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.table") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All user tables."), false );
    addStockVariable( variable  );
    
    return true;
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
    wxString lc_name = name.Lower();
    
    // Look for stock variables
    if ( lc_name.StartsWith("vscp.") ) {
        return getStockVariable( name, variable );
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
    
    return 0; // Not found
}

///////////////////////////////////////////////////////////////////////////////
// getStockVariable
//

bool CVariableStorage::getStockVariable(const wxString& name, CVSCPVariable& var )
{
    wxString wxstr;
    wxString strToken;
    wxString strrest;
    uint32_t id;
    
    var.init();
    
    wxString lcname= name.Lower();
    
    wxStringTokenizer tkz(lcname, ".");
    
    if ( !tkz.HasMoreTokens() ) return false;
    strToken = tkz.GetNextToken();
    
    // Make sure it starts with ".vscp"
    if ( "vscp" != strToken ) return false;  
    
    // Get the stock variable - NOTE!!! all will not be found here so all
    // variables will be let thru. But variable with id=0 are uninitialised.
    id = findNonPersistentVariable( name, var ); 
    
    if ( lcname == _("vscp.version.major") ) {
        var.setValue( wxString::Format( _("%d"), VSCPD_MAJOR_VERSION ) );
    }
    else if ( lcname == _("vscp.version.minor") ) {
        var.setValue( wxString::Format( _("%d"), VSCPD_MINOR_VERSION ) );
    }
    else if ( lcname == _("vscp.version.build") ) {
        var.setValue( wxString::Format( _("%d"), VSCPD_BUILD_VERSION ) );
    }
    else if ( lcname == _("vscp.version.str") ) {
        var.setValue( wxString::Format( _("%s"), VSCPD_DISPLAY_VERSION ) );
    }
    else if ( lcname == _("vscp.version.wxwidgets.str") ) {
        var.setValue( wxString::Format( _("%s"), wxVERSION_STRING ) );
    }
    else if ( lcname == _("vscp.version.wxwidgets.major") ) {
        var.setValue( wxString::Format( _("%d"), wxMAJOR_VERSION ) );
    }
    else if ( lcname == _("vscp.version.wxwidgets.minor") ) {
        var.setValue( wxString::Format( _("%d"), wxMINOR_VERSION ) );
    }
    else if ( lcname == _("vscp.version.wxwidgets.release") ) {
        var.setValue( wxString::Format( _("%d"), wxRELEASE_NUMBER ) );
    }
    else if ( lcname == _("vscp.version.wxwidgets.sub") ) {
        var.setValue( wxString::Format( _("%d"), wxSUBRELEASE_NUMBER ) );
    }
    else if ( lcname == _("vscp.copyright.vscp") ) {
        var.setValue( wxString::Format( _("%s"), VSCPD_COPYRIGHT ) );
    }

// *****************************************************************************
//                               wxWidgets
// *****************************************************************************

    else if ( lcname == _("vscp.copyright.wxwidgets") ) {
        var.setValue( wxString::Format( _("%s"),
                    "Copyright (c) 1998-2005 Julian Smart, Robert Roebling et al" ) );
    }

// *****************************************************************************
//                               Mongoose
// *****************************************************************************

    else if ( lcname == _("vscp.copyright.mongoose") ) {
        var.setValue( wxString::Format( _("%s"),
                    "Copyright (c) 2013-2015 Cesanta Software Limited" ) );
    }

// *****************************************************************************
//                                 LUA
// *****************************************************************************

#ifndef VSCP_DISABLE_LUA

    else if ( lcname == _("vscp.copyright.lua") ) {
        var.setValue( wxString::Format( _("%s"), LUA_COPYRIGHT ) );
    }
    else if ( lcname == _("vscp.version.lua.major") ) {
        var.setValue( wxString::Format( _("%s"), LUA_VERSION_MAJOR ) );
    }
    else if ( lcname == _("vscp.version.lua.minor") ) {
        var.setValue( wxString::Format( _("%s"), LUA_VERSION_MINOR) );
    }
    else if ( lcname == _("vscp.version.lua.release") ) {
        var.setValue( wxString::Format( _("%s"), LUA_VERSION_RELEASE ) );
    }
    else if ( lcname == _("vscp.version.lua.str") ) {
        var.setValue( wxString::Format( _("%s.%s.%s"),
                                    LUA_VERSION_MAJOR,
                                    LUA_VERSION_MINOR,
                                    LUA_VERSION_RELEASE ) );
    }

#endif

    // *************************************************************************
    //                                SQLite
    // *************************************************************************

    else if ( lcname == _("vscp.version.sqlite.major") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setValue( wxString::Format( _("%d"), major ) );
    }
    else if ( lcname == _("vscp.version.sqlite.minor") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setValue( wxString::Format( _("%d"), minor ) );
    }
    else if ( lcname == _("vscp.version.sqlite.release") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setValue( wxString::Format( _("%d"), sub ) );
    }
    else if ( lcname == _("vscp.version.sqlite.build") ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setValue( wxString::Format( _("%d"), build ) );
    }
    else if ( lcname == _("vscp.version.sqllite.str") ) {
        var.setValue( _(SQLITE_VERSION) );
    }

    // *************************************************************************
    //                                OPENSSL
    // *************************************************************************
    

    else if ( lcname == _("vscp.version.openssl.str") ) {
#ifdef WIN32        
        var.setValue( _(OPENSSL_VERSION_TEXT) );
#else       
        // http://linux.die.net/man/3/ssleay
        //var.setValue( _("OPENSSL_VERSION_NUMBER") );
        wxstr.Printf( _("%s"), SSLeay_version( SSLEAY_VERSION ) );
        var.setValue( wxstr );
#endif        
        
    }
    
    // *************************************************************************
    //                               System
    // *************************************************************************
    
    else if ( lcname == _("vscp.os.str") ) {
        var.setValue( wxGetOsDescription() );
    }
    else if ( lcname == _("vscp.os.width") ) {
        var.setValue( wxIsPlatform64Bit() ? 64 : 32 );
    }
    else if ( lcname == _("vscp.os.width.str") ) {
        var.setValue( wxIsPlatform64Bit() ? _("64") : _("32") );
    }
    else if ( lcname == _("vscp.os.width.is64bit") ) {
        var.setValue( wxIsPlatform64Bit() ? true : false );
    }
    else if ( lcname == _("vscp.os.width.is32bit") ) {
        var.setValue( !wxIsPlatform64Bit() ? true : false );
    }
    else if ( lcname == _("vscp.os.endiness.str") ) {
        var.setValue( wxIsPlatformLittleEndian() ? _("Little endian") : _("Big endian") );
    }
    else if ( lcname == _("vscp.os.endiness.islittleendian") ) {
        var.setValue( wxIsPlatformLittleEndian() ? true : false );
    }
   else if ( lcname == _("vscp.host.name") ) {
        var.setValue( wxGetFullHostName() );
    }
    else if ( lcname == _("vscp.host.ip") ) {
        cguid guid;
 
        if ( !gpctrlObj->getIPAddress( guid ) ) {
            guid.clear();
        }

        var.setValue( wxString::Format( _("%d.%d.%d.%d"),
                                                    guid.getAt( 11 ),
                                                    guid.getAt( 10 ),
                                                    guid.getAt( 9 ),
                                                    guid.getAt( 8 ) ) );
    }
    else if ( lcname == _("vscp.host.mac") ) {
        cguid guid;
        
        if ( !gpctrlObj->getMacAddress( guid ) ) {
            guid.clear();
        }

        var.setValue( wxString::Format( _("%02X:%02X:%02X:%02X:%02X:%02X"),
                            guid.getAt( 13 ),
                            guid.getAt( 12 ),
                            guid.getAt( 11 ),
                            guid.getAt( 10 ),
                            guid.getAt( 9 ),
                            guid.getAt( 8 ) ) );
    }
    else if ( lcname == _("vscp.host.userid") ) {
        var.setValue( wxGetUserId() );
    }
    else if ( lcname ==  _("vscp.host.username") ) {
        var.setValue( wxGetUserName() );
    }
    else if ( lcname == _("vscp.host.guid") ) {
        var.setValue( wxstr );
    }
    else if ( lcname == _("vscp.loglevel") ) {
        var.setValue( wxString::Format( _("%d "), gpctrlObj->m_logLevel ) );
    }
    else if ( lcname == _("vscp.loglevel.str") ) {
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
        var.setValue( wxstr );
    }
    else if ( lcname == _("vscp.client.receivequeue.max") ) {
        var.setValue( wxString::Format( _("%d"), 
                        gpctrlObj->m_maxItemsInClientReceiveQueue ) );
    }
    else if ( lcname ==  _("vscp.host.root.path") ) {
        var.setValue( gpctrlObj->m_rootFolder );
    }
    
    // *************************************************************************
    //                             TCP/IP
    // *************************************************************************
    
    else if ( lcname == _("vscp.tcpip.address") ) {
        var.setValue( gpctrlObj->m_strTcpInterfaceAddress );
    }
    
    // *************************************************************************
    //                              Discovery
    // *************************************************************************
    
    else if ( lcname == _("vscp.discovery.enable") ) {
        var.setValue( gpctrlObj->m_strTcpInterfaceAddress );
    }
    
    // *************************************************************************
    //                             Multicast
    // *************************************************************************
    
    else if ( lcname == _("vscp.multicast.address") ) {
        var.setValue( gpctrlObj->m_strMulticastAnnounceAddress );
    }
    else if ( lcname == _("vscp.multicast.ttl") ) {
        var.setValue( gpctrlObj->m_ttlMultiCastAnnounce );
    }
    
    // *************************************************************************
    //                                UDP
    // *************************************************************************
    
    else if ( lcname == _("vscp.udp.enable") ) {
        var.setValue( gpctrlObj->m_bUDP ? true : false );
    }
    else if ( lcname == _("vscp.udp.address") ) {
        var.setValue( gpctrlObj->m_strUDPInterfaceAddress );
    }
    
    // *************************************************************************
    //                                 MQTT
    // *************************************************************************
    
    else if ( lcname == _("vscp.mqtt.broker.enable") ) {
        var.setValue( gpctrlObj->m_bMQTTBroker ? _("true") : _("false") );
    }
    else if ( lcname == _("vscp.mqtt.broker.address") ) {
        var.setValue( gpctrlObj->m_strMQTTBrokerInterfaceAddress );
    }
    
    
    // *************************************************************************
    //                               CoAP
    // *************************************************************************
    
    
    else if ( lcname == _("vscp.coap.server.enable") ) {
        var.setValue( gpctrlObj->m_bCoAPServer ? _("true") : _("false") );
    }
    else if ( lcname == _("vscp.coap.server.address") ) {
        var.setValue( gpctrlObj->m_strCoAPServerInterfaceAddress );
    }
    
    // *************************************************************************
    //                              AUTOMATION
    // *************************************************************************
    
    else if ( lcname == _("vscp.automation.calc.last") ) {
        wxstr = gpctrlObj->m_automation.getLastCalculation().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getLastCalculation().FormatISOTime();
        var.setValue( wxstr );
    }
    
    else if ( lcname == _("vscp.automation.longitude") ) {
        var.setValue( wxString::Format( _("%f"), 
                        gpctrlObj->m_automation.getLongitude() ) );
    }
    else if ( lcname == _("vscp.automation.latitude") ) {
        var.setValue( wxString::Format( _("%f"), 
                        gpctrlObj->m_automation.getLatitude() ) );
    }
    
    
    else if ( lcname == _("vscp.automation.heartbeat.enable") ) {
        var.setValue( gpctrlObj->m_automation.isSendHeartbeat() ? true : false );
    }
    else if ( lcname == _("vscp.automation.heartbeat.period") ) {
        var.setValue( wxString::Format( _("%ld"), 
                        gpctrlObj->m_automation.getIntervalHeartbeat() ) );
    }
    else if ( lcname == _("vscp.automation.heartbeat.last") ) {
        wxstr = gpctrlObj->m_automation.getHeartbeatSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getHeartbeatSent().FormatISOTime();
        var.setValue( wxstr );
    }
    
    
    else if ( lcname ==  _("vscp.automation.segctrl-heartbeat.enable") ) {
        var.setValue(gpctrlObj->m_automation.isSendSegmentControllerHeartbeat() ? true : false );
    }
    else if ( lcname == _("vscp.automation.segctrl.heartbeat.period") ) {
        var.setValue( wxString::Format( _("%ld"), 
                gpctrlObj->m_automation.getIntervalSegmentControllerHeartbeat() ) );
    }  
    else if ( lcname == _("vscp.automation.segctrl.heartbeat.last") ) {
        wxstr = gpctrlObj->m_automation.getSegmentControllerHeartbeatSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getSegmentControllerHeartbeatSent().FormatISOTime();
        var.setValue( wxstr );
    }
    
    else if ( lcname == _("vscp.automation.daylength") ) {
        int hour,minute;
        double daylength = gpctrlObj->m_automation.getDayLength();
        gpctrlObj->m_automation.convert2HourMinute( daylength, &hour, &minute );
        wxstr.Printf(_("%02d:%02d"), hour, minute );
        var.setValue( wxstr );
    }
    
    else if ( lcname == _("vscp.automation.declination") ) {
        double declination = gpctrlObj->m_automation.getDeclination();
        wxstr.Printf(_("%f"), declination );
        var.setValue( wxstr );
    }
    
    else if ( lcname == _("vscp.automation.sun.max.altitude") ) {
        double maxalt = gpctrlObj->m_automation.getSunMaxAltitude();
        wxstr.Printf(_("%f"), maxalt );
        var.setValue( wxstr );
    }
    
    else if ( lcname == _("vscp.automation.twilightsunriseevent.enable") ) {
        var.setValue( gpctrlObj->m_automation.isSendSunriseTwilightEvent() ? true : false );
    }
    else if ( lcname == _("vscp.automation.twilightsunriseevent.last") ) {
        wxstr = gpctrlObj->m_automation.getSegmentControllerHeartbeatSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getSegmentControllerHeartbeatSent().FormatISOTime();
        var.setValue( wxstr );
    }
    
    
    else if ( lcname == _("vscp.automation.sunriseevent.enable") ) {
        var.setValue( gpctrlObj->m_automation.isSendSunriseEvent() ? true : false );
    }
    else if ( lcname == _("vscp.automation.sunriseevent.last") ) {
        wxstr = gpctrlObj->m_automation.getSunriseEventSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getSunriseEventSent().FormatISOTime();
        var.setValue( wxstr );
    }
    
    
    
    else if ( lcname == _("vscp.automation.sunsetevent.enable") ) {
        var.setValue( gpctrlObj->m_automation.isSendSunsetEvent() ? true : false );
    }
    else if ( lcname == _("vscp.automation.sunsetevent.last") ) {
        wxstr = gpctrlObj->m_automation.getSunsetEventSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getSunsetEventSent().FormatISOTime();
        var.setValue( wxstr );
    }
    
    
    
    else if ( lcname == _("vscp.automation.twilightsunsetevent.enable") ) {
        var.setValue( gpctrlObj->m_automation.isSendSunsetTwilightEvent() ? true : false );
    }
    else if ( lcname == _("vscp.automation.twilightsunsetevent.last") ) {
        wxstr = gpctrlObj->m_automation.getSunsetTwilightEventSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getSunsetTwilightEventSent().FormatISOTime();
        var.setValue( wxstr );
    }
    
    else if ( lcname == _("vscp.automation.calculatednoonevent.enable") ) {
        var.setValue( gpctrlObj->m_automation.isSendCalculatedNoonEvent() ? true : false );
    }
    else if ( lcname == _("vscp.automation.twilightsunsetevent.last") ) {
        wxstr = gpctrlObj->m_automation.getCalculatedNoonEventSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += gpctrlObj->m_automation.getCalculatedNoonEventSent().FormatISOTime();
        var.setValue( wxstr );
    }
    
 
    else if ( lcname == _("vscp.host.name") ) {
        var.setValue( gpctrlObj->m_strServerName );
    }
    
    // *************************************************************************
    //                            Websocket-Server
    // *************************************************************************

    else if ( lcname == _("vscp.websocket.auth.enable") ) {
        var.setValue( gpctrlObj->m_bAuthWebsockets ? true : false );
    }
 
    
    // *************************************************************************
    //                            Websocket-Server
    // *************************************************************************

    else if ( lcname == _("vscp.websocket.auth.enable") ) {
        var.setValue( gpctrlObj->m_bAuthWebsockets ? true : false );
    }
    

    // *************************************************************************
    //                                WEB-Server
    // *************************************************************************


    else if ( lcname == _("vscp.websrv.address") ) {
        var.setValue( gpctrlObj->m_strWebServerInterfaceAddress );
    }
    else if ( lcname == _("vscp.websrv.authentication.enable") ) {
        var.setValue( gpctrlObj->m_bDisableSecurityWebServer ? true : false );
    }
    else if ( lcname == _("vscp.websrv.root.path") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_pathWebRoot ) );
    }
    else if ( lcname == _("vscp.websrv.cert.path") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_pathCert ) );
    }
    else if ( lcname == _("vscp.websrv.extramimetypes") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_extraMimeTypes ) );
    }


    else if ( lcname == _("vscp.websrv.ssipatterns") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_ssi_pattern ) );
    }

    else if ( lcname == _("vscp.websrv.ipacl") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_ip_acl ) );
    }

    else if ( lcname == _("vscp.websrv.cgi.interpreter") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_cgiInterpreter ) );
    }

    else if ( lcname == _("vscp.websrv.cgi.pattern") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_cgiPattern ) );
    }
    else if ( lcname == _("vscp.websrv.directorylistings.enable") ) {
        var.setValue( ( NULL != 
            strstr( gpctrlObj->m_EnableDirectoryListings,"yes" ) ) ? _("true") : _("false") );
    }
    else if ( lcname == _("vscp.websrv.hidefile.pattern") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_hideFilePatterns ) );
    }
    else if ( lcname == _("vscp.websrv.indexfiles") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_indexFiles ) );
    }
    else if ( lcname == _("vscp.websrv.urlrewrites") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_urlRewrites ) );
    }
    else if ( lcname == _("vscp.websrv.auth.file.directory") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_per_directory_auth_file ) );
    }
    else if ( lcname == _("vscp.websrv.auth.file.global") ) {
        var.setValue( wxString::FromUTF8( gpctrlObj->m_global_auth_file ) );
    }


    


    // *************************************************************************
    //                            Decision Matrix
    // *************************************************************************


    else if ( lcname == _("vscp.dm.logging.enable") ) {
        var.setValue( gpctrlObj->m_dm.m_bLogEnable ? true : false );
    }
    else if ( lcname == _("vscp.dm.logging.path") ) {
        var.setValue( gpctrlObj->m_dm.m_logPath.GetFullPath() );
    }
    else if ( lcname == _("vscp.dm.db.path") ) {
        var.setValue( gpctrlObj->m_dm.m_path_db_vscp_dm.GetFullPath() ); 
    }
    else if ( lcname == _("vscp.dm.xml.path") ) {
        var.setValue( gpctrlObj->m_dm.m_staticXMLPath ); 
    }


    // *************************************************************************
    //                             Variables
    // *************************************************************************


    else if ( lcname == _("vscp.variable.db.path") ) {
        var.setValue( gpctrlObj->m_VSCP_Variables.m_dbFilename.GetFullPath() );
    }
    else if ( lcname == _("vscp.variable.xml.path") ) {
        var.setValue( gpctrlObj->m_VSCP_Variables.m_xmlPath );
    }


    // *************************************************************************
    //                              Log files
    // *************************************************************************

    // Enable syslog logging
    else if ( lcname == _("vscp.log.syslog.enable") ) {
        var.setValue( gpctrlObj->m_bLogToSysLog ? true : false );
    }
    
    // Enable database logging
    else if ( lcname == _("vscp.log.database.enable") ) {
        var.setValue( gpctrlObj->m_bLogToDatabase ? true : false );
    }
    
    else if ( lcname ==  _("vscp.log.database.path") ) {
        var.setValue( gpctrlObj->m_path_db_vscp_data.GetFullPath() );
    }
    
    // General
    else if ( lcname == _("vscp.log.general.enable") ) {
        var.setValue( gpctrlObj->m_bLogGeneralEnable ? true : false );
    }
    else if ( lcname == _("vscp.log.general.path") ) {
        var.setValue( gpctrlObj->m_logGeneralFileName.GetFullPath() );
    }

    // Access
    else if ( lcname == _("vscp.log.access.enable") ) {
        var.setValue( gpctrlObj->m_bLogAccessEnable ? true : false );
    }
    else if ( lcname == _("vscp.log.access.path") ) {
        var.setValue( gpctrlObj->m_logAccessFileName.GetFullPath() );
    }

    // Security
    else if ( lcname == _("vscp.log.security.enable") ) {
        var.setValue( gpctrlObj->m_bLogAccessEnable ? true : false );
    }   
    else if ( lcname == _("vscp.log.security.path") ) {
        var.setValue( gpctrlObj->m_logSecurityFileName.GetFullPath() );
    }


    
    // *************************************************************************
    //                             Databases
    // *************************************************************************    
    
        
    else if ( lcname ==  _("vscp.database.vscpdata.path") ) {
        var.setValue( gpctrlObj->m_path_db_vscp_data.GetFullPath() );
    } 
    else if ( lcname ==  _("vscp.database.vscpdconfig.path") ) {
        var.setValue( gpctrlObj->m_path_db_vscp_daemon.GetFullPath() );
    }
    
    // *************************************************************************
    //                             Decision Matrix
    // *************************************************************************
    
    
    else if ( lcname == _("vscp.dm.count") ) {
        var.setValue( (long)gpctrlObj->m_dm.getDatabaseRecordCount() );
    }
    else if ( lcname == _("vscp.dm.count.active") ) {
        var.setValue( gpctrlObj->m_dm.getMemoryElementCount() );      
    }
    else if ( lcname == _("vscp.dm") ) {
        var.setStockVariable();
        var.setPersistent( false );
        var.setAccessRights( PERMISSON_OWNER_READ );           
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        gpctrlObj->m_dm.getAllRows( wxstr );
        var.setValue( wxstr );      
    }
    else if ( wxNOT_FOUND != lcname.Find( _("vscp.dm.") ) ) {
        
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
            //???  TODO
        }
    }
    
    // *************************************************************************
    //                              Drivers
    // *************************************************************************
    // TODO
    
    // *************************************************************************
    //                             Interfaces
    // *************************************************************************
    // TODO  
    
    // *************************************************************************
    //                                Users
    // *************************************************************************
    else if ( lcname == _("vscp.user.count") ) {
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setValue( (long)gpctrlObj->m_userList.getUserCount() );
    }
    else if ( lcname == _("vscp.user") ) {
        gpctrlObj->m_userList.getAllUsers( wxstr );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxstr );      
    }
    else if ( lcname == _("vscp.user.names") ) {
        wxArrayString array;
        wxstr.Empty();
        
        gpctrlObj->m_userList.getAllUsers( array );
        for ( int i=0; i<array.Count(); i++ ) {
            wxstr += array[ i ];
            if ( i != ( array.Count()-1 ) ) wxstr += _(",");
        }
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxstr );      
    }
    // Individual user  (vscp.user.1 or vscp.user.1.name... )
    else if ( lcname.StartsWith("vscp.user.", &strrest ) ) {
        
        int pos;
        unsigned long idx;
        wxStringTokenizer tkz( strrest, _(".") );
        if ( !tkz.HasMoreTokens() ) return false;   
        
        wxString strToken = tkz.GetNextToken();         // Get idx
        if ( !strToken.ToULong( &idx ) ) return false;
        
        if ( !tkz.HasMoreTokens() ) {
            // vscp.user.n - return user record for index n
            if ( !gpctrlObj->m_userList.getUserAsString( idx, wxstr ) ) return false; 
            var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
            var.setValue( wxstr );
        }
        else {
            // vscp.user.n.field 
            CUserItem *pUserItem;
            if ( !gpctrlObj->m_userList.getUserAsString( idx, wxstr ) ) return false;       // Get name
            if ( NULL == ( pUserItem = 
                    gpctrlObj->m_userList.getUserItemFromOrdinal( idx ) ) ) return false;   // UserItem
            strToken = tkz.GetNextToken();  // Get field
            if ( _("userid") == strToken ) {
                wxstr = wxString::Format( _("%ld"), pUserItem->getUserID() );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( wxstr );
            }
            else if ( _("name") == strToken ) {
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pUserItem->getUser() );
            }
            else if ( _("password") == strToken ) {
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pUserItem->getPassword() );
            }
            else if ( _("fullname") == strToken ) {
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pUserItem->getFullname() );
            }
            else if ( _("filter") == strToken ) {
                var.setType( VSCP_DAEMON_VARIABLE_CODE_FILTER );
                var.setValue( wxstr );
            }
            else if ( _("rights") == strToken ) {
                // rights or rights.0..7
                if ( !tkz.HasMoreTokens() ) {
                    var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                    var.setValue( pUserItem->getUserRightsAsString() );
                }
                else {
                    strToken = tkz.GetNextToken();   // 0..7
                    if ( !strToken.ToULong( &idx ) ) return false;
                    if ( idx > 7 ) return false;
                    wxstr.Printf(_("%d"), idx );
                    var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                    var.setValue( wxstr );
                }
            }
            else if ( _("allowed") == strToken ) {
                //allowed.events or allowed.remotes
                if ( !tkz.HasMoreTokens() ) return false;
                strToken = tkz.GetNextToken();   // events/remotes
                if ( _("events") == strToken ) {
                    var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                    var.setValue( pUserItem->getAllowedEventsAsString() );
                }
                else if ( _("remotes") == strToken ) { 
                    var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                    var.setValue( pUserItem->getAllowedRemotesAsString() );
                }
                else {
                    return false;
                }
            }
            else if ( _("note") == strToken ) {
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pUserItem->getNote() );
            }
            
        }
        
    }
    
    // *************************************************************************
    //                                Tables
    // ************************************************************************* 
    // TODO 
    
    
    // ----------------------------- Not Found ---------------------------------
    
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
    wxString lcname = var.getName().Lower();
    
    // Make sure it starts with ".vscp"
    if ( !lcname.StartsWith("vscp.") ) return false;  
    
    if ( lcname == _("vscp.version.major") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.minor") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.build") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.str") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.wxwidgets.str") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.wxwidgets.major") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.wxwidgets.minor") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.wxwidgets.release") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.wxwidgets.sub") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.copyright.vscp") ) {
        return false;   // None writable
    }

// *****************************************************************************
//                               wxWidgets
// *****************************************************************************

    else if ( lcname == _("vscp.copyright.wxwidgets") ) {
        return false;   // None writable
    }

// *****************************************************************************
//                               Mongoose
// *****************************************************************************

    else if ( lcname == _("vscp.copyright.mongoose") ) {
        return false;   // None writable
    }

// *****************************************************************************
//                                 LUA
// *****************************************************************************

#ifndef VSCP_DISABLE_LUA

    else if ( lcname == _("vscp.copyright.lua") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.lua.major") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.lua.minor") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.lua.release") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.lua.str") ) {
        return false;   // None writable
    }

#endif

// *****************************************************************************
//                                SQLite
// *****************************************************************************

    else if ( lcname == _("vscp.version.sqlite.major") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.sqlite.minor") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.sqlite.release") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.sqlite.build") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.version.sqllite.str") ) {
        return false;   // None writable
    }

// *****************************************************************************
//                                OPENSSL
// *****************************************************************************
    
#ifndef WIN32
    else if ( lcname == _("vscp.version.openssl.str") ) {
        return false;   // None writable
    }
#endif
    
    
// *****************************************************************************
//                                 OS
// *****************************************************************************    
    
    
    else if ( lcname == _("vscp.os.str") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.os.wordwidth") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.os.wordwidth.str") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.os.width.is64bit") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.os.width.is32bit") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.os.endiness.str") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.os.endiness.islittleendian") ) {
        return false;   // None writable
    }
    
// *****************************************************************************
//                                 HOST
// *****************************************************************************    
    
    else if ( lcname == _("vscp.host.rootpath") ) {
        return false;
    }
    else if ( lcname == _("vscp.host.name") ) {
        gpctrlObj->m_strServerName = var.getValue();
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_servername"), 
                                                    wxString::Format( _("%s"), 
                                                    gpctrlObj->m_strServerName.mbc_str() ) );
    }
    else if ( lcname == _("vscp.host.ip") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.host.mac") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.host.userid") ) {
        return false;   // None writable
    }
    else if ( lcname ==  _("vscp.host.username") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.host.guid") ) {        
        gpctrlObj->m_guid.getFromString( var.getValue() );
    }
    else if ( lcname == _("vscp.loglevel") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_logLevel = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_LogLevel"), 
                                                    val ? _("1") : _("0") );      
    }
    else if ( lcname == _("vscp.loglevel.str") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.client.receivequeue.max") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_maxItemsInClientReceiveQueue = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd.maxqueue"), 
                                                    wxString::Format( _("%d"), val ) );
    }
    else if ( lcname == _("vscp.tcpip.address") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_strTcpInterfaceAddress = strval;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_TcpipInterface_Address"), 
                                                        strval );
    }
    else if ( lcname == _("vscp.udp.enable") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_bUDP = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_UdpSimpleInterface_Enable"), 
                                                        val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.udp.address") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_strUDPInterfaceAddress = strval;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_UdpSimpleInterface_Address"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.mqtt.broker.enable") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_bMQTTBroker = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_MqttBroker_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.mqtt.broker.address") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_strMQTTBrokerInterfaceAddress = strval;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_UdpSimpleInterface_Address"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.coap.server.enable") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_bCoAPServer = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_CoapServer_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.coap.server.address") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_strCoAPServerInterfaceAddress = strval;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_CoapServer_Address"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.automation.heartbeat.enable") ) {
        bool val;
        var.getValue( &val );
        val ? gpctrlObj->m_automation.enableHeartbeatEvent() : gpctrlObj->m_automation.disableHeartbeatEvent();
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Automation_HeartbeatEvent_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.automation.heartbeat.period") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_logLevel = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Automation_HeartbeatEvent_Interval"), 
                                                    wxString::Format(_("%d"), val ) );
    }
    else if ( lcname == _("vscp.automation.heartbeat.last") ) {
        return false;   // None writable
    }
    else if ( lcname ==  _("vscp.automation.segctrl-heartbeat.enable") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_logLevel = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_LogLevel"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.automation.segctrl.heartbeat.period") ) {
        long val;
        var.getValue( &val );
        gpctrlObj->m_automation.setSegmentControllerHeartbeatInterval( val );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Automation_SegmentControllerEvent_Interval"), 
                                                    wxString::Format(_("%ld"), val) );
    }  
    else if ( lcname == _("vscp.automation.segctrl.heartbeat.last") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.automation.longitude") ) {
        double val;
        var.getValue( &val );
        gpctrlObj->m_automation.setLongitude( val );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Automation_Longitude"), 
                                                    wxString::Format( _("%f"), gpctrlObj->m_automation.getLongitude() ) );
    }
    else if ( lcname == _("vscp.automation.latitude") ) {
        double val;
        var.getValue( &val );
        gpctrlObj->m_automation.setLatitude( val );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Automation_Latitude"), 
                                                    wxString::Format( _("%f"), gpctrlObj->m_automation.getLatitude() ) );
    }
    else if ( lcname == _("vscp.automation.twilightsunriseevent.enable") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_automation.enableSunRiseTwilightEvent( val );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Automation_SunriseTwilight_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.automation.twilightsunsetevent.enable") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_automation.enableSunSetTwilightEvent( val );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Automation_SunsetTwilight_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.automation.sunriseevent.enable") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_automation.enableSunRiseEvent( val );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Automation_Sunrise_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.automation.sunsetevent.enable") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_automation.enableSunSetEvent( val );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Automation_Sunset_Enable"),
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.workingfolder") ) {
        return false;   // Not writable
    }
 

// *****************************************************************************
//                                WEB-Server
// *****************************************************************************


    else if ( lcname == _("vscp.websrv.address") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_strWebServerInterfaceAddress = strval;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_Address"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.authentication.enable") ) {
        int val;
        var.getValue( &val );
        gpctrlObj->m_bDisableSecurityWebServer = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_Authentication_enable"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.websrv.root.path") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_pathWebRoot, 0, sizeof( gpctrlObj->m_pathWebRoot ) );
        memcpy( gpctrlObj->m_pathWebRoot, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_pathWebRoot)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_RootPath"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.cert.path") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_pathCert, 0, sizeof( gpctrlObj->m_pathCert ) );
        memcpy( gpctrlObj->m_pathCert, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_pathCert)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_PathCert"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.extramimetypes") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_extraMimeTypes, 0, sizeof( gpctrlObj->m_extraMimeTypes ) );
        memcpy( gpctrlObj->m_extraMimeTypes, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_extraMimeTypes)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_ExtraMimeTypes"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.ssipatterns") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_ssi_pattern, 0, sizeof( gpctrlObj->m_ssi_pattern ) );
        memcpy( gpctrlObj->m_ssi_pattern, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_ssi_pattern)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_SSIPattern"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.ipacl") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_ip_acl, 0, sizeof( gpctrlObj->m_ip_acl ) );
        memcpy( gpctrlObj->m_ip_acl, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_ip_acl)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_IpAcl"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.cgi.interpreter") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_cgiInterpreter, 0, sizeof( gpctrlObj->m_cgiInterpreter ) );
        memcpy( gpctrlObj->m_cgiInterpreter, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_cgiInterpreter)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_CgiInterpreter"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.cgi.pattern") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_cgiPattern, 0, sizeof( gpctrlObj->m_cgiPattern ) );
        memcpy( gpctrlObj->m_cgiPattern, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_cgiPattern)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_CgiPattern"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.directorylistings.enable") ) {
        int val;
        var.getValue( &val );
        strcpy( gpctrlObj->m_EnableDirectoryListings,
                val ? "yes" : "no" );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_EnableDirectoryListings"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.websrv.hidefile.pattern") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_hideFilePatterns, 0, sizeof( gpctrlObj->m_hideFilePatterns ) );
        memcpy( gpctrlObj->m_hideFilePatterns, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_hideFilePatterns)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_HideFilePatterns"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.indexfiles") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_indexFiles, 0, sizeof( gpctrlObj->m_indexFiles ) );
        memcpy( gpctrlObj->m_indexFiles, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_indexFiles)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_IndexFiles"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.urlrewrites") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_urlRewrites, 0, sizeof( gpctrlObj->m_urlRewrites ) );
        memcpy( gpctrlObj->m_urlRewrites, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_urlRewrites)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_UrlRewrites"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.auth.file.directory") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_per_directory_auth_file, 0, sizeof( gpctrlObj->m_per_directory_auth_file ) );
        memcpy( gpctrlObj->m_per_directory_auth_file, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_per_directory_auth_file)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_PerDirectoryAuthFile"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.websrv.auth.file.global") ) {
        wxString strval;
        strval = var.getValue();
        memset( gpctrlObj->m_global_auth_file, 0, sizeof( gpctrlObj->m_global_auth_file ) );
        memcpy( gpctrlObj->m_global_auth_file, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpctrlObj->m_global_auth_file)-1, strval.Length() ) );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Webserver_GlobalAuthFile"), 
                                                    strval );
    }


// *****************************************************************************
//                            Websocket-Server
// *****************************************************************************

    else if ( lcname == _("vscp.websocket.auth.enable") ) {
        bool val;
        var.getValue( &val );
        gpctrlObj->m_bAuthWebsockets = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_WebSocket_EnableAuth"), 
                                                    val ? _("1") : _("0") );
    }


// *****************************************************************************
//                            Decision Matrix
// *****************************************************************************


    else if ( lcname == _("vscp.dm.logging.enable") ) {
        bool val;
        var.getValue( &val );
        gpctrlObj->m_dm.m_bLogEnable = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_DM_Logging_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.dm.logging.path") ) {
        wxString strval;
        strval = var.getValue();
        var.setValue( gpctrlObj->m_dm.m_logPath.GetFullPath() );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_DM_Logging_Path"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.dm.xml.path") ) {
        wxString strval;
        strval = var.getValue();
        var.setValue( gpctrlObj->m_dm.m_staticXMLPath );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_DM_XML_Path"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.dm.db.path") ) {
        wxString strval;
        strval = var.getValue();
        var.setValue( gpctrlObj->m_dm.m_staticXMLPath );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_DM_DB_Path"), 
                                                    strval );
    }


// *****************************************************************************
//                             Variables
// *****************************************************************************

    else if ( lcname == _("vscp.variable.db.path") ) {
        wxString strval;
        strval = var.getValue();
        var.setValue( gpctrlObj->m_VSCP_Variables.m_dbFilename.GetFullPath() );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Variables_DB_Path"), 
                                                    strval );
    }
    else if ( lcname == _("vscp.variable.xml.path") ) {
        wxString strval;
        strval = var.getValue();
        var.setValue( gpctrlObj->m_VSCP_Variables.m_xmlPath );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Variables_XML_Path"), 
                                                    strval );
    }


// *****************************************************************************
//                              Log files
// *****************************************************************************

    // Enable syslog logging
    else if ( lcname == _("vscp.log.syslog.enable") ) {
        bool val;
        var.getValue( &val );
        gpctrlObj->m_bLogToSysLog = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_Syslog_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    
    // Enable database logging
    else if ( lcname == _("vscp.log.database.enable") ) {
        bool val;
        var.getValue( &val );
        gpctrlObj->m_bLogToDatabase = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_LogDB_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    
    else if ( lcname ==  _("vscp.log.database.path") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_path_db_vscp_data.SetPath( strval );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_GeneralLogFile_Path"), 
                                                    strval );
    } 
    
    // General
    else if ( lcname == _("vscp.log.general.enable") ) {
        bool val;
        var.getValue( &val );
        gpctrlObj->m_bLogGeneralEnable = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_GeneralLogFile_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.log.general.path") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_logGeneralFileName.SetPath( strval );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_GeneralLogFile_Path"), 
                                                    strval );
    }

    // Access
    else if ( lcname == _("vscp.log.access.enable") ) {
        bool val;
        var.getValue( &val );
        gpctrlObj->m_bLogAccessEnable = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_AccessLogFile_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    else if ( lcname == _("vscp.log.access.path") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_logAccessFileName.SetPath( strval );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_AccessLogFile_Path"), 
                                                    strval );
    }

    // Security
    else if ( lcname == _("vscp.log.security.enable") ) {
        bool val;
        var.getValue( &val );
        gpctrlObj->m_bLogAccessEnable = val;
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_SecurityLogFile_Enable"), 
                                                    val ? _("1") : _("0") );
    }   
    else if ( lcname == _("vscp.log.security.path") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_logSecurityFileName.SetPath( strval );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_SecurityLogFile_Path"), 
                                                    strval );
    }
    

    
// *****************************************************************************
//                             Databases
// *****************************************************************************    
    
    else if ( lcname ==  _("vscp.database.vscpdata.path") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_path_db_vscp_data.SetPath( strval );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_db_data_path"), 
                                                    strval );
    } 
    else if ( lcname ==  _("vscp.database.vscpdconfig.path") ) {
        wxString strval;
        strval = var.getValue();
        gpctrlObj->m_path_db_vscp_daemon.SetPath( strval );
        return gpctrlObj->updateConfigurationRecordItem( _("vscpd_db_vscpconf_path"), 
                                                    strval );
    }
    
    
// *****************************************************************************
//                             Decision Matrix
// *****************************************************************************
    else if ( lcname == _("vscp.dm.count") ) {
        return false;   // None writable
    }
    else if ( lcname == _("vscp.dm.count.active") ) {
        return false;   // None writable      
    }
    else if ( wxNOT_FOUND != lcname.Find( _("vscp.dm.") ) ) {
        
        int pos;
        short row;
        dmElement dmDB;
        wxString wxstr;
        wxstr = lcname.Right( lcname.Length() - 8 );    // remove "vscp.dm."
        if ( wxNOT_FOUND != ( pos = wxstr.Lower().Find( _(".") ) ) ) {
            
            // this is a sub variable.
            wxString strID;
            unsigned long id;
            
            // Get id
            if ( !wxstr.Left( pos ).ToULong( &id ) ) return false;
            
            // get the sub string
            wxstr = wxstr.Right( wxstr.Length() - pos - 1 );
                                 
            // Get the DM db record
            if ( !gpctrlObj->m_dm.getDatabaseRecord( id, &dmDB ) ) return false;
            
            // Get a pointer to memory DM row (if any)
            dmElement *pdm = 
                    gpctrlObj->m_dm.getMemoryElementFromId( var.getID(), &row );
                      
            if ( wxstr == _("id") ) {
                return false;   // None writable
            }
            else if ( wxstr == _("bEnable") ) {
                
                bool bVal;
                wxString strValue;
                
                var.getValue( &bVal );
                
                // Update database record                                
                bVal ? strValue=_("1") : _("0");
                if ( !gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                _("bEnable"),
                                                                strValue ) ) {
                    return false;
                }
                
                // if the record is in memory
                if ( NULL != pdm ) {
                    
                    pdm->m_bEnable = bVal;
                    
                    // if value is false the database row should be removed
                    if ( !bVal ) {
                        gpctrlObj->m_dm.removeMemoryElement( row );
                    }
                    
                }      
                
                // If true and not in memory the record should be added
                else {
                    dmElement *pdmnew = new dmElement();
                    if ( NULL!= pdmnew ) return false;
                    gpctrlObj->m_dm.getDatabaseRecord( dmDB.m_id, pdmnew );
                    gpctrlObj->m_dm.addMemoryElement( pdmnew );
                }
                              
            }
            else if ( wxstr == _("groupid") ) {
  
                wxString strValue;
                
                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_strGroupID = strValue;
                }
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("groupid"),
                                                                    strValue );
                
            }
            else if ( wxstr == _("mask.priority") ) {

                int val;
                wxString strValue;
               
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.mask_priority = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("maskPriority"),
                                                                    strValue );
            }
            else if ( wxstr == _("mask.class") ) {
                
                int val;
                wxString strValue;
                                
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.mask_class = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("maskClass"),
                                                                    strValue );
            }
            else if ( wxstr == _("mask.type") ) {
                
                int val;
                wxString strValue;
                               
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.mask_type = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("maskType"),
                                                                    strValue );
            }
            else if ( wxstr == _("mask.guid") ) {
                
                wxString strValue;

                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    cguid guid;
                    guid.getFromString( strValue );
                    memcpy( pdm->m_vscpfilter.mask_GUID, guid.getGUID(), 16 );
                }
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("maskGUID"),
                                                                    strValue );
            }
            else if ( wxstr == _("filter.priority") ) {
                
                int val;
                wxString strValue;
                
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.filter_priority = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("filterPriority"),
                                                                    strValue );
            }
            else if ( wxstr == _("filter.class") ) {
                
                int val;
                wxString strValue;
               
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.filter_class = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("filterClass"),
                                                                    strValue );
            }
            else if ( wxstr == _("filter.type") ) {
                
                int val;
                wxString strValue;
              
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.filter_type = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("filterType"),
                                                                    strValue );
            }
            else if ( wxstr == _("filter.guid") ) {
                
                wxString strValue;

                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    cguid guid;
                    guid.getFromString( strValue );
                    memcpy( pdm->m_vscpfilter.filter_GUID, guid.getGUID(), 16 );
                }
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("filterGUID"),
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.start") ) {
                
                wxString strValue;
                
                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {                    
                    pdm->m_timeAllow.m_fromTime.ParseISOCombined( strValue );
                }
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedStart"),
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.end") ) {
                
                wxString strValue;

                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.m_endTime.ParseISOCombined( strValue );
                }
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedEnd"),
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.monday") ) {
                
                bool bVal;
                wxString strValue;
                
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowMonday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedMonday"),
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.tuesday") ) {
                
                bool bVal;
                wxString strValue;

                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowTuesday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedTuesday"),
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.wednessday") ) {
                
                bool bVal;
                wxString strValue;
            
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowWednesday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedWednessday"),
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.thursday") ) {
                
                bool bVal;
                wxString strValue;
               
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowThursday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedThursday"),
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.friday") ) {
                
                bool bVal;
                wxString strValue;
             
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowFriday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedFriday"),
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.saturday") ) {
                
                bool bVal;
                wxString strValue;
                
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowSaturday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedSaturday"),
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.sunday") ) {
                
                bool bVal;                
                wxString strValue;
               
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowSunday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedSunday"),
                                                                    strValue );
            }
            else if ( wxstr == _("allowed.time") ) {
                
                wxString strValue;
 
                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.parseActionTime( strValue );
                }
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedTime"),
                                                                    strValue );
            }
            else if ( wxstr == _("bCheckIndex") ) {
                
                bool bVal;                
                wxString strValue;
              
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_bCheckIndex = bVal;
                }
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("bCheckIndex"),
                                                                    strValue );
            }
            else if ( wxstr == _("index") ) {
                
                int val;
                
                wxString strValue;
                
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_index = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("index"),
                                                                    strValue );
            }
            else if ( wxstr == _("bCheckZone") ) {
                
                bool bVal;
                wxString strValue;

                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_bCheckZone = bVal;
                }
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("bCheckZone"),
                                                                    strValue );
            }
            else if ( wxstr == _("zone") ) {
                
                int val;
                wxString strValue;
                              
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_zone = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("zone"),
                                                                    strValue );
            }
            else if ( wxstr == _("bCheckSubZone") ) {
                
                bool bVal;
                wxString strValue;
                             
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_bCheckZone = bVal;
                }
                bVal ? strValue=_("1") : _("0");
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("bCheckSubZone"),
                                                                    strValue );
            }
            else if ( wxstr == _("subzone") ) {
                
                int val;
                wxString strValue;
                               
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_subzone = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("subzone"),
                                                                    strValue );
            }
            else if ( wxstr == _("measurement.value") ) {
                
                double val;
                wxString strValue;
              
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_measurementValue = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("measurementValue"),
                                                                    strValue );
            }
            else if ( wxstr == _("measurement.unit") ) {
                
                int val;
                wxString strValue;
                              
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_measurementUnit = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("measurementUnit"),
                                                                    strValue );
            }
            else if ( wxstr == _("measurement.compare") ) {
                
                int val;
                wxString strValue;
                              
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_measurementUnit = val;
                }
                strValue.Format( _("%d"), val );
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("measurementCompare"),
                                                                    strValue );
            }
            else if ( wxstr == _("measurement.compare.string") ) {
                return false;
            }
            else if ( wxstr == _("comment") ) {
                
                wxString strValue;
                
                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_comment = strValue;
                }
                return gpctrlObj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("comment"),
                                                                    strValue );
            }
            else if ( wxstr == _("count.trigger") ) {
                if ( NULL != pdm ) {                
                    pdm->m_triggCounter = 0;
                }
            }
            else if ( wxstr == _("count.error") ) {
                if ( NULL == pdm ) {                
                    pdm->m_errorCounter = 0;
                }
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
    
    else if ( wxNOT_FOUND != lcname.Find( _("vscp.driver.") ) ) {
        
    }
    
// *****************************************************************************
//                             Interfaces
// ***************************************************************************** 
    
    else if ( wxNOT_FOUND != lcname.Find( _("vscp.interface.") ) ) {
        
    }
    
// *****************************************************************************
//                              Discovery
// ***************************************************************************** 
    
    else if ( wxNOT_FOUND != lcname.Find( _("vscp.discovery.") ) ) {
        
    }
    
// *****************************************************************************
//                                 Log
// *****************************************************************************    
    
    else if ( wxNOT_FOUND != lcname.Find( _("vscp.log.") ) ) {
        
    }
    
// *****************************************************************************
//                                Tables
// *****************************************************************************    
    
    else if ( wxNOT_FOUND != lcname.Find( _("vscp.table.") ) ) {
        
    }
    
// *****************************************************************************
//                                Users
// *****************************************************************************    
      
    else if ( wxNOT_FOUND != lcname.Find( _("vscp.user.") ) ) {
        
        wxStringTokenizer tkz( lcname, _(".") );
        wxString strToken;
        strToken = tkz.GetNextToken();  // vscp
        strToken = tkz.GetNextToken();  // user
        
        if ( !tkz.HasMoreTokens() ) return false;
        strToken = tkz.GetNextToken();  // vscp
        
        // Add new record
        if ( _("add") == strToken ) {
            if ( !gpctrlObj->m_userList.addUser( var.getValue() ) ) return false;
        }
        else {
            
            // Token should be a user ordinal number
            
            unsigned long idx;
            if ( !strToken.ToULong( &idx ) ) return false;
            
            CUserItem *pUserItem = gpctrlObj->m_userList.getUserItemFromOrdinal( idx );
            if ( NULL == pUserItem ) return false;
            
            if ( !tkz.HasMoreTokens() ) {
                // write
                pUserItem->setFromString( var.getValue() );
            }
            
            strToken = tkz.GetNextToken();
            
            if ( _("write") == strToken ) {
                pUserItem->setFromString( var.getValue() );
            }
            else if ( _("name") == strToken ) {
                pUserItem->setUser( var.getValue() );
            }
            else if ( _("password") == strToken ) {
                pUserItem->setPassword( var.getValue() );
            }
            else if ( _("fullname") == strToken ) {
                pUserItem->setFullname( var.getValue() );
            }
            else if ( _("filter") == strToken ) {
                pUserItem->setFilterFromString( var.getValue() );
            }
            else if ( _("rights") == strToken ) {
                if ( !tkz.HasMoreTokens() ) {
                    pUserItem->setUserRightsFromString( var.getValue() );
                }
                else {
                    unsigned long nRight, value;
                    strToken = tkz.GetNextToken();  // id
                    if ( !strToken.ToULong( &nRight ) ) return false;
                    if ( nRight > 7 ) return false;
                    if ( !var.getValue().ToULong( &value ) ) return false;
                    pUserItem->setUserRights( nRight, value );
                }
            }
            else if ( _("allowed") == strToken ) {
                if ( !tkz.HasMoreTokens() ) return false;
                strToken = tkz.GetNextToken();
                if ( _("events") == strToken ) {
                    pUserItem->setAllowedEventsFromString( var.getValue() );
                }
                else if ( _("remotes") == strToken ) {
                    pUserItem->setAllowedRemotesFromString( var.getValue() );
                }
                else {
                    return false;
                }
            }
            else if ( _("note") == strToken ) {
                pUserItem->setNote( var.getValue() );
            }
            else {
                return false;
            }
            
        }
        
    }
    
// ----------------------------- Not Found -------------------------------------    
    
    else {
        return false; // Not a stock variable
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// findNonPersistentVariable
//

uint32_t CVariableStorage::findNonPersistentVariable( const wxString& name, 
                                                        CVSCPVariable& variable )
{
    sqlite3_stmt *ppStmt;
    char psql[ 512 ];
    
    variable.setID( 0 );    // Invalid id
    
    sprintf( psql,
                VSCPDB_VARIABLE_FIND_FROM_NAME, 
                (const char *)name.Upper().mbc_str() );
    
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
        variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_ID ) );
        variable.m_lastChanged.ParseISOCombined( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
        variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) ) );
        variable.setType( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE ) ) );
        variable.setPersistent( false );
        variable.setUserID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) ); 
        variable.setAccessRights( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
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
                VSCPDB_VARIABLE_FIND_FROM_NAME, 
                (const char *)name.Upper().mbc_str() );
    
    if ( NULL == m_db_vscp_external_variable ) {
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
        variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_ID ) );
        variable.m_lastChanged.ParseISOCombined( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
        variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) ) );
        variable.setType( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE ) ) );
        variable.setPersistent( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERSISTENT ) ? true : false );
        variable.setUserID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) );
        variable.setAccessRights( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
    }
    
    sqlite3_finalize( ppStmt );
    
    return variable.getID();
}

///////////////////////////////////////////////////////////////////////////////
// getVarlistFromRegExp
//

bool CVariableStorage::getVarlistFromRegExp( wxArrayString& nameArray, 
                            const wxString& regex )
{
    wxString varname;
    sqlite3_stmt *ppStmt;
    wxString regexlocal = regex.Upper();
    //struct slre_cap caps[4];
        
    nameArray.Clear();
    
    regexlocal.Trim();
    if ( 0 == regexlocal.Length() ) regexlocal = _("(.*)");  // List all if empty
    wxRegEx wxregex( regexlocal );
    if ( !wxregex.IsValid() ) {
        nameArray.Empty();
        gpctrlObj->logMsg( wxString::Format( _("Invalid regular expression [%s]"), 
                            (const char *)regexlocal.mbc_str() ) );
        return false;
    }
        
    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_internal_variable,
                                            VSCPDB_VARIABLE_FIND_ALL,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        const unsigned char *p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME );
        varname = wxString::FromUTF8Unchecked( (const char *)p );
        if ( wxregex.Matches( varname ) ) {
        /*if ( slre_match( (const char *)regexlocal.mbc_str(),
                                        (const char *)varname.mbc_str(), 
                                        strlen( (const char *)varname.mbc_str() ), 
                                        caps, 4, 0) > 0) {*/
        
            nameArray.Add( varname );
        }
    }

    sqlite3_finalize( ppStmt );
        
    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_external_variable,
                                            VSCPDB_VARIABLE_FIND_ALL,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        sqlite3_finalize( ppStmt );
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        varname = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) );
        if ( wxregex.Matches( varname ) ) {
        /*if ( slre_match( (const char *)regexlocal.mbc_str(),
                                        (const char *)varname.mbc_str(), 
                                        strlen( (const char *)varname.mbc_str() ), 
                                        caps, 4, 0) > 0) {*/
            nameArray.Add( varname );
        }
    }

    sqlite3_finalize( ppStmt );
    
    // Sort the array
    nameArray.Sort();
        
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// listItem
//

bool CVariableStorage::listItem( varQuery *pq, CVSCPVariable& variable )
{
    // Must be a valid pointer
    if ( NULL == pq ) return false;
    
    //if ( VARIABLE_STOCK == pq->table  ) {
    //    if ( pq->stockId >= m_StockVariable.Count() ) return false;
    //    return getStockVariable( m_StockVariable[ pq->stockId ], variable );
    //}
    if ( VARIABLE_INTERNAL == pq->table  ) {
        if ( SQLITE_ROW != sqlite3_step( pq->ppStmt ) ) return false; 
        variable.setID( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_ID ) );
        variable.m_lastChanged.ParseISOCombined( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
        variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) ) );
        variable.setType( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE ) ) );
        variable.setPersistent( false );
        variable.setUserID( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) ); 
        variable.setAccessRights( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
    }
    else if ( VARIABLE_EXTERNAL == pq->table  ) {
        if ( SQLITE_ROW != sqlite3_step( pq->ppStmt ) ) return false;
        variable.setID( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_ID ) );
        variable.m_lastChanged.ParseISOCombined( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
        variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) ) );
        variable.setType( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE ) ) );
        variable.setPersistent( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_PERSISTENT ) ? true : false );
        variable.setUserID( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) );
        variable.setAccessRights( sqlite3_column_int( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( pq->ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
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
// addStockVariable
//

bool CVariableStorage::addStockVariable( CVSCPVariable& var )
{
    uint32_t id = 0;
    char *zErrMsg = 0;
    
    char *sql = sqlite3_mprintf( VSCPDB_VARIABLE_INSERT_STOCK,
                (const char *)var.m_lastChanged.Now().FormatISOCombined().mbc_str(),
                (const char *)var.getName().mbc_str(),
                var.getType(),
                (const char *)var.getValue().mbc_str(),
                0,  // not persistent
                0,  // admin
                var.getAccessRights(),
                (const char *)var.getNote().mbc_str() );

    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_internal_variable,  
                                        sql, NULL, NULL, &zErrMsg)) {
        sqlite3_free( sql );
        return false;
    }

    sqlite3_free( sql ); 
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// add
//

bool CVariableStorage::add( CVSCPVariable& var )
{
    uint32_t id = 0;
    char *zErrMsg = 0;
    
    // Must have a valid name
    if ( var.getName().IsEmpty() ) {
        gpctrlObj->logMsg( wxString::Format( _("Add variable: Variable name can't be empty.\n") ) );
        return false;
    }
    
    // Update last changed timestamp
    var.setLastChangedToNow();
    
    // Stock variables 
    if ( var.isStockVariable() ) {
        return addStockVariable( var );
    }    

    if ( id = exist( var.getName() ) ) {
        char *sql = sqlite3_mprintf( VSCPDB_VARIABLE_UPDATE,
                (const char *) var.m_lastChanged.FormatISOCombined().mbc_str(),
                (const char *) var.getName().mbc_str(),
                var.getType(),
                (const char *) var.getValue().mbc_str(),
                var.isPersistent() ? 1 : 0,
                var.getUserID(),
                var.getAccessRights(),
                (const char *) var.getNote().mbc_str(),
                id );

        if (SQLITE_OK != sqlite3_exec( ( var.isPersistent() ? m_db_vscp_external_variable : m_db_vscp_internal_variable ), 
                                            sql, NULL, NULL, &zErrMsg)) {            
            gpctrlObj->logMsg( wxString::Format( _("Add variable: Unable to update variable in db. [%s] Err=%s\n"), sql, zErrMsg ) );
            sqlite3_free(sql);
            return false;
        }

        sqlite3_free(sql);
    }
    else {
        char *sql = sqlite3_mprintf( VSCPDB_VARIABLE_INSERT,
                (const char *) var.m_lastChanged.Now().FormatISOCombined().mbc_str(),
                (const char *) var.getName().mbc_str(),
                var.getType(),
                (const char *) var.getValue().mbc_str(),
                var.isPersistent() ? 1 : 0,
                var.getUserID(),
                var.getAccessRights(),
                (const char *) var.getNote().mbc_str() );  

        if ( SQLITE_OK != sqlite3_exec( ( var.isPersistent() ? m_db_vscp_external_variable : m_db_vscp_internal_variable ), 
                                            sql, NULL, NULL, &zErrMsg ) ) {
            fprintf(stderr, "Error: %s\n", sqlite3_errmsg( m_db_vscp_external_variable ) );
            gpctrlObj->logMsg( wxString::Format( _("Add variable: Unable to add variable in db. [%s] Err=%s\n"), sql, zErrMsg ) );
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
                                    const bool bPersistent,
                                    const uint32_t accessRights,
                                    const wxString& note )
{
    CVSCPVariable variable;
            
    variable.setName( name );
    variable.setType( type );
    variable.setValueFromString( type, value );
    variable.setPersistent( bPersistent );
    variable.setUserID( userid );
    variable.setAccessRights( accessRights );
    variable.setNote( note );

    return add( variable );
}

///////////////////////////////////////////////////////////////////////////////
// addWithStringType
//

bool CVariableStorage::add( const wxString& varName,
                                            const wxString& value,
                                            const wxString& strType,    
                                            const uint32_t userid,
                                            bool bPersistent,
                                            const uint32_t accessRights,
                                            const wxString& note ) 
{
    uint8_t type = CVSCPVariable::getVariableTypeFromString( strType );
    return add( varName,
                    value,
                    type,
                    userid,
                    bPersistent,
                    accessRights,
                    note ); 
}

///////////////////////////////////////////////////////////////////////////////
// update
//

bool CVariableStorage::update( CVSCPVariable& var )
{
    uint32_t id = 0;
    char *zErrMsg = 0;
    
    // Can't be stock variables 
    if ( var.isStockVariable() ) {
        return false;
    }

    // Must exists    
    if ( 0 == ( id = exist( var.getName() ) ) ) {
        return false;
    }
    
    char *sql = sqlite3_mprintf( VSCPDB_VARIABLE_UPDATE, 
                (const char *) var.m_lastChanged.FormatISOCombined().mbc_str(),
                (const char *) var.getName().mbc_str(),
                var.getType(),
                (const char *) var.getValue().mbc_str(),
                var.isPersistent() ? 1 : 0,
                var.getUserID(),
                var.getAccessRights(),
                (const char *) var.getNote().mbc_str(),
                id );

    if (SQLITE_OK != sqlite3_exec( ( var.isPersistent() ? m_db_vscp_external_variable : m_db_vscp_internal_variable ), 
                                            sql, NULL, NULL, &zErrMsg)) {            
        gpctrlObj->logMsg( wxString::Format( _("Add variable: Unable to update variable in db. [%s] Err=%s\n"), sql, zErrMsg ) );
        sqlite3_free(sql);
        return false;
    }

    sqlite3_free(sql);    
    return true;
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
        char *sql = sqlite3_mprintf( VSCPDB_VARIABLE_WITH_ID,
                                    variable.getID() );
        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_external_variable, sql, NULL, NULL, &zErrMsg ) ) {
            sqlite3_free( sql );
            gpctrlObj->logMsg( wxString::Format( _("Delete variable: Unable to delete variable in db. [%s] Err=%s\n"), sql, zErrMsg ) );
            return false;
        }
        
        sqlite3_free( sql );                            
    }
    else {        
        char *sql = sqlite3_mprintf( VSCPDB_VARIABLE_WITH_ID,
                                    variable.getID() );
        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_internal_variable, sql, NULL, NULL, &zErrMsg ) ) {
            sqlite3_free( sql );
            gpctrlObj->logMsg( wxString::Format( _("Delete variable: Unable to delete variable in db. [%s] Err=%s\n"), sql, zErrMsg ) );
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
    const char *psql = VSCPDB_VARIABLE_CREATE; 
    
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
    const char *psql = VSCPDB_VARIABLE_CREATE;
    
    // Check if database is open
    if ( NULL == m_db_vscp_internal_variable ) return false;
    
    if ( SQLITE_OK != sqlite3_exec(m_db_vscp_internal_variable, psql, NULL, NULL, NULL) ) {
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadFromXML
//
// Read persistent variables from file
//

bool CVariableStorage::loadFromXML( const wxString& path  )
{
    wxString wxstr;
    unsigned long lval;
    wxXmlDocument doc;

#ifdef BUILD_VSCPD_SERVICE
    //wxStandardPaths stdPath;

    // Set the default dm configuration path
#ifdef WIN32
    m_configPath = wxStandardPaths::Get().GetConfigDir();
    m_configPath += _("/vscp/variable.xml");
#else
    m_configPath = _("/srv/vscp/variable.xml");
#endif
#endif 
    
    // If a null path is supplied use the configured path
    if ( path.IsEmpty() ) {
        
        gpctrlObj->logMsg( wxString::Format( _("Loading variable XML file from %s.\n"),
                                        (const char *)m_xmlPath.mbc_str() ), 
                                        DAEMON_LOGMSG_DEBUG );
        
        if ( !doc.Load( m_xmlPath ) ) {
            gpctrlObj->logMsg(_("Failed to load variable XML file from standard XML path.\n") );
            return false;
        }
        
    }
    else {
        
        gpctrlObj->logMsg( wxString::Format( _("Loading variable XML file from %s.\n"),
                                        (const char *)path.mbc_str() ),
                                        DAEMON_LOGMSG_DEBUG );
        
        if (!doc.Load( path ) ) {
            gpctrlObj->logMsg( wxString::Format( _("Failed to load variable XML file from %s.\n"),
                                        (const char *)path.mbc_str() ) );
            return false;
        }
    }
        
    // start processing the XML file
    if ( !( doc.GetRoot()->GetName() != wxT("variables") ||
            doc.GetRoot()->GetName() != wxT("persistent") ) ) {
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {

        if ( child->GetName() == wxT("variable") ) {

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
            else if ( wxNOT_FOUND != wxstr.Find( _("YES") ) ) {
                variable.setPersistent( true ); 
            } 
                    
            // userid
            if ( child->GetAttribute( wxT("user"), wxT("0") ).ToULong( &lval ) ) {
                variable.setUserID( lval );
            }
            
            // access-rights
            if ( child->GetAttribute( wxT("access-rights"), wxT("0") ).ToULong( &lval ) ) {
                variable.setAccessRights( lval );
            }
            
            // name
            wxstr = child->GetAttribute( wxT("name"), wxT("") );
            wxstr.MakeUpper();
            wxstr.Trim();
            wxstr.Trim( false );
            variable.setName( wxstr );
            
            // value
            wxstr = child->GetAttribute( wxT("value"), wxT("") );
            variable.setValue( wxstr );
            
            // note
            wxstr = child->GetAttribute( wxT("note"), wxT("") );
            variable.setNote( wxstr );
                    
            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName() == wxT("name")) {
                    wxString strName = subchild->GetNodeContent();
                    strName.Trim();
                    strName.Trim(false);
                    variable.setName( strName );
                }
                else if (subchild->GetName() == wxT("value")) {
                    variable.setValue( subchild->GetNodeContent() );
                }
                else if (subchild->GetName() == wxT("value-base64")) {
                    variable.setValue( (wxString)wxBase64Decode( subchild->GetNodeContent(), 
                                            subchild->GetNodeContent().Length() ) );
                }
                else if (subchild->GetName() == wxT("note")) {
                    variable.setNote( subchild->GetNodeContent(), false );
                }
                else if (subchild->GetName() == wxT("note-base64")) {
                    variable.setNote( (wxString)wxBase64Decode( subchild->GetNodeContent(), 
                                            subchild->GetNodeContent().Length() ) );
                }

                subchild = subchild->GetNext();

            }

            // Add the variable
            if ( add( variable ) ) {
                gpctrlObj->logMsg( wxString::Format( _("Loading XML file: Added variable %s.\n"),
                                        (const char *)variable.getName().mbc_str() ),
                                        DAEMON_LOGMSG_DEBUG );
            }
            else {
                gpctrlObj->logMsg( wxString::Format( _("Loading XML file: Failed to add variable %s.\n"),
                                        (const char *)variable.getName().mbc_str() ) );
            }

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
    wxCopyFile( m_xmlPath, m_xmlPath + _("~") );

    return save( m_xmlPath );
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
        const char *psql = "SELECT * from variable";        
    
        if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_internal_variable,
                                            psql,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
            return false;
        }
    
        while ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {
            
            CVSCPVariable variable;
            
            variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_ID ) );
            variable.m_lastChanged.ParseISOCombined( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
            variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) ) );
            variable.setType( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE ) );
            variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE ) ) );
            variable.setPersistent( false );
            variable.setUserID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) ); 
            variable.setAccessRights( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
            variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
            
            writeVariableToXmlFile( pFileStream, variable );
        }
        
        sqlite3_finalize( ppStmt );
    }
    
    if ( ( NULL == m_db_vscp_external_variable ) && 
            ( whatToSave & VARIABLE_EXTERNAL ) ) {
        
        char *pErrMsg = 0;
        sqlite3_stmt *ppStmt;
        const char *psql = "SELECT * from variable";        
    
        if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_external_variable,
                                            psql,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
            return false;
        }
    
        while ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {
            
            CVSCPVariable variable;
            
            variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_ID ) );
            variable.m_lastChanged.ParseISOCombined( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
            variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) ) );
            variable.setType( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE ) );
            variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE ) ) );
            variable.setPersistent( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERSISTENT ) ? true : false );
            variable.setUserID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) );
            variable.setAccessRights( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
            variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
            
            writeVariableToXmlFile( pFileStream, variable ); // Write the variable out on the stream
            
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
    wxString str, strtemp;
    wxString name = variable.getName();

    str.Printf(_("  <variable type=\"%d\" "), variable.getType());
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));

    str.Printf(_(" name=\"%s\" "), (const char *)name.mbc_str() );
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
    
    if (variable.isPersistent()) {
        str.Printf(_(" persistent=\"true\" "));
    } 
    else {
        str.Printf(_(" persistent=\"false\" "));
    }
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));

    str.Printf(_(" user=\"%d\" "), variable.getUserID());
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));

    str.Printf(_(" access-rights=\"%d\" "), variable.getAccessRights());
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
    
    variable.writeValueToString( strtemp );
    str.Printf(_(" value=\"%s\" "), (const char *)strtemp.mbc_str() );
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
    
    //Allways save on base64 encoded form
    str.Printf(_(" note-base64=\"%s\" "), (const char *)variable.getNote().mbc_str() );
    pFileStream->Write(str.mb_str(), strlen(str.mb_str()));

    pFileStream->Write(" />\n\n", strlen(" />\n\n"));
  
    return true;
}


