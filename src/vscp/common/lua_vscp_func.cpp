// duktape_vscp.c
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

#include <wx/jsonreader.h>
#include <wx/jsonval.h>

#include <vscp.h>
#include <vscpdb.h>
#include <version.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <actioncodes.h>
#include <remotevariable.h>
#include <userlist.h>
#include <controlobject.h>
#include <vscpremotetcpif.h>

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

#include <lua_vscp_func.h>


///////////////////////////////////////////////////
//                   GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

///////////////////////////////////////////////////
//                  HELPERS
///////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// lua_get_Event
//
// Make event from JSON data object on stack
//

int lua_get_Event( struct lua_State *L, vscpEventEx *pex )
{
    char buf[ 1024 ];
    int len = 1;
    
    lua_pushlstring( L, buf, len );
    
    /*struct web_connection *conn =
            (struct web_connection *) lua_touserdata(L, lua_upvalueindex(1));
    int num_args = lua_gettop(L);

    // This function may be called with one parameter (boolean) to set the
    // keep_alive state.
    // Or without a parameter to just query the current keep_alive state.
    if ((num_args == 1) && lua_isboolean(L, 1)) {
        conn->must_close = !lua_toboolean(L, 1);
    }
    else if (num_args != 0) {
        // Syntax error
        return luaL_error(L, "invalid keep_alive() call");
    }*/

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_vscp_print
//

int lua_vscp_print( struct lua_State *L )
{
    size_t len;
    const char *pstr;
    
    if ( !lua_isstring( L, 1 ) ) {
        return 0;
    }
    
    pstr = lua_tolstring ( L, 1, &len );
    wxPrintf( "%s\n", pstr );
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_vscp_log
//
// vscp.log("message"[,log-level,log-type])

int lua_vscp_log( struct lua_State *L ) 
{
    wxString wxMsg;
    uint8_t level = DAEMON_LOGMSG_NORMAL;
    uint8_t type = DAEMON_LOGTYPE_DM;
    
    int nArgs = lua_gettop( L );
    
    if ( 0 == nArgs) {
        return luaL_error( L, "vscp.log: Wrong number of arguments: "
                              "vscp.log(\"message\"[,log-level,log-type]) ");
    }    
    else if ( 1 == nArgs ) {        
        if ( !lua_isstring( L, 1 ) ) {
            return luaL_error( L, "vscp.log: Argument error, string expected: "
                                  "vscp.log(\"message\"[,log-level,log-type]) ");
        }
        size_t len;
        const char *pstr = lua_tolstring ( L, 1, &len ); 
        wxMsg = wxString::FromUTF8( pstr, len );
    }    
    else if ( 2 == nArgs ) {
        
        if ( !lua_isstring( L, 1 ) ) {
            return luaL_error( L, "vscp.log: Argument error, string expected: "
                                  "vscp.log(\"message\"[,log-level,log-type]) ");
        }
        
        size_t len;
        const char *pstr = lua_tolstring ( L, 1, &len ); 
        wxMsg = wxString::FromUTF8( pstr, len );
        
        if ( !lua_isnumber( L, 2 ) ) {
            return luaL_error( L, "vscp.log: Argument error, integer expected: "
                                  "vscp.log(\"message\"[,log-level,log-type]) ");
        }
        
        level = lua_tointeger( L, 2 );
                        
    }
    else if ( nArgs >= 3 ) {
        
        if ( !lua_isstring( L, 1 ) ) {
            return luaL_error( L, "vscp.log: Argument error, string expected: "
                                  "vscp.log(\"message\"[,log-level,log-type]) ");
        }
        
        size_t len;
        const char *pstr = lua_tolstring ( L, 1, &len ); 
        wxMsg = wxString::FromUTF8( pstr, len );
        
        if ( !lua_isnumber( L, 2 ) ) {
            return luaL_error( L, "vscp.log: Argument error, integer expected: "
                                  "vscp.log(\"message\"[,log-level,log-type]) ");
        }
        
        level = lua_tointeger( L, 2 );
        
        if ( !lua_isnumber( L, 3 ) ) {
            return luaL_error( L, "vscp.log: Argument error, integer expected: "
                                  "vscp.log(\"message\"[,log-level,log-type]) ");
        }
        
        type = lua_tointeger( L, 3 );
                        
    }
          
    gpobj->logMsg( wxMsg, level, type );
        
    lua_pushboolean( L, 1 );
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_vscp_sleep
//
// vscp.sleep( milliseconds )

int lua_vscp_sleep( struct lua_State *L )
{
    uint32_t sleep_ms;
    int nArgs = lua_gettop( L );
    
    if ( 0 == nArgs) {
        return luaL_error( L, "vscp.log: Wrong number of arguments: "
                              "vscp.sleep( milliseconds ) ");
    }
    else {
        if ( !lua_isnumber( L, 1 ) ) {
            return luaL_error( L, "vscp.sleep: Argument error, integer expected: "
                                  "vscp.sleep( milliseconds ) ");
        }
        
        sleep_ms = (uint32_t)lua_tonumber( L, 1 );
    }
    
    wxMilliSleep( sleep_ms );
    
    lua_pushboolean( L, 1 );
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_vscp_readVariable
//
// vscp.readvariable( "name", format )
//      format = 0 - string
//      format = 1 - XML
//      format = 2 - JSON

int lua_vscp_readVariable( struct lua_State *L )
{
    int format = 0;
    wxString varName;
    CVSCPVariable variable;
    wxString strResult;
    
    int nArgs = lua_gettop( L );
    
    if ( 0 == nArgs) {
        return luaL_error( L, "vscp.readvariable: Wrong number of arguments: "
                              "vscp.readvariable( \"name\"[,format] ) ");
    }
    else if ( 1 == nArgs ) {
        
        if ( !lua_isstring( L, 1 ) ) {
            return luaL_error( L, "vscp.readvariable: Argument error, string expected: "
                                  "vscp.readvariable( \"name\"[,format] ) ");
        }
        
        size_t len;
        const char *pstr = lua_tolstring ( L, 1, &len ); 
        varName = wxString::FromUTF8( pstr, len );
        
    }
    else {
        
        if ( !lua_isstring( L, 1 ) ) {
            return luaL_error( L, "vscp.readvariable: Argument error, string expected: "
                                  "vscp.readvariable( \"name\"[,format] ) ");
        }
        
        size_t len;
        const char *pstr = lua_tolstring ( L, 1, &len ); 
        varName = wxString::FromUTF8( pstr, len );
        
        if ( !lua_isnumber( L, 2 ) ) {
            return luaL_error( L, "vscp.readvariable: Argument error, number expected: "
                                  "vscp.readvariable( \"name\"[,format] ) ");
        }
        
        format = (int)lua_tointeger( L, 2 );
    }
    
    if ( !gpobj->m_variables.find( varName, variable ) ) {
        return luaL_error( L, "vscp.readvariable: No variable with that name found!");
    }
    
    // Get the variable on JSON format
    wxString varJSON;
    variable.getAsJSON( varJSON );
    
    lua_pushboolean( L, 1 );
    return 1;
}