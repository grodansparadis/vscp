// VscpCanalDeviceIf.cpp : Defines the initialization routines for the DLL.
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

#include <string>

#include <dlfcn.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>

#include <expat.h>
#include <json.hpp> // Needs C++11  -std=c++11

#include <canal.h>
#include <canal_macro.h>
#include <canaldlldef.h>
#include <vscp.h>
#include <vscpcanaldeviceif.h>
#include <vscphelper.h>

#define XML_BUFF_SIZE 0xffff

// for convenience
using json = nlohmann::json;

VscpCanalDeviceIf::VscpCanalDeviceIf()
{
    // Open syslog
    openlog("node-canal", LOG_CONS, LOG_LOCAL0);

    m_strPath.clear();
    m_strParameter.clear();
    m_openHandle  = 0;
    m_deviceFlags = 0;
    m_bAsync      = false;
}

VscpCanalDeviceIf::~VscpCanalDeviceIf()
{
    // Close syslog
    closelog();
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int
VscpCanalDeviceIf::init()
{
    const char *dlsym_error;

    m_openHandle = 0;

    // Load dynamic library
    m_hdll = dlopen(m_strPath.c_str(), RTLD_LAZY);
    if (!m_hdll) {
        syslog(LOG_ERR,
               "Devicethread: Unable to load dynamic library. path = %s",
               m_strPath.c_str());
        return CANAL_ERROR_PARAMETER;
    }

    // Now find methods in library
    syslog(LOG_INFO, "Loading level I driver: %s", m_strPath.c_str());

    // * * * * CANAL OPEN * * * *
    m_proc_CanalOpen = (LPFNDLL_CANALOPEN)dlsym(m_hdll, "CanalOpen");
    dlsym_error      = dlerror();

    if (dlsym_error) {
        // Free the library
        syslog(LOG_DEBUG,
               "%s : Unable to get dl entry for CanalOpen.",
               m_strPath.c_str());
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL CLOSE * * * *
    m_proc_CanalClose = (LPFNDLL_CANALCLOSE)dlsym(m_hdll, "CanalClose");
    dlsym_error       = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalClose.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GETLEVEL * * * *
    m_proc_CanalGetLevel =
      (LPFNDLL_CANALGETLEVEL)dlsym(m_hdll, "CanalGetLevel");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetLevel.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL SEND * * * *
    m_proc_CanalSend = (LPFNDLL_CANALSEND)dlsym(m_hdll, "CanalSend");
    dlsym_error      = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalSend.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL DATA AVAILABLE * * * *
    m_proc_CanalDataAvailable =
      (LPFNDLL_CANALDATAAVAILABLE)dlsym(m_hdll, "CanalDataAvailable");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalDataAvailable.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL RECEIVE * * * *
    m_proc_CanalReceive = (LPFNDLL_CANALRECEIVE)dlsym(m_hdll, "CanalReceive");
    dlsym_error         = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalReceive.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GET STATUS * * * *
    m_proc_CanalGetStatus =
      (LPFNDLL_CANALGETSTATUS)dlsym(m_hdll, "CanalGetStatus");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetStatus.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GET STATISTICS * * * *
    m_proc_CanalGetStatistics =
      (LPFNDLL_CANALGETSTATISTICS)dlsym(m_hdll, "CanalGetStatistics");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetStatistics.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL SET FILTER * * * *
    m_proc_CanalSetFilter =
      (LPFNDLL_CANALSETFILTER)dlsym(m_hdll, "CanalSetFilter");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalSetFilter.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL SET MASK * * * *
    m_proc_CanalSetMask = (LPFNDLL_CANALSETMASK)dlsym(m_hdll, "CanalSetMask");
    dlsym_error         = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalSetMask.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GET VERSION * * * *
    m_proc_CanalGetVersion =
      (LPFNDLL_CANALGETVERSION)dlsym(m_hdll, "CanalGetVersion");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetVersion.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GET DLL VERSION * * * *
    m_proc_CanalGetDllVersion =
      (LPFNDLL_CANALGETDLLVERSION)dlsym(m_hdll, "CanalGetDllVersion");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetDllVersion.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GET VENDOR STRING * * * *
    m_proc_CanalGetVendorString =
      (LPFNDLL_CANALGETVENDORSTRING)dlsym(m_hdll, "CanalGetVendorString");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetVendorString.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // ******************************
    //     Generation 2 Methods
    // ******************************

    // * * * * CANAL BLOCKING SEND * * * *
    m_proc_CanalBlockingSend =
      (LPFNDLL_CANALBLOCKINGSEND)dlsym(m_hdll, "CanalBlockingSend");
    dlsym_error = dlerror();
    if (dlsym_error) {
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalBlockingSend. Probably "
               "Generation 1 driver.",
               m_strPath.c_str());
        m_proc_CanalBlockingSend = NULL;
    }

    // * * * * CANAL BLOCKING RECEIVE * * * *
    m_proc_CanalBlockingReceive =
      (LPFNDLL_CANALBLOCKINGRECEIVE)dlsym(m_hdll, "CanalBlockingReceive");
    dlsym_error = dlerror();
    if (dlsym_error) {
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalBlockingReceive. "
               "Probably Generation 1 driver.",
               m_strPath.c_str());
        m_proc_CanalBlockingReceive = NULL;
    }

    // * * * * CANAL GET DRIVER INFO * * * *
    m_proc_CanalGetdriverInfo =
      (LPFNDLL_CANALGETDRIVERINFO)dlsym(m_hdll, "CanalGetDriverInfo");
    dlsym_error = dlerror();
    if (dlsym_error) {
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetDriverInfo. "
               "Probably Generation 1 driver.",
               m_strPath.c_str());
        m_proc_CanalGetdriverInfo = NULL;
    }

    return CANAL_ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------

int depth_init_parser = 0;

static void
startInitXMLParser(void *data, const char *name, const char **attr)
{
    VscpCanalDeviceIf *pObj = (VscpCanalDeviceIf *)data;
    if (NULL == pObj) return;

    if ((0 == strcmp(name, "init")) && (0 == depth_init_parser)) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            if (0 == strcmp(attr[i], "path")) {
                pObj->setPath(attribute);
            } else if (0 == strcmp(attr[i], "config")) {
                pObj->setParameter(attribute);
            } else if (0 == strcmp(attr[i], "flags")) {
                pObj->setFlags(vscp_readStringValue(attribute));
            } else if (0 == strcmp(attr[i], "basync")) {
                for (auto &c : attribute) c = toupper(c);
                if (std::string::npos != attribute.find("TRUE")) {
                    pObj->setbAsync(true);
                } else {
                    pObj->setbAsync(false);
                }
            }
        }
    }

    depth_init_parser++;
}

static void
endInitXMLParser(void *data, const char *name)
{
    depth_init_parser--;
}

int
VscpCanalDeviceIf::init(std::string objInit, uint8_t nFormat)
{
    if (CANAL_FORMAT_CAN_XML == nFormat) {

        XML_Parser xmlParser = XML_ParserCreate("UTF-8");
        XML_SetUserData(xmlParser, this);
        XML_SetElementHandler(
          xmlParser, startInitXMLParser, endInitXMLParser);

        int bytes_read;
        void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);

        strncpy((char *)buf, objInit.c_str(), objInit.length());

        bytes_read = objInit.length();
        if (!XML_ParseBuffer(xmlParser, bytes_read, bytes_read == 0)) {
            return false;
        }

        XML_ParserFree(xmlParser);

    } else if (CANAL_FORMAT_CAN_JSON == nFormat) {
        try {
            auto j = json::parse(objInit.c_str());

            // Path
            if (j.find("path") != j.end()) {
                m_strPath = j.at("path").get<std::string>();
            }

            // config
            if (j.find("path") != j.end()) {
                m_strPath = j.at("path").get<std::string>();
            }

            // flags
            if (j.find("path") != j.end()) {
                m_strParameter = j.at("path").get<uint32_t>();
            }

            // bAsync
            if (j.find("path") != j.end()) {
                m_bAsync = j.at("path").get<bool>();
            }

        } catch (...) {
            return CANAL_ERROR_INIT_FAIL;
        }
    }
    else {
        return CANAL_ERROR_NOT_SUPPORTED;
    }

    return init();
}

// -------------------------------------------------

int
VscpCanalDeviceIf::init(std::string strpath,
                        std::string strparam,
                        uint32_t flags,
                        bool bAsync)
{
    // Save config data
    m_strPath      = strpath;
    m_strParameter = strparam;
    m_deviceFlags  = flags;
    m_bAsync       = m_bAsync;

    return init();
}

///////////////////////////////////////////////////////////////////////////////
// CanalOpen
//

int
VscpCanalDeviceIf::CanalOpen()
{
    // Must NOT be open
    if (0 != m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    // Open the device
    m_openHandle =
      m_proc_CanalOpen((const char *)m_strParameter.c_str(), m_deviceFlags);

    // Check if the driver opened properly
    if (m_openHandle <= 0) {
        syslog(LOG_ERR,
               "Failed to open driver. Will not use it! %ld [%s] ",
               m_openHandle,
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_NOT_OPEN;
    }

    // Get Driver Level
    // m_driverLevel = m_proc_CanalGetLevel(m_openHandle);

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalClose
//

int
VscpCanalDeviceIf::CanalClose()
{

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalClose(m_openHandle);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSend
//

int
VscpCanalDeviceIf::CanalSend(const PCANALMSG pCanMsg)
{
    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalSend(m_openHandle, pCanMsg);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

// https://stackoverflow.com/questions/45934851/c-nlohmann-json-how-to-iterate-find-a-nested-object
template<class UnaryFunction>
void recursive_iterate(const json& j, UnaryFunction f)
{
    for(auto it = j.begin(); it != j.end(); ++it)
    {
        if (it->is_structured())
        {
            recursive_iterate(*it, f);
        }
        else
        {
            f(it);
        }
    }
}

//-----------------------------------------------------------------------------

/*
<canal flags="1234"
            id="1234"
            obid="1234"
            dataSize="1234"
            data="1,2,3,4..."
            timestamp="1234"  />
*/

int depth_canal_parser = 0;

static void
startCanalXMLParser(void *data, const char *name, const char **attr)
{
    canalMsg *pmsg = (canalMsg *)data;
    if (NULL == pmsg) return;

    if ((0 == strcmp(name, "canal")) && (0 == depth_canal_parser)) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            if (0 == strcmp(attr[i], "flags")) {
                pmsg->flags = vscp_readStringValue(attribute);
            } else if (0 == strcmp(attr[i], "id")) {
                pmsg->id = vscp_readStringValue(attribute);
            } else if (0 == strcmp(attr[i], "obid")) {
                pmsg->obid = vscp_readStringValue(attribute);
            } else if (0 == strcmp(attr[i], "timestamp")) {
                pmsg->timestamp = vscp_readStringValue(attribute);
            } else if (0 == strcmp(attr[i], "dataSize")) {
                pmsg->sizeData = vscp_readStringValue(attribute);
                pmsg->sizeData = MIN(pmsg->sizeData,8);
            } else if (0 == strcmp(attr[i], "data")) {
                std::string str = attribute;
                uint16_t sz;
                uint8_t buf[VSCP_MAX_DATA];
                memset(buf, 0, sizeof(buf));
                vscp_setVscpDataArrayFromString(buf, &sz, str);
                memcpy( pmsg->data, buf, pmsg->sizeData );
            }
        }
    }

    depth_canal_parser++;

}

static void
endCanalXMLParser(void *data, const char *name)
{
    depth_canal_parser--;
}

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// constructCanalMsg
//
// Construct CANAL msg from CANAL XML/JSON or VSCP XML/JSON
//

bool
VscpCanalDeviceIf::constructCanalMsg( canalMsg *pmsg,
                                        std::string& strObj,
                                        uint8_t nFormat )
{
    vscpEventEx ex;

    if ( NULL == pmsg ) {
        return CANAL_ERROR_PARAMETER;
    }

    memset( pmsg, 0, sizeof(canalMsg) );
    memset( &ex, 0, sizeof(vscpEventEx) );

    if (CANAL_FORMAT_CAN_XML == nFormat) {

        XML_Parser xmlParser = XML_ParserCreate("UTF-8");
        XML_SetUserData(xmlParser, pmsg);
        XML_SetElementHandler(
          xmlParser, startCanalXMLParser, endCanalXMLParser);

        int bytes_read;
        void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);

        strncpy((char *)buf, strObj.c_str(), strObj.length());

        bytes_read = strObj.length();
        if (!XML_ParseBuffer(xmlParser, bytes_read, bytes_read == 0)) {
            return false;
        }

        XML_ParserFree(xmlParser);

    } else if (CANAL_FORMAT_CAN_JSON == nFormat) {
        try {
            bool bVSCP = false;
            auto j = json::parse(strObj.c_str());

            // !!!! DO NOT REMOVE !!!!
            //recursive_iterate(j, [](json::const_iterator it) {
            //    std::cout << *it << std::endl;
            //});

            // can_flags
            if (j.find("flags") != j.end()) {
                pmsg->flags = j.at("flags").get<unsigned long>();
            }

            // obid
            if (j.find("obid") != j.end()) {
                pmsg->obid = j.at("obid").get<unsigned long>();
            }

            // id
            if (j.find("id") != j.end()) {
                pmsg->id = j.at("id").get<unsigned long>();
            }

            // timestamp
            if (j.find("timestamp") != j.end()) {
                pmsg->timestamp = j.at("timestamp").get<unsigned long>();
            }

            // sizeData
            if (j.find("sizeData") != j.end()) {
                pmsg->sizeData = j.at("sizeData").get<uint8_t>();
            }

            // data
            if (j.find("data") != j.end()) {
                std::vector<uint8_t> vecData = j.at("data").get<std::vector<uint8_t>>();
                pmsg->sizeData =  MIN( ex.sizeData, 8 );
                for (int i=0; i<8;i++) {
                    pmsg->data[i] = vecData[i];
                }
            }

        } catch (...) {
            return false;
        }
    }
    else if ( CANAL_FORMAT_VSCP_XML == nFormat ) {

        if ( !vscp_convertXMLToEventEx(strObj, &ex) ) {
            return false;
        }

    }
    else if ( CANAL_FORMAT_VSCP_JSON == nFormat ) {

        if ( !vscp_convertJSONToEventEx(strObj, &ex) ) {
            return false;
        }

    }

    return true;
}

int
VscpCanalDeviceIf::CanalSend(std::string& strObj, uint8_t nFormat)
{
    canalMsg msg;
    memset(&msg, 0, sizeof(canalMsg) );

    if ( !constructCanalMsg( &msg, strObj, nFormat ) ) {
        return CANAL_ERROR_PARAMETER;
    }

    return m_proc_CanalSend(m_openHandle, &msg);
}

///////////////////////////////////////////////////////////////////////////////
// CanalBlockingSend
//

int
VscpCanalDeviceIf::CanalBlockingSend( canalMsg *pmsg,
                                        uint32_t timeout )
{
    // Check pointer
    if ( NULL == pmsg ) {
        return CANAL_ERROR_PARAMETER;
    }

    // Check if generation 2
    if (NULL == m_proc_CanalBlockingSend) {
        return CANAL_ERROR_LIBRARY;
    }

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    return m_proc_CanalBlockingSend(m_openHandle, pmsg, timeout);
}

int
VscpCanalDeviceIf::CanalBlockingSend( std::string &strObj,
                                        uint32_t timeout,
                                        uint8_t nFormat)
{
    canalMsg msg;
    memset(&msg, 0, sizeof(canalMsg) );;

    // Check if generation 2
    if (NULL == m_proc_CanalBlockingSend) {
        return CANAL_ERROR_LIBRARY;
    }

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    if ( !constructCanalMsg( &msg, strObj, nFormat ) ) {
        return CANAL_ERROR_PARAMETER;
    }

    return m_proc_CanalBlockingSend(m_openHandle, &msg, timeout);
}


bool VscpCanalDeviceIf::CanalToFormatedEvent( std::string& strObj,
                                                canalMsg *pmsg,
                                                uint8_t nFormat )
{
    char buf[2048];

    std::string strdata;
    for (int i=0; i<pmsg->sizeData; i++ ) {
        strdata.append(vscp_str_format("%d",pmsg->data[i]));
    }

    if ( CANAL_FORMAT_CAN_XML == nFormat ) {
        sprintf( buf,
                    CANAL_XML_MSG_TEMPLATE,
                    /* CAN */
                    pmsg->flags,
                    pmsg->id,
                    pmsg->obid,
                    pmsg->sizeData,
                    strdata.c_str(),
                    pmsg->timestamp );
        strObj = buf;
    } else if ( CANAL_FORMAT_CAN_JSON == nFormat ) {
        sprintf( buf,
                    CANAL_JSON_MSG_TEMPLATE,
                    /* CAN */
                    pmsg->flags,
                    pmsg->id,
                    pmsg->obid,
                    pmsg->sizeData,
                    strdata.c_str(),
                    pmsg->timestamp );
        strObj = buf;
    }
    else if ( CANAL_FORMAT_VSCP_XML == nFormat ) {
        vscpEventEx ex;

        uint8_t guid[15];
        memset( guid, 0, 16 );

        if ( !vscp_convertCanalToEventEx( &ex, pmsg, guid ) ) {
            return false;
        }

        if ( !vscp_convertEventExToXML( &ex, strObj) ) {
            return false;
        }

    }
    else if ( CANAL_FORMAT_VSCP_JSON == nFormat ) {
        vscpEventEx ex;

        uint8_t guid[15];
        memset( guid, 0, 16 );

        if ( !vscp_convertCanalToEventEx( &ex, pmsg, guid ) ) {
            return false;
        }

        if ( !vscp_convertEventExToJSON( &ex, strObj) ) {
            return false;
        }
    }
    else {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// CanalReceive
//

int
VscpCanalDeviceIf::CanalReceive(canalMsg *pmsg)
{
    // Check pointer
    if (NULL == pmsg ) {
        return CANAL_ERROR_PARAMETER;
    }

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalReceive(m_openHandle, pmsg);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}


int
VscpCanalDeviceIf::CanalReceive(std::string& strObj, uint8_t nFormat)
{
    canalMsg CanMsg;

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalReceive(m_openHandle, &CanMsg);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }

    if ( !CanalToFormatedEvent( strObj, &CanMsg, nFormat ) ) {
        return CANAL_ERROR_CAN_MESSAGE;
    }

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalBlockingReceive
//

int
VscpCanalDeviceIf::CanalBlockingReceive(canalMsg *pmsg, uint32_t timeout)
{
    // Check pointer
    if (NULL == pmsg ) {
        return CANAL_ERROR_PARAMETER;
    }

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalBlockingReceive(m_openHandle, pmsg, timeout);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }

    return CANAL_ERROR_SUCCESS;
}

int
VscpCanalDeviceIf::CanalBlockingReceive(std::string &strObj,
                                        uint32_t timeout,
                                        uint8_t nFormat)
{
    canalMsg CanMsg;

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalBlockingReceive(m_openHandle, &CanMsg, timeout);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }

    if ( !CanalToFormatedEvent( strObj, &CanMsg, nFormat ) ) {
        return CANAL_ERROR_CAN_MESSAGE;
    }

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalDataAvailable
//

int
VscpCanalDeviceIf::CanalDataAvailable()
{
    int rv = m_proc_CanalDataAvailable(m_openHandle);
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatus
//

#define CANAL_STATUS_XML_TEMPLATE \
    "<canal_statatus channel_status=\"%lu\" " \
        "lasterrorcode=\"%lu\" " \
        "lasterrorsubcode=\"%lu\" " \
        "lasterrorstr=\"%s\"  />"

#define CANAL_STATUS_JSON_TEMPLATE \
    "{ " \
    "channel_status       : %lu " \
    "lasterrorcode        : %lu " \
    "lasterrorsubcode     : %lu " \
    "lasterrorstr         : %s " \
    "}"

int
VscpCanalDeviceIf::CanalGetStatus(std::string &objStatus, uint8_t nFormat)
{
    canalStatus status;

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalGetStatus(m_openHandle, &status);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }

    char buf[512];
    if ( CANAL_FORMAT_CAN_JSON == nFormat ) {
        sprintf( buf,
                    CANAL_STATUS_JSON_TEMPLATE,
                    status.channel_status,
                    status.lasterrorcode,
                    status.lasterrorsubcode,
                    status.lasterrorstr  );
        objStatus = buf;
    }
    else {  // XML-format for everything else
        sprintf( buf,
                    CANAL_STATUS_XML_TEMPLATE,
                    status.channel_status,
                    status.lasterrorcode,
                    status.lasterrorsubcode,
                    status.lasterrorstr );
        objStatus = buf;
    }

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatistics
//

#define CANAL_STATISTICS_XML_TEMPLATE \
    "<canal_statistics cntReceiveFrames=\"%lu\" " \
        "cntTransmittFrames=\"%lu\" " \
        "cntReceiveData=\"%lu\" " \
        "cntTransmittData=\"%lu\" " \
        "cntOverruns=\"%lu\" " \
        "cntBusWarnings=\"%lu\" " \
        "cntBusOff=\"%lu\" />"

#define CANAL_STATISTICS_JSON_TEMPLATE \
    "{ " \
    "cntReceiveFrames   : %lu " \
    "cntTransmittFrames : %lu " \
    "cntReceiveData     : %lu " \
    "cntTransmittData   : %lu " \
    "cntOverruns        : %lu " \
    "cntBusWarnings     : %lu " \
    "cntBusOff          : %lu "\
    "}"

int
VscpCanalDeviceIf::CanalGetStatistics( std::string& objStatistics,
                                        uint8_t nFormat)
{
    canalStatistics CanalStatistics;

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalGetStatistics(m_openHandle, &CanalStatistics);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }

    char buf[512];
    if ( CANAL_FORMAT_CAN_JSON == nFormat ) {
        sprintf( buf,
                    CANAL_STATISTICS_JSON_TEMPLATE,
                    CanalStatistics.cntReceiveFrames,
                    CanalStatistics.cntTransmitFrames,
                    CanalStatistics.cntReceiveData,
                    CanalStatistics.cntTransmitData,
                    CanalStatistics.cntTransmitData,
                    CanalStatistics.cntBusWarnings,
                    CanalStatistics.cntBusOff );
        objStatistics = buf;
    }
    else {  // XML-format for everything else
        sprintf( buf,
                    CANAL_STATISTICS_XML_TEMPLATE,
                    CanalStatistics.cntReceiveFrames,
                    CanalStatistics.cntTransmitFrames,
                    CanalStatistics.cntReceiveData,
                    CanalStatistics.cntTransmitData,
                    CanalStatistics.cntTransmitData,
                    CanalStatistics.cntBusWarnings,
                    CanalStatistics.cntBusOff );
        objStatistics = buf;
    }

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetFilter
//

int
VscpCanalDeviceIf::CanalSetFilter(uint32_t filter)
{
    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalSetFilter(m_openHandle, filter);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetMask
//

int
VscpCanalDeviceIf::CanalSetMask(uint32_t mask)
{
    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalSetMask(m_openHandle, mask);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetBaudrate
//

int
VscpCanalDeviceIf::CanalSetBaudrate(uint32_t baudrate)
{
    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalSetBaudrate(m_openHandle, baudrate);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVersion
//

uint32_t
VscpCanalDeviceIf::CanalGetVersion(void)
{
    return m_proc_CanalGetVersion();
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetDllVersion
//

uint32_t
VscpCanalDeviceIf::CanalGetDllVersion(void)
{
    return m_proc_CanalGetDllVersion();
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVendorString
//

const char *
VscpCanalDeviceIf::CanalGetVendorString(void)
{
    return m_proc_CanalGetVendorString();
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetDriverInfo
//

const char *
VscpCanalDeviceIf::CanalGetDriverInfo(void)
{
    // Check if generation 2
    if (NULL == m_proc_CanalGetdriverInfo) {
        return NULL;
    }

    return m_proc_CanalGetdriverInfo();
}