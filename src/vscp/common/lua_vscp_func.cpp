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

#include <json.hpp>             // Needs C++11  -std=c++11

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



using namespace std;

// https://github.com/nlohmann/json
using json = nlohmann::json;



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
// vscp.readvariable( "name"[, format ] )
//
//      format = 0 - string
//      format = 1 - XML
//      format = 2 - JSON
//      format = 3 - Just value (can be base64 encoded)
//      format = 4 - Just note (always base64 encoded)

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
        
        // variable name
        if ( !lua_isstring( L, 1 ) ) {
            return luaL_error( L, "vscp.readvariable: Argument error, string expected: "
                                  "vscp.readvariable( \"name\"[,format] ) ");
        }
        
        size_t len;
        const char *pstr = lua_tolstring ( L, 1, &len ); 
        varName = wxString::FromUTF8( pstr, len );        
        
    }
    else {
        
        // variable name
        if ( !lua_isstring( L, 1 ) ) {
            return luaL_error( L, "vscp.readvariable: Argument error, string expected: "
                                  "vscp.readvariable( \"name\"[,format] ) ");
        }
        
        size_t len;
        const char *pstr = lua_tolstring ( L, 1, &len ); 
        varName = wxString::FromUTF8( pstr, len );
        
        // format
        if ( !lua_isnumber( L, 2 ) ) {
            return luaL_error( L, "vscp.readvariable: Argument error, number expected: "
                                  "vscp.readvariable( \"name\"[,format] ) ");
        }
        
        format = (int)lua_tointeger( L, 2 );
    }
    
    if ( !gpobj->m_variables.find( varName, variable ) ) {
        return luaL_error( L, "vscp.readvariable: No variable with that "
                              "name found!");
    }
    
    // Get the variable in string format
    if ( 0 == format ) {
        // Get the variable in string format
        wxString varStr;
        varStr = variable.getAsString( false );
        lua_pushlstring( L, (const char *)varStr.mbc_str(),
                            varStr.Length() );
    }
    // Get variable in XML format
    else if ( 1 == format ) {
        // Get the variable in XML format
        wxString varXML;
        variable.getAsXML( varXML );
        lua_pushlstring( L, (const char *)varXML.mbc_str(),
                            varXML.Length() );
    } 
    // Get variable in JSON format
    else if ( 2 == format ) {
        // Get the variable on JSON format
        wxString varJSON;
        variable.getAsJSON( varJSON );
        lua_pushlstring( L, (const char *)varJSON.mbc_str(),
                            varJSON.Length() );
    }
    // Get only value
    else if ( 3 == format ) {
        // Get the variable value un encoded
        wxString strVal;
        strVal = variable.getValue();
        lua_pushlstring( L, (const char *)strVal.mbc_str(),
                            strVal.Length() );
    }
    // Get only note
    else if ( 4 == format ) {
        // Get the variable value un encoded
        wxString strVal;
        strVal = variable.getValue();
        lua_pushlstring( L, (const char *)strVal.mbc_str(),
                            strVal.Length() );
    }
    else {
        return luaL_error( L, "vscp.readvariable: Format must be 0=string, "
                              "1=XML, 2=JSON, 3=value, 4=note");
    }
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// js_vscp_writeVariable
//
// writeVariable( value[, format])
// 
//      format = 0 - string
//      format = 1 - XML
//      format = 2 - JSON
//

int lua_vscp_writeVariable( struct lua_State *L ) 
{    
    size_t len;
    int format = 0;
    wxString varValue;
    CVSCPVariable variable;
 
    int nArgs = lua_gettop( L );
    
    if ( 0 == nArgs ) {
        return luaL_error( L, "vscp.writeVariable: Wrong number of arguments: "
                              "vscp.writeVariable( varname[,format] ) ");
    }
    
    if ( nArgs >= 1 ) {
        
        // variable value
        if ( !lua_isstring( L, 1 ) ) {
            return luaL_error( L, "vscp.writeVariable: Argument error, string expected: "
                                  "vscp.writeVariable( name[,format] ) ");
        }
        
        const char *pstr = lua_tolstring ( L, 1, &len ); 
        varValue = wxString::FromUTF8( pstr, len );
        
    }
    
    if ( nArgs >= 2 ) {
                
        // format
        if ( !lua_isnumber( L, 2 ) ) {
            return luaL_error( L, "vscp.writevariable: Argument error, number expected: "
                                  "vscp.writevariable( name[,format] ) ");
        }
        
        format = (int)lua_tointeger( L, 2 );
    }
        
    if ( 0 == format ) {
        // Set variable from string
        if ( !variable.setVariableFromString( varValue ) ) {
            return luaL_error( L, "vscp.writevariable: Could not set variable"
                                  " from string.");
        }
    }    
    else if ( 1 == format ) {
        // Set variable from XML object
        if ( !variable.setFromXML( varValue ) ) {
            return luaL_error( L, "vscp.writevariable: Could not set variable"
                                  " from XML object.");
        }
    } 
    else if ( 2 == format ) {
        // Set variable from JSON object
        if ( !variable.setFromJSON( varValue ) ) {
            return luaL_error( L, "vscp.writevariable: Could not set variable"
                                  " from JSON object.");
        }
    }
    else {
        return luaL_error( L, "vscp.writevariable: Format must be 0=string, "
                              "1=XML, 2=JSON");
    }
    
    if ( !gpobj->m_variables.add( variable ) ) {
        return luaL_error( L, "vscp.writevariable: Failed to update/add variable!");
    }
    
    return 1;
 }


///////////////////////////////////////////////////////////////////////////////
// lua_vscp_writeVariableValue
//
// writeVariableValue( varname, value, bBase64)
// 
//

int lua_vscp_writeVariableValue( struct lua_State *L ) 
{  
    wxString varName;    
    wxString varValue;
    CVSCPVariable variable;
    
    int nArgs = lua_gettop( L );
    
    if ( nArgs < 3 ) {
        return luaL_error( L, "vscp.writeVariableValue: Wrong number of arguments: "
                              "vscp.writeVariableValue( varname, varvalue, bBase64 ) ");
    }
    
    // variable name
    if ( !lua_isstring( L, 1 ) ) {
        return luaL_error( L, "vscp.writeVariableValue: Argument error, string expected: "
                              "vscp.writeVariableValue( varname, varvalue, bBase64 ) ");
    }
        
    size_t len;
    const char *pstr = lua_tolstring ( L, 1, &len ); 
    varName = wxString::FromUTF8( pstr, len ); 
    
    
    // variable value
    if ( !lua_isstring( L, 2 ) ) {
        return luaL_error( L, "vscp.writeVariableValue: Argument error, string expected: "
                              "vscp.writeVariableValue( varname, varvalue, bBase64 ) ");
    }
        
    pstr = lua_tolstring ( L, 2, &len ); 
    varValue = wxString::FromUTF8( pstr, len );
    
    
    // variable base64 flag
    bool bBase64 = false;
    if ( !lua_isboolean( L, 3 ) ) {
        return luaL_error( L, "vscp.writeVariableValue: Argument error, boolean expected: "
                              "vscp.writeVariableValue( varname, varvalue, bBase64 ) ");
    }
    
    bBase64 = lua_toboolean ( L, 3 );
    
    if ( !gpobj->m_variables.find( varName, variable ) ) {
        return luaL_error( L, "vscp.writeVariableValue: No variable with that "
                              "name found!");
    }
    
    variable.setValue( varValue, bBase64 );
    
    if ( !gpobj->m_variables.update( variable ) ) {
        return luaL_error( L, "vscp.writeVariableValue: Failed to update variable value!");
    }
    
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_vscp_writeVariableNote
//
// writeVariableNote( varname, note, bBase64 )
// 
//

int lua_vscp_writeVariableNote( struct lua_State *L ) 
{
    wxString varName;    
    wxString varValue;
    CVSCPVariable variable;
    
    int nArgs = lua_gettop( L );
    
    if ( nArgs < 3 ) {
        return luaL_error( L, "vscp.writeVariableNote: Wrong number of arguments: "
                              "vscp.writeVariableNote( varname, varvalue, bBase64 ) ");
    }
    
    // variable name
    if ( !lua_isstring( L, 1 ) ) {
        return luaL_error( L, "vscp.writeVariableNote: Argument error, string expected: "
                              "vscp.writeVariableNote( varname, varvalue, bBase64 ) ");
    }
        
    size_t len;
    const char *pstr = lua_tolstring ( L, 1, &len ); 
    varName = wxString::FromUTF8( pstr, len ); 
    
    
    // variable value
    if ( !lua_isstring( L, 2 ) ) {
        return luaL_error( L, "vscp.writeVariableNote: Argument error, string expected: "
                              "vscp.writeVariableNote( varname, varvalue, bBase64 ) ");
    }
        
    pstr = lua_tolstring ( L, 2, &len ); 
    varValue = wxString::FromUTF8( pstr, len );
    
    
    // variable base64 flag
    bool bBase64 = false;
    if ( !lua_isboolean( L, 3 ) ) {
        return luaL_error( L, "vscp.writeVariableNote: Argument error, boolean expected: "
                              "vscp.writeVariableNote( varname, varvalue, bBase64 ) ");
    }
    
    bBase64 = lua_toboolean ( L, 3 );
    
    if ( !gpobj->m_variables.find( varName, variable ) ) {
        return luaL_error( L, "vscp.writeVariableNote: No variable with that "
                              "name found!");
    }
    
    variable.setNote( varValue, bBase64 );
    
    if ( !gpobj->m_variables.update( variable ) ) {
        return luaL_error( L, "vscp.writeVariableNote: Failed to update variable note!");
    }
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_vscp_deleteVariable
//

int lua_vscp_deleteVariable( struct lua_State *L ) 
{
    wxString varName;    
    CVSCPVariable variable;
    
    int nArgs = lua_gettop( L );
    
    if ( 0 == nArgs  ) {
        return luaL_error( L, "vscp.deleteVariable: Wrong number of arguments: "
                              "vscp.deleteVariable( varname ) ");
    }
    
    // variable name
    if ( !lua_isstring( L, 1 ) ) {
        return luaL_error( L, "vscp.deleteVariable: Argument error, string expected: "
                              "vscp.deleteVariable( varname  ) ");
    }
        
    size_t len;
    const char *pstr = lua_tolstring ( L, 1, &len ); 
    varName = wxString::FromUTF8( pstr, len ); 
    
    
    if ( !gpobj->m_variables.remove( varName ) ) {
        return luaL_error( L, "vscp.deleteVariable: Failed to delete variable!");
    }
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_vscp_sendEvent
//
// lua_vscp_sendEvent( event[,format] )
//
//      format = 0 - String format.
//      format = 1 - XML format.
//      format = 2 - JSON format.
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
// }
//
 
int lua_vscp_sendEvent( struct lua_State *L ) 
{
    int format = 0;
    vscpEventEx ex;
    wxString strEvent;
    CClientItem *pClientItem = NULL;
    
    int nArgs = lua_gettop( L );
    
    // Get the client item
    lua_pushlstring( L, "vscp_clientitem", 15 );
    lua_gettable (L, LUA_REGISTRYINDEX );
    pClientItem = (CClientItem *)lua_touserdata( L, -1 );
    
    if ( NULL == pClientItem ) {
        return luaL_error( L, "vscp.sendEvent: VSCP server client not found.");
    }
    
    if ( nArgs < 1  ) {
        return luaL_error( L, "vscp.sendEvent: Wrong number of arguments: "
                              "vscp.sendEvent( event[,format] ) ");
    }
    
    // Event
    if ( !lua_isstring( L, 1 ) ) {
        return luaL_error( L, "vscp.sendEvent: Argument error, string expected: "
                              "vscp.sendEvent( event[,format] ) ");
    }
        
    size_t len;
    const char *pstr = lua_tolstring ( L, 1, &len ); 
    strEvent = wxString::FromUTF8( pstr, len );
    
    if ( nArgs >= 2 ) {
                
        // format
        if ( !lua_isnumber( L, 2 ) ) {
            return luaL_error( L, "vscp.sendEvent: Argument error, number expected: "
                                  "vscp.sendEvent( event[,format] ) ");
        }
        
        format = (int)lua_tointeger( L, 2 );
    }
    
    if ( 0 == format ) {
        if ( !vscp_setVscpEventExFromString( &ex, strEvent ) ) {
            return luaL_error( L, "vscp.sendEvent: Failed to get VSCP event from string!");
        }
    }
    else if ( 1 == format ) {
        if ( !vscp_convertXMLToEventEx( strEvent, &ex ) ) {
            return luaL_error( L, "vscp.sendEvent: Failed to get "
                                  "VSCP event from XML!");
        }
    }
    else if ( 2 == format ) {
        if ( !vscp_convertJSONToEventEx( strEvent, &ex ) ) {
            return luaL_error( L, "vscp.sendEvent: Failed to get "
                                  "VSCP event from JSON!");
        }
    }
    
    vscpEvent *pEvent;
    vscp_convertVSCPfromEx( pEvent, &ex );
        
    if ( !gpobj->sendEvent( pClientItem, pEvent ) ) {
        // Failed to send event
        vscp_deleteVSCPevent( pEvent );
        return luaL_error( L, "vscp.sendEvent: Failed to send event!");
    }
    
    vscp_deleteVSCPevent( pEvent );
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_vscp_getEvent
//

int lua_vscp_getEvent( struct lua_State *L ) 
{
    int format = 0;
    vscpEventEx ex;
    wxString strEvent;
    CClientItem *pClientItem = NULL;
    
    int nArgs = lua_gettop( L );
    
    // Get the client item
    lua_pushlstring( L, "vscp_clientitem", 15 );
    lua_gettable (L, LUA_REGISTRYINDEX );
    pClientItem = (CClientItem *)lua_touserdata( L, -1 );
    
    if ( NULL == pClientItem ) {
        return luaL_error( L, "vscp.getEvent: VSCP server client not found.");
    }
    
    if ( 0 == nArgs  ) {
        return luaL_error( L, "vscp.getEvent: Wrong number of arguments: "
                              "vscp.getEvent( format ) ");
    }
    
try_again:
    
    // Check the client queue
    if ( pClientItem->m_bOpen && pClientItem->m_clientInputQueue.GetCount() ) {

        CLIENTEVENTLIST::compatibility_iterator nodeClient;
        vscpEvent *pEvent;

        pClientItem->m_mutexClientInputQueue.Lock();
        nodeClient = pClientItem->m_clientInputQueue.GetFirst();
        if ( NULL == nodeClient )  {
            return luaL_error( L, "vscp.getEvent: Failed to get event.");      
        }
        
        pEvent = nodeClient->GetData();                             
        pClientItem->m_clientInputQueue.DeleteNode( nodeClient );
        pClientItem->m_mutexClientInputQueue.Unlock();

        if ( NULL != pEvent ) {

            if ( vscp_doLevel2Filter( pEvent, &pClientItem->m_filterVSCP ) ) {

                // Write it out                
                wxString strResult;
                switch ( format ) {
                    case 0:  // String
                        if ( !vscp_writeVscpEventToString( pEvent, strResult ) ) {
                            return luaL_error( L, 
                                    "vscp.getEvent: Failed to "
                                    "convert event to string form."); 
                        }
                        break;
                        
                    case 1:  // XML
                        if ( !vscp_convertEventToXML( pEvent, strResult ) ) {
                            return luaL_error( L, 
                                    "vscp.getEvent: Failed to "
                                    "convert event to XML form.");
                        }
                        break;
                            
                    case 2:  // JSON
                        if ( !vscp_convertEventToJSON( pEvent, strResult ) ) {
                            return luaL_error( L, 
                                    "vscp.getEvent: Failed to "
                                    "convert event to JSON form.");
                        }
                        break;
                }
                
                // Event is not needed anymore
                vscp_deleteVSCPevent( pEvent );
                
                lua_pushlstring( L, 
                                    (const char *)strResult.mbc_str(),
                                    strResult.Length() );
                                
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
            lua_pushnil( L );    // return code failure
            return 1;
        }
                
    } // events available
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_vscp_getCountEvent
//
//

int lua_vscp_getCountEvent( struct lua_State *L ) 
{
    int count = 0;
    CClientItem *pClientItem = NULL;
    
    // Get the client item
    lua_pushlstring( L, "vscp_clientitem", 15 );
    lua_gettable (L, LUA_REGISTRYINDEX );
    pClientItem = (CClientItem *)lua_touserdata( L, -1 );
    
    if ( NULL == pClientItem ) {
        return luaL_error( L, "vscp.getCountEvent: VSCP server client not found.");
    }
    
    if ( pClientItem->m_bOpen ) {
        count = pClientItem->m_clientInputQueue.GetCount();
    }
    else {
        count = 0;
    }
    
    lua_pushinteger( L, count );    // return count
    
    return 1;
 }


///////////////////////////////////////////////////////////////////////////////
// lua_vscp_setFilter
//
// setFilter( strFilter[, format] )
//
// format = 0 - From string
// format = 1 - From XML
// format = 2 - from JSON
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

int lua_vscp_setFilter( struct lua_State *L ) 
{
    int format = 0;
    vscpEventFilter filter;
    wxString strFilter;
    CClientItem *pClientItem = NULL;
    
    int nArgs = lua_gettop( L );
    
    // Get the client item
    lua_pushlstring( L, "vscp_clientitem", 15 );
    lua_gettable (L, LUA_REGISTRYINDEX );
    pClientItem = (CClientItem *)lua_touserdata( L, -1 );
    
    if ( NULL == pClientItem ) {
        return luaL_error( L, "vscp.setFilter: VSCP server client not found.");
    }
    
    if ( nArgs < 1  ) {
        return luaL_error( L, "vscp.setFilter: Wrong number of arguments: "
                              "vscp.setFilter( filter[,format] ) ");
    }
    
    // Event
    if ( !lua_isstring( L, 1 ) ) {
        return luaL_error( L, "vscp.setFilter: Argument error, string expected: "
                              "vscp.setFilter( filter[,format] ) ");
    }
        
    size_t len;
    const char *pstr = lua_tolstring ( L, 1, &len ); 
    strFilter = wxString::FromUTF8( pstr, len );
    
    if ( nArgs >= 2 ) {
                
        // format
        if ( !lua_isnumber( L, 2 ) ) {
            return luaL_error( L, "vscp.setFilter: Argument error, number expected: "
                                  "vscp.setFilter( filter[,format] ) ");
        }
        
        format = (int)lua_tointeger( L, 2 );
    }
    
    switch ( format ) {
        
        case 0:
            if ( !vscp_readFilterFromString( &filter, strFilter) ) {
                luaL_error( L, "vscp.setFilter: Failed to read filter!");
            }
            break;
                    
        case 1:
            // TODO XML version
            break;
            
        case 2:
            // TODO JSON version
            break;
            
    }
    
    // Set the filter
    vscp_copyVSCPFilter( &pClientItem->m_filterVSCP, &filter );
    
    return 1;
 }


///////////////////////////////////////////////////////////////////////////////
// lua_send_Measurement
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

int lua_send_Measurement( struct lua_State *L )
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
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_is_Measurement
//

int lua_is_Measurement( struct lua_State *L ) 
{
    vscpEventEx ex;
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_get_MeasurementValue
//

int lua_get_MeasurementValue( struct lua_State *L ) 
{
    double value;
    vscpEventEx ex;
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_get_MeasurementUnit
//

int lua_get_MeasurementUnit( struct lua_State *L ) 
{
    vscpEventEx ex;
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_get_MeasurementSensorIndex
//

int lua_get_MeasurementSensorIndex( struct lua_State *L ) 
{
    vscpEventEx ex;
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_get_MeasurementZone
//

int lua_get_MeasurementZone( struct lua_State *L ) 
{
    vscpEventEx ex;
    
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// lua_get_MeasurementSubZone
//

int lua_get_MeasurementSubZone( struct lua_State *L ) 
{
    vscpEventEx ex;
    
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_connect
//
// Connect to remote server
//

int lua_tcpip_connect( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_connect_ssl
//
// Connect to remote server using SSL
//

int lua_tcpip_connect_ssl( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_connect_info
//
// Get connection info
//      remote address
//      remote port
//      server address
//      server port
//      bSSL
//      

int lua_tcpip_connect_info( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_close
//
// Close connection to remote server
//

int lua_tcpip_close( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_write
//
// Write data remote sever
//

int lua_tcpip_write( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_read
//
// Read data from remote sever
//

int lua_tcpip_read( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_get_response
//
// Wait for response from remote sever
//

int lua_tcpip_get_response( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_download
//
// Download data from remote web server
//

int lua_tcpip_download( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_get_httpd_version
//
// Get version for httpd code
//

int lua_get_httpd_version( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_url_decode
//
// URL-decode input buffer into destination buffer.
//

int lua_url_decode( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_url_encode
//
// URL-encode input buffer into destination buffer.
//

int lua_url_encode( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_websocket_connect
//
// Connect to remote websocket client.
//

int lua_websocket_connect( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_websocket_write
//
// URL-encode input buffer into destination buffer.
//

int lua_websocket_write( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_websocket_read
//
// Read data from remote websocket host
//

int lua_websocket_read( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// lua_websocket_lock
//
// lock connection
//

int lua_websocket_lock( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// js_websocket_unlock
//
// unlock connection
//

int js_websocket_unlock( struct lua_State *L )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// js_md5
//
// Calculate md5 digest
//

int js_md5( struct lua_State *L )
{
    return 1;
}