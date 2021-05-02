// web_template.h
//
// This file is part of the VSCP (https://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright © 2000-2021 Ake Hedman, the VSCP project <info@grodansparadis.com>
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
                           "    </ul>"\
                           " </li>"\
                           " <li>"\
                           "    <a href=\"#\">Tools</a>"\
                           "    <ul>"\
                           "        <li>"\
                           "            <a href=\"/vscp/password\">Password generator</a>"\
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
                           "            <a href=\"/vscp/interfaces\">Interfaces</a>"\
                           "        </li>"\
                           "        <li>"\
                           "            <a href=\"/vscp/configure\">Configure</a>"\
                           "        </li>"\
                           "    </ul>"\
                           " </li>"\
                           " <li>"\
                           "    <a href=\"#\">Logs</a>"\
                           "    <ul>"\
                           "        <li>"\
                           "            <a href=\"/vscp/log\">View Log</a>"\
                           "        </li>"\
                           "    </ul>"\
                           " </li>"\
                           " <li>"\
                           "     <a href=\"#\">Help</a>"\
                           "     <ul>"\
                           "         <li>"\
                           "             <a href=\"https://docs.vscp.org/#vscpd\" target=\"moon\">VSCP Daemon</a>"\
                           "         </li>"\
                           "         <li>"\
                           "             <a href=\"https://docs.vscp.org/#vscpspec\" target=\"moon\">VSCP Specification</a>"\
                           "         </li>"\
                           "         <li>"\
                           "             <a href=\"https://docs.vscp.org/#vscpworks\" target=\"moon\">VSCP Works</a>"\
                           "         </li>"\
                           "         <li>"\
                           "             <a href=\"https://docs.vscp.org/#vscphelper\" target=\"moon\">VSCP Helper lib</a>"\
                           "         </li>"\
                           "         <li>"\
                           "             <a href=\"https://docs.vscp.org\" target=\"moon\">VSCP Docs Site</a>"\
                           "         </li>"\
                           "         <li>"\
                           "             <a href=\"https://www.vscp.org\" target=\"moon\">VSCP Site</a>"\
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



// * * * Logfile * * *

// Place after menus
#define WEB_LOGLIST_BODY_START "<br><div id=\"content\"><div id=\"header\">\
                                <h1 id=\"header\">Log file</h1></div>\
                                <table><tbody>"

// Table head
#define WEB_LOGLIST_TR_HEAD "<tr><th>Line</th></tr>"


// Place before common end
#define WEB_LOGLIST_TABLE_END "</tbody></table>"







#endif
