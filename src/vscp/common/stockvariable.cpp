// handleStockVariable.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
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
#include <mongoose.h> // For copytight info

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
                                      CUserItem &user)
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
        var.init();
        var.setName(name);
        var.setID(ID_NON_PERSISTENT);
        var.setStockVariable(true);
        var.setPersistent(false);
        var.setOwnerId(USER_ID_ADMIN);
        var.setAccessRights(PERMISSION_ALL_READ | PERMISSION_OWNER_WRITE);
        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
        var.setValue("");
        var.setNote("");
        var.setLastChangedToNow();
    } else if (STOCKVAR_READ == op) {
        // Try to read the variable from stock storage
        var_id = findNonPersistentVariable(name, var);
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
            }
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
        }

        if ("flags2" == strToken) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
        }

        if ("flags3" == strToken) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
        }

        if ("flags" == strToken) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
        }

        if ("flags5" == strToken) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
        }

        if ("flags6" == strToken) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
        }

        if ("flags7" == strToken) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
        }

        if ("flags8" == strToken) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
        }
    }

    // *************************************************************************
    //                                Configure
    // *************************************************************************

    if ("config" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();
    }

    // *************************************************************************
    //                      event (info from vscp data database)
    // *************************************************************************

    // vscp.event.type.m.n               - Return VSCP type as string:
    // m=class, n=type vscp.event.type.m.n.description   - Return VSCP type
    // description as string: m=class vscp.event.type.m.n.token         -
    // Return VSCP type token as string: m=class vscp.event.type.m.n.unit -
    // Return VSCP type units as a comma separated string: m=class
    // vscp.event.type.m.n.unit.k        - Return VSCP type unit k
    // vscp.event.class.m                - Return VSCP class as string:
    // m=class vscp.event.class.m.description    - Return VSCP class
    // description as string: m=class vscp.event.class.m.count          -
    // Return VSCP # types for class: m=class vscp.event.class.count -
    // Return VSCP # classes as long vscp.event.class                  -
    // Return comma separated list with all classes vscp.event.class.m.type
    // - Return comma separated list with all types of a classe m

    if ("event" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();
    }

    // *************************************************************************
    //                            Decision Matrix (DM)
    // *************************************************************************

    // vscp.dm.count
    // vscp.dm.count.active
    // vscp.dm.db.path
    // vscp.dm.xml.path
    // vscp.dm.loglevel
    // vscp.dm                  - Full DM
    // vscp.dm.xml              - Full DM XML    FUTURE
    // vscp.dm.json             - Full DM JSON   FUTURE
    // vscp.dm.n                - Row n of DM
    // vscp.dm.n.field          - Field 'field' of rown n of DM

    if ("dm" == strToken) {

        // 'vscp.dm' - Return all rows of DM - Only read allowed
        if (tokens.empty()) {
            if (STOCKVAR_INIT == op) {
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setName("vscp.dm");
                var.setNote("All decision matrix rows.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                var.setLastChangedToNow();
                gpobj->m_dm.getAllRows(str);
                var.setValue(str);
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(LOG_ERR,
                       "Not allowed to write remote variable %s, "
                       "write individual DM rows instead.",
                       name.c_str());
                return false;
            }
        }

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();

        // vscp.dm.enable
        if ("enable" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName("vscp.dm.enable");
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue(true); // temporary, set when read
                var.setNote("Read/write enable/disable DM.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                var.setLastChangedToNow();
                var.setValue(gpobj->m_dm.m_bEnable);
                return true;
            } else if (STOCKVAR_WRITE == op) {

                // Only admin is allowed to write
                if (USER_ID_ADMIN != user.getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }

                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be BOOL to write remote variable %s",
                           name.c_str());
                    return false;
                }

                bool bEnable;
                var.getValue(&bEnable);

                pthread_mutex_lock(&gpobj->m_dm.m_mutexDM);
                gpobj->m_dm.m_bEnable = bEnable;
                pthread_mutex_unlock(&gpobj->m_dm.m_mutexDM);

                return true;
            }
        } else if ("count" == strToken) {

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            // vscp.dm.count
            if (tokens.empty()) {
                if (STOCKVAR_INIT == op) {
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setName("vscp.dm.count");
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                    var.setValue(0); // temporary, set when read
                    var.setNote("Total number of DM rows.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    var.setLastChangedToNow();
                    var.setValue((long)gpobj->m_dm.getDatabaseRecordCount());
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            }

            // vscp.dm.count.active
            if ("active" == strToken) {
                if (STOCKVAR_INIT == op) {
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setName("vscp.dm.count.active");
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                    var.setValue(0); // temporary, set when read
                    var.setNote("Number of active DM rows.");
                    return addStockVariable(var);
                } else if (STOCKVAR_READ == op) {
                    var.setLastChangedToNow();
                    var.setValue(gpobj->m_dm.getMemoryElementCount());
                    return true;
                } else if (STOCKVAR_WRITE == op) {
                    syslog(
                      LOG_ERR, "Remote variable %s is read only", name.c_str());
                    return false;
                }
            }
        } else if ("path-db" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setName("vscp.dm.path-db");
                var.setNote("Path to DM database file.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                var.setLastChangedToNow();
                var.setValue(gpobj->m_dm.getMemoryElementCount());
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("path-xml" == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setAccessRights(PERMISSION_ALL_READ);
                var.setName("vscp.dm.path-xml");
                var.setNote("Path to DM XML file.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                var.setLastChangedToNow();
                var.setValue(gpobj->m_dm.getMemoryElementCount());
                return true;
            } else if (STOCKVAR_WRITE == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is read only", name.c_str());
                return false;
            }
        } else if ("add" == strToken) { // value is var-def
            if (STOCKVAR_INIT == op) {
                var.setAccessRights(PERMISSION_OWNER_EXECUTE);
                var.setName("vscp.dm.add");
                var.setNote("Add DM row.");
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is write only", name.c_str());
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Add DM row in value
            }
        } else if ("delete" == strToken) { // arg is row
            if (STOCKVAR_INIT == op) {
                var.setAccessRights(PERMISSION_OWNER_EXECUTE);
                var.setName("vscp.dm.remove");
                var.setNote("Delete DM row.");
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(0); // temporary, set when read
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                syslog(
                  LOG_ERR, "Remote variable %s is write only", name.c_str());
                return true;
            } else if (STOCKVAR_WRITE == op) {
                // Add DM row in value
            }
        } else if (vscp_isNumber(strToken)) { // vscp.dm.n[.field]

            long id; // DM row id
            id = vscp_readStringValue(strToken);

            dmElement *pDMRow = NULL;
            if (!gpobj->m_dm.getDatabaseRecord(id, pDMRow)) {
                syslog(LOG_ERR,
                       "Remote variable read of %s. A DM row with the "
                       "specified id [%ld] does not exist",
                       name.c_str(),
                       id);
                return false;
            }

            var.setStockVariable();
            var.setPersistent(false);

            // vscp.dm.n  -  Get full DM row
            if (tokens.empty()) {
                if (STOCKVAR_READ == op) {
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setName(name);
                    var.setNote("DM row.");
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(pDMRow->getAsString(false));
                } else if (STOCKVAR_WRITE == op) {
                    // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write DM row %ld for "
                               "remote variable %s",
                               id,
                               name.c_str());
                        return false;
                    }
                    std::string str = var.getValue();
                    if (!pDMRow->setFromString(str)) {
                        syslog(LOG_ERR,
                               "Failed to write DM row %ld for remote "
                               "variable %s",
                               id,
                               name.c_str());
                        return false;
                    } else {
                        syslog(LOG_ERR,
                               "Init. is an invalid operation for remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }
                }
            }

            strToken = tokens.front();
            vscp_trim(strToken);
            tokens.pop_front();

            // We expect a field name    vscp.dm.n.field

            if ("id" == strToken) {

                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                    var.setValue((long)pDMRow->m_id);
                    var.setNote("The id field for DM row %ld", id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // Only admin is allowed to write
                    syslog(LOG_ERR,
                           "DM id=%ld field for remote variable %s is read "
                           "only and can not be changed.",
                           id,
                           name.c_str());
                    return false;
                }
            } else if ("enable" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                    var.setValue((long)pDMRow->m_bEnable);
                    var.setNote("The bEnable field for DM row %ld", id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write DM bEnable "
                               "field for "
                               "remote variable %s",

                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                        syslog(LOG_ERR,
                               "DM bEnable must be of type boolean to be "
                               "able to write remote variable %s",
                               name.c_str());
                        return false;
                    }

                    bool bEnable;
                    var.getValue(&bEnable);
                    pDMRow->m_bEnable = bEnable;

                    return true;
                }

            } else if ("groupid" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(pDMRow->m_strGroupID);
                    var.setNote("The groupid field for DM row %ld", id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write the DM groupid "
                               "field for remote variable %s",

                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                        syslog(LOG_ERR,
                               "The DM groupid field must be of type "
                               "string to be "
                               "able to write remote variable %s",
                               name.c_str());
                        return false;
                    }

                    std::string str      = var.getValue();
                    pDMRow->m_strGroupID = str;

                    return true;
                }
            } else if ("mask" == strToken) {

                // Full mask
                if (tokens.empty()) { // 'vscp.dm.n.mask'

                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                        std::string str;
                        vscp_writeMaskToString(&pDMRow->m_vscpfilter, str);
                        var.setValue(str);
                        var.setNote("mask part of filter field for DM "
                                    "rowwith id  %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {
                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM mask part of filter must be of type "
                                   "string to be able to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        if (!vscp_readMaskFromString(&pDMRow->m_vscpfilter,
                                                     str)) {
                            syslog(LOG_ERR,
                                   "Failed to read mask for remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        return true;
                    }

                } // full mask

                strToken = tokens.front();
                vscp_trim(strToken);
                tokens.pop_front();

                if ("priority" == strToken) { // vscp.dm.n.mask.priority
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                        var.setValue((int)pDMRow->m_vscpfilter.mask_priority);
                        var.setNote("Filter mask priority field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {
                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_INTEGER !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter mask priority field must be of "
                                   "type integer to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        int val;
                        var.getValue(&val);
                        pDMRow->m_vscpfilter.mask_priority = val;

                        return true;
                    }
                } else if ("class" == strToken) { // vscp.dm.n.mask.class
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                        var.setValue(pDMRow->m_vscpfilter.mask_class);
                        var.setNote("Filter mask class field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter mask priority field must be of "
                                   "type integer to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        int val;
                        var.getValue(&val);
                        pDMRow->m_vscpfilter.mask_class = val;

                        return true;
                    }
                } else if ("type" == strToken) { // vscp.dm.n.mask.type
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                        var.setValue(pDMRow->m_vscpfilter.mask_type);
                        var.setNote("Filter mask type field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter mask type field must be of "
                                   "type integer to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        int val;
                        var.getValue(&val);
                        pDMRow->m_vscpfilter.mask_type = val;

                        return true;
                    }
                } else if ("guid" == strToken) { // vscp.dm.n.mask.guid
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_GUID);
                        std::string str;
                        vscp_writeGuidArrayToString(
                          pDMRow->m_vscpfilter.mask_GUID, str);
                        var.setValue(str);
                        var.setNote("Filter mask GUID field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (!((VSCP_DAEMON_VARIABLE_CODE_GUID !=
                               var.getType()) ||
                              (VSCP_DAEMON_VARIABLE_CODE_STRING !=
                               var.getType()))) {
                            syslog(LOG_ERR,
                                   "DM filter type GUID field must be of "
                                   "type integer to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        vscp_getGuidFromStringToArray(
                          pDMRow->m_vscpfilter.mask_GUID, str);

                        return true;
                    }
                }
            } // vscp.dm.n.mask
            else if ("filter" == strToken) {

                // Full filter
                if (tokens.empty()) { // 'vscp.dm.n.filter'

                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                        std::string str;
                        vscp_writeMaskToString(&pDMRow->m_vscpfilter, str);
                        var.setValue(str);
                        var.setNote("filter part of filter field for DM "
                                    "rowwith id  %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {
                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter part of filter must be of type "
                                   "string to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        if (!vscp_readMaskFromString(&pDMRow->m_vscpfilter,
                                                     str)) {
                            syslog(LOG_ERR,
                                   "Failed to read filter "
                                   "for remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        return true;
                    }

                } // full filter

                strToken = tokens.front();
                vscp_trim(strToken);
                tokens.pop_front();

                if ("priority" == strToken) { // vscp.dm.n.filter.priority
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                        var.setValue((int)pDMRow->m_vscpfilter.filter_priority);
                        var.setNote(
                          "Filter filter priority field for DM row %ld", id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {
                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_INTEGER !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter filter priority field must be of "
                                   "type integer to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        int val;
                        var.getValue(&val);
                        pDMRow->m_vscpfilter.filter_priority = val;

                        return true;
                    }
                } else if ("class" == strToken) { // vscp.dm.n.filter.class
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                        var.setValue(pDMRow->m_vscpfilter.filter_class);
                        var.setNote("Filter filter class field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter filter priority field must be of "
                                   "type integer to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        int val;
                        var.getValue(&val);
                        pDMRow->m_vscpfilter.filter_class = val;

                        return true;
                    }
                } else if ("type" == strToken) { // vscp.dm.n.filter.type
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                        var.setValue(pDMRow->m_vscpfilter.filter_type);
                        var.setNote("Filter filter type field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter filter type field must be of "
                                   "type integer to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        int val;
                        var.getValue(&val);
                        pDMRow->m_vscpfilter.filter_type = val;

                        return true;
                    }
                } else if ("guid" == strToken) { // vscp.dm.n.filter.guid
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_GUID);
                        std::string str;
                        vscp_writeGuidArrayToString(
                          pDMRow->m_vscpfilter.filter_GUID, str);
                        var.setValue(str);
                        var.setNote("Filter filter GUID field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if ((VSCP_DAEMON_VARIABLE_CODE_GUID != var.getType()) &&
                            (VSCP_DAEMON_VARIABLE_CODE_STRING !=
                             var.getType())) {
                            syslog(LOG_ERR,
                                   "DM filter type GUID field must be of "
                                   "type integer to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        vscp_getGuidFromStringToArray(
                          pDMRow->m_vscpfilter.filter_GUID, str);

                        return true;
                    }
                }
            } else if ("allowed" == strToken) { // 'vscp.dm.n.allowed'

                // A string on the following form:
                // start,end,weekdays,time

                if (tokens.empty()) {

                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                        std::string str;
                        // Start time
                        str =
                          pDMRow->m_timeAllow.getStartTime().getISODateTime() +
                          ",";
                        // End time
                        str +=
                          pDMRow->m_timeAllow.getEndTime().getISODateTime() +
                          ",";
                        // Allowed weekdays
                        str += pDMRow->m_timeAllow.getWeekDays() + ",";
                        // Action time
                        str += pDMRow->m_timeAllow.getActionTimeAsString();
                        var.setValue(str);
                        var.setNote("DM allowed section for DM "
                                    "row %ld",
                                    id);
                        var.setLastChangedToNow();
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is
                        // allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must "
                                   "be of type "
                                   "long to be "
                                   "able to "
                                   "write "
                                   "remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::deque<std::string> tokAllowed;
                        vscp_split(tokAllowed, strToken, ",");

                        // Start time
                        if (tokAllowed.empty()) {
                            syslog(LOG_ERR,
                                   "Missing allow start time trying to "
                                   "write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = tokAllowed.front();
                        tokAllowed.pop_front();
                        pDMRow->m_timeAllow.setStartTime(str);

                        // End time
                        if (tokAllowed.empty()) {
                            syslog(LOG_ERR,
                                   "Missing allow end time trying to write "
                                   "remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        str = tokAllowed.front();
                        tokAllowed.pop_front();
                        pDMRow->m_timeAllow.setEndTime(str);

                        // Weekdays
                        if (tokAllowed.empty()) {
                            syslog(LOG_ERR,
                                   "Missing allow weekdays trying to write "
                                   "remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        str = tokAllowed.front();
                        tokAllowed.pop_front();
                        if (!pDMRow->m_timeAllow.setWeekDays(str)) {
                            syslog(LOG_ERR,
                                   "Failed to write allowed weekdays for "
                                   "remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        // time allow
                        if (tokAllowed.empty()) {
                            syslog(LOG_ERR,
                                   "Missing allow time trying to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        str = tokAllowed.front();
                        tokAllowed.pop_front();
                        if (!pDMRow->m_timeAllow.parseActionTime(str)) {
                            syslog(LOG_ERR,
                                   "Failed to write allowed action time for "
                                   "remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        return true;
                    }
                } // 'vscp.dm.n.allowed'

                strToken = tokens.front();
                vscp_trim(strToken);
                tokens.pop_front();

                if ("start" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
                        vscpdatetime dt = pDMRow->m_timeAllow.getStartTime();
                        var.setValue(dt);
                        var.setNote("DM allow start time field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_DATETIME !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM start field must  be of type "
                                   "date/time to be  able to "
                                   "write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        pDMRow->m_timeAllow.setStartTime(str);

                        return true;
                    }
                } else if ("end" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                        vscpdatetime dt = pDMRow->m_timeAllow.getEndTime();
                        var.setValue(dt);
                        var.setNote("DM allow end time field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM end field must be of type "
                                   "date/time to be able to "
                                   "write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        pDMRow->m_timeAllow.setEndTime(str);

                        return true;
                    }
                } else if ("weekdays" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                        std::string str = pDMRow->m_timeAllow.getWeekDays();
                        var.setValue(str);
                        var.setNote("DM allow weekdays field for row %ld", id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM value must be of type string to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        if (!pDMRow->m_timeAllow.setWeekDays(str)) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        return true;
                    }
                } else if ("monday" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                        var.setValue(
                          pDMRow->m_timeAllow.getWeekday(DM_DAY_ALLOW_MONDAY));
                        var.setNote("DM allow weekday monday field for row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type boolean to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        pDMRow->m_timeAllow.allowMonday(var.isTrue());

                        return true;
                    }
                } else if ("tuesday" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                        var.setValue(
                          pDMRow->m_timeAllow.getWeekday(DM_DAY_ALLOW_TUESDAY));
                        var.setNote("DM allow tuesday field for row %ld", id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type boolean to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        pDMRow->m_timeAllow.allowTuesday(var.isTrue());

                        return true;
                    }
                } else if ("wednessday" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                        var.setValue(pDMRow->m_timeAllow.getWeekday(
                          DM_DAY_ALLOW_WEDNESDAY));
                        var.setNote("DM allow wednessday field for row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type boolean to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        pDMRow->m_timeAllow.allowWednesday(var.isTrue());

                        return true;
                    }
                } else if ("thursday" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                        var.setValue(pDMRow->m_timeAllow.getWeekday(
                          DM_DAY_ALLOW_THURSDAY));
                        var.setNote("DM allow thursday field for row %ld", id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type boolean to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        pDMRow->m_timeAllow.allowThursday(var.isTrue());

                        return true;
                    }
                } else if ("friday" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                        var.setValue(
                          pDMRow->m_timeAllow.getWeekday(DM_DAY_ALLOW_FRIDAY));
                        var.setNote("DM allow friday field for row %ld", id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type boolean to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        pDMRow->m_timeAllow.allowFriday(var.isTrue());

                        return true;
                    }
                } else if ("saturday" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                        var.setValue(pDMRow->m_timeAllow.getWeekday(
                          DM_DAY_ALLOW_SATURDAY));
                        var.setNote("DM allow saturday field for row %ld", id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type boolean to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        pDMRow->m_timeAllow.allowSaturday(var.isTrue());

                        return true;
                    }
                } else if ("sunday" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                        var.setValue(
                          pDMRow->m_timeAllow.getWeekday(DM_DAY_ALLOW_SUNDAY));
                        var.setNote("DM allow sunday field for row %ld", id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type boolean to be "
                                   "able to write remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        pDMRow->m_timeAllow.allowSunday(var.isTrue());

                        return true;
                    }
                } else if ("time" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                        std::string str =
                          pDMRow->m_timeAllow.getActionTimeAsString();
                        var.setValue(str);
                        var.setNote("DM allow time field for row %ld", id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "Remote variable value must be of type "
                                   "string to "
                                   "be written for remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        if (!pDMRow->m_timeAllow.parseActionTime(str)) {
                            syslog(LOG_ERR,
                                   "Unable to set action time for remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        return true;
                    }
                } // vscp.dm.allowed
            } else if ("bCheckIndex" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                    var.setValue(pDMRow->getCheckIndex());
                    var.setNote("Flag to indicate that index should be "
                                "checked for row %ld",
                                id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of type boolean to "
                               "be able to write it %s",
                               name.c_str());
                        return false;
                    }

                    bool b;
                    var.getValue(&b);
                    pDMRow->setCheckIndex(b);

                    return true;
                }
            } else if ("index" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                    var.setValue((int)pDMRow->getIndex());
                    var.setNote("index for DM row %ld", id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of type integer to "
                               "be able to write it %s",
                               name.c_str());
                        return false;
                    }

                    int val;
                    var.getValue(&val);
                    pDMRow->setIndex((uint8_t)val);

                    return true;
                }
            } else if ("bCheckZone" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                    var.setValue(pDMRow->getCheckZone());
                    var.setNote("Flag to indicate that zone should be "
                                "checked for row %ld",
                                id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of type boolean to "
                               "be able to write it %s",
                               name.c_str());
                        return false;
                    }

                    bool val;
                    var.getValue(&val);
                    pDMRow->setCheckZone(val);

                    return true;
                }
            } else if ("zone" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                    var.setValue((long)pDMRow->getZone());
                    var.setNote("zone for DM row %ld", id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of type integer to "
                               "be able to write it %s",
                               name.c_str());
                        return false;
                    }

                    int val;
                    var.getValue(&val);
                    pDMRow->setZone((uint8_t)val);

                    return true;
                }
            } else if ("bCheckSubZone" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                    var.setValue(pDMRow->getCheckZone());
                    var.setNote("Flag to indicate that SubZone should be "
                                "checked for row%ld",
                                id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of type boolean to "
                               "be able to write it %s",
                               name.c_str());
                        return false;
                    }

                    bool val;
                    var.getValue(&val);
                    pDMRow->setCheckSubZone(val);

                    return true;
                }
            } else if ("subzone" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                    var.setValue((int)pDMRow->getSubZone());
                    var.setNote("zone for DM row %ld", id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of type integer to "
                               "be able to write it %s",
                               name.c_str());
                        return false;
                    }

                    int val;
                    var.getValue(&val);
                    pDMRow->setSubZone((uint8_t)val);

                    return true;
                }
            } else if ("measurement" == strToken) {

                if (tokens.empty()) { // vscp.dm.n.measurement

                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                        var.setValue(pDMRow->getMeasurementGroup());
                        var.setNote("Measurement value field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "Remote variable must be of type string to "
                                   "be able to write it %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        pDMRow->setMeasurementGroup(str);

                        return true;
                    }
                }

                strToken = tokens.front();
                tokens.pop_front();
                vscp_makeLower(strToken);
                vscp_trim(strToken);

                if ("enable" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                        var.setValue(pDMRow->getCheckMeasurementIndex());
                        var.setNote("Measurement enable field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "Remote variable must be of type string to "
                                   "be able to write it %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        pDMRow->setMeasurementGroup(str);

                        return true;
                    }
                } else if ("value" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_DOUBLE);
                        var.setValue(pDMRow->getMeasurementValue());
                        var.setNote("Measurement value field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "Remote variable must be of type double to "
                                   "be able to write it %s",
                                   name.c_str());
                            return false;
                        }

                        bool val;
                        var.getValue(&val);
                        pDMRow->setCheckMeasurementIndex(val);

                        return true;
                    }
                } else if ("unit" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                        var.setValue((int)pDMRow->getMeasurementUnit());
                        var.setNote("Measurement unit field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type integer to be able "
                                   "to write "
                                   "remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        int val;
                        var.getValue(&val);
                        pDMRow->setMeasurementUnit(val);

                        return true;
                    }
                } else if ("compare-code" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                        var.setValue((int)pDMRow->getMeasurementCompareCode());
                        var.setNote("Measurement compare field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_INTEGER !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM value must be of type integer to be "
                                   "able to "
                                   "write "
                                   "remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        int val;
                        var.getValue(&val);
                        pDMRow->setMeasurementCompareCode(val);

                        return true;
                    }
                } else if ("compare-string" == strToken) {
                    if (STOCKVAR_READ == op) {
                        var.setID(ID_NON_PERSISTENT);
                        var.setStockVariable(true);
                        var.setPersistent(false);
                        var.setOwnerId(USER_ID_ADMIN);
                        var.setAccessRights(PERMISSION_ALL_READ |
                                            PERMISSION_OWNER_WRITE);
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                        std::string str =
                          pDMRow->getSymbolicMeasurementFromCompareCode(
                            pDMRow->getMeasurementCompareCode());
                        var.setValue(str);
                        var.setNote("Measurement compare string for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(
                              LOG_ERR,
                              "DM field value must be of type string to be "
                              "able to write "
                              "remote variable %s",
                              name.c_str());
                            return false;
                        }

                        uint8_t code;
                        std::string str = var.getValue();
                        if (!pDMRow->setSymbolicMeasurementCompareCode(str)) {
                            syslog(
                              LOG_ERR,
                              "Measurement compare code has invalid format "
                              "for remote variable %s",
                              name.c_str());
                            return false;
                        }

                        return true;
                    }
                } // end of vscp.dm.measurement

            } else if ("comment" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(pDMRow->getComment());
                    var.setNote("Comment field for DM row %ld", id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                        syslog(LOG_ERR,
                               "DM value must be of type string to be able "
                               "to write "
                               "remote variable %s",
                               name.c_str());
                        return false;
                    }

                    std::string str = var.getValue();
                    pDMRow->setComment(str);

                    return true;
                }
            } else if ("count-trigger" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                    var.setValue((long)pDMRow->getTriggerCounter());
                    var.setNote("Action count trigger field for DM row %ld",
                                id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                        syslog(LOG_ERR,
                               "DM value must be of type integer to be able to "
                               "write "
                               "remote variable %s",
                               name.c_str());
                        return false;
                    }

                    pDMRow->clrTriggerCounter();

                    return true;
                }
            } else if ("count-error" == strToken) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                    var.setValue((long)pDMRow->getErrorCounter());
                    var.setNote("Action count error field for DM row %ld", id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                        syslog(LOG_ERR,
                               "DM value must be of type integer to be able to "
                               "write "
                               "remote variable %s",
                               name.c_str());
                        return false;
                    }

                    pDMRow->clrErrorCounter();

                    return true;
                }
            } else if (("error" == strToken) || ("error-string" == strToken)) {
                if (STOCKVAR_READ == op) {
                    var.setID(ID_NON_PERSISTENT);
                    var.setStockVariable(true);
                    var.setPersistent(false);
                    var.setOwnerId(USER_ID_ADMIN);
                    var.setAccessRights(PERMISSION_ALL_READ |
                                        PERMISSION_OWNER_WRITE);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(pDMRow->getErrorString());
                    var.setNote("Action error string field for DM row %ld", id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                        syslog(LOG_ERR,
                               "DM id must be of type string to be "
                               "able to write remote variable %s",
                               name.c_str());
                        return false;
                    }

                    pDMRow->clrErrorString();

                    return true;

                } else {
                    syslog(LOG_ERR,
                           "DM value must be of type integer to be able to "
                           "write "
                           "remote variable %s",
                           name.c_str());
                    return false;
                }

            } // vscp.dm.n.error

        } // vscp.dm.n

    } // vscp.dm

    // *************************************************************************
    //                                 disovery
    // *************************************************************************

    if ("discovery" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();
    }

    // *************************************************************************
    //                                Interface
    // *************************************************************************

    if ("interface" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();
    }

    // *************************************************************************
    //                                  Users
    // *************************************************************************

    if ("user" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();
    }

    // *************************************************************************
    //                                 Drivers
    // *************************************************************************

    if ("driver" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();
    }

    // *************************************************************************
    //                                  Tables
    // *************************************************************************

    if ("table" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();
    }

    // *************************************************************************
    //                                   UDP
    // *************************************************************************

    if ("udp" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();
    }

    // *************************************************************************
    //                                   MULTICAST
    // *************************************************************************

    if ("multicast" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// initStockVariables
//

void
CVariableStorage::initStockVariables(CUserItem &user)
{
    CVariable var;

    handleStockVariable("vscp.version", STOCKVAR_INIT, var, user);

    handleStockVariable("vscp.version.major", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.version.minor", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.version.release", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.version.build", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.version.string", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.version.copyright", STOCKVAR_INIT, var, user);

    handleStockVariable(
      "vscp.version.sqlite3.string", STOCKVAR_INIT, var, user);
    handleStockVariable(
      "vscp.version.sqlite3.copyright", STOCKVAR_INIT, var, user);

    handleStockVariable(
      "vscp.version.openssl.string", STOCKVAR_INIT, var, user);
    handleStockVariable(
      "vscp.version.openssl.copyright", STOCKVAR_INIT, var, user);

    handleStockVariable(
      "vscp.version.duktape.string", STOCKVAR_INIT, var, user);
    handleStockVariable(
      "vscp.version.duktape.copyright", STOCKVAR_INIT, var, user);

    handleStockVariable(
      "vscp.version.civetweb.string", STOCKVAR_INIT, var, user);
    handleStockVariable(
      "vscp.version.civetweb.copyright", STOCKVAR_INIT, var, user);

    handleStockVariable("vscp.version.lua.string", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.version.lua.copyright", STOCKVAR_INIT, var, user);

    handleStockVariable(
      "vscp.version.mongoose.string", STOCKVAR_INIT, var, user);
    handleStockVariable(
      "vscp.version.mongoose.copyright", STOCKVAR_INIT, var, user);

    handleStockVariable(
      "vscp.version.nlohmann-json.string", STOCKVAR_INIT, var, user);
    handleStockVariable(
      "vscp.version.nlohmann-json.copyright", STOCKVAR_INIT, var, user);

    handleStockVariable(
      "vscp.version.xml2json.string", STOCKVAR_INIT, var, user);
    handleStockVariable(
      "vscp.version.xml2json.copyright", STOCKVAR_INIT, var, user);

    handleStockVariable("vscp.os.width", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.endian.big", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.endian.little", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.userid", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.groupid", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.username", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.sysname", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.nodename", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.release", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.version", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.machine", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.host.name", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.host.domain", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.host.ip", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.os.host.mac", STOCKVAR_INIT, var, user);

    handleStockVariable("vscp.daemon.runasuser", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.daemon.guid", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.daemon.name", STOCKVAR_INIT, var, user);
    handleStockVariable(
      "vscp.daemon.client-buffer-size", STOCKVAR_INIT, var, user);

    handleStockVariable("vscp.debug.level", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.debug.flags1", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.debug.flags2", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.debug.flags3", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.debug.flags4", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.debug.flags5", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.debug.flags6", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.debug.flags7", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.debug.flags8", STOCKVAR_INIT, var, user);

    handleStockVariable("vscp.dm.enable", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.dm.count", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.dm.count.active", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.dm.path-db", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.dm.path-xml", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.dm.add", STOCKVAR_INIT, var, user);
    handleStockVariable("vscp.dm.delete", STOCKVAR_INIT, var, user);
}