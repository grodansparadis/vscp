// vscpcmd.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB,<akhe@grodansparadis.com>
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
//
//
// Command line switches
// =====================
//      -e/--event 
//      ==========
//      event data on the form head,class,type,obid,timestamp,GUID,data1,data2,data3....
//
//      -m/--measurement
//      ================
//      Set this event as a measurement event. Argumen can be given as
//      three bytes as format,intrepretion,index
//      format
//      ======
//      0 - Bit format
//      1 or 0x20 - Byte format
//      2 or 0x40 - String format.
//      3 or 0x60 - Integer format.
//      4 or 0x80 - Normalized integer format
//      interpretion
//      ============
//      0-3 Measurement interpretion for example Kevin Celsius, Fahrenheit..
//      Defaults to zero.
//      index
//      =====
//      0-7 Sensor index. Defaults to zero.
//
//      -v/--value
//      ==========
//      Value to for a measuement or other data. A string should have 
//      enlosing ". Decimal numbers use a point "." as decimal mark.
//
//      -h/--host
//      =========
//      host    user:password@host or just host
//
//      -u/--user
//      =========
//      username
//
//      -p/--password
//      =============
//      password
//
// 		-v/--verbose 
//		============
//      Set verbose mode.
//

#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include "wx/cmdline.h"
#include "wx/tokenzr.h"
#include <wx/version.h> 
#include <math.h>


#ifdef WIN32
//#include "../common/controlobject.h"
#else

#ifndef WIN32
#include <unistd.h>
#endif
#include <wchar.h>
#endif


#include "../common/canal_macro.h"
#include "vscpcmd.h"
#include "../common/vscptcpif.h"
#include "../common/vscphelper.h"

// Prototypes
void setDataFromValue(uint8_t encoding, uint8_t type, wxString& wxstrval, uint8_t *msgdata, uint16_t *pnDataCnt);
uint8_t setNormalizedValue(wxString& wxstrval, uint8_t *msgdata, uint8_t type);

static const wxCmdLineEntryDesc cmdLineDesc[] = {
    {
        wxCMD_LINE_OPTION,
        _("q"),
        _("host"),
        _("VSCP server on the form user:password@host or just host"),
        wxCMD_LINE_VAL_STRING,
        wxCMD_LINE_OPTION_MANDATORY
    },
    {
        wxCMD_LINE_OPTION,
        _("e"),
        _("event"),
        _("event data on the form head,class,type,obid,timestamp,GUID,data1,data2,data3...."),
        wxCMD_LINE_VAL_STRING,
        wxCMD_LINE_OPTION_MANDATORY
    },
    {
        wxCMD_LINE_OPTION,
        _("u"),
        _("user"),
        _("Username for VSCP server."),
        wxCMD_LINE_VAL_STRING,
        wxCMD_LINE_PARAM_OPTIONAL
    },
    {
        wxCMD_LINE_OPTION,
        _("p"),
        _("password"),
        _("Password for VSCP server."),
        wxCMD_LINE_VAL_STRING,
        wxCMD_LINE_PARAM_OPTIONAL
    },
    {
        wxCMD_LINE_OPTION,
        _("m"),
        _("measurement"),
        _("Set this event as a measurement event. Argument can be given as\n three bytes as format,intrepretion,index"),
        wxCMD_LINE_VAL_STRING,
        wxCMD_LINE_PARAM_OPTIONAL
    },
    {
        wxCMD_LINE_OPTION,
        _("y"),
        _("value"),
        _("VSCP value"),
        wxCMD_LINE_VAL_STRING,
        wxCMD_LINE_PARAM_OPTIONAL
    },
    {
        wxCMD_LINE_OPTION,
        _("n"),
        _("count"),
        _("# of messages to send or receive."),
        wxCMD_LINE_VAL_NUMBER,
        wxCMD_LINE_PARAM_OPTIONAL
    },
    {
        wxCMD_LINE_OPTION,
        _("z"),
        _("zone"),
        _("Zone for event."),
        wxCMD_LINE_VAL_NUMBER,
        wxCMD_LINE_PARAM_OPTIONAL
    },
    {
        wxCMD_LINE_OPTION,
        _("s"),
        _("subzone"),
        _("Subzone for event."),
        wxCMD_LINE_VAL_NUMBER,
        wxCMD_LINE_PARAM_OPTIONAL
    },
    { wxCMD_LINE_SWITCH, _("v"), _("verbose"), _("Vebose mode"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL},
    { wxCMD_LINE_SWITCH, _("t"), _("test"), _("Interface test mode (for vscpd developers only)."), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL},
    { wxCMD_LINE_SWITCH, _("h"), _("help"), _("Shows this message"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_OPTION_HELP},
    { wxCMD_LINE_NONE}
};

int main(int argc, char **argv)
{
    bool bVerbose = false;
    bool bTestMode = false;
    bool bHostParam = false; // Set to true if host parameter given
    bool bEventParam = false; // Set to true if event parameter given
    int cntSend = 1; // One event is default
    wxString strUsername = _("admin"); // Default user
    wxString strPassword = _("secret"); // Default password
    wxString strHost = _("localhost"); // The local machine

    // VSCP tcp/ip interface
    VscpTcpIf m_vscpif;

    // Event 
    vscpEventEx event;

#if wxUSE_UNICODE
    wxChar **wxArgv = new wxChar *[argc + 1];

    {
        int n;

        for (n = 0; n < argc; n++) {
            wxMB2WXbuf warg = wxConvertMB2WX(argv[n]);
            wxArgv[ n ] = wxStrdup(warg);
        }

        wxArgv[n] = NULL;
    }

#else // !wxUSE_UNICODE
#define wxArgv argv
#endif // wxUSE_UNICODE/!wxUSE_UNICODE

    wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");

    //wxInitializer initializer;
    if (!::wxInitialize()) {
        fprintf(stderr, "Failed to initialize the wxWindows library, aborting.");
        return -1;
    }

    wxCmdLineParser *pparser = new wxCmdLineParser(cmdLineDesc, argc, argv);

    if (NULL != pparser) {

        wxString wxstr;
        if (pparser->Parse(true) > 0) {
            //pparser->Usage();
            /*
            printf("\n\nUsage for vscpcmd\n");
            printf("---------------------\n");
            char idx = 0;
            while (cmdLineDesc[idx].kind != wxCMD_LINE_NONE) {
                wxPrintf(_("%s or %s \t %s \n"),
                        cmdLineDesc[idx].shortName,
                        cmdLineDesc[idx].longName,
                        cmdLineDesc[idx].description);
                idx++;
            }*/
        }

        long opt;

        // * * * Verbose * * *
        if (pparser->Found(wxT("verbose"))) {
            bVerbose = true;
            {
                wxString wxstr;
                wxstr.Printf(wxT("Verbose mode set\n"));
            }
        }

        // * * * Count * * *
        if (pparser->Found(_T("count"), &opt)) {
            cntSend = (int) opt;
            if (bVerbose) {
                wxString wxstr;
                wxstr.Printf(_("Count set to %d\n"), cntSend);
            }
        }

        // * * * Test * * *
        if (pparser->Found(_T("test"))) {
            bTestMode = true;
            if (bVerbose) {
                wxString wxstr;
                wxstr.Printf(_("Testmode set\n"));
            }
        }

        // * * * Username * * *
        if (pparser->Found(_T("user"), &wxstr)) {
            strUsername = wxstr;
        }

        // * * * Password * * *
        if (pparser->Found(_T("password"), &wxstr)) {
            strPassword = wxstr;
        }

        // * * * Host * * *
        if (pparser->Found(_T("host"), &wxstr)) {

            // Flag host parameter
            bHostParam = true;

            // Can be on the form "user:password@host" or just "host"

            wxStringTokenizer tok;
            tok.SetString(wxstr, wxT("@"));

            if (tok.CountTokens() > 1) {

                // Get username/password
                wxString str = tok.GetNextToken();

                // get host
                strHost = tok.GetNextToken();

                wxStringTokenizer tok2;
                tok2.SetString(str, wxT(":"));

                // Get username
                strUsername = tok2.GetNextToken();

                if (tok2.HasMoreTokens()) {
                    // Get password
                    strPassword = tok2.GetNextToken();
                }

            } else {
                // Get host
                strHost = tok.GetNextToken();
            }
        }

        // * * * Event * * *
        if (pparser->Found(_T("event"), &wxstr)) {

            // Flag event parameter
            bEventParam = true;

            getVscpEventExFromString(&event, wxstr);
        }

        // * * * Value * * *
        if (pparser->Found(_T("value"), &wxstr)) {

            uint8_t encoding;
            uint8_t type;
            wxString wxstrEncoding, wxstrType, wxstrValue, wxstr;
            wxStringTokenizer tok;

            tok.SetString(wxstr, wxT(",\r\n"));

            // Get encoding
            wxstr = tok.GetNextToken();
            encoding = (uint8_t) readStringValue(wxstr);
            if (!tok.HasMoreTokens()) {
                wxString wxstr;
                wxstr.Printf(_("Invalid format for value. Should be {encoding,type,value}\n"));
            }

            // Get type
            wxstr = tok.GetNextToken();
            type = readStringValue(wxstr);
            if (!tok.HasMoreTokens()) {
                wxString wxstr;
                wxstr.Printf(_("Invalid format for value. Should be {encoding,type,value}\n"));
            }

            // Get value
            wxstr = tok.GetNextToken();

            uint8_t msgdata[512];
            uint16_t nDataCnt;
            setDataFromValue( encoding, type, wxstr, msgdata, &nDataCnt );
        }


    } else {
        //fprintf( stderr, _("Failed to initialize parser class.") );
        return -1;
    }

    delete pparser;

    // Terminate if required parameters are missing
    if (!bHostParam || !bEventParam) {
        //fprintf( stderr, _("Missing required parameter.") );
        exit(-1);
    }


    if (bTestMode) {

        if (bVerbose) {
            wxString wxstr;
            wxstr.Printf(_("================================================\n"));
            wxstr.Printf(_(" V S C P D  I N T E R F A C E  T E S T  M O D E \n"));
            wxstr.Printf(_("================================================\n"));
        }

    } else {

        // We should send an event to the world

        if (bVerbose) {
            wxString wxstr =
                    _("Connecting to host ") + strHost + _("\r\n");
        }

        // Open the interface
        if (m_vscpif.doCmdOpen(strHost, VSCP_LEVEL2_TCP_PORT, strUsername, strPassword)) {

            if (bVerbose) {
                wxString wxstr =
                        _("Connection open to host ") + strHost + _("\r\n");
            }


            for (int i = 0; i < cntSend; i++) {
                if (CANAL_ERROR_SUCCESS == m_vscpif.doCmdSendEx(&event)) {
                    if (bVerbose) {
                        wxString wxstr =
                                _("successfully sent one event ") + strHost + _("\r\n");
                    }
                } else {
                    if (bVerbose) {
                        wxString wxstr =
                                _("Failed to send event ") + strHost + _("\r\n");
                    }
                }
            }


            // Close the connection
            m_vscpif.doCmdClose();
        }

    }

    ::wxUninitialize();
    return 0;

}

///////////////////////////////////////////////////////////////////////////////
// setDataFromValue
// TODO: byte order

void setDataFromValue(uint8_t encoding,
        uint8_t type,
        wxString& wxstrval,
        uint8_t *msgdata,
        uint16_t *pnDataCnt)
{
    wxString wxstr;

    msgdata[ 0 ] = type;

    switch (encoding) {

    case 0: // Bit format
    {
        int idx = 1;
        wxStringTokenizer tok;

        msgdata[ 0 ] |= VSCP_DATACODING_BIT;

        tok.SetString(wxstr, wxT(",\r\n"));

        while ((idx < (512 - 25)) && tok.HasMoreTokens()) {

            wxString wxstrbuf = tok.GetNextToken();
            msgdata[ idx ] = readStringValue(wxstrbuf);
            idx++;

        }

        *pnDataCnt = idx;
    }
        break;

    case 1: // Byte format
    {
        int idx = 1;
        wxStringTokenizer tok;

        msgdata[ 0 ] |= VSCP_DATACODING_BYTE;

        tok.SetString(wxstr, wxT(",\r\n"));

        while ((idx < (512 - 25)) && tok.HasMoreTokens()) {

            wxString wxstrbuf = tok.GetNextToken();
            msgdata[ idx ] = readStringValue(wxstrbuf);
            idx++;

        }

        *pnDataCnt = idx;
    }
        break;

    case 2: // String format
        msgdata[ 0 ] |= VSCP_DATACODING_STRING;
        strncpy((char *) (msgdata + 1), wxstr.mb_str(), 512 - 25 - 2);
        *pnDataCnt = strlen((char *) (msgdata + 1)) + 1;
        break;

    case 3: // Integer format
    {
        msgdata[ 0 ] |= VSCP_DATACODING_INTEGER;

        uint32_t intval = readStringValue(wxstrval);
        if (intval > 0x00ffffff) {
            *pnDataCnt = 5;
            msgdata[ 1 ] = (intval >> 24) & 0xff;
            msgdata[ 2 ] = (intval >> 16) & 0xff;
            msgdata[ 3 ] = (intval >> 8) & 0xff;
            msgdata[ 4 ] = intval & 0xff;
        } else if (intval > 0x0000ffff) {
            *pnDataCnt = 4;
            msgdata[ 1 ] = (intval >> 16) & 0xff;
            msgdata[ 2 ] = intval >> 8;
            msgdata[ 3 ] = intval & 0xff;
        } else if (intval > 0x000000ff) {
            *pnDataCnt = 3;
            msgdata[ 1 ] = intval >> 8;
            msgdata[ 2 ] = intval & 0xff;
        } else {
            *pnDataCnt = 2;
            msgdata[ 1 ] = intval;
        }
    }
        break;

    case 4: // Normalized integer
        *pnDataCnt = setNormalizedValue(wxstrval, msgdata, type);
        break;

    case 5: // Floating point format
    {
        double value;
        unsigned char *p;

        msgdata[ 0 ] |= VSCP_DATACODING_SINGLE;

        wxstrval.ToDouble(&value);
        p = (unsigned char *) &value;

        memcpy(msgdata + 1, p, sizeof( value));
    }
        break;

    case 6: // Reserved
    {
        wxString wxstr;
        wxstr.Printf(_("Unsupported data interpretion [6] used.\n"));
    }
        break;

    case 7: // Reserved
    {
        wxString wxstr;
        wxstr.Printf(_("Unsupported data interpretion [7] used.\n"));
    }
        break;

    }
}

///////////////////////////////////////////////////////////////////////////////
// setDataFromValue
//
// returns count

uint8_t setNormalizedValue(wxString& wxstrval, 
                            uint8_t *msgdata, uint8_t type)
{
    uint8_t *pto;
    unsigned char pos = 0;
    bool bNegative = false;
    bool bDPFound = false;
    uint8_t cntDecimals = 0;
    uint8_t cntNum = 0;
    uint8_t size = 0;
    uint32_t ivalue = 0;
    uint8_t buf[ 12 ];

    memset(buf, 0, sizeof( buf));

    msgdata[ 0 ] = (type | VSCP_DATACODING_NORMALIZED);
    pto = buf;

#if wxCHECK_VERSION(3,0,0)
    while (wxstrval[pos].GetValue()) {
#else
    while (wxstrval.GetChar(pos)) {    
#endif        

#if wxCHECK_VERSION(3,0,0)
        switch ( wxstrval[pos].GetValue() ) {
#else
        switch ( wxstrval.GetChar(pos) ) {
#endif 

        case '+':
            bNegative = false;
            break;

        case '-':
            bNegative = true;
            break;

        case ',':
        case '.':
            bDPFound = true;
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            cntNum++;
            if (bDPFound) cntDecimals++;
            *pto = wxstrval.GetChar(pos);
            pto++;
            break;

        }

        pos++;

        // There is only room for ten numbers
        if (cntNum >= 10) break;

    } // while


    if (cntNum > 0) {

        ivalue = atol((const char *) buf);
        if (ivalue > 0x00ffffff) {
            size = 6;
            msgdata[ 1 ] = cntDecimals;
            msgdata[ 2 ] = (ivalue >> 24) & 0xff;
            msgdata[ 3 ] = (ivalue >> 16) & 0xff;
            msgdata[ 4 ] = (ivalue >> 8) & 0xff;
            msgdata[ 5 ] = ivalue & 0xff;
        } else if (ivalue > 0x000000ffff) {
            size = 5;
            msgdata[ 1 ] = cntDecimals;
            msgdata[ 2 ] = (ivalue >> 16) & 0xff;
            msgdata[ 3 ] = ivalue >> 8;
            msgdata[ 4 ] = ivalue & 0xff;
        } else if (ivalue > 0x000000ff) {
            size = 4;
            msgdata[ 1 ] = cntDecimals;
            msgdata[ 2 ] = ivalue >> 8;
            msgdata[ 3 ] = ivalue & 0xff;
        } else {
            size = 3;
            msgdata[ 1 ] = cntDecimals;
            msgdata[ 2 ] = ivalue;
        }

    } else {

        size = 3;
        msgdata[ 1 ] = 0;
        msgdata[ 2 ] = 0;

    }

    if (bNegative) msgdata[ 1 ] |= 0x80;

    return size;
}
