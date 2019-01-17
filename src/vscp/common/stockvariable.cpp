// stockvariable.cpp
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
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <duktape.h>
#include <lua.h>

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
CVariableStorage::stockvariable(std::string &name,
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
    //                            Debug flags
    // *************************************************************************

    // Make sure it starts with ".vscp"
    if ("debug" == strToken) {

        // Must be at least one other token
        if (tokens.empty()) return false;

        strToken = tokens.front();
        vscp_trim(strToken);
        tokens.pop_front();

        // --------------------------------------------------------------------

        if ("flags1" == strToken) {
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
                    return false; // Must be admin user to e able to write
                }
                long value;
                var.getValue(&value);
                gpobj->m_debugFlags[0] = (uint32_t)value;
                return true;
            }
        }
    }

    // *************************************************************************
    //                                Configure
    // *************************************************************************

    // *************************************************************************
    //                            Decision Matrix (DM)
    // *************************************************************************

    // vscp.dm.count
    // vscp.dm.count.active
    // vscp.dm.db.path
    // vscp.dm.xml.path
    // vscp.dm.loglevel
    // vscp.dm                  - Full DM
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
                        syslog(
                          LOG_ERR,
                          "Failed to write DM row %ld for remote variable %s",
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
                        syslog(
                          LOG_ERR,
                          "Only admin is allowed to write DM bEnable field for "
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
                        syslog(
                          LOG_ERR,
                          "The DM groupid field must be of type string to be "
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
                        var.setNote(
                          "mask part of filter field for DM rowwith id  %ld",
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
                            syslog(
                              LOG_ERR,
                              "DM mask part of filter must be of type "
                              "string to be able to write remote variable %s",
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
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID);
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

                        if (!((VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID !=
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
                        var.setType(VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID);
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

                        if ((VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID !=
                             var.getType()) &&
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
                            syslog(
                              LOG_ERR,
                              "Missing allow start time trying to write remote "
                              "variable %s",
                              name.c_str());
                            return false;
                        }

                        std::string str = tokAllowed.front();
                        tokAllowed.pop_front();
                        pDMRow->m_timeAllow.setStartTime(str);

                        // End time
                        if (tokAllowed.empty()) {
                            syslog(
                              LOG_ERR,
                              "Missing allow end time trying to write remote "
                              "variable %s",
                              name.c_str());
                            return false;
                        }

                        str = tokAllowed.front();
                        tokAllowed.pop_front();
                        pDMRow->m_timeAllow.setEndTime(str);

                        // Weekdays
                        if (tokAllowed.empty()) {
                            syslog(
                              LOG_ERR,
                              "Missing allow weekdays trying to write remote "
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
                            syslog(
                              LOG_ERR,
                              "Remote variable value must be of type string to "
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
                            syslog(
                              LOG_ERR,
                              "DM value must be of type integer to be able to "
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
                        syslog(
                          LOG_ERR,
                          "DM value must be of type string to be able to write "
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
                    syslog(
                      LOG_ERR,
                      "DM value must be of type integer to be able to write "
                      "remote variable %s",
                      name.c_str());
                    return false;
                }

            } // vscp.dm.n.error

        } // vscp.dm.n

    } // vscp.dm

    // *************************************************************************
    //                                Interfaces
    // *************************************************************************

    // *************************************************************************
    //                                  Users
    // *************************************************************************

    // *************************************************************************
    //                                 Drivers
    // *************************************************************************

    // *************************************************************************
    //                                  Tables
    // *************************************************************************

    // *************************************************************************
    //                                 Discovery
    // *************************************************************************

    // *************************************************************************
    //                                   UDP
    // *************************************************************************

    return false;
}
