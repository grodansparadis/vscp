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
    
    v7_val_t valName = v7_arg(v7, 0);
    
    wxString wxName = v7_get_cstring( v7, &valName );
    if ( gpobj->m_VSCP_Variables.find( wxName, variable ) ) {
        
        // Get the variable on JSON format
        variable.getAsJSON( strResult );
        
        // Return result to JavaScript 
        *res = v7_mk_string( v7, strResult.mbc_str(), strlen( strResult.mbc_str() ), 1 );
        
    }
    else {
        *res = v7_mk_null();
    }
    
    return V7_OK;
}