// VSCP helper dll.cpp : Defines the initialization routines for the DLL.
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

#ifdef WIN32
#include <winsock2.h>
#endif

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <vscpdatetime.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>
#ifdef WIN32
#include "dlldrvobj.h"
#include "vscphelperdll.h"
#else
#include "linux/libvscphelper.h"
#endif

#ifdef WIN32
#define DllExport __declspec(dllexport)
#endif

//-----------------------------------------------------------------------------
//                  T C P / I P  I N T E R F A C E
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------------
//
// To use any of the methods below this point you have to
// obtain a handle first with vscphlp_gethandle and remember
// to release it with vscphlp_releasehandle when you are done.
//
//-------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// vscphlp_newSession
//

#ifdef WIN32
extern "C" DllExport long WINAPI EXPORT
vscphlp_newSession(void)
#else
extern "C" long EXPORT
vscphlp_newSession(void)
#endif
{
    VscpRemoteTcpIf *pvscpif = new VscpRemoteTcpIf;
    if (NULL == pvscpif) return 0;

    return addDriverObject(pvscpif);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_closeSession
//

#ifdef WIN32
extern "C" DllExport void WINAPI EXPORT
vscphlp_closeSession(long handle)
#else
extern "C" void
vscphlp_closeSession(long handle)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL != pvscpif) {
        pvscpif->doCmdClose();
    }

    removeDriverObject(handle);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_setResponseTimeout
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setResponseTimeout(long handle, unsigned long timeout)
#else
extern "C" int
vscphlp_setResponseTimeout(long handle, unsigned long timeout)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    pvscpif->setResponseTimeout(timeout);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_setAfterCommandSleep
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setAfterCommandSleep(long handle, unsigned short sleeptime)
#else
extern "C" int
vscphlp_setAfterCommandSleep(long handle, unsigned short sleeptime)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    pvscpif->setAfterCommandSleep(sleeptime);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_isConnected
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_isConnected(const long handle)
#else
extern "C" int
vscphlp_isConnected(const long handle)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    return pvscpif->isConnected() ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_doCommand
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_doCommand(long handle, const char *cmd)
#else
extern "C" int
vscphlp_doCommand(long handle, const char *cmd)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string strCmd = std::string(cmd);
    return pvscpif->doCommand(strCmd);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_checkReply
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_checkReply(long handle, int bClear)
#else
extern "C" int
vscphlp_checkReply(long handle, int bClear)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->checkReturnValue(bClear ? true : false) ? VSCP_ERROR_SUCCESS
                                                            : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_clearLocalInputQueue
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_clearLocalInputQueue(long handle)
#else
extern "C" int
vscphlp_clearLocalInputQueue(long handle)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    pvscpif->doClrInputQueue();
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_OpenInterface
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_openInterface(long handle, const char *pInterface, unsigned long flags)
#else
extern "C" int
vscphlp_openInterface(long handle, const char *pInterface, unsigned long flags)
#endif
{
    std::string strInterface;
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    strInterface = std::string(pInterface);
    return pvscpif->doCmdOpen(strInterface, flags);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_open
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_open(const long handle,
             const char *pHostname,
             const char *pUsername,
             const char *pPassword)
#else

extern "C" int
vscphlp_open(const long handle,
             const char *pHostname,
             const char *pUsername,
             const char *pPassword)
#endif
{
    std::string strHostname;
    std::string strUsername;
    std::string strPassword;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    strHostname = std::string(pHostname);
    strUsername = std::string(pUsername);
    strPassword = std::string(pPassword);

    return pvscpif->doCmdOpen(strHostname, strUsername, strPassword);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_close
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_close(long handle)
#else
extern "C" int
vscphlp_close(long handle)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdClose();
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_noop
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_noop(long handle)
#else
extern "C" int
vscphlp_noop(long handle)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdNOOP();
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_clearDaemonEventQueue
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_clearDaemonEventQueue(long handle)
#else
extern "C" int
vscphlp_clearDaemonEventQueue(long handle)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdClear();
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_sendEvent
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_sendEvent(long handle, const vscpEvent *pEvent)
#else
extern "C" int
vscphlp_sendEvent(long handle, const vscpEvent *pEvent)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdSend(pEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_sendEventEx
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_sendEventEx(long handle, const vscpEventEx *pEvent)
#else
extern "C" int
vscphlp_sendEventEx(long handle, const vscpEventEx *pEvent)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdSendEx(pEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_receiveEvent
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_receiveEvent(long handle, vscpEvent *pEvent)
#else
extern "C" int
vscphlp_receiveEvent(long handle, vscpEvent *pEvent)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdReceive(pEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_receiveEventEx
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_receiveEventEx(long handle, vscpEventEx *pEvent)
#else
extern "C" int
vscphlp_receiveEventEx(long handle, vscpEventEx *pEvent)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdReceiveEx(pEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_blockingReceiveEvent
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_blockingReceiveEvent(long handle,
                             vscpEvent *pEvent,
                             unsigned long timeout)
#else
extern "C" int
vscphlp_blockingReceiveEvent(long handle,
                             vscpEvent *pEvent,
                             unsigned long timeout)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdBlockingReceive(pEvent, timeout);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_blockingReceiveEventEx
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_blockingReceiveEventEx(long handle,
                               vscpEventEx *pEvent,
                               unsigned long timeout)
#else
extern "C" int
vscphlp_blockingReceiveEventEx(long handle,
                               vscpEventEx *pEvent,
                               unsigned long timeout)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdBlockingReceive(pEvent, timeout);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_isDataAvailable
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_isDataAvailable(long handle, unsigned int *pCount)
#else
extern "C" int
vscphlp_isDataAvailable(long handle, unsigned int *pCount)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    *pCount = pvscpif->doCmdDataAvailable();

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getStatus
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getStatus(long handle, VSCPStatus *pStatus)
#else
extern "C" int
vscphlp_getStatus(long handle, VSCPStatus *pStatus)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdStatus(pStatus);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getStatistics
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getStatistics(long handle, VSCPStatistics *pStatistics)
#else
extern "C" int
vscphlp_getStatistics(long handle, canalStatistics *pStatistics)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdStatistics(pStatistics);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_setFilter
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setFilter(long handle, const vscpEventFilter *pFilter)
#else
extern "C" int
vscphlp_setFilter(long handle, const vscpEventFilter *pFilter)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdFilter(pFilter);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getVersion
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getVersion(long handle,
                   unsigned char *pMajorVer,
                   unsigned char *pMinorVer,
                   unsigned char *pSubMinorVer)
#else
extern "C" int
vscphlp_getVersion(long handle,
                   unsigned char *pMajorVer,
                   unsigned char *pMinorVer,
                   unsigned char *pSubMinorVer)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdVersion(pMajorVer, pMinorVer, pSubMinorVer);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getDLLVersion
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getDLLVersion(long handle, unsigned long *pVersion)
#else
extern "C" int
vscphlp_getDLLVersion(long handle, unsigned long *pVersion)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    *pVersion = pvscpif->doCmdDLLVersion();

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getVendorString
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getVendorString(long handle, char *pVendorStr, size_t len)
#else
extern "C" int
vscphlp_getVendorString(long handle, char *pVendorStr, size_t len)
#endif
{
    if (NULL == pVendorStr) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string str = std::string(pvscpif->doCmdVendorString());
    memset(pVendorStr, 0, len);
    strncpy(pVendorStr, str.c_str(), std::min(strlen(str.c_str()), len));

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getDriverInfo
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getDriverInfo(long handle, char *pInfoStr, size_t len)
#else
extern "C" int
vscphlp_getDriverInfo(long handle, char *pInfoStr, size_t len)
#endif
{
    if (NULL == pInfoStr) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string str = std::string(pvscpif->doCmdGetDriverInfo());
    memset(pInfoStr, 0, len);
    strncpy(pInfoStr, str.c_str(), std::min(strlen(str.c_str()), len));

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getGUID
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getGUID(long handle, unsigned char *pGUID)
#else
extern "C" int
vscphlp_getGUID(long handle, unsigned char *pGUID)
#endif
{
    if (NULL == pGUID) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    cguid GUID;
    int rv = pvscpif->doCmdGetGUID(GUID);
    memcpy(pGUID, GUID.getGUID(), 16);

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_setGUID
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setGUID(long handle, const unsigned char *pGUID)
#else
extern "C" int
vscphlp_setGUID(long handle, const unsigned char *pGUID)
#endif
{
    if (NULL == pGUID) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    cguid guid;
    guid.getFromArray(pGUID);

    return pvscpif->doCmdSetGUID(guid);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_shutDownServer
//
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_shutDownServer(long handle)
#else
extern "C" int
vscphlp_shutDownServer(long handle)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdShutDown();
}

/*!
    This command sets an open interface in the receive loop (RCVLOOP).
    It does nothing other then putting the interface in the loop mode and
    checking that it went there.

    Note! The only way to terminate this receive loop is to close the session
   with 'CLOSE' or sending 'QUITLOOP' . \param handle to server object \return
   CANAL_ERROR_SUCCESS if success CANAL_ERROR_GENERIC if not.
 */
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_enterReceiveLoop(const long handle)
#else
extern "C" int
vscphlp_enterReceiveLoop(const long handle)
#endif
{
    // Get VSCP TCP/IP object
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdEnterReceiveLoop();
}

/*!
    Quit the receiveloop
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_quitReceiveLoop(const long handle)
#else
extern "C" int
vscphlp_quitReceiveLoop(const long handle)
#endif
{
    // Get VSCP TCP/IP object
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdQuitReceiveLoop();
}

//-------------------------------------------------------------------------
//                                Variables
//-------------------------------------------------------------------------

/*!
    \fn int vscphlp_deleteVariable( long handle, const char *pName )
    \brief Delete a variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_deleteRemoteVariable(long handle, const char *pName)
#else
extern "C" int
vscphlp_deleteRemoteVariable(long handle, const char *pName)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->deleteRemoteVariable(name);
}

/*!
    \fn int vscphlp_deleteVariable( long handle, const char *pName )
    \brief Create variable
    \param name of variable
    \param type Either numerical (in string form) or as mennomic
    \param strValue Value to set for string
    \param bPersistent Non zero for persistent, zero if not.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_createRemoteVariable(long handle,
                             const char *pName,
                             const char *pType,
                             const int bPersistent,
                             const char *pUser,
                             const unsigned long rights,
                             const char *pValue,
                             const char *pNote)
#else
extern "C" int
vscphlp_createRemoteVariable(long handle,
                             const char *pName,
                             const char *pType,
                             const int bPersistent,
                             const char *pUser,
                             const unsigned long rights,
                             const char *pValue,
                             const char *pNote)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pType) return VSCP_ERROR_PARAMETER;
    if (NULL == pUser) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;
    if (NULL == pNote) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name  = std::string(pName);
    std::string type  = std::string(pType);
    std::string user  = std::string(pUser);
    std::string value = std::string(pValue);
    std::string note  = std::string(pNote);

    return pvscpif->createRemoteVariable(
      name, type, (bPersistent ? true : false), user, rights, value, note);
}

/*!
    \fn int vscphlp_saveVariablesToDisk( long handle, const char *pPath )
    \brief Save variables to disk
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_saveRemoteVariablesToDisk(long handle,
                                  const char *pPath,
                                  const int select,
                                  const char *pRegExp)
#else
extern "C" int
vscphlp_saveRemoteVariablesToDisk(long handle,
                                  const char *pPath,
                                  const int select,
                                  const char *pRegExp)
#endif
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string path = std::string(pPath);

    return pvscpif->saveRemoteVariablesToDisk(path);
}

/*!
    \fn bool vscphlp_getVariableString( const char *pName, char *pValue )
    \brief Get variable value from string variable
    \param name of variable
    \param pointer to string that get the value of the string variable.
    \param size for buffer
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableString(long handle,
                                const char *pName,
                                char *pValue,
                                size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableString(long handle,
                                const char *pName,
                                char *pValue,
                                size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string strValue;
    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableValue(name, strValue))) {
        memset(pValue, 0, len);
        strncpy(
          pValue, strValue.c_str(), std::min(strlen(strValue.c_str()), len));
    }

    return rv;
}

/*!
    \fn bool vscphlp_setVariableString( const char *pName, char *pValue )
    \brief set string variable from a pointer to a string
    \param name of variable
    \param pointer to string that contains the string.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableString(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableString(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name     = std::string(pName);
    std::string strValue = std::string(pValue);

    return pvscpif->setRemoteVariableValue(name, strValue);
}

/*!
    \fn bool vscphlp_getVariableValue( const char *pName, char *pValue )
    \brief Get variable value from string variable
    \param name of variable
    \param pointer to string that get the value of the string variable.
    \param size for buffer
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableValue(long handle,
                               const char *pName,
                               char *pValue,
                               size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableValue(long handle,
                               const char *pName,
                               char *pValue,
                               size_t len)
#endif
{
    return vscphlp_getRemoteVariableString(handle, pName, pValue, len);
}

/*!
    \fn bool vscphlp_setVariableValue( const char *pName, char *pValue )
    \brief set string variable from a pointer to a string
    \param name of variable
    \param pointer to string that contains the string.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableValue(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableValue(long handle, const char *pName, char *pValue)
#endif
{
    return vscphlp_setRemoteVariableString(handle, pName, pValue);
}

/*!
    \fn bool vscphlp_getVariableBool( const char *pName, bool *bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string
   variable. \return Returns VSCP_ERROR_SUCCESS on success, otherwise error
   code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableBool(long handle, const char *pName, int *bValue)
#else
extern "C" int
vscphlp_getRemoteVariableBool(long handle, const char *pName, int *bValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == bValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    bool bBoolValue;
    int rv = pvscpif->getRemoteVariableBool(name, &bBoolValue);
    if (bBoolValue) {
        *bValue = 1;
    } else {
        *bValue = 0;
    }
    return rv;
};

/*!
    \fn bool vscphlp_setVariableBool( const char *pName, bool bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string
   variable. \return Returns VSCP_ERROR_SUCCESS on success, otherwise error
   code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableBool(long handle, const char *pName, int bValue)
#else
extern "C" int
vscphlp_setRemoteVariableBool(long handle, const char *pName, int bValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableBool(name, (bValue ? true : false));
};

/*!
    \fn bool vscphlp_getVariableInt( const char *pName, int *value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string
   variable. \return Returns VSCP_ERROR_SUCCESS on success, otherwise error
   code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableInt(long handle, const char *pName, int *value)
#else
extern "C" int
vscphlp_getRemoteVariableInt(long handle, const char *pName, int *value)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == value) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->getRemoteVariableInt(name, value);
};

/*!
    \fn bool vscphlp_setVariableInt( const char *pName, int value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string
   variable. \return Returns VSCP_ERROR_SUCCESS on success, otherwise error
   code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableInt(long handle, const char *pName, int value)
#else
extern "C" int
vscphlp_setRemoteVariableInt(long handle, const char *pName, int value)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableInt(name, value);
};

/*!
    \fn bool vscphlp_getVariableLong( const char *pName, long *value )
    \brief Get variable value from long variable
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableLong(long handle, const char *pName, long *value)
#else
extern "C" int
vscphlp_getRemoteVariableLong(long handle, const char *pName, long *value)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == value) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->getRemoteVariableLong(name, value);
};

/*!
    \fn bool vscphlp_setVariableLong( const char *pName, long value )
    \brief Get variable value from long variable
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableLong(long handle, const char *pName, long value)
#else
extern "C" int
vscphlp_setRemoteVariableLong(long handle, const char *pName, long value)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableLong(name, value);
};

/*!
    \fn bool vscphlp_getVariableDouble( const char *pName, double *value )
    \brief Get variable value from double variable
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code..
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableDouble(long handle, const char *pName, double *value)
#else
extern "C" int
vscphlp_getRemoteVariableDouble(long handle, const char *pName, double *value)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == value) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->getRemoteVariableDouble(name, value);
};

/*!
    \fn bool vscphlp_setVariableDouble( const char *pName, double value )
    \brief Get variable value from double variable
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableDouble(long handle, const char *pName, double value)
#else
extern "C" int
vscphlp_setRemoteVariableDouble(long handle, const char *pName, double value)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableDouble(name, value);
};

/*!
    \fn bool vscphlp_getVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param name of variable
    \param String that get that get the
        value of the measurement.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableMeasurement(long handle,
                                     const char *pName,
                                     double *pvalue,
                                     int *punit,
                                     int *psensoridx,
                                     int *pzone,
                                     int *psubzone)
#else
extern "C" int
vscphlp_getRemoteVariableMeasurement(long handle,
                                     const char *pName,
                                     double *pvalue,
                                     int *punit,
                                     int *psensoridx,
                                     int *pzone,
                                     int *psubzone)
#endif
{
    int rv;
    uint8_t unit, sensoridx, zone, subzone;

    // Check pointers
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pvalue) return VSCP_ERROR_PARAMETER;
    if (NULL == punit) return VSCP_ERROR_PARAMETER;
    if (NULL == psensoridx) return VSCP_ERROR_PARAMETER;
    if (NULL == pzone) return VSCP_ERROR_PARAMETER;
    if (NULL == psubzone) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);

    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableMeasurement(
           name, pvalue, &unit, &sensoridx, &zone, &subzone))) {
        *punit      = unit;
        *psensoridx = sensoridx;
        *pzone      = zone;
        *psubzone   = subzone;
    }

    return rv;
}

/*!
    \fn bool vscphlp_setVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param name of variable
    \param String that get that get the
    value of the measurement.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableMeasurement(long handle,
                                     const char *pName,
                                     double value,
                                     int unit,
                                     int sensoridx,
                                     int zone,
                                     int subzone)
#else
extern "C" int
vscphlp_setRemoteVariableMeasurement(long handle,
                                     const char *pName,
                                     double value,
                                     int unit,
                                     int sensoridx,
                                     int zone,
                                     int subzone)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);

    return pvscpif->setRemoteVariableMeasurement(name,
                                                 value,
                                                 (uint8_t)unit,
                                                 (uint8_t)sensoridx,
                                                 (uint8_t)zone,
                                                 (uint8_t)subzone);
}

/*!
    \fn bool vscphlp_getVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableEvent(long handle,
                               const char *pName,
                               vscpEvent *pEvent)
#else
extern "C" int
vscphlp_getRemoteVariableEvent(long handle,
                               const char *pName,
                               vscpEvent *pEvent)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->getRemoteVariableEvent(name, pEvent);
}

/*!
    \fn bool vscphlp_setVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableEvent(long handle,
                               const char *pName,
                               vscpEvent *pEvent)
#else
extern "C" int
vscphlp_setRemoteVariableEvent(long handle,
                               const char *pName,
                               vscpEvent *pEvent)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableEvent(name, pEvent);
}

/*!
    \fn bool vscphlp_getVariableEventEx( const char *pName, vscpEventEx *pEvent
   ) \brief Get variable value from event variable \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableEventEx(long handle,
                                 const char *pName,
                                 vscpEventEx *pEvent)
#else
extern "C" int
vscphlp_getRemoteVariableEventEx(long handle,
                                 const char *pName,
                                 vscpEventEx *pEvent)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->getRemoteVariableEventEx(name, pEvent);
}

/*!
    \fn bool vscphlp_setVariableEventEx( const char *pName, vscpEventEx *pEvent
   ) \brief Get variable value from event variable \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableEventEx(long handle,
                                 const char *pName,
                                 vscpEventEx *pEvent)
#else
extern "C" int
vscphlp_setRemoteVariableEventEx(long handle,
                                 const char *pName,
                                 vscpEventEx *pEvent)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableEventEx(name, pEvent);
}

/*!
    \fn bool vscphlp_getVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableGUIDString(long handle,
                                    const char *pName,
                                    char *pGUID,
                                    size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableGUIDString(long handle,
                                    const char *pName,
                                    char *pGUID,
                                    size_t len)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pGUID) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    cguid GUID;
    std::string strGUID;
    std::string name = std::string(pName);
    int rv           = pvscpif->getRemoteVariableGUID(name, GUID);
    GUID.toString(strGUID);
    memset(pGUID, 0, len);
    strncpy(pGUID, strGUID.c_str(), std::min(strlen(strGUID.c_str()), len));
    return rv;
}

/*!
    \fn bool vscphlp_setVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableGUIDString(long handle,
                                    const char *pName,
                                    const char *pGUID)
#else
extern "C" int
vscphlp_setRemoteVariableGUIDString(long handle,
                                    const char *pName,
                                    const char *pGUID)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pGUID) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    cguid guid;
    guid.getFromString(pGUID);
    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableGUID(name, guid);
}

/*!
    \fn bool vscphlp_getVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer GUID array
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableGUIDArray(long handle,
                                   const char *pName,
                                   unsigned char *pGUID)
#else
extern "C" int
vscphlp_getRemoteVariableGUIDArray(long handle,
                                   const char *pName,
                                   unsigned char *pGUID)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pGUID) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    cguid GUID;
    std::string name = std::string(pName);
    int rv           = pvscpif->getRemoteVariableGUID(name, GUID);
    memcpy(pGUID, GUID.getGUID(), 16);
    return rv;
}

/*!
    \fn bool vscphlp_setVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableGUIDArray(long handle,
                                   const char *pName,
                                   const unsigned char *pGUID)
#else
extern "C" int
vscphlp_setRemoteVariableGUIDArray(long handle,
                                   const char *pName,
                                   const unsigned char *pGUID)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pGUID) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    cguid guid;
    guid.getFromArray(pGUID);
    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableGUID(name, guid);
}

/*!
    \fn bool vscphlp_getVariableVSCPdata( const char *pName, uint16_t
   *psizeData, uint8_t *pData ) \brief Get variable value from VSCP data
   variable \param name of variable \param Pointer to variable that will hold
   the size of the data array \param pointer to VSCP data array variable
   (unsigned char [8] ) that get the value of the string variable. \return
   Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableVSCPData(long handle,
                                  const char *pName,
                                  unsigned char *pData,
                                  unsigned short *psize)
#else
extern "C" int
vscphlp_getRemoteVariableVSCPData(long handle,
                                  const char *pName,
                                  unsigned char *pData,
                                  unsigned short *psize)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pData) return VSCP_ERROR_PARAMETER;
    if (NULL == psize) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->getRemoteVariableVSCPdata(name, pData, psize);
}

/*!
    \fn bool vscphlp_setVariableVSCPdata( const char *pName, uint16_t sizeData,
   uint8_t *pData ) \brief Get variable value from VSCP data variable \param
   name of variable \param Pointer to variable that will hold the size of the
   data array \param pointer to VSCP data array variable (unsigned char [8] )
   that get the value of the string variable. \return Returns VSCP_ERROR_SUCCESS
   on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableVSCPData(long handle,
                                  const char *pName,
                                  uint8_t *pData,
                                  unsigned short size)
#else
extern "C" int
vscphlp_setRemoteVariableVSCPData(long handle,
                                  const char *pName,
                                  uint8_t *pData,
                                  unsigned short size)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pData) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableVSCPdata(name, pData, size);
}

/*!
    \fn bool vscphlp_getVariableVSCPclass( const char *pName, uint16_t
   *vscphlp_class ) \brief Get variable value from class variable \param name of
   variable \param pointer to int that get the value of the class variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableVSCPClass(long handle,
                                   const char *pName,
                                   unsigned short *vscp_class)
#else
extern "C" int
vscphlp_getRemoteVariableVSCPClass(long handle,
                                   const char *pName,
                                   unsigned short *vscp_class)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == vscp_class) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->getRemoteVariableVSCPclass(name, vscp_class);
}

/*!
    \fn bool vscphlp_setVariableVSCPclass( const char *pName, uint16_t
   vscp_class ) \brief Get variable value from class variable \param name of
   variable \param pointer to int that get the value of the class variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableVSCPClass(long handle,
                                   const char *pName,
                                   unsigned short vscp_class)
#else
extern "C" int
vscphlp_setRemoteVariableVSCPClass(long handle,
                                   const char *pName,
                                   unsigned short vscp_class)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableVSCPclass(name, vscp_class);
}

/*!
    \fn bool vscphlp_getVariableVSCPtype( const char *pName, uint8_t *vscp_type
   ) \brief Get variable value from type variable \param name of variable \param
   pointer to int that get the value of the type variable. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableVSCPType(long handle,
                                  const char *pName,
                                  unsigned short *vscp_type)
#else
extern "C" int
vscphlp_getRemoteVariableVSCPType(long handle,
                                  const char *pName,
                                  unsigned short *vscp_type)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == vscp_type) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->getRemoteVariableVSCPtype(name, vscp_type);
}

/*!
    \fn bool WINAPI EXPORT vscphlp_setVariableVSCPtype( const char *pName,
   uint8_t vscp_type ) \brief Get variable value from type variable \param name
   of variable \param pointer to int that get the value of the type variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableVSCPType(long handle,
                                  const char *pName,
                                  unsigned short vscp_type)
#else
extern "C" int
vscphlp_setRemoteVariableVSCPType(long handle,
                                  const char *pName,
                                  unsigned short vscp_type)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableVSCPtype(name, vscp_type);
}

// -----

/*!
    \fn int vscphlp_getRemoteVariableVSCPTimestamp( long handle, const char
   *pName, unsigned long *vscp_timestamp ) \brief Get variable value for VSCP
   timestamp type variable \param name of variable \param pointer to unsigned
   long that get the value of the variable. \return Returns VSCP_ERROR_SUCCESS
   on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableVSCPTimestamp(long handle,
                                       const char *pName,
                                       unsigned long *vscp_timestamp)
#else
extern "C" int
vscphlp_getRemoteVariableVSCPTimestamp(long handle,
                                       const char *pName,
                                       unsigned long *vscp_timestamp)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == vscp_timestamp) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    uint32_t timestamp;
    if (VSCP_ERROR_SUCCESS !=
        (rv = pvscpif->getRemoteVariableVSCPTimestamp(name, &timestamp))) {
        return rv;
    }

    *vscp_timestamp = timestamp;

    return VSCP_ERROR_SUCCESS;
}

/*!
    \fn int vscphlp_setRemoteVariableVSCPTimestamp( long handle, const char
   *pName, unsigned long vscp_timestamp ) \brief Get variable value from a VSCP
   timestamp variable \param name of variable \param Unsigned long that holds
   the value of the VSCP timestamp variable. \return Returns VSCP_ERROR_SUCCESS
   on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableVSCPTimestamp(long handle,
                                       const char *pName,
                                       unsigned long vscp_timestamp)
#else
extern "C" int
vscphlp_setRemoteVariableVSCPTimestamp(long handle,
                                       const char *pName,
                                       unsigned long vscp_timestamp)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    return pvscpif->setRemoteVariableVSCPTimestamp(name, vscp_timestamp);
}

/*!
    \fn int vscphlp_getRemoteVariableDateTime( long handle, const char *pName,
   unsigned long *vscp_timestamp ) \brief Get variable value for datetime type
   variable \param name of variable \param pointer to string buffer that get the
   value of the variable. \param size The size of the buffer. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableDateTime(long handle,
                                  const char *pName,
                                  char *pValue,
                                  size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableDateTime(long handle,
                                  const char *pName,
                                  char *pValue,
                                  size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    vscpdatetime datetime;
    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableDateTime(name, datetime))) {
        std::string str = datetime.getISODateTime();
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)str.c_str(),
                std::min(strlen((const char *)str.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableDateTime( long handle, const char *pName,
   char *pValue ) \brief Get variable value from a VSCP datetime variable \param
   name of variable \param Pointer to string that holds the value of the
   datetime variable. \return Returns VSCP_ERROR_SUCCESS on success, otherwise
   error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableDateTime(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableDateTime(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name  = std::string(pName);
    std::string value = std::string(pValue);
    vscpdatetime datetime;
    if (!datetime.set(value)) {
        return VSCP_ERROR_PARAMETER;
    }

    return pvscpif->setRemoteVariableDateTime(name, datetime);
}

/*!
    \fn int vscphlp_getRemoteVariableDate( long handle, const char *pName, char
   *pValue, int size ) \brief Get variable value for date type variable \param
   name of variable \param pointer to string buffer that get the value of the
   variable. \param size The size of the buffer. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableDate(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableDate(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    vscpdatetime datetime;
    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableDate(name, datetime))) {
        std::string str = datetime.getISODateTime();
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)str.c_str(),
                std::min(strlen((const char *)str.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableDate( long handle, const char *pName, char
   *pValue ) \brief Get variable value from a date variable \param name of
   variable \param Pointer to string that holds the value of the date variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableDate(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableDate(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name = std::string(pName);
    vscpdatetime datetime;
    if (!datetime.set(pValue)) {
        return VSCP_ERROR_PARAMETER;
    }

    return pvscpif->setRemoteVariableDate(name, datetime);
}

/*!
    \fn int vscphlp_getRemoteVariableTime( long handle, const char *pName, char
   *pValue, int size ) \brief Get variable value for datetime type variable
    \param name of variable
    \param pointer to string buffer that get the value of the variable.
    \param size The size of the buffer.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableTime(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableTime(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    vscpdatetime datetime;
    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableTime(name, datetime))) {
        std::string str = datetime.getISODateTime();
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)str.c_str(),
                std::min(strlen((const char *)str.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableTime( long handle, const char *pName, char
   *pValue ) \brief Get variable value from a VSCP time variable \param name of
   variable \param Pointer to string that holds the value of the time variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableTime(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableTime(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name = std::string(pName);
    vscpdatetime datetime;
    if (!datetime.setISOTime(pValue)) {
        return VSCP_ERROR_PARAMETER;
    }

    return pvscpif->setRemoteVariableTime(name, datetime);
}

/*!
    \fn int vscphlp_getRemoteVariableBlob( long handle, const char *pName, char
   *pValue, int size ) \brief Get variable value for blob type variable \param
   name of variable \param pointer to string buffer that get the value of the
   variable. \param size The size of the buffer. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableBlob(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableBlob(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string blob;

    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableBlob(name, blob))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)blob.c_str(),
                std::min(strlen((const char *)blob.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableBlob( long handle, const char *pName, char
   *pValue ) \brief Get variable value from a blob variable \param name of
   variable \param Pointer to string that holds the value of the variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableBlob(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableBlob(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name = std::string(pName);
    std::string blob = std::string(pValue);

    return pvscpif->setRemoteVariableBlob(name, blob);
}

/*!
    \fn int vscphlp_getRemoteVariableMIME( long handle, const char *pName, char
   *pValue, int size ) \brief Get variable value for mime type variable \param
   name of variable \param pointer to string buffer that get the value of the
   variable. \param size The size of the buffer. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableMIME(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableMIME(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string mime;

    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableMIME(name, mime))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)mime.c_str(),
                std::min(strlen((const char *)mime.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableMIME( long handle, const char *pName, char
   *pValue ) \brief Get variable value from a mime variable \param name of
   variable \param Pointer to string that holds the value of the variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableMIME(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableMIME(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name = std::string(pName);
    std::string mime = std::string(pValue);

    return pvscpif->setRemoteVariableMIME(name, mime);
}

/*!
    \fn int vscphlp_getRemoteVariableHTML( long handle, const char *pName, char
   *pValue, int size ) \brief Get variable value for html type variable \param
   name of variable \param pointer to string buffer that get the value of the
   variable. \param size The size of the buffer. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableHTML(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableHTML(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string html;

    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableHTML(name, html))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)html.c_str(),
                std::min(strlen((const char *)html.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableHTML( long handle, const char *pName, char
   *pValue ) \brief Get variable value from a html variable \param name of
   variable \param Pointer to string that holds the value of the variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableHTML(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableHML(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name = std::string(pName);
    std::string html = std::string(pValue);

    return pvscpif->setRemoteVariableHTML(name, html);
}

/*!
    \fn int vscphlp_getRemoteVariableJavaScript( long handle, const char *pName,
   char *pValue, int size ) \brief Get variable value for JavaScript type
   variable \param name of variable \param pointer to string buffer that get the
   value of the variable. \param size The size of the buffer. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableJavaScript(long handle,
                                    const char *pName,
                                    char *pValue,
                                    size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableJavaScript(long handle,
                                    const char *pName,
                                    char *pValue,
                                    size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string js;

    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableJavaScript(name, js))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)js.c_str(),
                std::min(strlen((const char *)js.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableJavacript( long handle, const char *pName,
   char *pValue ) \brief Get variable value from a JavaScript variable \param
   name of variable \param Pointer to string that holds the value of the
   variable. \return Returns VSCP_ERROR_SUCCESS on success, otherwise error
   code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableJavaScript(long handle,
                                    const char *pName,
                                    char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableJavaScript(long handle,
                                    const char *pName,
                                    char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name = std::string(pName);
    std::string js   = std::string(pValue);

    return pvscpif->setRemoteVariableJavaScript(name, js);
}

/*!
    \fn int vscphlp_getRemoteVariableLUA( long handle, const char *pName, char
   *pValue, int size ) \brief Get variable value for lua type variable \param
   name of variable \param pointer to string buffer that get the value of the
   variable. \param size The size of the buffer. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableLUA(long handle,
                             const char *pName,
                             char *pValue,
                             size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableLUA(long handle,
                             const char *pName,
                             char *pValue,
                             size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string lua;

    if (VSCP_ERROR_SUCCESS == (rv = pvscpif->getRemoteVariableLUA(name, lua))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)lua.c_str(),
                std::min(strlen((const char *)lua.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableLUA( long handle, const char *pName, char
   *pValue ) \brief Get variable value from a lua variable \param name of
   variable \param Pointer to string that holds the value of the variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableLUA(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableLUA(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name = std::string(pName);
    std::string lua  = std::string(pValue);

    return pvscpif->setRemoteVariableLUA(name, lua);
}

/*!
    \fn int vscphlp_getRemoteVariableLUARES( long handle, const char *pName,
   char *pValue, int size ) \brief Get variable value for luares type variable
    \param name of variable
    \param pointer to string buffer that get the value of the variable.
    \param size The size of the buffer.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableLUARES(long handle,
                                const char *pName,
                                char *pValue,
                                size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableLUARES(long handle,
                                const char *pName,
                                char *pValue,
                                size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string luares;

    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableLUARES(name, luares))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)luares.c_str(),
                std::min(strlen((const char *)luares.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableLUARES( long handle, const char *pName,
   char *pValue ) \brief Get variable value from a luares variable \param name
   of variable \param Pointer to string that holds the value of the variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableLUARES(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableLUARES(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name   = std::string(pName);
    std::string luares = std::string(pValue);

    return pvscpif->setRemoteVariableLUARES(name, luares);
}

/*!
    \fn int vscphlp_getRemoteVariableUX1( long handle, const char *pName, char
   *pValue, int size ) \brief Get variable value for ux1 type variable \param
   name of variable \param pointer to string buffer that get the value of the
   variable. \param size The size of the buffer. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableUX1(long handle,
                             const char *pName,
                             char *pValue,
                             size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableUX1(long handle,
                             const char *pName,
                             char *pValue,
                             size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string ux1;

    if (VSCP_ERROR_SUCCESS == (rv = pvscpif->getRemoteVariableUX1(name, ux1))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)ux1.c_str(),
                std::min(strlen((const char *)ux1.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableUX1( long handle, const char *pName, char
   *pValue ) \brief Get variable value from a ux1 variable \param name of
   variable \param Pointer to string that holds the value of the variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableUX1(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableUX1(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name = std::string(pName);
    std::string ux1  = std::string(pValue);

    return pvscpif->setRemoteVariableUX1(name, ux1);
}

/*!
    \fn int vscphlp_getRemoteVariableDMROW( long handle, const char *pName, char
   *pValue, int size ) \brief Get variable value for dmrow type variable \param
   name of variable \param pointer to string buffer that get the value of the
   variable. \param size The size of the buffer. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableDMROW(long handle,
                               const char *pName,
                               char *pValue,
                               size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableDMROW(long handle,
                               const char *pName,
                               char *pValue,
                               size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string dm;

    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableDMROW(name, dm))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)dm.c_str(),
                std::min(strlen((const char *)dm.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableDMROW( long handle, const char *pName, char
   *pValue ) \brief Get variable value from a dmrow variable \param name of
   variable \param Pointer to string that holds the value of the variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableDMROW(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableDMROW(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name = std::string(pName);
    std::string dm   = std::string(pValue);

    return pvscpif->setRemoteVariableDMROW(name, dm);
}

/*!
    \fn int vscphlp_getRemoteVariableDriver( long handle, const char *pName,
   char *pValue, int size ) \brief Get variable value for driver type variable
    \param name of variable
    \param pointer to string buffer that get the value of the variable.
    \param size The size of the buffer.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableDriver(long handle,
                                const char *pName,
                                char *pValue,
                                size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableDriver(long handle,
                                const char *pName,
                                char *pValue,
                                size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string driver;

    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableDriver(name, driver))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)driver.c_str(),
                std::min(strlen((const char *)driver.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableDriver( long handle, const char *pName,
   char *pValue ) \brief Get variable value from a driver variable \param name
   of variable \param Pointer to string that holds the value of the variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableDriver(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableDriver(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name   = std::string(pName);
    std::string driver = std::string(pValue);

    return pvscpif->setRemoteVariableDriver(name, driver);
}

/*!
    \fn int vscphlp_getRemoteVariableUser( long handle, const char *pName, char
   *pValue, int size ) \brief Get variable value for user type variable \param
   name of variable \param pointer to string buffer that get the value of the
   variable. \param size The size of the buffer. \return Returns
   VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableUser(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableUser(long handle,
                              const char *pName,
                              char *pValue,
                              size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string user;

    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableUser(name, user))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)user.c_str(),
                std::min(strlen((const char *)user.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableUser( long handle, const char *pName, char
   *pValue ) \brief Get variable value from a user variable \param name of
   variable \param Pointer to string that holds the value of the variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableUser(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableUser(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name = std::string(pName);
    std::string user = std::string(pValue);

    return pvscpif->setRemoteVariableUser(name, user);
}

/*!
    \fn int vscphlp_getRemoteVariableFilter( long handle, const char *pName,
   char *pValue, int size ) \brief Get variable value for filter type variable
    \param name of variable
    \param pointer to string buffer that get the value of the variable.
    \param size The size of the buffer.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getRemoteVariableFilter(long handle,
                                const char *pName,
                                char *pValue,
                                size_t len)
#else
extern "C" int
vscphlp_getRemoteVariableFilter(long handle,
                                const char *pName,
                                char *pValue,
                                size_t len)
#endif
{
    int rv;

    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    std::string name = std::string(pName);
    std::string filter;

    if (VSCP_ERROR_SUCCESS ==
        (rv = pvscpif->getRemoteVariableFilter(name, filter))) {
        memset(pValue, 0, len);
        strncpy(pValue,
                (const char *)filter.c_str(),
                std::min(strlen((const char *)filter.c_str()), len));
    }

    return rv;
}

/*!
    \fn int vscphlp_setRemoteVariableFilter( long handle, const char *pName,
   char *pValue ) \brief Get variable value from a filter variable \param name
   of variable \param Pointer to string that holds the value of the variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setRemoteVariableFilter(long handle, const char *pName, char *pValue)
#else
extern "C" int
vscphlp_setRemoteVariableFilter(long handle, const char *pName, char *pValue)
#endif
{
    if (NULL == pName) return VSCP_ERROR_PARAMETER;
    if (NULL == pValue) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if (!pvscpif->isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if (NULL == pName) return false;
    if (NULL == pValue) return false;

    std::string name   = std::string(pName);
    std::string filter = std::string(pValue);

    return pvscpif->setRemoteVariableFilter(name, filter);
}

////////////////////////////////////////////////////////////////////////////////
//                              H E L P E R S
////////////////////////////////////////////////////////////////////////////////

// * * * * *    G E N E R A L   * * * * *

/*!
    \fn long vscphlp_readStringValue( const char * pStrValue )
    \brief Read a value (decimal or hex) from a string.
        \return The converted number.
*/
#ifdef WIN32
extern "C" DllExport unsigned long WINAPI EXPORT
vscphlp_readStringValue(const char *pStrValue)
#else
extern "C" unsigned long
vscphlp_readStringValue(const char *pStrValue)
#endif
{
    std::string strVal;
    strVal = std::string(pStrValue);

    return vscp_readStringValue(strVal);
}

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_replaceBackslash(char *pStr)
#else
extern "C" int
vscphlp_replaceBackslash(char *pStr)
#endif
{
    if (NULL == pStr) return VSCP_ERROR_ERROR;

    std::string str = std::string(pStr);
    str             = vscp_replaceBackslash(str);
    strcpy(pStr, str.c_str());
    return VSCP_ERROR_SUCCESS;
}

// * * * * *  E V E N T - H E L P E R S * * * * *

/*!
    \fn bool vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx,
                                    const vscpEvent *pEvent )
    \brief Convert VSCP standard event form to ex. form.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertVSCPtoEx(vscpEventEx *pEventEx, const vscpEvent *pEvent)
#else
extern "C" int
vscphlp_convertVSCPtoEx(vscpEventEx *pEventEx, const vscpEvent *pEvent)
#endif
{
    return vscp_convertVSCPtoEx(pEventEx, pEvent) ? VSCP_ERROR_SUCCESS
                                                  : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_convertVSCPfromEx( vscpEvent *pEvent,
                                        const vscpEventEx *pEventEx )
    \brief Convert VSCP ex. event form to standard form.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertVSCPfromEx(vscpEvent *pEvent, const vscpEventEx *pEventEx)
#else
extern "C" int
vscphlp_convertVSCPfromEx(vscpEvent *pEvent, const vscpEventEx *pEventEx)
#endif
{
    return vscp_convertVSCPfromEx(pEvent, pEventEx) ? VSCP_ERROR_SUCCESS
                                                    : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_newVSCPevent( vscpEvent **ppEvent )
    \brief Create a new VSCP event.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_newVSCPevent(vscpEvent **ppEvent)
#else
extern "C" int
vscphlp_newVSCPevent(vscpEvent **ppEvent)
#endif
{
    return vscp_newVSCPevent(ppEvent) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn void vscphlp_deleteVSCPevent( vscpEvent *pEvent )
    \brief Delete VSCP event.
*/
#ifdef WIN32
extern "C" DllExport void WINAPI EXPORT
vscphlp_deleteVSCPevent(vscpEvent *pEvent)
#else
extern "C" void
vscphlp_deleteVSCPevent(vscpEvent *pEvent)
#endif
{
    return vscp_deleteVSCPevent(pEvent);
}

/*!
    \fn void vscphlp_deleteVSCPevent_v2( vscpEvent **ppEvent )
    \brief Delete and null VSCP event.
*/
#ifdef WIN32
extern "C" DllExport void WINAPI EXPORT
vscphlp_deleteVSCPevent_v2(vscpEvent **ppEvent)
#else
extern "C" void
vscphlp_deleteVSCPevent_v2(vscpEvent **ppEvent)
#endif
{
    return vscp_deleteVSCPevent_v2(ppEvent);
}

/*!
    \fn void vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx )
    \brief Delete VSCP event ex.
*/
#ifdef WIN32
extern "C" DllExport void WINAPI EXPORT
vscphlp_deleteVSCPeventEx(vscpEventEx *pEventEx)
#else
extern "C" void
vscphlp_deleteVSCPeventEx(vscpEventEx *pEventEx)
#endif
{
    return vscp_deleteVSCPeventEx(pEventEx);
}

/*!
    \fn unsigned char vscphlp_getVscpPriority( const vscpEvent *pEvent )
    \brief Get VSCP priority.
*/
#ifdef WIN32
extern "C" DllExport unsigned char WINAPI EXPORT
vscphlp_getVscpPriority(const vscpEvent *pEvent)
#else
extern "C" unsigned char
vscphlp_getVscpPriority(const vscpEvent *pEvent)
#endif
{
    return vscp_getVscpPriority(pEvent);
}

#ifdef WIN32
extern "C" DllExport unsigned char WINAPI EXPORT
vscphlp_getVscpPriorityEx(const vscpEventEx *pEvent)
#else
extern "C" unsigned char
vscphlp_getVscpPriorityEx(const vscpEventEx *pEvent)
#endif
{
    return vscp_getVscpPriorityEx(pEvent);
}

/*!
    \fn void vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority
   ) \brief Set VSCP priority.
*/
#ifdef WIN32
extern "C" DllExport void WINAPI EXPORT
vscphlp_setVscpPriority(vscpEvent *pEvent, unsigned char priority)
#else
extern "C" void
vscphlp_setVscpPriority(vscpEvent *pEvent, unsigned char priority)
#endif
{
    vscp_setVscpPriority(pEvent, priority);
}

#ifdef WIN32
extern "C" DllExport void WINAPI EXPORT
vscphlp_setVscpPriorityEx(vscpEventEx *pEvent, unsigned char priority)
#else
extern "C" void
vscphlp_setVscpPriorityEx(vscpEventEx *pEvent, unsigned char priority)
#endif
{
    vscp_setVscpPriorityEx(pEvent, priority);
}

/*!
    \fn vscphlp_getVSCPheadFromCANALid( const unsigned long id )
    \brief Get the VSCP head from a CANAL message id (CANAL id).
*/
#ifdef WIN32
extern "C" DllExport unsigned char WINAPI EXPORT
vscphlp_getVSCPheadFromCANALid(const unsigned long id)
#else
extern "C" unsigned char
vscphlp_getVSCPheadFromCANALid(const unsigned long id)
#endif
{
    return vscp_getVSCPheadFromCANALid(id);
}

/*!
    \fn vscphlp_getVSCPclassFromCANALid( const unsigned long id )
    \brief Get the VSCP class from a CANAL message id (CANAL id).
*/
#ifdef WIN32
extern "C" DllExport unsigned short WINAPI EXPORT
vscphlp_getVSCPclassFromCANALid(const unsigned long id)
#else
extern "C" unsigned short
vscphlp_getVSCPclassFromCANALid(const unsigned long id)
#endif
{
    return vscp_getVSCPclassFromCANALid(id);
}

/*!
    \fn unsigned short vscphlp_getVSCPtypeFromCANALid( const unsigned long id )
    \brief Get the VSCP type from a a CANAL message id (CANAL id).
*/
#ifdef WIN32
extern "C" DllExport unsigned short WINAPI EXPORT
vscphlp_getVSCPtypeFromCANALid(const unsigned long id)
#else
extern "C" unsigned short
vscphlp_getVSCPtypeFromCANALid(const unsigned long id)
#endif
{
    return vscp_getVSCPtypeFromCANALid(id);
}

/*!
    \fn unsigned short vscphlp_getVSCPnicknameFromCANALid( const unsigned long
   id ) \brief Get the VSCP nickname from a a CANAL message id (CANAL id).
*/
#ifdef WIN32
extern "C" DllExport unsigned char WINAPI EXPORT
vscphlp_getVSCPnicknameFromCANALid(const unsigned long id)
#else
extern "C" unsigned char
vscphlp_getVSCPnicknameFromCANALid(const unsigned long id)
#endif
{
    return vscp_getVSCPnicknameFromCANALid(id);
}

/*!
    \fn unsigned long vscphlp_getCANALidFromVSCPdata( const unsigned char
   priority, const unsigned short vscp_class, const unsigned short vscp_type )
    \brief Construct a CANAL id (CANAL id ) from VSCP.
*/
#ifdef WIN32
extern "C" DllExport unsigned long WINAPI EXPORT
vscphlp_getCANALidFromVSCPdata(const unsigned char priority,
                               const unsigned short vscp_class,
                               const unsigned short vscp_type)
#else
extern "C" unsigned long
vscphlp_getCANALidFromVSCPdata(const unsigned char priority,
                               const unsigned short vscp_class,
                               const unsigned short vscp_type)
#endif
{
    return vscp_getCANALidFromVSCPdata(priority, vscp_class, vscp_type);
}

/*!
    \fn unsigned long vscphlp_getCANALidFromVSCPevent( const vscpEvent *pEvent )
    \brief Get CANAL id (CAN id) from VSCP event.
*/
#ifdef WIN32
extern "C" DllExport unsigned long WINAPI EXPORT
vscphlp_getCANALidFromVSCPevent(const vscpEvent *pEvent)
#else
extern "C" unsigned long
vscphlp_getCANALidFromVSCPevent(const vscpEvent *pEvent)
#endif
{
    return vscp_getCANALidFromVSCPevent(pEvent);
}

/*!
    \fn unsigned long vscphlp_getCANALidFromVSCPevent( const vscpEvent *pEvent )
    \brief Get CANAL id (CAN id) from VSCP event.
*/
#ifdef WIN32
extern "C" DllExport unsigned long WINAPI EXPORT
vscphlp_getCANALidFromVSCPeventEx(const vscpEventEx *pEvent)
#else
extern "C" unsigned long
vscphlp_getCANALidFromVSCPeventEx(const vscpEventEx *pEvent)
#endif
{
    return vscp_getCANALidFromVSCPeventEx(pEvent);
}

/*!
    \fn unsigned short vscphlp_calc_crc_Event( vscpEvent *pEvent, short bSet )
    \brief Calculate VSCP crc.
*/
#ifdef WIN32
extern "C" DllExport unsigned short WINAPI EXPORT
vscphlp_calc_crc_Event(vscpEvent *pEvent, short bSet)
#else
extern "C" unsigned short
vscphlp_calc_crc_Event(vscpEvent *pEvent, short bSet)
#endif
{
    return vscp_calc_crc_Event(pEvent, bSet);
}

/*!
    \fn unsigned short vscphlp_calc_crc_EventEx( vscpEvent *pEvent, short bSet )
    \brief Calculate VSCP crc.
*/
#ifdef WIN32
extern "C" DllExport unsigned short WINAPI EXPORT
vscphlp_calc_crc_EventEx(vscpEventEx *pEvent, short bSet)
#else
extern "C" unsigned short
vscphlp_calc_crc_EventEx(vscpEventEx *pEvent, short bSet)
#endif
{
    return vscp_calc_crc_EventEx(pEvent, bSet);
}

// * * * * *   G U I D  H E L P E R S    * * * * *

/*!
        calcCRC4GUIDArray

        \param Pointer to GUID array (MSB-LSB order)
        \return crc for GUID.
    */
#ifdef WIN32
extern "C" DllExport unsigned char WINAPI EXPORT
vscphlp_calcCRC4GUIDArray(unsigned char *pguid)
#else
extern "C" unsigned char
vscphlp_calcCRC4GUIDArray(unsigned char *pguid)
#endif
{
    return vscp_calcCRC4GUIDArray(pguid);
}

/*!
  calcCRC4GUIDString

  \param Pointer to GUID array (MSB-LSB order)
  \return crc for GUID.
*/
#ifdef WIN32
extern "C" DllExport unsigned char WINAPI EXPORT
vscphlp_calcCRC4GUIDString(const char *strguid)
#else
extern "C" unsigned char
vscphlp_calcCRC4GUIDString(const char *strguid)
#endif
{
    std::string str = std::string(strguid);
    return vscp_calcCRC4GUIDString(str);
}

/*!
    \fn bool vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
    \brief Write GUID into VSCP event from string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getGuidFromString(vscpEvent *pEvent, const char *pGUID)
#else
extern "C" int
vscphlp_getGuidFromString(vscpEvent *pEvent, const char *pGUID)
#endif
{
    std::string strGUID = std::string(pGUID);
    return vscp_setEventGuidFromString(pEvent, strGUID) ? VSCP_ERROR_SUCCESS
                                                        : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_getGuidFromStringEx( vscpEventEx *pEvent, const char *
   pGUID ) \brief Write GUID into VSCP event from string.
*/

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getGuidFromStringEx(vscpEventEx *pEvent, const char *pGUID)
#else
extern "C" int
vscphlp_getGuidFromStringEx(vscpEventEx *pEvent, const char *pGUID)
#endif
{
    std::string strGUID = std::string(pGUID);
    return vscp_setEventExGuidFromString(pEvent, strGUID) ? VSCP_ERROR_SUCCESS
                                                          : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr
   ) \brief Write GUID from string into array.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getGuidFromStringToArray(uint8_t *pGUID, const char *pStr)
#else
extern "C" int
vscphlp_getGuidFromStringToArray(uint8_t *pGUID, const char *pStr)
#endif
{
    std::string strGUID = std::string(pStr);
    return vscp_getGuidFromStringToArray(pGUID, strGUID) ? VSCP_ERROR_SUCCESS
                                                         : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_writeGuidToString( const vscpEvent *pEvent, char *pStr, int
   size ) \brief Write GUID from VSCP event to string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeGuidToString(const vscpEvent *pEvent, char *pStr, size_t len)
#else
extern "C" int
vscphlp_writeGuidToString(const vscpEvent *pEvent, char *pStr, size_t len)
#endif
{
    bool rv;

    std::string strGUID;
    rv = vscp_writeGuidToString(pEvent, strGUID);
    memset(pStr, 0, len);
    strncpy(pStr, strGUID.c_str(), std::min(strlen(strGUID.c_str()), len));
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_writeGuidToStringEx( const vscpEventEx *pEvent, char * pStr
   ) \brief Write GUID froom VSCP event to string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeGuidToStringEx(const vscpEventEx *pEvent, char *pStr, size_t len)
#else
extern "C" int
vscphlp_writeGuidToStringEx(const vscpEventEx *pEvent, char *pStr, size_t len)
#endif
{
    bool rv;

    std::string strGUID;
    rv = vscp_writeGuidToStringEx(pEvent, strGUID);
    memset(pStr, 0, len);
    strncpy(pStr, strGUID.c_str(), std::min(strlen(strGUID.c_str()), len));
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent,
                                            std::string& strGUID )
    \brief Write GUID from VSCP event to string with four bytes on each
    row seperated by \r\n.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeGuidToString4Rows(const vscpEvent *pEvent,
                               char *strGUID,
                               size_t len)
#else
extern "C" int
vscphlp_writeGuidToString4Rows(const vscpEvent *pEvent,
                               char *strGUID,
                               size_t len)
#endif
{
    std::string str = std::string(strGUID);
    bool rv         = vscp_writeGuidToString4Rows(pEvent, str);
    memset(strGUID, 0, len);
    strncpy(strGUID, str.c_str(), std::min(strlen(str.c_str()), len));

    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_writeGuidToString4RowsEx( const vscpEvent *pEvent,
                                            std::string& strGUID )
    \brief Write GUID from VSCP event to string with four bytes on each
    row seperated by \r\n.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeGuidToString4RowsEx(const vscpEventEx *pEvent,
                                 char *strGUID,
                                 size_t len)
#else
extern "C" int
vscphlp_writeGuidToString4RowsEx(const vscpEventEx *pEvent,
                                 char *strGUID,
                                 size_t len)
#endif
{
    std::string str = std::string(strGUID);
    bool rv         = vscp_writeGuidToString4RowsEx(pEvent, str);

    memset(strGUID, 0, len);
    strncpy(strGUID, str.c_str(), std::min(strlen(str.c_str()), len));

    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_writeGuidArrayToString( const unsigned char * pGUID,
                                            std::string& strGUID,
                                            int size )
    \brief Write GUID from byte array to string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeGuidArrayToString(const unsigned char *pGUID,
                               char *strGUID,
                               size_t len)
#else
extern "C" int
vscphlp_writeGuidArrayToString(const unsigned char *pGUID,
                               char *strGUID,
                               size_t len)
#endif
{
    std::string str;
    bool rv = vscp_writeGuidArrayToString(pGUID, str);

    memset(strGUID, 0, len);
    strncpy(strGUID, str.c_str(), std::min(strlen(str.c_str()), len));

    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_isGUIDEmpty( unsigned char *pGUID )
    \brief Check if GUID is empty (all nills).
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_isGUIDEmpty(unsigned char *pGUID)
#else
extern "C" int
vscphlp_isGUIDEmpty(unsigned char *pGUID)
#endif
{
    return vscp_isGUIDEmpty(pGUID) ? 1 : 0;
}

/*!
    \fn bool vscphlp_isSameGUID( const unsigned char *pGUID1,
                                const unsigned char *pGUID2 )
    \brief Check if two GUID's is equal to each other.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_isSameGUID(const unsigned char *pGUID1, const unsigned char *pGUID2)
#else
extern "C" int
vscphlp_isSameGUID(const unsigned char *pGUID1, const unsigned char *pGUID2)
#endif
{
    return vscp_isSameGUID(pGUID1, pGUID2) ? 1 : 0;
}

/*!
    \fn bool vscphlp_reverseGUID(unsigned char *pGUID)
    \brief Reverse a GUID
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_reverseGUID(unsigned char *pGUID)
#else
extern "C" int
vscphlp_reverseGUID(unsigned char *pGUID)
#endif
{
    return vscp_reverseGUID(pGUID) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

// * * * * *   F I L T E R  * * * * *

/*!
    \fn void vscphlp_clearVSCPFilter( vscpEventFilter *pFilter )
    \brief Clear VSCP filter.
*/
#ifdef WIN32
extern "C" DllExport void WINAPI EXPORT
vscphlp_clearVSCPFilter(vscpEventFilter *pFilter)
#else
extern "C" void
vscphlp_clearVSCPFilter(vscpEventFilter *pFilter)
#endif
{
    return vscp_clearVSCPFilter(pFilter);
}

/*!
    \fn void vscphlp_clearVSCPFilter( vscpEventFilter *pFilter )
    \brief Clear VSCP filter.
*/
#ifdef WIN32
extern "C" DllExport void WINAPI EXPORT
vscphlp_copyVSCPFilter(vscpEventFilter *pToFilter,
                       const vscpEventFilter *pFromFilter)
#else
extern "C" void
vscphlp_copyVSCPFilter(vscpEventFilter *pToFilter,
                       const vscpEventFilter *pFromFilter)
#endif
{
    return vscp_copyVSCPFilter(pToFilter, pFromFilter);
}

/*!
    \fn bool readFilterFromString( vscpEventFilter *pFilter, std::string&
   strFilter ) \brief Read a filter from a string \param pFilter Filter
   structure to write filter to. \param strFilter Filter in string form
            filter-priority, filter-class, filter-type, filter-GUID
    \return true on success, fals eon failure.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_readFilterFromString(vscpEventFilter *pFilter, const char *strFilter)
#else
extern "C" int
vscphlp_readFilterFromString(vscpEventFilter *pFilter, const char *strFilter)
#endif
{
    std::string str = std::string(strFilter);
    return vscp_readFilterFromString(pFilter, str) ? VSCP_ERROR_SUCCESS
                                                   : VSCP_ERROR_ERROR;
}

/*!
    \fn bool readMaskFromString( vscpEventFilter *pFilter, std::string& strMask
   ) \brief Read a mask from a string \param pFilter Filter structure to write
   mask to. \param strMask Mask in string form mask-priority, mask-class,
   mask-type, mask-GUID \return true on success, fals eon failure.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_readMaskFromString(vscpEventFilter *pFilter, const char *strMask)
#else
extern "C" int
vscphlp_readMaskFromString(vscpEventFilter *pFilter, const char *strMask)
#endif
{
    std::string str = std::string(strMask);
    return vscp_readMaskFromString(pFilter, str) ? VSCP_ERROR_SUCCESS
                                                 : VSCP_ERROR_ERROR;
}

/*!
    \fn bool writeFilterToString( vscpEventFilter *pFilter, std::string&
   strFilter ) \brief Write a filter to a string \param pFilter Filter structure
   to write filter to. \param strFilter Will get filter in string form
            filter-priority, filter-class, filter-type, filter-GUID
    \return true on success, fals eon failure.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeFilterToString(vscpEventFilter *pFilter, char *strFilter)
#else
extern "C" int
vscphlp_writeFilterToString(vscpEventFilter *pFilter, char *strFilter)
#endif
{
    int rv;
    std::string str;

    rv = vscp_writeFilterToString(pFilter, str) ? VSCP_ERROR_SUCCESS
                                                : VSCP_ERROR_ERROR;
    if (VSCP_ERROR_SUCCESS == rv)
        memcpy(strFilter, (const char *)str.c_str(), strlen(str.c_str()));

    return rv;
}

/*!
    \fn bool writeMaskToString( vscpEventFilter *pFilter, std::string& strMask )
    \brief Write a mask to a string
    \param pFilter Filter structure to write mask to.
    \param strMask Will get mask in string form
            mask-priority, mask-class, mask-type, mask-GUID
    \return true on success, fals eon failure.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeMaskToString(vscpEventFilter *pFilter, char *strMask)
#else
extern "C" int
vscphlp_writeMaskToString(vscpEventFilter *pFilter, char *strMask)
#endif
{
    int rv;
    std::string str;

    rv = vscp_writeMaskToString(pFilter, str) ? VSCP_ERROR_SUCCESS
                                              : VSCP_ERROR_ERROR;
    if (VSCP_ERROR_SUCCESS == rv)
        memcpy(strMask, (const char *)str.c_str(), strlen(str.c_str()));

    return rv;
}

/*!
    \fn bool vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                    const vscpEventFilter *pFilter )
    \brief Check VSCP filter condition.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_doLevel2Filter(const vscpEvent *pEvent, const vscpEventFilter *pFilter)
#else
extern "C" int
vscphlp_doLevel2Filter(const vscpEvent *pEvent, const vscpEventFilter *pFilter)
#endif
{
    return vscp_doLevel2Filter(pEvent, pFilter) ? VSCP_ERROR_SUCCESS
                                                : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                            const canalMsg *pcanalMsg,
                                            unsigned char *pGUID,
                                            bool bCAN )
    \brief Convert CANAL message to VSCP event.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertCanalToEvent(vscpEvent *pvscpEvent,
                            const canalMsg *pcanalMsg,
                            unsigned char *pGUID)
#else
extern "C" int
vscphlp_convertCanalToEvent(vscpEvent *pvscpEvent,
                            const canalMsg *pcanalMsg,
                            unsigned char *pGUID)
#endif
{
    return vscp_convertCanalToEvent(pvscpEvent, pcanalMsg, pGUID)
             ? VSCP_ERROR_SUCCESS
             : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                            const canalMsg *pcanalMsg,
                                            unsigned char *pGUID,
                                            bool bCAN )
    \brief Convert CANAL message to VSCP event.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertCanalToEventEx(vscpEventEx *pvscpEvent,
                              const canalMsg *pcanalMsg,
                              unsigned char *pGUID)
#else
extern "C" int
vscphlp_convertCanalToEventEx(vscpEventEx *pvscpEvent,
                              const canalMsg *pcanalMsg,
                              unsigned char *pGUID)
#endif
{
    return vscp_convertCanalToEventEx(pvscpEvent, pcanalMsg, pGUID)
             ? VSCP_ERROR_SUCCESS
             : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                           const vscpEvent *pvscpEvent )
    \brief Convert VSCP event to CANAL message.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertEventToCanal(canalMsg *pcanalMsg, const vscpEvent *pvscpEvent)
#else
extern "C" int
vscphlp_convertEventToCanal(canalMsg *pcanalMsg, const vscpEvent *pvscpEvent)
#endif
{
    return vscp_convertEventToCanal(pcanalMsg, pvscpEvent) ? VSCP_ERROR_SUCCESS
                                                           : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                            const vscpEventEx *pvscpEvent )
    \brief Convert VSCP event to CANAL message.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertEventExToCanal(canalMsg *pcanalMsg,
                              const vscpEventEx *pvscpEvent)
#else
extern "C" int
vscphlp_convertEventExToCanal(canalMsg *pcanalMsg,
                              const vscpEventEx *pvscpEvent)
#endif
{
    return vscp_convertEventExToCanal(pcanalMsg, pvscpEvent)
             ? VSCP_ERROR_SUCCESS
             : VSCP_ERROR_ERROR;
}

/*!
    \fn unsigned long vscphlp_makeTimeStamp( void )
    \brief Get VSCP timestamp.
*/
#ifdef WIN32
extern "C" DllExport unsigned long WINAPI EXPORT
vscphlp_makeTimeStamp(void)
#else
extern "C" unsigned long
vscphlp_makeTimeStamp(void)
#endif
{
    return vscp_makeTimeStamp();
}

/*!
    \fn int vscphlp_setEventDateTimeBlock( vscpEvent *pEvent )
    \brief date/time stamp for event.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setEventDateTimeBlockToNow(vscpEvent *pEvent)
#else
extern "C" int
vscphlp_setEventDateTimeBlockToNow(vscpEvent *pEvent)
#endif
{
    return vscp_setEventDateTimeBlockToNow(pEvent) ? VSCP_ERROR_SUCCESS
                                                   : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_setEventDateTimeBlockEx( vscpEventEx *pEventEx )
    \briefSet date/time stamp for Ex event.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setEventExDateTimeBlockToNow(vscpEventEx *pEventEx)
#else
extern "C" int
vscphlp_setEventExDateTimeBlockToNow(vscpEventEx *pEventEx)
#endif
{
    return vscp_setEventExDateTimeBlockToNow(pEventEx) ? VSCP_ERROR_SUCCESS
                                                       : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_copyVSCPEvent( vscpEvent *pEventTo,
                                    const vscpEvent *pEventFrom )
    \brief Copy VSCP event.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_copyVSCPEvent(vscpEvent *pEventTo, const vscpEvent *pEventFrom)
#else
extern "C" int
vscphlp_copyVSCPEvent(vscpEvent *pEventTo, const vscpEvent *pEventFrom)
#endif
{
    return vscp_copyVSCPEvent(pEventTo, pEventFrom) ? VSCP_ERROR_SUCCESS
                                                    : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_writeVscpDataToString( const vscpEvent *pEvent,
                                                char *pstr,
                                                size_t len,
                                                int bUseHtmlBreak  )
    \brief Write VSCP data in readable form to a (multi line) string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeVscpDataToString(const vscpEvent *pEvent,
                              char *pStr,
                              size_t len,
                              int bUseHtmlBreak)
#else
extern "C" int
vscphlp_writeVscpDataToString(const vscpEvent *pEvent,
                              char *pStr,
                              size_t len,
                              int bUseHtmlBreak)
#endif
{
    std::string str;
    bool rv =
      vscp_writeVscpDataToString(pEvent, str, bUseHtmlBreak ? true : false);
    memset(pStr, 0, len);
    strncpy(pStr, str.c_str(), std::min(strlen(str.c_str()), len));
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_writeVscpDataWithSizeToString( const unsigned short
   sizeData, const unsigned char *pData, char *pStr, size_t len, int
   bUseHtmlBreak, int bBreak ) \brief Write VSCP data given with size in
   readable form to a (multi line) string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeVscpDataWithSizeToString(const unsigned char *pData,
                                      const unsigned short sizeData,
                                      char *pStr,
                                      size_t len,
                                      int bUseHtmlBreak,
                                      int bBreak)
#else
extern "C" int
vscphlp_writeVscpDataWithSizeToString(const unsigned char *pData,
                                      const unsigned short sizeData,
                                      char *pStr,
                                      size_t len,
                                      int bUseHtmlBreak,
                                      int bBreak)
#endif
{
    std::string str;
    bool rv = vscp_writeVscpDataWithSizeToString(sizeData,
                                                 pData,
                                                 str,
                                                 bUseHtmlBreak ? true : false,
                                                 bBreak ? true : false);
    memset(pStr, 0, len);
    strncpy(pStr, str.c_str(), std::min(strlen(str.c_str()), len));
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_getVscpDataFromString( vscpEvent *pEvent,
                                            const std::string& str )
    \brief Set data in VSCP event from a string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setVscpDataFromString(vscpEvent *pEvent, const char *pstr)
#else
extern "C" int
vscphlp_setVscpDataFromString(vscpEvent *pEvent, const char *pstr)
#endif
{
    std::string str = std::string(pstr);
    return vscp_setVscpEventDataFromString(pEvent, str) ? VSCP_ERROR_SUCCESS
                                                        : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_getVscpDataAttayFromString( vscpEvent *pEvent,
                                                     unsigned short *psizeData,
                                                     const char *pstr )
    \brief Set data in VSCP event from a string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setVscpDataArrayFromString(unsigned char *pData,
                                   unsigned short *psizeData,
                                   const char *pstr)
#else
extern "C" int
vscphlp_setVscpDataArrayFromString(unsigned char *pData,
                                   unsigned short *psizeData,
                                   const char *pstr)
#endif
{
    std::string str = std::string(pstr);
    return vscp_setVscpDataArrayFromString(pData, psizeData, str)
             ? VSCP_ERROR_SUCCESS
             : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_writeVscpEventToString( vscpEvent *pEvent,
                                            char *p )
    \brief Write VSCP data to a string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeVscpEventToString(vscpEvent *pEvent, char *p, size_t len)
#else
extern "C" int
vscphlp_writeVscpEventToString(vscpEvent *pEvent, char *p, size_t len)
#endif
{
    bool rv;

    std::string str;
    ;
    if ((rv = vscp_writeVscpEventToString(pEvent, str))) {
        memset(p, 0, len);
        strncpy(p, str.c_str(), std::min(strlen(str.c_str()), len));
    }
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_writeVscpEventExToString( vscpEventEx *pEvent,
                                                char *p )
    \brief Write VSCP data to a string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_writeVscpEventExToString(vscpEventEx *pEvent, char *p, size_t len)
#else
extern "C" int
vscphlp_writeVscpEventExToString(vscpEventEx *pEvent, char *p, size_t len)
#endif
{
    bool rv;

    std::string str;
    if ((rv = vscp_writeVscpEventExToString(pEvent, str))) {
        memset(p, 0, len);
        strncpy(p, str.c_str(), std::min(strlen(str.c_str()), len));
    }
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_getVscpEventFromString( vscpEvent *pEvent,
                                            const char *p )
    \brief Get VSCP event from string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setVscpEventFromString(vscpEvent *pEvent, const char *p)
#else
extern "C" int
vscphlp_setVscpEventFromString(vscpEvent *pEvent, const char *p)
#endif
{
    std::string str = std::string(p);
    return vscp_setVscpEventFromString(pEvent, str) ? VSCP_ERROR_SUCCESS
                                                    : VSCP_ERROR_ERROR;
}

/*!
    \fn int vscphlp_setVscpEventExFromString( vscpEventEx *pEvent,
                                            const char *p )
    \brief Get VSCP event from string.
*/
#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_setVscpEventExFromString(vscpEventEx *pEvent, const char *p)
#else
extern "C" int
vscphlp_setVscpEventExFromString(vscpEventEx *pEvent, const char *p)
#endif
{
    std::string str = std::string(p);
    return vscp_setVscpEventExFromString(pEvent, str) ? VSCP_ERROR_SUCCESS
                                                      : VSCP_ERROR_ERROR;
}

// * * * * *    D A T A C O D I N G   * * * * *

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" DllExport unsigned char WINAPI EXPORT
vscphlp_getMeasurementDataCoding(const vscpEvent *pEvent)
#else
extern "C" unsigned char
vscphlp_getMeasurementDataCoding(const vscpEvent *pEvent)
#endif
{
    return vscp_getMeasurementDataCoding(pEvent);
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" DllExport unsigned long long WINAPI EXPORT
vscphlp_getDataCodingBitArray(const unsigned char *pCode, int size)
#else
extern "C" unsigned long long
vscphlp_getDataCodingBitArray(const unsigned char *pCode, int size)
#endif
{
    return vscp_getDataCodingBitArray(pCode, size);
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" DllExport unsigned long long WINAPI EXPORT
vscphlp_getDataCodingInteger(const unsigned char *pCode, int size)
#else
extern "C" unsigned long long
vscphlp_getDataCodingInteger(const unsigned char *pCode, int size)
#endif
{
    return vscp_getDataCodingInteger(pCode, size);
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" DllExport double WINAPI EXPORT
vscphlp_getDataCodingNormalizedInteger(const unsigned char *pCode, int size)
#else
extern "C" double
vscphlp_getDataCodingNormalizedInteger(const unsigned char *pCode, int size)
#endif
{
    return vscp_getDataCodingNormalizedInteger(pCode, size);
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getDataCodingString(const unsigned char *pCode,
                            unsigned char dataLength,
                            char *strResult,
                            size_t len)
#else
extern "C" int
vscphlp_getDataCodingString(const unsigned char *pCode,
                            unsigned char dataLength,
                            char *strResult,
                            size_t len)
#endif
{
    std::string str;

    if (NULL == pCode) return VSCP_ERROR_ERROR;
    if (NULL == strResult) return VSCP_ERROR_ERROR;

    bool rv = vscp_getDataCodingString(pCode, dataLength, str);
    memset(strResult, 0, len);
    strncpy(strResult, str.c_str(), std::min(strlen(str.c_str()), len));
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getVSCPMeasurementAsString(const vscpEvent *pEvent,
                                   char *pResult,
                                   size_t len)
#else
extern "C" int
vscphlp_getVSCPMeasurementAsString(const vscpEvent *pEvent,
                                   char *pResult,
                                   size_t len)
#endif
{
    std::string str;

    if (NULL == pEvent) return VSCP_ERROR_ERROR;

    bool rv = vscp_getVSCPMeasurementAsString(pEvent, str);
    memset(pResult, 0, len);
    strncpy(pResult, str.c_str(), std::min(strlen(str.c_str()), len));
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent, double *pvalue)
#else
extern "C" int
vscphlp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent, double *pvalue)
#endif
{
    std::string str;

    if (NULL == pEvent) return VSCP_ERROR_ERROR;

    return vscp_getVSCPMeasurementAsDouble(pEvent, pvalue) ? VSCP_ERROR_SUCCESS
                                                           : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent,
                                          char *pStrResult,
                                          size_t len)
#else
extern "C" int
vscphlp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent,
                                          char *pStrResult,
                                          size_t len)
#endif
{
    std::string str;

    if (NULL == pEvent) return VSCP_ERROR_ERROR;

    bool rv = vscp_getVSCPMeasurementFloat64AsString(pEvent, str);
    memset(pStrResult, 0, len);
    strncpy(pStrResult, str.c_str(), std::min(strlen(str.c_str()), len));
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertFloatToNormalizedEventData(unsigned char *pdata,
                                          unsigned short *psize,
                                          double value,
                                          unsigned char unit,
                                          unsigned char sensoridx)
#else
extern "C" int
vscphlp_convertFloatToNormalizedEventData(unsigned char *pdata,
                                          unsigned short *psize,
                                          double value,
                                          unsigned char unit,
                                          unsigned char sensoridx)
#endif
{
    std::string str;

    if (NULL == pdata) return VSCP_ERROR_ERROR;

    bool rv = vscp_convertFloatToNormalizedEventData(
      pdata, psize, value, unit, sensoridx);
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertFloatToFloatEventData(unsigned char *pdata,
                                     unsigned short *psize,
                                     float value,
                                     unsigned char unit,
                                     unsigned char sensoridx)
#else
extern "C" int
vscphlp_convertFloatToFloatEventData(unsigned char *pdata,
                                     unsigned short *psize,
                                     float value,
                                     unsigned char unit,
                                     unsigned char sensoridx)
#endif
{
    std::string str;

    if (NULL == pdata) return VSCP_ERROR_ERROR;

    bool rv =
      vscp_convertFloatToFloatEventData(pdata, psize, value, unit, sensoridx);
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertIntegerToNormalizedEventData
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertIntegerToNormalizedEventData(unsigned char *pdata,
                                            unsigned short *psize,
                                            unsigned long long val64,
                                            unsigned char unit,
                                            unsigned char sensoridx)
#else
extern "C" int
vscphlp_convertIntegerToNormalizedEventData(unsigned char *pdata,
                                            unsigned short *psize,
                                            unsigned long long val64,
                                            unsigned char unit,
                                            unsigned char sensoridx)
#endif
{
    std::string str;

    if (NULL == pdata) return VSCP_ERROR_ERROR;
    if (NULL == psize) return VSCP_ERROR_ERROR;

    bool rv = vscp_convertIntegerToNormalizedEventData(
      pdata, psize, val64, unit, sensoridx);
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_makeFloatMeasurementEvent
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_makeFloatMeasurementEvent(vscpEvent *pEvent,
                                  float value,
                                  unsigned char unit,
                                  unsigned char sensoridx)
#else
extern "C" int
vscphlp_makeFloatMeasurementEvent(vscpEvent *pEvent,
                                  float value,
                                  unsigned char unit,
                                  unsigned char sensoridx)
#endif
{
    std::string str;

    if (NULL == pEvent) return VSCP_ERROR_ERROR;

    bool rv = vscp_makeFloatMeasurementEvent(pEvent, value, unit, sensoridx);
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getMeasurementAsFloat
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getMeasurementAsFloat(const unsigned char *pNorm,
                              unsigned char length,
                              float *pResult)
#else
extern "C" int
vscphlp_getMeasurementAsFloat(const unsigned char *pNorm,
                              unsigned char length,
                              float *pResult)
#endif
{
    std::string str;

    if (NULL == pNorm) return VSCP_ERROR_ERROR;
    if (NULL == pResult) return VSCP_ERROR_ERROR;

    *pResult = vscp_getMeasurementAsFloat(pNorm, length);
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getMeasurementUnit
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getMeasurementUnit(const vscpEvent *pEvent)
#else
extern "C" int
vscphlp_getMeasurementUnit(const vscpEvent *pEvent)
#endif
{
    return vscp_getVSCPMeasurementUnit(pEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getMeasurementSensorIndex
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getMeasurementSensorIndex(const vscpEvent *pEvent)
#else
extern "C" int
vscphlp_getMeasurementSensorIndex(const vscpEvent *pEvent)
#endif
{
    return vscp_getVSCPMeasurementSensorIndex(pEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getMeasurementZone
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getMeasurementZone(const vscpEvent *pEvent)
#else
extern "C" int
vscphlp_getMeasurementZone(const vscpEvent *pEvent)
#endif
{
    return vscp_getVSCPMeasurementZone(pEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getMeasurementSubZone
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getMeasurementSubZone(const vscpEvent *pEvent)
#else
extern "C" int
vscphlp_getMeasurementSubZone(const vscpEvent *pEvent)
#endif
{
    return vscp_getVSCPMeasurementSubZone(pEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_isMeasurement
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_isMeasurement(const vscpEvent *pEvent)
#else
extern "C" int
vscphlp_isMeasurement(const vscpEvent *pEvent)
#endif
{
    return vscp_isVSCPMeasurement(pEvent) ? VSCP_ERROR_SUCCESS
                                          : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventToJSON
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertEventToJSON(vscpEvent *pEvent, char *p, size_t len)
#else
extern "C" int
vscphlp_convertEventToJSON(vscpEvent *pEvent, char *p, size_t len)
#endif
{
    std::string str;

    // Do the conversion
    vscp_convertEventToJSON(pEvent, str);

    // Check if there is room for the JSON string
    if (len <= strlen(str.c_str())) return VSCP_ERROR_BUFFER_TO_SMALL;

    // Copy in JSON string
    memset(p, 0, len);
    strncpy(p, str.c_str(), std::min(strlen(str.c_str()), len));

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventExToJSON
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertEventExToJSON(vscpEventEx *pEventEx, char *p, size_t len)
#else
extern "C" int
vscphlp_convertEventExToJSON(vscpEventEx *pEventEx, char *p, size_t len)
#endif
{
    std::string str;

    // Do the conversion
    vscp_convertEventExToJSON(pEventEx, str);

    // Check if there is room for the JSON string
    if (len <= strlen(str.c_str())) return VSCP_ERROR_BUFFER_TO_SMALL;

    // Copy in JSON string
    memset(p, 0, len);
    strncpy(p, str.c_str(), std::min(strlen(str.c_str()), len));

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventToXML
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertEventToXML(vscpEvent *pEvent, char *p, size_t len)
#else
extern "C" int
vscphlp_convertEventToXML(vscpEvent *pEvent, char *p, size_t len)
#endif
{
    std::string str;

    // Do the conversion
    vscp_convertEventToXML(pEvent, str);

    // Check if there is room for the XML string
    if (len <= strlen(str.c_str())) return VSCP_ERROR_BUFFER_TO_SMALL;

    // Copy in XML string
    memset(p, 0, len);
    strncpy(p, str.c_str(), std::min(strlen(str.c_str()), len));

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventExToXML
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertEventExToXML(vscpEventEx *pEventEx, char *p, size_t len)
#else
extern "C" int
vscphlp_convertEventExToXML(vscpEventEx *pEventEx, char *p, size_t len)
#endif
{
    std::string str;

    // Do the conversion
    vscp_convertEventExToXML(pEventEx, str);

    // Check if there is room for the XML string
    if (len <= strlen(str.c_str())) return VSCP_ERROR_BUFFER_TO_SMALL;

    // Copy in XML string
    memset(p, 0, len);
    strncpy(p, str.c_str(), std::min(strlen(str.c_str()), len));

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventToHTML
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertEventToHTML(vscpEvent *pEvent, char *p, size_t len)
#else
extern "C" int
vscphlp_convertEventToHTML(vscpEvent *pEvent, char *p, size_t len)
#endif
{
    std::string str;

    // Do the conversion
    vscp_convertEventToHTML(pEvent, str);

    // Check if there is room for the HTML string
    if (len <= strlen(str.c_str())) return VSCP_ERROR_BUFFER_TO_SMALL;

    // Copy in HTML string
    memset(p, 0, len);
    strncpy(p, str.c_str(), std::min(strlen(str.c_str()), len));

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventExToHTML
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertEventExToHTML(vscpEventEx *pEventEx, char *p, size_t len)
#else
extern "C" int
vscphlp_convertEventExToHTML(vscpEventEx *pEventEx, char *p, size_t len)
#endif
{
    std::string str;

    // Do the conversion
    vscp_convertEventExToHTML(pEventEx, str);

    // Check if there is room for the HTML string
    if (len <= strlen(str.c_str())) return VSCP_ERROR_BUFFER_TO_SMALL;

    // Copy in HTML string
    memset(p, 0, len);
    strncpy(p, str.c_str(), std::min(strlen(str.c_str()), len));

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertLevel1MeasuremenToLevel2Double
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertLevel1MeasuremenToLevel2Double(vscpEvent *pEventLevel1)
#else
extern "C" int
vscphlp_convertLevel1MeasuremenToLevel2Double(vscpEvent *pEventLevel1)
#endif
{
    return (vscp_convertLevel1MeasuremenToLevel2Double(pEventLevel1)
              ? VSCP_ERROR_SUCCESS
              : VSCP_ERROR_ERROR);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertLevel1MeasuremenToLevel2String
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_convertLevel1MeasuremenToLevel2String(vscpEvent *pEventLevel1)
#else
extern "C" int
vscphlp_convertLevel1MeasuremenToLevel2String(vscpEvent *pEventLevel1)
#endif
{
    return (vscp_convertLevel1MeasuremenToLevel2String(pEventLevel1)
              ? VSCP_ERROR_SUCCESS
              : VSCP_ERROR_ERROR);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_makeLevel2FloatMeasurementEvent
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_makeLevel2FloatMeasurementEvent(vscpEvent *pEvent,
                                        uint16_t type,
                                        double value,
                                        uint8_t unit,
                                        uint8_t sensoridx,
                                        uint8_t zone,
                                        uint8_t subzone)
#else
extern "C" int
vscphlp_makeLevel2FloatMeasurementEvent(vscpEvent *pEvent,
                                        uint16_t type,
                                        double value,
                                        uint8_t unit,
                                        uint8_t sensoridx,
                                        uint8_t zone,
                                        uint8_t subzone)
#endif
{
    return (vscp_makeLevel2FloatMeasurementEvent(
              pEvent, type, value, unit, sensoridx, zone, subzone)
              ? VSCP_ERROR_SUCCESS
              : VSCP_ERROR_ERROR);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_makeLevel2StringMeasurementEvent
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_makeLevel2StringMeasurementEvent(vscpEvent *pEvent,
                                         uint16_t type,
                                         double value,
                                         uint8_t unit,
                                         uint8_t sensoridx,
                                         uint8_t zone,
                                         uint8_t subzone)
#else
extern "C" int
vscphlp_makeLevel2StringMeasurementEvent(vscpEvent *pEvent,
                                         uint16_t type,
                                         double value,
                                         uint8_t unit,
                                         uint8_t sensoridx,
                                         uint8_t zone,
                                         uint8_t subzone)
#endif
{
    return (vscp_makeLevel2FloatMeasurementEvent(
              pEvent, type, value, unit, sensoridx, zone, subzone)
              ? VSCP_ERROR_SUCCESS
              : VSCP_ERROR_ERROR);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getTimeString
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getTimeString(char *buf, size_t buf_len, time_t *t)
#else
extern "C" int
vscphlp_getTimeString(char *buf, size_t buf_len, time_t *t)
#endif
{
    // Check pointers
    if (NULL == buf) return VSCP_ERROR_PARAMETER;
    if (NULL == t) return VSCP_ERROR_PARAMETER;

    return vscp_getTimeString(buf, buf_len, t) ? VSCP_ERROR_SUCCESS
                                               : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getISOTimeString
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getISOTimeString(char *buf, size_t buf_len, time_t *t)
#else
extern "C" int
vscphlp_getISOTimeString(char *buf, size_t buf_len, time_t *t)
#endif
{
    // Check pointers
    if (NULL == buf) return VSCP_ERROR_PARAMETER;
    if (NULL == t) return VSCP_ERROR_PARAMETER;

    return vscp_getISOTimeString(buf, buf_len, t) ? VSCP_ERROR_SUCCESS
                                                  : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getDateStringFromEvent
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getDateStringFromEvent(char *buf, size_t buf_len, vscpEvent *pEvent)
#else
extern "C" int
vscphlp_getDateStringFromEvent(char *buf, size_t buf_len, vscpEvent *pEvent)
#endif
{
    // Check pointers
    if (NULL == buf) return VSCP_ERROR_PARAMETER;
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    std::string str;
    vscp_getDateStringFromEvent(pEvent, str);
    if (0 == str.length()) return VSCP_ERROR_ERROR;

    memcpy(buf, (const char *)str.c_str(), std::min(buf_len, str.length()));
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getDateStringFromEventEx
//

#ifdef WIN32
extern "C" DllExport int WINAPI EXPORT
vscphlp_getDateStringFromEventEx(char *buf,
                                 size_t buf_len,
                                 vscpEventEx *pEventEx)
#else
extern "C" int
vscphlp_getDateStringFromEventEx(char *buf,
                                 size_t buf_len,
                                 vscpEventEx *pEventEx)
#endif
{
    // Check pointers
    if (NULL == buf) return VSCP_ERROR_PARAMETER;
    if (NULL == pEventEx) return VSCP_ERROR_PARAMETER;

    std::string str;
    vscp_getDateStringFromEventEx(pEventEx, str);
    if (0 == str.length()) return VSCP_ERROR_ERROR;

    memcpy(buf, (const char *)str.c_str(), std::min(buf_len, str.length()));
    return VSCP_ERROR_SUCCESS;
}