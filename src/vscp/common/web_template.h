// web_template.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 
// Grodans Paradis AB, <akhe@grodansparadis.com>
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

#if !defined(WEBTEMPLATE__INCLUDED_)
#define WEBTEMPLATE__INCLUDED_


// * * * M e n u * * *

#define WEB_COMMON_MENU "<ul id=\"nav\"><li><a href=\"#\">Configuration</a><ul><li><a href=\"#\">Configuration File</a></li><li><a href=\"#\">Variables</a></li><li><a href=\"%s/vscp/dm\">DM - list</a></li><li><a href=\"%s/vscp/dmedit?new=true\">DM - new element</a></li></ul></li><li><a href=\"#\">Tools</a><ul><li><a href=\"#\">VSCP Client</a></li></ul></li><li><a href=\"#\">Devices</a><ul><li><a href=\"#\">Discovery</a></li><li><a href=\"%s/vscp/interfaces\">Interfaces</a></li><li><a href=\"#\">Configure</a></li><li><a href=\"#\">Update firmware</a></li></ul></li></ul>"


// * * * C o m m o n * * *

// Common head up to <body>
// Contains three string insert points for printf
// 1. Page title
#define WEB_COMMON_HEAD "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head><meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\"><title>%s</title>"

#define WEB_COMMON_HEAD_END_BODY_START "</head><body>"

// End with footer
#define WEB_COMMON_END "<br><br><br><br><div id=\"footer\"> Copyright &copy; 2000-2013 VSCP & Friends,<a href=\"mailto:info@grodansparadis.com\">Ake Hedman</a><a href=\"http://www.grodansparadis.com\"> Grodans Paradis AB.</a></div></div></body></html>"

// List navigation - 
// Action url			- string
// First shown record	- integer
// Last shown record	- integer
// total records		- integer
// count to show		- integer
#define WEB_COMMON_LIST_NAVIGATION "<br><div style=\"text-align:center\"><form method=\"get\" action=\"%s\" name=\"PageSelect\"><button type=\"submit\" value=\"first\" name=\"navbtn\">&lt;&lt;</button><button type=\"submit\" value=\"previous\" name=\"navbtn\">&lt;</button><button type=\"submit\" value=\"next\" name=\"navbtn\">&gt;</button><button type=\"submit\" value=\"last\" name=\"navbtn\">&gt;&gt;</button><span style=\"font-weight:bold\">%d - %d</span> of <span style=\"font-weight:bold\">%d</span> Show: <input name=\"count\" value=\"%d\" size=\"3\"><input name=\"from\" value=\"%d\" type=\"hidden\"><input name=\"light\" value=\"%s\" type=\"hidden\"><br></form></div>"

// Clickable/navigational table rows
// Fill in Navigation url
#define WEB_COMMON_TR_CLICKABLE_ROW "<tr onmouseover=\"ChangeColor(this, true);\" onmouseout=\"ChangeColor(this, false);\" onclick=\"DoNav('%s')\" >"

// * * * DM List * * *

// Place after menus
#define WEB_DMLIST_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Decision Matrix</h1></div><table><tbody>"

// Place before common end
#define WEB_DMLIST_TABLE_END "</tbody></table>"

// Rows
#define WEB_DMLIST_TR_HEAD "<tr><th>Pos</th><th>Entry</th></tr>"

#define WEB_DMLIST_TD_CENTERED "<td id=\"tdcenter\">"



// * * * DM Edit * * *


// Place after menus
#define WEB_DMEDIT_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Decision Matrix Edit</h1></div><table><tbody>"

// Place before common end
#define WEB_DMEDIT_TABLE_END "</tbody></table>"

// DM-Edit post - 
#define WEB_DMEDIT_SUBMIT "<br><div style=\"text-align:center\"><button type=\"submit\" value=\"btnexit\" name=\"btnexit\">Exit</button><button type=\"submit\" value=\"btnsave\" name=\"btnsave\">save</button><br></div>"


// * * * DM Post * * *

#define WEB_DMPOST_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Decision Matrix Edit</h1></div><table><tbody>"



// * * * Interface List * * *

// Place after menus
#define WEB_IFLIST_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Interfaces</h1></div><table><tbody>"

// Place before common end
#define WEB_IFLIST_TABLE_END "</tbody></table>"

// Rows
#define WEB_IFLIST_TR_HEAD "<tr><th>I/f Id</th><th>Type</th><th>Interface GUID</th><th>Name</th><th>Start time</th></tr>"

#define WEB_IFLIST_TR "<tr onmouseover=\"ChangeColor(this, true);\" onmouseout=\"ChangeColor(this, false);\" onclick=\"DoNav('http://www.yahoo.com/')\" >"

#define WEB_IFLIST_TD_CENTERED "<td id=\"tdcenter\">"

#define WEB_IFLIST_TD_GUID "<td id=\"tdguid\">"

#endif