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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                var.setNote("Configuration: VSCP Daemon UDP user.");
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                var.setNote("Configuration: Web-server run as user.");
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
            // Automation
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Automation enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_ENABLE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                      VSCPDB_CONFIG_NAME_AUTOMATION_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                int val = vscp_readStringValue( str );
                if ( val ) {
                    gpobj->m_automation.enableAutomation();
                }
                else {
                    gpobj->m_automation.disableAutomation();
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_ZONE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote("Configuration: Automation zone.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_ZONE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                      VSCPDB_CONFIG_NAME_AUTOMATION_ZONE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                uint8_t zone = vscp_readStringValue(str);
                gpobj->m_automation.setZone(zone);

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_SUBZONE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setNote("Configuration: Automation subzone.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SUBZONE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                      VSCPDB_CONFIG_NAME_AUTOMATION_SUBZONE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                uint8_t subzone = vscp_readStringValue(str);
                gpobj->m_automation.setSubzone(subzone);

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_LONGITUDE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_DOUBLE);
                var.setNote("Configuration: Automation longitude.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_LONGITUDE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_DOUBLE != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be float to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_AUTOMATION_LONGITUDE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                try {
                    double val = std::stod(str);
                    gpobj->m_automation.setLongitude( val );
                }
                catch (...) {
                    syslog(LOG_ERR,
                           "Failed to set acive value for remote "
                           "variable %s",
                           name.c_str());
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_LATITUDE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_DOUBLE);
                var.setNote("Configuration: Automation latitude.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_LATITUDE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
                    syslog(LOG_ERR,
                           "Only admin is allowed to write remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e able to write
                }
                if (VSCP_DAEMON_VARIABLE_CODE_DOUBLE != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be float to write remote variable %s",
                           name.c_str());
                    return false;
                }

                // Set database configuration value
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_AUTOMATION_LATITUDE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                try {
                    double val = std::stod(str);
                    gpobj->m_automation.setLatitude( val );
                }
                catch (...) {
                    syslog(LOG_ERR,
                           "Failed to set acive value for remote "
                           "variable %s",
                           name.c_str());
                }

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Automation sunrise event enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_ENABLE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                      VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                bool bEnable = (0 != atoi(str.c_str()));
                gpobj->m_automation.enableSunRiseEvent(bEnable);

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Automation sunset event enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_ENABLE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                bool bEnable = (0 != atoi(str.c_str()));
                gpobj->m_automation.enableSunSetEvent(bEnable);

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_TWILIGHT_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Automation sunrise twilight event enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_TWILIGHT_ENABLE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_TWILIGHT_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                bool bEnable = (0 != atoi(str.c_str()));
                gpobj->m_automation.enableSunRiseTwilightEvent(bEnable);

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_TWILIGHT_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Automation sunset twilight event enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_TWILIGHT_ENABLE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_TWILIGHT_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                bool bEnable = (0 != atoi(str.c_str()));
                gpobj->m_automation.enableSunSetTwilightEvent(bEnable);

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Automation segment controller event enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_ENABLE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                bool bEnable = (0 != atoi(str.c_str()));
                gpobj->m_automation.enableSegmentControllerHeartbeat(bEnable);

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Automation hearbeat event enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_ENABLE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                bool bEnable = (0 != atoi(str.c_str()));
                gpobj->m_automation.enableHeartbeatEvent(bEnable);

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_ENABLE == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setNote("Configuration: Automation server capabilities event enable.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_ENABLE, buf, sizeof(buf))) {
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                std::string str = var.getValue();
                if (gpobj->updateConfigurationRecordItem(
                      VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_ENABLE, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                bool bEnable = (0 != atoi(str.c_str()));
                gpobj->m_automation.enableCapabilitiesEvent(bEnable);

                return true;
            }
        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_INTERVAL == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote(
                  "Configuration: Automation segment controller event interval in milliseconds.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_INTERVAL,
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                      VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_INTERVAL, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                long val = vscp_readStringValue(str);
                gpobj->m_automation.setSegmentControllerHeartbeatInterval( val );

                return true;
            }

        }  else if (VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_INTERVAL == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote(
                  "Configuration: Automation heartbeat event interval in milliseconds.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_INTERVAL,
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                      VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_INTERVAL, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                long val = vscp_readStringValue(str);
                gpobj->m_automation.setHeartbeatEventInterval( val );

                return true;
            }

        } else if (VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_INTERVAL == strToken) {
            if (STOCKVAR_INIT == op) {
                var.setName(name);
                var.setAccessRights(PERMISSION_OWNER_WRITE |
                                    PERMISSION_ALL_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setNote(
                  "Configuration: Automation server capabilities event interval in milliseconds.");
                var.setValue(0); // Dummy value
                return addStockVariable(var);
            } else if (STOCKVAR_READ == op) {
                char buf[80];
                if (!gpobj->getConfigurationValueFromDatabase(
                      VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_INTERVAL,
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
                if (USER_ID_ADMIN != user.getUserID()) {
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
                      VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_INTERVAL, str.c_str())) {
                    syslog(LOG_ERR,
                           "Failed to update configuration database for remote "
                           "variable %s",
                           name.c_str());
                    return false;
                }

                long val = vscp_readStringValue(str);
                gpobj->m_automation.setCapabilitiesEventInterval( val );

                return true;
            }

        }
 
    }

    // *************************************************************************
    //                      event (info from vscp data
    //                      database)
    // *************************************************************************

    // vscp.event.type.m.n               - Return VSCP type
    // as string: m=class, n=type
    // vscp.event.type.m.n.description   - Return VSCP type
    // description as string: m=class
    // vscp.event.type.m.n.token         - Return VSCP type
    // token as string: m=class vscp.event.type.m.n.unit -
    // Return VSCP type units as a comma separated string:
    // m=class vscp.event.type.m.n.unit.k        - Return
    // VSCP type unit k vscp.event.class.m                -
    // Return VSCP class as string: m=class
    // vscp.event.class.m.description    - Return VSCP class
    // description as string: m=class
    // vscp.event.class.m.count          - Return VSCP #
    // types for class: m=class vscp.event.class.count -
    // Return VSCP # classes as long vscp.event.class -
    // Return comma separated list with all classes
    // vscp.event.class.m.type
    // - Return comma separated list with all types of a
    // classe m

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
    // vscp.dm.n.field          - Field 'field' of rown n of
    // DM

    if ("dm" == strToken) {

        // 'vscp.dm' - Return all rows of DM - Only read
        // allowed
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
                       "Not allowed to write remote "
                       "variable %s, "
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
                           "Only admin is allowed to write "
                           "remote variable %s",
                           name.c_str());
                    return false; // Must be admin user to e
                                  // able to write
                }

                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                    syslog(LOG_ERR,
                           "Type must be BOOL to write "
                           "remote variable %s",
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
                       "Remote variable read of %s. A DM "
                       "row with the "
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
                               "Only admin is allowed to "
                               "write DM row %ld for "
                               "remote variable %s",
                               id,
                               name.c_str());
                        return false;
                    }
                    std::string str = var.getValue();
                    if (!pDMRow->setFromString(str)) {
                        syslog(LOG_ERR,
                               "Failed to write DM row %ld "
                               "for remote "
                               "variable %s",
                               id,
                               name.c_str());
                        return false;
                    } else {
                        syslog(LOG_ERR,
                               "Init. is an invalid "
                               "operation for remote "
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
                           "DM id=%ld field for remote "
                           "variable %s is read "
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
                               "Only admin is allowed to "
                               "write DM bEnable "
                               "field for "
                               "remote variable %s",

                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                        syslog(LOG_ERR,
                               "DM bEnable must be of type "
                               "boolean to be "
                               "able to write remote "
                               "variable %s",
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
                               "Only admin is allowed to write "
                               "the DM groupid "
                               "field for remote variable %s",

                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                        syslog(LOG_ERR,
                               "The DM groupid field must "
                               "be of type "
                               "string to be "
                               "able to write remote "
                               "variable %s",
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
                        var.setNote("mask part of filter "
                                    "field for DM "
                                    "rowwith id  %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {
                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM mask part of filter "
                                   "must be of type "
                                   "string to be able to "
                                   "write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        if (!vscp_readMaskFromString(&pDMRow->m_vscpfilter,
                                                     str)) {
                            syslog(LOG_ERR,
                                   "Failed to read mask "
                                   "for remote variable %s",
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
                        var.setNote("Filter mask priority "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {
                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_INTEGER !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter mask priority "
                                   "field must be of "
                                   "type integer to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("Filter mask class "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter mask priority "
                                   "field must be of "
                                   "type integer to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("Filter mask type "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter mask type "
                                   "field must be of "
                                   "type integer to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("Filter mask GUID "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (!((VSCP_DAEMON_VARIABLE_CODE_GUID !=
                               var.getType()) ||
                              (VSCP_DAEMON_VARIABLE_CODE_STRING !=
                               var.getType()))) {
                            syslog(LOG_ERR,
                                   "DM filter type GUID "
                                   "field must be of "
                                   "type integer to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("filter part of filter "
                                    "field for DM "
                                    "rowwith id  %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {
                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter part of "
                                   "filter must be of type "
                                   "string to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("Filter filter priority field "
                                    "for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {
                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_INTEGER !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter filter priority "
                                   "field must be of "
                                   "type integer to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("Filter filter class "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter filter priority "
                                   "field must be of "
                                   "type integer to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("Filter filter type "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM filter filter type "
                                   "field must be of "
                                   "type integer to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("Filter filter GUID "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if ((VSCP_DAEMON_VARIABLE_CODE_GUID != var.getType()) &&
                            (VSCP_DAEMON_VARIABLE_CODE_STRING !=
                             var.getType())) {
                            syslog(LOG_ERR,
                                   "DM filter type GUID "
                                   "field must be of "
                                   "type integer to be "
                                   "able to write remote "
                                   "variable %s",
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
                                   "Only admin is allowed "
                                   "to write remote "
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
                                   "Missing allow start "
                                   "time trying to "
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
                                   "Missing allow end time "
                                   "trying to write "
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
                                   "Missing allow weekdays "
                                   "trying to write "
                                   "remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        str = tokAllowed.front();
                        tokAllowed.pop_front();
                        if (!pDMRow->m_timeAllow.setWeekDays(str)) {
                            syslog(LOG_ERR,
                                   "Failed to write "
                                   "allowed weekdays for "
                                   "remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        // time allow
                        if (tokAllowed.empty()) {
                            syslog(LOG_ERR,
                                   "Missing allow time "
                                   "trying to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        str = tokAllowed.front();
                        tokAllowed.pop_front();
                        if (!pDMRow->m_timeAllow.parseActionTime(str)) {
                            syslog(LOG_ERR,
                                   "Failed to write allowed "
                                   "action time for "
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
                        var.setNote("DM allow start time "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_DATETIME !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM start field must  be of "
                                   "type "
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
                        var.setNote("DM allow end time "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM end field must be of "
                                   "type "
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
                        var.setNote("DM allow weekdays "
                                    "field for row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM value must be of "
                                   "type string to be "
                                   "able to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        if (!pDMRow->m_timeAllow.setWeekDays(str)) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
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
                        var.setNote("DM allow weekday monday field "
                                    "for row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type "
                                   "boolean to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("DM allow tuesday "
                                    "field for row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type "
                                   "boolean to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("DM allow wednessday "
                                    "field for row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type "
                                   "boolean to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("DM allow thursday "
                                    "field for row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type "
                                   "boolean to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("DM allow friday field "
                                    "for row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type "
                                   "boolean to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("DM allow saturday "
                                    "field for row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type "
                                   "boolean to be "
                                   "able to write remote "
                                   "variable %s",
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
                        var.setNote("DM allow sunday field "
                                    "for row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type "
                                   "boolean to be "
                                   "able to write remote "
                                   "variable %s",
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
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "Remote variable value "
                                   "must be of type "
                                   "string to "
                                   "be written for remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        std::string str = var.getValue();
                        if (!pDMRow->m_timeAllow.parseActionTime(str)) {
                            syslog(LOG_ERR,
                                   "Unable to set action "
                                   "time for remote "
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
                    var.setNote("Flag to indicate that "
                                "index should be "
                                "checked for row %ld",
                                id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to "
                               "write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of "
                               "type boolean to "
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
                               "Only admin is allowed to "
                               "write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of "
                               "type integer to "
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
                    var.setNote("Flag to indicate that "
                                "zone should be "
                                "checked for row %ld",
                                id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to "
                               "write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of "
                               "type boolean to "
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
                               "Only admin is allowed to "
                               "write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_INTEGER != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of "
                               "type integer to "
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
                    var.setNote("Flag to indicate that "
                                "SubZone should be "
                                "checked for row%ld",
                                id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to "
                               "write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of "
                               "type boolean to "
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
                               "Only admin is allowed to "
                               "write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != var.getType()) {
                        syslog(LOG_ERR,
                               "Remote variable must be of "
                               "type integer to "
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
                        var.setNote("Measurement value "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "Remote variable must "
                                   "be of type string to "
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
                        var.setNote("Measurement enable "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "Remote variable must "
                                   "be of type string to "
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
                        var.setNote("Measurement value "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "Remote variable must "
                                   "be of type double to "
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
                        var.setNote("Measurement unit "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM id must be of type "
                                   "integer to be able "
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
                        var.setNote("Measurement compare "
                                    "field for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_INTEGER !=
                            var.getType()) {
                            syslog(LOG_ERR,
                                   "DM value must be of "
                                   "type integer to be "
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
                        var.setNote("Measurement compare "
                                    "string for DM row %ld",
                                    id);
                        var.setLastChangedToNow();
                        return true;
                    } else if (STOCKVAR_WRITE == op) {

                        // // Only admin is allowed to write
                        if (USER_ID_ADMIN != user.getUserID()) {
                            syslog(LOG_ERR,
                                   "Only admin is allowed "
                                   "to write remote "
                                   "variable %s",
                                   name.c_str());
                            return false;
                        }

                        if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                            syslog(LOG_ERR,
                                   "DM field value must be "
                                   "of type string to be "
                                   "able to write "
                                   "remote variable %s",
                                   name.c_str());
                            return false;
                        }

                        uint8_t code;
                        std::string str = var.getValue();
                        if (!pDMRow->setSymbolicMeasurementCompareCode(str)) {
                            syslog(LOG_ERR,
                                   "Measurement compare code "
                                   "has invalid format "
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
                               "Only admin is allowed to "
                               "write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                        syslog(LOG_ERR,
                               "DM value must be of type "
                               "string to be able "
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
                    var.setNote("Action count trigger "
                                "field for DM row %ld",
                                id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to "
                               "write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                        syslog(LOG_ERR,
                               "DM value must be of type "
                               "integer to be able to "
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
                    var.setNote("Action count error field "
                                "for DM row %ld",
                                id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to "
                               "write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_LONG != var.getType()) {
                        syslog(LOG_ERR,
                               "DM value must be of type "
                               "integer to be able to "
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
                    var.setNote("Action error string field "
                                "for DM row %ld",
                                id);
                    var.setLastChangedToNow();
                    return true;
                } else if (STOCKVAR_WRITE == op) {

                    // // Only admin is allowed to write
                    if (USER_ID_ADMIN != user.getUserID()) {
                        syslog(LOG_ERR,
                               "Only admin is allowed to "
                               "write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    if (VSCP_DAEMON_VARIABLE_CODE_STRING != var.getType()) {
                        syslog(LOG_ERR,
                               "DM id must be of type "
                               "string to be "
                               "able to write remote "
                               "variable %s",
                               name.c_str());
                        return false;
                    }

                    pDMRow->clrErrorString();

                    return true;

                } else {
                    syslog(LOG_ERR,
                           "DM value must be of type "
                           "integer to be able to "
                           "write "
                           "remote variable %s",
                           name.c_str());
                    return false;
                }

            } // vscp.dm.n.error

        } // vscp.dm.n

    } // vscp.dm

    // *************************************************************************
    //                                 discovery
    // *************************************************************************
    // vscp.discovery.count     - Number of nodes found
    // vscp.discovery           - Info about all discovered
    // nodes vscp.discovery.n         - Info about node n
    // vscp.discovery.n.forget  - Forget found node n

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
    // vscp.interface.count - Number of current interfaces
    // vscp.interface       - Info about all interfaces
    // vscp.interface.n     - Info about interface n

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
    // vscp.user.count      - Number of users
    // vscp.user            - Info about all users
    // vscp.user.n          - Info about user n
    // vscp.user.n.name     - Read/write user name
    // vscp.user.n.password - Read/write user password
    // vscp.user.n.filter   - Read/write user filter
    // vscp.uer.n.mask      - Read write user mask
    // vscp-user.host-list  - Read/write host list
    // vscp.user.event-list - Read write event-list for user
    //
    // vscp.user.add        - Add a user
    // vscp.user.delete     - Delete a user

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
    // vscp.driver.count        - Number of drivers
    // vscp.driver.count.l1     - Number of level I drivers
    // vscp.driver.count.l2     - Number of level II drivers
    // vscp.driver.count.l3     - Number of level III
    // drivers vscp.driver.count.active - Number of active
    // drivers vscp.driver.count.active.l1  - Number of
    // active level L1 drivers vscp.driver.count.active.l2
    // - Number of active level II drivers
    // vscp.driver.count.active.l3  - Number of active level
    // III drivers vscp.driver.start            - Star a
    // diabled driver vscp.driver.stop             - Stop a
    // running driver vscp.driver.restarts         - Restart
    // a driver

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
    // vscp.table.count         - Number of tables
    // vscp.table               - List with all tables
    // vscp.table.n             - List table n
    // vscp.table.n.name        - Read/write table name
    // vscp.table.n.xml         - list table n in XML format
    // vscp.table.n.from.to     - Get Table data for table n
    // in the interval from - to vscp.table.n.raw.from.to -
    // Get Table data raw for table n in the interval from -
    // to vscp.table.n.count.from.to   - Return number of
    // records in range vscp.table.n.first.from.to   - First
    // date in range vscp.table.n.last.from.to    - Last
    // date in range vscp.table.n.sum.from.to     - Sum of
    // records in range vscp.table.n.min.from.to     - Min
    // of records in range vscp.table.n.max.from.to     -
    // Max of records in range vscp.table.n.average.from.to
    // - Average of records in range
    // vscp.table.n.median.from.to  - Median of records in
    // range vscp.table.n.stddev.from.to  - stddev of
    // records in range vscp.table.n.variance.from.to -
    // Variance of records in range
    // vscp.table.n.mode.from.to    - Mode of records in
    // range vscp.table.n.lowerq.from.to  - Lowerq of
    // records in range vscp.table.n.upperq.from.to  -
    // Upperq of records in range vscp.table.n.clear.from.to
    // - Clear records in range vscp.table.log           -
    // Log data (double: just value, string: value,datetime)
    // vscp.table.logsql        - Log data using sql (only
    // admin) vscp.table.add           - Add table
    // vscp.table.delete        - Delete table.

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