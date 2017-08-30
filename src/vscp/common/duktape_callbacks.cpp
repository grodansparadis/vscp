// duktape_callbacks.c
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

//#include <v7.h>

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
#include <duktape.h>
#include <duk_module_node.h>
#include <duktape_callbacks.h>

///////////////////////////////////////////////////
//                   GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

///////////////////////////////////////////////////
//                  HELPERS
///////////////////////////////////////////////////


 bool get_js_Event( duk_context *ctx, int *pvarObjEvent, vscpEventEx *pex )
{
 /*   
    // Must be object
    if ( !v7_is_object( *pvarObjEvent ) ) {
        // Not good
        return false;
    }
    
    // Must be an event allocated
    if ( NULL == pex ) {
        // Not good
        return false;
    }
      
    // Head
    pex->head = VSCP_PRIORITY_NORMAL;
    v7_val_t v7_head = v7_get( v7, *pvarObjEvent, "head", 4 );
    if ( !v7_is_undefined( v7_head ) && v7_is_number( v7_head ) ) {
        pex->head = v7_get_int( v7, v7_head );
    }
    
    // Timestamp
    pex->timestamp = 0;
    v7_val_t v7_timestamp = v7_get( v7, *pvarObjEvent, "timestamp", 9 );
    if ( !v7_is_undefined( v7_timestamp ) && v7_is_number( v7_timestamp ) ) {
        pex->timestamp = v7_get_int( v7, v7_timestamp );
    }
    
    // obid
    pex->obid = 0;
    v7_val_t v7_obid = v7_get( v7, *pvarObjEvent, "obid", 4 );
    if ( !v7_is_undefined( v7_obid ) && v7_is_number( v7_obid ) ) {
        pex->obid = v7_get_int( v7, v7_obid );
    }
    
    // class
    pex->vscp_class = 0;
    v7_val_t v7_class = v7_get( v7, *pvarObjEvent, "class", 5 );
    if ( !v7_is_undefined( v7_class ) && v7_is_number( v7_class ) ) {
        pex->vscp_class = v7_get_int( v7, v7_class );
    }
    else {
        // Must be defined
        return false;;
    }
    
    // type
    pex->vscp_type = 0;
    v7_val_t v7_type = v7_get( v7, *pvarObjEvent, "type", 4 );
    if ( !v7_is_undefined( v7_type ) && v7_is_number( v7_type ) ) {
        pex->vscp_type = v7_get_int( v7, v7_type );
    }
    else {
        // Must be defined
        return false;;
    }
    
    //GUID
    memset( pex->GUID, 0, 16 ); 
    v7_val_t v7_guid = v7_get( v7, *pvarObjEvent, "guid", 4 );
    if ( !v7_is_undefined( v7_guid ) && !v7_is_number( v7_guid ) ) {
        wxString strGUID = v7_get_cstring( v7, &v7_guid );
        vscp_getGuidFromStringEx( pex, strGUID );
    }
    
    // Data
    pex->sizeData = 0;
    v7_val_t v7_dataArray = v7_get( v7, *pvarObjEvent, "data", 4 );
    if ( v7_is_undefined( v7_dataArray ) || !v7_is_array( v7, v7_dataArray ) ) {
        return false;
    }
    
    pex->sizeData = v7_array_length( v7, v7_dataArray );
    if ( pex->sizeData ) {
        for ( int i=0; i<pex->sizeData; i++ ) {
            v7_val_t v7_item = v7_array_get( v7, v7_dataArray, i );
            if ( !v7_is_undefined( v7_item ) && v7_is_number( v7_item ) && ( i < 512 ) ) {
                pex->data[ i ] = v7_get_int( v7, v7_item );
            }
        }
    }
 */   
    return true;
}



///////////////////////////////////////////////////
//                  CALLABLES
///////////////////////////////////////////////////


 ///////////////////////////////////////////////////////////////////////////////
// js_resolve_module
//
 
duk_ret_t js_resolve_module(duk_context *ctx) 
{
    const char *module_id;
    const char *parent_id;

    module_id = duk_require_string(ctx, 0);
    parent_id = duk_require_string(ctx, 1);

    duk_push_sprintf(ctx, "%s.js", module_id);
    printf("resolve_cb: id:'%s', parent-id:'%s', resolve-to:'%s'\n",
            module_id, parent_id, duk_get_string(ctx, -1) );

    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// js_load_module
//

duk_ret_t js_load_module(duk_context *ctx) 
{
    const char *filename;
    const char *module_id;
    
    // TODO Modules can only be loaded from /srv/vscp/javascript or
    // configured other location

    module_id = duk_require_string(ctx, 0);
    duk_get_prop_string(ctx, 2, "filename");
    filename = duk_require_string(ctx, -1);

    printf("load_cb: id:'%s', filename:'%s'\n", module_id, filename);

    if ( strcmp(module_id, "pig.js") == 0 ) {
	duk_push_sprintf(ctx, "module.exports = 'you\\'re about to get eaten by %s';",
                            module_id );
    } 
    else if (strcmp(module_id, "cow.js") == 0) {
	duk_push_string(ctx, "module.exports = require('pig');");
    } 
    else if (strcmp(module_id, "ape.js") == 0) {
	duk_push_string(ctx, "module.exports = { module: module, __filename: __filename, wasLoaded: module.loaded };");
    } 
    else if (strcmp(module_id, "badger.js") == 0) {
	duk_push_string(ctx, "exports.foo = 123; exports.bar = 234;");
    } 
    else if (strcmp(module_id, "comment.js") == 0) {
	duk_push_string(ctx, "exports.foo = 123; exports.bar = 234; // comment");
    } 
    else if (strcmp(module_id, "shebang.js") == 0) {
	duk_push_string(ctx, "#!ignored\nexports.foo = 123; exports.bar = 234;");
    } 
    else {
	//duk_error(ctx, DUK_ERR_TYPE_ERROR, "cannot find module: %s", module_id);
    }

    return 1;
}
 
///////////////////////////////////////////////////////////////////////////////
// js_vscp_print
//
duk_ret_t js_vscp_print( duk_context *ctx ) 
{
    duk_push_string(ctx, " ");
    duk_insert(ctx, 0);
    duk_join(ctx, duk_get_top(ctx) - 1);
    printf("%s\n", duk_safe_to_string(ctx, -1));
    return JAVASCRIPT_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_vscp_log
//
// vscp_log("message"[,log-level,log-type])

duk_ret_t js_vscp_log( duk_context *ctx ) 
{
    int nArgs = duk_get_top(ctx);
    if ( nArgs == 1 ) {
        duk_push_int(ctx, DAEMON_LOGMSG_NORMAL );
        duk_push_int(ctx, DAEMON_LOGTYPE_DM );
    }
    else if ( nArgs == 2 ) {
        duk_push_int(ctx, DAEMON_LOGMSG_NORMAL );
    }
    
    wxString wxDebug = duk_get_string_default(ctx, -3, "---Log fail---");
    uint8_t level = duk_get_int_default(ctx, -2, DAEMON_LOGMSG_NORMAL );
    uint8_t type = duk_get_int_default(ctx, -1, DAEMON_LOGTYPE_DM );
      
    gpobj->logMsg( wxDebug, level, type );
    
    duk_pop_n(ctx, 3); // Clear stack
    duk_push_boolean(ctx, 1);  // Return success
    return JAVASCRIPT_OK;
}

///////////////////////////////////////////////////////////////////////////////
// js_vscp_sleep
//
// vscp_sleep(Time-to-sleep-in-ms)
//

duk_ret_t js_vscp_sleep( duk_context *ctx ) 
{
    uint32_t sleep_ms = 1000;
    
    int nArgs = duk_get_top(ctx);
    if ( nArgs == 0 ) {
        duk_push_number(ctx, 1000 );
    }
    
    if ( nArgs ) { 
        sleep_ms = duk_to_uint32(ctx, -1);
    }
        
    wxMilliSleep( sleep_ms );
       
    duk_pop_n(ctx, 1); // Clear stack
    duk_push_boolean(ctx, 1);  // Return success
    return JAVASCRIPT_OK;
}

///////////////////////////////////////////////////////////////////////////////
// js_vscp_readVariable
//
// set var = vscp_readvariable("name");
//
//{  
//	"name": "variable-name",
//	"type": 1,                      
//	"user": 2,
//	"accessrights": 0x777,
//      "persistence": true|false,
//	"lastchange": "YYYYMMDDTHHMMSS",
//      "isnumerical": true|false
//      "isbase64": true|false
//	"value": "This is a test variable",
//	"note": "This is a note about this variable"
//}
//

 duk_ret_t js_vscp_readVariable( duk_context *ctx ) 
{
    CVSCPVariable variable;
    wxString strResult;
    
    // Get the variable name
    wxString varName = duk_get_string_default(ctx, -1, "");
    if ( 0 == varName.Length() ) {
        duk_pop_n(ctx, 1);  // Clear stack
        duk_push_null(ctx); // Return failure
        return JAVASCRIPT_OK;
    }
   
    // Get the clientitem pointer
    duk_push_global_object(ctx);                /* -> stack: [ global ] */
    duk_push_string(ctx, "vscp_clientitem");    /* -> stack: [ global "vscp_clientItem" ] */
    duk_get_prop(ctx, -2);                      /* -> stack: [ global vscp_clientItem ] */
    CClientItem *pItem = (CClientItem *)duk_get_pointer_default(ctx,-1,0);
    
    duk_pop_n(ctx, 3); // Clear stack
    
    if ( !gpobj->m_VSCP_Variables.find( varName, variable ) ) {
        duk_push_null(ctx);  // Return failure
        return JAVASCRIPT_OK;
    }
    
    // Get the variable on JSON format
    wxString varJSON;
    variable.getAsJSON( varJSON );
    duk_push_string(ctx, (const char *)varJSON.mbc_str() );
    duk_json_decode(ctx, -1);
    
    duk_get_prop_string(ctx, -1, "name");
    wxString str = wxString::Format("name=%s", duk_to_string(ctx, -1));
    duk_pop_n(ctx, 1); // Clear stack
       
    return JAVASCRIPT_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_vscp_writeVariable
//
// writeVariable("name",value)
// 

 duk_ret_t js_vscp_writeVariable( duk_context *ctx ) 
{    
    wxString varName;
    CVSCPVariable variable;
    duk_ret_t err;
    
    //  Should be a JSON variable object
    if ( !duk_is_object(ctx, -1) ) {
        duk_push_boolean(ctx,0);    // return code false
        return JAVASCRIPT_OK;
    }
    
    duk_push_string(ctx, "name");
    duk_get_prop(ctx, -2);
    varName = duk_get_string_default(ctx, -1, "");
    duk_pop_n(ctx, 1);
    if ( 0 == varName.Length() ) {
        duk_push_boolean(ctx,0);    // return code false
        return JAVASCRIPT_OK;
    }
        
    if ( !gpobj->m_VSCP_Variables.find( varName, variable ) ) {
    
        // Variable does not exist - should be created
        
        // Get type
        duk_push_string(ctx, "type");
        duk_get_prop(ctx, -2);
        
        uint16_t type;
        if ( duk_is_number(ctx, -1) ) {
            type = (uint16_t)duk_get_int_default(ctx, -1, VSCP_DAEMON_VARIABLE_CODE_STRING );
        }
        else if ( duk_is_string(ctx, -1) ) {
            wxString str = duk_get_string_default(ctx, -1, "string");
            duk_pop_n(ctx, 1);
            type = CVSCPVariable::getVariableTypeFromString( str );
        }
        else {
            duk_push_boolean(ctx,0);    // return code false
            return JAVASCRIPT_OK;
        }        
        duk_pop_n(ctx, 1);
        
        // Get the value
        wxString strValue;
        duk_push_string(ctx, "value");
        duk_get_prop(ctx, -2);
    
        // The value can be number, boolean, string
        if ( duk_is_number(ctx, -1) ) {
            double val = duk_get_number_default(ctx, -1, 0.0 );
            strValue = wxString::Format( "%lf", val );
        }
        else if ( duk_is_string(ctx, -1) ) {
            strValue = duk_get_string_default(ctx, -1, "" );
        }
        else if ( duk_is_boolean(ctx, -1) ) {
            bool bval = duk_get_boolean_default(ctx,-1, false );
            strValue = wxString::Format(_("%s"), bval ? "true" : "false");
        }
        else {
            duk_push_boolean(ctx,0);    // return code false
            return JAVASCRIPT_OK;
        }
        duk_pop_n(ctx, 1);
        
        // Get user
        uint32_t userid;
        duk_push_string(ctx, "user");
        duk_get_prop(ctx, -2);
        
        // The user can be given as a number or a  string
        if ( duk_is_number(ctx, -1) ) {
            userid = (uint32_t)duk_get_number_default(ctx, -1, 0.0 );
        }
        else if ( duk_is_string(ctx, -1) ) {
            wxString strUser( duk_get_string_default(ctx, -1, "" ) );
            CUserItem *pUser;
            pUser= gpobj->m_userList.getUser( strUser );
            if ( NULL == pUser ) {
                duk_push_boolean(ctx,0);    // return code false
                return JAVASCRIPT_OK;
            }
            userid = pUser->getUserID();
        }
        else {
            duk_push_boolean(ctx,0);    // return code false
            return JAVASCRIPT_OK;
        }
        
        duk_pop_n(ctx, 1);
        
        
        // Get rights (if there)
        uint32_t accessRights = PERMISSON_OWNER_ALL;
        duk_push_string(ctx, "accessrights");
        duk_get_prop(ctx, -2);
        if ( duk_is_number(ctx, -1) ) {
            accessRights = (uint32_t)duk_get_number_default(ctx, -1, PERMISSON_OWNER_ALL );
        }
        
        duk_pop_n(ctx, 1);
        
        
        // Get persistence (if there)
        bool bPersistense = false;
        duk_push_string(ctx, "persistence");
        duk_get_prop(ctx, -2);
        
        if ( duk_is_boolean(ctx, -1) ) {
            bPersistense = duk_get_boolean_default(ctx,-1, false );
        }
        
        duk_pop_n(ctx, 1);     
        
        // Get note (if there)
        wxString strNote;
        duk_push_string(ctx, "note");
        duk_get_prop(ctx, -2);
        
        if ( duk_is_string(ctx, -1) ) {
            strNote = duk_get_string_default(ctx, -1, "" );
        }
        
        duk_pop_n(ctx, 1);
        
        if( !gpobj->m_VSCP_Variables.add( varName, 
                                            strValue,
                                            type,
                                            userid,
                                            bPersistense,
                                            accessRights,
                                            strNote ) ) {
            
        }
        
    }
    else {
        
        duk_push_string(ctx, "value");
        duk_get_prop(ctx, -2);
    
        // The value can be number, boolean, string
        if ( duk_is_number(ctx, -1) ) {
            double val = duk_get_number_default(ctx, -1, 0.0 );
            wxString strval = wxString::Format( "%lf", val );
            variable.setValueFromString( variable.getType(), strval );
        }
        else if ( duk_is_string(ctx, -1) ) {
            wxString strval( duk_get_string_default(ctx, -1, "" ) );
            variable.setValueFromString( variable.getType(), strval );
        }
        else if ( duk_is_boolean(ctx, -1) ) {
            bool bval = duk_get_boolean_default(ctx,-1, false );
            variable.setValueFromString( variable.getType(), bval ? "true" : "false" );
        }
        else {
            duk_push_boolean(ctx,0);    // return code false
            return JAVASCRIPT_OK;
        }
        
        int nArgs = duk_get_top(ctx);               
        duk_pop_n(ctx, 2); // Clear stack        
        
                // Update variable storage
        if ( !gpobj->m_VSCP_Variables.update( variable ) ) {
            duk_push_null(ctx);  // Return failure
            return JAVASCRIPT_OK;
        }
        
        /*if ( !gpobj->m_VSCP_Variables.find( varName, variable ) ) {
            duk_push_null(ctx);  // Return failure
            return JAVASCRIPT_OK;
        }*/
    }
    
   
    // Get the clientitem pointer
    /*duk_push_global_object(ctx);                
    duk_push_string(ctx, "vscp_clientitem");    
    duk_get_prop(ctx, -2);                      
    CClientItem *pItem = (CClientItem *)duk_get_pointer_default(ctx,-1,0);
    
    duk_pop_n(ctx, 3); // Clear stack
   
            
    if ( !gpobj->m_VSCP_Variables.find( varName, variable ) ) {
        
        // Found - update
        
        // Set the variable value - if there
        duk_push_string(ctx, "value");
        
        duk_get_prop(ctx, -2);              // -> [ global json_test param3 ] 
        wxString str2 = wxString::Format("json_test.param3 is %s\n", duk_get_string(ctx, -1));
        
        
        v7_val_t v7_varValue = v7_get( v7, varObj, "value", 5 );
        if ( v7_is_boolean( v7_varValue ) ) {
            
            // Boolean form
            bool bValue = v7_get_bool( v7, v7_varValue );
            bValue ? variable.setTrue() : variable.setFalse();
            
        }
        else if ( v7_is_number( v7_varValue ) ) {
            
            // Numeric form
            if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType() ) {
                double value = v7_get_double( v7, v7_varValue );
                variable.setValue( value );
            }
            else {
                long value = v7_get_int( v7, v7_varValue );
                variable.setValue( value );
            }
            
        }
        else {
            
            // String form
            const char *pValue = v7_get_cstring( v7, &v7_varValue );
            if ( NULL != pValue ) {
                variable.setValue( pValue );
            }
            
        }
        
        // If note is present set note
        v7_val_t v7_varNote = v7_get( v7, varObj, "note", 4 );
        if ( !v7_is_undefined( v7_varNote ) ) {
            const char *pVarNote = v7_get_cstring( v7, &v7_varNote );
        }
        
        // Save it
        if ( !gpobj->m_VSCP_Variables.add( variable ) ) {
            // Not good
            *res = v7_mk_boolean( v7, 0 );  // Return error
            return 1;
        }*/
    
    /*
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    v7_val_t varObj = v7_arg(v7, 1);
    
    // Must be object
    if ( !v7_is_object( varObj ) ) {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return 1;
    }
    
    // Get the variable name
    v7_val_t v7_varName = v7_get( v7, varObj, "name", 4 );
    const char *pVarName = v7_get_cstring( v7, &v7_varName );
     
    if ( gpobj->m_VSCP_Variables.find( pVarName, variable ) ) {
       
        // Found - update
        
        // Set the variable value
        
        v7_val_t v7_varValue = v7_get( v7, varObj, "value", 5 );
        if ( v7_is_boolean( v7_varValue ) ) {
            
            // Boolean form
            bool bValue = v7_get_bool( v7, v7_varValue );
            bValue ? variable.setTrue() : variable.setFalse();
            
        }
        else if ( v7_is_number( v7_varValue ) ) {
            
            // Numeric form
            if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType() ) {
                double value = v7_get_double( v7, v7_varValue );
                variable.setValue( value );
            }
            else {
                long value = v7_get_int( v7, v7_varValue );
                variable.setValue( value );
            }
            
        }
        else {
            
            // String form
            const char *pValue = v7_get_cstring( v7, &v7_varValue );
            if ( NULL != pValue ) {
                variable.setValue( pValue );
            }
            
        }
        
        // If note is present set note
        v7_val_t v7_varNote = v7_get( v7, varObj, "note", 4 );
        if ( !v7_is_undefined( v7_varNote ) ) {
            const char *pVarNote = v7_get_cstring( v7, &v7_varNote );
        }
        
        // Save it
        if ( !gpobj->m_VSCP_Variables.add( variable ) ) {
            // Not good
            *res = v7_mk_boolean( v7, 0 );  // Return error
            return 1;
        }
        
    }
    else {
        
        // Not found - create a new variable
        
        variable.setName( pVarName );                           // Set name
        
        variable.setType( VSCP_DAEMON_VARIABLE_CODE_STRING );   // Default type
        
        v7_val_t v7_varType = v7_get( v7, varObj, "type", 4 );
        if ( !v7_is_undefined( v7_varType ) ) {
            int type = v7_get_int( v7, v7_varType );
            variable.setType( type );
        }
                
        // Persistence
        variable.setPersistent( false );    // Default is non-persistent
        v7_val_t v7_varPersitence = v7_get( v7, varObj, "persistence", 11 );
        if ( !v7_is_undefined( v7_varPersitence ) ) {
            variable.setPersistent( v7_get_bool( v7, v7_varPersitence ) ? true : false );
        }
        
        // User
        variable.setUserID( USER_ID_ADMIN );    // Default is Admin user
        v7_val_t v7_varUser = v7_get( v7, varObj, "user", 4 );
        if ( !v7_is_undefined( v7_varUser ) && v7_is_number( v7_varUser ) ) {
            variable.setUserID( v7_get_int( v7, v7_varUser ) );
        }
        
        // Access-Rights
        variable.setAccessRights( PERMISSON_OWNER_ALL );    // Owner have all rights
        v7_val_t v7_varAccessRights = v7_get( v7, varObj, "accessrights", 12 );
        if ( !v7_is_undefined( v7_varAccessRights ) && v7_is_number( v7_varAccessRights ) ) {
            variable.setUserID( v7_get_int( v7, v7_varAccessRights ) );
        }
        
        // Set the variable value
        
        v7_val_t v7_varValue = v7_get( v7, varObj, "value", 5 );
        if ( v7_is_boolean( v7_varValue ) ) {
            
            // Boolean form
            bool bValue = v7_get_bool( v7, v7_varValue );
            bValue ? variable.setTrue() : variable.setFalse();
            
        }
        else if ( v7_is_number( v7_varValue ) ) {
            
            // Numeric form
            if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType() ) {
                double value = v7_get_double( v7, v7_varValue );
                variable.setValue( value );
            }
            else {
                long value = v7_get_int( v7, v7_varValue );
                variable.setValue( value );
            }
            
        }
        else {
            
            // String form
            const char *pValue = v7_get_cstring( v7, &v7_varValue );
            if ( NULL != pValue ) {
                variable.setValue( pValue );
            }
            
        }
        
        // If note is present set note
        v7_val_t v7_varNote = v7_get( v7, varObj, "note", 4 );
        if ( !v7_is_undefined( v7_varNote ) ) {
            const char *pVarNote = v7_get_cstring( v7, &v7_varNote );
        }
        
        // Save it
        if ( !gpobj->m_VSCP_Variables.add( variable ) ) {
            // Not good
            *res = v7_mk_boolean( v7, 0 );  // Return error
            return 1;
        }
        
    }

    *res = v7_mk_boolean( v7, 1 );  // Return success
    */
    return JAVASCRIPT_OK;
}

///////////////////////////////////////////////////////////////////////////////
// js_vscp_deleteVariable
//

 duk_ret_t js_vscp_deleteVariable( duk_context *ctx ) 
{
    CVSCPVariable variable;
    bool bResult;
    /*
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    v7_val_t v7_varName = v7_arg(v7, 1);
    
    // Must be string
    if ( !v7_is_string( v7_varName ) ) {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return 1;
    }
    
    // Get the variable name
    const char *pVarName = v7_get_cstring( v7, &v7_varName );
     
    wxString strName = pVarName;
    if ( gpobj->m_VSCP_Variables.remove( strName ) ) {
        *res = v7_mk_boolean( v7, 1 );  // Return success
    }
    else {
        *res = v7_mk_boolean( v7, 0 );  // Return success
    }
    */
    return JAVASCRIPT_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_vscp_sendEvent
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

duk_ret_t js_vscp_sendEvent( duk_context *ctx ) 
{
    vscpEventEx ex;
    /*
    // Get client item object
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    // Get event
    v7_val_t varObjEvent = v7_arg(v7, 1);
    
    // Must be object
    if ( !v7_is_object( varObjEvent ) ) {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return 1;
    }
      
    if ( !get_js_Event( v7, &varObjEvent, &ex ) ) {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  
        return 1;
    }
    
    // Send the event
    vscpEvent *pEvent = new vscpEvent;
    if ( NULL == pEvent ) {
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return 1;
    }
    
    vscp_convertVSCPfromEx( pEvent, &ex );
    
    if ( !gpobj->sendEvent( pClientItem, pEvent ) ) {
        // Failed to send event
        vscp_deleteVSCPevent( pEvent );
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return 1;
    }
    
    vscp_deleteVSCPevent( pEvent );
    
    
    *res = v7_mk_boolean( v7, 1 );  // Return success
    */
    return JAVASCRIPT_OK;
}

///////////////////////////////////////////////////////////////////////////////
// js_vscp_getEvent
//

 duk_ret_t js_vscp_getEvent( duk_context *ctx ) 
{    
    /*
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
try_again:
    
    // Check the client queue
    if ( pClientItem->m_bOpen && pClientItem->m_clientInputQueue.GetCount() ) {

        CLIENTEVENTLIST::compatibility_iterator nodeClient;
        vscpEvent *pEvent;

        pClientItem->m_mutexClientInputQueue.Lock();
        nodeClient = pClientItem->m_clientInputQueue.GetFirst();
        if ( NULL == nodeClient )  {
            
            // Exception
            *res = v7_mk_null();
            return 1;        
            
        }
        
        pEvent = nodeClient->GetData();                             
        pClientItem->m_clientInputQueue.DeleteNode( nodeClient );
        pClientItem->m_mutexClientInputQueue.Unlock();

        if ( NULL != pEvent ) {

            if ( vscp_doLevel2Filter( pEvent, &pClientItem->m_filterVSCP ) ) {

                // Write it out
                
                wxString strResult;
                vscp_convertEventToJSON( pEvent, strResult );
                
                if ( V7_OK != v7_parse_json( v7, (const char *)strResult.mbc_str(), res ) ) {
                    *res = v7_mk_null();
                    return 1;        
                }       
                
                // All OK return event
                return 1;
   
            }
            else {
                                
                // Filtered out
                vscp_deleteVSCPevent( pEvent );
                goto try_again;
                
            }
            
            // Remove the event
            vscp_deleteVSCPevent( pEvent );

        } // Valid pEvent pointer
        else {
            // NULL event
            *res = v7_mk_null();
            return 1;
        }
                
    } // events available
    
    // Fail
    *res = v7_mk_null();
    */
    return JAVASCRIPT_OK;    
}

///////////////////////////////////////////////////////////////////////////////
// js_vscp_getCountEvent
//
//

 duk_ret_t js_vscp_getCountEvent( duk_context *ctx ) 
{
    int count = 0;
    /*
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    if ( pClientItem->m_bOpen ) {
        count = pClientItem->m_clientInputQueue.GetCount();
    }
    else {
        count = 0;
    }
    
    *res = v7_mk_number( v7, count );  // Return success
    */
    return JAVASCRIPT_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_vscp_setFilter
//
// {
//     'mask_priorit': number,
//     'mask_class': number,
//     'mask_type': number,
//     'mask_guid': 'string',
//     'filter_priority'; number,
//     'filter_class': number,
//     'filter_type': number,
//     'filter_guid' 'string'
// }

 duk_ret_t js_vscp_setFilter( duk_context *ctx ) 
{
    vscpEventFilter filter;
    /*
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    // Get filter
    v7_val_t varObjFilter = v7_arg(v7, 1);
    
    // Must be an object
    if ( !v7_is_object( varObjFilter ) ) {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return 1;
    }
    
    // Mask priority
    filter.mask_priority = 0;   // Default is don't care
    v7_val_t v7_mask_priority = v7_get( v7, varObjFilter, "mask_priority", 13 );
    if ( !v7_is_undefined( v7_mask_priority ) && v7_is_number( v7_mask_priority ) ) {
        filter.mask_priority = v7_get_int( v7, v7_mask_priority );
    }
    
    // Mask class
    filter.mask_class = 0;   // Default is don't care
    v7_val_t v7_mask_class = v7_get( v7, varObjFilter, "mask_class", 10 );
    if ( !v7_is_undefined( v7_mask_class ) && v7_is_number( v7_mask_class ) ) {
        filter.mask_class = v7_get_int( v7, v7_mask_class );
    }
    
    // Mask type
    filter.mask_type = 0;   // Default is don't care
    v7_val_t v7_mask_type = v7_get( v7, varObjFilter, "mask_type", 9 );
    if ( !v7_is_undefined( v7_mask_type ) && v7_is_number( v7_mask_type ) ) {
        filter.mask_type = v7_get_int( v7, v7_mask_type );
    }
    
    // mask GUID
    memset( filter.mask_GUID, 0, 16 ); // Default is don't care
    v7_val_t v7_mask_guid = v7_get( v7, varObjFilter, "mask_guid", 9 );
    if ( !v7_is_undefined( v7_mask_guid ) && !v7_is_number( v7_mask_guid ) ) {
        wxString strGUID = v7_get_cstring( v7, &v7_mask_guid );
        vscp_getGuidFromStringToArray( filter.mask_GUID, strGUID );
    }
    
    // Filter priority
    filter.mask_priority = 0;
    v7_val_t v7_filter_priority = v7_get( v7, varObjFilter, "filter_priority", 15 );
    if ( !v7_is_undefined( v7_filter_priority ) && v7_is_number( v7_filter_priority ) ) {
        filter.mask_priority = v7_get_int( v7, v7_filter_priority );
    }
    
    // Filter class
    filter.filter_class = 0;   // Default is don't care
    v7_val_t v7_filter_class = v7_get( v7, varObjFilter, "filter_class", 12 );
    if ( !v7_is_undefined( v7_filter_class ) && v7_is_number( v7_filter_class ) ) {
        filter.filter_class = v7_get_int( v7, v7_filter_class );
    }
    
    // Filter type
    filter.filter_type = 0;   // Default is don't care
    v7_val_t v7_filter_type = v7_get( v7, varObjFilter, "filter_type", 11 );
    if ( !v7_is_undefined( v7_filter_type ) && v7_is_number( v7_filter_type ) ) {
        filter.filter_type = v7_get_int( v7, v7_filter_type );
    }
    
    // filter GUID
    memset( filter.filter_GUID, 0, 16 ); // Default is don't care
    v7_val_t v7_filter_guid = v7_get( v7, varObjFilter, "filter_guid", 11 );
    if ( !v7_is_undefined( v7_filter_guid ) && !v7_is_number( v7_filter_guid ) ) {
        wxString strGUID = v7_get_cstring( v7, &v7_filter_guid );
        vscp_getGuidFromStringToArray( filter.filter_GUID, strGUID );
    }
    
    // Set the filter
    vscp_copyVSCPFilter( &pClientItem->m_filterVSCP, &filter);        
    
    *res = v7_mk_boolean( v7, 1 );  // Return success
    */
    return JAVASCRIPT_OK;
}





///////////////////////////////////////////////////////////////////////////////
// js_send_Measurement
//
// {
//     'level': 1|2, (defaults to 2)
//     'string': true|false,  (default false, only valid for level II event)
//     'value': 123.5,
//     'vscptype': 6,
//     'unit': 1, (defaults to 0) 
//     'sensorindex': 0, (defaults to 0)
//     'zone': 0, (defaults to 0)
//     'subzone': 0 (defaults to 0)
// }
//

 duk_ret_t js_send_Measurement( duk_context *ctx )
{
    vscpEvent *pEvent;
    double value;           // Measurement value
    bool bLevel2 = true;    // True if level II
    bool bString = false;   // If level II string or float
    int type;               // VSCP type
    int unit = 0;
    int sensoridx = 0;
    int zone = 0;
    int subzone = 0;
    
    /*
    v7_val_t valClientItem = v7_arg(v7, 0);
    CClientItem *pClientItem = (CClientItem *)v7_get_ptr( v7, valClientItem );
    
    // Get measurement JSON object
    v7_val_t varObjMeasurement = v7_arg(v7, 1);
    
    // Must be an object
    if ( !v7_is_object( varObjMeasurement ) ) {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return 1;
    }
    
    // Value
    v7_val_t v7_value = v7_get( v7, varObjMeasurement, "value", 5 );
    if ( !v7_is_undefined( v7_value ) && v7_is_number( v7_value ) ) {
        value = v7_get_double( v7, v7_value ); 
    }
    else {
        // Must be defined
        *res = v7_mk_boolean( v7, 0 );  // Failed
        return 1;
    }
    
    // GUID
    uint8_t guid[16];
    memset( guid, 0, 16 ); // Default is don't care
    v7_val_t v7_guid = v7_get( v7, varObjMeasurement, "guid", 4 );
    if ( !v7_is_undefined( v7_guid ) && !v7_is_number( v7_guid ) ) {
        wxString strGUID = v7_get_cstring( v7, &v7_guid );
        vscp_getGuidFromStringToArray( guid, strGUID );
    }
    
    // VSCP type
    v7_val_t v7_type = v7_get( v7, varObjMeasurement, "vscptype", 8 );
    if ( !v7_is_undefined( v7_type ) && v7_is_number( v7_type ) ) {
        type = v7_get_double( v7, v7_type ); 
    }
    else {
        // Must be defined
        *res = v7_mk_boolean( v7, 0 );  // Failed
        return 1;
    }
    
    // Level
    v7_val_t v7_level = v7_get( v7, varObjMeasurement, "level", 5 );
    if ( !v7_is_undefined( v7_level ) && v7_is_number( v7_level ) ) {
        if ( 2 != v7_get_int( v7, v7_level ) ) {
            bLevel2 = false;
        }
    }
    
    // String
    v7_val_t v7_string = v7_get( v7, varObjMeasurement, "string", 6 );
    if ( !v7_is_undefined( v7_string ) && v7_is_boolean( v7_string ) ) {
        if ( v7_get_bool( v7, v7_string ) ) {
            bString = true;
        }
    }
        
    // unit
    v7_val_t v7_unit = v7_get( v7, varObjMeasurement, "unit", 4 );
    if ( !v7_is_undefined( v7_unit ) && v7_is_number( v7_unit ) ) {
        unit = v7_get_int( v7, v7_unit ); 
    }
    
    // zone
    v7_val_t v7_zone = v7_get( v7, varObjMeasurement, "zone", 4 );
    if ( !v7_is_undefined( v7_zone ) && v7_is_number( v7_zone ) ) {
        zone = v7_get_int( v7, v7_zone ); 
    }
    
    // subzone
    v7_val_t v7_subzone = v7_get( v7, varObjMeasurement, "subzone", 7 );
    if ( !v7_is_undefined( v7_subzone ) && v7_is_number( v7_subzone ) ) {
        subzone = v7_get_int( v7, v7_subzone ); 
    }
    
    if ( bLevel2 ) {
        
        if ( bString ) {
            
            pEvent = new vscpEvent;
            if ( NULL == pEvent ) {
                *res = v7_mk_boolean( v7, 0 );  // Failed
                return 1;
            }
            pEvent->pdata = NULL;
        
            // Set GUID
            memcpy( pEvent->GUID, guid, 16 );
            
            if ( !vscp_makeLevel2StringMeasurementEvent( pEvent, 
                                                            type,
                                                            value,
                                                            unit,
                                                            sensoridx,
                                                            zone,
                                                            subzone ) ) {
                // Failed
                *res = v7_mk_boolean( v7, 0 );  // Failed
                return 1;
            }
            
        }
        else {
            
            pEvent = new vscpEvent;
            if ( NULL == pEvent ) {
                *res = v7_mk_boolean( v7, 0 );  // Failed
                return 1;
            }
            pEvent->pdata = NULL;
        
            // Set GUID
            memcpy( pEvent->GUID, guid, 16 );
                
            if ( !vscp_makeLevel2FloatMeasurementEvent( pEvent, 
                                                            type,
                                                            value,
                                                            unit,
                                                            sensoridx,
                                                            zone,
                                                            subzone ) ) {
                // Failed
                *res = v7_mk_boolean( v7, 0 );  // Failed
                return 1;
            }
            
        }
        
    }
    else {
        
        // Level I
        
        if ( bString ) { 
            
            pEvent = new vscpEvent;
            if ( NULL == pEvent ) {
                *res = v7_mk_boolean( v7, 0 );  // Failed
                return 1;
            }
                
            memcpy( pEvent->GUID, guid, 16 );
            pEvent->vscp_type = type;
            pEvent->vscp_class = VSCP_CLASS1_MEASUREMENT;
            pEvent->obid = 0;
            pEvent->timestamp = 0;                    
            pEvent->pdata = NULL;
                
            if ( !vscp_makeStringMeasurementEvent( pEvent, 
                                                        value,
                                                        unit,
                                                        sensoridx ) ) {
                vscp_deleteVSCPevent( pEvent );
                *res = v7_mk_boolean( v7, 0 );  // Failed
                return 1;
            }
            
            //
            
        }
        else {
            
            pEvent = new vscpEvent;
            if ( NULL == pEvent ) {
                *res = v7_mk_boolean( v7, 0 );  // Failed
                return 1;
            }

            memcpy( pEvent->GUID, guid, 16 );
            pEvent->vscp_type = type;
            pEvent->vscp_class = VSCP_CLASS1_MEASUREMENT;
            pEvent->obid = 0;
            pEvent->timestamp = 0; 
            pEvent->pdata = NULL;

            if ( !vscp_makeFloatMeasurementEvent( pEvent, 
                                                    value,
                                                    unit,
                                                    sensoridx ) ) {
                vscp_deleteVSCPevent( pEvent );
                *res = v7_mk_boolean( v7, 0 );  // Failed
                return 1;
            }
        
        }
        
    }
    
    // Send the event
    if ( !gpobj->sendEvent( pClientItem, pEvent ) ) {
        // Failed to send event
        vscp_deleteVSCPevent( pEvent );
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return 1;
    }
    
    vscp_deleteVSCPevent( pEvent );
   
    *res = v7_mk_boolean( v7, 1 );  // Success
    */
    return JAVASCRIPT_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_is_Measurement
//

 duk_ret_t js_is_Measurement( duk_context *ctx ) 
{
    vscpEventEx ex;
    /*
    // Get event
    v7_val_t varObjEvent = v7_arg(v7, 0);
    
    if ( !get_js_Event( v7, &varObjEvent, &ex ) ) {
        // Not good
        *res = v7_mk_boolean( v7, 0 );  
        return  1;
    }
    
    vscpEvent *pEvent = new vscpEvent;
    if ( NULL == pEvent ) {
        *res = v7_mk_boolean( v7, 0 );  // Return error
        return 1;
    }
    
    pEvent->pdata = NULL;
    
    vscp_convertVSCPfromEx( pEvent, &ex );
    bool bMeasurement = vscp_isVSCPMeasurement( pEvent );
    vscp_deleteVSCPevent( pEvent );
    
    *res = v7_mk_boolean( v7, bMeasurement ? 1 : 0 );  // Return result
    */
    return JAVASCRIPT_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_get_MeasurementValue
//

 duk_ret_t js_get_MeasurementValue( duk_context *ctx ) 
{
    double value;
    vscpEventEx ex;
    /*
    // Get measurement JSON object
    v7_val_t varObjEvent = v7_arg(v7, 0);
    
    // Must be an object
    if ( !v7_is_object( varObjEvent ) ) {
        // Not good
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    if ( !get_js_Event( v7, &varObjEvent, &ex ) ) {
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    vscpEvent *pEvent = new vscpEvent;
    if ( NULL == pEvent ) {
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    pEvent->pdata = NULL;
    
    vscp_convertVSCPfromEx( pEvent, &ex );
    vscp_getVSCPMeasurementAsDouble( pEvent, &value );
    vscp_deleteVSCPevent( pEvent );
           
    *res = v7_mk_number( v7, value );  // Success
    */
    return JAVASCRIPT_OK;
}

///////////////////////////////////////////////////////////////////////////////
// js_get_MeasurementUnit
//

 duk_ret_t js_get_MeasurementUnit( duk_context *ctx ) 
{
    vscpEventEx ex;
    /*
    // Get measurement JSON object
    v7_val_t varObjEvent = v7_arg(v7, 0);
    
    // Must be an object
    if ( !v7_is_object( varObjEvent ) ) {
        // Not good
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    if ( !get_js_Event( v7, &varObjEvent, &ex ) ) {
        *res = v7_mk_null();  // Return error
        return 1;
    }
           
    vscpEvent *pEvent = new vscpEvent;
    if ( NULL == pEvent ) {
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    pEvent->pdata = NULL;
    
    vscp_convertVSCPfromEx( pEvent, &ex );
    int unit = vscp_getVSCPMeasurementUnit( pEvent );
    vscp_deleteVSCPevent( pEvent );
           
    *res = v7_mk_number( v7, unit );  // Success
    */
    return JAVASCRIPT_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_get_MeasurementSensorIndex
//

 duk_ret_t js_get_MeasurementSensorIndex( duk_context *ctx ) 
{
    vscpEventEx ex;
    /*
    // Get measurement JSON object
    v7_val_t varObjEvent = v7_arg(v7, 0);
    
    // Must be an object
    if ( !v7_is_object( varObjEvent ) ) {
        // Not good
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    if ( !get_js_Event( v7, &varObjEvent, &ex ) ) {
        *res = v7_mk_null();  // Return error
        return 1;
    }
           
    vscpEvent *pEvent = new vscpEvent;
    if ( NULL == pEvent ) {
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    pEvent->pdata = NULL;
    
    vscp_convertVSCPfromEx( pEvent, &ex );
    int sensorindex = vscp_getVSCPMeasurementSensorIndex( pEvent );
    vscp_deleteVSCPevent( pEvent );
           
    *res = v7_mk_number( v7, sensorindex );  // Success
    */
    return JAVASCRIPT_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_get_MeasurementZone
//

 duk_ret_t js_get_MeasurementZone( duk_context *ctx ) 
{
    vscpEventEx ex;
    /*
    // Get measurement JSON object
    v7_val_t varObjEvent = v7_arg(v7, 0);
    
    // Must be an object
    if ( !v7_is_object( varObjEvent ) ) {
        // Not good
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    if ( !get_js_Event( v7, &varObjEvent, &ex ) ) {
        *res = v7_mk_null();  // Return error
        return 1;
    }
           
    vscpEvent *pEvent = new vscpEvent;
    if ( NULL == pEvent ) {
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    pEvent->pdata = NULL;
    
    vscp_convertVSCPfromEx( pEvent, &ex );
    int zone = vscp_getVSCPMeasurementZone( pEvent );
    vscp_deleteVSCPevent( pEvent );
           
    *res = v7_mk_number( v7, zone );  // Success
    */
    return JAVASCRIPT_OK;
}


///////////////////////////////////////////////////////////////////////////////
// js_get_MeasurementSubZone
//

 duk_ret_t js_get_MeasurementSubZone( duk_context *ctx ) 
{
    vscpEventEx ex;
    /*
    // Get measurement JSON object
    v7_val_t varObjEvent = v7_arg(v7, 0);
    
    // Must be an object
    if ( !v7_is_object( varObjEvent ) ) {
        // Not good
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    if ( !get_js_Event( v7, &varObjEvent, &ex ) ) {
        *res = v7_mk_null();  // Return error
        return 1;
    }
           
    vscpEvent *pEvent = new vscpEvent;
    if ( NULL == pEvent ) {
        *res = v7_mk_null();  // Return error
        return 1;
    }
    
    pEvent->pdata = NULL;
    
    vscp_convertVSCPfromEx( pEvent, &ex );
    int subzone = vscp_getVSCPMeasurementSubZone( pEvent );
    vscp_deleteVSCPevent( pEvent );
           
    *res = v7_mk_number( v7, subzone );  // Success
    */
    return JAVASCRIPT_OK;
}


