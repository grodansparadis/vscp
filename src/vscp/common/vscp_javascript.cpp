// vscp_javascript.cpp
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
#include <vscp_javascript.h>
#include <duktape.h>
#include <duktape_vscp.h>
#include <duk_module_node.h>
#include <dm.h>

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

///////////////////////////////////////////////////////////////////////////////
// actionThread_JavaScript
//
// This thread executes a JavaScript 
//

actionThread_JavaScript::actionThread_JavaScript( wxString& strScript,
                                                    wxThreadKind kind )
                                            : wxThread( kind )
{
    //OutputDebugString( "actionThreadURL: Create");
    m_wxstrScript = strScript;  // Script to execute
}

actionThread_JavaScript::~actionThread_JavaScript()
{

}


///////////////////////////////////////////////////////////////////////////////
// Entry
//
//

void *actionThread_JavaScript::Entry()
{
    m_start = wxDateTime::Now();    // Mark start time
    
    // Create new JavaScript context
    duk_context *ctx = duk_create_heap_default();
    
    // Check if OK
    if ( !ctx ) { 
        // Failure
        return NULL; 
    }
    
    // Helpers   
    duk_push_c_function( ctx, js_vscp_print, 1 );
    duk_put_global_string(ctx, "print");
    
    // External module support 
    duk_push_object( ctx );
    duk_push_c_function( ctx, js_resolve_module, DUK_VARARGS );
    duk_put_prop_string( ctx, -2, "resolve" );
    duk_push_c_function( ctx, js_load_module, DUK_VARARGS );
    duk_put_prop_string( ctx, -2, "load");
    duk_module_node_init( ctx );
    
    // Add VSCP methods
    duk_push_c_function( ctx, js_vscp_log, DUK_VARARGS );
    duk_put_global_string(ctx, "vscp_log");
    
    duk_push_c_function( ctx, js_vscp_sleep, 1 );
    duk_put_global_string(ctx, "vscp_sleep");
    
    duk_push_c_function( ctx, js_vscp_readVariable, 1 );
    duk_put_global_string(ctx, "vscp_readVariable");
    
    duk_push_c_function( ctx, js_vscp_writeVariable, 1 );
    duk_put_global_string(ctx, "vscp_writeVariable");
    
    duk_push_c_function( ctx, js_vscp_deleteVariable, 1 );
    duk_put_global_string(ctx, "vscp_deleteVariable");
    
    duk_push_c_function( ctx, js_vscp_sendEvent, 1 );
    duk_put_global_string(ctx, "vscp_sendEvent");
    
    duk_push_c_function( ctx, js_vscp_getEvent, 1 );
    duk_put_global_string(ctx, "vscp_receiveEvent");
    
    duk_push_c_function( ctx, js_vscp_getCountEvent, 1 );
    duk_put_global_string(ctx, "vscp_countEvent");
    
    duk_push_c_function( ctx, js_vscp_setFilter, 1 );
    duk_put_global_string(ctx, "vscp_setFilter");
    
    duk_push_c_function( ctx, js_is_Measurement, 1 );    
    duk_put_global_string(ctx, "vscp_isMeasurement");
    
    duk_push_c_function( ctx, js_send_Measurement,1  );
    duk_put_global_string(ctx, "vscp_sendMeasurement");
    
    duk_push_c_function( ctx, js_get_MeasurementValue, 1 );
    duk_put_global_string(ctx, "vscp_getMeasurementValue"); 
    
    duk_push_c_function( ctx, js_get_MeasurementUnit, 1 );
    duk_put_global_string(ctx, "vscp_getMeasurementUnit");
    
    duk_push_c_function( ctx, js_get_MeasurementSensorIndex, 1 );
    duk_put_global_string(ctx, "vscp_getMeasurementSensorIndex");
    
    duk_push_c_function( ctx, js_get_MeasurementZone, 1 );
    duk_put_global_string(ctx, "vscp_getMeasurementZone");
    
    duk_push_c_function( ctx, js_get_MeasurementSubZone, 1 );
    duk_put_global_string(ctx, "vscp_getMeasurementSubZone");
    
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
    
    // This is an active client
    m_pClientItem->m_bOpen = false;
    m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_JAVASCRIPT;
    m_pClientItem->m_strDeviceName = _("Internal daemon JavaScript client. ");
    wxDateTime now = wxDateTime::Now();
    m_pClientItem->m_strDeviceName += now.FormatISODate();
    m_pClientItem->m_strDeviceName += _(" ");
    m_pClientItem->m_strDeviceName += now.FormatISOTime();

    // Add the client to the Client List
    gpobj->m_wxClientMutex.Lock();
    gpobj->addClient( m_pClientItem );
    gpobj->m_wxClientMutex.Unlock();
    
    // Open the channel
    m_pClientItem->m_bOpen = true;
    
    // Execute the JavaScript
    duk_push_string( ctx, (const char *)m_wxstrScript.mbc_str() );
    if ( 0 != duk_peval( ctx ) ) {
        wxString strError = 
                wxString::Format( "JavaScript failed to execute: %s\n", 
                                    duk_safe_to_string(ctx, -1) );
        gpobj->logMsg( strError, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
    }
    
    // If the script wants to log results it can do so 
    // by itself with the log function
 
    duk_pop(ctx);  // pop eval. result 
    
    // Close the channel
    m_pClientItem->m_bOpen = false;
    
    // Remove client and session item
    gpobj->m_wxClientMutex.Lock();
    gpobj->removeClient( m_pClientItem );
    m_pClientItem = NULL;
    gpobj->m_wxClientMutex.Unlock();

    // Destroy the JavaScript context
    duk_destroy_heap( ctx );
    
    m_stop = wxDateTime::Now();     // Mark stop time
 
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void actionThread_JavaScript::OnExit()
{

}