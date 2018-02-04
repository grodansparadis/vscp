// web_template.h
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

#if !defined(WEBTEMPLATE__INCLUDED_)
#define WEBTEMPLATE__INCLUDED_


// * * *    M e n u    * * *

#define WEB_COMMON_MENU "<ul id=\"nav\">      "\
                           " <li>"\
                           "    <a href=\"/vscp/\">Home</a>"\
                           " </li>"\
                           " <li>"\
                           "    <a href=\"#\">System</a>"\
                           "    <ul>"\
                           "        <li>"\
                           "            <a href=\"/vscp/configure\">List configuration</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/settings\">Persistent settings</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/varlist\">Variables</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/dm\">Decision Matrix</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/zone\">Zones</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/subzone\">Subzones</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/guid\">GUIDs</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/location\">Locations</a>"\
                           "        </li>"\
                           "    </ul>"\
                           " </li>"\
                           " <li>"\
                           "    <a href=\"#\">Tools</a>"\
                           "    <ul>"\
                           "        <li>"\
                           "            <a href=\"/vscp/password\">Password generator</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/bootload\">Update firmware</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/sendevent\">VSCP Send event</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/tests\">VSCP system tests</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/restart\">Restart</a>"\
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
                           "    </ul>"\
                           " </li>"\
                           " <li>"\
                           "    <a href=\"#\">Logs</a>"\
                           "    <ul>"\
                           "        <li>"\
                           "            <a href=\"/vscp/log\">View Log</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/logdelete\">Clear Log</a>"\
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
#define WEB_COMMON_HEAD "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \
\"http://www.w3.org/TR/html4/strict.dtd\"><html><head>\
<meta content=\"text/html; charset=utf8\" http-equiv=\"\
content-type\" /><title>%s</title>"

#define WEB_COMMON_HEAD_END_BODY_START "</head><body>"

// End with footer
#define WEB_COMMON_END "<br><br><br><br><div id=\"footer\"> %s \
</div></body></html>"

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
#define WEB_COMMON_TR_CLICKABLE_ROW "<tr onmouseover=\"ChangeColor(this, true);\" \
                                        onmouseout=\"ChangeColor(this, false);\" \
                                        onclick=\"DoNav('%s')\" >"

#define WEB_COMMON_TR_NON_CLICKABLE_ROW "<tr onmouseover=\"ChangeColor(this, true);\" \
                                        onmouseout=\"ChangeColor(this, false);\" >"

#define WEB_COMMON_TABLE_END "</tbody></table>"


// * * * DM List * * *

// Place after menus
#define WEB_DMLIST_BODY_START "<br><div id=\"content\"><div \
                                id=\"header\"><h1 id=\"header\">\
                                VSCP - Decision Matrix</h1></div><table><tbody>"

// Place before common end
#define WEB_DMLIST_TABLE_END "</tbody></table>"

// Header
#define WEB_DMLIST_TR_HEAD "<tr><th>Pos</th><th>Entry</th></tr>"

#define WEB_DMLIST_TD_CENTERED "<td id=\"tdcenter\">"


// * * * DM Edit * * *


// Place after menus
#define WEB_DMEDIT_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                <h1 id=\"header\">Decision Matrix Edit</h1>\
                                </div><table><tbody>"

// Place before common end
#define WEB_DMEDIT_TABLE_END "</tbody></table>"

// DM-Edit post - 
#define WEB_DMEDIT_SUBMIT "<br><div style=\"text-align:center\">\
                            <button type=\"submit\" value=\"btnsave\" \
                            name=\"btnsave\">save</button><br></div>"


// * * * DM Post * * *

#define WEB_DMPOST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                <h1 id=\"header\">Decision Matrix Edit</h1>\
                                </div><table><tbody>"



// * * * Interface List * * *

// Place after menus
#define WEB_IFLIST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                  <h1 id=\"header\">Interfaces</h1></div>\
                                  <table><tbody>"

// Place before common end
#define WEB_IFLIST_TABLE_END "</tbody></table>"

// Rows
#define WEB_IFLIST_TR_HEAD "<tr><th>I/f Id</th><th>Type</th><th>Interface GUID\
                            </th><th>Name</th><th>Start time</th></tr>"

#define WEB_IFLIST_TR "<tr onmouseover=\"ChangeColor(this, true);\" \
                            onmouseout=\"ChangeColor(this, false);\" \
                            onclick=\"DoNav('/vscp/ifinfo')\" >"

#define WEB_IFLIST_TD_CENTERED "<td id=\"tdcenter\">"

#define WEB_IFLIST_TD_GUID "<td id=\"tdguid\">"



// * * * Variables List * * *

// Place after menus
#define WEB_VARLIST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                <h1 id=\"header\">Variables</h1></div>\
                                <table><tbody>"

// Table head
#define WEB_VARLIST_TR_HEAD "<tr><th>Pos</th><th>Type</th><th>Variable</th>\
                                <th></th></tr>"



// * * * Variables Edit * * *

// Place after menus
#define WEB_VAREDIT_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">VSCP - Variable Edit</h1>\
                                    </div><table><tbody>"

// Place before common end
#define WEB_VAREDIT_TABLE_END "</tbody></table>"


// * * * Zone List * * *

// Place after menus
#define WEB_ZONELIST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                <h1 id=\"header\">Zones</h1></div>\
                                <table><tbody>"

// * * * Zone Edit * * *

// Place after menus
#define WEB_ZONEEDIT_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">VSCP - Zone Edit</h1>\
                                    </div><table><tbody>"

// Table head
#define WEB_ZONELIST_TR_HEAD "<tr><th>Pos</th><th>Type</th><th>Zones</th>\
                                <th></th></tr>"



// * * * Zone Edit * * *

// Place after menus
#define WEB_ZONEEDIT_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">VSCP - Zone Edit</h1>\
                                    </div><table><tbody>"

// Place before common end
#define WEB_ZONEEDIT_TABLE_END "</tbody></table>"

// * * * Post * * *
#define WEB_ZONE_POST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">Zone Post</h1>\
                                    </div>"

// * * * Subzone List * * *

// Place after menus
#define WEB_SUBZONELIST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                <h1 id=\"header\">Subzones</h1></div>\
                                <table><tbody>"

// Table head
#define WEB_SUBZONELIST_TR_HEAD "<tr><th>Pos</th><th>Type</th><th>Subzones</th>\
                                <th></th></tr>"


// * * * Subzone Edit * * *

// Place after menus
#define WEB_SUBZONEEDIT_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">VSCP - Subzone Edit</h1>\
                                    </div><table><tbody>"

// Place before common end
#define WEB_SUBZONEEDIT_TABLE_END "</tbody></table>"

// * * * Post * * *
#define WEB_SUBZONE_POST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">Subzone Post</h1>\
                                    </div>"

// * * * GUID List * * *

// Place after menus
#define WEB_GUIDLIST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                <h1 id=\"header\">GUID</h1></div>\
                                <table><tbody>"

#define WEB_GUIDLIST_TD_CENTERED "<td id=\"tdcenter\">"






// Table head
#define WEB_GUIDLIST_TR_HEAD "<tr><th>Pos</th><th>Type</th><th>GUID's</th>\
                                <th></th></tr>"



// * * * GUID Edit * * *

// Place after menus
#define WEB_GUIDEDIT_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">VSCP - GUID Edit</h1>\
                                    </div><table><tbody>"

// Place before common end
#define WEB_GUIDEDIT_TABLE_END "</tbody></table>"

// * * * Post * * *
#define WEB_GUID_POST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">GUID Post</h1>\
                                    </div>"



// * * * Location List * * *

// Place after menus
#define WEB_LOCATIONLIST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                <h1 id=\"header\">Locations</h1></div>\
                                <table><tbody>"

// Table head
#define WEB_LOCATIONLIST_TR_HEAD "<tr><th>Pos</th><th>Type</th><th>Locations</th>\
                                <th></th></tr>"



// * * * Location Edit * * *

// Place after menus
#define WEB_LOCATIONEDIT_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">VSCP - Location Edit</h1>\
                                    </div><table><tbody>"

// Place before common end
#define WEB_LOCATIONEDIT_TABLE_END "</tbody></table>"


// * * * Post * * *
#define WEB_LOCATION_POST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">GUID Post</h1>\
                                    </div>"

///////////////////////////////////////////////////////////////////////////////
//                              Variable
//////////////////////////////////////////////////////////////////////////////

//  Variable-New step1 submit 
#define WEB_VARNEW_SUBMIT "<br><div style=\"text-align:center\">\
                            <button type=\"submit\" value=\"btnnext\" \
                            name=\"btnnext\">next</button><br></div>"

/*
 * <form onsubmit="return isValidForm()" />
 *
 * If isValidForm() returns false, then your form doesn't submit.
 *
 * You should also probably move your event handler from inline.
 *
 * document.getElementById('my-form').onsubmit = function() {
 *   return false;
 * };
 */

//  Common (Variable/zone/subzone/guid)-Edit submit 
#define WEB_COMMON_SUBMIT "<br><div style=\"text-align:center\">\
                            <button type=\"submit\" \
                               onclick=\"DoNav(%s);\" name=\"btnsave\">Save</button> \
                            <button type=\"reset\" value=\"reset\">Reset</button> \
                            <button type=\"submit\" value=\"true\" name=\"btncancel\" >Cancel</button>\
                            <br></div>"
 
// onclick=\"javascript:alert('hi');window.location='varlist';\"
//  <button type=\"cancel\" onclick=\"javascript:window.location='%s';\">Cancel</button>\

// * * * Variable Post * * *
#define WEB_VARPOST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">Variable Post</h1>\
                                    </div>"


////////////////////////////////////////////////////////////////////////////////
//                                 Log
////////////////////////////////////////////////////////////////////////////////


// Place after menus
#define WEB_LOG_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                <h1 id=\"header\">Log</h1></div>\
                                <table><tbody>"

// Table head
#define WEB_LOG_TR_HEAD "<tr> <th>Date</th> <th>Type</th> <th>Level</th> <th>Message</th> </tr>"

//  Log pre step submit 
#define WEB_LOG_SUBMIT "<br><div style=\"text-align:center\">\
                            <button type=\"submit\" value=\"btnnext\" \
                            name=\"btnnext\">next</button><br></div>"

// List navigation - 
// Action url           - string
// First shown record   - integer
// Last shown record    - integer
// total records        - integer
// count to show    - integer
#define WEB_LOG_LIST_NAVIGATION "<br><div style=\"text-align:center\">\
                        <form method=\"get\" action=\"%s\" name=\"PageSelect\">\
                        <button type=\"submit\" value=\"first\" name=\"navbtn\">&lt;&lt;</button>\
                        <button type=\"submit\" value=\"previous\" name=\"navbtn\">&lt;</button>\
                        <button type=\"submit\" value=\"next\" name=\"navbtn\">&gt;</button>\
                        <button type=\"submit\" value=\"last\" name=\"navbtn\">&gt;&gt;</button>\
                        <span style=\"font-weight:bold\"> %lu - %lu</span> of \
                        <span style=\"font-weight:bold\">%lu</span> Show: \
                        <input name=\"count\" value=\"%lu\" size=\"3\">\
                        <input name=\"from\" value=\"%lu\" type=\"hidden\">\
                        <input name=\"light\" value=\"%s\" type=\"hidden\">\
                        <input name=\"type\" value=\"%d\" type=\"hidden\">\
                        <input name=\"level\" value=\"%d\" type=\"hidden\">\
                        </form><br></div> "

// * * * Table List * * *

// Place after menus
#define WEB_TABLELIST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                    <h1 id=\"header\">Tables</h1></div>\
                                    <table><tbody>"

// Place before common end
#define WEB_TABLELIST_TABLE_END "</tbody></table>"

// Rows
#define WEB_TABLELIST_TR_HEAD "<tr><th>Name</th><th>Type</th></tr>"

#define WEB_TABLELIST_TR "<tr onmouseover=\"ChangeColor(this, true);\" \
                            onmouseout=\"ChangeColor(this, false);\" \
                            onclick=\"DoNav('http://www.vscp.org/')\" >"

#define WEB_TABLELIST_TD_CENTERED "<td id=\"tdcenter\">"




// * * * Table value List * * *

// Place after menus
#define WEB_TABLEVALUELIST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                        <h1 id=\"header\">List of values - \
                                        %s</h1></div><table><tbody>"

// Place before common end
#define WEB_TABLEVALUELIST_TABLE_END "</tbody></table>"

// Rows
#define WEB_TABLEVALUELIST_TR_HEAD "<tr><th>Record</th><th>Date</th><th>\
                                    Value</th></tr>"

#define WEB_TABLEVALUELIST_TR "<tr onmouseover=\"ChangeColor(this, true);\" \
                                onmouseout=\"ChangeColor(this, false);\" \
                                onclick=\"DoNav('http://www.vscp.org/')\" >"

#define WEB_TABLEVALUELIST_TD_CENTERED "<td id=\"tdcenter\">"

#define WEB_TABLEVALUELIST_TD_GUID "<td id=\"tdguid\">"

// Tablevalue navigation - 
// Action url           - string
// First shown record   - integer
// Last shown record    - integer
// total records	        - integer
// count to show        - integer
#define WEB_TABLEVALUE_LIST_NAVIGATION "<br><div style=\"text-align:center\">\
                                        <form method=\"get\" action=\"%s\" \
                                        name=\"PageSelect\"><button type=\"submit\" \
                                        value=\"first\" name=\"navbtn\">&lt;&lt;\
                                        </button><button type=\"submit\" \
                                        value=\"previous\" name=\"navbtn\">&lt;\
                                        </button><button type=\"submit\" \
                                        value=\"next\" name=\"navbtn\">&gt;\
                                        </button><button type=\"submit\" \
                                        value=\"last\" name=\"navbtn\">&gt;&gt;\
                                        </button><span style=\"font-weight:bold\"> \
                                        %d - %d</span> of <span \
                                        style=\"font-weight:bold\">%d</span> Show: \
                                        <input name=\"count\" value=\"%d\" size=\"3\">\
                                        <input name=\"from\" value=\"%d\" \
                                        type=\"hidden\"><input name=\"tblname\" \
                                        value=\"%s\" type=\"hidden\"><br></form></div> "




#endif
