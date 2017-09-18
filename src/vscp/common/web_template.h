// web_template.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2017 
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


// * * *    M e n u    * * *

#define WEB_COMMON_MENU "<ul id=\"nav\">      "\
                           " <li>"\
                           "    <a href=\"/vscp/\">Home</a>"\
                           " </li>"\
                           " <li>"\
                           "    <a href=\"#\">Configuration</a>"\
                           "    <ul>"\
                           "        <li>"\
                           "            <a href=\"/vscp/configure\">Configuration File</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/variables\">Variable - List</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/varnew\">Variable - New</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/dm\">DM - list</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/dmedit?new=true\">DM - new element</a>"\
                           "        </li>"\
                           "    </ul>"\
                           " </li>"\
                           " <li>"\
                           "    <a href=\"#\">Tools</a>"\
                           "    <ul>"\
                           "        <li>"\
                           "            <a href=\"/vscp/session\">VSCP Client</a>"\
                           "        </li>"\
                           "    </ul>"\
                           " </li>"\
                           " <li>"\
                           "    <a href=\"#\">Devices</a>"\
                           "    <ul>"\
                           "        <li>"\
                           "            <a href=\"/vscp/discovery\">Discovery</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/interfaces\">Interfaces</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/configure\">Configure</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/table\">Tables</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/bootload\">Update firmware</a>"\
                           "        </li>"\
                           "    </ul>"\
                           " </li>"\
                           " <li>"\
                           "    <a href=\"#\">Logs</a>"\
                           "    <ul>"\
                           "        <li>"\
                           "            <a href=\"/vscp/log/general\">General Log</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/log/access\">Access Log</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/log/security\">Security Log</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/log/dm\">Decision Matrix Log</a>"\
                           "        </li>"\
                           "    </ul>"\
                           " </li>"\
                           " <li>"\
                           "     <a href=\"#\">Help</a>"\
                           "     <ul>"\
                           "         <li>"\
                           "             <a href=\"http://www.vscp.org/docs/vscpd/doku.php\" target=\"moon\">VSCP Daemon</a>"\
                           "         </li>"\
                           "         <li>"\
                           "             <a href=\"http://www.vscp.org/docs/vscpworks/doku.php\" target=\"moon\">VSCP Works</a>"\
                           "         </li>"\
                           "         <li>"\
                           "             <a href=\"http://www.vscp.org/docs/vscphelper/doku.php\" target=\"moon\">VSCP Helper lib</a>"\
                           "         </li>"\
                           "         <li>"\
                           "             <a href=\"http://www.vscp.org/wiki/doku.php\" target=\"moon\">VSCP wiki</a>"\
                           "         </li>"\
                           "         <li>"\
                           "             <a href=\"http://www.vscp.org\" target=\"moon\">VSCP Site</a>"\
                           "         </li>"\
                           "     </ul>"\
                           " </li>"\
                           "</ul>"

// Common head up to <body>
// Contains three string insert points for printf
// 1. Page title
#define WEB_COMMON_HEAD "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head><meta content=\"text/html; charset=utf8\" http-equiv=\"content-type\"><title>%s</title>"

#define WEB_COMMON_HEAD_END_BODY_START "</head><body>"

// End with footer
#define WEB_COMMON_END "<br><br><br><br><div id=\"footer\"> Copyright &copy; 2000-2017 VSCP & Friends,<a href=\"mailto:info@paradiseofthefrog.com\">Ake Hedman</a><a href=\"http://www.paradiseofthefrog.com\"> Paradise of the Frog.</a></div></div></body></html>"

// List navigation - 
// Action url           - string
// First shown record   - integer
// Last shown record    - integer
// total records        - integer
// count to show    - integer
#define WEB_COMMON_LIST_NAVIGATION "<br><div style=\"text-align:center\">\
                        <form method=\"get\" action=\"%s\" name=\"PageSelect\">\
                        <button type=\"submit\" value=\"first\" name=\"navbtn\">&lt;&lt;</button>\
                        <button type=\"submit\" value=\"previous\" name=\"navbtn\">&lt;</button>\
                        <button type=\"submit\" value=\"next\" name=\"navbtn\">&gt;</button>\
                        <button type=\"submit\" value=\"last\" name=\"navbtn\">&gt;&gt;</button>\
                        <span style=\"font-weight:bold\"> %lu - %lu</span> of \
                        <span style=\"font-weight:bold\">%lu</span> Show: \
                        <input name=\"count\" value=\"%lu\" size=\"3\">\
                        <input name=\"from\" value=\"%lu\" type=\"hidden\">\
                        <input name=\"light\" value=\"%s\" type=\"hidden\"></form><br></div> "

// Clickable/navigational table rows
// Fill in Navigation url
#define WEB_COMMON_TR_CLICKABLE_ROW "<tr onmouseover=\"ChangeColor(this, true);\" onmouseout=\"ChangeColor(this, false);\" onclick=\"DoNav('%s')\" >"

// * * * DM List * * *

// Place after menus
#define WEB_DMLIST_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Decision Matrix</h1></div><table><tbody>"

// Place before common end
#define WEB_DMLIST_TABLE_END "</tbody></table>"

// Header
#define WEB_DMLIST_TR_HEAD "<tr><th>Pos</th><th>Entry</th></tr>"

#define WEB_DMLIST_TD_CENTERED "<td id=\"tdcenter\">"



// * * * DM Edit * * *


// Place after menus
#define WEB_DMEDIT_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Decision Matrix Edit</h1></div><table><tbody>"

// Place before common end
#define WEB_DMEDIT_TABLE_END "</tbody></table>"

// DM-Edit post - 
#define WEB_DMEDIT_SUBMIT "<br><div style=\"text-align:center\"><button type=\"submit\" value=\"btnsave\" name=\"btnsave\">save</button><br></div>"


// * * * DM Post * * *

#define WEB_DMPOST_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Decision Matrix Edit</h1></div><table><tbody>"



// * * * Interface List * * *

// Place after menus
#define WEB_IFLIST_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Interfaces</h1></div><table><tbody>"

// Place before common end
#define WEB_IFLIST_TABLE_END "</tbody></table>"

// Rows
#define WEB_IFLIST_TR_HEAD "<tr><th>I/f Id</th><th>Type</th><th>Interface GUID</th><th>Name</th><th>Start time</th></tr>"

#define WEB_IFLIST_TR "<tr onmouseover=\"ChangeColor(this, true);\" onmouseout=\"ChangeColor(this, false);\" onclick=\"DoNav('http://www.vscp.org/')\" >"

#define WEB_IFLIST_TD_CENTERED "<td id=\"tdcenter\">"

#define WEB_IFLIST_TD_GUID "<td id=\"tdguid\">"



// * * * Variables List * * *

// Place after menus
#define WEB_VARLIST_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Variables</h1></div><table><tbody>"

// Table head
#define WEB_VARLIST_TR_HEAD "<tr><th>Pos</th><th>Type</th><th>Variable</th></tr>"



// * * * Variables Edit * * *

// Place after menus
#define WEB_VAREDIT_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Variable Edit</h1></div><table><tbody>"

// Place before common end
#define WEB_VAREDIT_TABLE_END "</tbody></table>"

//  Variable-Edit submit 
#define WEB_VAREDIT_SUBMIT "<br><div style=\"text-align:center\"><button type=\"submit\" value=\"btnsave\" name=\"btnsave\">save</button><br></div>"

// * * * Variable Post * * *

#define WEB_VARPOST_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Variable Edit</h1></div><table><tbody>"

//  Variable-New step1 submit 
#define WEB_VARNEW_SUBMIT "<br><div style=\"text-align:center\"><button type=\"submit\" value=\"btnnext\" name=\"btnnext\">next</button><br></div>"




// * * * Table List * * *

// Place after menus
#define WEB_TABLELIST_BODY_START "<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">VSCP - Tables</h1></div><table><tbody>"

// Place before common end
#define WEB_TABLELIST_TABLE_END "</tbody></table>"

// Rows
#define WEB_TABLELIST_TR_HEAD "<tr><th>Name</th><th>Type</th></tr>"

#define WEB_TABLELIST_TR "<tr onmouseover=\"ChangeColor(this, true);\" onmouseout=\"ChangeColor(this, false);\" onclick=\"DoNav('http://www.vscp.org/')\" >"

#define WEB_TABLELIST_TD_CENTERED "<td id=\"tdcenter\">"




// * * * Table value List * * *

// Place after menus
#define WEB_TABLEVALUELIST_BODY_START _("<br><div id=\"content\"><div id=\"header\"><h1 id=\"header\">List of values - %s</h1></div><table><tbody>")

// Place before common end
#define WEB_TABLEVALUELIST_TABLE_END "</tbody></table>"

// Rows
#define WEB_TABLEVALUELIST_TR_HEAD "<tr><th>Record</th><th>Date</th><th>Value</th></tr>"

#define WEB_TABLEVALUELIST_TR "<tr onmouseover=\"ChangeColor(this, true);\" onmouseout=\"ChangeColor(this, false);\" onclick=\"DoNav('http://www.vscp.org/')\" >"

#define WEB_TABLEVALUELIST_TD_CENTERED "<td id=\"tdcenter\">"

#define WEB_TABLEVALUELIST_TD_GUID "<td id=\"tdguid\">"

// Tablevalue navigation - 
// Action url			- string
// First shown record	- integer
// Last shown record	- integer
// total records		- integer
// count to show		- integer
#define WEB_TABLEVALUE_LIST_NAVIGATION "<br><div style=\"text-align:center\"><form method=\"get\" action=\"%s\" name=\"PageSelect\"><button type=\"submit\" value=\"first\" name=\"navbtn\">&lt;&lt;</button><button type=\"submit\" value=\"previous\" name=\"navbtn\">&lt;</button><button type=\"submit\" value=\"next\" name=\"navbtn\">&gt;</button><button type=\"submit\" value=\"last\" name=\"navbtn\">&gt;&gt;</button><span style=\"font-weight:bold\"> %d - %d</span> of <span style=\"font-weight:bold\">%d</span> Show: <input name=\"count\" value=\"%d\" size=\"3\"><input name=\"from\" value=\"%d\" type=\"hidden\"><input name=\"tblname\" value=\"%s\" type=\"hidden\"><br></form></div> "


#endif
