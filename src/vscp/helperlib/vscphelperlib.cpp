// VSCP helper dll.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
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
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//


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

#include "../common/vscphelper.h"
#include "../common/vscpremotetcpif.h"
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
//						 T C P / I P  I N T E R F A C E
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
                                                    unsigned char timeout )
#else
extern "C" int vscphlp_setResponseTimeout(long handle, 
                                                    unsigned char timeout )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    pvscpif->setResponseTimeout( timeout );

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
	if ( NULL == pvscpif ) VSCP_ERROR_INVALID_HANDLE;

    return pvscpif->isConnected() ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_doCommand
//
#ifdef WIN32
int WINAPI EXPORT vscphlp_doCommand( long handle, const char * cmd )
#else
int vscphlp_doCommand( long handle, const char * cmd )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString wxCmd = wxString::FromUTF8( cmd );
    return pvscpif->doCommand( wxCmd );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_checkReply
//
#ifdef WIN32
int WINAPI EXPORT vscphlp_checkReply( long handle, int bClear )
#else
int vscphlp_checkReply( long handle, int bClear )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    return pvscpif->checkReturnValue( bClear ? true : false ) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_clearLocalInputQueue
//
#ifdef WIN32
int WINAPI EXPORT vscphlp_clearLocalInputQueue( long handle )
#else
int vscphlp_clearInputQueue( long handle )
#endif
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INVALID_HANDLE;

    pvscpif->doClrInputQueue();
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_OpenInterface
//
#ifdef WIN32
extern "C" long WINAPI EXPORT vscphlp_openInterface( long handle,
												            const char *pInterface,
                                                            unsigned long flags )
#else
extern "C" long vscphlp_openInterface( long handle,
										const char *pInterface,
                                        unsigned long flags )
#endif
{	
	wxString strInterface;
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INVALID_HANDLE;

    strInterface = wxString::FromUTF8( pInterface );
    return pvscpif->doCmdOpen( strInterface, flags );
}

///////////////////////////////////////////////////////////////////////////////
// vscphlp_open
//
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_open( const long handle,
												    const char *pHostname, 
                                                    const char *pUsername, 
                                                    const char *pPassword )
#else
extern "C" int vscphlp_open( const long handle,
									const char *pHostname, 
                                    const char *pUsername, 
                                    const char *pPassword )
#endif
{	
	wxString strHostname;
    wxString strUsername;
    wxString strPassword;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INVALID_HANDLE;

    strHostname = wxString::FromUTF8( pHostname );
    strUsername = wxString::FromUTF8( pUsername );
    strPassword = wxString::FromUTF8( pPassword );

    return pvscpif->doCmdOpen( strHostname, strUsername, strPassword );
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
extern "C" int vscphlp_clearInQueue( long handle )
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

    pvscpif->doCmdVersion( pMajorVer, pMinorVer, pSubMinorVer);

    return VSCP_ERROR_SUCCESS;
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
    strncpy( pVendorStr, str.mbc_str(), size );
    
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
    strncpy( pInfoStr, str.mbc_str(), size );
    
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
    \fn bool vscphlp_getVariableString( const char *pName, char *pValue ) 
    \brief Get variable value from string variable
    \param name of variable
    \param pointer to string that get the value of the string variable.
    \param size fo buffer
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableString( long handle, const char *pName, char *pValue, int size ) 
#else
extern "C" int vscphlp_getVariableString( long handle, const char *pName, char *pValue, int size ) 
#endif
{ 
    int rv;

	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    if ( VSCP_ERROR_SUCCESS == ( rv = pvscpif->getVariableString( name, &strValue ) ) ) {
        strncpy( pValue, strValue.ToAscii(), size );
    }

    return rv;
}

/*!
    \fn bool vscphlp_setVariableString( const char *pName, char *pValue ) 
    \brief set string variable from a pointer to a string
    \param name of variable
    \param pointer to string that contains the string.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableString( long handle, const char *pName, char *pValue ) 
#else
extern "C" int vscphlp_setVariableString( long handle, const char *pName, char *pValue ) 
#endif
{ 
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    // Check pointers
    if ( NULL == pName ) return false;
    if ( NULL == pValue ) return false;

    wxString name = wxString::FromAscii( pName );
    wxString strValue = wxString::FromAscii( pValue );

    return pvscpif->setVariableString( name, strValue );
}

/*!
    \fn bool vscphlp_getVariableBool( const char *pName, bool *bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableBool( long handle, const char *pName, int *bValue )
#else
extern "C" int vscphlp_getVariableBool( long handle, const char *pName, int *bValue )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
	bool bBoolValue;
    int rv =  pvscpif->getVariableBool( name, &bBoolValue ); 
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
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableBool( long handle, const char *pName, int bValue )
#else
extern "C" int vscphlp_setVariableBool( long handle, const char *pName, int bValue )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableBool( name, ( bValue ? true : false ) ); 
};


/*!
    \fn bool vscphlp_getVariableInt( const char *pName, int *value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableInt( long handle, const char *pName, int *value )
#else
extern "C" int vscphlp_getVariableInt( long handle, const char *pName, int *value )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableInt( name, value ); 
};


/*!
    \fn bool vscphlp_setVariableInt( const char *pName, int value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableInt( long handle, const char *pName, int value )
#else
extern "C" int vscphlp_setVariableInt( long handle, const char *pName, int value )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableInt( name, value ); 
};

/*!
    \fn bool vscphlp_getVariableLong( const char *pName, long *value )
    \brief Get variable value from long variable
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableLong( long handle, const char *pName, long *value )
#else
extern "C" int vscphlp_getVariableLong( long handle, const char *pName, long *value )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableLong( name, value );
};

/*!
    \fn bool vscphlp_setVariableLong( const char *pName, long value )
    \brief Get variable value from long variable
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableLong( long handle, const char *pName, long value )
#else
extern "C" int vscphlp_setVariableLong( long handle, const char *pName, long value )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableLong( name, value );
};

/*!
    \fn bool vscphlp_getVariableDouble( const char *pName, double *value )
    \brief Get variable value from double variable
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableDouble( long handle, const char *pName, double *value )
#else
extern "C" int vscphlp_getVariableDouble( long handle, const char *pName, double *value )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableDouble( name, value );
};

/*!
    \fn bool vscphlp_setVariableDouble( const char *pName, double value )
    \brief Get variable value from double variable
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableDouble( long handle, const char *pName, double value )
#else
extern "C" int vscphlp_setVariableDouble( long handle, const char *pName, double value )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableDouble( name, value );
};

/*!
    \fn bool vscphlp_getVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param name of variable
    \param String that get that get the 
        value of the measurement.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableMeasurement( long handle, const char *pName, char *pValue, int size )
#else
extern "C" int vscphlp_getVariableMeasurement( long handle, const char *pName, char *pValue, int size )
#endif
{ 
    int rv;

	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    if ( VSCP_ERROR_SUCCESS == ( rv = pvscpif->getVariableMeasurement( name, strValue ) ) ) {
        strncpy( pValue, strValue.ToAscii(), size );
    }

    return rv;
};

/*!
    \fn bool vscphlp_setVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param name of variable
    \param String that get that get the 
    value of the measurement.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableMeasurement( long handle, const char *pName, const char *pValue )
#else
extern "C" int vscphlp_setVariableMeasurement( long handle, const char *pName, char *pValue )
#endif
{ 
    bool rv;

	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    wxString strValue = wxString::FromAscii( pValue );
    return pvscpif->setVariableMeasurement( name, strValue );

    return rv;
};

/*!
    \fn bool vscphlp_getVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableEvent( long handle, const char *pName, vscpEvent *pEvent )
#else
extern "C" int vscphlp_getVariableEvent( long handle, const char *pName, vscpEvent *pEvent )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableEvent( name, pEvent );
}

/*!
    \fn bool vscphlp_setVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableEvent( long handle, const char *pName, vscpEvent *pEvent )
#else
extern "C" int vscphlp_setVariableEvent( long handle, const char *pName, vscpEvent *pEvent )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableEvent( name, pEvent );
}

/*!
    \fn bool vscphlp_getVariableEventEx( const char *pName, vscpEventEx *pEvent )
    \brief Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent )
#else
extern "C" int vscphlp_getVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableEventEx( name, pEvent ); 
}

/*!
    \fn bool vscphlp_setVariableEventEx( const char *pName, vscpEventEx *pEvent )
    \brief Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent )
#else
extern "C" int vscphlp_setVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableEventEx( name, pEvent );
}

/*!
    \fn bool vscphlp_getVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableGUIDString( long handle, const char *pName, char *pGUID, int size )
#else
extern "C" int vscphlp_getVariableGUIDString( long handle, const char *pName, char *pGUID, int size )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    cguid GUID;
    wxString strGuid;
    wxString name = wxString::FromAscii( pName );
    int rv =  pvscpif->getVariableGUID( name, GUID ); 
    GUID.toString( strGuid );
    strncpy( pGUID, strGuid.mbc_str(), size );
    return rv;
}

/*!
    \fn bool vscphlp_setVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableGUIDString( long handle, const char *pName, const char *pGUID )
#else
extern "C" int vscphlp_setVariableGUIDString( long handle, const char *pName, const char *pGUID )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;
    
    cguid guid;
    guid.getFromString( pGUID );
    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableGUID( name, guid );
}

/*!
    \fn bool vscphlp_getVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer GUID array
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableGUIDArray( long handle, const char *pName, unsigned char *pGUID )
#else
extern "C" int vscphlp_getVariableGUIDArray( long handle, const char *pName, unsigned char *pGUID )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    cguid GUID;
    wxString name = wxString::FromAscii( pName );
    int rv =  pvscpif->getVariableGUID( name, GUID ); 
    memcpy( pGUID, GUID.getGUID(), 16 );  
    return rv;
}

/*!
    \fn bool vscphlp_setVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableGUIDArray( long handle, const char *pName, const unsigned char *pGUID )
#else
extern "C" int vscphlp_setVariableGUIDArray( long handle, const char *pName, const unsigned char *pGUID )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;
    
    cguid guid;
    guid.getFromArray( pGUID );
    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableGUID( name, guid );
}

/*!
    \fn bool vscphlp_getVariableVSCPdata( const char *pName, uint16_t *psizeData, uint8_t *pData )
    \brief Get variable value from VSCP data variable
    \param name of variable
    \param Pointer to variable that will hold the size of the data array
    \param pointer to VSCP data array variable (unsigned char [8] ) that get the 
    value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short *psize )
#else
extern "C" int vscphlp_getVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short *psize )
#endif
{ 
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableVSCPdata( name, pData, psize ); 
}

/*!
    \fn bool vscphlp_setVariableVSCPdata( const char *pName, uint16_t sizeData, uint8_t *pData )
    \brief Get variable value from VSCP data variable
    \param name of variable
    \param Pointer to variable that will hold the size of the data array
    \param pointer to VSCP data array variable (unsigned char [8] ) that get the 
    value of the string variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableVSCPData( long handle, const char *pName,  uint8_t *pData, unsigned short size )
#else
extern "C" int vscphlp_setVariableVSCPData( long handle, const char *pName, uint8_t *pData, unsigned short size )
#endif
{ 
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableVSCPdata( name, pData, size ); 
}

/*!
    \fn bool vscphlp_getVariableVSCPclass( const char *pName, uint16_t *vscphlp_class )
    \brief Get variable value from class variable
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableVSCPClass( long handle, const char *pName, unsigned short *vscp_class )
#else
extern "C" int vscphlp_getVariableVSCPClass( long handle, const char *pName, unsigned short *vscp_class )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableVSCPclass( name, vscp_class ); 
}

/*!
    \fn bool vscphlp_setVariableVSCPclass( const char *pName, uint16_t vscp_class )
    \brief Get variable value from class variable
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableVSCPClass( long handle, const char *pName, unsigned short vscp_class )
#else
extern "C" int vscphlp_setVariableVSCPClass( long handle, const char *pName, unsigned short vscp_class )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableVSCPclass( name, vscp_class );
}

/*!
    \fn bool vscphlp_getVariableVSCPtype( const char *pName, uint8_t *vscp_type )
    \brief Get variable value from type variable
    \param name of variable
    \param pointer to int that get the value of the type variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_getVariableVSCPType( long handle, const char *pName, unsigned short *vscp_type )
#else
extern "C" int vscphlp_getVariableVSCPType( long handle, const char *pName, unsigned short *vscp_type )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableVSCPtype( name, vscp_type );
}

/*!
    \fn bool WINAPI EXPORT vscphlp_setVariableVSCPtype( const char *pName, uint8_t vscp_type )
    \brief Get variable value from type variable
    \param name of variable
    \param pointer to int that get the value of the type variable.
    \return true if the variable is of type string.
*/
#ifdef WIN32
extern "C" int WINAPI EXPORT vscphlp_setVariableVSCPType( long handle, const char *pName, unsigned short vscp_type )
#else
extern "C" int vscphlp_setVariableVSCPtype( long handle, const char *pName, unsigned short vscp_type )
#endif
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INVALID_HANDLE;

    // Check that we are connected
    if ( !pvscpif->isConnected() ) return VSCP_ERROR_CONNECTION;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableVSCPtype( name, vscp_type );
}





////////////////////////////////////////////////////////////////////////////////
//                              H E L P E R S
////////////////////////////////////////////////////////////////////////////////





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
    strVal.FromAscii( pStrValue );

    return vscp_readStringValue( strVal );
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


/*!
    \fn vscphlp_getVSCPheadFromCANid( const unsigned long id )
    \brief Get the VSCP head from a CANAL message id (CAN id).
*/
#ifdef WIN32
extern "C" unsigned char WINAPI EXPORT vscphlp_getVSCPheadFromCANid( const unsigned long id )
#else
extern "C" unsigned char vscphlp_getVSCPheadFromCANid( const unsigned long id )
#endif
{
    return  vscp_getVSCPheadFromCANid( id );
}

/*!
    \fn vscphlp_getVSCPclassFromCANid( const unsigned long id )
    \brief Get the VSCP class from a CANAL message id (CAN id).
*/
#ifdef WIN32
extern "C" unsigned short WINAPI EXPORT vscphlp_getVSCPclassFromCANid( const unsigned long id )
#else
extern "C" unsigned short vscphlp_getVSCPclassFromCANid( const unsigned long id )
#endif
{
    return vscp_getVSCPclassFromCANid( id );
}


/*!
    \fn unsigned short vscphlp_getVSCPtypeFromCANid( const unsigned long id )
    \brief Get the VSCP type from a a CANAL message id (CAN id).
*/
#ifdef WIN32
extern "C" unsigned short WINAPI EXPORT vscphlp_getVSCPtypeFromCANid( const unsigned long id )
#else
extern "C" unsigned short vscphlp_getVSCPtypeFromCANid( const unsigned long id )
#endif
{
    return vscp_getVSCPtypeFromCANid( id );
}

/*!
    \fn unsigned short vscphlp_getVSCPnicknameFromCANid( const unsigned long id )
    \brief Get the VSCP nickname from a a CANAL message id (CAN id).
*/
#ifdef WIN32
extern "C" unsigned short WINAPI EXPORT vscphlp_getVSCPnicknameFromCANid( const unsigned long id )
#else
extern "C" unsigned short vscphlp_getVSCPnicknameFromCANid( const unsigned long id )
#endif
{
    return vscp_getVSCPnicknameFromCANid( id );
}

/*!
    \fn unsigned long vscphlp_getCANidFromVSCPdata( const unsigned char priority, 
                                                    const unsigned short vscp_class, 
                                                    const unsigned short vscp_type )
    \brief Construct a CANAL id (CAN id ) from VSCP.
*/
#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT vscphlp_getCANidFromVSCPdata( const unsigned char priority, 
                                                                    const unsigned short vscp_class, 
                                                                   const unsigned short vscp_type )
#else
extern "C" unsigned long vscphlp_getCANidFromVSCPdata( const unsigned char priority, 
                                                                    const unsigned short vscp_class, 
                                                                   const unsigned short vscp_type )
#endif
{
    return  vscp_getCANidFromVSCPdata( priority, vscp_class, vscp_type );
}

/*!
    \fn unsigned long vscphlp_getCANidFromVSCPevent( const vscpEvent *pEvent )
    \brief Get CANAL id (CAN id) from VSCP event.
*/
#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT vscphlp_getCANidFromVSCPevent( const vscpEvent *pEvent )
#else
extern "C" unsigned long vscphlp_getCANidFromVSCPevent( const vscpEvent *pEvent )
#endif
{
    return vscp_getCANidFromVSCPevent( pEvent );
}

/*!
    \fn unsigned short vscphlp_calcCRC( vscpEvent *pEvent, short bSet )
    \brief Calculate VSCP crc.
*/
#ifdef WIN32
extern "C" unsigned short WINAPI EXPORT vscphlp_calcCRC( vscpEvent *pEvent, short bSet )
#else
extern "C" unsigned short vscphlp_calcCRC( vscpEvent *pEvent, short bSet )
#endif
{
    return vscp_vscp_calc_crc( pEvent, bSet );
}


/*!
    \fn bool vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
    \brief Write GUID into VSCP event from string.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
#else
extern "C" bool vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
#endif
{
    wxString strGUID = wxString::FromAscii( pGUID );
    return  vscp_getGuidFromString( pEvent, strGUID );
}

/*!
    \fn bool vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
    \brief Write GUID from string into array.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
#else
extern "C" bool vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
#endif
{
    wxString strGUID = wxString::FromAscii( pStr );
    return vscp_getGuidFromStringToArray( pGUID, strGUID );
}

/*!
    \fn bool vscphlp_writeGuidToString( const vscpEvent *pEvent, char * pStr )
    \brief Write GUID froom VSCP event to string.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_writeGuidToString( const vscpEvent *pEvent, char * pStr )
#else
extern "C" bool vscphlp_writeGuidToString( const vscpEvent *pEvent, char * pStr )
#endif
{
    bool rv;

    wxString strGUID;
    rv = vscp_writeGuidToString( pEvent, strGUID );
    strcpy( pStr, strGUID.ToAscii() );
    return rv;
}


/*!
    \fn bool vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                            wxString& strGUID )
    \brief Write GUID from VSCP event to string with four bytes on each
    row seperated by \r\n. 
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                                                wxString& strGUID )
#else
extern "C" bool vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                                                wxString& strGUID )
#endif
{
    return vscp_writeGuidToString4Rows( pEvent, strGUID );
}

/*!
    \fn bool vscphlp_writeGuidArrayToString( const unsigned char * pGUID, 
                                            wxString& strGUID )
    \brief Write GUID from byte array to string.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_writeGuidArrayToString( const unsigned char * pGUID, 
                                                                wxString& strGUID )
#else
extern "C" bool vscphlp_writeGuidArrayToString( const unsigned char * pGUID, 
                                                                wxString& strGUID )
#endif
{
    return vscp_writeGuidArrayToString( pGUID, strGUID );
}

/*!
    \fn bool vscphlp_isGUIDEmpty( unsigned char *pGUID )
    \brief Check if GUID is empty (all nills).
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_isGUIDEmpty( unsigned char *pGUID )
#else
extern "C" bool vscphlp_isGUIDEmpty( unsigned char *pGUID )
#endif
{
    return vscp_isGUIDEmpty( pGUID );
}

/*!
    \fn bool vscphlp_isSameGUID( const unsigned char *pGUID1, 
                                const unsigned char *pGUID2 )
    \brief Check if two GUID's is equal to each other.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_isSameGUID( const unsigned char *pGUID1, 
                                                const unsigned char *pGUID2 )
#else
extern "C" bool vscphlp_isSameGUID( const unsigned char *pGUID1, 
                                                const unsigned char *pGUID2 )
#endif
{
    return vscp_isSameGUID( pGUID1, pGUID2 );
}

/*!
    \fn bool vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                    const vscpEvent *pEvent )
    \brief Convert VSCP standard event form to ex. form.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                                        const vscpEvent *pEvent )
#else
extern "C" bool vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                                        const vscpEvent *pEvent )
#endif
{
    return vscp_convertVSCPtoEx( pEventEx, pEvent );
}

/*!
    \fn bool vscphlp_convertVSCPfromEx( vscpEvent *pEvent, 
                                        const vscpEventEx *pEventEx )
    \brief Convert VSCP ex. event form to standard form.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_convertVSCPfromEx( vscpEvent *pEvent, 
                                                        const vscpEventEx *pEventEx )
#else
extern "C" bool vscphlp_convertVSCPfromEx( vscpEvent *pEvent, 
                                                        const vscpEventEx *pEventEx )
#endif
{
    return vscp_convertVSCPfromEx( pEvent, pEventEx );
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
    \fn bool readFilterFromString( vscpEventFilter *pFilter, wxString& strFilter )
    \brief Read a filter from a string
	\param pFilter Filter structure to write filter to.
	\param strFilter Filter in string form 
				filter-priority, filter-class, filter-type, filter-GUID
	\return true on success, fals eon failure.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_readFilterFromString( vscpEventFilter *pFilter, wxString& strFilter )
#else
extern "C" bool vscphlp_readFilterFromString( vscpEventFilter *pFilter, wxString& strFilter )
#endif
{
	return vscp_readFilterFromString( pFilter, strFilter );
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
extern "C" bool WINAPI EXPORT vscphlp_readMaskFromString( vscpEventFilter *pFilter, wxString& strMask )
#else
extern "C" bool vscphlp_readMaskFromString( vscpEventFilter *pFilter, wxString& strMask )
#endif
{
	return vscp_readMaskFromString( pFilter, strMask );
}

/*!
    \fn bool vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                    const vscpEventFilter *pFilter )
    \brief Check VSCP filter condition.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                                    const vscpEventFilter *pFilter )
#else
extern "C" bool vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                                    const vscpEventFilter *pFilter )
#endif
{
    return  vscp_doLevel2Filter( pEvent, pFilter );
}


/*!
    \fn bool vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                            const canalMsg *pcanalMsg,
                                            unsigned char *pGUID,
                                            bool bCAN )
    \brief Convert CANAL message to VSCP event.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                                            const canalMsg *pcanalMsg,
                                                            unsigned char *pGUID,
                                                            bool bCAN )
#else
extern "C" bool vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                                            const canalMsg *pcanalMsg,
                                                            unsigned char *pGUID,
                                                            bool bCAN )
#endif
{
    return vscp_convertCanalToEvent( pvscpEvent,
                                        pcanalMsg,
                                        pGUID,
                                        bCAN );
}


/*!
    \fn bool vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                        const vscpEvent *pvscpEvent )
    \brief Convert VSCP event to CANAL message.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                                            const vscpEvent *pvscpEvent )
#else
extern "C" bool vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                                            const vscpEvent *pvscpEvent )
#endif
{
    return  vscp_convertEventToCanal( pcanalMsg, pvscpEvent );
}


/*!
    \fn bool vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                            const vscpEventEx *pvscpEventEx )
    \brief Convert VSCP event ex. to CANAL message.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                                            const vscpEventEx *pvscpEventEx )
#else
extern "C" bool vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                                            const vscpEventEx *pvscpEventEx )
#endif
{
    return  vscp_convertEventExToCanal( pcanalMsg, pvscpEventEx );
}

/*!
    \fn unsigned long vscphlp_getTimeStamp( void )
    \brief Get VSCP timestamp.
*/
#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT vscphlp_getTimeStamp( void )
#else
extern "C" unsigned long vscphlp_getTimeStamp( void )
#endif
{
    return vscp_makeTimeStamp();
}

/*!
    \fn bool vscphlp_copyVSCPEvent( vscpEvent *pEventTo, 
                                    const vscpEvent *pEventFrom )
    \brief Copy VSCP event.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_copyVSCPEvent( vscpEvent *pEventTo, 
                                                    const vscpEvent *pEventFrom )
#else
extern "C" bool vscphlp_copyVSCPEvent( vscpEvent *pEventTo, 
                                                    const vscpEvent *pEventFrom )
#endif
{
    return vscp_copyVSCPEvent( pEventTo, pEventFrom );
}

/*!
    \fn bool vscphlp_writeVscpDataToString( const vscpEvent *pEvent, 
                                            wxString& str, 
                                            bool bUseHtmlBreak )
    \brief Write VSCP data in readable form to a (multiline) string.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_writeVscpDataToString( const vscpEvent *pEvent, 
                                                            wxString& str, 
                                                            bool bUseHtmlBreak )
#else
extern "C" bool vscphlp_writeVscpDataToString( const vscpEvent *pEvent, 
                                                            wxString& str, 
                                                            bool bUseHtmlBreak )
#endif
{
    return vscp_writeVscpDataToString( pEvent, 
                                    str, 
                                    bUseHtmlBreak );
}


/*!
    \fn bool vscphlp_getVscpDataFromString( vscpEvent *pEvent, 
                                            const wxString& str )
    \brief Set data in VSCP event from a string.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_getVscpDataFromString( vscpEvent *pEvent, 
                                                                const wxString& str )
#else
extern "C" bool vscphlp_getVscpDataFromString( vscpEvent *pEvent, 
                                                   const wxString& str )
#endif
{
    return vscp_getVscpDataFromString( pEvent, str );
}

/*!
    \fn bool vscphlp_writeVscpEventToString( vscpEvent *pEvent, 
                                            char *p )
    \brief Write VSCP data to a string.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_writeVscpEventToString( vscpEvent *pEvent, 
                                                                char *p )
#else
extern "C" bool vscphlp_writeVscpEventToString( vscpEvent *pEvent, 
                                                  char *p )
#endif
{
    bool rv;

    wxString str = wxString::FromAscii( p );
    if ( ( rv =  vscp_writeVscpEventToString( pEvent, str ) ) ) {
        strcpy( p, str.ToAscii() );
    }
    return rv;
}

/*!
    \fn bool vscphlp_getVscpEventFromString( vscpEvent *pEvent, 
                                            const char *p )
    \brief Get VSCP event from string.
*/
#ifdef WIN32
extern "C" bool WINAPI EXPORT vscphlp_getVscpEventFromString( vscpEvent *pEvent, 
                                                                const char *p )
#else
extern "C" bool vscphlp_getVscpEventFromString( vscpEvent *pEvent, 
                                                                const char *p )
#endif
{
    wxString str = wxString::FromAscii( p );
    return vscp_getVscpEventFromString( pEvent, str ); 
}


