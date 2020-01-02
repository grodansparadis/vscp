// handleStockVariable.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2020 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <fstream>
#include <iostream>
#include <list>
#include <regex>
#include <string>

#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <syslog.h>
#include <unistd.h>

#include <duktape.h>
#include <lua.h>
#include <mongoose.h> // For copyright info

#include <expat.h>
#include <json.hpp> // Needs C++11  -std=c++11

#include <controlobject.h>
#include <guid.h>
#include <variablecodes.h>
#include <version.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscpbase64.h>
#include <vscpdatetime.h>
#include <vscpdb.h>
#include <vscphelper.h>

#include "remotevariable.h"
#include "stockvariable.h"

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

// The global control object
extern CControlObject *gpobj;

bool
CVariableStorage::handleStockVariable(std::string name,
                                      int op,
                                      CVariable &var,
                                      CUserItem *pUser)
{
    std::string str;
    std::string strToken;
    std::string strrest;
    uint32_t var_id = 0;

    var.init();
    var.setStockVariable();

    std::string lcname;
    vscp_lower(lcname);

    std::deque<std::string> tokens;
    vscp_split(tokens, lcname, ".");

    if (tokens.empty()) return false;
    strToken = tokens.front();
    vscp_trim(strToken);
    tokens.pop_front();

    // Make sure it starts with ".vscp"
    if ("vscp" != strToken) return false;

    // Set read defaults
    if (STOCKVAR_INIT == op) {
        // Set common values
        var.init(); // Owner admin,non-persistent,non-stock
        var.setName(name);
        var.setID(ID_VAR_DYNAMIC);
        var.setStockVariable(true);
        var.setAccessRights(PERMISSION_ALL_READ | PERMISSION_OWNER_WRITE);
        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
        var.setValue("");
        var.setNote("");
        var.setLastChangedToNow();
    } else if (STOCKVAR_READ == op) {
        // Try to read the variable from stock storage
        var_id = findNonPersistentVariable(name, pUser, var);
        // Will be zero for a dynamic stock variable
        if (0 == var_id) {
            var.setID(ID_VAR_DYNAMIC);
        }
    } else if (STOCKVAR_WRITE == op) {
        // Are we allowed to write the variable?
    }

    // Must be at least one other token
    if (tokens.empty()) return false;

    strToken = tokens.front();
    vscp_trim(strToken);
    tokens.pop_front();

    // *************************************************************************
    //                          versions & copyrights
    // *************************************************************************

    if ("version" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();

        if ("major" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.version.major");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue(VSCPD_MAJOR_VERSION);
                var.setNote("VSCP & Friends major version.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("minor" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.version.minor");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue(VSCPD_MINOR_VERSION);
                var.setNote("VSCP & Friends minod version.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("release" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.version.release");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue(VSCPD_RELEASE_VERSION);
                var.setNote("VSCP & Friends release version.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("build" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.version.build");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue(VSCPD_BUILD_VERSION);
                var.setNote("VSCP & Friends build version.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("string" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.version.string");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(VSCPD_DISPLAY_VERSION);
                var.setNote("VSCP & Friends version in string form.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("copyright" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.version.copyright");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(VSCPD_COPYRIGHT);
                var.setNote("VSCP & Friends copyright.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("sqlite3" == strToken) {

            // Must be at least one other token
            if (tokens.empty()) return false;

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            if ("string" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.sqlite3.string");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(SQLITE_VERSION);
                    var.setNote("Sqlite3 version in string form.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("copyright" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.sqlite3.copyright");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue("SQLite Is Public Domain");
                    var.setNote("VSCP & Friends copyright.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else {
                syslog(
                  LOG_ERR, "Remote variable %s is not available", name.c_str());
                return false;
            }

        } else if ("openssl" == strToken) {

            // Must be at least one other token
            if (tokens.empty()) return false;

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            if ("string" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.openssl.string");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    // http://linux.die.net/man/3/ssleay
                    // var.setValue( ("OPENSSL_VERSION_NUMBER") );
                    var.setValue(
                      vscp_str_format("%s", SSLeay_version(SSLEAY_VERSION)));
                    var.setNote("Openssl version in string form.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("copyright" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.openssl.copyright");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(
                      "Copyright © 1999-2019, OpenSSL Software Foundation.");
                    var.setNote("Openssl copyright.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else {
                syslog(
                  LOG_ERR, "Remote variable %s is not available", name.c_str());
                return false;
            }

        } else if ("duktape" == strToken) {

            // Must be at least one other token
            if (tokens.empty()) return false;

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            if ("string" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.duktape.string");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(DUK_GIT_DESCRIBE);
                    var.setNote("Duktape version in string form.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("copyright" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.duktape.copyright");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue("Duktape copyrights are held by its authors. "
                                 "(https://github.com/grodansparadis/vscp/blob/"
                                 "master/src/common/duktape.h)");
                    var.setNote("Duktape copyright.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else {
                syslog(
                  LOG_ERR, "Remote variable %s is not available", name.c_str());
                return false;
            }

        } else if ("civetweb" == strToken) {

            // Must be at least one other token
            if (tokens.empty()) return false;

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            if ("string" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.civitweb.string");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue("Code derived from 1.10");
                    var.setNote("Civetweb version in string form.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("copyright" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.civetweb.copyright");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(
                      "Copyright (c) 2004-2013 Sergey Lyubka, Copyright (c) "
                      "2013-2018 the Civetweb developers, (modifications.) "
                      "Copyright (c) 2017-2019 "
                      "Ake Hedman, Grodans Paradis AB");
                    var.setNote("Civitweb copyright.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else {
                syslog(
                  LOG_ERR, "Remote variable %s is not available", name.c_str());
                return false;
            }

        } else if ("lua" == strToken) {

            // Must be at least one other token
            if (tokens.empty()) return false;

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            if ("string" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.lua.string");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(LUA_COPYRIGHT);
                    var.setNote("Lua version in string form.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("copyright" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.lua.copyright");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(LUA_RELEASE);
                    var.setNote("Lua copyright.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else {
                syslog(
                  LOG_ERR, "Remote variable %s is not available", name.c_str());
                return false;
            }

        } else if ("mongoose" == strToken) {

            // Must be at least one other token
            if (tokens.empty()) return false;

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            if ("string" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.mongoose.string");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(MG_VERSION);
                    var.setNote("Mongoose version.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("copyright" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.mongoose.copyright");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(
                      "Copyright (c) 2004-2013 Sergey Lyubka, Copyright (c) "
                      "2013-2015 Cesanta Software Limited");
                    var.setNote("Mongoose copyright.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else {
                syslog(
                  LOG_ERR, "Remote variable %s is not available", name.c_str());
                return false;
            }

        } else if ("nlohmann-json" == strToken) {

            // Must be at least one other token
            if (tokens.empty()) return false;

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            if ("string" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.nlohmann-json.string");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue("version 3.5.0");
                    var.setNote("nlohmann-json version in string form.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("copyright" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.nlohmann-json.copyright");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue("Copyright (c) 2013-2018 Niels Lohmann "
                                 "<http://nlohmann.me>.");
                    var.setNote("nlohmann-json copyright.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            }
        } else if ("xml2json" == strToken) {

            // Must be at least one other token
            if (tokens.empty()) return false;

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            if ("string" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.xml2json.string");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue("version 2015");
                    var.setNote("xml2json version in string form.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("copyright" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.version.xml2json.copyright");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(
                      "Copyright (C) 2013-1015 Alan Zhuang (Cheedoong)	"
                      "Tencent, Inc, The MIT License (MIT)");
                    var.setNote("xml2json copyright.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else {
                syslog(
                  LOG_ERR, "Remote variable %s is not available", name.c_str());
                return false;
            }
        } else {
            syslog(LOG_ERR, "Remote variable %s is not avilable", name.c_str());
            return false;
        }
    }

    // *************************************************************************
    //                                  os
    // *************************************************************************

    if ("os" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();

        if ("width" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.os.width");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                int width = 8 * sizeof(size_t);
                var.setValue(width);
                var.setNote("os word width (compiler).");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("endian" == strToken) {

            // Must be at least one other token
            if (tokens.empty()) return false;

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            if ("big" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.os.endian.big");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                    var.setValue(vscp_isBigEndian());
                    var.setNote("True if bitorder is bigendian.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("little" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.os.endian.little");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                    var.setValue(vscp_isBigEndian());
                    var.setNote("True if bitorder is littleendian.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else {
                syslog(
                  LOG_ERR, "Remote variable %s is not available", name.c_str());
                return false;
            }
        } else if ("host" == strToken) {

            // Must be at least one other token
            if (tokens.empty()) return false;

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            if ("name" == strToken) {

                char buf[80];
                memset(buf, 0, sizeof(buf));
                if (gethostname(buf, sizeof(buf))) {
                    syslog(
                      LOG_ERR,
                      "Unable to get host name info for remote variable %s",
                      name.c_str());
                    return false;
                }
                std::string str = buf;

                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.os.host.name");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(str);
                    var.setNote("System hostname.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("domain" == strToken) {

                char buf[80];
                memset(buf, 0, sizeof(buf));
                if (getdomainname(buf, sizeof(buf))) {
                    syslog(
                      LOG_ERR,
                      "Unable to get domain name info for remote variable %s",
                      name.c_str());
                    return false;
                }
                std::string str = buf;

                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.os.host.domain");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(str);
                    var.setNote("System hostname.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("ip" == strToken) {

                cguid guid;

                if (!gpobj->getMacAddress(guid)) {
                    guid.clear();
                }

                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.os.host.ip");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(
                      vscp_str_format("%02X:%02X:%02X:%02X:%02X:%02X",
                                      guid.getAt(13),
                                      guid.getAt(12),
                                      guid.getAt(11),
                                      guid.getAt(10),
                                      guid.getAt(9),
                                      guid.getAt(8)));
                    var.setNote("System mac address.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else if ("mac" == strToken) {

                cguid guid;

                if (!gpobj->getIPAddress(guid)) {
                    guid.clear();
                }

                if (STOCKVAR_INIT == op) {
                    var.setName("vscp.os.host.mac");
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(vscp_str_format("%d.%d.%d.%d",
                                                 guid.getAt(11),
                                                 guid.getAt(10),
                                                 guid.getAt(9),
                                                 guid.getAt(8)));
                    var.setNote("System mac address.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    // Just read defined stock variable
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            } else {
                syslog(
                  LOG_ERR, "Remote variable %s is not available", name.c_str());
                return false;
            }
        } else if ("userid" == strToken) {

            uid_t uid         = geteuid();
            struct passwd *pw = getpwuid(uid);
            if (NULL == pw) {
                syslog(LOG_ERR,
                       "Unable to get userid for remote variable %s",
                       name.c_str());
                return false;
            }

            std::string str = std::string(pw->pw_name);

            if (STOCKVAR_INIT == op) {
                var.setName("vscp.os.host.userid");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(vscp_str_format("%uld", pw->pw_uid));
                var.setNote("Userid VSCP daemon is runing under.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("groupid" == strToken) {

            uid_t uid         = geteuid();
            struct passwd *pw = getpwuid(uid);
            if (NULL == pw) {
                syslog(LOG_ERR,
                       "Unable to get groupid for remote variable %s",
                       name.c_str());
                return false;
            }

            std::string str = std::string(pw->pw_name);

            if (STOCKVAR_INIT == op) {
                var.setName("vscp.os.host.groupid");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(vscp_str_format("%uld", pw->pw_gid));
                var.setNote("User group id VSCP daemon is runing under.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("username" == strToken) {

            uid_t uid         = geteuid();
            struct passwd *pw = getpwuid(uid);
            if (NULL == pw) {
                syslog(LOG_ERR,
                       "Unable to get username for remote variable %s",
                       name.c_str());
                return false;
            }

            std::string str = std::string(pw->pw_name);

            if (STOCKVAR_INIT == op) {
                var.setName("vscp.os.host.username");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(std::string(pw->pw_name));
                var.setNote("Username VSCP daemon is runing under.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("sysname" == strToken) {

            struct utsname unamebuf;
            if (0 != uname(&unamebuf)) {
                syslog(LOG_ERR,
                       "Unable to get uname info for remote variable %s",
                       name.c_str());
                return false;
            }
            std::string str = unamebuf.sysname;

            if (STOCKVAR_INIT == op) {
                var.setName("vscp.os.sysname");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(str);
                var.setNote("Operating system name (e.g., \"Linux\").");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("nodename" == strToken) {

            struct utsname unamebuf;
            if (0 != uname(&unamebuf)) {
                syslog(LOG_ERR,
                       "Unable to get uname info for remote variable %s",
                       name.c_str());
                return false;
            }
            std::string str = unamebuf.sysname;

            if (STOCKVAR_INIT == op) {
                var.setName("vscp.os.nodename");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(str);
                var.setNote(
                  "Name within 'some implementation-defined network'");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("release" == strToken) {

            struct utsname unamebuf;
            if (0 != uname(&unamebuf)) {
                syslog(LOG_ERR,
                       "Unable to get uname info for remote variable %s",
                       name.c_str());
                return false;
            }
            std::string str = unamebuf.release;

            if (STOCKVAR_INIT == op) {
                var.setName("vscp.os.release");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(str);
                var.setNote("Operating system release (e.g., '2.6.28').");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("version" == strToken) {

            struct utsname unamebuf;
            if (0 != uname(&unamebuf)) {
                syslog(LOG_ERR,
                       "Unable to get version info for remote variable %s",
                       name.c_str());
                return false;
            }
            std::string str = unamebuf.sysname;

            if (STOCKVAR_INIT == op) {
                var.setName("vscp.os.version");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(str);
                var.setNote("Operating system version.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("machine" == strToken) {

            struct utsname unamebuf;
            if (0 != uname(&unamebuf)) {
                syslog(LOG_ERR,
                       "Unable to get machine info for remote variable %s",
                       name.c_str());
                return false;
            }
            std::string str = unamebuf.sysname;

            if (STOCKVAR_INIT == op) {
                var.setName("vscp.os.machine");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(str);
                var.setNote("Hardware identifier.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else {
            syslog(
              LOG_ERR, "Remote variable %s is not available", name.c_str());
            return false;
        }
    }

    // *************************************************************************
    //                                daemon
    // *************************************************************************

    if ("daemon" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();

        if ("runasuser" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.daemon.runasuser");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(gpobj->m_runAsUser);
                var.setNote("VSCP daemon requested use to run as.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("guid" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.daemon.guid");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_GUID);
                var.setValue(gpobj->m_guid);
                var.setNote("GUID for VSCP daemon.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("name" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.daemon.name");
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(gpobj->m_strServerName);
                var.setNote("Server name for VSCP daemon.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("client-buffer-size" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.daemon.client-bugger-size");
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue((long)gpobj->m_maxItemsInClientReceiveQueue);
                var.setNote("Max client bugger size for VSCP daemon.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Just read defined stock variable
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be LONG to write remote variable %s",
                           name.c_str());
                    return false;
                }
                long value;
                var.getValue(&value);
                gpobj->m_maxItemsInClientReceiveQueue = (uint32_t)value;
                return true;
            }
        } else {
            syslog(
              LOG_ERR, "Remote variable %s is not available", name.c_str());
            return false;
        }
    }

    // *************************************************************************
    //                              Debug flags
    // *************************************************************************

    // Make sure it starts with ".vscp"
    if ("debug" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();

        // --------------------------------------------------------------------

        if ("level" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue(gpobj->m_logLevel);
                var.setNote("VSCP debug level.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Is user allowed to read?
                var.setValue((int)gpobj->m_logLevel);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be INTEGER to write remote variable %s",
                           name.c_str());
                    return false;
                }
                int value;
                var.getValue(&value);
                gpobj->m_logLevel = value;
                return true;
            }
        } else if ("flags1" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(0); // Dummy value
                var.setNote("VSCP debug flags 1.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Is user allowed to read?
                var.setValue((long)gpobj->m_debugFlags[0]);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be LONG to write remote variable %s",
                           name.c_str());
                    return false;
                }
                long value;
                var.getValue(&value);
                gpobj->m_debugFlags[0] = (uint32_t)value;
                return true;
            }
        } else if ("flags2" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(0); // Dummy value
                var.setNote("VSCP debug flags 2.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Is user allowed to read?
                var.setValue((long)gpobj->m_debugFlags[1]);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be LONG to write remote variable %s",
                           name.c_str());
                    return false;
                }
                long value;
                var.getValue(&value);
                gpobj->m_debugFlags[1] = (uint32_t)value;
                return true;
            }
        } else if ("flags3" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(0); // Dummy value
                var.setNote("VSCP debug flags 3.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Is user allowed to read?
                var.setValue((long)gpobj->m_debugFlags[2]);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be LONG to write remote variable %s",
                           name.c_str());
                    return false;
                }
                long value;
                var.getValue(&value);
                gpobj->m_debugFlags[2] = (uint32_t)value;
                return true;
            }
        } else if ("flags4" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(0); // Dummy value
                var.setNote("VSCP debug flags 4.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Is user allowed to read?
                var.setValue((long)gpobj->m_debugFlags[3]);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be LONG to write remote variable %s",
                           name.c_str());
                    return false;
                }
                long value;
                var.getValue(&value);
                gpobj->m_debugFlags[3] = (uint32_t)value;
                return true;
            }
        } else if ("flags5" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(0); // Dummy value
                var.setNote("VSCP debug flags 5.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Is user allowed to read?
                var.setValue((long)gpobj->m_debugFlags[4]);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be LONG to write remote variable %s",
                           name.c_str());
                    return false;
                }
                long value;
                var.getValue(&value);
                gpobj->m_debugFlags[4] = (uint32_t)value;
                return true;
            }
        } else if ("flags6" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(0); // Dummy value
                var.setNote("VSCP debug flags 6.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Is user allowed to read?
                var.setValue((long)gpobj->m_debugFlags[5]);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be LONG to write remote variable %s",
                           name.c_str());
                    return false;
                }
                long value;
                var.getValue(&value);
                gpobj->m_debugFlags[5] = (uint32_t)value;
                return true;
            }
        } else if ("flags7" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(0); // Dummy value
                var.setNote("VSCP debug flags .");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Is user allowed to read?
                var.setValue((long)gpobj->m_debugFlags[6]);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be LONG to write remote variable %s",
                           name.c_str());
                    return false;
                }
                long value;
                var.getValue(&value);
                gpobj->m_debugFlags[6] = (uint32_t)value;
                return true;
            }
        } else if ("flags8" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(0); // Dummy value
                var.setNote("VSCP debug flags 8.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                // Is user allowed to read?
                var.setValue((long)gpobj->m_debugFlags[7]);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be LONG to write remote variable %s",
                           name.c_str());
                    return false;
                }
                long value;
                var.getValue(&value);
                gpobj->m_debugFlags[7] = (uint32_t)value;
                return true;
            }
        } else {
            syslog(
              LOG_ERR, "Remote variable %s is not available", name.c_str());
            return false;
        }
    }

    // *************************************************************************
    //                                Configure
    // *************************************************************************
    // Keys comes from vscpdb.h

    if ("config" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();

        if (VSCPDB_CONFIG_NAME_DBVERSION == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote("VSCP configuration database version.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_DBVERSION, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                int n = vscp_readStringValue(buf);
                var.setValue(n);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if (VSCPDB_CONFIG_NAME_CLIENTBUFFERSIZE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote("Configuration: Client Buffer Size.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_CLIENTBUFFERSIZE, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                long n = vscp_readStringValue(buf);
                var.setValue(n);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be LONG to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_CLIENTBUFFERSIZE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                // Set current run value
                gpobj->m_maxItemsInClientReceiveQueue = atol(str.c_str());
                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_GUID == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_GUID);
                var.setNote("Configuration: VSCP Daemon GUID.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_GUID, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                cguid guid;
                guid.getFromString(buf);
                var.setValue(guid);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_GUID != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be GUID to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_GUID, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                // Set current run value
                gpobj->m_guid.getFromString(str);
                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_SERVERNAME == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: VSCP Daemon name.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_SERVERNAME, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_SERVERNAME, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                // Set current run value
                gpobj->m_strServerName = str;
                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_ANNOUNCE_ADDR == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: VSCP Daemon announce interface address.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_ANNOUNCE_ADDR, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_ANNOUNCE_ADDR, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_ANNOUNCE_TTL == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote(
                  "Configuration: VSCP Daemon announce interface ttl.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_ANNOUNCE_TTL, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_ANNOUNCE_TTL, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_ADDR == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip interface address.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_ADDR, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_ADDR, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_ENCRYPTION == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip interface encryption.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_ENCRYPTION, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_ENCRYPTION, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICATE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip SSL certificat.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICATE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICATE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICAT_CHAIN == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip SSL certificat chain.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICAT_CHAIN,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICAT_CHAIN,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_PEER == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip SSL verify peer.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_PEER,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_PEER, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_PATH == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: VSCP Daemon tcp/ip SSL ca path.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_PATH, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_PATH, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_FILE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: VSCP Daemon tcp/ip SSL ca file.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_FILE, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_FILE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_DEPTH == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip SSL verify depth.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_DEPTH,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_DEPTH, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_SSL_DEFAULT_VERIFY_PATHS ==
                   strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip #vSSL verify paths.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_DEFAULT_VERIFY_PATHS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_DEFAULT_VERIFY_PATHS,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_SSL_CHIPHER_LIST == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip SSL cipher list.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_CHIPHER_LIST,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_CHIPHER_LIST, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_SSL_PROTOCOL_VERSION == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip SSL protocol version.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_PROTOCOL_VERSION,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_PROTOCOL_VERSION,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_TCPIP_SSL_SHORT_TRUST == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip SSL protocol version.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_SHORT_TRUST,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_TCPIP_SSL_SHORT_TRUST, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_UDP_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: VSCP Daemon UDP enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_UDP_ENABLE, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_UDP_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_UDP_ADDR == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: VSCP Daemon tcp/ip UDP address (host:port).");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_UDP_ADDR, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_UDP_ADDR, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_UDP_USER == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: VSCP Daemon UDP pUser->");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_UDP_USER, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_UDP_USER, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_UDP_PASSWORD == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: VSCP Daemon UDP password.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_UDP_PASSWORD, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_UDP_PASSWORD, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_UDP_UNSECURE_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: VSCP Daemon UDP un-secure enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_UDP_UNSECURE_ENABLE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_UDP_UNSECURE_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_UDP_FILTER == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_FILTER);
                var.setNote("Configuration: VSCP Daemon UDP filter.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_UDP_FILTER, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_FILTER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                // vscp_readFilterMaskFromString
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_UDP_FILTER, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_UDP_GUID == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_GUID);
                var.setNote("Configuration: VSCP Daemon UDP GUID.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_UDP_GUID, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_GUID != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                // vscp_readFilterMaskFromString
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_UDP_GUID, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_UDP_ACK_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: VSCP Daemon UDP ack enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_UDP_ACK_ENABLE, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_UDP_ACK_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_MULTICAST_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: VSCP Daemon Multicast enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_MULTICAST_ENABLE, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_MULTICAST_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_DM_PATH_DB == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Path to decision matrix database.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_DM_PATH_DB, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_DM_PATH_DB, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_DM_PATH_XML == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Path to decision matrix XML file.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_DM_PATH_XML, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_DM_PATH_XML, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_DM_ALLOW_XML_SAVE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Allow XML save of decsion matrix.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_DM_ALLOW_XML_SAVE, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_DM_ALLOW_XML_SAVE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_VARIABLES_PATH_DB == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Path to variable database file.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_VARIABLES_PATH_DB, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_DM_PATH_XML, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_VARIABLES_PATH_XML == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Path to variable XML file.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_VARIABLES_PATH_XML,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_DM_PATH_XML, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_PATH_DB_EVENTS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: Path to VSCP events database file.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_PATH_DB_EVENTS, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_PATH_DB_EVENTS, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Enable web-server.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ENABLE, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Path to web-server document root.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server listening ports (comma "
                            "separated list).");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_INDEX_FILES == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server index files (comma "
                            "separated list).");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_INDEX_FILES, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_INDEX_FILES, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server authentication domain.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK ==
                   strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote(
                  "Configuration: Web-server enable auth domain check.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICATE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: Web-server SSL certificate path (.pem).");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICATE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICATE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server SSL certificate chain.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote(
                  "Configuration: Web-server SSL enable verify peer.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSL_CA_PATH == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server SSL path to "
                            "certification authority.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSL_CA_PATH, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSL_CA_PATH, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server SSL path to "
                            "certification authority file.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote("Configuration: Web-server SLL verify depth.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be integer to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS ==
                   strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote(
                  "Configuration: Web-server SSL default verify paths.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server SLL chipher list.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote("Configuration: Web-server SLL protocol version.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be integer to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote(
                  "Configuration: Web-server SSL enable short trust.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server cgi interpreter.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_CGI_PATTERN == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server cgi pattern.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_CGI_PATTERN, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_CGI_PATTERN, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server cgi environment.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_PROTECT_URI == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server protect uri.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_PROTECT_URI, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_PROTECT_URI, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_TROTTLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server trottle.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_TROTTLE, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_TROTTLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING ==
                   strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote(
                  "Configuration: Web-server enable directory listings.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Web-server enable keep alive.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_KEEP_ALIVE_TIMEOUT_MS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote("Configuration: Web-server keep alive timeout in "
                            "milliseconds.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_KEEP_ALIVE_TIMEOUT_MS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be long to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_KEEP_ALIVE_TIMEOUT_MS,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server access control list.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server extra mime types.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_NUM_THREADS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote(
                  "Configuration: Web-server max number of working threads.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_NUM_THREADS, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be integer to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_NUM_THREADS, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_RUN_AS_USER == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server run as pUser->");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_RUN_AS_USER, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_RUN_AS_USER, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server rewrite patterns.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server hide file patterns.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote(
                  "Configuration: Web-server request timeout in milliseconds.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be long to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote(
                  "Configuration: Web-server linger timeout in milliseconds.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be long to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_DECODE_URL == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Web-server enable url decode.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_DECODE_URL, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_DECODE_URL, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: Web-server global authentication file.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server per directory "
                            "authentication file.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server ssi patterns.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN ==
                   strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: Web-server access control allow origin.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS ==
                   strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: Web-server access control allow methods.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS ==
                   strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: Web-server access control allow headers.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server error pages.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote("Configuration: Web-server no delay.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be long to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote(
                  "Configuration: Web-server static file max age in seconds.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be long to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE ==
                   strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote("Configuration: Web-server strict transport "
                            "security max age in seconds.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be long to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ALLOW_SENDFILE_CALL == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Web-server allow sendfile call.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ALLOW_SENDFILE_CALL,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ALLOW_SENDFILE_CALL,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server additional headers.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote(
                  "Configuration: Web-server max request size in bytes.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be long to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE ==
                   strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote(
                  "Configuration: Web-server allow index script resource.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERN == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: Web-server Duktape script patterns.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERN,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERN,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: Web-server path to Lua preload file.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERN == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server Lua script pattern.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERN,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERN, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERN == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote(
                  "Configuration: Web-server Lua server page pattern.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERN,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote "
                           "variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to
                                  // write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERN,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database "
                           "for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server Lua "
                            "websocket pattern.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote "
                           "variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to
                                  // write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration "
                           "database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server Lua "
                            "background script.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write "
                           "remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e
                                  // able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write "
                           "remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration "
                           "database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS ==
                   strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-server Lua "
                            "background script params.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable "
                           "%s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write "
                           "remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e
                                  // able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write "
                           "remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration "
                           "database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEBSOCKET_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Web-sockets enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEBSOCKET_ENABLE, buf, sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be boolean to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.isTrue() ? "1" : "0";
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEBSOCKET_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEBSOCKET_DOCUMENT_ROOT == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setNote("Configuration: Web-sockets document root.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEBSOCKET_DOCUMENT_ROOT,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable "
                           "%s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write "
                           "remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e
                                  // able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be string to write "
                           "remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEBSOCKET_DOCUMENT_ROOT,
                      str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration "
                           "database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_WEBSOCKET_TIMEOUT_MS == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote(
                  "Configuration: Web-sockets timeout in milliseconds.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_WEBSOCKET_TIMEOUT_MS,
                      buf,
                      sizeof(buf))) {
                    syslog(LOG_ERR,
                           "Failed to read remote variable %s from "
                           "configuration database.",
                           name.c_str());
                    return false;
                }
                var.setValue(buf);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Only admin is allowed to write?
                if (USER_ID_ADMIN != pUser->getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be long to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_WEBSOCKET_TIMEOUT_MS, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                return true;
            }

        }

    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// initStockVariables
//

void
CVariableStorage::initStockVariables(CUserItem *pUser)
{
    CVariable var;

    // * * * version * * *
    handleStockVariable("vscp.version", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.version.major", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.version.minor", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.version.release", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.version.build", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.version.string", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.version.copyright", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.sqlite3.string", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.sqlite3.copyright", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.openssl.string", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.openssl.copyright", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.duktape.string", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.duktape.copyright", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.civetweb.string", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.civetweb.copyright", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.version.lua.string", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.lua.copyright", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.mongoose.string", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.mongoose.copyright", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.nlohmann-json.string", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.nlohmann-json.copyright", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.xml2json.string", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.version.xml2json.copyright", STOCKVAR_INIT, var, pUser);

    // * * * os * * *
    handleStockVariable("vscp.os.width", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.endian.big", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.endian.little", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.userid", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.groupid", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.username", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.sysname", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.nodename", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.release", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.version", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.machine", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.host.name", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.host.domain", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.host.ip", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.os.host.mac", STOCKVAR_INIT, var, pUser);

    // * * * daemon * * *
    handleStockVariable("vscp.daemon.runasuser", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.daemon.guid", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.daemon.name", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.daemon.client-buffer-size", STOCKVAR_INIT, var, pUser);

    // * * * debug * * *
    handleStockVariable("vscp.debug.level", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.debug.flags1", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.debug.flags2", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.debug.flags3", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.debug.flags4", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.debug.flags5", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.debug.flags6", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.debug.flags7", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.debug.flags8", STOCKVAR_INIT, var, pUser);

    // * * * dm * * *
    handleStockVariable("vscp.dm.enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.dm.count", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.dm.count.active", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.dm.path-db", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.dm.path-xml", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.dm.add", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.dm.delete", STOCKVAR_INIT, var, pUser);

    // * * * config * * *
    handleStockVariable("vscp.config.dbversion", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.client-buffer-size", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.guid", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.servername", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.announceinterface-address", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.announceinterface-ttl", STOCKVAR_INIT, var, pUser);

    handleStockVariable(
      "vscp.config.tcpipinterface-address", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-encryption", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-ssl-certificate", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-ssl-certificate-chain", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-ssl-verify-peer", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-ssl-ca-path", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-ssl-ca-file", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-ssl-verify-depth", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-ssl-default-verify-paths", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-ssl-cipher-list", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-ssl-protocol-version", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.tcpip-ssl-short-trust", STOCKVAR_INIT, var, pUser);

    handleStockVariable("vscp.config.udp-enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.udp-address", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.udp-user", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.udp-password", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.udp-unsecure-enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.udp-filter", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.udp-mask", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.udp-guid", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.udp-ack-enable", STOCKVAR_INIT, var, pUser);

    handleStockVariable(
      "vscp.config.muticast-enable", STOCKVAR_INIT, var, pUser);

    handleStockVariable("vscp.config.dm-path-db", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.dm-path-xml", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.dm-allow-xml-save", STOCKVAR_INIT, var, pUser);

    handleStockVariable(
      "vscp.config.variable-path-db", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.variable-path-xml", STOCKVAR_INIT, var, pUser);

    handleStockVariable(
      "vscp.config.path-db-event-data", STOCKVAR_INIT, var, pUser);

    handleStockVariable("vscp.config.web-enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-document-root", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-listening-ports", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-index-files", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-authentication-domain", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-enable-auth-domain-check", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssl-certificate", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssl-certificate-chain", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssl-verify-peer", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssl-ca-path", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssl-ca-file", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssl-verify-depth", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssl-default-verify-paths", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssl-cipher-list", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssl-protocol-version", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssl-short-trust", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-cgi-interpreter", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-cgi-pattern", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-cgi-environment", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-protect-uri", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.web-trottle", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-enable-directory-listing", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-enable-keep-alive", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-keep-alive-timeout-ms", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-access-control-list", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-extra-mime-types", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-num-threads", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-run-as-user", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-url-rewrite-patterns", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-hide-file-patterns", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-request-timeout-ms", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-linger-timeout-ms", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-decode-url", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-global-authfile", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-per-directory-auth-file", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-ssi-patterns", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-access-control-allow-origin", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.web-access-control-allow-methods",
                        STOCKVAR_INIT,
                        var,
                        pUser);
    handleStockVariable("vscp.config.web-access-control-allow-headers",
                        STOCKVAR_INIT,
                        var,
                        pUser);
    handleStockVariable(
      "vscp.config.web-error-pages", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-tcp-nodelay", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-static-file-max-age", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.web-strict-transport-security-max-age",
                        STOCKVAR_INIT,
                        var,
                        pUser);
    handleStockVariable(
      "vscp.config.web-allow-sendfile-call", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-additional-headers", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-max-request-size", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-allow-index-script-resource", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-duktape-script-pattern", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-lua-preload-file", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-lua-script-pattern", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-lua-server-page-pattern", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-lua-websocket-pattern", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.web-lua-background-script", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.web-lua-background-script-params",
                        STOCKVAR_INIT,
                        var,
                        pUser);

    handleStockVariable(
      "vscp.config.websocket-enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.websocket-document-root", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.websocket-timeout-ms", STOCKVAR_INIT, var, pUser);

    handleStockVariable(
      "vscp.config.automation-enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.automation-zone", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.automation-subzone", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.automation-longitude", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.automation-latitude", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.automation-sunrise-enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.automation-sunset-enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.automation-sunset-twilight-enable",
                        STOCKVAR_INIT,
                        var,
                        pUser);
    handleStockVariable("vscp.config.automation-sunrise-twilight-enable",
                        STOCKVAR_INIT,
                        var,
                        pUser);
    handleStockVariable(
      "vscp.config.automation-segment-ctrl-enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.automation-segment-ctrl-interval",
                        STOCKVAR_INIT,
                        var,
                        pUser);
    handleStockVariable(
      "vscp.config.automation-heartbeat-enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.automation-heartbeat-interval", STOCKVAR_INIT, var, pUser);
    handleStockVariable(
      "vscp.config.automation-capabilities-enable", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.config.automation-capabilities-interval",
                        STOCKVAR_INIT,
                        var,
                        pUser);
    // * * * user * * *
    handleStockVariable("vscp.user.count", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.user", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.user.add", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.user.delete", STOCKVAR_INIT, var, pUser);

    // * * * interface * * *
    handleStockVariable("vscp.interface.count", STOCKVAR_INIT, var, pUser);
    handleStockVariable("vscp.interface", STOCKVAR_INIT, var, pUser);
}