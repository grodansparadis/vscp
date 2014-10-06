// libvscphelper.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//

#ifdef __GNUG__
//#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include  "wx/ownerdrw.h"
#endif

#include "wx/tokenzr.h"

#include "libvscphelper.h"
#include "stdio.h"
#include "stdlib.h"

void _init() __attribute__((constructor));
void _fini() __attribute__((destructor));

void _init()
{
    printf("initializing\n");
}

void _fini()
{
    printf("finishing\n");
}



////////////////////////////////////////////////////////////////////////////
// CVSCPLApp construction

CVSCPLApp::CVSCPLApp()
{
    m_instanceCounter = 0;
    pthread_mutex_init(&m_objMutex, NULL);

    // Init. the driver array
    for (int i = 0; i < VSCP_INTERFACE_MAX_OPEN; i++) {
        m_pvscpifArray[ i ] = NULL;
    }

    UNLOCK_MUTEX(m_objMutex);
}

CVSCPLApp::~CVSCPLApp()
{
    LOCK_MUTEX(m_objMutex);

    for (int i = 0; i < VSCP_INTERFACE_MAX_OPEN; i++) {

        if (NULL == m_pvscpifArray[ i ]) {

            VscpTcpIf *pvscpif = getDriverObject(i);
            if (NULL != pvscpif) {
                pvscpif->doCmdClose();
                delete m_pvscpifArray[ i ];
                m_pvscpifArray[ i ] = NULL;
            }
        }
    }

    UNLOCK_MUTEX(m_objMutex);
    pthread_mutex_destroy(&m_objMutex);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLoggerdllApp object

CVSCPLApp theApp;




///////////////////////////////////////////////////////////////////////////////
// CreateObject


///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CVSCPLApp::addDriverObject(VscpTcpIf *pvscpif)
{
    long h = 0;

    LOCK_MUTEX(m_objMutex);
    for (int i = 0; i < VSCP_INTERFACE_MAX_OPEN; i++) {

        if (NULL == m_pvscpifArray[ i ]) {

            m_pvscpifArray[ i ] = pvscpif;
            h = i + 1681;
            break;

        }

    }

    UNLOCK_MUTEX(m_objMutex);

    return h;
}


///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

VscpTcpIf *CVSCPLApp::getDriverObject(long h)
{
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return NULL;
    if (idx >= VSCP_INTERFACE_MAX_OPEN) return NULL;
    return m_pvscpifArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CVSCPLApp::removeDriverObject(long h)
{
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return;
    if (idx >= VSCP_INTERFACE_MAX_OPEN) return;

    LOCK_MUTEX(m_objMutex);
    if (NULL != m_pvscpifArray[ idx ]) delete m_pvscpifArray[ idx ];
    m_pvscpifArray[ idx ] = NULL;
    UNLOCK_MUTEX(m_objMutex);
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CVSCPLApp::InitInstance()
{
    m_instanceCounter++;
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//                      V S C P   H E L P E R -  A P I
///////////////////////////////////////////////////////////////////////////////



//-----------------------------------------------------------------------------
//						 T C P / I P  I N T E R F A C E
//-----------------------------------------------------------------------------




//-------------------------------------------------------------------------
//
// To use any of the methods below this point you have to 
// obtain a handle first with vscphlp_gethandle and remember 
// to release it with vscphlp_releasehandle when you are done.
//
//-------------------------------------------------------------------------

/*!
    \fn long vscphlp_gethandle(void)
    \brief Initialise the library. This is only needed
            if you use methods which has a handle in them that is
            methods that talk to the VSCP daemon TCP/IP interface.
    \return The handle for the TCP/IP server object or
            zero if the initialisation failed. 
 */

extern "C" long vscphlp_gethandle(void)
{
    VscpTcpIf *pvscpif = new VscpTcpIf;
    if (NULL == pvscpif) return 0;

    return theApp.addDriverObject(pvscpif);
}

/*!
    \fn void vscphlp_releasehandle(long handle)
    \brief Clean up the library. This is only needed
            if you use methods which has a handle in them that is
            methods that talk to the VSCP daemon TCP/IP interface.
    \return The handle for the TCP/IP server object or
            zero if the initialisation failed. 
 */

extern "C" void vscphlp_releasehandle(long handle)
{

    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL != pvscpif) pvscpif->doCmdClose();

    theApp.removeDriverObject(handle);
}


//-------------------------------------------------------------------------

/*!
        Set response timeout
        \param handle to server object
        \param to Timeout value in seconds. (Default = 2 seconds.)
 */
extern "C" void vscphlp_setResponseTimeout(long handle, unsigned char to)
{
    VscpTcpIf *pvscpif = new VscpTcpIf;
    if (NULL == pvscpif) return;

    pvscpif->setResponseTimeout((uint8_t) to);
}

/*!
    \fn int vscphlp_isConnected(const long handle) 
    \brief Check if we are connected
    \param handle to server object
    \return non zero if we are connected, zero otherwise.
 */
extern "C" int vscphlp_isConnected(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    return(pvscpif->isConnected() ? TRUE : FALSE);
}

/*!
    \fn int vscphlp_checkReturnValue( const long handle ) 
    \brief Get variable value from string variable
    \param handle to server object
    \return non-zero for "-OK" and zero for "+OK"
 */
BOOL vscphlp_checkReturnValue(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    return(pvscpif->checkReturnValue() ? TRUE : FALSE);
}

/*!
    Open communication interface.
    \param handle to server object
    \param strInterface should contain "username;password;ip-addr;port" if used. All including 
        port are optional and defaults to no username/password, server as "localhost" and "9598" 
        as default port.
    \param flags are not used at the moment.
    \return CANAL_ERROR_SUCCESS if channel is open or CANAL error code  if error 
    or the channel is already opened or other error occur.
 */
extern "C" long vscphlp_doCmdOpenParam(const long handle,
        const char *pInterface,
        unsigned long flags)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;

    wxString strInterface = wxString::FromAscii(pInterface);
    return pvscpif->doCmdOpen(strInterface, flags);
}

/*!
    Open communication interface.
    \param handle to server object
    \param strHostname Host to connect to.
    \param port TCP/IP port to use.
    \param strUsername Username.
    \param strPassword Username.
    \return CANAL_ERROR_SUCCESS if channel is open or CANAL error code if error 
        or the channel is already opened or other error occur.
 */
extern "C" long vscphlp_doCmdOpen(const long handle,
        const char *pHostname,
        const short port,
        const char *pUsername,
        const char *pPassword)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;

    wxString strHostname = wxString::FromAscii(pHostname);
    wxString strUsername = wxString::FromAscii(pUsername);
    wxString strPassword = wxString::FromAscii(pPassword);
    return pvscpif->doCmdOpen(strHostname,
            port,
            strUsername,
            strPassword);
}

/*!
    \brief Close communication interface
    \param handle to server object
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdClose(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdClose();
}

/*!
    Write NOOP message through pipe.
    \param handle to server object 
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdNOOP(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdNOOP();
}

/*!
    Clear input queue
    \param handle to server object
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdClear(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdClear();
}

/*!
    Get the Canal protocol level
    \param handle to server object
    \return VSCP Level
 */
extern "C" unsigned long vscphlp_doCmdGetLevel(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdGetLevel();
}

/*!
    Send a VSCP Level II event through interface. 
    If a GUID sent with all items set to zero the GUID of the
    interface ("-") will be used.
    \param handle to server object
    \param pEvent VSCP Level II event to send.		
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdSend(const long handle,
        const vscpEvent *pEvent)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdSend(pEvent);
}

/*!
    Send a VSCP ex event through interface. 
    \param handle to server object
    \param pEvent VSCP Level II ex event to send.	
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdSendEx(const long handle,
        const vscpEventEx *pEvent)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdSendEx(pEvent);
}

/*!
    Send a Level I event through interface.
    \param handle to server object 
    \param pMsg VSCP Level I event (==canalMsg).	
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdSendLevel1(const long handle,
        const canalMsg *pMsg)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdSendLevel1(pMsg);
}

/*!
    Receive a VSCP event through the interface. 
    \param handle to server object 
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdReceive(const long handle, vscpEvent *pEvent)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdReceive(pEvent);
}

/*!
    Receive an VSCP ex event through the interface. 
    \param handle to server object 
    \return true if success false if not.
 */
extern "C" int vscphlp_doCmdReceiveEx(const long handle, vscpEventEx *pEvent)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdReceiveEx(pEvent);
}

/*!
    Receive an VSCP Level I event through the interface.
    For the extended and the RTR bit to be handled the
    CAN message bit in the head byte must be set.
    Low eight bits of the CAN id is fetched from GUID[15]
    that is LSB of GUID.
    \param handle to server object 
    \param Pointer to CANAL message
    \return CANAL_ERROR_SUCCESS if success false if not.
 */
extern "C" int vscphlp_doCmdReceiveLevel1(const long handle, canalMsg *pCanalMsg)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdReceiveLevel1(pCanalMsg);
}

/*!
    This command sets an open interface in the receive loop (RCVLOOP). 
    It does nothing other then putting the interface in the loop mode and 
    checking that it went there.

    Note! The only way to terminate this receive loop is to close the session with 
    doCmdClose (or just close the socket).
    \param handle to server object
    \return CANAL_ERROR_SUCCESS if success CANAL_ERROR_GENERIC if not.
 */
extern "C" int vscphlp_doCmdEnterReceiveLoop(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdEnterReceiveLoop();
}

/*!
    Receive an event 
    The receiveloop command must have been issued for this method to work as 
    it sets up the ringbuffer used for the blocking receive.
    \param handle to server object
    \param pEvent Pointer to VSCP event.
    \param timeout Timout to wait for data. Recommended 500 milliseconds.
    \return CANAL_ERROR_SUCCESS when event is received. CANAL_ERROR_FIFO_EMPTY is
    returned if no event was available. CANAL_ERROR_TIMEOUT on timeout.CANAL_ERROR_COMMUNICATION
    is returned if a socket error is detected.
 */
extern "C" int vscphlp_doCmdBlockReceive(const long handle,
        vscpEvent *pEvent,
        unsigned long timeout)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdBlockReceive(pEvent, timeout);
}

/*!
    Get the number of events in the input queue of this interface
    \param handle to server object
    \return the number of events available or if negative
    an error code.
 */
extern "C" int vscphlp_doCmdDataAvailable(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdDataAvailable();
}

/*!
    Receive CAN status through the interface. 
    \param handle to server object
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdStatus(const long handle, canalStatus *pStatus)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdStatus(pStatus);
}

/*!
    Receive CAN statistics through the interface. 
    \param handle to server object
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdStatistics(const long handle,
        canalStatistics *pStatistics)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdStatistics(pStatistics);
}

/*!
    Set/Reset a filter through the interface.
    \param handle to server object 
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdFilter(const long handle, const vscpEventFilter *pFilter)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdFilter(pFilter);
}

/*!
    Set/Reset filter through the interface. 
    \param handle to server object
    \param filter Filter on string form (priority,class,type,guid).
    \param mask Mask on string form (priority,class,type,guid).
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdFilterString(const long handle,
        const char *pfilter,
        const char *pmask)
{
    wxString filter = wxString::FromAscii(pfilter);
    wxString mask = wxString::FromAscii(pmask);

    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdFilter(filter, mask);
}

/*!
    Get i/f version through the interface. 
    \param handle to server object
    \return version number.
 */
extern "C" unsigned long vscphlp_doCmdVersion(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdVersion();
}

/*!
    Get interface version
    \param handle to server object
 */
extern "C" unsigned long vscphlp_doCmdDLLVersion(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdDLLVersion();
}

/*!
    Get vendor string
    \param handle to server object
 */
extern "C" const char *vscphlp_doCmdVendorString(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return NULL;
    return pvscpif->doCmdVendorString();
}

/*!
    Get Driver information string.
    \param handle to server object
 */
extern "C" const char *vscphlp_doCmdGetDriverInfo(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return NULL;
    return pvscpif->doCmdGetDriverInfo();
}

/*!
    Get GUID for this interface. 
    \param handle to server object
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdGetGUID(const long handle, char *pGUID)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdGetGUID(pGUID);
}

/*!
        Set GUID for this interface.  
        \param handle to server object
        \param pGUID Array with uint8_t GUID's. (Note! Not a string).
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdSetGUID(const long handle, const char *pGUID)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdSetGUID(pGUID);
}

/*!
    Get information about a channel.. 
    \param handle to server object
    \param pChannelInfo The stucture that will get the information
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdGetChannelInfo(const long handle,
        VSCPChannelInfo *pChannelInfo)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdGetChannelInfo(pChannelInfo);
}

/*!
    Get Channel ID for the open channel
    \param handle to server object
    \param pChannelID Channel ID for channel
    \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdGetChannelID(const long handle,
        uint32_t *pChannelID)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdGetChannelID(pChannelID);
}

/*!
       Get available interfaces
       \param handle to server object
       \param array Returns a pointer to a string array that will have 
                interface list allocated as a list of strings. It's the 
                callers responsibility to free this storage. Not also
                that a NULL pointer can be returned if something went wrong
                or the interface list is empty.
       \return CANAL_ERROR_SUCCESS on success and error code if failure.
 */
extern "C" int vscphlp_doCmdInterfaceList(const long handle, char **parray)
{
    wxArrayString array;
    char *p;
    char buf[512];

    parray = NULL;

    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    if (!pvscpif->doCmdInterfaceList(array)) return CANAL_ERROR_INIT_FAIL;

    if (0 == array.Count()) return CANAL_ERROR_SUCCESS;

    // Create the array
    parray = new char *[array.Count()];
    if (NULL == parray) return CANAL_ERROR_MEMORY;

    // Fill the array
    for (int i = 0; i < array.Count(); i++) {
        wxString str = array.Item(i);
        if (p = new char(str.Length() + 1)) {
            memset(p, 0, str.Length() + 1);
            memcpy(parray[i], p, str.Length());
        }
    }

    return CANAL_ERROR_SUCCESS;
}

/*!
    Shutdown the VSCP daemon.
    \param handle to server object
 */
extern "C" int vscphlp_doCmdShutDown(const long handle)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_INIT_FAIL;
    return pvscpif->doCmdShutDown();
}

//-------------------------------------------------------------------------
//					V A R I A B L E  H A N D L I N G
//-------------------------------------------------------------------------

/*!
    \fn int vscphlp_getVariableString( const char *pName, char *pValue ) 
    \brief Get variable value from string variable
    \param handle to server object
    \param name of variable
    \param pointer to string that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableString(const long handle,
        const char *pName,
        char *pValue)
{
    wxString name = wxString::FromAscii(pName);
    wxString strValue;

    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    if (!pvscpif->getVariableString(name, &strValue)) {
        return FALSE;
    }

    strcpy(pValue, strValue.ToAscii());

    return TRUE;
}

/*!
    \fn int vscphlp_setVariableString( const char *pName, char *pValue ) 
    \brief set string variable from a pointer to a string
    \param handle to server object
    \param name of variable
    \param pointer to string that contains the string.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableString(const long handle, const char *pName, char *pValue)
{
    // Check pointers
    if (NULL == pName) return FALSE;
    if (NULL == pValue) return FALSE;

    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    wxString strValue = wxString::FromAscii(pValue);
    return pvscpif->setVariableString(name, strValue);

    return TRUE;
}

/*!
    \fn int vscphlp_getVariableBool( const char *pName, bool *bValue )
    \brief Get variable value from boolean variable
    \param handle to server object
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableBool(const long handle, const char *pName, bool *bValue)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->getVariableBool(name, bValue) ? TRUE : FALSE);
};

/*!
    \fn int vscphlp_setVariableBool( const char *pName, bool bValue )
    \brief Get variable value from boolean variable
    \param handle to server object
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableBool(const long handle, const char *pName, bool bValue)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->setVariableBool(name, bValue) ? TRUE : FALSE);
};

/*!
    \fn int vscphlp_getVariableInt( const char *pName, int *value )
    \brief Get variable value from integer variable
    \param handle to server object
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableInt(const long handle, const char *pName, int *value)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return false;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->getVariableInt(name, value) ? TRUE : FALSE);
};

/*!
    \fn int vscphlp_setVariableInt( const char *pName, int value )
    \brief Get variable value from integer variable
    \param handle to server object
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableInt(const long handle, const char *pName, int value)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->setVariableInt(name, value) ? TRUE : FALSE);
};

/*!
    \fn int vscphlp_getVariableLong( const char *pName, long *value )
    \brief Get variable value from long variable
    \param handle to server object
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableLong(const long handle, const char *pName, long *value)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->getVariableLong(name, value) ? TRUE : FALSE);
};

/*!
    \fn int vscphlp_setVariableLong( const char *pName, long value )
    \brief Get variable value from long variable
    \param handle to server object
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableLong(const long handle, const char *pName, long value)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->setVariableLong(name, value) ? TRUE : FALSE);
};

/*!
    \fn int vscphlp_getVariableDouble( const char *pName, double *value )
    \brief Get variable value from double variable
    \param handle to server object
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableDouble(const long handle, const char *pName, double *value)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->getVariableDouble(name, value) ? TRUE : FALSE);
};

/*!
    \fn int vscphlp_setVariableDouble( const char *pName, double value )
    \brief Get variable value from double variable
    \param handle to server object
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableDouble(const long handle, const char *pName, double value)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->setVariableDouble(name, value) ? TRUE : FALSE);
};

/*!
    \fn int vscphlp_getVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param handle to server object
    \param name of variable
    \param String that get that get the 
    value of the measurement.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableMeasurement(const long handle, const char *pName, char *pValue)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    wxString strValue;
    if (!pvscpif->getVariableMeasurement(name, strValue)) {
        return FALSE;
    }

    strcpy(pValue, strValue.ToAscii());

    return TRUE;
};

/*!
    \fn int vscphlp_setVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param handle to server object
    \param name of variable
    \param String that get that get the 
    value of the measurement.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableMeasurement(const long handle, const char *pName, char *pValue)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    wxString strValue;
    return(pvscpif->setVariableMeasurement(name, strValue) ? TRUE : FALSE);
};

/*!
    \fn int vscphlp_getVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param handle to server object
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableEvent(const long handle, const char *pName, vscpEvent *pEvent)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->getVariableEvent(name, pEvent) ? TRUE : FALSE);
}

/*!
    \fn int vscphlp_setVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param handle to server object
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableEvent(const long handle, const char *pName, vscpEvent *pEvent)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->setVariableEvent(name, pEvent) ? TRUE : FALSE);
}

/*!
    \fn int vscphlp_getVariableEventEx( const char *pName, vscpEventEx *pEvent )
    \brief Get variable value from event variable
    \param handle to server object
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableEventEx(const long handle, const char *pName, vscpEventEx *pEvent)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->getVariableEventEx(name, pEvent) ? TRUE : FALSE);
}

/*!
    \fn int vscphlp_setVariableEventEx( const char *pName, vscpEventEx *pEvent )
    \brief Get variable value from event variable
    \param handle to server object
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableEventEx(const long handle, const char *pName, vscpEventEx *pEvent)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->setVariableEventEx(name, pEvent) ? TRUE : FALSE);
}

/*!
    \fn int vscphlp_getVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param handle to server object
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableGUID(const long handle, const char *pName, cguid& GUID)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->getVariableGUID(name, GUID) ? TRUE : FALSE);
}

/*!
    \fn int vscphlp_setVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param handle to server object
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableGUID(const long handle, const char *pName, cguid& GUID)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->setVariableGUID(name, GUID) ? TRUE : FALSE);
}

/*!
    \fn int vscphlp_getVariableVSCPdata( const char *pName, uint16_t *psizeData, uint8_t *pData )
    \brief Get variable value from VSCP data variable
    \param handle to server object
    \param name of variable
    \param Pointer to variable that will hold the size of the data array
    \param pointer to VSCP data array variable (unsigned char [8] ) that get the 
    value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableVSCPdata(const long handle, const char *pName, uint16_t *psizeData, uint8_t *pData)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->getVariableVSCPdata(name, psizeData, pData) ? TRUE : FALSE);
}

/*!
    \fn int vscphlp_setVariableVSCPdata( const char *pName, uint16_t sizeData, uint8_t *pData )
    \brief Get variable value from VSCP data variable
    \param handle to server object
    \param name of variable
    \param Pointer to variable that will hold the size of the data array
    \param pointer to VSCP data array variable (unsigned char [8] ) that get the 
    value of the string variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableVSCPdata(const long handle,
        const char *pName,
        uint16_t sizeData,
        uint8_t *pData)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->setVariableVSCPdata(name, sizeData, pData) ? TRUE : FALSE);
}

/*!
    \fn int vscphlp_getVariableVSCPclass( const char *pName, uint16_t *vscp_class )
    \brief Get variable value from class variable
    \param handle to server object
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return true if the variable is of type string.
 */
extern "C" int vscp_getVariableVSCPclass(const long handle,
        const char *pName,
        uint16_t *vscp_class)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->getVariableVSCPclass(name, vscp_class) ? TRUE : FALSE);
}

/*!
    \fn v vscp_setVariableVSCPclass( const char *pName, uint16_t vscp_class )
    \brief Get variable value from class variable
    \param handle to server object
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableVSCPclass(const long handle, const char *pName, uint16_t vscp_class)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->setVariableVSCPclass(name, vscp_class) ? TRUE : FALSE);
}

/*!
    \fn int vscp_getVariableVSCPtype( const char *pName, uint8_t *vscp_type )
    \brief Get variable value from type variable
    \param handle to server object
    \param name of variable
    \param pointer to int that get the value of the type variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_getVariableVSCPtype(const long handle, const char *pName, uint8_t *vscp_type)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->getVariableVSCPtype(name, vscp_type) ? TRUE : FALSE);
}

/*!
    \fn int vscphlp_setVariableVSCPtype( const char *pName, uint8_t vscp_type )
    \brief Get variable value from type variable
    \param handle to server object
    \param name of variable
    \param pointer to int that get the value of the type variable.
    \return true if the variable is of type string.
 */
extern "C" int vscphlp_setVariableVSCPtype(const long handle, const char *pName, uint8_t vscp_type)
{
    // Get VSCP TCP/IP object
    VscpTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return FALSE;

    wxString name = wxString::FromAscii(pName);
    return(pvscpif->setVariableVSCPtype(name, vscp_type) ? TRUE : FALSE);
}






//-----------------------------------------------------------------------------
//								H E L P E R S
//-----------------------------------------------------------------------------








/*!
  Get bitarray from coded event data
  \param pNorm Pointer to normalized integer.
  \param length Number of bytes it consist of including
  the first normalize byte.
  \return Bitarray as a unsigned 64-bit integer.
 */
extern "C" unsigned long vscphlp_getDataCodingBitArray(const unsigned char *pNorm,
        const unsigned char length)
{
    return vscp_getDataCodingBitArray(pNorm, length);
}

/*!
      Get normalized integer from coded event data
      \param pNorm Pointer to normalized integer.
      \param length Number of bytes it consist of including
      the first normalize byte.
      \return returns value as a double.
 */
extern "C" double vscphlp_getDataCodingNormalizedInteger(const unsigned char
        *pNorm, const unsigned char length)
{
    return vscp_getDataCodingNormalizedInteger(pNorm, length);
}

/*!
  Get the string from coded event data
  \param pString Pointer to normalised integer.
  \param length Number of bytes it consist of including
  the first normalise byte.
  \param pstrCoding String that will hold the result.
  \param 
  \return Returns unicode UTF-8 string of event data
 */
extern "C" void vscphlp_getDataCodingString(const unsigned char *pString,
        const unsigned char length,
        char *pstrCoding,
        int len)
{
    // Check pointer
    if (NULL == pstrCoding) return;

    wxString str = vscp_getDataCodingString(pString, length);
    strncpy(pstrCoding, str.ToAscii(), len);
}

/*!
  Get data in the VSCP data coding format to a string
  \param pEvent Pointer to VSCP event.
  \param str String that holds the result
  \return true on success, false on failure.
 */
extern "C" BOOL vscphlp_getVSCPMeasurementAsString(const vscpEvent *pEvent,
        char *pStr,
        int len)
{
    wxString wxstr;
    if (!vscp_getVSCPMeasurementAsString(pEvent, wxstr)) return FALSE;

    strncpy(pStr, wxstr.ToAscii(), len);

    return TRUE;
}

/*!
  Get data in the VSCP data coding format to a float
  \param pNorm Pointer to VSCP event.
  \param length Number of bytes it consist of including datacoding byte
  \return value as float
 */
extern "C" float vscphlp_getMeasurementAsFloat(const unsigned char *pNorm,
        const unsigned char length)
{
    return vscp_getMeasurementAsFloat(pNorm, length);
}

/*!
  Get data in the VSCP data coding format to a float
  \param value Floating point value to convert.
  \param pdata Pointer to first byte of VSCP data.
  \param unit Code for the unit the data is represented in.
  \param sendoridx Index for sensor-   
  \return value as float
 */
extern "C" int vscphlp_convertFloatToNormalizedEventData(double value,
        unsigned char *pdata,
        unsigned int *psize,
        unsigned char unit,
        unsigned char sensoridx)
{
    uint16_t size;
    int rv =  vscp_convertFloatToNormalizedEventData(value,
                                                    pdata,
                                                    &size,
                                                    unit,
                                                    sensoridx);
    
    *psize = size;
    return rv;
}

/*!
    \fn long vscphlp_readStringValue( const char * pStrValue )
    \brief Read a value (decimal or hex) from a string.
    \return The converted number.
 */

extern "C" unsigned long vscphlp_readStringValue(const char * pStrValue)
{
    wxString strVal;
    strVal.FromAscii(pStrValue);

    return vscp_readStringValue(strVal);
}

/*!
    \fn unsigned char vscphlp_getVscpPriority( const vscpEvent *pEvent )
    \brief Get VSCP priority.
 */

extern "C" unsigned char vscphlp_getVscpPriority(const vscpEvent *pEvent)
{
    return vscp_getVscpPriority(pEvent);
}

/*!
    \fn unsigned char vscphlp_getVscpPriority( const vscpEvent *pEvent )
    \brief Get VSCP priority.
 */

extern "C" unsigned char vscphlp_getVscpPriorityEx(const vscpEventEx *pEvent)
{
    return vscp_getVscpPriorityEx(pEvent);
}

/*!
    \fn void vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority )
    \brief Set VSCP priority.
 */

extern "C" void vscphlp_setVscpPriority(vscpEvent *pEvent, unsigned char priority)
{
    vscp_setVscpPriority(pEvent, priority);
}

/*!
 Set VSCP priority Ex
 \param pEvent Pointer to VSCP event to set priority for.
 \param priority Priority (0-7) to set.
 */
extern "C" void vscphlp_setVscpPriorityEx(vscpEventEx *pEvent, unsigned char priority)
{
    vscp_setVscpPriorityEx(pEvent, priority);
}

/*!
    \fn vscphlp_getVSCPheadFromCANid( const unsigned long id )
    \brief Get the VSCP head from a CANAL message id (CAN id).
 */

extern "C" unsigned char vscphlp_getVSCPheadFromCANid(const unsigned long id)
{
    return vscp_getVSCPheadFromCANid(id);
}

/*!
    \fn vscphlp_getVSCPclassFromCANid( const unsigned long id )
    \brief Get the VSCP class from a CANAL message id (CAN id).
 */

extern "C" unsigned short vscphlp_getVSCPclassFromCANid(const unsigned long id)
{
    return vscp_getVSCPclassFromCANid(id);
}

/*!
    \fn unsigned short vscphlp_getVSCPtypeFromCANid( const unsigned long id )
    \brief Get the VSCP type from a a CANAL message id (CAN id).
 */

extern "C" unsigned short vscphlp_getVSCPtypeFromCANid(const unsigned long id)
{
    return vscp_getVSCPtypeFromCANid(id);
}

/*!
    \fn unsigned short vscphlp_getVSCPnicknameFromCANid( const unsigned long id )
    \brief Get the VSCP nickname from a a CANAL message id (CAN id).
 */

extern "C" unsigned short vscphlp_getVSCPnicknameFromCANid(const unsigned long id)
{
    return vscp_getVSCPnicknameFromCANid(id);
}

/*!
    \fn unsigned long vscphlp_getCANidFromVSCPdata( const unsigned char priority, 
                                                    const unsigned short vscphlp_class, 
                                                    const unsigned short vscp_type )
    \brief Construct a CANAL id (CAN id ) from VSCP.
 */

extern "C" unsigned long vscphlp_getCANidFromVSCPdata(const unsigned char priority,
        const unsigned short vscphlp_class,
        const unsigned short vscp_type)
{
    return vscp_getCANidFromVSCPdata(priority, vscphlp_class, vscp_type);
}

/*!
    \fn unsigned long vscphlp_getCANidFromVSCPevent( const vscpEvent *pEvent )
    \brief Get CANAL id (CAN id) from VSCP event.
 */

extern "C" unsigned long vscphlp_getCANidFromVSCPevent(const vscpEvent *pEvent)
{
    return vscp_getCANidFromVSCPevent(pEvent);
}

/*!
  Get CAN id from VSCP event
  \param pEvent Pointer to VSCP event
  \return CAN id with nickname == 0
 */
extern "C" unsigned long vscphlp_getCANidFromVSCPeventEx(const vscpEventEx *pEvent)
{
    return vscp_getCANidFromVSCPeventEx(pEvent);
}

/*!
    \fn unsigned short vscphlp_calcCRC( vscpEvent *pEvent, short bSet )
    \brief Calculate VSCP crc.
 */

extern "C" unsigned short vscphlp_calcCRC(vscpEvent *pEvent, short bSet)
{
    return vscp_vscp_calc_crc(pEvent, bSet);
}


/*!
    \fn unsigned short vscp_getCrcOfGuidFromArray( const unsigned char * pGUID )
    \brief Calculate 8-bit crc for GUID in array
	\param Pointer to GUID array
 	\return 8-bit crc of GUID.
*/

extern "C" bool vscp_getCrcOfGuidFromArray( const unsigned char * pGUID )
{
    return vscp_calcCRC4GUIDArray( pGUID );
}


/*!
    \fn bool vscp_getGuidFromString( const char * strGUID )
    \brief Calculate 8-bit crc for GUID in array
    \param Pointer to GUID string.
 	\return 8-bit crc of GUID.
*/

extern "C" bool vscp_getCrcOfGuidFromString( const char * strGUID )
{
    wxString wxGUID = wxString::FromAscii( strGUID );
    return  vscp_calcCRC4GUIDString( wxGUID );
}


/*!
    \fn BOOL vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
    \brief Write GUID into VSCP event from string.
 */

extern "C" BOOL vscphlp_getGuidFromString(vscpEvent *pEvent, const char * pGUID)
{
    wxString strGUID = wxString::FromAscii(pGUID);
    return (vscp_getGuidFromString(pEvent, strGUID) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
    \brief Write GUID into VSCP event from string.
 */

extern "C" BOOL vscphlp_getGuidFromStringEx(vscpEventEx *pEvent, const char * pGUID)
{
    wxString strGUID = wxString::FromAscii(pGUID);
    return (vscp_getGuidFromStringEx(pEvent, strGUID) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
    \brief Write GUID from string into array.
 */

extern "C" BOOL vscphlp_getGuidFromStringToArray(uint8_t *pGUID, const char * pStr)
{
    wxString strGUID = wxString::FromAscii(pStr);
    return (vscp_getGuidFromStringToArray(pGUID, strGUID) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_writeGuidToString( const vscpEvent *pEvent, char * pStr )
    \brief Write GUID froom VSCP event to string.
 */

extern "C" BOOL vscphlp_writeGuidToString(const vscpEvent *pEvent, char * pStr)
{
    BOOL rv;

    wxString strGUID;
    rv = (vscp_writeGuidToString(pEvent, strGUID) ? TRUE : FALSE);
    strcpy(pStr, strGUID.ToAscii());
    return rv;
}

extern "C" BOOL vscphlp_writeGuidToStringEx(const vscpEventEx *pEvent, char * pStr)
{
    BOOL rv;

    wxString strGUID;
    rv = (vscp_writeGuidToStringEx(pEvent, strGUID) ? TRUE : FALSE);
    strcpy(pStr, strGUID.ToAscii());
    return rv;
}

/*!
    \fn BOOL vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                            wxString& strGUID )
    \brief Write GUID from VSCP event to string with four bytes on each
    row seperated by \r\n. 
 */

extern "C" BOOL vscphlp_writeGuidToString4Rows(const vscpEvent *pEvent,
        wxString& strGUID)
{
    return (vscp_writeGuidToString4Rows(pEvent, strGUID) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                            wxString& strGUID )
    \brief Write GUID from VSCP event to string with four bytes on each
    row seperated by \r\n. 
 */

extern "C" BOOL vscphlp_writeGuidToString4RowsEx(const vscpEventEx *pEvent,
        wxString& strGUID)
{
    return (vscp_writeGuidToString4RowsEx(pEvent, strGUID) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_writeGuidArrayToString( const unsigned char * pGUID, 
                                            wxString& strGUID )
    \brief Write GUID from byte array to string.
 */

extern "C" BOOL vscphlp_writeGuidArrayToString(const unsigned char * pGUID,
        wxString& strGUID)
{
    return (vscp_writeGuidArrayToString(pGUID, strGUID) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_isGUIDEmpty( unsigned char *pGUID )
    \brief Check if GUID is empty (all nills).
 */

extern "C" BOOL vscphlp_isGUIDEmpty(unsigned char *pGUID)
{
    return (vscp_isGUIDEmpty(pGUID) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_isSameGUID( const unsigned char *pGUID1, 
                                const unsigned char *pGUID2 )
    \brief Check if two GUID's is equal to each other.
 */

extern "C" BOOL vscphlp_isSameGUID(const unsigned char *pGUID1,
        const unsigned char *pGUID2)
{
    return (vscp_isSameGUID(pGUID1, pGUID2) ? TRUE : FALSE);
}

/*!
    Recerse GUID
    \param pGUID Pointer to GUID to reverse.
    \return true if OK.
 */
extern "C" BOOL vscphlp_reverseGUID(unsigned char *pGUID)
{
    return (vscp_reverseGUID(pGUID) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                    const vscpEvent *pEvent )
    \brief Convert VSCP standard event form to ex. form.
 */

extern "C" BOOL vscphlp_convertVSCPtoEx(vscpEventEx *pEventEx,
        const vscpEvent *pEvent)
{
    return (vscp_convertVSCPtoEx(pEventEx, pEvent) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_convertVSCPfromEx( vscpEvent *pEvent, 
                                        const vscpEventEx *pEventEx )
    \brief Convert VSCP ex. event form to standard form.
 */

extern "C" BOOL vscphlp_convertVSCPfromEx(vscpEvent *pEvent,
        const vscpEventEx *pEventEx)
{
    return (vscp_convertVSCPfromEx(pEvent, pEventEx) ? TRUE : FALSE);
}

/*!
    \fn void vscphlp_deleteVSCPevent( vscpEvent *pEvent )
    \brief Delete VSCP event.
 */

extern "C" void vscphlp_deleteVSCPevent(vscpEvent *pEvent)
{
    vscp_deleteVSCPevent(pEvent);
}

/*!
    \fn void vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx )
    \brief Delete VSCP event ex.
 */

extern "C" void vscphlp_deleteVSCPeventEx(vscpEventEx *pEventEx)
{
    vscp_deleteVSCPeventEx(pEventEx);
}

/*!
    \fn void vscphlp_clearVSCPFilter( vscpEventFilter *pFilter )
    \brief Clear VSCP filter.
 */

extern "C" void vscphlp_clearVSCPFilter(vscpEventFilter *pFilter)
{
    vscp_clearVSCPFilter(pFilter);
}

/*!
    \fn BOOL vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                    const vscpEventFilter *pFilter )
    \brief Check VSCP filter condition.
 */

extern "C" BOOL vscphlp_doLevel2Filter(const vscpEvent *pEvent,
        const vscpEventFilter *pFilter)
{
    return (vscp_doLevel2Filter(pEvent, pFilter) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                    const vscpEventFilter *pFilter )
    \brief Check VSCP filter condition.
 */

extern "C" BOOL vscphlp_doLevel2FilterEx(const vscpEventEx *pEvent,
        const vscpEventFilter *pFilter)
{
    return (vscp_doLevel2FilterEx(pEvent, pFilter) ? TRUE : FALSE);
}

/*!
    \fn BOOL readFilterFromString( vscpEventFilter *pFilter, wxString& strFilter )
    \brief Read a filter from a string
    \param pFilter Filter structure to write filter to.
    \param strFilter Filter in string form 
                filter-priority, filter-class, filter-type, filter-GUID
    \return true on success, false on failure.
 */

extern "C" BOOL vscphlp_readFilterFromString(vscpEventFilter *pFilter, wxString& strFilter)
{
    return (vscp_readFilterFromString(pFilter, strFilter) ? TRUE : FALSE);
}

/*!
    \fn BOOL readMaskFromString( vscpEventFilter *pFilter, wxString& strMask )
    \brief Read a mask from a string
    \param pFilter Filter structure to write mask to.
    \param strMask Mask in string form 
                mask-priority, mask-class, mask-type, mask-GUID
    \return true on success, fals eon failure.
 */

extern "C" BOOL vscphlp_readMaskFromString(vscpEventFilter *pFilter, wxString& strMask)
{
    return (vscp_readMaskFromString(pFilter, strMask) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                            const canalMsg *pcanalMsg,
                                            unsigned char *pGUID,
                                            BOOL bCAN )
    \brief Convert CANAL message to VSCP event.
 */

extern "C" BOOL vscphlp_convertCanalToEvent(vscpEvent *pvscpEvent,
        const canalMsg *pcanalMsg,
        unsigned char *pGUID,
        BOOL bCAN)
{
    return vscp_convertCanalToEvent(pvscpEvent,
            pcanalMsg,
            pGUID,
            (bCAN ? true : false));
}

/*!
    \fn BOOL vscphlp_convertCanalToEventEx( vscpEventEx *pvscpEvent,
                                            const canalMsg *pcanalMsg,
                                            unsigned char *pGUID,
                                            BOOL bCAN )
    \brief Convert CANAL message to VSCP event.
 */

extern "C" BOOL vscphlp_convertCanalToEventEx(vscpEventEx *pvscpEvent,
        const canalMsg *pcanalMsg,
        unsigned char *pGUID,
        BOOL bCAN)
{
    return vscp_convertCanalToEventEx(pvscpEvent,
            pcanalMsg,
            pGUID,
            (bCAN ? true : false));
}

/*!
    \fn BOOL vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                        const vscpEvent *pvscpEvent )
    \brief Convert VSCP event to CANAL message.
 */

extern "C" BOOL vscphlp_convertEventToCanal(canalMsg *pcanalMsg,
        const vscpEvent *pvscpEvent)
{
    return (vscp_convertEventToCanal(pcanalMsg, pvscpEvent) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                            const vscpEventEx *pvscpEventEx )
    \brief Convert VSCP event ex. to CANAL message.
 */

extern "C" BOOL vscphlp_convertEventExToCanal(canalMsg *pcanalMsg,
        const vscpEventEx *pvscpEventEx)
{
    return (vscp_convertEventExToCanal(pcanalMsg, pvscpEventEx) ? TRUE : FALSE);
}

/*!
    \fn unsigned long vscphlp_getTimeStamp( void )
    \brief Get VSCP timestamp.
 */

extern "C" unsigned long vscphlp_getTimeStamp(void)
{
    return vscp_makeTimeStamp();
}

/*!
    \fn BOOL vscphlp_copyVSCPEvent( vscpEvent *pEventTo, 
                                    const vscpEvent *pEventFrom )
    \brief Copy VSCP event.
 */

extern "C" BOOL vscphlp_copyVSCPEvent(vscpEvent *pEventTo,
        const vscpEvent *pEventFrom)
{
    return (vscp_copyVSCPEvent(pEventTo, pEventFrom) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_writeVscpDataToString( const vscpEvent *pEvent, 
                                            wxString& str, 
                                            BOOL bUseHtmlBreak )
    \brief Write VSCP data in readable form to a (multiline) string.
 */

extern "C" BOOL vscphlp_writeVscpDataToString(const vscpEvent *pEvent,
        wxString& str,
        BOOL bUseHtmlBreak)
{
    return (vscp_writeVscpDataToString(pEvent,
            str,
            (bUseHtmlBreak ? true : false)) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_getVscpDataFromString( vscpEvent *pEvent, 
                                            const wxString& str )
    \brief Set data in VSCP event from a string.
 */
extern "C" BOOL vscphlp_getVscpDataFromString(vscpEvent *pEvent,
        const wxString& str)
{
    return (vscp_getVscpDataFromString(pEvent, str) ? TRUE : FALSE);
}

/*!
    \fn BOOL vscphlp_writeVscpEventToString( vscpEvent *pEvent, 
                                            char *p )
    \brief Write VSCP data to a string.
 */

extern "C" BOOL vscphlp_writeVscpEventToString(vscpEvent *pEvent,
        char *p)
{
    BOOL rv;

    wxString str = wxString::FromAscii(p);
    if ((rv = (vscp_writeVscpEventToString(pEvent, str) ? TRUE : FALSE))) {
        strcpy(p, str.ToAscii());
    }

    return rv;
}

/*!
    \fn BOOL vscphlp_getVscpEventFromString( vscpEvent *pEvent, 
                                            const char *p )
    \brief Get VSCP event from string.
 */

extern "C" BOOL vscphlp_getVscpEventFromString(vscpEvent *pEvent,
        const char *p)
{
    wxString str = wxString::FromAscii(p);
    return (vscp_getVscpEventFromString(pEvent, str) ? TRUE : FALSE);
}

/*!
  Write VSCP data to string
  \param sizeData Number of databytes.
  \param pData Pointer to datastructure.
   \param str String that receive result.
  \param bUseHtmlBreak Set to true to use <br> instead of \\n as
  line break 
  \return True on success false on failure.
 */
extern "C" BOOL vscphlp_writeVscpDataWithSizeToString(const uint16_t sizeData,
        const unsigned char *pData,
        char *pstr,
        BOOL bUseHtmlBreak)
{
    wxString wxstr = wxString::FromAscii(pstr);
    return vscp_writeVscpDataWithSizeToString(sizeData,
            pData,
            wxstr,
            (bUseHtmlBreak ? true : false));
}


