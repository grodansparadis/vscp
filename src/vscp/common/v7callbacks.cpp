// v7.c
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
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
// wxJSON - http://wxcode.sourceforge.net/docs/wxjson/wxjson_tutorial.html
//

#ifdef WIN32
#include <winsock2.h>
#endif

#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/app.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/tokenzr.h>
#include <wx/stdpaths.h>
#include <wx/thread.h>
#include <wx/socket.h>
#include <wx/url.h>
#include <wx/datetime.h>
#include <wx/filename.h>
#include <wx/cmdline.h>

#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <v7.h>

#include <wx/jsonreader.h>
#include <wx/jsonval.h>

#include <vscp.h>
#include <vscpdb.h>
#include <version.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <actioncodes.h>
#include <vscpvariable.h>
#include <userlist.h>
#include <controlobject.h>
#include <vscpremotetcpif.h>
#include <v7callbacks.h>

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;


///////////////////////////////////////////////////////////////////////////////
// js_read_VSCP_Variable
//

enum v7_err js_read_VSCP_Variable( struct v7 *v7, v7_val_t *res ) 
{
    CVSCPVariable variable;
    wxString strResult;
    enum v7_err err;
    
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    v7_val_t valName = v7_arg(v7, 1);
    
    wxString wxName = v7_get_cstring( v7, &valName );
    if ( !gpobj->m_VSCP_Variables.find( wxName, variable ) ) {
        *res = v7_mk_null();
        return V7_OK;
    }
        
    // Get the variable on JSON format
    variable.getAsJSON( strResult );
        
    if ( V7_OK != ( err = v7_parse_json( v7, (const char *)strResult.mbc_str(), res ) ) ) {
        *res = v7_mk_null();
        return V7_OK;        
    }
        
    // Return result to JavaScript 
    //*res = v7_mk_string( v7, strResult.mbc_str(), strlen( strResult.mbc_str() ), 1 );        
    
    return V7_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_write_VSCP_Variable
//
//{  
//	"name": "variable-name",
//	"type": 1,                      
//	"user": 2,
//	"access-rights": 0x777,
//      "persistence": true|false,
//	"last-change": "YYYYMMDDTHHMMSS",
//	"value": "This is a test variable",
//	"note": "This is a note about this variable"
//}

enum v7_err js_write_VSCP_Variable( struct v7 *v7, v7_val_t *res ) 
{
    CVSCPVariable variable;
    bool bResult;
    wxJSONValue  root;      // JSON root object
    wxJSONReader reader;    // JSON parser
    
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    v7_val_t val1 = v7_arg(v7, 1);
    wxString strJSON = v7_get_cstring( v7, &val1 );
    
    int numErrors = reader.Parse( strJSON, &root );
    if ( numErrors > 0 )  {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return V7_OK;
    }
    
    // Make sure name is there
    if ( !root.HasMember( "name" ) ) {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return V7_OK;
    }
    
    // Get the name of the variable
    wxString wxName = root["name"].AsString();
    
    if ( gpobj->m_VSCP_Variables.find( wxName, variable ) ) {        
        
        // If value is present set value
        if ( root.HasMember( "value" ) ) {
            variable.setValueFromString( variable.getType(),
                                           root["value"].AsString(),
                                           false );
        }
        
        // If note is present set note
        if ( root.HasMember( "note" ) ) {
            variable.setNote( root["note"].AsString(), 
                                false );
        }
        
        // Save it
        if ( !gpobj->m_VSCP_Variables.add( variable ) ) {
            // Not good
            *res = v7_mk_boolean( v7, 0 );  // Return error
            return V7_OK;
        }
        
    }
    else {
        
        // Need to be added
        
        // Set the name
        variable.setName( wxName );
        
        // set user id (always admin user)
        variable.setUserID( 0 );
        
        // access rights
        if ( root.HasMember( "access-rights" ) ) {
            variable.setRighs( root["access-rights"].AsInt() );
        }
        else {
            variable.setPersistent( false );
        }
        
        // Persistence
        if ( root.HasMember( "persistence" ) ) {
            variable.setPersistent( root["persistence"].AsBool() );
        }
        else {
            variable.setPersistent( false );
        }
                
        // If value is present set value
        if ( root.HasMember( "value" ) ) {
            variable.setValueFromString( variable.getType(),
                                           root["value"].AsString(),
                                           false );
        }
        
        // If note is present set note
        if ( root.HasMember( "note" ) ) {
            variable.setNote( root["note"].AsString(), 
                                false );
        }
        
        // Save it
        if ( !gpobj->m_VSCP_Variables.add( variable ) ) {
            // Not good
            *res = v7_mk_boolean( v7, 0 );  // Return error
            return V7_OK;
        }
        
        // Add the variable
        if ( !gpobj->m_VSCP_Variables.add( variable ) ) {
            // Not good
            *res = v7_mk_boolean( v7, 0 );  // Return error
            return V7_OK;
        }
        
    }

    *res = v7_mk_boolean( v7, 1 );  // Return success
    return V7_OK;
}

///////////////////////////////////////////////////////////////////////////////
// js_send_VSCP_Event
//
// {
//    "time": "2017-01-13T10:16:02",
//    "head": 2,
//    "timestamp":50817,
//    "obid"; 123,
//    "class": 10,
//    "type": 8,
//    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02",
//    "data": [1,2,3,4,5,6,7],
//    "note": "This is some text"
// }

enum v7_err js_send_VSCP_Event( struct v7 *v7, v7_val_t *res ) 
{
    CVSCPVariable variable;
    bool bResult;
    wxJSONValue  root;      // JSON root object
    wxJSONReader reader;    // JSON parser
    
    // Get client item object
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    v7_val_t val1 = v7_arg( v7, 1 );
    wxString strJSON = v7_get_cstring( v7, &val1 );
    
    int numErrors = reader.Parse( strJSON, &root );
    if ( numErrors > 0 )  {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return V7_OK;
    }
    
    *res = v7_mk_boolean( v7, 1 );  // Return success
    return V7_OK;
}

///////////////////////////////////////////////////////////////////////////////
// js_get_VSCP_Event
//

enum v7_err js_get_VSCP_Event( struct v7 *v7, v7_val_t *res ) 
{
    CVSCPVariable variable;
    bool bResult;
    wxJSONValue  root;      // JSON root object
    wxJSONReader reader;    // JSON parser
    
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    v7_val_t val1 = v7_arg(v7, 1);
    wxString strJSON = v7_get_cstring( v7, &val1 );
    
    int numErrors = reader.Parse( strJSON, &root );
    if ( numErrors > 0 )  {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return V7_OK;
    }
    
    *res = v7_mk_boolean( v7, 1 );  // Return success
    return V7_OK;
}

///////////////////////////////////////////////////////////////////////////////
// js_count_VSCP_Event
//
//

enum v7_err js_count_VSCP_Event( struct v7 *v7, v7_val_t *res ) 
{
    double result;
    
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    *res = v7_mk_number( v7, result );  // Return success
    return V7_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_set_VSCP_Filter
//

enum v7_err js_set_VSCP_Filter( struct v7 *v7, v7_val_t *res ) 
{
    CVSCPVariable variable;
    bool bResult;
    wxJSONValue  root;      // JSON root object
    wxJSONReader reader;    // JSON parser
    
    v7_val_t val1 = v7_arg(v7, 0);
    wxString strJSON = v7_get_cstring( v7, &val1 );
    
    int numErrors = reader.Parse( strJSON, &root );
    if ( numErrors > 0 )  {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return V7_OK;
    }
    
    *res = v7_mk_boolean( v7, 1 );  // Return success
    return V7_OK;
}


// get measurement
// unit
// sensorindex
// zone
// subzone
// send measurement

