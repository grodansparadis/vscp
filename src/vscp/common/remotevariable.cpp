// vscpvariable.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
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
#include <wx/listimpl.cpp>
#include <wx/versioninfo.h>

#include <openssl/opensslv.h>
#include <openssl/crypto.h>

#include <mongoose.h>
#include <lua.h>
#include <duktape.h>

#include <vscp.h>
#include <vscpdb.h>
#include <version.h>
#include <vscp_debug.h>
#include <controlobject.h>
#include <vscphelper.h>
#include <guid.h>
#include <variablecodes.h>
#include <remotevariable.h>
#include <vscpbase64.h>

// The global control object
extern CControlObject *gpobj;

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
// makeAccessTightString
//

void CVSCPVariable::makeAccessRightString( uint32_t accessrights, 
                                                wxString& strAccessRights )
{
    strAccessRights.Empty();
    
    // other
    strAccessRights = ( accessrights & PERMISSON_OTHER_EXECUTE ) ?
         'x' + strAccessRights : '-' + strAccessRights ;
    
    strAccessRights = ( accessrights & PERMISSON_OTHER_WRITE ) ?
         'w' + strAccessRights : '-' + strAccessRights ;
    
    strAccessRights = ( accessrights & PERMISSON_OTHER_READ ) ?
         'r' + strAccessRights : '-' + strAccessRights ;
   
    
    // group
    strAccessRights = ' ' + strAccessRights;
    strAccessRights = ( accessrights & PERMISSON_GROUP_EXECUTE ) ?
         'x' + strAccessRights : '-' + strAccessRights ;
    
    strAccessRights = ( accessrights & PERMISSON_GROUP_WRITE ) ?
         'w' + strAccessRights : '-' + strAccessRights ;
    
    strAccessRights = ( accessrights & PERMISSON_GROUP_READ ) ?
         'r' + strAccessRights : '-' + strAccessRights ;
    
    // owner
    strAccessRights = ' ' + strAccessRights;
    strAccessRights = ( accessrights & PERMISSON_OWNER_EXECUTE ) ?
         'x' + strAccessRights : '-' + strAccessRights ;
    
    strAccessRights = ( accessrights & PERMISSON_OWNER_WRITE ) ?
         'w' + strAccessRights : '-' + strAccessRights ;
    
    strAccessRights = ( accessrights & PERMISSON_OWNER_READ ) ?
         'r' + strAccessRights : '-' + strAccessRights ;
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
        else if ( 0 == str.Find( _("EVENTDATA") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA;
        }
        else if ( 0 == str.Find( _("EVENTCLASS") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS;
        }
        else if ( 0 == str.Find( _("EVENTTYPE") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE;
        }
        else if ( 0 == str.Find( _("EVENTTIMESTAMP") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP;
        }
        else if ( 0 == str.Find( _("EVENT") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT;
        }        
        else if ( 0 == str.Find( _("GUID") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID;
        }        
        else if ( 0 == str.Find( _("DATA") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA;
        }
        else if ( 0 == str.Find( _("VSCPCLASS") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS;
        }       
        else if ( 0 == str.Find( _("VSCPTYPE") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE;
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
// isNumerical
//

bool CVSCPVariable::isNumerical( void )
{
    if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == m_type ) ||
         ( VSCP_DAEMON_VARIABLE_CODE_LONG == m_type ) || 
         ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == m_type ) ) {
        return true;
    }
    
    return false;
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
        wxstr += _(";");
        wxstr += m_strValue;
        wxstr += _(";");
        wxstr += m_note;
    }
    
    return wxstr;
}

///////////////////////////////////////////////////////////////////////////////
// getAsJSON
//

bool CVSCPVariable::getAsJSON( wxString &strVariable )
{
    // name,type,user,rights,persistence,last,bnumerical,bbase64,value,note
    // value is numerical for a numerical variable else string
    strVariable.Printf( VARIABLE_JSON_TEMPLATE, 
                            m_name,
                            (unsigned short int)m_type,
                            (unsigned long int)m_userid,
                            (unsigned long int)m_accessRights,
                            m_bPersistent ? _("true") : _("false"),
                            (const char *)m_lastChanged.FormatISOCombined().mbc_str(),
                            isNumerical() ? "true" : "false",
                            isValueBase64Encoded( m_type ) ? "true" : "false",
                            isNumerical() ? m_strValue : "\"" + m_strValue + "\"",
                            m_note );
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAsXML
//

bool CVSCPVariable::getAsXML( wxString &strVariable )
{
    strVariable.Printf( VARIABLE_XML_TEMPLATE,
                            m_name,
                            (unsigned short int)m_type,
                            (unsigned long int)m_userid,
                            (unsigned long int)m_accessRights,
                            m_bPersistent ? _("true") : _("false"),
                            (const char *)m_lastChanged.FormatISOCombined().mbc_str(),
                            m_strValue,
                            m_note );
    return true;
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
        gpobj->logMsg( wxString::Format( _("Setting variable name: Variable name can't be empty.\n") ) );
        return false;
    }
    
    m_name = str;
    fixName();

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// setOwnerId
//

bool CVSCPVariable::setOwnerId( uint32_t userid )
{
    // Check for admin user
    if ( 0 == userid ) {
        m_userid = USER_ID_ADMIN;
        return true;
    }
    
    // Check if id is defined
    //if ( !CUserItem::isUserInDB( userid ) ) return false;
    
    // Assign userid    
    m_userid = userid;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setOwnerIdFromUserName
//

bool CVSCPVariable::setOwnerIdFromUserName( wxString& strUser )
{
    long userid;
    
    // Check for admin user
    if ( gpobj->m_admin_user.Upper() == strUser.Upper() ) {
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
                strValueOut = m_strValue;
                vscp_base64_wxdecode( strValueOut );
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
                vscp_base64_wxdecode( strValueOut );
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
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_JSON:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_XML:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_SQL:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LUA:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break; 
            
        case VSCP_DAEMON_VARIABLE_CODE_DM_ROW:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break;  
            
        case VSCP_DAEMON_VARIABLE_CODE_DRIVER:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_USER:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
            }
            else {
                strValueOut = m_strValue;
            }
            break;   
            
        case VSCP_DAEMON_VARIABLE_CODE_FILTER:
            if ( bBase64 ) {
                vscp_base64_wxdecode( strValueOut );
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
    m_strValue.Printf(_("%lf"), val );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strlen( strValue.mbc_str() ) ) + 1 ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strlen( strValue.mbc_str() ) ), 
                                                (const char *)strValue.mbc_str(),
                                                strlen( strValue.mbc_str() ) );
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
                strValue.ToLong( &lval );     // Ignore possible error, get what we get
                m_strValue = wxString::Format(_("%d"), (int)lval );
            }
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LONG:
            long lval;
            strValue.ToLong( &lval );       // Ignore possible error, get what we get
            m_strValue = wxString::Format(_("%ld"), lval );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
            double dval;
            if ( strValue.ToDouble( &dval ) ) {
                m_strValue = wxString::Format(_("%lf"), dval );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
                m_strValue = _("");
                char *pbuf = new char[ wxBase64EncodedSize( strValue.Length() ) ];
                if ( NULL == pbuf ) return false;
                size_t len = wxBase64Encode( pbuf, 
                                                wxBase64EncodedSize( strValue.Length() ), 
                                                (const char *)strValue.mbc_str(),
                                                strValue.Length() );
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
// getNote
//
//

bool CVSCPVariable::getNote( wxString& strNote, bool bBase64 )
{
    strNote = m_note;
    return vscp_base64_wxdecode( strNote );
}


///////////////////////////////////////////////////////////////////////////////
// getVariableFromString
//
// Format is: "variable name";"type";"persistence";"user";"rights";"value";"note"
//

bool CVSCPVariable::getVariableFromString( const wxString& strVariable, 
                                                const bool bBase64,
                                                const wxString& strUser )
{
    wxString strRights;             // User rights for variable               
    bool     bPersistent = false;   // Persistence of variable
    bool     brw = true;            // Writable    
    
    // Update last changed
    setLastChangedToNow();

    wxStringTokenizer tkz( strVariable, _(";\r\n") );

    // Name
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str.Trim();
        if ( str.Length() ) setName( str );
    }
    else {
        return false;	
    }

    // Type
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str.Trim();
        if ( str.Length() ) {
            m_type = getVariableTypeFromString( str );
        }
        else {
            m_type = getVariableTypeFromString( _("STRING") );
        }
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
        else {
            setPersistent( false );
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
                setOwnerId( userid );
            }
            else {
                wxString str = strUser;
                if ( !setOwnerIdFromUserName( str ) ) {
                    return false;
                }
            }
        }
        else {
            wxString str = strUser;
            if ( !setOwnerIdFromUserName( str ) ) {
                return false;
            }
        }
    }
    else {
        return false;
    }
    
    // Access rights
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str.Trim();
        if ( str.Length() ) {
            uint32_t rights = vscp_readStringValue( str );
            setAccessRights( rights );
        }
        else {
            setAccessRights( 0x744 );
        }
    }
    else {
        return false;
    }
    
    
    
    // Get the value of the variable
    if ( tkz.HasMoreTokens() ) {
        
        wxString value = tkz.GetNextToken();
        value.Trim();
        if ( value.Length() ) {
            setValueFromString( m_type, value, bBase64 );
        }
        else {
            // If no value given use RESET value
            Reset();
        }

    }
    else {
        return false;
    }
    
    // Get the note (if any)
    // Get the value of the variable
    if ( tkz.HasMoreTokens() ) {
        
        wxString note = tkz.GetNextToken();
        setNote( note );

    }
    
    return true;

}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void CVSCPVariable::setValue( wxString value, bool bBase64  ) 
{ 
    if ( bBase64 ) {
        m_strValue = wxBase64Encode( value, value.Length() );
    }
    else {
        m_strValue = value; 
    }
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
                m_strValue = _("0,0,0,255,255");    // value,unit,sensor-index,zone,subzone
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
        m_strValue = _("true"); 
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
        m_strValue = _("false"); 
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
    m_dbFilename.Assign( wxStandardPaths::Get().GetConfigDir() +
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
    //fprintf( stderr, wxstr.mbc_str() );
    gpobj->logMsg( "[Variable database init] " + wxstr, DAEMON_LOGMSG_DEBUG );
        
    // Check filename
    if ( m_dbFilename.IsOk() && m_dbFilename.FileExists() ) {

        if ( SQLITE_OK != sqlite3_open( (const char *)m_dbFilename.GetFullPath().mbc_str(),
                                            &m_db_vscp_external_variable ) ) {

            // Failed to open/create the database file
            fprintf( stderr, "[Variable database init] VSCP Daemon external variable database could not be opened. - Will not be used.\n" );
            wxstr.Printf( _("Path=%s error=%s\n"),
                            (const char *)m_dbFilename.GetFullPath().mbc_str(),
                            sqlite3_errmsg( m_db_vscp_external_variable ) );
            //fprintf( stderr, wxstr.mbc_str() );
            gpobj->logMsg(wxstr );
        }

    }
    else {
        if ( m_dbFilename.IsOk() ) {
            
            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            fprintf( stderr, "[Variable database init] VSCP Daemon external variable database does not exist - will be created.\n" );
            wxstr.Printf( _("Path=%s\n"), (const char *)m_dbFilename.GetFullPath().mbc_str() );
            //fprintf( stderr, wxstr.mbc_str() );            
            gpobj->logMsg( wxstr );
            
            if ( SQLITE_OK == sqlite3_open( (const char *)m_dbFilename.GetFullPath().mbc_str(),
                                            &m_db_vscp_external_variable ) ) {            
                // create the table.
                doCreateExternalVariableTable();
                
            }
            else {
                
                // Failed to create the variable database
                gpobj->logMsg( wxString::Format( _("[Variable database init] VSCP Daemon external variable database open  Err=%s\n"), sqlite3_errmsg( m_db_vscp_external_variable ) ) );
            
                
            }
            
        }
        else {
            fprintf( stderr, "[Variable database init] VSCP Daemon external variable database path invalid - will not be used.\n" );
            wxstr.Printf(_("Path=%s\n"), m_dbFilename.GetFullPath().mbc_str() );
            //fprintf( stderr, wxstr.mbc_str() );
            gpobj->logMsg(wxstr);
        }

    }
    
    // * * * VSCP Daemon internal variable database - Always created in-memory * * *
    
    if ( SQLITE_OK == sqlite3_open( ":memory:", &m_db_vscp_internal_variable ) ) {
        // Should always be created
        doCreateInternalVariableTable();
    }
    else {
        // Failed to open/create the database file
        fprintf( stderr, "[Variable database init] VSCP Daemon internal variable database could not be opened - Will not be used.\n" );
        wxstr.Printf( _("Error=%s\n"),
                            sqlite3_errmsg( m_db_vscp_internal_variable ) );
        //fprintf( stderr, wxstr.mbc_str() );
        gpobj->logMsg(wxstr);
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
    variable.setOwnerId( USER_ID_ADMIN );
    variable.setLastChangedToNow(); 
    variable.setNote(_(""));
    
    // *************************************************************************
    //                             Version
    // *************************************************************************
       
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.major") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("VSCP major version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.minor") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("VSCP minor version number."), true );
    addStockVariable( variable  );

    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.release") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("VSCP release version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.build") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("VSCP build version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.version.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("VSCP version string."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.copyright") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("VSCP copyright."), true );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                               wxWidgets
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.wxwidgets.version.major") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("wxWidgets major version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.wxwidgets.version.minor") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("wxWidgets minor version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.wxwidgets.version.release") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("wxWidgets release version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.wxwidgets.version.sub") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("wxWidgets sub version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.wxwidgets.version.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("wxWidgets version string."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.wxwidgets.copyright") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("wxWidgets copyright."), true );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                               Mongoose
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.mongoose.version.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Mongoose version."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.mongoose.copyright") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Mongoose copyright."), true );
    addStockVariable( variable  );


    // *************************************************************************
    //                                SQLite
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.sqlite.version.major") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("SQLite3 major version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.sqlite.version.minor") );
    variable.setNote( _("SQLite3 minor version number."), true );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.sqlite.version.release") );
    variable.setNote( _("SQLite3 release version number."), true );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.sqlite.version.build") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("SQLite3 build version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.sqlite.version.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("SQLite3 version string."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.sqlite.copyright") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("SQLite3 copyright."), true );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                               OPENSSL
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.openssl.version.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Open SSL version string."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.openssl.copyright") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Open SSL copyright string."), true );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                               Duktape
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.duktape.version.major") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Duktape major version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.duktape.version.minor") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Duktape minor version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.duktape.version.release") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Duktape release version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.duktape.version.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Version for Duktape library."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.duktape.copyright") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Licens for Duktape library."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                                 Cicitweb
    // *************************************************************************

    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.civetweb.copyright") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Licens for Civetweb library."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                                 LUA
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.lua.version.major") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("LUA major version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.lua.version.minor") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("LUA minor version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.lua.version.release") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("LUA release version number."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.lua.version.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("LUA version string."), true );
    addStockVariable( variable  );
      
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.lua.copyright") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("LUA copyright."), true );
    addStockVariable( variable  );
      

    
    // *************************************************************************
    //                               System
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Operating system information string."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.width") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Operating system CPU word width (numerical)."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.width.is64bit") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("True if operating system CPU word width is 64-bit."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.width.is32bit") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("True if operating system CPU word width is 32-bit."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.width.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Operating system CPU word width (text form)."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.endiness.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Operating system endiness."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.os.endiness.islittleendian") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("True if Operating system endiness is little endian."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.name") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Name of host machine."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.ip") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("IP address for host machine."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.mac") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("MAC address for host machine."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.userid") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("The user the VSCP daemon is running as."), true ); 
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.username") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Full name for the user the VSCP daemon is running as."), true );
    addStockVariable( variable );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.guid") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID );
    variable.setNote( _("GUID for host."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.loglevel") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Active log level (numerical form)."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.loglevel.str") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Active log level (text form)."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.client.receivequeue.max") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Maximum number of events in client receive queue."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.host.root.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Root path for VSCP daemon."), true );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                             TCP/IP
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.tcpip.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Address for VSCP daemon TCP/IP interface."), true );
    addStockVariable( variable  );
    

    
    // *************************************************************************
    //                             Multicast
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.multicast.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Address for VSCP daemon multicast TCP/IP interface (Default: 224.0.23.158)."), true );
    addStockVariable( variable );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.multicast.ttl") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("TTL for VSCP daemon multicast TCP/IP interface (Default: 224.0.23.158)."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                                UDP
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.udp.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon simple UDP interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.udp.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Address for VSCP daemon simple UDP interface."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                                 MQTT
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.mqtt.server.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP server MQTT interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.mqtt.server.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Address for VSCP server MQTT interface."), true );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                               CoAP
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.coap.server.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon CoAP interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.coap.server.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Address for VSCP daemon CoAP interface."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                              Discovery
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.discovery.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon discovery."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                              AUTOMATION
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.calc.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last VSCP daemon automation calculation."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.longitude") );
    variable.setNote( _("VSCP daemon automation longitude setting."), true );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.latitude") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
    variable.setNote( _("VSCP daemon automation latitude setting."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.heartbeat.enable") );
    variable.setNote( _("Enable flag for VSCP daemon automation heartbeat event."), true );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ  | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.heartbeat.period") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Period in seconds for VSCP daemon automation heartbeat event."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.heartbeat.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation heart beat event."), true );
    addStockVariable( variable  );
    
    
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.segctrl-heartbeat.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation segment controller heartbeat event."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.segctrl-heartbeat.period") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Period in seconds for VSCP daemon automation heart beat event."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.segctrl-heartbeat.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation heart beat event."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.daylength") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("VSCP daemon automation calculated daylength."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.declination") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
    variable.setNote( _("VSCP daemon automation calculated sun declination."), true );
    addStockVariable( variable  );  
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.sun.max.altitude") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
    variable.setNote( _("VSCP daemon automation calculated max sun altitude."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.twilightsunriseevent.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation twilight sunrise event."), true );
    addStockVariable( variable  );
       
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.twilightsunriseevent.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation twilight sunrise event."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.sunriseevent.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation sunrise event."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.sunriseevent.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation sunrise event."), true );
    addStockVariable( variable  );
    
    
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.sunsetevent.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation sunset event."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.sunsetevent.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation sunset event."), true );
    addStockVariable( variable  );
           
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.twilightsunsetevent.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation twilight sunset event."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.twilightsunsetevent.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation twilight sunset event."), true );
    addStockVariable( variable  );
       
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.automation.calculatednoonevent.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Enable flag for VSCP daemon automation calculated noon event."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.automation.calculatednoonevent.last") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
    variable.setNote( _("Date and time for last sent VSCP daemon automation calculated noon event."), true );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                            Websocket-Server
    // *************************************************************************
    

    
    
    // *************************************************************************
    //                                WEB-Server
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.address") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Address for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.authentication.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable authentication for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.root.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Root folder for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.authdomain") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Auth domain for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.cert.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Certification path for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.extramimetypes") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Extra mime types for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.ssipatterns") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("SSI patterns for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.ipacl") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("IP access control list for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.cgi.interpreter") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to CGI interpreter for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.cgi.pattern") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("CGI pattern for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.directorylistings.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable directory listings for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.hidefile.pattern") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Hide file pattern for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.indexfiles") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("List of indexfiles for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.urlrewrites") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("URL rewrites for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.auth.file.directory") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Authentication file directory for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.websrv.auth.file.global") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to global authentication file for VSCP daemon web interface."), true );
    addStockVariable( variable  );
    
    
    // *************************************************************************
    //                            Decision Matrix
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.loglevel") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("DM log level 'normal' or 'debug'."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.db.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to decision matrix database."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.xml.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to decision matrix XML file to load at startup."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Total number of decision matrix rows."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.dm.count.active") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Number of active decision matrix rows."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.dm") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Full decision matrix (all rows)."), true );
    addStockVariable( variable  );
        
    
    // *************************************************************************
    //                               Debug
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.debug.flags1") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("VSCP debug flags 1."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.debug.flags2") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("VSCP debug flags 2."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.debug.flags3") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("VSCP debug flags 3."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.debug.flags4") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("VSCP debug flags 4."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.debug.flags5") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("VSCP debug flags 5."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.debug.flags6") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("VSCP debug flags 6."), true );
    addStockVariable( variable  );
    variable.init();
    
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.debug.flags7") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("VSCP debug flags 7."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.debug.flags8") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("VSCP debug flags 8."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                             Variables
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.variable.xml.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to XML file to load variables from on startup."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.variable.db.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to variable database."), true );
    addStockVariable( variable  );    
    
    // *************************************************************************
    //                              Log files
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.syslog.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable syslog."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.database.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable logging to database."), true );
    addStockVariable( variable  );
        
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
    variable.setName( _("vscp.log.database.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to logging database."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );
    variable.setName( _("vscp.log.database.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
    variable.setNote( _("Number of records in log database."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );
    variable.setName( _("vscp.log.database.sql") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_SQL );
    variable.setNote( _("SQL select for log database retrieval (Default: 'ORDER BY date DESC LIMIT 500) ."), true );
    // SELECT * FROM log WHERE date BETWEEN '2016-12-01' AND '2011-11-02';
    // SELECT * FROM log WHERE strftime('%H',date)  BETWEEN '12' AND '13';
    variable.setValue( _("ORDER BY date DESC LIMIT 500"), true );
    addStockVariable( variable );
    
    variable.setAccessRights( PERMISSON_ALL_READ );
    variable.setName( _("vscp.log.database.search") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Log database search result."), true );
    addStockVariable( variable  );
   
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.general.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable general logging to a text file."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.general.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to general logging text file."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.access.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable access logging to a text file."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.access.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to access logging text file."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.security.enable") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Flag to enable security logging to a text file."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.log.security.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to security logging text file."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                             Databases
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.database.vscpdata.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to VSCP data database."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE );    
    variable.setName( _("vscp.database.vscpdconfig.path") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Path to VSCP daemon main database."), true );
    addStockVariable( variable  );
    
    
    
    // *************************************************************************
    //                              Drivers
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of loaded drivers."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.level1.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of Level I drivers."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.level2.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of level II drivers."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.add") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Add a driver."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.delete") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Delete a driver."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.start") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Start a driver."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.pause") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Pause a driver."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver.stop") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
    variable.setNote( _("Stop a driver."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.driver") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All driver info."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                             Interfaces
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.interface.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of active interfaces."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.interface") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All interfaces."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                              Discovery
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.discovery.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of discovered units."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.discovery") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All discovered units."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                                Users
    // *************************************************************************
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );
    variable.setOwnerID( 0 );
    variable.setName( _("vscp.user.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of defined users."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_OWNER_ALL );
    variable.setOwnerID( 0 );
    variable.setName( _("vscp.user") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All defined users."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );
    variable.setOwnerID( 0 );
    variable.setName( _("vscp.user.names") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All defined users names."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_OWNER_WRITE );
    variable.setOwnerID( 0 );
    variable.setStockVariable();
    variable.setName( _("vscp.user.add") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Add a new user."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_OWNER_WRITE );
    variable.setOwnerID( 0 );
    variable.setStockVariable();
    variable.setName( _("vscp.user.delete") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("Delete a user."), true );
    addStockVariable( variable  );
    
    // *************************************************************************
    //                                Tables
    // *************************************************************************   
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.table.count") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
    variable.setNote( _("Number of defined user tables."), true );
    addStockVariable( variable  );
    
    variable.init();
    variable.setAccessRights( PERMISSON_ALL_READ );    
    variable.setName( _("vscp.table") );
    variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
    variable.setNote( _("All user tables."), true );
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

uint32_t CVariableStorage::find( const wxString& name, CVSCPVariable& variable )
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

uint32_t CVariableStorage::getStockVariable( const wxString& name, 
                                                CVSCPVariable& var,
                                                CUserItem *pUser )
{
    wxString wxstr;
    wxString strToken;
    wxString strrest;
    uint32_t id = 0;
    
    var.init();
    var.setStockVariable();
    
    wxString lcname= name.Lower();
    
    wxStringTokenizer tkz(lcname, ".");
    
    if ( !tkz.HasMoreTokens() ) return false;
    strToken = tkz.GetNextToken();
    
    // Make sure it starts with ".vscp"
    if ( "vscp" != strToken ) return false;  
    
    // Get persistent variable
    id = findNonPersistentVariable( name, var );
    if ( 0 == id ) return 0;
    
    
    // *************************************************************************
    //                            Debug flags
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.debug") ) ) {
            
        if ( lcname.StartsWith( _("vscp.debug.flags1") ) ) {
            var.setValue( (long)gpobj->m_debugFlags1 );
            return var.getID();
        }
    
        if ( lcname.StartsWith( _("vscp.debug.flags2") ) ) {
            var.setValue( (long)gpobj->m_debugFlags2 );
            return var.getID();
        }
    
        if ( lcname.StartsWith( _("vscp.debug.flags3") ) ) {
            var.setValue( (long)gpobj->m_debugFlags3 );
            return var.getID();
        }
    
        if ( lcname.StartsWith( _("vscp.debug.flags4") ) ) {
            var.setValue( (long)gpobj->m_debugFlags4 );
            return var.getID();
        }
    
        if ( lcname.StartsWith( _("vscp.debug.flags5") ) ) {
            var.setValue( (long)gpobj->m_debugFlags5 );
            return var.getID();
        }
    
        if ( lcname.StartsWith( _("vscp.debug.flags6") ) ) {
            var.setValue( (long)gpobj->m_debugFlags6 );
            return var.getID();
        }
    
        if ( lcname.StartsWith( _("vscp.debug.flags7") ) ) {
            var.setValue( (long)gpobj->m_debugFlags7 );
            return var.getID();
        }
        
        if ( lcname.StartsWith( _("vscp.debug.flags8") ) ) {
            var.setValue( (long)gpobj->m_debugFlags8 );
            return var.getID();
        }

    }    
    
    // *************************************************************************
    //                            VSCP version info.
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.version.major") ) ) {
        var.setValue( wxString::Format( _("%d"), VSCPD_MAJOR_VERSION ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.version.minor") ) ) {
        var.setValue( wxString::Format( _("%d"), VSCPD_MINOR_VERSION ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.version.release") ) ) {
        var.setValue( wxString::Format( _("%d"), VSCPD_RELEASE_VERSION ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.version.build") ) ) {
        var.setValue( wxString::Format( _("%d"), VSCPD_BUILD_VERSION ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.version.str") ) ) {
        var.setValue( wxString::Format( _("%s"), VSCPD_DISPLAY_VERSION ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.copyright") ) ) {
        var.setValue( wxString::Format( _("%s"), VSCPD_COPYRIGHT ), true );
        return var.getID();
    }

    // *************************************************************************
    //                               wxWidgets
    // *************************************************************************

    
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.version.major") ) ) {
        var.setValue( wxString::Format( _("%d"), wxMAJOR_VERSION ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.version.minor") ) ) {
        var.setValue( wxString::Format( _("%d"), wxMINOR_VERSION ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.version.release") ) ) {
        var.setValue( wxString::Format( _("%d"), wxRELEASE_NUMBER ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.version.sub") ) ) {
        var.setValue( wxString::Format( _("%d"), wxSUBRELEASE_NUMBER ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.version.str") ) ) {
        var.setValue( wxString::Format( _("%s"), wxVERSION_STRING ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.copyright") ) ) {
        wxVersionInfo verinfo;
        var.setValue( wxString::Format( _("%s"),
                    (const char *)verinfo.GetCopyright().mbc_str() ), 
                    true );
        return var.getID();
    }    
    

    // *************************************************************************
    //                               Mongoose
    // *************************************************************************

    if ( lcname.StartsWith( _("vscp.mongoose.version.str") ) ) {
        var.setValue( wxString::Format( _("%s"),
                        MG_VERSION ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.mongoose.copyright") ) ) {
        var.setValue( wxString::Format( _("%s"),
                    "Copyright (c) 2013-2015 Cesanta Software Limited" ), true );
        return var.getID();
    }

    

    // *************************************************************************
    //                                SQLite
    // *************************************************************************

    if ( lcname.StartsWith( _("vscp.sqlite.version.major") ) ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setValue( wxString::Format( _("%d"), major ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.sqlite.version.minor") ) ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setValue( wxString::Format( _("%d"), minor ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.sqlite.version.release") ) ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setValue( wxString::Format( _("%d"), sub ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.sqlite.version.build") ) ) {
        int major, minor, sub, build;
        sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );
        var.setValue( wxString::Format( _("%d"), build ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.sqlite.version.str") ) ) {
        var.setValue( _(SQLITE_VERSION), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.sqlite.copyright") ) ) {
        var.setValue( wxString::Format( _("%s"),
                    "SQLite Is Public Domain" ), true );
        return var.getID();
    }

    // *************************************************************************
    //                               OPENSSL
    // *************************************************************************
    

    if ( lcname.StartsWith( _("vscp.openssl.version.str") ) ) {
#ifdef WIN32        
        var.setValue( _(OPENSSL_VERSION_TEXT) );
#else       
        // http://linux.die.net/man/3/ssleay
        //var.setValue( _("OPENSSL_VERSION_NUMBER") );
        wxstr.Printf( _("%s"), SSLeay_version( SSLEAY_VERSION ) );
        var.setValue( wxstr, true );
        return var.getID();
#endif        
        
    }
    
    if ( lcname.StartsWith( _("vscp.openssl.copyright") ) ) {
        wxstr.Printf( _("%s"), 
                "Copyright (c) 1998-2006 The OpenSSL Project.  "
                "All rights reserved." );
        var.setValue( wxstr, true );
        return var.getID();
    }
    
    
    // *************************************************************************
    //                                 LUA
    // *************************************************************************
    
    
    
    if ( lcname.StartsWith( _("vscp.lua.version.major") ) ) {
        var.setValue( LUA_VERSION_MAJOR );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.lua.version.minor") ) ) {
        var.setValue( LUA_VERSION_MINOR );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.lua.version.release") ) ) {
        var.setValue( LUA_VERSION_RELEASE );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.lua.version.str") ) ) {
        var.setValue( wxString::Format( _("%s"),
                    LUA_RELEASE ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.lua.copyright") ) ) {
        var.setValue( wxString::Format( _("%s"),
                        LUA_COPYRIGHT ), true );
        return var.getID();
    }
    
    
    
    
    
    // *************************************************************************
    //                                 Duktape
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.duktape.version.major") ) ) {
        int dukmajor = DUK_VERSION / 10000;
        var.setValue( DUK_VERSION / 10000 );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.duktape.version.minor") ) ) {
        var.setValue( ( DUK_VERSION - ( DUK_VERSION / 10000 ) ) / 100 );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.duktape.version.release") ) ) {
        var.setValue( DUK_VERSION - ( DUK_VERSION / 100 ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.duktape.version.str") ) ) {
        var.setValue( wxString::Format( _("%s"),
                    DUK_GIT_DESCRIBE ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.duktape.copyright") ) ) {
        var.setValue( _("Duktape copyrights are held by its authors. (https://github.com/grodansparadis/vscp/blob/master/src/common/duktape.h)"), 
                        true );
        return var.getID();
    }
    
    
    
    // *************************************************************************
    //                                Civitweb
    // *************************************************************************
    
    
    if ( lcname.StartsWith( _("vscp.civetweb.copyright") ) ) {
        var.setValue( _("Copyright (c) 2004-2013 Sergey Lyubka, Copyright (c) 2013-2017 the Civetweb developers, Copyright (c) 2017 Ake Hedman, Grodans Paradis AB"),
                        true );
        return var.getID();
    }
    
    
    // *************************************************************************
    //                               System
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.os.str") ) ) {
        var.setValue( wxGetOsDescription(), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.os.width.str") ) ) {
        var.setValue( wxIsPlatform64Bit() ? _("64") : _("32"), true );
        return var.getID();   
    }
    
    if ( lcname.StartsWith( _("vscp.os.width.is64bit") ) ) {
        var.setValue( wxIsPlatform64Bit() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.os.width.is32bit") ) ) {
        var.setValue( !wxIsPlatform64Bit() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.os.width") ) ) {
        var.setValue( wxIsPlatform64Bit() ? 64 : 32 );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.os.endiness.str") ) ) {
        var.setValue( wxIsPlatformLittleEndian() ? _("Little endian") : _("Big endian"), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.os.endiness.islittleendian") ) ) {
        var.setValue( wxIsPlatformLittleEndian() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.host.name") ) ) {
        var.setValue( wxGetFullHostName(), true );
        return var.getID();
    }
        
    if ( lcname.StartsWith( _("vscp.host.ip") ) ) {
        cguid guid;
 
        if ( !gpobj->getIPAddress( guid ) ) {
            guid.clear();
        }

        var.setValue( wxString::Format( _("%d.%d.%d.%d"),
                                                    guid.getAt( 11 ),
                                                    guid.getAt( 10 ),
                                                    guid.getAt( 9 ),
                                                    guid.getAt( 8 ) ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.host.mac") ) ) {
        cguid guid;
        
        if ( !gpobj->getMacAddress( guid ) ) {
            guid.clear();
        }

        var.setValue( wxString::Format( _("%02X:%02X:%02X:%02X:%02X:%02X"),
                                            guid.getAt( 13 ),
                                            guid.getAt( 12 ),
                                            guid.getAt( 11 ),
                                            guid.getAt( 10 ),
                                            guid.getAt( 9 ),
                                            guid.getAt( 8 ) ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.host.userid") ) ) {
        var.setValue( wxGetUserId(), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.host.username") ) ) {
        var.setValue( wxGetUserName(), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.host.guid") ) ) {
        var.setValue( wxstr );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.loglevel.str") ) ) {
        switch ( gpobj->m_logLevel  ) {
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
        var.setValue( wxstr, true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.loglevel") ) ) {
        var.setValue( wxString::Format( _("%d "), gpobj->m_logLevel ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.client.receivequeue.max") ) ) {
        var.setValue( wxString::Format( _("%d"), 
                        gpobj->m_maxItemsInClientReceiveQueue ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith(  _("vscp.host.root.path") ) ) {
        var.setValue( gpobj->m_rootFolder, true );
        return var.getID();
    }
    
    // *************************************************************************
    //                             TCP/IP
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.tcpip.address") ) ) {
        var.setValue( gpobj->m_strTcpInterfaceAddress, true );
        return var.getID();
    }
    
    // *************************************************************************
    //                              Discovery
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.discovery.enable") ) ) {
        var.setValue( true );   // TODO
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.discovery.count") ) ) {
        var.setValue( 1 );      // TODO
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.discovery") ) ) {
        var.setValue( _("Functionality has not been implemented yet"), true ); // TODO
        return var.getID();
    }
    
    // *************************************************************************
    //                             Multicast
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.multicast.address") ) ) {
        var.setValue( gpobj->m_strMulticastAnnounceAddress, true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.multicast.ttl") ) ) {
        var.setValue( gpobj->m_ttlMultiCastAnnounce );
        return var.getID();
    }
    
    // *************************************************************************
    //                                UDP
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.udp.enable") ) ) {
        var.setValue( gpobj->m_udpInfo.m_bEnable ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.udp.address") ) ) {
        var.setValue( gpobj->m_udpInfo.m_interface , true );
        return var.getID();
    }
    
    // *************************************************************************
    //                                 COAP
    // *************************************************************************
    
    
    if ( lcname.StartsWith( _("vscp.coap.server.enable") ) ) {
        var.setValue( false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.coap.server.address") ) ) {
        var.setValue( wxString::Format( _("%s"),
                    "COAP is not part of VSCP anymore." ), true );
        return var.getID();
    }
    
    
        
    // *************************************************************************
    //                                MQTT
    // *************************************************************************
       
    
    if ( lcname.StartsWith( _("vscp.mqtt.server.enable") ) ) {
        var.setValue( false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.mqtt.server.address") ) ) {
        var.setValue( wxString::Format( _("%s"),
                    "MQTT is not part of VSCP anymore." ), true );
        return var.getID();
    }
    
    
    // *************************************************************************
    //                              AUTOMATION
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.automation.calc.last") ) ) {
        wxstr = gpobj->m_automation.getLastCalculation().FormatISOCombined();
        var.setValue( wxstr );
        return var.getID();
    }
    
    
    if ( lcname.StartsWith( _("vscp.automation.longitude") ) ) {
        var.setValue( wxString::Format( _("%f"), 
                        gpobj->m_automation.getLongitude() ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.latitude") ) ) {
        var.setValue( wxString::Format( _("%f"), 
                        gpobj->m_automation.getLatitude() ) );
        return var.getID();
    }
    
    
    if ( lcname.StartsWith( _("vscp.automation.heartbeat.enable") ) ) {
        var.setValue( gpobj->m_automation.isSendHeartbeat() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.heartbeat.period") ) ) {
        var.setValue( wxString::Format( _("%ld"), 
                        gpobj->m_automation.getIntervalHeartbeat() ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.heartbeat.last") ) ) {
        wxstr = gpobj->m_automation.getHeartbeatSent().FormatISOCombined();
        var.setValue( wxstr );
        return var.getID();
    }
    
    
    if ( lcname.StartsWith(  _("vscp.automation.segctrl-heartbeat.enable") ) ) {
        var.setValue(gpobj->m_automation.isSendSegmentControllerHeartbeat() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.segctrl-heartbeat.period") ) ) {
        var.setValue( wxString::Format( _("%ld"), 
                gpobj->m_automation.getIntervalSegmentControllerHeartbeat() ) );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.segctrl-heartbeat.last") ) ) {
        wxstr = gpobj->m_automation.getSegmentControllerHeartbeatSent().FormatISOCombined();
        var.setValue( wxstr );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.daylength") ) ) {
        int hour,minute;
        double daylength = gpobj->m_automation.getDayLength();
        gpobj->m_automation.convert2HourMinute( daylength, &hour, &minute );
        wxstr.Printf(_("%02d:%02d"), hour, minute );
        var.setValue( wxstr, true );
        return var.getID();
    }
        
    if ( lcname.StartsWith( _("vscp.automation.declination") ) ) {
        double declination = gpobj->m_automation.getDeclination();
        wxstr.Printf(_("%f"), declination );
        var.setValue( wxstr );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.sun.max.altitude") ) ) {
        double maxalt = gpobj->m_automation.getSunMaxAltitude();
        wxstr.Printf(_("%f"), maxalt );
        var.setValue( wxstr );
        return var.getID();
    }
    
    
    
    if ( lcname.StartsWith( _("vscp.automation.twilightsunriseevent.enable") ) ) {
        var.setValue( gpobj->m_automation.isSendSunriseTwilightEvent() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.twilightsunriseevent.last") ) ) {
        wxstr = gpobj->m_automation.getSegmentControllerHeartbeatSent().FormatISOCombined();
        var.setValue( wxstr );
        return var.getID();
    }
    
    
    
    if ( lcname.StartsWith( _("vscp.automation.sunriseevent.enable") ) ) {
        var.setValue( gpobj->m_automation.isSendSunriseEvent() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.sunriseevent.last") ) ) {
        wxstr = gpobj->m_automation.getSunriseEventSent().FormatISOCombined();
        var.setValue( wxstr );
        return var.getID();
    }
    
    
    
    if ( lcname.StartsWith( _("vscp.automation.sunsetevent.enable") ) ) {
        var.setValue( gpobj->m_automation.isSendSunsetEvent() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.sunsetevent.last") ) ) {
        wxstr = gpobj->m_automation.getSunsetEventSent().FormatISOCombined();
        var.setValue( wxstr );
        return var.getID();
    }
    
    
    if ( lcname.StartsWith( _("vscp.automation.twilightsunsetevent.enable") ) ) {
        var.setValue( gpobj->m_automation.isSendSunsetTwilightEvent() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.twilightsunsetevent.last") ) ) {
        wxstr = gpobj->m_automation.getSunsetTwilightEventSent().FormatISOCombined();
        var.setValue( wxstr );
        return var.getID();
    }
    
    
    if ( lcname.StartsWith( _("vscp.automation.calculatednoonevent.enable") ) ) {
        var.setValue( gpobj->m_automation.isSendCalculatedNoonEvent() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.calculatednoonevent.last") ) ) {
        wxstr = gpobj->m_automation.getCalculatedNoonEventSent().FormatISOCombined();
        var.setValue( wxstr );
        return var.getID();
    }
    
    
    if ( lcname.StartsWith( _("vscp.automation.twilightsunsetevent.enable") ) ) {
        var.setValue( gpobj->m_automation.isSendSunsetTwilightEvent() ? true : false );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.automation.twilightsunsetevent.last") ) ) {
        wxstr = gpobj->m_automation.getSunsetTwilightEventSent().FormatISOCombined();
        var.setValue( wxstr );
        return var.getID();
    }
    
 
    
    // *************************************************************************
    //                            Websocket-Server
    // *************************************************************************

 

    // *************************************************************************
    //                                WEB-Server
    // *************************************************************************


    if ( lcname.StartsWith( _("vscp.websrv.address") ) ) {
        // String
        var.setValue( gpobj->m_strWebServerInterfaceAddress, true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.authdomain") ) ) {
        // String
        var.setValue( gpobj->m_authDomain, true );
        return var.getID();
    }
        
    if ( lcname.StartsWith( _("vscp.websrv.root.path") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_pathWebRoot ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.cert.path") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_pathCert ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.extramimetypes") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_extraMimeTypes ), true );
        return var.getID();
    }

    if ( lcname.StartsWith( _("vscp.websrv.ssipatterns") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_ssi_pattern ), true );
        return var.getID();
    }

    if ( lcname.StartsWith( _("vscp.websrv.ipacl") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_ip_acl ), true );
        return var.getID();
    }

    if ( lcname.StartsWith( _("vscp.websrv.cgi.interpreter") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_cgiInterpreter ), true );
        return var.getID();
    }

    if ( lcname.StartsWith( _("vscp.websrv.cgi.pattern") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_cgiPattern ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.directorylistings.enable") ) ) {
        var.setValue( ( NULL != 
            strstr( gpobj->m_EnableDirectoryListings,"yes" ) ) ? _("true") : _("false") );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.hidefile.pattern") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_hideFilePatterns ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.indexfiles") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_indexFiles ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.urlrewrites") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_urlRewrites ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.auth.file.directory") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_per_directory_auth_file ), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.auth.file.global") ) ) {
        var.setValue( wxString::FromUTF8( gpobj->m_global_auth_file ), true );
        return var.getID();
    }


    


    // *************************************************************************
    //                            Decision Matrix
    // *************************************************************************

    
    if ( lcname.StartsWith( _("vscp.dm.db.path") ) ) {
        var.setValue( gpobj->m_dm.m_path_db_vscp_dm.GetFullPath(), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.dm.xml.path") ) ) {
        var.setValue( gpobj->m_dm.m_staticXMLPath, true ); 
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.dm.loglevel") ) ) {
        if (gpobj->m_debugFlags1 | VSCP_DEBUG1_DM ) {
            var.setValue( "debug", true ); 
        }
        else {
            var.setValue( "normal", true );
        }
        return var.getID();
    }


    // *************************************************************************
    //                             Variables
    // *************************************************************************


    if ( lcname.StartsWith( _("vscp.variable.db.path") ) ) {
        var.setValue( gpobj->m_variables.m_dbFilename.GetFullPath(), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.variable.xml.path") ) ) {
        var.setValue( gpobj->m_variables.m_xmlPath, true );
        return var.getID();
    }


    // *************************************************************************
    //                              Log files
    // *************************************************************************

    // Enable syslog logging
    if ( lcname.StartsWith( _("vscp.log.syslog.enable") ) ) {
        var.setValue( gpobj->m_bLogToSysLog ? true : false );
        return var.getID();
    }
    
    // Enable database log
    if ( lcname.StartsWith( _("vscp.log.database.enable") ) ) {
        var.setValue( false );  // TODO
        return var.getID();
    }
       
    // Read Log database path
    if ( lcname.StartsWith(  _("vscp.log.database.path") ) ) {
        var.setValue( gpobj->m_path_db_vscp_data.GetFullPath(), true );
        return var.getID();
    }
    
    // Read Log database row count
    if ( lcname.StartsWith(  _("vscp.log.database.count") ) ) {
        var.setValue( gpobj->getCountRecordsLogDB() );
        return var.getID();
    }
    
    // Read Log database search result
    if ( lcname.StartsWith(  _("vscp.log.database.search") ) ) {
        wxString strResult;
        
        var.setStockVariable();
        var.setPersistent( false );
        
        CVSCPVariable sqlvar;
        if ( 0 == findNonPersistentVariable( _("vscp.log.database.sql"), sqlvar ) ) {
            var.setValue( _("0,0,0,0,ERROR: sql variable was not found") );
        }
        else {
            
            wxString wxstr = sqlvar.getValue();
            if ( !vscp_base64_wxdecode( wxstr ) ) return false;
            
            wxString searchStr = "SELECT * from log ";
            searchStr += wxstr;
            
            // Do search 
            gpobj->searchLogDB( (const char *)searchStr.mbc_str(), strResult );
        
            // Set the value
            var.setValue( strResult, true );
        }
        
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.log.database.sql") ) ) {
        // SQL
        return var.getID();
    }
    
    if ( lcname.StartsWith(  _("vscp.log.general.enable") ) ) {
        // Boolean
        var.setValue( false ); // TODO
        return var.getID();
    }
    
    if ( lcname.StartsWith(  _("vscp.log.general.path") ) ) {
        // String
        var.setValue( "" ); // TODO
        return var.getID();
    }
    
    if ( lcname.StartsWith(  _("vscp.log.access.enable") ) ) {
        // Boolean
        var.setValue( false ); // TODO
        return var.getID();
    }
    
    if ( lcname.StartsWith(  _("vscp.log.access.path") ) ) {
        // String
        var.setValue( "" ); // TODO
        return var.getID();
    }

    if ( lcname.StartsWith(  _("vscp.log.security.enable") ) ) {
        // Boolean
        var.setValue( false ); // TODO
        return var.getID();
    }
    
    if ( lcname.StartsWith(  _("vscp.log.security.path") ) ) {
        // String
        var.setValue( "" ); // TODO
        return var.getID();
    }
    
    // *************************************************************************
    //                             Databases
    // *************************************************************************    
    
        
    if ( lcname.StartsWith(  _("vscp.database.vscpdata.path") ) ) {
        var.setValue( gpobj->m_path_db_vscp_data.GetFullPath(), true );
        return var.getID();
    }
    
    if ( lcname.StartsWith(  _("vscp.database.vscpdconfig.path") ) ) {
        var.setValue( gpobj->m_path_db_vscp_daemon.GetFullPath(), true );
        return var.getID();
    }
    
    // *************************************************************************
    //                             Decision Matrix
    // *************************************************************************
    
    
    if ( lcname.StartsWith( _("vscp.dm.count.active") ) ) {
        var.setValue( gpobj->m_dm.getMemoryElementCount() );   
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.dm.count") ) ) {
        var.setValue( (long)gpobj->m_dm.getDatabaseRecordCount() );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.dm") ) ) {
        var.setStockVariable();
        var.setPersistent( false );
        var.setAccessRights( PERMISSON_OWNER_READ );           
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        gpobj->m_dm.getAllRows( wxstr );
        var.setValue( wxstr, true );     
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.dm.") ) ) {
        
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
            if ( !gpobj->m_dm.getDatabaseRecord( id, pElement ) ) return false;
            
            var.setStockVariable();
            var.setPersistent( false );
            
            if ( wxstr.StartsWith( _("id") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ );;           
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("bEnable") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( pElement->m_bEnable );
            }
            else if ( wxstr.StartsWith( _("groupid") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pElement->m_strGroupID, true );
            }
            else if ( wxstr.StartsWith( _("mask.priority") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( pElement->m_vscpfilter.mask_priority );
            }
            else if ( wxstr.StartsWith( _("mask.class") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( pElement->m_vscpfilter.mask_class );
            }
            else if ( wxstr.StartsWith( _("mask.type") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( pElement->m_vscpfilter.mask_type );
            }
            else if ( wxstr.StartsWith( _("mask.guid") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID );
                var.setValue( pElement->m_vscpfilter.mask_GUID );
            }
            else if ( wxstr.StartsWith( _("filter.priority") ) ) {
                cguid guid;
                guid.getFromArray( pElement->m_vscpfilter.mask_GUID );
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID );
                var.setValue( guid.getAsString() );
            }
            else if ( wxstr.StartsWith( _("filter.class") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( pElement->m_vscpfilter.filter_class );
            }
            else if ( wxstr.StartsWith( _("filter.type") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( pElement->m_vscpfilter.filter_type );
            }
            else if ( wxstr.StartsWith( _("filter.guid") ) ) {
                cguid guid;
                guid.getFromArray( pElement->m_vscpfilter.filter_GUID );
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID );
                var.setValue( guid.getAsString() );
            }
            else if ( wxstr.StartsWith( _("allowed.start") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("allowed.end") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_DATETIME );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("allowed.monday") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("allowed.tuesday") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );          
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("allowed.wednessday") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );           
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("allowed.thursday") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );         
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("allowed.friday") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );           
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("allowed.saturday") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );           
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("allowed.sunday") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("allowed.time") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("bCheckIndex") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("index") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("bCheckZone") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("zone") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("bCheckSubZone") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("subzone") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("measurement.value") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("measurement.unit") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("measurement.compare") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("measurement.compare.string") ) ){
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("measurement.comment") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );    
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("count.trigger") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ );          
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( (long)pElement->m_id );
            }
            else if ( wxstr.StartsWith( _("count.error") ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ );          
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( (long)pElement->m_id );
            }
            else if ( ( wxstr.StartsWith( _("error") ) ) || 
                      ( wxstr.StartsWith( _("error.string") ) ) ) {
                var.setAccessRights( PERMISSON_OWNER_READ | PERMISSON_OWNER_WRITE );           
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( (long)pElement->m_id );
            }
            
            return var.getID();
            
        }
        else {
            // A full DM row is requested ("vscp.dm.35" for row with id = 35)
            //???  TODO
        }
    }
    
    // *************************************************************************
    //                              Drivers
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.driver" ) ) ) {
    
        if ( lcname.StartsWith( _("vscp.driver.level1.count") ) ) {
            // Integer
            var.setValue( 0 ); // TODO
            return var.getID();
        }
        else if ( lcname.StartsWith( _("vscp.driver.level2.count") ) ) {
            // Integer
            var.setValue( 0 ); // TODO
            return var.getID();
        }
        else if ( lcname.StartsWith( _("vscp.driver.count") ) ) {
            // Integer
            var.setValue( 0 ); // TODO
            return var.getID();
        }
        else if ( lcname.StartsWith( _("vscp.driver.add") ) ) {
            // Boolean
            var.setValue( false ); // TODO
            return var.getID();
        }
        else if ( lcname.StartsWith( _("vscp.driver.delete") ) ) {
            // Boolean
            var.setValue( false ); // TODO
            return var.getID();
        }
        else if ( lcname.StartsWith( _("vscp.driver.start") ) ) {
            // Boolean
            var.setValue( false ); // TODO
            return var.getID();
        }
        else if ( lcname.StartsWith( _("vscp.driver.pause") ) ) {
            // Boolean
            var.setValue( false ); // TODO
            return var.getID();
        }
        else if ( lcname.StartsWith( _("vscp.driver.stop") ) ) {
            // Boolean
            var.setValue( false ); // TODO
            return var.getID();
        }
        // "vscp.driver"
        else {
            // String
            var.setValue( _("") ); // TODO
            return var.getID();
        }
    
    }
    
    // *************************************************************************
    //                             Interfaces
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.interface" ) ) ) {
        
        if ( lcname.StartsWith( _("vscp.interface.count") ) ) {
            // Integer
            var.setValue( 0 ); // TODO
            return var.getID();
        }
        // "vscp.interface"
        else {
            // String
            var.setValue( _("") ); // TODO
            return var.getID();
        }
        
    } 
    
    // *************************************************************************
    //                                Users
    // *************************************************************************
    if ( lcname.StartsWith( _("vscp.user.count") ) ) {
        // long
        var.setValue( (long)gpobj->m_userList.getUserCount() );
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.user.names") ) ) {
        wxArrayString array;
        wxstr.Empty();
        
        gpobj->m_userList.getAllUsers( array );
        for ( int i=0; i<array.Count(); i++ ) {
            wxstr += array[ i ];
            if ( i != ( array.Count()-1 ) ) wxstr += _(",");
        }
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxstr, true );  
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.user.add") ) ) {
        // holder for write only variable
        return var.getID();
    }
    
    if ( lcname.StartsWith( _("vscp.user.delete") ) ) {
        // holder for write only variable
        return var.getID();
    }
    
    // Individual user  (vscp.user.1 or vscp.user.1.name... )
    if ( lcname.StartsWith( _("vscp.user."), &strrest ) ) {
        
        int pos;
        unsigned long idx;
        wxStringTokenizer tkz( strrest, _(".") );
        if ( !tkz.HasMoreTokens() ) return 0;   
        
        wxString strToken = tkz.GetNextToken();         // Get idx
        if ( !strToken.ToULong( &idx ) ) return 0;
        
        if ( !tkz.HasMoreTokens() ) {
            // vscp.user.n - return user record for index n
            if ( !gpobj->m_userList.getUserAsString( idx, wxstr ) ) return 0; 
            var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
            var.setValue( wxstr, true );
        }
        else {
            // vscp.user.n.field 
            CUserItem *pUserItem;
            if ( !gpobj->m_userList.getUserAsString( idx, wxstr ) ) return 0;       // Get name
            if ( NULL == ( pUserItem = 
                    gpobj->m_userList.getUserItemFromOrdinal( idx ) ) ) return 0;   // UserItem
            strToken = tkz.GetNextToken();  // Get field
            if ( strToken.StartsWith( _("userid") ) ) {
                wxstr = wxString::Format( _("%ld"), pUserItem->getUserID() );
                var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
                var.setValue( wxstr );
            }
            else if ( strToken.StartsWith( _("name") ) ) {
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pUserItem->getUserName(), true );
            }
            else if ( strToken.StartsWith( _("password") ) ) {
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pUserItem->getPassword(), true );
            }
            else if ( strToken.StartsWith( _("fullname") ) ) {
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pUserItem->getFullname(), true );
            }
            else if ( strToken.StartsWith( _("filter") ) ) {
                var.setType( VSCP_DAEMON_VARIABLE_CODE_FILTER );
                var.setValue( wxstr );
            }
            else if ( strToken.StartsWith( _("rights") ) ) {
                // rights or rights.0..7
                if ( !tkz.HasMoreTokens() ) {
                    var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                    var.setValue( pUserItem->getUserRightsAsString(), true );
                }
                else {
                    strToken = tkz.GetNextToken();   // 0..7
                    if ( !strToken.ToULong( &idx ) ) return 0;
                    if ( idx > 7 ) return false;
                    wxstr.Printf(_("%d"), idx );
                    var.setType( VSCP_DAEMON_VARIABLE_CODE_INTEGER );
                    var.setValue( wxstr );
                }
            }
            else if (  strToken.StartsWith( _("allowed") ) ) {
                //allowed.events or allowed.remotes
                if ( !tkz.HasMoreTokens() ) return 0;
                strToken = tkz.GetNextToken();   // events/remotes
                if ( _("events") == strToken ) {
                    var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                    var.setValue( pUserItem->getAllowedEventsAsString(), true );
                }
                else if (  strToken.StartsWith( _("remotes") ) ) { 
                    var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                    var.setValue( pUserItem->getAllowedRemotesAsString(), true );
                }
                else {
                    return 0;
                }
            }
            else if ( strToken.StartsWith( _("note") ) ) {
                var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
                var.setValue( pUserItem->getNote(), true );
            }
            
        }
        
        return var.getID();
        
    }
    
    if ( lcname.StartsWith( _("vscp.user") ) ) {
        gpobj->m_userList.getAllUsers( wxstr );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );
        var.setValue( wxstr, true ); 
        return var.getID();
    }
    
    // *************************************************************************
    //                                Tables
    // ************************************************************************* 
    
    if ( lcname.StartsWith( _("vscp.table"), &strrest ) ) {

        if ( lcname.StartsWith( _("vscp.table.count") ) ) {
            // Integer
            var.setValue( 0 ); // TODO
            return var.getID();
        }
        // "vscp.table[....]"
        else {
            // String
            var.setValue( _("") ); // TODO
            return var.getID();
        }
        
    }
    

    
    // ----------------------------- Not Found ---------------------------------
    
    
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// putStockVariable
//

bool CVariableStorage::putStockVariable( CVSCPVariable& var, 
                                            CUserItem *pUser )
{
    wxString wxstr;
    wxString lcname = var.getName().Lower();
    
    // Make sure it starts with ".vscp"
    if ( !lcname.StartsWith("vscp.") ) return false;  
    
    ///////////////////////////////////////////////////////////////////////////
    //                               DEBUG
    ///////////////////////////////////////////////////////////////////////////
    
    if ( lcname.StartsWith( _("vscp.debug.flags1") ) ) {
        long value;
        var.getValue( &value );
        gpobj->m_debugFlags1 = (uint32_t)value;
        return true;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.debug.flags2") ) ) {
        long value;
        var.getValue( &value );
        gpobj->m_debugFlags2 = (uint32_t)value;
        return true;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.debug.flags3") ) ) {
        long value;
        var.getValue( &value );
        gpobj->m_debugFlags3 = (uint32_t)value;
        return true;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.debug.flags4") ) ) {
        long value;
        var.getValue( &value );
        gpobj->m_debugFlags4 = (uint32_t)value;
        return true;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.debug.flags5") ) ) {
        long value;
        var.getValue( &value );
        gpobj->m_debugFlags5 = (uint32_t)value;
        return true;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.debug.flags6") ) ) {
        long value;
        var.getValue( &value );
        gpobj->m_debugFlags6 = (uint32_t)value;
        return true;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.debug.flags7") ) ) {
        long value;
        var.getValue( &value );
        gpobj->m_debugFlags7 = (uint32_t)value;
        return true;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.debug.flags8") ) ) {
        long value;
        var.getValue( &value );
        gpobj->m_debugFlags8 = (uint32_t)value;
        return true;   // None writable
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //                              VERSIONS
    ///////////////////////////////////////////////////////////////////////////
    
    if ( lcname.StartsWith( _("vscp.version.major") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.version.minor") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.version.build") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.version.str") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.copyright.vscp") ) ) {
        return false;   // None writable
    }
    
    // *************************************************************************
    //                               wxWidgets
    // *************************************************************************
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.version.str") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.version.major") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.version.minor") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.version.release") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.version.sub") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.wxwidgets.copyright") ) ) {
        return false;   // None writable
    }

    // *************************************************************************
    //                               Mongoose
    // *************************************************************************

    if ( lcname.StartsWith( _("vscp.mongoose.copyright") ) ) {
        return false;   // None writable
    }

// *****************************************************************************
//                                 LUA
// *****************************************************************************


    if ( lcname.StartsWith( _("vscp.lua.copyright") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.lua.version.major") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.lua.version.minor") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.lua.version.release") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.lua.version.str") ) ) {
        return false;   // None writable
    }


// *****************************************************************************
//                                SQLite
// *****************************************************************************

    if ( lcname.StartsWith( _("vscp.sqlite.version.major") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.sqlite.version.minor") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.sqlite.version.release") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.sqlite.version.build") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.sqlite.version.str") ) ) {
        return false;   // None writable
    }

// *****************************************************************************
//                                OPENSSL
// *****************************************************************************
    
#ifndef WIN32
    if ( lcname.StartsWith( _("vscp.openssl.version.str") ) ) {
        return false;   // None writable
    }
#endif
    
    
// *****************************************************************************
//                                 OS
// *****************************************************************************    
    
    
    if ( lcname.StartsWith( _("vscp.os.str") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.os.wordwidth") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.os.wordwidth.str") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.os.width.is64bit") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.os.width.is32bit") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.os.endiness.str") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.os.endiness.islittleendian") ) ) {
        return false;   // None writable
    }
    
// *****************************************************************************
//                                 HOST
// *****************************************************************************    
    
    if ( lcname.StartsWith( _("vscp.host.rootpath") ) ) {
        return false;
    }
    
    if ( lcname.StartsWith( _("vscp.host.name") ) ) {
        gpobj->m_strServerName = var.getValue();
        return gpobj->updateConfigurationRecordItem( _("vscpd_servername"), 
                                                    wxString::Format( _("%s"), 
                                                    gpobj->m_strServerName.mbc_str() ) );
    }
    
    if ( lcname.StartsWith( _("vscp.host.ip") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.host.mac") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.host.userid") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith(  _("vscp.host.username") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.host.guid") ) ) {        
        gpobj->m_guid.getFromString( var.getValue() );
    }
    
    if ( lcname.StartsWith( _("vscp.loglevel") ) ) {
        int val;
        var.getValue( &val );
        gpobj->m_logLevel = val;
        return gpobj->updateConfigurationRecordItem( _("vscpd_LogLevel"), 
                                                    val ? _("1") : _("0") );      
    }
    
    if ( lcname.StartsWith( _("vscp.loglevel.str") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.client.receivequeue.max") ) ) {
        int val;
        var.getValue( &val );
        gpobj->m_maxItemsInClientReceiveQueue = val;
        return gpobj->updateConfigurationRecordItem( _("vscpd.maxqueue"), 
                                             wxString::Format( _("%d"), val ) );
    }
    
    if ( lcname.StartsWith( _("vscp.tcpip.address") ) ) {
        wxString strval;
        strval = var.getValue();
        gpobj->m_strTcpInterfaceAddress = strval;
        return gpobj->updateConfigurationRecordItem( _("vscpd_TcpipInterface_Address"), 
                                                        strval );
    }
    
    if ( lcname.StartsWith( _("vscp.udp.enable") ) ) {
        int val;
        var.getValue( &val );
        gpobj->m_udpInfo.m_bEnable = val;
        return gpobj->updateConfigurationRecordItem( _("vscpd_UdpSimpleInterface_Enable"), 
                                                        val ? _("1") : _("0") );
    }
    
    if ( lcname.StartsWith( _("vscp.udp.address") ) ) {
        wxString strval;
        strval = var.getValue();
        gpobj->m_udpInfo.m_interface  = strval;
        return gpobj->updateConfigurationRecordItem( _("vscpd_UdpSimpleInterface_Address"), 
                                                    strval );
    }
      
    
    if ( lcname.StartsWith( _("vscp.automation.heartbeat.enable") ) ) {
        bool val;
        var.getValue( &val );
        val ? gpobj->m_automation.enableHeartbeatEvent() : gpobj->m_automation.disableHeartbeatEvent();
        return gpobj->updateConfigurationRecordItem( _("vscpd_Automation_HeartbeatEvent_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    
    if ( lcname.StartsWith( _("vscp.automation.heartbeat.period") ) ) {
        int val;
        var.getValue( &val );
        gpobj->m_logLevel = val;
        return gpobj->updateConfigurationRecordItem( _("vscpd_Automation_HeartbeatEvent_Interval"), 
                                                    wxString::Format(_("%d"), val ) );
    }
    
    if ( lcname.StartsWith( _("vscp.automation.heartbeat.last") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith(  _("vscp.automation.segctrl-heartbeat.enable") ) ) {
        int val;
        var.getValue( &val );
        gpobj->m_logLevel = val;
        return gpobj->updateConfigurationRecordItem( _("vscpd_LogLevel"), 
                                                    val ? _("1") : _("0") );
    }
    
    if ( lcname.StartsWith( _("vscp.automation.segctrl.heartbeat.period") ) ) {
        long val;
        var.getValue( &val );
        gpobj->m_automation.setSegmentControllerHeartbeatInterval( val );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Automation_SegmentControllerEvent_Interval"), 
                                                    wxString::Format(_("%ld"), val) );
    }
    
    if ( lcname.StartsWith( _("vscp.automation.segctrl.heartbeat.last") ) ) {
        return false;   // None writable
    }
    
    if ( lcname.StartsWith( _("vscp.automation.longitude") ) ) {
        double val;
        var.getValue( &val );
        gpobj->m_automation.setLongitude( val );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Automation_Longitude"), 
                                                    wxString::Format( _("%f"), gpobj->m_automation.getLongitude() ) );
    }
    
    if ( lcname.StartsWith( _("vscp.automation.latitude") ) ) {
        double val;
        var.getValue( &val );
        gpobj->m_automation.setLatitude( val );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Automation_Latitude"), 
                                                    wxString::Format( _("%f"), gpobj->m_automation.getLatitude() ) );
    }
    
    if ( lcname.StartsWith( _("vscp.automation.twilightsunriseevent.enable") ) ) {
        int val;
        var.getValue( &val );
        gpobj->m_automation.enableSunRiseTwilightEvent( val );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Automation_SunriseTwilight_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    
    if ( lcname.StartsWith( _("vscp.automation.twilightsunsetevent.enable") ) ) {
        int val;
        var.getValue( &val );
        gpobj->m_automation.enableSunSetTwilightEvent( val );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Automation_SunsetTwilight_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    
    if ( lcname.StartsWith( _("vscp.automation.sunriseevent.enable") ) ) {
        int val;
        var.getValue( &val );
        gpobj->m_automation.enableSunRiseEvent( val );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Automation_Sunrise_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    
    if ( lcname.StartsWith( _("vscp.automation.sunsetevent.enable") ) ) {
        int val;
        var.getValue( &val );
        gpobj->m_automation.enableSunSetEvent( val );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Automation_Sunset_Enable"),
                                                    val ? _("1") : _("0") );
    }
    
    if ( lcname.StartsWith( _("vscp.workingfolder") ) ) {
        return false;   // Not writable
    }
 

// *****************************************************************************
//                                WEB-Server
// *****************************************************************************


    if ( lcname.StartsWith( _("vscp.websrv.address") ) ) {
        wxString strval;
        strval = var.getValue();
        gpobj->m_strWebServerInterfaceAddress = strval;
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_Address"), 
                                                    strval );
    }
        
    if ( lcname.StartsWith( _("vscp.websrv.root.path") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_pathWebRoot, 0, sizeof( gpobj->m_pathWebRoot ) );
        memcpy( gpobj->m_pathWebRoot, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_pathWebRoot)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_RootPath"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.cert.path") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_pathCert, 0, sizeof( gpobj->m_pathCert ) );
        memcpy( gpobj->m_pathCert, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_pathCert)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_PathCert"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.extramimetypes") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_extraMimeTypes, 0, sizeof( gpobj->m_extraMimeTypes ) );
        memcpy( gpobj->m_extraMimeTypes, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_extraMimeTypes)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_ExtraMimeTypes"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.ssipatterns") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_ssi_pattern, 0, sizeof( gpobj->m_ssi_pattern ) );
        memcpy( gpobj->m_ssi_pattern, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_ssi_pattern)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_SSIPattern"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.ipacl") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_ip_acl, 0, sizeof( gpobj->m_ip_acl ) );
        memcpy( gpobj->m_ip_acl, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_ip_acl)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_IpAcl"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.cgi.interpreter") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_cgiInterpreter, 0, sizeof( gpobj->m_cgiInterpreter ) );
        memcpy( gpobj->m_cgiInterpreter, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_cgiInterpreter)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_CgiInterpreter"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.cgi.pattern") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_cgiPattern, 0, sizeof( gpobj->m_cgiPattern ) );
        memcpy( gpobj->m_cgiPattern, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_cgiPattern)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_CgiPattern"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.directorylistings.enable") ) ) {
        int val;
        var.getValue( &val );
        strcpy( gpobj->m_EnableDirectoryListings,
                val ? "yes" : "no" );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_EnableDirectoryListings"), 
                                                    val ? _("1") : _("0") );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.hidefile.pattern") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_hideFilePatterns, 0, sizeof( gpobj->m_hideFilePatterns ) );
        memcpy( gpobj->m_hideFilePatterns, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_hideFilePatterns)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_HideFilePatterns"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.indexfiles") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_indexFiles, 0, sizeof( gpobj->m_indexFiles ) );
        memcpy( gpobj->m_indexFiles, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_indexFiles)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_IndexFiles"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.urlrewrites") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_urlRewrites, 0, sizeof( gpobj->m_urlRewrites ) );
        memcpy( gpobj->m_urlRewrites, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_urlRewrites)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_UrlRewrites"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.auth.file.directory") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_per_directory_auth_file, 0, sizeof( gpobj->m_per_directory_auth_file ) );
        memcpy( gpobj->m_per_directory_auth_file, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_per_directory_auth_file)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_PerDirectoryAuthFile"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.websrv.auth.file.global") ) ) {
        wxString strval;
        strval = var.getValue();
        memset( gpobj->m_global_auth_file, 0, sizeof( gpobj->m_global_auth_file ) );
        memcpy( gpobj->m_global_auth_file, 
                    (const char *)strval.mbc_str(), 
                    wxMin( sizeof( gpobj->m_global_auth_file)-1, strval.Length() ) );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Webserver_GlobalAuthFile"), 
                                                    strval );
    }


// *****************************************************************************
//                            Websocket-Server
// *****************************************************************************




// *****************************************************************************
//                            Decision Matrix
// *****************************************************************************

    
    if ( lcname.StartsWith( _("vscp.dm.xml.path") ) ) {
        wxString strval;
        strval = var.getValue();
        var.setValue( gpobj->m_dm.m_staticXMLPath );
        return gpobj->updateConfigurationRecordItem( _("vscpd_DM_XML_Path"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.dm.db.path") ) ) {
        wxString strval;
        strval = var.getValue();
        var.setValue( gpobj->m_dm.m_staticXMLPath );
        return gpobj->updateConfigurationRecordItem( _("vscpd_DM_DB_Path"), 
                                                    strval );
    }


// *****************************************************************************
//                             Variables
// *****************************************************************************

    if ( lcname.StartsWith( _("vscp.variable.db.path") ) ) {
        wxString strval;
        strval = var.getValue();
        var.setValue( gpobj->m_variables.m_dbFilename.GetFullPath() );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Variables_DB_Path"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith( _("vscp.variable.xml.path") ) ) {
        wxString strval;
        strval = var.getValue();
        var.setValue( gpobj->m_variables.m_xmlPath );
        return gpobj->updateConfigurationRecordItem( _("vscpd_Variables_XML_Path"), 
                                                    strval );
    }


// *****************************************************************************
//                              Log files
// *****************************************************************************

    // Enable syslog logging
    if ( lcname.StartsWith( _("vscp.log.syslog.enable") ) ) {
        bool val;
        var.getValue( &val );
        gpobj->m_bLogToSysLog = val;
        return gpobj->updateConfigurationRecordItem( _("vscpd_Syslog_Enable"), 
                                                    val ? _("1") : _("0") );
    }
    
    if ( lcname.StartsWith(  _("vscp.log.database.path") ) ) {
        wxString strval;
        strval = var.getValue();
        gpobj->m_path_db_vscp_data.SetPath( strval );
        return gpobj->updateConfigurationRecordItem( _("vscpd_GeneralLogFile_Path"), 
                                                    strval );
    }  
    
    // Write SQL value for log database retreival
    if ( lcname.StartsWith(  _("vscp.log.database.sql") ) ) {
        // Handled by the stock variable itself 
        return true;
    }

    
// *****************************************************************************
//                             Databases
// *****************************************************************************    
    
    if ( lcname.StartsWith(  _("vscp.database.vscpdata.path") ) ) {
        wxString strval;
        strval = var.getValue();
        gpobj->m_path_db_vscp_data.SetPath( strval );
        return gpobj->updateConfigurationRecordItem( _("vscpd_db_data_path"), 
                                                    strval );
    }
    
    if ( lcname.StartsWith(  _("vscp.database.vscpdconfig.path") ) ) {
        wxString strval;
        strval = var.getValue();
        gpobj->m_path_db_vscp_daemon.SetPath( strval );
        return gpobj->updateConfigurationRecordItem( _("vscpd_db_vscpconf_path"), 
                                                    strval );
    }
    
    
// *****************************************************************************
//                             Decision Matrix
// *****************************************************************************
    if ( lcname.StartsWith( _("vscp.dm.count") ) ) {
        return false;   // None writable
    }
    
    else if ( lcname.StartsWith( _("vscp.dm.count.active") ) ) {
        return false;   // None writable      
    }
    
    else if ( lcname.StartsWith( _("vscp.dm.logging.enable") ) ) {
        return false;   // None writable      
    }
    
    else if ( lcname.StartsWith( _("vscp.dm.logging.path") ) ) {
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
            if ( !gpobj->m_dm.getDatabaseRecord( id, &dmDB ) ) return false;
            
            // Get a pointer to memory DM row (if any)
            dmElement *pdm = 
                    gpobj->m_dm.getMemoryElementFromId( var.getID(), &row );
                      
            if ( wxstr.StartsWith( _("id") ) ) {
                return false;   // None writable
            }
            
            else if ( wxstr.StartsWith( _("bEnable") ) ) {
                
                bool bVal;
                wxString strValue;
                
                var.getValue( &bVal );
                
                // Update database record                                
                bVal ? strValue=_("1") : _("0");
                if ( !gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                _("bEnable"),
                                                                strValue ) ) {
                    return false;
                }
                
                // if the record is in memory
                if ( NULL != pdm ) {
                    
                    pdm->m_bEnable = bVal;
                    
                    // if value is false the database row should be removed
                    if ( !bVal ) {
                        gpobj->m_dm.removeMemoryElement( row );
                    }
                    
                }      
                
                // If true and not in memory the record should be added
                else {
                    dmElement *pdmnew = new dmElement();
                    if ( NULL!= pdmnew ) return false;
                    gpobj->m_dm.getDatabaseRecord( dmDB.m_id, pdmnew );
                    gpobj->m_dm.addMemoryElement( pdmnew );
                }
                              
            }
            else if ( wxstr.StartsWith( _("groupid") ) ) {
  
                wxString strValue;
                
                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_strGroupID = strValue;
                }
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                               _("groupid"),
                                                               strValue );
                
            }
            else if ( wxstr.StartsWith( _("mask.priority") ) ) {

                int val;
                wxString strValue;
               
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.mask_priority = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("maskPriority"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("mask.class") ) ) {
                
                int val;
                wxString strValue;
                                
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.mask_class = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("maskClass"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("mask.type") ) ) {
                
                int val;
                wxString strValue;
                               
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.mask_type = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("maskType"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("mask.guid") ) ) {
                
                wxString strValue;

                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    cguid guid;
                    guid.getFromString( strValue );
                    memcpy( pdm->m_vscpfilter.mask_GUID, guid.getGUID(), 16 );
                }
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("maskGUID"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("filter.priority") ) ) {
                
                int val;
                wxString strValue;
                
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.filter_priority = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("filterPriority"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("filter.class") ) ) {
                
                int val;
                wxString strValue;
               
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.filter_class = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("filterClass"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("filter.type") ) ) {
                
                int val;
                wxString strValue;
              
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_vscpfilter.filter_type = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("filterType"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("filter.guid") ) ) {
                
                wxString strValue;

                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    cguid guid;
                    guid.getFromString( strValue );
                    memcpy( pdm->m_vscpfilter.filter_GUID, guid.getGUID(), 16 );
                }
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("filterGUID"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("allowed.start") ) ) {
                
                wxString strValue;
                
                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {                    
                    pdm->m_timeAllow.m_fromTime.ParseISOCombined( strValue );
                }
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedStart"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("allowed.end") ) ) {
                
                wxString strValue;

                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.m_endTime.ParseISOCombined( strValue );
                }
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedEnd"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("allowed.monday") ) ) {
                
                bool bVal;
                wxString strValue;
                
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowMonday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedMonday"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("allowed.tuesday") ) ) {
                
                bool bVal;
                wxString strValue;

                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowTuesday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedTuesday"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("allowed.wednessday") ) ) {
                
                bool bVal;
                wxString strValue;
            
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowWednesday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedWednessday"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("allowed.thursday") ) ) {
                
                bool bVal;
                wxString strValue;
               
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowThursday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedThursday"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("allowed.friday") ) ) {
                
                bool bVal;
                wxString strValue;
             
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowFriday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedFriday"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("allowed.saturday") ) ) {
                
                bool bVal;
                wxString strValue;
                
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowSaturday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedSaturday"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("allowed.sunday") ) ) {
                
                bool bVal;                
                wxString strValue;
               
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.allowSunday( bVal );
                }
                bVal ? strValue=_("1") : _("0");
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedSunday"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("allowed.time") ) ) {
                
                wxString strValue;
 
                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_timeAllow.parseActionTime( strValue );
                }
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("allowedTime"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("bCheckIndex") ) ) {
                
                bool bVal;                
                wxString strValue;
              
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_bCheckIndex = bVal;
                }
                bVal ? strValue=_("1") : _("0");
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("bCheckIndex"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("index") ) ) {
                
                int val;
                
                wxString strValue;
                
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_index = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("index"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("bCheckZone") ) ) {
                
                bool bVal;
                wxString strValue;

                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_bCheckZone = bVal;
                }
                bVal ? strValue=_("1") : _("0");
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("bCheckZone"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("zone") ) ) {
                
                int val;
                wxString strValue;
                              
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_zone = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("zone"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("bCheckSubZone") ) ) {
                
                bool bVal;
                wxString strValue;
                             
                var.getValue( &bVal );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_bCheckZone = bVal;
                }
                bVal ? strValue=_("1") : _("0");
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("bCheckSubZone"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("subzone") ) ) {
                
                int val;
                wxString strValue;
                               
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_subzone = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("subzone"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("measurement.value") ) ) {
                
                double val;
                wxString strValue;
              
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_measurementValue = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("measurementValue"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("measurement.unit") ) ) {
                
                int val;
                wxString strValue;
                              
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_measurementUnit = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("measurementUnit"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("measurement.compare") ) ) {
                
                int val;
                wxString strValue;
                              
                var.getValue( &val );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_measurementUnit = val;
                }
                strValue.Format( _("%d"), val );
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("measurementCompare"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("measurement.compare.string") ) ) {
                return false;
            }
            else if ( wxstr.StartsWith( _("comment") ) ) {
                
                wxString strValue;
                
                var.getValue( &strValue );
                // Change in memory record
                if ( NULL != pdm ) {
                    pdm->m_comment = strValue;
                }
                return gpobj->m_dm.updateDatabaseRecordItem( id,
                                                                    _("comment"),
                                                                    strValue );
            }
            else if ( wxstr.StartsWith( _("count.trigger") ) ) {
                if ( NULL != pdm ) {                
                    pdm->m_triggCounter = 0;
                }
            }
            else if ( wxstr.StartsWith( _("count.error") ) ) {
                if ( NULL == pdm ) {                
                    pdm->m_errorCounter = 0;
                }
            }
            else if ( ( wxstr.StartsWith( _("error") ) ) || ( wxstr.StartsWith( _("error.string") ) ) ) {
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
    
    if ( lcname.StartsWith( _("vscp.driver.") ) ) {
        
    }
    
// *****************************************************************************
//                             Interfaces
// ***************************************************************************** 
    
    if ( lcname.StartsWith( _("vscp.interface.") ) ) {
        ;
    }
    
// *****************************************************************************
//                              Discovery
// ***************************************************************************** 
    
    if ( lcname.StartsWith( _("vscp.discovery.") ) ) {
        ;
    }
    
// *****************************************************************************
//                                 Log
// *****************************************************************************    
    
    if ( lcname.StartsWith( _("vscp.log.") ) ) {
        ;
    }
    
// *****************************************************************************
//                                Tables
// *****************************************************************************    
    
    if ( lcname.StartsWith( _("vscp.table.") ) ) {
        ;
    }
    
// *****************************************************************************
//                                Users
// *****************************************************************************    
      
    if ( lcname.StartsWith( _("vscp.user.") ) ) {
        
        wxStringTokenizer tkz( lcname, _(".") );
        wxString strToken;
        strToken = tkz.GetNextToken();  // vscp
        strToken = tkz.GetNextToken();  // user
        
        if ( !tkz.HasMoreTokens() ) return false;
        strToken = tkz.GetNextToken();  // vscp
        
        // Add new user record
        if (  strToken.StartsWith( _("add") ) ) {
            
            // String is BASE64 encoded so it must be decoded before it
            // can be used
            wxstr = var.getValue();  
            
            size_t len = wxBase64Decode( NULL, 0, wxstr );
            if ( 0 == len ) return false;
            uint8_t *pbuf = new uint8_t[len];
            if ( NULL == pbuf ) return false;
            len = wxBase64Decode( pbuf, len, wxstr );
            wxstr = wxString::FromUTF8( (const char *)pbuf, len );
            delete [] pbuf;
            
            return gpobj->m_userList.addUser( wxstr, true );
        }
        // Delete a user record (value = userid)
        if (  strToken.StartsWith( _("delete") ) ) {
            
            // String is BASE64 encoded so it must be decoded before it
            // can be used
            wxstr = var.getValue();
            
            const char *p = wxstr.mbc_str();
            size_t len = wxBase64Decode( NULL, 0, wxstr );
            if ( 0 == len ) return false;
            uint8_t *pbuf = new uint8_t[len];
            if ( NULL == pbuf ) return false;
            len = wxBase64Decode( pbuf, len, wxstr );
            wxstr = wxString::FromUTF8( (const char *)pbuf, len );
            delete [] pbuf;
            return gpobj->m_userList.deleteUser( wxstr );
        }
        else {
            
            // Token should be a user ordinal number
            
            unsigned long idx;
            if ( !strToken.ToULong( &idx ) ) return false;
            
            CUserItem *pUserItem = gpobj->m_userList.getUserItemFromOrdinal( idx );
            if ( NULL == pUserItem ) return false;
            
            if ( !tkz.HasMoreTokens() ) {
                
                // write
                wxstr = var.getValue();
                
                size_t len = wxBase64Decode( NULL, 0, wxstr );
                if ( 0 == len ) return false;
                uint8_t *pbuf = new uint8_t[len];
                if ( NULL == pbuf ) return false;
                len = wxBase64Decode( pbuf, len, wxstr );
                wxstr = wxString::FromUTF8( (const char *)pbuf, len );
                delete [] pbuf;
                
                if ( pUserItem->setFromString( wxstr ) ) {
                    pUserItem->saveToDatabase();
                    return true;
                }
            }
            
            strToken = tkz.GetNextToken();
            
            if ( strToken.StartsWith( _("write") ) ) {
                pUserItem->setFromString( var.getValue() );
            }
            else if ( strToken.StartsWith( _("name") )  ) {
                pUserItem->setUserName( var.getValue() );
            }
            else if ( strToken.StartsWith( _("password") )  ) {
                pUserItem->setPassword( var.getValue() );
            }
            else if ( strToken.StartsWith( _("fullname") )  ) {
                pUserItem->setFullname( var.getValue() );
            }
            else if ( strToken.StartsWith( _("filter") )  ) {
                pUserItem->setFilterFromString( var.getValue() );
            }
            else if ( strToken.StartsWith( _("rights") ) ) {
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
            else if ( strToken.StartsWith( _("allowed") )  ) {
                if ( !tkz.HasMoreTokens() ) return false;
                strToken = tkz.GetNextToken();
                if ( _("events") == strToken ) {
                    pUserItem->setAllowedEventsFromString( var.getValue() );
                }
                else if ( strToken.StartsWith( _("remotes") )  ) {
                    pUserItem->setAllowedRemotesFromString( var.getValue() );
                }
                else {
                    return false;
                }
            }
            else if ( strToken.StartsWith( _("note") )  ) {
                pUserItem->setNote( var.getValue() );
            }
            else {
                return false;
            }
            
        }
        
    }
    
// ----------------------------- Not Found -------------------------------------    
    
    return false; // Not a stock variable
}

///////////////////////////////////////////////////////////////////////////////
// setVariableFromDbRecord
//

bool CVariableStorage::setVariableFromDbRecord( sqlite3_stmt *ppStmt,
                                                    CVSCPVariable& variable )
{
    // Check pointer
    if ( NULL == ppStmt ) return false;
    
    // ID
    variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_ID ) );
    
    // Last changed
    variable.m_lastChanged.ParseISOCombined( 
            (const char *)sqlite3_column_text( ppStmt, 
                                                VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
    
    // Name
    variable.setName( wxString::FromUTF8( 
            (const char *)sqlite3_column_text( ppStmt, 
                                                VSCPDB_ORDINAL_VARIABLE_NAME ) ) );
    
    // Variable type
    variable.setType( sqlite3_column_int( ppStmt, 
                                            VSCPDB_ORDINAL_VARIABLE_TYPE ) );
    
    // Value
    variable.setValue( wxString::FromUTF8( 
            (const char *)sqlite3_column_text( ppStmt, 
                                            VSCPDB_ORDINAL_VARIABLE_VALUE ) ) );
    
    // Persistence
    variable.setPersistent( sqlite3_column_int( ppStmt, 
                                VSCPDB_ORDINAL_VARIABLE_PERSISTENT ) ? true : false );
    
    // Owner
    variable.setOwnerID( sqlite3_column_int( ppStmt, 
                                                VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) );
    
    // Access rights
    variable.setAccessRights( sqlite3_column_int( ppStmt, 
                                                    VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
    
    // Note
    variable.setNote( wxString::FromUTF8( 
            (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// findNonPersistentVariable
//

uint32_t CVariableStorage::findNonPersistentVariable( const wxString& name, 
                                                        CVSCPVariable& variable )
{
    sqlite3_stmt *ppStmt;
    char psql[ 8192 ];
    
    variable.setID( 0 );    // Invalid id
    
    sprintf( psql,
                VSCPDB_VARIABLE_FIND_FROM_NAME, 
                (const char *)name.Upper().mbc_str() );
    
    if ( NULL == m_db_vscp_internal_variable ) {
        return 0;
    }
    
    if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_internal_variable,
                                            psql,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        return 0;
    }
    
    // Get the data for the variable
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        setVariableFromDbRecord( ppStmt, variable );
        /*
        variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_ID ) );
        variable.m_lastChanged.ParseISOCombined( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
        variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) ) );
        variable.setType( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE ) ) );
        variable.setPersistent( false );
        variable.setUserID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) ); 
        variable.setAccessRights( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
        */
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
    
    if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_external_variable,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        return 0;
    }
    
    // Get the result
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        setVariableFromDbRecord( ppStmt, variable );
        /*
        variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_ID ) );
        variable.m_lastChanged.ParseISOCombined( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
        variable.setName( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) ) );
        variable.setType( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE ) );
        variable.setValue( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE ) ) );
        variable.setPersistent( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERSISTENT ) ? true : false );
        variable.setUserID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) );
        variable.setAccessRights( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
        variable.setNote( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
        */
    }
    
    sqlite3_finalize( ppStmt );
    
    return variable.getID();
}

///////////////////////////////////////////////////////////////////////////////
// getVarlistFromRegExp
//

bool CVariableStorage::getVarlistFromRegExp( wxArrayString& nameArray, 
                                                const wxString& regex,
                                                const int type,
                                                bool bClear )
{
    wxString varname;
    sqlite3_stmt *ppStmt;
    wxString regexlocal = regex.Upper();
        
    // Clear array if set
    if ( bClear ) nameArray.Clear();
    
    regexlocal.Trim();
    if ( 0 == regexlocal.Length() ) regexlocal = _("(.*)");  // List all if empty
    wxRegEx wxregex( regexlocal );
    if ( !wxregex.IsValid() ) {
        
        nameArray.Empty();
        gpobj->logMsg( wxString::Format( _("[getVarlistFromRegExp] Invalid "
                                           "regular expression [%s]"), 
                            (const char *)regexlocal.mbc_str() ) );
        return false;
        
    }
        
    if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_internal_variable,
                                            VSCPDB_VARIABLE_FIND_ALL,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        const unsigned char *p = sqlite3_column_text( ppStmt, 
                                                VSCPDB_ORDINAL_VARIABLE_NAME );
        varname = wxString::FromUTF8Unchecked( (const char *)p );
        if ( wxregex.Matches( varname ) ) {
            nameArray.Add( varname );
        }
    }

    sqlite3_finalize( ppStmt );
        
    if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_external_variable,
                                            VSCPDB_VARIABLE_FIND_ALL,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        sqlite3_finalize( ppStmt );
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        varname = wxString::FromUTF8( 
                (const char *)sqlite3_column_text( ppStmt, 
                                               VSCPDB_ORDINAL_VARIABLE_NAME ) );
        if ( wxregex.Matches( varname ) ) {
            nameArray.Add( varname );
        }
    }

    sqlite3_finalize( ppStmt );
    
    // Sort the array
    nameArray.Sort();
        
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// listEnumerationFromRegExp
//

bool CVariableStorage::listEnumerationFromRegExp( varQuery *pq, 
                                                    const wxString& regex,
                                                    const int type )
{
    wxString varname;
    wxString regexlocal = regex.Upper();
    
    // Must be a valid pointer
    if ( NULL == pq ) return false;
        
    regexlocal.Trim();
    if ( 0 == regexlocal.Length() ) regexlocal = _("(.*)");  // List all if empty
    wxRegEx wxregex( regexlocal );
    if ( !wxregex.IsValid() ) {
        
        gpobj->logMsg( wxString::Format( _("[getVarlistFromRegExp] Invalid "
                                           "regular expression [%s]"), 
                            (const char *)regexlocal.mbc_str() ) );
        return false;
        
    }
    
    if ( ( VARIABLE_INTERNAL | pq->tableType ) || ( VARIABLE_STOCK | pq->tableType )  ) {
        
        if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_internal_variable,
                                                VSCPDB_VARIABLE_FIND_ALL,
                                                -1,
                                                &pq->ppStmt,
                                                NULL ) ) {
            return false;
        }
        
    }
    else if ( VARIABLE_EXTERNAL | pq->tableType ) {
        
        if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_external_variable,
                                                VSCPDB_VARIABLE_FIND_ALL,
                                                -1,
                                                &pq->ppStmt,
                                                NULL ) ) {
            return false;
        }
        
    }   
    else {
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// listItem
//

bool CVariableStorage::listItem( varQuery *pq, CVSCPVariable& variable )
{
    // Must be a valid pointer
    if ( NULL == pq ) return false;

 try_again:   
    
    if ( ( VARIABLE_INTERNAL | pq->tableType ) || ( VARIABLE_STOCK | pq->tableType )  ) {
        
        if ( SQLITE_ROW != sqlite3_step( pq->ppStmt ) ) return false; 
                
        setVariableFromDbRecord( pq->ppStmt, variable );
        
        // If stock variable is requested and it is not try again
        if ( ( VARIABLE_STOCK == pq->tableType ) &&
               ( !variable.isStockVariable() ) ) {
            goto try_again;
        }
        
    }
    else if ( VARIABLE_EXTERNAL | pq->tableType  ) {
        
        if ( SQLITE_ROW != sqlite3_step( pq->ppStmt ) ) return false;
        
        setVariableFromDbRecord( pq->ppStmt, variable );
        
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
    
    if ( VARIABLE_STOCK == pq->tableType  ) {
        // Nothing to do
    }
    else if ( VARIABLE_INTERNAL == pq->tableType  ) {
        if (NULL != pq->ppStmt ) sqlite3_finalize( pq->ppStmt );
    }
    else if ( VARIABLE_EXTERNAL == pq->tableType  ) {
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
    
    var.setStockVariable();  // Make sure it's marked as a stock variable
    
    char *sql = sqlite3_mprintf( VSCPDB_VARIABLE_INSERT,
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
        gpobj->logMsg( wxString::Format( _("[Add variable] Variable name can't be empty.\n") ) );
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
                var.getOwnerID(),
                var.getAccessRights(),
                (const char *) var.getNote().mbc_str(),
                id );

        if (SQLITE_OK != sqlite3_exec( ( var.isPersistent() ? m_db_vscp_external_variable : m_db_vscp_internal_variable ), 
                                            sql, NULL, NULL, &zErrMsg)) {            
            gpobj->logMsg( wxString::Format( _("[Add variable] Unable to update variable in db. [%s] Err=%s\n"), sql, zErrMsg ) );
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
                var.getOwnerID(),
                var.getAccessRights(),
                (const char *) var.getNote().mbc_str() );  

        if ( SQLITE_OK != sqlite3_exec( ( var.isPersistent() ? m_db_vscp_external_variable : m_db_vscp_internal_variable ), 
                                            sql, NULL, NULL, &zErrMsg ) ) {
            fprintf(stderr, "Error: %s\n", sqlite3_errmsg( m_db_vscp_external_variable ) );
            gpobj->logMsg( wxString::Format( _("[Add variable] Unable to add variable in db. [%s] Err=%s\n"), sql, zErrMsg ) );
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
                                    const uint16_t type,
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
    variable.setOwnerID( userid );
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
    long id = 0;
    char *zErrMsg = 0;
    
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
                var.getOwnerID(),
                var.getAccessRights(),
                (const char *) var.getNote().mbc_str(),
                id );

    if (SQLITE_OK != sqlite3_exec( ( var.isPersistent() ? m_db_vscp_external_variable : m_db_vscp_internal_variable ), 
                                            sql, NULL, NULL, &zErrMsg)) {            
        gpobj->logMsg( wxString::Format( _("[Add variable] Unable to update variable in db. [%s] Err=%s\n"), sql, zErrMsg ) );
        sqlite3_free(sql);
        return false;
    }

    sqlite3_free(sql);    
    
    // If stock variables update locally
    if ( var.isStockVariable() ) {
        putStockVariable( var );
    }
    
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
            gpobj->logMsg( wxString::Format( _("[Delete variable] Unable to delete variable in db. [%s] Err=%s\n"), sql, zErrMsg ) );
            return false;
        }
        
        sqlite3_free( sql );                            
    }
    else {        
        char *sql = sqlite3_mprintf( VSCPDB_VARIABLE_WITH_ID,
                                    variable.getID() );
        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_internal_variable, sql, NULL, NULL, &zErrMsg ) ) {
            sqlite3_free( sql );
            gpobj->logMsg( wxString::Format( _("[Delete variable] Unable to delete variable in db. [%s] Err=%s\n"), sql, zErrMsg ) );
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

        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_VARIABLE ) {
            gpobj->logMsg(wxString::Format(_("[loadFromXML] Loading variable XML file from %s.\n"),
                    (const char *) m_xmlPath.mbc_str()),
                    DAEMON_LOGMSG_DEBUG);
        }

        if (!doc.Load(m_xmlPath)) {
            gpobj->logMsg(_("[loadFromXML] Failed to load variable XML file from standard XML path.\n"));
            return false;
        }

    } 
    else {

        if (gpobj->m_debugFlags1 & VSCP_DEBUG1_VARIABLE) {
            gpobj->logMsg(wxString::Format(_("[loadFromXML] Loading variable XML file from %s.\n"),
                    (const char *) path.mbc_str()),
                    DAEMON_LOGMSG_DEBUG);
        }

        if (!doc.Load(path)) {
            gpobj->logMsg(wxString::Format(_("[loadFromXML] Failed to load variable XML file from %s.\n"),
                    (const char *) path.mbc_str()));
            return false;
        }
    }

    // start processing the XML file
    if (!(doc.GetRoot()->GetName() != _("variables") ||
            doc.GetRoot()->GetName() != _("persistent"))) {
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {
        
        if ( child->GetName() == _("variable") ) {

            CVSCPVariable variable;

            // Mark last changed as now
            variable.setLastChangedToNow();
                       
            // Get variable type - String is default           
            variable.setType( variable.getVariableTypeFromString( child->GetAttribute( _("type"), 
                                                                    _("string") ) ) );
            
            // Persistence
            wxstr = child->GetAttribute( _("persistent"), _("false") );
            wxstr.MakeUpper();
            if ( wxNOT_FOUND != wxstr.Find( _("TRUE") ) ) {
                variable.setPersistent( true ); 
            }
            else if ( wxNOT_FOUND != wxstr.Find( _("YES") ) ) {
                variable.setPersistent( true ); 
            } 
                    
            // userid
            if ( child->GetAttribute( _("user"), _("0") ).ToULong( &lval ) ) {
                variable.setOwnerID( lval );
            }
            
            // access-rights 0x744 = 1860
            wxstr = child->GetAttribute( _("access-rights"), _("0x744") );
            lval = vscp_readStringValue( wxstr );
            variable.setAccessRights( lval );
            
            // name
            wxstr = child->GetAttribute( _("name"), _("") );
            wxstr.MakeUpper();
            wxstr.Trim();
            wxstr.Trim( false );
            if ( wxstr.Length() ) {
                variable.setName( wxstr );
            }
            
            // value
            wxstr = child->GetAttribute( _("value"), _("") );
            variable.setValueFromString( variable.getType(),
                                            wxstr,
                                            CVSCPVariable::isValueBase64Encoded( variable.getType() ) );
            
            // note
            wxstr = child->GetAttribute( _("note"), _("") );
            variable.setNote( wxstr, true );
                    
            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName() == _("name")) {
                    wxString strName = subchild->GetNodeContent();
                    strName.Trim();
                    strName.Trim(false);
                    variable.setName( strName );
                }
                else if (subchild->GetName() == _("value")) {
                    variable.setValueFromString( variable.getType(),
                                                    subchild->GetNodeContent(),
                                                    CVSCPVariable::isValueBase64Encoded( variable.getType() ) );
                }
                else if (subchild->GetName() == _("value-base64")) {
                    variable.setValue( subchild->GetNodeContent() );
                    
                }
                else if (subchild->GetName() == _("note")) {
                    variable.setNote( subchild->GetNodeContent(), true );
                }
                else if (subchild->GetName() == _("note-base64")) {
                    
                    wxstr = subchild->GetNodeContent();
                    size_t len = wxBase64Decode( NULL, 0, wxstr );
                    if ( 0 == len ) return false;
                    uint8_t *pbuf = new uint8_t[len];
                    if ( NULL == pbuf ) return false;
                    len = wxBase64Decode( pbuf, len, wxstr );
                    wxstr = wxString::FromUTF8( (const char *)pbuf, len );
                    delete [] pbuf;
                    
                    variable.setNote( wxstr );
                }

                subchild = subchild->GetNext();

            }

            // Add the variable
            if ( exist( variable.getName() ) ) {
                
                if ( update( variable ) ) {
                    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_VARIABLE ) {                
                        gpobj->logMsg( wxString::Format( _("[Loading XML file] Updates variable %s.\n"),
                                            (const char *)variable.getName().mbc_str() ),
                                            DAEMON_LOGMSG_DEBUG ); 
                    }
                }
                else { 
                    gpobj->logMsg( wxString::Format( _("[Loading XML file] Failed to update variable %s.\n"),
                                            (const char *)variable.getName().mbc_str() ) );
                }
                
            }
            else {
                
                if ( add( variable ) ) {
                    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_VARIABLE ) {                
                        gpobj->logMsg( wxString::Format( _("[Loading XML file] Added variable %s.\n"),
                                            (const char *)variable.getName().mbc_str() ),
                                            DAEMON_LOGMSG_DEBUG ); 
                    }
                }
                else { 
                    gpobj->logMsg( wxString::Format( _("[Loading XML file] Failed to add variable %s.\n"),
                                            (const char *)variable.getName().mbc_str() ) );
                }
            
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
    
        if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_internal_variable,
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
            variable.setOwnerID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) ); 
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
    
        if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_external_variable,
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
            variable.setOwnerID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) );
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

    str.Printf(_(" user=\"%d\" "), variable.getOwnerID());
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

