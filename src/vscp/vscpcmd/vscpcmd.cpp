// vscpcmd.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
// Copyright © 2000-2022 Ake Hedman,
// Ake Hedman, the VSCP project,<info@vscp.org>
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
//      event data on the form
//      head,class,type,obid,timestamp,GUID,data1,data2,data3....
//
//      -m/--measurement
//      ================
//      Set this event as a measurement event. Argument can be given as
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

#include <string>

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vscpcmd.h"
#include <canal_macro.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

// Prototypes
void
setDataFromValue(uint8_t encoding,
                 uint8_t type,
                 std::string &strval,
                 uint8_t *msgdata,
                 uint16_t *pnDataCnt);
uint8_t
setNormalizedValue(std::string &strval, uint8_t *msgdata, uint8_t type);

#define HELP_STR                                                               \
    "vscpcmd - VSCP general command line commander.\n Version: "               \
    "%d.%d.%d.%d\n\n"                                                          \
    "-q/--host         VSCP server on the form user:password@host or just "    \
    "host.\n"                                                                  \
    "-u/--user         VSCP server username used to connect to it.\n"          \
    "-p/--password     VSCP server password used to connect to it.\n"          \
    "-e/--event        Event to send. Event data on the form "                 \
    "head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....\n"     \
    "-m/--measurement  A VSCP measurement value. Set this event as a "         \
    "measurement event. Argument can be given as three bytes as "              \
    "format,intrepretion,index.\n"                                             \
    "-n/--value        A VSCP value.\n"                                        \
    "-c/--count        # of events to send or receive.\n"                      \
    "-z/--zone         zone to use for event.\n"                               \
    "-s/--subzone      subzone to use for event.\n"                            \
    "-i/--index        index to use for event.\n"                              \
    "-v/--verbose      Turn on verbose mode.\n"                                \
    "-t/--test         Test (optional test number as argument).\n"             \
    "-h/--help         Show this help.\n"

int
main(int argc, char **argv)
{
    bool bVerbose           = false;
    bool bTestMode          = false;
    bool bHostParam         = false;    // Set to true if host parameter given
    bool bEventParam        = false;    // Set to true if event parameter given
    int cntSend             = 1;        // One event is default
    uint8_t opt_zone        = 1;        // Zone
    uint8_t opt_subzone     = 1;        // Subzone
    uint8_t opt_nTest       = 0;        // Default test sequency
    std::string strUsername = "admin";  // Default user
    std::string strPassword = "secret"; // Default password
    std::string strHost     = "localhost"; // The local machine

    // VSCP tcp/ip interface
    VscpRemoteTcpIf m_vscpif;

    // Event
    vscpEventEx event;

    int c;
    //int digit_optind = 0;

    while (1) {

        //int this_option_optind = optind ? optind : 1;
        int option_index       = 0;

        static struct option long_options[] = {
            { "host", required_argument, 0, 'q' },
            { "event", required_argument, 0, 'e' },
            { "user", required_argument, 0, 'u' },
            { "password", required_argument, 0, 'p' },
            { "measurement", required_argument, 0, 'm' },
            { "value", required_argument, 0, 'n' },
            { "count", required_argument, 0, 'c' },
            { "zone", required_argument, 0, 'z' },
            { "subzone", required_argument, 0, 's' },
            { "index", required_argument, 0, 'i' },
            { "verbose", no_argument, 0, 'v' },
            { "test", optional_argument, 0, 't' },
            { "help", no_argument, 0, 'h' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long(
          argc, argv, "c:e:hi:m:n:p:q:s:t:u:vz:", long_options, &option_index);

        if (-1 == c) {
            break;
        }

        switch (c) {

            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg) printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'q': // Can be on the form "user:password@host" or just "host"
                if (optarg) {
                    size_t pos;
                    strHost    = optarg;
                    bHostParam = true;
                    if (std::string::npos != (pos = strHost.find('@'))) {
                        // Expect "username:password@host"
                        std::string str = vscp_str_left(strHost, pos);
                        strHost = vscp_str_right(strHost, strHost.length() - pos);
                        if (std::string::npos != (pos = str.find(':'))) {
                            strUsername = vscp_str_left(str, pos);
                            strPassword = vscp_str_right( str, str.length() - pos);
                        } else {
                            strUsername = str;
                        }
                    }

                } else {
                    fprintf(stderr,
                            "Argument -q/--host should have an argument "
                            "[user[:pass]@]host.\n");
                    exit(-1);
                }
                break;

            case 'u': // Username used to connect to host
                if (optarg) {
                    strUsername = optarg;
                } else {
                    fprintf(stderr,
                            " A username should be given after -u/--user.");
                    exit(-1);
                }
                break;

            case 'p': // Password used to connect to host
                if (optarg) {
                    strPassword = optarg;
                } else {
                    fprintf(stderr,
                            " A password should be given after -p/--password.");
                    exit(-1);
                }
                break;

            case 'e': // Event to send
                if (optarg) {
                    // Flag event parameter
                    bEventParam     = true;
                    std::string str = optarg;
                    vscp_convertStringToEventEx(&event, str);
                } else {
                    fprintf(stderr,
                            "Argument -e/--event should have an argument.\n");
                    exit(-1);
                }
                break;

            case 'm': // measurement to send
                break;

            case 'n': // Value to send
            {
                uint8_t encoding;
                uint8_t type;
                std::string strEncoding, strType, strValue, wrkstr;
                std::string str = optarg;

                if (NULL == optarg) {
                    fprintf(
                      stderr,
                      "Must specify a value to send if using -v/--value. "
                      "Format for option argument is 'encoding,type,value'\n");
                    exit(-1);
                }

                std::deque<std::string> tokens;
                vscp_split(tokens, str, ",");

                if (tokens.empty()) {
                    fprintf(
                      stderr,
                      "Must specify an encoding if using -v/--value. "
                      "Format for option argument is 'encoding,type,value'\n");
                    exit(-1);
                }

                // Get encoding
                wrkstr = tokens.front();
                tokens.pop_front();
                encoding = (uint8_t)vscp_readStringValue(wrkstr);

                if (tokens.empty()) {
                    fprintf(
                      stderr,
                      "Must specify a type if using -v/--value. "
                      "Format for option argument is 'encoding,type,value'\n");
                    exit(-1);
                }

                // Get type
                wrkstr = tokens.front();
                tokens.pop_front();
                type = vscp_readStringValue(wrkstr);

                if (tokens.empty()) {
                    fprintf(
                      stderr,
                      "Must specify a value if using -v/--value. "
                      "Format for option argument is 'encoding,type,value'\n");
                    exit(-1);
                }

                // Get value
                wrkstr = tokens.front();
                tokens.pop_front();

                uint8_t msgdata[512];
                uint16_t nDataCnt;
                setDataFromValue(encoding, type, str, msgdata, &nDataCnt);
            } break;

        case 'c':    // Number of events to send
            if (optarg) {
                cntSend = vscp_readStringValue(optarg);
                if (bVerbose) {
                    printf("Count set to %d\n", cntSend );
                }
            }
            else {
                fprintf( stderr, "argument -c/--count should have a value. Set = 1\n");
                cntSend = 1;
            }
            break;        

        case 'z':   // zone
            if (optarg) {
                opt_zone = vscp_readStringValue(optarg);
                if (bVerbose) {
                    printf("Zone set to %d\n", (int)opt_zone);
                }
            } else {
                fprintf(stderr,
                        "argument -z/--zone should have a value. Set = 1\n");
                opt_zone = 1;
            }
            break;

        case 's': // subzone
            if (optarg) {
                opt_subzone = vscp_readStringValue(optarg);
                if (bVerbose) {
                    printf("Subzone set to %d\n", (int)opt_subzone);
                }
            } else {
                fprintf(stderr,
                        "argument -s/--subzone should have a value. Set = 1\n");
                opt_zone = 1;
            }
            break;

        case 'v': // verbose
            bVerbose = true;
            printf("Verbose mode enabled.\n");
            break;

        case 't': // test
            if (optarg) {
                bTestMode = true;
                opt_nTest = vscp_readStringValue(optarg);
                if (bVerbose) {
                    printf("Test sequency set to %d\n", opt_nTest);
                }
            } else {
                fprintf(stderr, "No test sequency specified. Set to zero.\n");
                opt_nTest = 0;
            }
            break;

        case 'h': // help
            break;
        }

    } // while comand line parser

    // Terminate if required parameters are missing
    if (!bHostParam || !bEventParam) {
        fprintf(stderr, ("Missing required parameter(s).\n"));
        exit(-1);
    }

    if (bTestMode) {

        if (bVerbose) {
            std::string str;
            str = "================================================\n";
            str = " V S C P D  I N T E R F A C E  T E S T  M O D E \n";
            str = "================================================\n";
        }

    } else {

        // We should send an event to the world

        if (bVerbose) {
            printf("Connecting to host %s\n", strHost.c_str());
        }

        // Open the interface
        if (CANAL_ERROR_SUCCESS ==
            m_vscpif.doCmdOpen(strHost, strUsername, strPassword)) {

            if (bVerbose) {
                printf("Connection open to host %s\n", strHost.c_str());
            }

            for (int i = 0; i < cntSend; i++) {
                if (CANAL_ERROR_SUCCESS == m_vscpif.doCmdSendEx(&event)) {
                    if (bVerbose) {
                        printf("successfully sent one event to %s\n",
                               strHost.c_str());
                    }
                } else {
                    if (bVerbose) {
                        fprintf(stderr,
                                "Failed to send event to %s\n",
                                strHost.c_str());
                    }
                }
            }

            // Close the connection
            m_vscpif.doCmdClose();
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// setDataFromValue
// TODO: byte order

void
setDataFromValue(uint8_t encoding,
                 uint8_t type,
                 std::string &strval,
                 uint8_t *msgdata,
                 uint16_t *pnDataCnt)
{
    std::string str;

    msgdata[0] = type;

    switch (encoding) {

        case 0: // Bit format
        {
            int idx = 1;

            msgdata[0] |= VSCP_DATACODING_BIT;

            std::deque<std::string> tokens;
            vscp_split(tokens, strval, ",");

            while ((idx < 512) && !tokens.empty()) {

                std::string strbuf = tokens.front();
                tokens.pop_front();
                msgdata[idx] = vscp_readStringValue(strbuf);
                idx++;
            }

            *pnDataCnt = idx;

        } break;

        case 1: // Byte format
        {
            int idx = 1;

            msgdata[0] |= VSCP_DATACODING_BYTE;

            std::deque<std::string> tokens;
            vscp_split(tokens, strval, ",");

            while ((idx < 512) && !tokens.empty()) {

                std::string strbuf = tokens.front();
                tokens.pop_front();
                msgdata[idx] = vscp_readStringValue(strbuf);
                idx++;
            }

            *pnDataCnt = idx;
        } break;

        case 2: // String format
            msgdata[0] |= VSCP_DATACODING_STRING;
            strncpy((char *)(msgdata + 1), str.c_str(), 512 - 25 - 2);
            *pnDataCnt = strlen((char *)(msgdata + 1)) + 1;
            break;

        case 3: // Integer format
        {
            msgdata[0] |= VSCP_DATACODING_INTEGER;

            uint32_t intval = vscp_readStringValue(strval);
            if (intval > 0x00ffffff) {
                *pnDataCnt = 5;
                msgdata[1] = (intval >> 24) & 0xff;
                msgdata[2] = (intval >> 16) & 0xff;
                msgdata[3] = (intval >> 8) & 0xff;
                msgdata[4] = intval & 0xff;
            } else if (intval > 0x0000ffff) {
                *pnDataCnt = 4;
                msgdata[1] = (intval >> 16) & 0xff;
                msgdata[2] = intval >> 8;
                msgdata[3] = intval & 0xff;
            } else if (intval > 0x000000ff) {
                *pnDataCnt = 3;
                msgdata[1] = intval >> 8;
                msgdata[2] = intval & 0xff;
            } else {
                *pnDataCnt = 2;
                msgdata[1] = intval;
            }
        } break;

        case 4: // Normalized integer
            *pnDataCnt = setNormalizedValue(strval, msgdata, type);
            break;

        case 5: // Floating point format
        {
            double value;
            unsigned char *p;

            msgdata[0] |= VSCP_DATACODING_SINGLE;

            value = std::stod(strval);
            p     = (unsigned char *)&value;

            memcpy(msgdata + 1, p, sizeof(value));
        } break;

        case 6: // Reserved
        {
            std::string str;
            str = "Unsupported data interpretion [6] used.\n";
        } break;

        case 7: // Reserved
        {
            std::string str;
            str = "Unsupported data interpretion [7] used.\n";
        } break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// setDataFromValue
//
// returns count

uint8_t
setNormalizedValue(std::string &strval, uint8_t *msgdata, uint8_t type)
{
    uint8_t *pto;
    unsigned char pos   = 0;
    bool bNegative      = false;
    bool bDPFound       = false;
    uint8_t cntDecimals = 0;
    uint8_t cntNum      = 0;
    uint8_t size        = 0;
    uint32_t ivalue     = 0;
    uint8_t buf[12];

    memset(buf, 0, sizeof(buf));

    msgdata[0] = (type | VSCP_DATACODING_NORMALIZED);
    pto        = buf;

    while (strval[pos]) {

        switch (strval[pos]) {

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
                *pto = strval[pos];
                pto++;
                break;
        }

        pos++;

        // There is only room for ten numbers
        if (cntNum >= 10) break;

    } // while

    if (cntNum > 0) {

        ivalue = atol((const char *)buf);
        if (ivalue > 0x00ffffff) {
            size       = 6;
            msgdata[1] = cntDecimals;
            msgdata[2] = (ivalue >> 24) & 0xff;
            msgdata[3] = (ivalue >> 16) & 0xff;
            msgdata[4] = (ivalue >> 8) & 0xff;
            msgdata[5] = ivalue & 0xff;
        } else if (ivalue > 0x000000ffff) {
            size       = 5;
            msgdata[1] = cntDecimals;
            msgdata[2] = (ivalue >> 16) & 0xff;
            msgdata[3] = ivalue >> 8;
            msgdata[4] = ivalue & 0xff;
        } else if (ivalue > 0x000000ff) {
            size       = 4;
            msgdata[1] = cntDecimals;
            msgdata[2] = ivalue >> 8;
            msgdata[3] = ivalue & 0xff;
        } else {
            size       = 3;
            msgdata[1] = cntDecimals;
            msgdata[2] = ivalue;
        }

    } else {

        size       = 3;
        msgdata[1] = 0;
        msgdata[2] = 0;
    }

    if (bNegative) msgdata[1] |= 0x80;

    return size;
}
