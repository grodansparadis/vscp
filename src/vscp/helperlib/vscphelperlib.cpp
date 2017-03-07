// VSCP helper dll.cpp : Defines the initialization routines for the DLL.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2017 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
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

#include "wx/wxprec.h"
#include "wx/wx.h"

#include <stdio.h>
#include <stdlib.h>

#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include <wx/utils.h>

#include <vscphelper.h>
#include <vscpremotetcpif.h>
#ifdef WIN32
#include "dlldrvobj.h"
#include "vscphelperdll.h"
#else
#include "linux/libvscphelper.h"
#endif

#ifdef _MSC_VER
   #include <wx/msw/msvcrt.h>
#endif

#ifdef WIN32
extern CHelpDllObj theApp;
#else
extern CVSCPLApp theApp;
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
extern "C"
long WINAPI EXPORT vscphlp_newSession( void )
#else
extern "C" long
EXPORT vscphlp_newSession( void )
#endif
{
    VscpRemoteTcpIf *pvscpif = new VscpRemoteTcpIf;
    if (NULL == pvscpif) return 0;

    return theApp.addDriverObject( pvscpif );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_closeSession
//

#ifdef WIN32
extern "C" void WINAPI EXPORT vscphlp_closeSession(long handle)
#else
extern "C" void vscphlp_closeSession(long handle)
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if (NULL != pvscpif) pvscpif->doCmdClose();

    theApp.removeDriverObject(handle);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_setResponseTimeout
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setResponseTimeout(long handle,
                                                    unsigned long timeout )
#else
extern "C" int vscphlp_setResponseTimeout(long handle,
                                            unsigned long timeout )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    pvscpif->setResponseTimeout( timeout );

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_setAfterCommandSleep
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setAfterCommandSleep( long handle,
                                                            unsigned short sleeptime )
#else
extern "C" int vscphlp_setAfterCommandSleep( long handle,
                                           unsigned short sleeptime )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    pvscpif->setAfterCommandSleep( sleeptime );

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_isConnected
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_isConnected(const long handle)
#else
extern "C" int vscphlp_isConnected(const long handle)
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    return pvscpif->isConnected() ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_doCommand
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_doCommand( long handle, const char * cmd )
#else
extern "C" int vscphlp_doCommand( long handle, const char * cmd )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString wxCmd = wxString::FromUTF8( cmd );
    return pvscpif->doCommand( wxCmd );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_checkReply
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_checkReply( long handle, int bClear )
#else
extern "C" int vscphlp_checkReply( long handle, int bClear )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->checkReturnValue( bClear ? true : false ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_clearLocalInputQueue
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_clearLocalInputQueue( long handle )
#else
extern "C" int vscphlp_clearLocalInputQueue( long handle )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    pvscpif->doClrInputQueue();
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_OpenInterface
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_openInterface( long handle,
                                                        const char *pInterface,
                                                        unsigned long flags )
#else
extern "C" int vscphlp_openInterface( long handle,
                                        const char *pInterface,
                                        unsigned long flags )
#endif
{
    wxString strInterface;
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    strInterface = wxString::FromUTF8( pInterface );
    return pvscpif->doCmdOpen( strInterface, flags );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_open
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_open(const long handle,
        const char *pHostname,
        const char *pUsername,
        const char *pPassword)
#else

extern "C" int vscphlp_open(const long handle,
        const char *pHostname,
        const char *pUsername,
        const char *pPassword)
#endif
{
    wxString strHostname;
    wxString strUsername;
    wxString strPassword;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    strHostname = wxString::FromUTF8(pHostname);
    strUsername = wxString::FromUTF8(pUsername);
    strPassword = wxString::FromUTF8(pPassword);

    return pvscpif->doCmdOpen(strHostname, strUsername, strPassword);
}



///////////////////////////////////////////////////////////////////////////////
// vscphlp_close
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_close( long handle )
#else
extern "C" int vscphlp_close( long handle )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdClose();
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_noop
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_noop( long handle )
#else
extern "C" int vscphlp_noop( long handle )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdNOOP();
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_clearDaemonEventQueue
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_clearDaemonEventQueue( long handle )
#else
extern "C" int vscphlp_clearDaemonEventQueue( long handle )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdClear();
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_sendEvent
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_sendEvent( long handle,
                                                    const vscpEvent *pEvent )
#else
extern "C" int vscphlp_sendEvent( long handle,
                                    const vscpEvent *pEvent )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdSend( pEvent );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_sendEventEx
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_sendEventEx( long handle,
                                                    const vscpEventEx *pEvent )
#else
extern "C" int vscphlp_sendEventEx( long handle,
                                        const vscpEventEx *pEvent )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdSendEx( pEvent );
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_receiveEvent
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_receiveEvent( long handle,
                                                    vscpEvent *pEvent )
#else
extern "C" int vscphlp_receiveEvent( long handle,
                                        vscpEvent *pEvent )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdReceive( pEvent );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_receiveEventEx
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_receiveEventEx( long handle,
                                                        vscpEventEx *pEvent )
#else
extern "C" int vscphlp_receiveEventEx( long handle,
                                        vscpEventEx *pEvent )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdReceiveEx( pEvent );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_blockingReceiveEvent
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_blockingReceiveEvent( long handle,
                                                            vscpEvent *pEvent )
#else
extern "C" int vscphlp_blockingReceiveEvent( long handle,
                                                vscpEvent *pEvent )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdBlockingReceive( pEvent );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_blockingReceiveEventEx
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_blockingReceiveEventEx( long handle,
                                                            vscpEventEx *pEvent )
#else
extern "C" int vscphlp_blockingReceiveEventEx( long handle,
                                            vscpEventEx *pEvent )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdBlockingReceive( pEvent );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_isDataAvailable
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_isDataAvailable( long handle, unsigned int *pCount )
#else
extern "C" int vscphlp_isDataAvailable( long handle, unsigned int *pCount )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    *pCount = pvscpif->doCmdDataAvailable();

    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_getStatus
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getStatus( long handle,
                                                        VSCPStatus *pStatus )
#else
extern "C" int vscphlp_getStatus( long handle,
                                        VSCPStatus *pStatus )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdStatus( pStatus );
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_getStatistics
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getStatistics( long handle,
                                                        VSCPStatistics *pStatistics )
#else
extern "C" int vscphlp_getStatistics( long handle,
                                        canalStatistics *pStatistics )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdStatistics( pStatistics );
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_setFilter
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setFilter( long handle,
                                                    const vscpEventFilter *pFilter )
#else
extern "C" int vscphlp_setFilter( long handle,
                                    const vscpEventFilter *pFilter )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdFilter( pFilter );
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_getVersion
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVersion( long handle,
                                                    unsigned char *pMajorVer,
                                                    unsigned char *pMinorVer,
                                                    unsigned char *pSubMinorVer )
#else
extern "C" int vscphlp_getVersion( long handle,
                                      unsigned char *pMajorVer,
                                      unsigned char *pMinorVer,
                                      unsigned char *pSubMinorVer )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdVersion( pMajorVer, pMinorVer, pSubMinorVer);
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getDLLVersion
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getDLLVersion( long handle, unsigned long *pVersion )
#else
extern "C" int vscphlp_getDLLVersion( long handle, unsigned long *pVersion )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    *pVersion = pvscpif->doCmdDLLVersion();

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getVendorString
//
#ifdef WIN32
extern "C"  int WINAPI EXPORT vscphlp_getVendorString( long handle, char *pVendorStr, int size )
#else
extern "C"  int vscphlp_getVendorString( long handle, char *pVendorStr, int size  )
#endif
{
    if ( NULL == pVendorStr ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString str =  wxString::FromAscii( pvscpif->doCmdVendorString() );
    strncpy( pVendorStr, str.mbc_str(), MIN( strlen( str.mbc_str() ),size) );

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_getDriverInfo
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getDriverInfo( long handle, char *pInfoStr, int size )
#else
extern "C" int vscphlp_getDriverInfo( long handle, char *pInfoStr, int size )
#endif
{
    if ( NULL == pInfoStr ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString str =  wxString::FromAscii( pvscpif->doCmdGetDriverInfo() );
    strncpy( pInfoStr, str.mbc_str(), MIN( strlen( str.mbc_str() ),size ) );

    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_shutDownServer
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_shutDownServer( long handle )
#else
extern "C" int vscphlp_shutDownServer( long handle )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdShutDown();
}


/*!
    This command sets an open interface in the receive loop (RCVLOOP).
    It does nothing other then putting the interface in the loop mode and
    checking that it went there.

    Note! The only way to terminate this receive loop is to close the session with
    'CLOSE' or sending 'QUITLOOP' .
    \param handle to server object
    \return CANAL_ERROR_SUCCESS if success CANAL_ERROR_GENERIC if not.
 */
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_enterReceiveLoop(const long handle)
#else
extern "C" int vscphlp_enterReceiveLoop(const long handle)
#endif
{
    // Get VSCP TCP/IP object
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->doCmdEnterReceiveLoop();
}

/*!
    Quit the receiveloop
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_quitReceiveLoop(const long handle)
#else
extern "C" int vscphlp_quitReceiveLoop(const long handle)
#endif
{
    // Get VSCP TCP/IP object
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject(handle);
    if (NULL == pvscpif) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

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
extern "C" int WINAPI EXPORT vscphlp_deleteRemoteVariable( long handle, const char *pName )
#else
extern "C" int vscphlp_deleteRemoteVariable( long handle, const char *pName )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->deleteRemoteVariable( name );
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
extern "C" int WINAPI EXPORT vscphlp_createRemoteVariable( const char *pName,
                                                        const char* pType,
                                                        const int bPersistent,
                                                        const char *pUser,
                                                        const unsigned long rights,
                                                        const char* pValue,
                                                        const char* pNote  )
#else
extern "C" int vscphlp_createRemoteVariable( long handle,
                                              const char *pName,
                                              const char* pType,
                                              const int bPersistent,
                                              const char *pUser,
                                              const unsigned long rights,
                                              const char* pValue,
                                              const char* pNote )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pType ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pUser ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pValue ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pNote ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    wxString type = wxString::FromAscii( pType );
    wxString user = wxString::FromAscii( pUser );
    wxString value = wxString::FromAscii( pValue );
    wxString note = wxString::FromAscii( pNote );
    
    return pvscpif->createRemoteVariable( name, 
                                            type,
                                            ( bPersistent ? true : false ),
                                            user,
                                            rights,
                                            value,
                                            note );
}


/*!
    \fn int vscphlp_saveVariablesToDisk( long handle, const char *pPath )
    \brief Save variables to disk
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_saveRemoteVariablesToDisk( long handle, 
                                                            const char *pPath,
                                                            const int select,
                                                            const char *pRegExp )
#else
extern "C" int vscphlp_saveRemoteVariablesToDisk( long handle, 
                                                    const char *pPath,
                                                    const int select,
                                                    const char *pRegExp )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString path = wxString::FromAscii( pPath );
    
    return pvscpif->saveRemoteVariablesToDisk( path );
}



/*!
    \fn bool vscphlp_getVariableString( const char *pName, char *pValue )
    \brief Get variable value from string variable
    \param name of variable
    \param pointer to string that get the value of the string variable.
    \param size fo buffer
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableString( long handle, const char *pName, char *pValue, int size )
#else
extern "C" int vscphlp_getRemoteVariableString( long handle, const char *pName, char *pValue, int size )
#endif
{
    int rv;

    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pValue ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    if ( VSCP_ERROR_SUCCESS == ( rv = pvscpif->getRemoteVariableValue( name, strValue ) ) ) {
        strncpy( pValue, strValue.mbc_str(), MIN( strlen( strValue.mbc_str() ), size ) );
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
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableString( long handle, const char *pName, char *pValue )
#else
extern "C" int vscphlp_setRemoteVariableString( long handle, const char *pName, char *pValue )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pValue ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if ( NULL == pName ) return false;
    if ( NULL == pValue ) return false;

    wxString name = wxString::FromAscii( pName );
    wxString strValue = wxString::FromAscii( pValue );

    return pvscpif->setRemoteVariableValue( name, strValue );
}

/*!
    \fn bool vscphlp_getVariableBool( const char *pName, bool *bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableBool( long handle, const char *pName, int *bValue )
#else
extern "C" int vscphlp_getRemoteVariableBool( long handle, const char *pName, int *bValue )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == bValue ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    bool bBoolValue;
    int rv =  pvscpif->getRemoteVariableBool( name, &bBoolValue );
    if ( bBoolValue ) {
        *bValue = 1;
    }
    else {
        *bValue = 0;
    }
    return rv;
};


/*!
    \fn bool vscphlp_setVariableBool( const char *pName, bool bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableBool( long handle, const char *pName, int bValue )
#else
extern "C" int vscphlp_setRemoteVariableBool( long handle, const char *pName, int bValue )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableBool( name, ( bValue ? true : false ) );
};


/*!
    \fn bool vscphlp_getVariableInt( const char *pName, int *value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableInt( long handle, const char *pName, int *value )
#else
extern "C" int vscphlp_getRemoteVariableInt( long handle, const char *pName, int *value )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == value ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getRemoteVariableInt( name, value );
};


/*!
    \fn bool vscphlp_setVariableInt( const char *pName, int value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableInt( long handle, const char *pName, int value )
#else
extern "C" int vscphlp_setRemoteVariableInt( long handle, const char *pName, int value )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableInt( name, value );
};

/*!
    \fn bool vscphlp_getVariableLong( const char *pName, long *value )
    \brief Get variable value from long variable
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableLong( long handle, const char *pName, long *value )
#else
extern "C" int vscphlp_getRemoteVariableLong( long handle, const char *pName, long *value )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == value ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getRemoteVariableLong( name, value );
};

/*!
    \fn bool vscphlp_setVariableLong( const char *pName, long value )
    \brief Get variable value from long variable
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableLong( long handle, const char *pName, long value )
#else
extern "C" int vscphlp_setRemoteVariableLong( long handle, const char *pName, long value )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableLong( name, value );
};

/*!
    \fn bool vscphlp_getVariableDouble( const char *pName, double *value )
    \brief Get variable value from double variable
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code..
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableDouble( long handle, const char *pName, double *value )
#else
extern "C" int vscphlp_getRemoteVariableDouble( long handle, const char *pName, double *value )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == value ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getRemoteVariableDouble( name, value );
};

/*!
    \fn bool vscphlp_setVariableDouble( const char *pName, double value )
    \brief Get variable value from double variable
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableDouble( long handle, const char *pName, double value )
#else
extern "C" int vscphlp_setRemoteVariableDouble( long handle, const char *pName, double value )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableDouble( name, value );
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
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableMeasurement( long handle, 
                                                                    const char *pName, 
                                                                    double *pvalue,
                                                                    int *punit,
                                                                    int *psensoridx,
                                                                    int *pzone,
                                                                    int *psubzone )
#else
extern "C" int vscphlp_getRemoteVariableMeasurement( long handle, 
                                                        const char *pName, 
                                                        double *pvalue,
                                                        int *punit,
                                                        int *psensoridx,
                                                        int *pzone,
                                                        int *psubzone )
#endif
{
    int rv;
    uint8_t unit,sensoridx,zone,subzone;

    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pvalue ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    
    if ( VSCP_ERROR_SUCCESS == 
            ( rv = pvscpif->getRemoteVariableMeasurement( name, 
                                                            pvalue,
                                                            &unit,
                                                            &sensoridx,
                                                            &zone,
                                                            &subzone ) ) ) {
        *punit = unit;
        *psensoridx = sensoridx;
        *pzone = zone;
        *psubzone = subzone;
    }

    return rv;
};

/*!
    \fn bool vscphlp_setVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param name of variable
    \param String that get that get the
    value of the measurement.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableMeasurement( long handle, 
                                                                    const char *pName, 
                                                                    double value,
                                                                    int unit,
                                                                    int sensoridx,
                                                                    int zone,
                                                                    int subzone )
#else
extern "C" int vscphlp_setRemoteVariableMeasurement( long handle, 
                                                        const char *pName, 
                                                        double value,
                                                        int unit,
                                                        int sensoridx,
                                                        int zone,
                                                        int subzone )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    
    return pvscpif->setRemoteVariableMeasurement( name, 
                                                    value,
                                                    (uint8_t)unit,
                                                    (uint8_t)sensoridx,
                                                    (uint8_t)zone,
                                                    (uint8_t)subzone );
};

/*!
    \fn bool vscphlp_getVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableEvent( long handle, const char *pName, vscpEvent *pEvent )
#else
extern "C" int vscphlp_getRemoteVariableEvent( long handle, const char *pName, vscpEvent *pEvent )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pEvent ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getRemoteVariableEvent( name, pEvent );
}

/*!
    \fn bool vscphlp_setVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableEvent( long handle, const char *pName, vscpEvent *pEvent )
#else
extern "C" int vscphlp_setRemoteVariableEvent( long handle, const char *pName, vscpEvent *pEvent )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pEvent ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableEvent( name, pEvent );
}

/*!
    \fn bool vscphlp_getVariableEventEx( const char *pName, vscpEventEx *pEvent )
    \brief Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent )
#else
extern "C" int vscphlp_getRemoteVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pEvent ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getRemoteVariableEventEx( name, pEvent );
}

/*!
    \fn bool vscphlp_setVariableEventEx( const char *pName, vscpEventEx *pEvent )
    \brief Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent )
#else
extern "C" int vscphlp_setRemoteVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pEvent ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableEventEx( name, pEvent );
}

/*!
    \fn bool vscphlp_getVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableGUIDString( long handle, const char *pName, char *pGUID, int size )
#else
extern "C" int vscphlp_getRemoteVariableGUIDString( long handle, const char *pName, char *pGUID, int size )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pGUID ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    cguid GUID;
    wxString strGUID;
    wxString name = wxString::FromAscii( pName );
    int rv =  pvscpif->getRemoteVariableGUID( name, GUID );
    GUID.toString( strGUID );
    strncpy( pGUID, strGUID.mbc_str(), MIN( strlen( strGUID.mbc_str() ), size ) );
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
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableGUIDString( long handle, const char *pName, const char *pGUID )
#else
extern "C" int vscphlp_setRemoteVariableGUIDString( long handle, const char *pName, const char *pGUID )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pGUID ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    cguid guid;
    guid.getFromString( pGUID );
    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableGUID( name, guid );
}

/*!
    \fn bool vscphlp_getVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer GUID array
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableGUIDArray( long handle, const char *pName, unsigned char *pGUID )
#else
extern "C" int vscphlp_getRemoteVariableGUIDArray( long handle, const char *pName, unsigned char *pGUID )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pGUID ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    cguid GUID;
    wxString name = wxString::FromAscii( pName );
    int rv =  pvscpif->getRemoteVariableGUID( name, GUID );
    memcpy( pGUID, GUID.getGUID(), 16 );
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
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableGUIDArray( long handle, const char *pName, const unsigned char *pGUID )
#else
extern "C" int vscphlp_setRemoteVariableGUIDArray( long handle, const char *pName, const unsigned char *pGUID )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pGUID ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    cguid guid;
    guid.getFromArray( pGUID );
    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableGUID( name, guid );
}

/*!
    \fn bool vscphlp_getVariableVSCPdata( const char *pName, uint16_t *psizeData, uint8_t *pData )
    \brief Get variable value from VSCP data variable
    \param name of variable
    \param Pointer to variable that will hold the size of the data array
    \param pointer to VSCP data array variable (unsigned char [8] ) that get the
    value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short *psize )
#else
extern "C" int vscphlp_getRemoteVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short *psize )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pData ) return VSCP_ERROR_PARAMETER;
    if ( NULL == psize ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getRemoteVariableVSCPdata( name, pData, psize );
}

/*!
    \fn bool vscphlp_setVariableVSCPdata( const char *pName, uint16_t sizeData, uint8_t *pData )
    \brief Get variable value from VSCP data variable
    \param name of variable
    \param Pointer to variable that will hold the size of the data array
    \param pointer to VSCP data array variable (unsigned char [8] ) that get the
    value of the string variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableVSCPData( long handle, const char *pName,  uint8_t *pData, unsigned short size )
#else
extern "C" int vscphlp_setRemoteVariableVSCPData( long handle, const char *pName, uint8_t *pData, unsigned short size )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == pData ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableVSCPdata( name, pData, size );
}

/*!
    \fn bool vscphlp_getVariableVSCPclass( const char *pName, uint16_t *vscphlp_class )
    \brief Get variable value from class variable
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableVSCPClass( long handle, const char *pName, unsigned short *vscp_class )
#else
extern "C" int vscphlp_getRemoteVariableVSCPClass( long handle, const char *pName, unsigned short *vscp_class )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == vscp_class ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getRemoteVariableVSCPclass( name, vscp_class );
}

/*!
    \fn bool vscphlp_setVariableVSCPclass( const char *pName, uint16_t vscp_class )
    \brief Get variable value from class variable
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableVSCPClass( long handle, const char *pName, unsigned short vscp_class )
#else
extern "C" int vscphlp_setRemoteVariableVSCPClass( long handle, const char *pName, unsigned short vscp_class )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableVSCPclass( name, vscp_class );
}

/*!
    \fn bool vscphlp_getVariableVSCPtype( const char *pName, uint8_t *vscp_type )
    \brief Get variable value from type variable
    \param name of variable
    \param pointer to int that get the value of the type variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getRemoteVariableVSCPType( long handle, const char *pName, unsigned short *vscp_type )
#else
extern "C" int vscphlp_getRemoteVariableVSCPType( long handle, const char *pName, unsigned short *vscp_type )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;
    if ( NULL == vscp_type ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getRemoteVariableVSCPtype( name, vscp_type );
}

/*!
    \fn bool WINAPI EXPORT vscphlp_setVariableVSCPtype( const char *pName, uint8_t vscp_type )
    \brief Get variable value from type variable
    \param name of variable
    \param pointer to int that get the value of the type variable.
    \return Returns VSCP_ERROR_SUCCESS on success, otherwise error code.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setRemoteVariableVSCPType( long handle, const char *pName, unsigned short vscp_type )
#else
extern "C" int vscphlp_setRemoteVariableVSCPType( long handle, const char *pName, unsigned short vscp_type )
#endif
{
    if ( NULL == pName ) return VSCP_ERROR_PARAMETER;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setRemoteVariableVSCPtype( name, vscp_type );
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
extern "C" unsigned long WINAPI EXPORT vscphlp_readStringValue( const char * pStrValue )
#else
extern "C" unsigned long vscphlp_readStringValue( const char * pStrValue )
#endif
{
    wxString strVal;
    strVal = wxString::FromAscii( pStrValue );

    return vscp_readStringValue( strVal );
}


#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_replaceBackslash( char *pStr )
#else
extern "C" int vscphlp_replaceBackslash( char *pStr )
#endif
{
    if ( NULL == pStr ) return VSCP_ERROR_ERROR;

    wxString wxstr = wxString::FromAscii( pStr );
    wxstr = vscp_replaceBackslash( wxstr );
    strcpy(pStr, wxstr.mbc_str() );
    return VSCP_ERROR_SUCCESS;
}


// * * * * *  E V E N T - H E L P E R S * * * * *


/*!
    \fn bool vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx,
                                    const vscpEvent *pEvent )
    \brief Convert VSCP standard event form to ex. form.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx,
                                                        const vscpEvent *pEvent )
#else
extern "C" int vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx,
                                                        const vscpEvent *pEvent )
#endif
{
    return vscp_convertVSCPtoEx( pEventEx, pEvent ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_convertVSCPfromEx( vscpEvent *pEvent,
                                        const vscpEventEx *pEventEx )
    \brief Convert VSCP ex. event form to standard form.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertVSCPfromEx( vscpEvent *pEvent,
                                                        const vscpEventEx *pEventEx )
#else
extern "C" int vscphlp_convertVSCPfromEx( vscpEvent *pEvent,
                                                        const vscpEventEx *pEventEx )
#endif
{
    return vscp_convertVSCPfromEx( pEvent, pEventEx ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


/*!
    \fn void vscphlp_deleteVSCPevent( vscpEvent *pEvent )
    \brief Delete VSCP event.
*/
#ifdef WIN32
extern "C" void WINAPI EXPORT vscphlp_deleteVSCPevent( vscpEvent *pEvent )
#else
extern "C" void vscphlp_deleteVSCPevent( vscpEvent *pEvent )
#endif
{
    return vscp_deleteVSCPevent( pEvent );
}

/*!
    \fn void vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx )
    \brief Delete VSCP event ex.
*/
#ifdef WIN32
extern "C" void WINAPI EXPORT vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx )
#else
extern "C" void vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx )
#endif
{
    return vscp_deleteVSCPeventEx( pEventEx );
}


/*!
    \fn unsigned char vscphlp_getVscpPriority( const vscpEvent *pEvent )
    \brief Get VSCP priority.
*/
#ifdef WIN32
extern "C" unsigned char WINAPI EXPORT vscphlp_getVscpPriority( const vscpEvent *pEvent )
#else
extern "C" unsigned char vscphlp_getVscpPriority( const vscpEvent *pEvent )
#endif
{
    return vscp_getVscpPriority( pEvent );
}

#ifdef WIN32
extern "C" unsigned char WINAPI EXPORT vscphlp_getVscpPriorityEx( const vscpEventEx *pEvent )
#else
extern "C" unsigned char vscphlp_getVscpPriorityEx( const vscpEventEx *pEvent )
#endif
{
    return vscp_getVscpPriorityEx( pEvent );
}


/*!
    \fn void vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority )
    \brief Set VSCP priority.
*/
#ifdef WIN32
extern "C" void WINAPI EXPORT vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority )
#else
extern "C" void vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority )
#endif
{
    vscp_setVscpPriority( pEvent, priority );
}

#ifdef WIN32
extern "C" void WINAPI EXPORT vscphlp_setVscpPriorityEx( vscpEventEx *pEvent, unsigned char priority )
#else
extern "C" void vscphlp_setVscpPriorityEx( vscpEventEx *pEvent, unsigned char priority )
#endif
{
    vscp_setVscpPriorityEx( pEvent, priority );
}




/*!
    \fn vscphlp_getVSCPheadFromCANALid( const unsigned long id )
    \brief Get the VSCP head from a CANAL message id (CANAL id).
*/
#ifdef WIN32
extern "C" unsigned char WINAPI EXPORT vscphlp_getVSCPheadFromCANALid( const unsigned long id )
#else
extern "C" unsigned char vscphlp_getVSCPheadFromCANALid( const unsigned long id )
#endif
{
    return  vscp_getVSCPheadFromCANALid( id );
}

/*!
    \fn vscphlp_getVSCPclassFromCANALid( const unsigned long id )
    \brief Get the VSCP class from a CANAL message id (CANAL id).
*/
#ifdef WIN32
extern "C" unsigned short WINAPI EXPORT vscphlp_getVSCPclassFromCANALid( const unsigned long id )
#else
extern "C" unsigned short vscphlp_getVSCPclassFromCANALid( const unsigned long id )
#endif
{
    return vscp_getVSCPclassFromCANALid( id );
}


/*!
    \fn unsigned short vscphlp_getVSCPtypeFromCANALid( const unsigned long id )
    \brief Get the VSCP type from a a CANAL message id (CANAL id).
*/
#ifdef WIN32
extern "C" unsigned short WINAPI EXPORT vscphlp_getVSCPtypeFromCANALid( const unsigned long id )
#else
extern "C" unsigned short vscphlp_getVSCPtypeFromCANALid( const unsigned long id )
#endif
{
    return vscp_getVSCPtypeFromCANALid( id );
}

/*!
    \fn unsigned short vscphlp_getVSCPnicknameFromCANALid( const unsigned long id )
    \brief Get the VSCP nickname from a a CANAL message id (CANAL id).
*/
#ifdef WIN32
extern "C" unsigned char WINAPI EXPORT vscphlp_getVSCPnicknameFromCANALid( const unsigned long id )
#else
extern "C" unsigned char vscphlp_getVSCPnicknameFromCANALid( const unsigned long id )
#endif
{
    return vscp_getVSCPnicknameFromCANALid( id );
}

/*!
    \fn unsigned long vscphlp_getCANALidFromVSCPdata( const unsigned char priority,
                                                    const unsigned short vscp_class,
                                                    const unsigned short vscp_type )
    \brief Construct a CANAL id (CANAL id ) from VSCP.
*/
#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT vscphlp_getCANALidFromVSCPdata( const unsigned char priority,
                                                                        const unsigned short vscp_class,
                                                                        const unsigned short vscp_type )
#else
extern "C" unsigned long vscphlp_getCANALidFromVSCPdata( const unsigned char priority,
                                                                    const unsigned short vscp_class,
                                                                    const unsigned short vscp_type )
#endif
{
    return  vscp_getCANALidFromVSCPdata( priority, vscp_class, vscp_type );
}

/*!
    \fn unsigned long vscphlp_getCANALidFromVSCPevent( const vscpEvent *pEvent )
    \brief Get CANAL id (CAN id) from VSCP event.
*/
#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT vscphlp_getCANALidFromVSCPevent( const vscpEvent *pEvent )
#else
extern "C" unsigned long vscphlp_getCANALidFromVSCPevent( const vscpEvent *pEvent )
#endif
{
    return vscp_getCANALidFromVSCPevent( pEvent );
}


/*!
    \fn unsigned long vscphlp_getCANALidFromVSCPevent( const vscpEvent *pEvent )
    \brief Get CANAL id (CAN id) from VSCP event.
*/
#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT vscphlp_getCANALidFromVSCPeventEx( const vscpEventEx *pEvent )
#else
extern "C" unsigned long vscphlp_getCANALidFromVSCPeventEx( const vscpEventEx *pEvent )
#endif
{
    return vscp_getCANALidFromVSCPeventEx( pEvent );
}

/*!
    \fn unsigned short vscphlp_calc_crc_Event( vscpEvent *pEvent, short bSet )
    \brief Calculate VSCP crc.
*/
#ifdef WIN32
extern "C" unsigned short WINAPI EXPORT vscphlp_calc_crc_Event( vscpEvent *pEvent, short bSet )
#else
extern "C" unsigned short vscphlp_calc_crc_Event( vscpEvent *pEvent, short bSet )
#endif
{
    return vscp_calc_crc_Event( pEvent, bSet );
}


/*!
    \fn unsigned short vscphlp_calc_crc_EventEx( vscpEvent *pEvent, short bSet )
    \brief Calculate VSCP crc.
*/
#ifdef WIN32
extern "C" unsigned short WINAPI EXPORT vscphlp_calc_crc_EventEx( vscpEventEx *pEvent, short bSet )
#else
extern "C" unsigned short vscphlp_calc_crc_EventEx( vscpEventEx *pEvent, short bSet )
#endif
{
    return vscp_calc_crc_EventEx( pEvent, bSet );
}



// * * * * *   G U I D  H E L P E R S    * * * * *



/*!
        calcCRC4GUIDArray

        \param Pointer to GUID array (MSB-LSB order)
        \return crc for GUID.
    */
#ifdef WIN32
extern "C" unsigned char WINAPI EXPORT vscphlp_calcCRC4GUIDArray( unsigned char *pguid )
#else
extern "C" unsigned char vscphlp_calcCRC4GUIDArray( unsigned char *pguid)
#endif
{
    return vscp_calcCRC4GUIDArray( pguid );
}


/*!
  calcCRC4GUIDString

  \param Pointer to GUID array (MSB-LSB order)
  \return crc for GUID.
*/
#ifdef WIN32
extern "C" unsigned char WINAPI EXPORT vscphlp_calcCRC4GUIDString(const char *strguid)
#else
extern "C" unsigned char vscphlp_calcCRC4GUIDString(const char *strguid)
#endif
{
    wxString wxstr = wxString::FromAscii(strguid);
    return vscp_calcCRC4GUIDString(wxstr);
}


/*!
    \fn bool vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
    \brief Write GUID into VSCP event from string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
#else
extern "C" int vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
#endif
{
    wxString strGUID = wxString::FromAscii( pGUID );
    return  vscp_getGuidFromString( pEvent, strGUID ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_getGuidFromStringEx( vscpEventEx *pEvent, const char * pGUID )
    \brief Write GUID into VSCP event from string.
*/

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getGuidFromStringEx( vscpEventEx *pEvent, const char * pGUID )
#else
extern "C" int vscphlp_getGuidFromStringEx( vscpEventEx *pEvent, const char * pGUID )
#endif
{
    wxString strGUID = wxString::FromAscii( pGUID );
    return  vscp_getGuidFromStringEx( pEvent, strGUID ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
    \brief Write GUID from string into array.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
#else
extern "C" int vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
#endif
{
    wxString strGUID = wxString::FromAscii( pStr );
    return vscp_getGuidFromStringToArray( pGUID, strGUID ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_writeGuidToString( const vscpEvent *pEvent, char *pStr, int size )
    \brief Write GUID froom VSCP event to string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_writeGuidToString( const vscpEvent *pEvent, char * pStr, int size )
#else
extern "C" int vscphlp_writeGuidToString( const vscpEvent *pEvent, char *pStr, int size )
#endif
{
    bool rv;

    wxString strGUID;
    rv = vscp_writeGuidToString( pEvent, strGUID );
    strncpy( pStr, strGUID.mbc_str(), MIN( strlen( strGUID.mbc_str() ), size ) );
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_writeGuidToStringEx( const vscpEventEx *pEvent, char * pStr )
    \brief Write GUID froom VSCP event to string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_writeGuidToStringEx( const vscpEventEx *pEvent, char *pStr, int size )
#else
extern "C" int vscphlp_writeGuidToStringEx( const vscpEventEx *pEvent, char * pStr, int size )
#endif
{
    bool rv;

    wxString strGUID;
    rv = vscp_writeGuidToStringEx( pEvent, strGUID );
    strncpy( pStr, strGUID.mbc_str(), MIN( strlen( strGUID.mbc_str() ), size ) );
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


/*!
    \fn bool vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent,
                                            wxString& strGUID )
    \brief Write GUID from VSCP event to string with four bytes on each
    row seperated by \r\n.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent,
                                                                char *strGUID,
                                                                int size )
#else
extern "C" int vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent,
                                                                char *strGUID,
                                                                int size )
#endif
{
    wxString str = wxString::FromAscii( strGUID );
    return vscp_writeGuidToString4Rows( pEvent, str ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


/*!
    \fn bool vscphlp_writeGuidToString4RowsEx( const vscpEvent *pEvent,
                                            wxString& strGUID )
    \brief Write GUID from VSCP event to string with four bytes on each
    row seperated by \r\n.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_writeGuidToString4RowsEx( const vscpEventEx *pEvent,
                                                                char *strGUID,
                                                                int size )
#else
extern "C" int vscphlp_writeGuidToString4RowsEx( const vscpEventEx *pEvent,
                                                                char *strGUID,
                                                                int size )
#endif
{
    wxString str = wxString::FromAscii( strGUID );
    return vscp_writeGuidToString4RowsEx( pEvent, str ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool vscphlp_writeGuidArrayToString( const unsigned char * pGUID,
                                            wxString& strGUID,
                                            int size )
    \brief Write GUID from byte array to string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_writeGuidArrayToString( const unsigned char *pGUID,
                                                                char *strGUID,
                                                                int size )
#else
extern "C" int vscphlp_writeGuidArrayToString( const unsigned char * pGUID,
                                                                char *strGUID,
                                                                int size )
#endif
{
    wxString str;
    bool rv = vscp_writeGuidArrayToString( pGUID, str );
    strncpy( strGUID, str.mbc_str(), MIN( strlen( str.mbc_str() ), size ) );

    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}



/*!
    \fn bool vscphlp_isGUIDEmpty( unsigned char *pGUID )
    \brief Check if GUID is empty (all nills).
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_isGUIDEmpty( unsigned char *pGUID )
#else
extern "C" int vscphlp_isGUIDEmpty( unsigned char *pGUID )
#endif
{
    return vscp_isGUIDEmpty( pGUID ) ? 1 : 0;
}

/*!
    \fn bool vscphlp_isSameGUID( const unsigned char *pGUID1,
                                const unsigned char *pGUID2 )
    \brief Check if two GUID's is equal to each other.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_isSameGUID( const unsigned char *pGUID1,
                                                const unsigned char *pGUID2 )
#else
extern "C" int vscphlp_isSameGUID( const unsigned char *pGUID1,
                                                const unsigned char *pGUID2 )
#endif
{
    return vscp_isSameGUID( pGUID1, pGUID2 ) ? 1 : 0;
}


/*!
    \fn bool vscphlp_reverseGUID(unsigned char *pGUID)
    \brief Reverse a GUID
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_reverseGUID(unsigned char *pGUID)
#else
extern "C" int vscphlp_reverseGUID(unsigned char *pGUID)
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
extern "C" void WINAPI EXPORT vscphlp_clearVSCPFilter( vscpEventFilter *pFilter )
#else
extern "C" void vscphlp_clearVSCPFilter( vscpEventFilter *pFilter )
#endif
{
    return vscp_clearVSCPFilter( pFilter );
}


/*!
    \fn void vscphlp_clearVSCPFilter( vscpEventFilter *pFilter )
    \brief Clear VSCP filter.
*/
#ifdef WIN32
extern "C" void WINAPI EXPORT vscphlp_copyVSCPFilter( vscpEventFilter *pToFilter, const vscpEventFilter *pFromFilter )
#else
extern "C" void vscphlp_copyVSCPFilter( vscpEventFilter *pToFilter, const vscpEventFilter *pFromFilter )
#endif
{
    return vscp_copyVSCPFilter( pToFilter, pFromFilter );
}

/*!
    \fn bool readFilterFromString( vscpEventFilter *pFilter, wxString& strFilter )
    \brief Read a filter from a string
    \param pFilter Filter structure to write filter to.
    \param strFilter Filter in string form
            filter-priority, filter-class, filter-type, filter-GUID
    \return true on success, fals eon failure.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_readFilterFromString( vscpEventFilter *pFilter, const char *strFilter )
#else
extern "C" int vscphlp_readFilterFromString( vscpEventFilter *pFilter, const char * strFilter )
#endif
{
    wxString str = wxString::FromAscii( strFilter );
    return vscp_readFilterFromString( pFilter, str ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool readMaskFromString( vscpEventFilter *pFilter, wxString& strMask )
    \brief Read a mask from a string
    \param pFilter Filter structure to write mask to.
    \param strMask Mask in string form
            mask-priority, mask-class, mask-type, mask-GUID
    \return true on success, fals eon failure.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_readMaskFromString( vscpEventFilter *pFilter, const char *strMask )
#else
extern "C" int vscphlp_readMaskFromString( vscpEventFilter *pFilter, const char *strMask )
#endif
{
    wxString str = wxString::FromAscii( strMask );
    return vscp_readMaskFromString( pFilter, str ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

/*!
    \fn bool writeFilterToString( vscpEventFilter *pFilter, wxString& strFilter )
    \brief Write a filter to a string
    \param pFilter Filter structure to write filter to.
    \param strFilter Will get filter in string form
            filter-priority, filter-class, filter-type, filter-GUID
    \return true on success, fals eon failure.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_writeFilterToString( vscpEventFilter *pFilter, char *strFilter )
#else
extern "C" int vscphlp_writeFilterToString( vscpEventFilter *pFilter, char *strFilter )
#endif
{
    int rv;
    wxString wxstr;

    rv = vscp_writeFilterToString( pFilter, wxstr ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
    if ( VSCP_ERROR_SUCCESS == rv ) memcpy( strFilter, (const char *)wxstr.mbc_str(), strlen( wxstr.mbc_str() ) );

    return rv;
}

/*!
    \fn bool writeMaskToString( vscpEventFilter *pFilter, wxString& strMask )
    \brief Write a mask to a string
    \param pFilter Filter structure to write mask to.
    \param strMask Will get mask in string form
            mask-priority, mask-class, mask-type, mask-GUID
    \return true on success, fals eon failure.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_writeMaskToString( vscpEventFilter *pFilter, char *strMask )
#else
extern "C" int vscphlp_writeMaskToString( vscpEventFilter *pFilter, char *strMask )
#endif
{
    int rv;
    wxString wxstr;

    rv = vscp_writeMaskToString( pFilter, wxstr ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
    if ( VSCP_ERROR_SUCCESS == rv ) memcpy( strMask, (const char *)wxstr.mbc_str(), strlen( wxstr.mbc_str() ) );

    return rv;
}

/*!
    \fn bool vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                    const vscpEventFilter *pFilter )
    \brief Check VSCP filter condition.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                                    const vscpEventFilter *pFilter )
#else
extern "C" int vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                                    const vscpEventFilter *pFilter )
#endif
{
    return  vscp_doLevel2Filter( pEvent, pFilter ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


/*!
    \fn int vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                            const canalMsg *pcanalMsg,
                                            unsigned char *pGUID,
                                            bool bCAN )
    \brief Convert CANAL message to VSCP event.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                                            const canalMsg *pcanalMsg,
                                                            unsigned char *pGUID )
#else
extern "C" int vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                                            const canalMsg *pcanalMsg,
                                                            unsigned char *pGUID )
#endif
{
    return vscp_convertCanalToEvent( pvscpEvent,
                                        pcanalMsg,
                                        pGUID ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


/*!
    \fn int vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                            const canalMsg *pcanalMsg,
                                            unsigned char *pGUID,
                                            bool bCAN )
    \brief Convert CANAL message to VSCP event.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertCanalToEventEx( vscpEventEx *pvscpEvent,
                                                               const canalMsg *pcanalMsg,
                                                               unsigned char *pGUID )
#else
extern "C" int vscphlp_convertCanalToEventEx( vscpEventEx *pvscpEvent,
                                                               const canalMsg *pcanalMsg,
                                                               unsigned char *pGUID )
#endif
{
    return vscp_convertCanalToEventEx( pvscpEvent,
                                          pcanalMsg,
                                          pGUID ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}






/*!
    \fn int vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                           const vscpEvent *pvscpEvent )
    \brief Convert VSCP event to CANAL message.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                                            const vscpEvent *pvscpEvent )
#else
extern "C" int vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                                            const vscpEvent *pvscpEvent )
#endif
{
    return  vscp_convertEventToCanal( pcanalMsg, pvscpEvent ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}






/*!
    \fn int vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                            const vscpEventEx *pvscpEvent )
    \brief Convert VSCP event to CANAL message.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                                            const vscpEventEx *pvscpEvent )
#else
extern "C" int vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                                            const vscpEventEx *pvscpEvent )
#endif
{
    return  vscp_convertEventExToCanal( pcanalMsg, pvscpEvent ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}






/*!
    \fn unsigned long vscphlp_makeTimeStamp( void )
    \brief Get VSCP timestamp.
*/
#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT vscphlp_makeTimeStamp( void )
#else
extern "C" unsigned long vscphlp_makeTimeStamp( void )
#endif
{
    return vscp_makeTimeStamp();
}






/*!
    \fn int vscphlp_copyVSCPEvent( vscpEvent *pEventTo,
                                    const vscpEvent *pEventFrom )
    \brief Copy VSCP event.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_copyVSCPEvent( vscpEvent *pEventTo,
                                                    const vscpEvent *pEventFrom )
#else
extern "C" int vscphlp_copyVSCPEvent( vscpEvent *pEventTo,
                                                    const vscpEvent *pEventFrom )
#endif
{
    return vscp_copyVSCPEvent( pEventTo, pEventFrom ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}






/*!
    \fn int vscphlp_writeVscpDataToString( const vscpEvent *pEvent,
                                            wxString& str,
                                            bool bUseHtmlBreak )
    \brief Write VSCP data in readable form to a (multiline) string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_writeVscpDataToString( const vscpEvent *pEvent,
                                                               char *pstr,
                                                               int size,
                                                               int bUseHtmlBreak )
#else
extern "C" int vscphlp_writeVscpDataToString( const vscpEvent *pEvent,
                                                                  char *pstr,
                                                                  int size,
                                                                  int bUseHtmlBreak )
#endif
{
    wxString wxstr;
    bool rv = vscp_writeVscpDataToString( pEvent,
                                         wxstr,
                                         bUseHtmlBreak ? true : false );
    strncpy( pstr, wxstr.mbc_str(), MIN( strlen( wxstr.mbc_str() ), size ) );
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}





/*!
    \fn int vscphlp_getVscpDataFromString( vscpEvent *pEvent,
                                            const wxString& str )
    \brief Set data in VSCP event from a string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVscpDataFromString( vscpEvent *pEvent,
                                                                const char *pstr )
#else
extern "C" int vscphlp_setVscpDataFromString( vscpEvent *pEvent,
                                                   const char *pstr )
#endif
{
    wxString wxstr = wxString::FromAscii( pstr );
    return vscp_setVscpDataFromString( pEvent, wxstr ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}






/*!
    \fn int vscphlp_getVscpDataAttayFromString( vscpEvent *pEvent,
                                                     unsigned short *psizeData,
                                                     const char *pstr )
    \brief Set data in VSCP event from a string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVscpDataArrayFromString( unsigned char *pData,
                                                                    unsigned short *psizeData,
                                                                    const char *pstr )
#else
extern "C" int vscphlp_setVscpDataArrayFromString( unsigned char *pData,
                                                      unsigned short *psizeData,
                                                      const char *pstr )
#endif
{
    wxString wxstr = wxString::FromAscii( pstr );
    return vscp_setVscpDataArrayFromString( pData, psizeData, wxstr ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


/*!
    \fn int vscphlp_writeVscpEventToString( vscpEvent *pEvent,
                                            char *p )
    \brief Write VSCP data to a string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_writeVscpEventToString( vscpEvent *pEvent, char *p, int size )
#else
extern "C" int vscphlp_writeVscpEventToString( vscpEvent *pEvent, char *p, int size )
#endif
{
    bool rv;

    wxString str;;
    if ( ( rv =  vscp_writeVscpEventToString( pEvent, str ) ) ) {
        strncpy( p, str.mbc_str(), MIN( strlen( str.mbc_str() ), size ) );
    }
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


/*!
    \fn int vscphlp_writeVscpEventExToString( vscpEventEx *pEvent,
                                                char *p )
    \brief Write VSCP data to a string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_writeVscpEventExToString( vscpEventEx *pEvent, char *p, int size )
#else
extern "C" int vscphlp_writeVscpEventExToString( vscpEventEx *pEvent,
                                                  char *p, int size )
#endif
{
    bool rv;

    wxString str;
    if ( ( rv =  vscp_writeVscpEventExToString( pEvent, str ) ) ) {
        strncpy( p, str.mbc_str(), MIN( strlen( str.mbc_str() ), size ) );
    }
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


/*!
    \fn int vscphlp_getVscpEventFromString( vscpEvent *pEvent,
                                            const char *p )
    \brief Get VSCP event from string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVscpEventFromString( vscpEvent *pEvent,
                                                                const char *p )
#else
extern "C" int vscphlp_setVscpEventFromString( vscpEvent *pEvent, const char *p )
#endif
{
    wxString str = wxString::FromAscii( p );
    return vscp_setVscpEventFromString( pEvent, str ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


/*!
    \fn int vscphlp_setVscpEventExFromString( vscpEventEx *pEvent,
                                            const char *p )
    \brief Get VSCP event from string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVscpEventExFromString( vscpEventEx *pEvent,
                                                                  const char *p )
#else
extern "C" int vscphlp_setVscpEventExFromString( vscpEventEx *pEvent,
                                                                const char *p )
#endif
{
    wxString str = wxString::FromAscii( p );
    return vscp_setVscpEventExFromString( pEvent, str ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


// * * * * *    D A T A C O D I N G   * * * * *


///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" unsigned char WINAPI EXPORT vscphlp_getMeasurementDataCoding( const vscpEvent *pEvent )
#else
extern "C" unsigned char vscphlp_getMeasurementDataCoding( const vscpEvent *pEvent )
#endif
{
    return vscp_getMeasurementDataCoding( pEvent );
}


///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" unsigned long long WINAPI EXPORT vscphlp_getDataCodingBitArray(const unsigned char *pCode, int size )
#else
extern "C" unsigned long long vscphlp_getDataCodingBitArray(const unsigned char *pCode, int size )
#endif
{
    return vscp_getDataCodingBitArray(pCode, size );
}


///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" unsigned long long WINAPI EXPORT vscphlp_getDataCodingInteger(const unsigned char *pCode,
                                                                            int size )
#else
extern "C" unsigned long long vscphlp_getDataCodingInteger(const unsigned char *pCode,
                                                              int size )
#endif
{
    return vscp_getDataCodingInteger(pCode, size );
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" double WINAPI EXPORT vscphlp_getDataCodingNormalizedInteger(const unsigned char *pCode,
                                                                                     int size )
#else
extern "C" double vscphlp_getDataCodingNormalizedInteger(const unsigned char *pCode,
                                                                       int size )
#endif
{
    return vscp_getDataCodingNormalizedInteger(pCode, size );
}



///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getDataCodingString(const unsigned char *pCode,
                                                              unsigned char dataLength,
                                                              char *strResult,
                                                              int size )
#else
extern "C" int vscphlp_getDataCodingString(const unsigned char *pCode,
                                                              unsigned char dataLength,
                                                              char *strResult,
                                                              int size )
#endif
{
    wxString wxstr;

    if ( NULL == pCode ) return VSCP_ERROR_ERROR;
    if ( NULL == strResult ) return VSCP_ERROR_ERROR;

    bool rv =  vscp_getDataCodingString( pCode,
                                           dataLength,
                                           wxstr );
    strncpy( strResult, wxstr.mbc_str(), MIN( strlen( wxstr.mbc_str() ), size ) );
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVSCPMeasurementAsString( const vscpEvent *pEvent,
                                                                    char *pResult,
                                                                    int size )
#else
extern "C" int vscphlp_getVSCPMeasurementAsString(const vscpEvent *pEvent,
                                                                    char *pResult,
                                                                    int size)
#endif
{
    wxString wxstr;

    if ( NULL == pEvent ) return VSCP_ERROR_ERROR;

    bool rv =  vscp_getVSCPMeasurementAsString( pEvent, wxstr );
    strncpy( pResult, wxstr.mbc_str(), MIN( strlen( wxstr.mbc_str() ), size ) );
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}



///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent, double *pvalue)
#else
extern "C" int vscphlp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent, double *pvalue)
#endif
{
    wxString wxstr;

    if ( NULL == pEvent ) return VSCP_ERROR_ERROR;

    return  vscp_getVSCPMeasurementAsDouble(pEvent, pvalue) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent,
                                                                            char *pStrResult,
                                                                            int size )
#else
extern "C" int vscphlp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent,
                                                                            char *pStrResult,
                                                                            int size )
#endif
{
    wxString wxstr;

    if ( NULL == pEvent ) return VSCP_ERROR_ERROR;

    bool rv = vscp_getVSCPMeasurementFloat64AsString( pEvent, wxstr );
    strncpy( pStrResult, wxstr.mbc_str(), MIN( strlen( wxstr.mbc_str() ), size ) );
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertFloatToNormalizedEventData( unsigned char *pdata,
                                                                            unsigned short *psize,
                                                                            double value,
                                                                            unsigned char unit,
                                                                            unsigned char sensoridx )
#else
extern "C" int vscphlp_convertFloatToNormalizedEventData( unsigned char *pdata,
                                                              unsigned short *psize,
                                                              double value,
                                                              unsigned char unit,
                                                              unsigned char sensoridx )
#endif
{
    wxString wxstr;

    if ( NULL == pdata ) return VSCP_ERROR_ERROR;

    bool rv = vscp_convertFloatToNormalizedEventData( pdata,
                                                        psize,
                                                        value,
                                                        unit,
                                                        sensoridx );
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
//
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertFloatToFloatEventData( unsigned char *pdata,
                                                                        unsigned short *psize,
                                                                        float value,
                                                                        unsigned char unit,
                                                                        unsigned char sensoridx )
#else
extern "C" int vscphlp_convertFloatToFloatEventData( unsigned char *pdata,
                                                                        unsigned short *psize,
                                                                        float value,
                                                                        unsigned char unit,
                                                                        unsigned char sensoridx )
#endif
{
    wxString wxstr;

    if ( NULL == pdata ) return VSCP_ERROR_ERROR;

    bool rv = vscp_convertFloatToFloatEventData( pdata,
                                                    psize,
                                                    value,
                                                    unit,
                                                    sensoridx );
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertIntegerToNormalizedEventData
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertIntegerToNormalizedEventData( unsigned char *pdata,
                                                                            unsigned short *psize,
                                                                            unsigned long long val64,
                                                                            unsigned char unit,
                                                                            unsigned char sensoridx )
#else
extern "C" int vscphlp_convertIntegerToNormalizedEventData( unsigned char *pdata,
                                                              unsigned short *psize,
                                                              unsigned long long val64,
                                                              unsigned char unit,
                                                              unsigned char sensoridx )
#endif
{
    wxString wxstr;

    if ( NULL == pdata ) return VSCP_ERROR_ERROR;
    if ( NULL == psize ) return VSCP_ERROR_ERROR;

    bool rv = vscp_convertIntegerToNormalizedEventData( pdata,
                                                            psize,
                                                            val64,
                                                            unit,
                                                            sensoridx );
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}



///////////////////////////////////////////////////////////////////////////////
// vscphlp_makeFloatMeasurementEvent
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_makeFloatMeasurementEvent( vscpEvent *pEvent,
                                                                    float value,
                                                                    unsigned char unit,
                                                                    unsigned char sensoridx )
#else
extern "C" int vscphlp_makeFloatMeasurementEvent( vscpEvent *pEvent,
                                                      float value,
                                                      unsigned char unit,
                                                      unsigned char sensoridx )
#endif
{
    wxString wxstr;

    if ( NULL == pEvent ) return VSCP_ERROR_ERROR;

    bool rv = vscp_makeFloatMeasurementEvent( pEvent,
                                                 value,
                                                 unit,
                                                 sensoridx );
    return rv ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_getMeasurementAsFloat
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getMeasurementAsFloat(const unsigned char *pNorm,
                                                                unsigned char length,
                                                                float *pResult )
#else
extern "C" int vscphlp_getMeasurementAsFloat(const unsigned char *pNorm,
                                                                unsigned char length,
                                                                float *pResult )
#endif
{
    wxString wxstr;

    if ( NULL == pNorm ) return VSCP_ERROR_ERROR;
    if ( NULL == pResult ) return VSCP_ERROR_ERROR;

    *pResult = vscp_getMeasurementAsFloat(pNorm, length );
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_getMeasurementUnit
//

#ifdef WIN32
extern "C" int WINAPI EXPORT  vscphlp_getMeasurementUnit( const vscpEvent *pEvent )
#else
extern "C" int vscphlp_getMeasurementUnit( const vscpEvent *pEvent )
#endif
{
    return vscp_getVSCPMeasurementUnit( pEvent );
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getMeasurementSensorIndex
//

#ifdef WIN32
extern "C" int WINAPI EXPORT  vscphlp_getMeasurementSensorIndex( const vscpEvent *pEvent )
#else
extern "C" int vscphlp_getMeasurementSensorIndex( const vscpEvent *pEvent )
#endif
{
    return vscp_getVSCPMeasurementSensorIndex( pEvent );
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getMeasurementZone
//

#ifdef WIN32
extern "C" int WINAPI EXPORT  vscphlp_getMeasurementZone( const vscpEvent *pEvent )
#else
extern "C" int vscphlp_getMeasurementZone( const vscpEvent *pEvent )
#endif
{
    return vscp_getVSCPMeasurementZone( pEvent );
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getMeasurementSubZone
//

#ifdef WIN32
extern "C" int WINAPI EXPORT  vscphlp_getMeasurementSubZone( const vscpEvent *pEvent )
#else
extern "C" int vscphlp_getMeasurementSubZone( const vscpEvent *pEvent )
#endif
{
    return vscp_getVSCPMeasurementSubZone( pEvent );
}

///////////////////////////////////////////////////////////////////////////////
// vscp_isMeasurement
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_isMeasurement( const vscpEvent *pEvent )
#else
extern "C" int vscphlp_isMeasurement( const vscpEvent *pEvent )
#endif
{
    return vscp_isVSCPMeasurement( pEvent ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}





///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventToJSON
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertEventToJSON( vscpEvent *pEvent, char *p, int size )
#else
extern "C" int vscphlp_convertEventToJSON( vscpEvent *pEvent, char *p, int size )
#endif
{
    wxString str;
    
    // Do the conversion
    vscp_convertEventToJSON( pEvent, str );
    
    // Check if there is room for the JSON string
    if ( size <= strlen( str.mbc_str() ) ) return VSCP_ERROR_BUFFER_TO_SMALL;
    
    // Copy in JSON string
    strncpy( p, str.mbc_str(), MIN( strlen( str.mbc_str() ), size ) );
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventExToJSON
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertEventExToJSON( vscpEventEx *pEventEx, char *p, int size )
#else
extern "C" int vscphlp_convertEventExToJSON( vscpEventEx *pEventEx, char *p, int size )
#endif
{
    wxString str;
    
    // Do the conversion
    vscp_convertEventExToJSON( pEventEx, str );
    
    // Check if there is room for the JSON string
    if ( size <= strlen( str.mbc_str() ) ) return VSCP_ERROR_BUFFER_TO_SMALL;
    
    // Copy in JSON string
    strncpy( p, str.mbc_str(), MIN( strlen( str.mbc_str() ), size ) );
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventToXML
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertEventToXML( vscpEvent *pEvent, char *p, int size )
#else
extern "C" int vscphlp_convertEventToXML( vscpEvent *pEvent, char *p, int size )
#endif
{
    wxString str;
    
    // Do the conversion
    vscp_convertEventToXML( pEvent, str );
    
    // Check if there is room for the XML string
    if ( size <= strlen( str.mbc_str() ) ) return VSCP_ERROR_BUFFER_TO_SMALL;
    
    // Copy in XML string
    strncpy( p, str.mbc_str(), MIN( strlen( str.mbc_str() ), size ) );
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventExToXML
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertEventExToXML( vscpEventEx *pEventEx, char *p, int size )
#else
extern "C" int vscphlp_convertEventExToXML( vscpEventEx *pEventEx, char *p, int size )
#endif
{
    wxString str;
    
    // Do the conversion
    vscp_convertEventExToXML( pEventEx, str );
    
    // Check if there is room for the XML string
    if ( size <= strlen( str.mbc_str() ) ) return VSCP_ERROR_BUFFER_TO_SMALL;
    
    // Copy in XML string
    strncpy( p, str.mbc_str(), MIN( strlen( str.mbc_str() ), size ) );
    
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventToHTML
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertEventToHTML( vscpEvent *pEvent, char *p, int size )
#else
extern "C" int vscphlp_convertEventToHTML( vscpEvent *pEvent, char *p, int size )
#endif
{
    wxString str;
    
    // Do the conversion
    vscp_convertEventToHTML( pEvent, str );
    
    // Check if there is room for the HTML string
    if ( size <= strlen( str.mbc_str() ) ) return VSCP_ERROR_BUFFER_TO_SMALL;
    
    // Copy in HTML string
    strncpy( p, str.mbc_str(), MIN( strlen( str.mbc_str() ), size ) );
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertEventExToHTML
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertEventExToHTML( vscpEventEx *pEventEx, char *p, int size )
#else
extern "C" int vscphlp_convertEventExToHTML( vscpEventEx *pEventEx, char *p, int size )
#endif
{
    wxString str;
    
    // Do the conversion
    vscp_convertEventExToHTML( pEventEx, str );
    
    // Check if there is room for the HTML string
    if ( size <= strlen( str.mbc_str() ) ) return VSCP_ERROR_BUFFER_TO_SMALL;
    
    // Copy in HTML string
    strncpy( p, str.mbc_str(), MIN( strlen( str.mbc_str() ), size ) );
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertLevel1MeasuremenToLevel2Double
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertLevel1MeasuremenToLevel2Double( vscpEvent *pEventLevel1 )
#else
extern "C" int vscphlp_convertLevel1MeasuremenToLevel2Double( vscpEvent *pEventLevel1 )
#endif
{
    return ( vscp_convertLevel1MeasuremenToLevel2Double( pEventLevel1 ) ?
                    VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_convertLevel1MeasuremenToLevel2String
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_convertLevel1MeasuremenToLevel2String( vscpEvent *pEventLevel1 )
#else
extern "C" int vscphlp_convertLevel1MeasuremenToLevel2String( vscpEvent *pEventLevel1 )
#endif
{
    return ( vscp_convertLevel1MeasuremenToLevel2String( pEventLevel1 ) ?
                    VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_makeLevel2FloatMeasurementEvent
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_makeLevel2FloatMeasurementEvent( vscpEvent *pEvent, 
                                                                        uint16_t type,
                                                                        double value,
                                                                        uint8_t unit,
                                                                        uint8_t sensoridx,
                                                                        uint8_t zone,
                                                                        uint8_t subzone )
#else
extern "C" int vscphlp_makeLevel2FloatMeasurementEvent( vscpEvent *pEvent, 
                                                            uint16_t type,
                                                            double value,
                                                            uint8_t unit,
                                                            uint8_t sensoridx,
                                                            uint8_t zone,
                                                            uint8_t subzone )
#endif
{
    return ( vscp_makeLevel2FloatMeasurementEvent( pEvent, 
                                                    type,
                                                    value,
                                                    unit,
                                                    sensoridx,
                                                    zone,
                                                    subzone ) ?
                    VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR );
}


///////////////////////////////////////////////////////////////////////////////
// vscphlp_makeLevel2StringMeasurementEvent
//

#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_makeLevel2StringMeasurementEvent( vscpEvent *pEvent, 
                                                                        uint16_t type,
                                                                        double value,
                                                                        uint8_t unit,
                                                                        uint8_t sensoridx,
                                                                        uint8_t zone,
                                                                        uint8_t subzone )
#else
extern "C" int vscphlp_makeLevel2StringMeasurementEvent( vscpEvent *pEvent, 
                                                            uint16_t type,
                                                            double value,
                                                            uint8_t unit,
                                                            uint8_t sensoridx,
                                                            uint8_t zone,
                                                            uint8_t subzone )
#endif
{
    return ( vscp_makeLevel2FloatMeasurementEvent( pEvent, 
                                                    type,
                                                    value,
                                                    unit,
                                                    sensoridx,
                                                    zone,
                                                    subzone ) ?
                    VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR );
}