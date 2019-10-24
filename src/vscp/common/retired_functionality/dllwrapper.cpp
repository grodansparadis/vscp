///////////////////////////////////////////////////////////////////////////////
// DllWrapper.cpp: implementation of the CDllWrapper class.
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

#if WIN32
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <windows.h>
#else
#define _POSIX
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

#include "wx/defs.h"
#include "wx/wx.h"

#endif

#include "dllwrapper.h"
#include <vscp_class.h>
#include <vscp_type.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDllWrapper::CDllWrapper()
{
    m_bInit = false;
    m_devid = 0;

    // Init. register read parameters
    m_registerReadErrorTimeout  = DLL_REGISTER_READ_ERROR_TIMEOUT;
    m_registerReadResendTimeout = DLL_REGISTER_READ_RESEND_TIMEOUT;
    m_registerReadMaxRetries    = DLL_REGISTER_READ_MAX_TRIES;
}

CDllWrapper::~CDllWrapper()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// initialize
//

int
CDllWrapper::initialize(const wxString &strPath)
{
    // Check that the file exists
    if (!(::wxFileExists(strPath))) return CANAL_ERROR_PARAMETER;

    // Save the path
    m_strPath = strPath;

    // Load dynamic library
    if (!m_wxdll.Load(strPath, wxDL_LAZY)) {
        wxLogDebug(_("Unable to load dynamic library."));
        return CANAL_ERROR_PARAMETER;
    }

    // Now find methods in library

    // * * * * CANAL OPEN * * * *
    if (NULL == (m_proc_CanalOpen =
                   (LPFNDLL_CANALOPEN)m_wxdll.GetSymbol(_T ( "CanalOpen" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalOpen."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL CLOSE * * * *
    if (NULL == (m_proc_CanalClose = (LPFNDLL_CANALCLOSE)m_wxdll.GetSymbol(
                   _T ( "CanalClose" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalClose."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL GETLEVEL * * * *
    if (NULL ==
        (m_proc_CanalGetLevel =
           (LPFNDLL_CANALGETLEVEL)m_wxdll.GetSymbol(_T ( "CanalGetLevel" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetLevel."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL SEND * * * *
    if (NULL == (m_proc_CanalSend =
                   (LPFNDLL_CANALSEND)m_wxdll.GetSymbol(_T ( "CanalSend" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalSend."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL DATA AVAILABLE * * * *
    if (NULL == (m_proc_CanalDataAvailable =
                   (LPFNDLL_CANALDATAAVAILABLE)m_wxdll.GetSymbol(
                     _T ( "CanalDataAvailable" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalDataAvailable."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL RECEIVE * * * *
    if (NULL == (m_proc_CanalReceive = (LPFNDLL_CANALRECEIVE)m_wxdll.GetSymbol(
                   _T ( "CanalReceive" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalReceive."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL GET STATUS * * * *
    if (NULL ==
        (m_proc_CanalGetStatus = (LPFNDLL_CANALGETSTATUS)m_wxdll.GetSymbol(
           _T ( "CanalGetStatus" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetStatus."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL GET STATISTICS * * * *
    if (NULL == (m_proc_CanalGetStatistics =
                   (LPFNDLL_CANALGETSTATISTICS)m_wxdll.GetSymbol(
                     _T ( "CanalGetStatistics" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetStatistics."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL SET FILTER * * * *
    if (NULL ==
        (m_proc_CanalSetFilter = (LPFNDLL_CANALSETFILTER)m_wxdll.GetSymbol(
           _T ( "CanalSetFilter" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalSetFilter."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL SET MASK * * * *
    if (NULL == (m_proc_CanalSetMask = (LPFNDLL_CANALSETMASK)m_wxdll.GetSymbol(
                   _T ( "CanalSetMask" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalSetMask."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL GET VERSION * * * *
    if (NULL ==
        (m_proc_CanalGetVersion = (LPFNDLL_CANALGETVERSION)m_wxdll.GetSymbol(
           _T ( "CanalGetVersion" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetVersion."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL GET DLL VERSION * * * *
    if (NULL == (m_proc_CanalGetDllVersion =
                   (LPFNDLL_CANALGETDLLVERSION)m_wxdll.GetSymbol(
                     _T ( "CanalGetDllVersion" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetDllVersion."));
        return CANAL_ERROR_PARAMETER;
    }

    // * * * * CANAL GET VENDOR STRING * * * *
    if (NULL == (m_proc_CanalGetVendorString =
                   (LPFNDLL_CANALGETVENDORSTRING)m_wxdll.GetSymbol(
                     _T ( "CanalGetVendorString" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetVendorString."));
        return CANAL_ERROR_PARAMETER;
    }

    wxLogNull logNo;

    // ******************************
    //     Generation 2 Methods
    // ******************************

    // * * * * CANAL BLOCKING SEND * * * *
    if (NULL == (m_proc_CanalBlockingSend =
                   (LPFNDLL_CANALBLOCKINGSEND)m_wxdll.GetSymbol(
                     _T ( "CanalBlockingSend" )))) {
        wxLogDebug(_("Unable to get dl entry for CanalBlockingSend. Probably "
                     "Generation 1 driver."));
        m_proc_CanalBlockingSend = NULL;
    }

    // * * * * CANAL BLOCKING RECEIVE * * * *
    if (NULL == (m_proc_CanalBlockingReceive =
                   (LPFNDLL_CANALBLOCKINGRECEIVE)m_wxdll.GetSymbol(
                     _T ( "CanalBlockingReceive" )))) {
        wxLogDebug(_("Unable to get dl entry for CanalBlockingReceive. "
                     "Probably Generation 1 driver."));
        m_proc_CanalBlockingReceive = NULL;
    }

    // * * * * CANAL GET DRIVER INFO * * * *
    if (NULL == (m_proc_CanalGetdriverInfo =
                   (LPFNDLL_CANALGETDRIVERINFO)m_wxdll.GetSymbol(
                     _T ( "CanalGetDriverInfo" )))) {
        wxLogDebug(_("Unable to get dl entry for CanalGetDriverInfo. Probably "
                     "Generation 1 driver."));
        m_proc_CanalGetdriverInfo = NULL;
    }

    // Mark as initialized
    m_bInit = true;

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// loadGetDriverInfo
//

int
CDllWrapper::loadGetDriverInfo(const wxString &strPath,
                               wxString &strDrvInfo,
                               foundMetods *pFoundMethods)
{
    int rv = CANAL_ERROR_SUCCESS;

    // Must be there
    if (NULL == pFoundMethods) return CANAL_ERROR_PARAMETER;

    // Check that the file exists
    if (!(::wxFileExists(strPath))) return CANAL_ERROR_PARAMETER;

    // Initialize
    memset(pFoundMethods, 0, sizeof(foundMetods));

    // Load dynamic library
    if (!m_wxdll.Load(strPath, wxDL_LAZY)) {
        wxLogDebug(_("Unable to load dynamic library."));
        return CANAL_ERROR_PARAMETER;
    }

    // Now find methods in library

    // * * * * CANAL OPEN * * * *
    if (NULL == (m_proc_CanalOpen =
                   (LPFNDLL_CANALOPEN)m_wxdll.GetSymbol(_T( "CanalOpen" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalOpen."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalOpen = TRUE;

    // * * * * CANAL CLOSE * * * *
    if (NULL == (m_proc_CanalClose =
                   (LPFNDLL_CANALCLOSE)m_wxdll.GetSymbol(_T( "CanalClose" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalClose."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalClose = TRUE;

    // * * * * CANAL GETLEVEL * * * *
    if (NULL ==
        (m_proc_CanalGetLevel =
           (LPFNDLL_CANALGETLEVEL)m_wxdll.GetSymbol(_T( "CanalGetLevel" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetLevel."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalGetLevel = TRUE;

    // * * * * CANAL SEND * * * *
    if (NULL == (m_proc_CanalSend =
                   (LPFNDLL_CANALSEND)m_wxdll.GetSymbol(_T( "CanalSend" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalSend."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalSend = TRUE;

    // * * * * CANAL DATA AVAILABLE * * * *
    if (NULL == (m_proc_CanalDataAvailable =
                   (LPFNDLL_CANALDATAAVAILABLE)m_wxdll.GetSymbol(
                     _T( "CanalDataAvailable" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalDataAvailable."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalDataAvailable = TRUE;

    // * * * * CANAL RECEIVE * * * *
    if (NULL == (m_proc_CanalReceive = (LPFNDLL_CANALRECEIVE)m_wxdll.GetSymbol(
                   _T( "CanalReceive" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalReceive."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalReceive = TRUE;

    // * * * * CANAL GET STATUS * * * *
    if (NULL ==
        (m_proc_CanalGetStatus =
           (LPFNDLL_CANALGETSTATUS)m_wxdll.GetSymbol(_T( "CanalGetStatus" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetStatus."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalGetStatus = TRUE;

    // * * * * CANAL GET STATISTICS * * * *
    if (NULL == (m_proc_CanalGetStatistics =
                   (LPFNDLL_CANALGETSTATISTICS)m_wxdll.GetSymbol(
                     _T( "CanalGetStatistics" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetStatistics."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalGetStatistics = TRUE;

    // * * * * CANAL SET FILTER * * * *
    if (NULL ==
        (m_proc_CanalSetFilter =
           (LPFNDLL_CANALSETFILTER)m_wxdll.GetSymbol(_T( "CanalSetFilter" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalSetFilter."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalSetFilter = TRUE;

    // * * * * CANAL SET MASK * * * *
    if (NULL == (m_proc_CanalSetMask = (LPFNDLL_CANALSETMASK)m_wxdll.GetSymbol(
                   _T( "CanalSetMask" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalSetMask."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalSetMask = TRUE;

    // * * * * CANAL GET VERSION * * * *
    if (NULL ==
        (m_proc_CanalGetVersion = (LPFNDLL_CANALGETVERSION)m_wxdll.GetSymbol(
           _T( "CanalGetVersion" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetVersion."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalGetVersion = TRUE;

    // * * * * CANAL GET DLL VERSION * * * *
    if (NULL == (m_proc_CanalGetDllVersion =
                   (LPFNDLL_CANALGETDLLVERSION)m_wxdll.GetSymbol(
                     _T( "CanalGetDllVersion" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetDllVersion."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalGetDllVersion = TRUE;

    // * * * * CANAL GET VENDOR STRING * * * *
    if (NULL == (m_proc_CanalGetVendorString =
                   (LPFNDLL_CANALGETVENDORSTRING)m_wxdll.GetSymbol(
                     _T( "CanalGetVendorString" )))) {
        // Free the library
        wxLogDebug(_("Unable to get dl entry for CanalGetVendorString."));
        return CANAL_ERROR_PARAMETER;
    }
    pFoundMethods->bCanalGetVendorString = TRUE;

    // wxLogNull logNo;

    // ******************************
    //     Generation 2 Methods
    // ******************************

    // * * * * CANAL BLOCKING SEND * * * *
    if (NULL == (m_proc_CanalBlockingSend =
                   (LPFNDLL_CANALBLOCKINGSEND)m_wxdll.GetSymbol(
                     _T( "CanalBlockingSend" )))) {
        wxLogDebug(_("Unable to get dl entry for CanalBlockingSend. Probably "
                     "Generation 1 driver."));
        m_proc_CanalBlockingSend = NULL;
    } else {
        pFoundMethods->bCanalBlockingSend = TRUE;
    }

    // * * * * CANAL BLOCKING RECEIVE * * * *
    if (NULL == (m_proc_CanalBlockingReceive =
                   (LPFNDLL_CANALBLOCKINGRECEIVE)m_wxdll.GetSymbol(
                     _T( "CanalBlockingReceive" )))) {
        wxLogDebug(_("Unable to get dl entry for CanalBlockingReceive. "
                     "Probably Generation 1 driver."));
        m_proc_CanalBlockingReceive = NULL;
    } else {
        pFoundMethods->bCanalBlockingReceive = TRUE;
    }

    // * * * * CANAL GET DRIVER INFO * * * *
    if (NULL == (m_proc_CanalGetdriverInfo =
                   (LPFNDLL_CANALGETDRIVERINFO)m_wxdll.GetSymbol(
                     _T( "CanalGetDriverInfo" )))) {
        wxLogDebug(_("Unable to get dl entry for CanalGetDriverInfo. Probably "
                     "Generation 1 driver."));
        m_proc_CanalGetdriverInfo = NULL;
        rv                        = CANAL_ERROR_PARAMETER;
    } else {
        pFoundMethods->bCanalGetdriverInfo = TRUE;
    }

    if (pFoundMethods->bCanalGetdriverInfo) {
        const char *p = m_proc_CanalGetdriverInfo();
        if (NULL != p) {
            strDrvInfo = wxString::FromUTF8(p);
        }
    }

    // Unload the DLL
    m_wxdll.Unload();

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// open
//

long
CDllWrapper::doCmdOpen(const wxString &strConfiguration, unsigned long flags)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

#ifdef WIN32
    char szCfg[MAX_PATH];
    strncpy(szCfg, strConfiguration.mb_str(), MAX_PATH);
#else
    char szCfg[PATH_MAX];
    strncpy(szCfg, strConfiguration.mb_str(), PATH_MAX);
#endif
    // memcpy( szInterface, strInterface.ToAscii(), strInterface.Length() );
    // TODO

    return (m_devid = m_proc_CanalOpen(szCfg, flags));
}

long
CDllWrapper::doCmdOpen(const wxString &strPath,
                       const wxString &strConfiguration,
                       unsigned long flags)
{
    if (!m_bInit) {
        // Initialize
        if (CANAL_ERROR_SUCCESS != initialize(strPath)) {
            return -1;
        }
    }

    return doCmdOpen(strConfiguration, flags);
}

///////////////////////////////////////////////////////////////////////////////
// close
//

int
CDllWrapper::doCmdClose(void)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalClose(m_devid);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetLevel
//

unsigned long
CDllWrapper::doCmdGetLevel(void)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalGetLevel(m_devid);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

int
CDllWrapper::doCmdSend(canalMsg *pMsg)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalSend(m_devid, pMsg);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBlockingSend
//

int
CDllWrapper::doCmdBlockingSend(canalMsg *pMsg, unsigned long timeout)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    if (NULL == m_proc_CanalBlockingSend) return CANAL_ERROR_NOT_SUPPORTED;

    return m_proc_CanalBlockingSend(m_devid, pMsg, timeout);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int
CDllWrapper::doCmdReceive(canalMsg *pMsg)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalReceive(m_devid, pMsg);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBlockingReceive
//

int
CDllWrapper::doCmdBlockingReceive(canalMsg *pMsg, unsigned long timeout)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    if (NULL == m_proc_CanalBlockingReceive) return CANAL_ERROR_NOT_SUPPORTED;

    return m_proc_CanalBlockingReceive(m_devid, pMsg, timeout);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int
CDllWrapper::doCmdDataAvailable(void)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return 0;

    return m_proc_CanalDataAvailable(m_devid);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

int
CDllWrapper::doCmdStatus(canalStatus *pStatus)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalGetStatus(m_devid, pStatus);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int
CDllWrapper::doCmdStatistics(canalStatistics *pStatistics)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalGetStatistics(m_devid, pStatistics);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

int
CDllWrapper::doCmdFilter(unsigned long filter)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalSetFilter(m_devid, filter);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdMask
//

int
CDllWrapper::doCmdMask(unsigned long mask)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalSetMask(m_devid, mask);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBaudrate

int
CDllWrapper::doCmdBaudrate(unsigned long baudrate)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalSetBaudrate(m_devid, baudrate);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

unsigned long
CDllWrapper::doCmdVersion(void)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalGetVersion();
}

///////////////////////////////////////////////////////////////////////////////
// doCmdDLLVersion
//

unsigned long
CDllWrapper::doCmdDLLVersion(void)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return CANAL_ERROR_INIT_FAIL;

    return m_proc_CanalGetDllVersion();
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVendorString
//

const char *
CDllWrapper::doCmdVendorString(void)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return NULL;

    return m_proc_CanalGetVendorString();
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetDriverInfo
//

const char *
CDllWrapper::doCmdGetDriverInfo(void)
{
    // If not initialized there is nothing we can do
    if (!m_bInit) return NULL;

    if (NULL == m_proc_CanalGetdriverInfo) return NULL;

    return m_proc_CanalGetdriverInfo();
}

///////////////////////////////////////////////////////////////////////////////
// isBlockingSupported
//

bool
CDllWrapper::isBlockingSupported(bool bRead, bool bWrite)
{
    bool bReceiveBlockingSupported  = true;
    bool bTransmitBlockingSupported = true;

    if (bRead && (NULL == m_proc_CanalBlockingReceive)) {
        bReceiveBlockingSupported = false;
    }

    if (bWrite && (NULL == m_proc_CanalBlockingSend)) {
        bTransmitBlockingSupported = false;
    }

    return (bReceiveBlockingSupported && bTransmitBlockingSupported);
}

//////////////////////////////////////////////////////////////////////////////
// readLevel1Register
//

int
CDllWrapper::readLevel1Register(unsigned char nodeid,
                                unsigned short page,
                                unsigned char reg,
                                unsigned char *val)
{
    int rv = CANAL_ERROR_SUCCESS;
    wxString strBuf;
    unsigned long resendTime = m_registerReadResendTimeout;
    canalMsg canalSendEvent;
    canalMsg canalReceiveEvent;

    // Check pointer
    if (NULL == val) return CANAL_ERROR_PARAMETER;

    canalSendEvent.flags    = CANAL_IDFLAG_EXTENDED;
    canalSendEvent.obid     = 0;
    canalSendEvent.id       = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ << 8;
    canalSendEvent.sizeData = 5;
    canalSendEvent.data[0]  = nodeid;             // Node to read from
    canalSendEvent.data[1]  = (page >> 8) & 0xff; // MSB of page
    canalSendEvent.data[2]  = page & 0xff;        // LSB of page
    canalSendEvent.data[3]  = reg;                // Register to read
    canalSendEvent.data[4]  = 1;                  // Read one register

    doCmdSend(&canalSendEvent);

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while (true) {

        if (0 < doCmdDataAvailable()) { // Message available
            if (CANAL_ERROR_SUCCESS ==
                doCmdReceive(&canalReceiveEvent)) { // Valid event
                if (5 != canalReceiveEvent.sizeData) continue;
                if ((unsigned short)(canalReceiveEvent.id & 0xffff) ==
                    ((VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE << 8) +
                     nodeid)) { // Read reply?
                    if ((0 == canalReceiveEvent.data[0]) &&
                        ((page >> 8) == canalReceiveEvent.data[1]) &&
                        ((page & 0xff) == canalReceiveEvent.data[2]) &&
                        (canalReceiveEvent.data[3] ==
                         reg)) { // Requested register?

                        *val = canalReceiveEvent.data[4];
                        break;
                    }

                } // Check for correct reply event
            }
        } else {
            wxMilliSleep(2);
        }

        // Check for read error timeout
        if ((::wxGetLocalTimeMillis() - startTime) >
            m_registerReadErrorTimeout) {
            rv = CANAL_ERROR_TIMEOUT;
            break;
        }
        // Should we resend?
        else if ((::wxGetLocalTimeMillis() - startTime) > resendTime) {
            // Send again
            doCmdSend(&canalSendEvent);
            resendTime += m_registerReadResendTimeout;
        }

    } // while

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// readRegistersfromLevel1Device
//
// 1,3,7,15,31,63,127,255,  (2^n)-1  ((2^nPages)-1)
//

int
CDllWrapper::readRegistersfromLevel1Device(unsigned char nodeid,
                                           unsigned short reg,
                                           unsigned short page,
                                           unsigned char count,
                                           unsigned char *pRegs)
{
    int rv   = CANAL_ERROR_SUCCESS;
    int fcnt = 0;
    unsigned char data[256]; // This makes range checking simpler

    // Max 128 bytes can be read
    if (count > 128) return false;

    unsigned long resendTime = m_registerReadResendTimeout;
    canalMsg canalSendEvent;
    canalMsg canalReceiveEvent;

    canalSendEvent.flags    = CANAL_IDFLAG_EXTENDED;
    canalSendEvent.obid     = 0;
    canalSendEvent.id       = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ << 8;
    canalSendEvent.sizeData = 5;
    canalSendEvent.data[0]  = nodeid;      // Node to read from
    canalSendEvent.data[1]  = page >> 8;   // MSB of page
    canalSendEvent.data[2]  = page & 0xff; // LSB of page
    canalSendEvent.data[3]  = reg;         // Start register to read
    canalSendEvent.data[4]  = count;       // All 32 bytes

    // Send it away
    doCmdSend(&canalSendEvent);

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    // We should get eight response frames back
    unsigned long receive_flags = 0;
    unsigned char nPages        = count / 4;
    unsigned char lastpageCnt   = count % 4;
    if (lastpageCnt) nPages++;
    unsigned long allRcvValue = pow(2.0, nPages) - 1;

    while (allRcvValue !=
           (receive_flags &
            0xffffffff)) { // Mask for systems where long is > 32 bits

        if (0 < doCmdDataAvailable()) { // Message available

            if (CANAL_ERROR_SUCCESS ==
                doCmdReceive(&canalReceiveEvent)) { // Valid event

                // if ( 8 != canalEvent.sizeData ) continue;

                if ((unsigned short)(canalReceiveEvent.id & 0xffff) ==
                    ((VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE << 8) +
                     nodeid)) { // Read reply?

                    fcnt++;

                    // Mark frame as received
                    receive_flags |= (1 << canalReceiveEvent.data[0]);

                    if (lastpageCnt &&
                        (count - 1) == canalReceiveEvent.data[0]) {
                        // Last frame
                        for (int i = 0; i < lastpageCnt; i++) {
                            data[canalReceiveEvent.data[0] * 4 + i] =
                              canalReceiveEvent.data[4 + i];
                        }
                    } else {
                        for (int i = 0; i < 4; i++) {
                            data[canalReceiveEvent.data[0] * 4 + i] =
                              canalReceiveEvent.data[4 + i];
                        }
                    }

                } // Check for correct reply event
            }
        } else {
            wxMilliSleep(2);
        }

        if ((::wxGetLocalTimeMillis() - startTime) >
            m_registerReadErrorTimeout) {
            rv = CANAL_ERROR_TIMEOUT;
            goto error;
        }
    }

error:

    // Copy data on success
    if (CANAL_ERROR_SUCCESS == rv) {
        memcpy(pRegs, data, count);
    }

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// writeLevel1Register
//

int
CDllWrapper::writeLevel1Register(unsigned char nodeid,
                                 unsigned short page,
                                 unsigned char reg,
                                 unsigned char *pval)
{
    int rv = CANAL_ERROR_SUCCESS;
    wxString strBuf;
    unsigned long resendTime = m_registerReadResendTimeout;
    canalMsg canalSendEvent;
    canalMsg canalReceiveEvent;

    canalSendEvent.flags    = CANAL_IDFLAG_EXTENDED;
    canalSendEvent.obid     = 0;
    canalSendEvent.id       = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE << 8;
    canalSendEvent.sizeData = 5;
    canalSendEvent.data[0]  = nodeid;             // Node to read from
    canalSendEvent.data[1]  = (page >> 8) & 0xff; // MSB of page
    canalSendEvent.data[2]  = page & 0xff;        // LSB of page
    canalSendEvent.data[3]  = reg;                // Register to write
    canalSendEvent.data[4]  = *pval;              // Value

    doCmdSend(&canalSendEvent);

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while (true) {

        if (0 < doCmdDataAvailable()) { // Message available
            if (CANAL_ERROR_SUCCESS ==
                doCmdReceive(&canalReceiveEvent)) { // Valid event
                if (5 != canalReceiveEvent.sizeData) continue;
                if ((unsigned short)(canalReceiveEvent.id & 0xffff) ==
                    ((VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE << 8) +
                     nodeid)) { // Read reply?
                    if ((0 == canalReceiveEvent.data[0]) &&
                        ((page >> 8) == canalReceiveEvent.data[1]) &&
                        ((page & 0xff) == canalReceiveEvent.data[2]) &&
                        (canalReceiveEvent.data[3] ==
                         reg)) { // Requested register?

                        if (*pval != canalReceiveEvent.data[4])
                            rv = CANAL_ERROR_REGISTER;

                        // Save read value
                        *pval = canalReceiveEvent.data[4];
                        break;

                    } // Check for correct node

                    // Save read value
                    *pval = canalReceiveEvent.data[4];

                } // Check for correct reply event
            }
        } else {
            wxMilliSleep(2);
        }

        // Check for read error timeout
        if ((::wxGetLocalTimeMillis() - startTime) >
            m_registerReadErrorTimeout) {
            rv = CANAL_ERROR_TIMEOUT;
            break;
        }
        // Should we resend?
        else if ((::wxGetLocalTimeMillis() - startTime) > resendTime) {
            // Send again
            doCmdSend(&canalSendEvent);
            resendTime += m_registerReadResendTimeout;
        }

    } // while

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// getMDFUrlFromLevel1Device
//

bool
CDllWrapper::getMDFUrlFromLevel1Device(unsigned char nodeid, wxString &strurl)
{
    char url[33];
    unsigned char *p         = (unsigned char *)url;
    bool rv                  = true;
    unsigned long resendTime = m_registerReadResendTimeout;
    canalMsg canalSendEvent;
    canalMsg canalReceiveEvent;

    canalSendEvent.flags    = CANAL_IDFLAG_EXTENDED;
    canalSendEvent.obid     = 0;
    canalSendEvent.id       = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ << 8;
    canalSendEvent.sizeData = 5;
    canalSendEvent.data[0]  = nodeid; // Node to read from
    canalSendEvent.data[1]  = 0;      // MSB of page
    canalSendEvent.data[2]  = 0;      // LSB of page
    canalSendEvent.data[3]  = 0xE0;   // Register to write
    canalSendEvent.data[4]  = 32;     // All 32 bytes

    // Send it away
    doCmdSend(&canalSendEvent);

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    // We should get eight response frames back
    unsigned char receive_flags = 0;
    while (255 != receive_flags) {

        if (0 < doCmdDataAvailable()) { // Message available

            if (CANAL_ERROR_SUCCESS ==
                doCmdReceive(&canalReceiveEvent)) { // Valid event

                if (8 != canalReceiveEvent.sizeData) continue;

                if ((unsigned short)(canalReceiveEvent.id & 0xffff) ==
                    ((VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE << 8) +
                     nodeid)) { // Read reply?

                    // Check bounds
                    if (canalReceiveEvent.data[0] > 7) continue;

                    // Mark frame as received
                    receive_flags |= (1 << canalReceiveEvent.data[0]);

                    for (int i = 0; i < 4; i++) {
                        url[canalReceiveEvent.data[0] * 4 + i] =
                          canalReceiveEvent.data[4 + i];
                    }

                } // Check for correct reply event
            }
        } else {
            wxMilliSleep(2);
        }

        if ((::wxGetLocalTimeMillis() - startTime) >
            m_registerReadErrorTimeout) {
            rv = false;
            goto error;
        }
    }

    strurl = strurl.From8BitData(url);
    if (wxNOT_FOUND == strurl.Find(_("http://"))) {
        wxString str;
        str = _("http://");
        str += strurl;
        strurl = str;
    }

error:

    return rv;
}
