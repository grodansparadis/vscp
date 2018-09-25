// vscp_Lua.cpp
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
#include <wx/base64.h>

#include <stdlib.h>
#include <string.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
#define LUA_LIB
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#include "httpd.h"
#include "httpd_lua.h"

#include <vscp.h>
#include <vscpdb.h>
#include <version.h>
#include <vscp_debug.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <actioncodes.h>
#include <remotevariable.h>
#include <userlist.h>
#include <controlobject.h>
#include <vscpremotetcpif.h>
#include <dm.h>
#include <lua_vscp_wrkthread.h>
#include <lua_vscp_func.h>

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

///////////////////////////////////////////////////
//                   KEYS
///////////////////////////////////////////////////
const char lua_vscp__regkey_clientitem = 100;


///////////////////////////////////////////////////////////////////////////////
// actionThread_Lua
//
// This thread executes a Lua 
//

actionThread_Lua::actionThread_Lua( wxString& strScript,
                                    wxThreadKind kind )
                                            : wxThread( kind )
{
    //OutputDebugString( "actionThreadURL: Create");
    m_wxstrScript = strScript;  // Script to execute
}

actionThread_Lua::~actionThread_Lua()
{

}


///////////////////////////////////////////////////////////////////////////////
// Entry
//
//

void *actionThread_Lua::Entry()
{
    struct lua_State *L;
    int lua_ret;
    const char *lua_err_txt;
    
    m_start = wxDateTime::Now();    // Mark start time
    
    
    // Create new Lua context
    L = luaL_newstate();
    
    // Check if OK
    if ( !L ) { 
        // Failure
        return NULL; 
    }

    //lua_pushlstring( L, const char *s, size_t len);
    
    luaL_openlibs( L );
    
    // Standard libs
        //      luaopen_base,
        //      luaopen_package,
        //      luaopen_coroutine,
        //      luaopen_table,
        //      luaopen_io,
        //      luaopen_os,
        //      luaopen_string,
        //      luaopen_math,
        //      luaopen_utf8,
        //      luaopen_debug,
    
    // lsqlite3 - Sqlite3
    // LuaXML_lib - XML support
    // lfs - File system support
    web_open_lua_libs( L );
        
    lua_newtable( L );
    
    web_reg_string( L, "lua_type", "dmscript" );
    web_reg_string( L, "version", VSCPD_DISPLAY_VERSION );
    
    wxString strFeedEvent;
    if ( vscp_writeVscpEventExToString( &m_feedEvent, strFeedEvent ) ) {
        web_reg_string( L, "feedevent_str", (const char *)strFeedEvent.mbc_str() );
    }
    else {
        wxString strError = 
                wxString::Format( _("Failed to convert Lua feed event to string." ) );
        gpobj->logMsg( strError, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
    }
        
    if ( vscp_convertEventExToJSON( &m_feedEvent, strFeedEvent ) ) {
        web_reg_string( L, "feedevent_json", (const char *)strFeedEvent.mbc_str() );
    }
    else {
        wxString strError = 
                wxString::Format( _("Failed to convert Lua feed event to JSON." ) );
        gpobj->logMsg( strError, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
    }
    
    if ( vscp_convertEventExToXML( &m_feedEvent, strFeedEvent ) ) {
        web_reg_string( L, "feedevent_xml", (const char *)strFeedEvent.mbc_str() );
    }
    else {
        wxString strError = 
                wxString::Format( _("Failed to conver Lua feed event to XML." ) );
        gpobj->logMsg( strError, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
    }
    
    // From httpd
    web_reg_function( L, "md5", web_lsp_md5 );
    web_reg_function( L, "get_time", web_lsp_get_time);
    web_reg_function( L, "random", web_lsp_random );
    web_reg_function( L, "uuid", web_lsp_uuid );
    
    web_reg_function( L, "print", lua_vscp_print );
    web_reg_function( L, "log", lua_vscp_log );
    web_reg_function( L, "sleep", lua_vscp_sleep );
    
    web_reg_function( L, "base64encode", lua_vscp_base64_encode );
    web_reg_function( L, "base64decode", lua_vscp_base64_decode );
    
    web_reg_function( L, "readvariable", lua_vscp_readVariable );
    web_reg_function( L, "writevariable", lua_vscp_writeVariable );
    web_reg_function( L, "writevariablevalue", lua_vscp_writeVariableValue );
    web_reg_function( L, "deletevariable", lua_vscp_deleteVariable );
    
    web_reg_function( L, "isVariableBase64Encoded", lua_vscp_isVariableBase64Encoded );
    web_reg_function( L, "isVariablePersistent", lua_vscp_isVariablePersistent );
    web_reg_function( L, "isVariableNumerical", lua_vscp_isVariableNumerical );
    web_reg_function( L, "isStockVariable", lua_vscp_isStockVariable );
            
    web_reg_function( L, "sendevent", lua_vscp_sendEvent );
    web_reg_function( L, "receiveevent", lua_vscp_getEvent );
    web_reg_function( L, "countevent", lua_vscp_getCountEvent );
    web_reg_function( L, "setfilter", lua_vscp_setFilter );
    
    web_reg_function( L, "ismeasurement", lua_is_Measurement );
    web_reg_function( L, "sendmeasurement", lua_send_Measurement );
    web_reg_function( L, "getmeasurementvalue", lua_get_MeasurementValue );
    web_reg_function( L, "getmeasurementunit", lua_get_MeasurementUnit );
    web_reg_function( L, "getmeasurementsensorindex", lua_get_MeasurementSensorIndex );
    web_reg_function( L, "getmeasurementzone", lua_get_MeasurementZone );
    web_reg_function( L, "getmeasurementsubzone", lua_get_MeasurementSubZone );      
    
/*    
    // Save the DM feed event for easy access
    wxString strEvent;
    vscp_convertEventExToJSON( &m_feedEvent, strEvent );
    duk_push_string( ctx, (const char *)strEvent.mbc_str() );
    duk_json_decode(ctx, -1);
    duk_put_global_string(ctx, "vscp_feedevent");
    
    // Save client object as a global pointer
    duk_push_pointer(ctx, (void *)m_pClientItem );
    duk_put_global_string(ctx, "vscp_controlobject");
    
    // Create VSCP client
    m_pClientItem = new CClientItem();
    vscp_clearVSCPFilter( &m_pClientItem->m_filterVSCP );
    
    // Save the client object as a global pointer
    duk_push_pointer(ctx, (void *)m_pClientItem );
    duk_put_global_string(ctx, "vscp_clientitem");
    
    // reading [global object].vscp_clientItem 
    duk_push_global_object(ctx);                // -> stack: [ global ] 
    duk_push_string(ctx, "vscp_clientitem");    // -> stack: [ global "vscp_clientItem" ] 
    duk_get_prop(ctx, -2);                      // -> stack: [ global vscp_clientItem ] 
    CClientItem *pItem = (CClientItem *)duk_get_pointer(ctx,-1);
    wxString user = pItem->m_UserName;
    
    duk_bool_t rc;

  */
    
    if ( pf_uuid_generate.f ) {
        web_reg_function(L, "uuid", web_lsp_uuid);
    }
    
    // Create VSCP client
    m_pClientItem = new CClientItem();
    if ( NULL != m_pClientItem ) return NULL;
    vscp_clearVSCPFilter( &m_pClientItem->m_filterVSCP );

    // This is an active client
    m_pClientItem->m_bOpen = false;
    m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_LUA;
    m_pClientItem->m_strDeviceName = _("Internal daemon Lua client.");
    m_pClientItem->m_strDeviceName += _("|Started at ");
    wxDateTime now = wxDateTime::Now();
    m_pClientItem->m_strDeviceName += now.FormatISODate();
    m_pClientItem->m_strDeviceName += _(" ");
    m_pClientItem->m_strDeviceName += now.FormatISOTime();

    // Add the client to the Client List
    gpobj->m_wxClientMutex.Lock();
    if ( !gpobj->addClient( m_pClientItem ) ) {
        // Failed to add client
        delete m_pClientItem;
        m_pClientItem = NULL;
        gpobj->m_wxClientMutex.Unlock();
        gpobj->logMsg( _("LUA worker: Failed to add client. Terminating thread.") );
        return NULL;
    }
    gpobj->m_wxClientMutex.Unlock();
    
    // Open the channel
    m_pClientItem->m_bOpen = true;
    
    // Register client item object
    //lua_pushlightuserdata( L, (void *)&lua_vscp__regkey_clientitem );
    lua_pushlstring( L, "vscp_clientitem", 15 );
    lua_pushlightuserdata( L, (void *)m_pClientItem );
    lua_settable( L, LUA_REGISTRYINDEX );
    
    lua_setglobal( L, "vscp" );
    
    // Execute the Lua
    //web_run_lua_string_script( (const char *)m_wxstrScript.mbc_str() );
    //luaL_dostring( lua, (const char *)m_wxstrScript.mbc_str() );
    /*if ( 0 != duk_peval( ctx ) ) {
        wxString strError = 
                wxString::Format( "Lua failed to execute: %s\n", 
                                    duk_safe_to_string(ctx, -1) );
        gpobj->logMsg( strError, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
    }*/
    
    
    web_open_lua_libs( L );

    lua_ret = luaL_loadstring( L, (const char *)m_wxstrScript.mbc_str() );

    if ( lua_ret != LUA_OK ) {

        wxString strError = 
                wxString::Format( _("Lua failed to load script from"
                                    " DM parameter. Script = %s\n"),
                                    (const char *)m_wxstrScript.mbc_str() );
        gpobj->logMsg( strError, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        
         return NULL;
    }

    // The script file is loaded, now call it
    lua_ret = lua_pcall( L,
                            0, // no arguments
                            1, // zero or one return value
                            0  // errors as string return value
                        );

    if ( lua_ret != LUA_OK ) {
        
        // Error when executing the script
        lua_err_txt = lua_tostring( L, -1 );
        
        wxString strError = 
                wxString::Format( _("Error running Lua script. "
                                    "Error = %s : Script = %s\n"),
                                    lua_err_txt,
                                    (const char *)m_wxstrScript.mbc_str() );
        gpobj->logMsg( strError, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        
        return NULL;
        
    }

    //	lua_close(L); must be done somewhere else
    
    
    // If the script wants to log results it can do so 
    // by itself with the log function
 
    //duk_pop(ctx);  // pop eval. result 
    
    // Close the channel
    m_pClientItem->m_bOpen = false;
    
    // Remove client and session item
    gpobj->m_wxClientMutex.Lock();
    gpobj->removeClient( m_pClientItem );
    m_pClientItem = NULL;
    gpobj->m_wxClientMutex.Unlock();

    // Destroy the Lua context
    //duk_destroy_heap( ctx );
    
    m_stop = wxDateTime::Now();     // Mark stop time
 
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void actionThread_Lua::OnExit()
{

}