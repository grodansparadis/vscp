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

//#include "stdafx.h"
//#define WIN32_LEAN_AND_MEAN
#include "wx/wxprec.h"
#include "wx/wx.h"

//#include "dlldrvobj.h"

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

#include "../../common/vscphelper.h"
#include "../../common/vscpremotetcpif.h"
#include "dlldrvobj.h"
#include "vscphelperdll.h"

#ifdef _MSC_VER
   #include <wx/msw/msvcrt.h>
#endif

static HANDLE hThisInstDll = NULL;
CHelpDllObj theApp;

///////////////////////////////////////////////////////////////////////////////
// DllMain
//

BOOL APIENTRY DllMain( HANDLE hInstDll, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved )
{
	switch( ul_reason_for_call ) {

		case DLL_PROCESS_ATTACH:
			hThisInstDll = hInstDll;
            wxInitialize();
			theApp.InitInstance();
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
 			break;

		case DLL_PROCESS_DETACH:
            wxUninitialize();
 			break;
   }

	return TRUE;

}






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
    \fn long vscphlp_newSession(void)
    \brief Initialise the library. This is only needed
            if you use methods which has a handle in them that is
            methods that talk to the VSCP daemon TCP/IP interface.
    \return The handle for the TCP/IP server object or
            zero if the initialisation failed. 
 */

extern "C" long WINAPI vscphlp_newSession( void )
{
	//if ( NULL == theApp ) return NULL;

    VscpRemoteTcpIf *pvscpif = new VscpRemoteTcpIf;
    if (NULL == pvscpif) return 0;

    return theApp.addDriverObject( pvscpif );
}

/*!
    \fn void vscphlp_closeSession(long handle)
    \brief Clean up the library. This is only needed
            if you use methods which has a handle in them that is
            methods that talk to the VSCP daemon TCP/IP interface.
    \return The handle for the TCP/IP server object or
            zero if the initialisation failed. 
*/

extern "C" void WINAPI vscphlp_closeSession(long handle)
{
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
    if (NULL != pvscpif) pvscpif->doCmdClose();

    theApp.removeDriverObject(handle);
}


/*!
    \fn long vscphlp_OpenInterface( const char *pInterface,
                                            unsigned long flags )
	\return true if channel is open or false if error or the channel is
        already opened.							
    \brief Open interface
*/

extern "C" long WINAPI EXPORT vscphlp_OpenInterface( long handle,
												            const char *pInterface,
                                                            unsigned long flags )
{	
	wxString strInterface;
    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) 0;

    strInterface = wxString::FromUTF8( pInterface );
    return pvscpif->doCmdOpen( strInterface, flags );
}


/*!
    \fn long vscphlp_Open( const long handle,
                                    const char *pHostname, 
                                    const char *pUsername,
                                    const char *pPassword )
    \handle handle for dll
    \phostname Name of host to conect to including port and prefix
    \pUserame  Username to use as credentials to login with.
    \pPassword  Password to use as credentials to login wiith.
	\return true if channel is open or false if error or the channel is
        already opened.							
    \brief Open interface
*/

extern "C" long WINAPI EXPORT vscphlp_Open( const long handle,
												    const char *pHostname, 
                                                    const char *pUsername, 
                                                    const char *pPassword )
{	
	wxString strHostname;
    wxString strUsername;
    wxString strPassword;

    VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    strHostname = wxString::FromUTF8( pHostname );
    strUsername = wxString::FromUTF8( pUsername );
    strPassword = wxString::FromUTF8( pPassword );

    return pvscpif->doCmdOpen( strHostname, strUsername, strPassword );
}


/*!
    \fn int vscphlp_Close( void )
    \brief Close interface
	\return true if the close was successful.
*/

extern "C" int WINAPI EXPORT vscphlp_Close( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdClose();
}


/*!
    \fn int vscphlp_doCmdNoop( void )
    \brief No operation (use for tests).
	\return true if success false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_Noop( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdNOOP();
}

/*!
    \fn long vscphlp_doCmdGetLevel( void )
    \brief Get driverlevel.
	\return CANAL_LEVEL_USES_TCPIP for a Level II driver and
	CANAL_LEVEL_STANDARD for a Level I driver. Will return 
	CANAL_ERROR_NOT_SUPPORTED if the driver reports some unknown 
	level.
*/

extern "C" unsigned long WINAPI EXPORT vscphlp_doCmdGetLevel( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdGetLevel();
}


/*!
    \fn int vscphlp_doCmdSendEvent( const vscpEvent *pEvent )
    \brief Send VSCP event.
	\return true if success false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdSendEvent( long handle,
													const vscpEvent *pEvent )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdSend( pEvent );
}

/*!
    \fn int vscphlp_doCmdSendEventEx( const vscpEventEx *pEvent )
    \brief Send VSCP event (data is in event).
	\return true if success false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdSendEventEx( long handle,
														const vscpEventEx *pEvent )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdSendEx( pEvent );
}


/*!
    \fn int vscphlp_doCmdReceiveEvent( vscpEvent *pEvent )
    \brief Receive a VSCP event
	\return True if success false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdReceiveEvent( long handle,
														vscpEvent *pEvent )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdReceive( pEvent );
}

/*!
    \fn int vscphlp_doCmdReceiveEventEx( vscpEventEx *pEvent )
    \brief Receive a VSCP event.
	\return True if success false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdReceiveEventEx( long handle,
														vscpEventEx *pEvent )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdReceiveEx( pEvent );
}

/*!
    \fn int vscphlp_doCmdDataAvailable( void )
    \brief Check if an event is available.
	\return The number of messages available or if negative
        an error code.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdDataAvailable( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdDataAvailable();
}

/*!
    \fn int vscphlp_doCmdStatus( canalStatus *pStatus )
    \brief Get CANAL status.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdStatus( long handle,
													canalStatus *pStatus )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdStatus( pStatus );
}

/*!
    \fn int vscphlp_doCmdStatistics( canalStatistics *pStatistics )
    \brief Get CANAL statistics.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdStatistics( long handle,
														canalStatistics *pStatistics )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdStatistics( pStatistics );
}

/*!
    \fn int vscphlp_doCmdFilter( unsigned long filter )
    \brief Set CANAL filter.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdFilter( long handle,
													unsigned long filter )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdFilter( filter );
}


/*!
    \fn int vscphlp_doCmdMask( unsigned long mask )
    \brief Set CANAL mask.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdMask( long handle,
												unsigned long mask )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdMask( mask );
}

/*!
    \fn int vscphlp_doCmdVscpFilter( const vscpEventFilter *pFilter )
    \brief Set VSCP filter/mask.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdVscpFilter( long handle,
														const vscpEventFilter *pFilter )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INIT_MISSING;

   return pvscpif->doCmdFilter( pFilter );
}


/*!
    \fn long vscphlp_doCmdVersion( void  )
    \brief Get interface version.
	\return Return the interface version.
*/

extern "C" unsigned long WINAPI EXPORT vscphlp_doCmdVersion( long handle  )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdVersion();
}

/*!
    \fn long vscphlp_doCmdDLLVersion( void )
    \brief Get driver dll version.
	\return Return the interface dll version.
*/

extern "C" unsigned long WINAPI EXPORT vscphlp_doCmdDLLVersion( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return VSCP_ERROR_INIT_MISSING;

    return pvscpif->doCmdDLLVersion();
}

/*!
    \fn char * vscphlp_doCmdVendorString( void )
    \brief Get vendor string.
	\return Pointer to vendor string.
*/

extern "C"  const char * WINAPI EXPORT vscphlp_doCmdVendorString( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) NULL;

    return pvscpif->doCmdVendorString();
}

/*!
    \fn char * vscphlp_doCmdGetDriverInfo( void )
    \brief Get driver information.
	\return Pointer to driver information string.
*/

extern "C"  const char * WINAPI EXPORT vscphlp_doCmdGetDriverInfo( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) NULL;

    return pvscpif->doCmdGetDriverInfo();
}

/*!
    \fn int vscphlp_getDeviceType( void )
    \brief Get the type of interface that is active.
	\return USE_DLL_INTERFACE if direct DLL interface active.
            USE_TCPIP_INTERFACE	if TCP/IP interface active.
*/
/*
extern "C" int WINAPI EXPORT vscphlp_getDeviceType( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    return pvscpif->getDeviceType();
}*/

/*!
    \fn bool vscphlp_isOpen( void )
    \brief Check if communication channel is open.
	\return True of open, false if not.
*/

extern "C" BOOL WINAPI EXPORT vscphlp_isConnected( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    return pvscpif->isConnected();
}

/*!
    \fn int vscphlp_doCmdShutDown( void )
    \brief Shutdown daemon.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscphlp_doCmdShutDown( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) 0;

    return pvscpif->doCmdShutDown();
}

/*!
    \fn VscpRemoteTcpIf  vscphlp_getTcpIpInterface( void )
    \brief Get pointer to TCP/IP interface object.
	\return Pointer to the TCP/IP interface object if OK, 
		NULL if failure.
*/
/*
extern "C" VscpRemoteTcpIf * WINAPI EXPORT vscphlp_getTcpIpInterface( long handle )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) NULL;

    return pvscpif->getTcpIpInterface();
}
*/

//-------------------------------------------------------------------------
//                                Variables 
//-------------------------------------------------------------------------


/*!
    \fn bool vscphlp_getVariableString( const char *pName, char *pValue ) 
    \brief Get variable value from string variable
    \param name of variable
    \param pointer to string that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableString( long handle, const char *pName, char *pValue ) 
{ 
    bool rv;

	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    if ( ( rv = pvscpif->getVariableString( name, &strValue ) ) ) {
        strcpy( pValue, strValue.ToAscii() );
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
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableString( long handle, const char *pName, char *pValue ) 
{ 
    bool rv;

	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    // Check pointers
    if ( NULL == pName ) return false;
    if ( NULL == pValue ) return false;

    wxString name = wxString::FromAscii( pName );
    wxString strValue = wxString::FromAscii( pValue );
    return pvscpif->setVariableString( name, strValue );

    return rv;
}

/*!
    \fn bool vscphlp_getVariableBool( const char *pName, bool *bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableBool( long handle, const char *pName, bool *bValue )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableBool( name, bValue ); 
};


/*!
    \fn bool vscphlp_setVariableBool( const char *pName, bool bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableBool( long handle, const char *pName, bool bValue )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableBool( name, bValue ); 
};


/*!
    \fn bool vscphlp_getVariableInt( const char *pName, int *value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableInt( long handle, const char *pName, int *value )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableInt( long handle, const char *pName, int value )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableLong( long handle, const char *pName, long *value )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableLong( long handle, const char *pName, long value )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableDouble( long handle, const char *pName, double *value )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableDouble( long handle, const char *pName, double value )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableMeasurement( long handle, const char *pName, char *pValue )
{ 
    bool rv;

	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    if ( rv = pvscpif->getVariableMeasurement( name, strValue ) ) {
        strcpy( pValue, strValue.ToAscii() );
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
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableMeasurement( long handle, const char *pName, char *pValue )
{ 
    bool rv;

	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
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
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableEvent( long handle, const char *pName, vscpEvent *pEvent )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableEvent( long handle, const char *pName, vscpEvent *pEvent )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableGUID( long handle, const char *pName, cguid& GUID )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableGUID( name, GUID ); 
}

/*!
    \fn bool vscphlp_setVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return true if the variable is of type string.
*/
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableGUID( long handle, const char *pName, cguid& GUID )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableGUID( name, GUID ); 
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
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableVSCPdata( long handle, const char *pName, uint16_t *psizeData, uint8_t *pData )
{ 
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->getVariableVSCPdata( name, psizeData, pData ); 
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
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableVSCPdata( long handle, const char *pName, uint16_t sizeData, uint8_t *pData )
{ 
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

    wxString name = wxString::FromAscii( pName );
    return pvscpif->setVariableVSCPdata( name, sizeData, pData ); 
}

/*!
    \fn bool vscphlp_getVariableVSCPclass( const char *pName, uint16_t *vscphlp_class )
    \brief Get variable value from class variable
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return true if the variable is of type string.
*/
extern "C"  BOOL WINAPI EXPORT vscphlp_getVariableVSCPclass( long handle, const char *pName, uint16_t *vscp_class )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C"  BOOL WINAPI EXPORT vscphlp_setVariableVSCPclass( long handle, const char *pName, uint16_t vscp_class )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_getVariableVSCPtype( long handle, const char *pName, uint8_t *vscp_type )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableVSCPtype( long handle, const char *pName, uint8_t vscp_type )
{
	VscpRemoteTcpIf *pvscpif = theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) FALSE;

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

extern "C" unsigned long WINAPI EXPORT vscphlp_readStringValue( const char * pStrValue )
{
    wxString strVal;
    strVal.FromAscii( pStrValue );

    return vscp_readStringValue( strVal );
}



/*!
    \fn unsigned char vscphlp_getVscpPriority( const vscpEvent *pEvent )
    \brief Get VSCP priority.
*/

extern "C" unsigned char WINAPI EXPORT vscphlp_getVscpPriority( const vscpEvent *pEvent )
{
    return vscp_getVscpPriority( pEvent );
}

/*!
    \fn void vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority )
    \brief Set VSCP priority.
*/

extern "C" void WINAPI EXPORT vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority )
{
    vscp_setVscpPriority( pEvent, priority );
}


/*!
    \fn vscphlp_getVSCPheadFromCANid( const unsigned long id )
    \brief Get the VSCP head from a CANAL message id (CAN id).
*/

extern "C" unsigned char WINAPI EXPORT vscphlp_getVSCPheadFromCANid( const unsigned long id )
{
    return  vscp_getVSCPheadFromCANid( id );
}

/*!
    \fn vscphlp_getVSCPclassFromCANid( const unsigned long id )
    \brief Get the VSCP class from a CANAL message id (CAN id).
*/

extern "C" unsigned short WINAPI EXPORT vscphlp_getVSCPclassFromCANid( const unsigned long id )
{
    return vscp_getVSCPclassFromCANid( id );
}


/*!
    \fn unsigned short vscphlp_getVSCPtypeFromCANid( const unsigned long id )
    \brief Get the VSCP type from a a CANAL message id (CAN id).
*/

extern "C" unsigned short WINAPI EXPORT vscphlp_getVSCPtypeFromCANid( const unsigned long id )
{
    return vscp_getVSCPtypeFromCANid( id );
}

/*!
    \fn unsigned short vscphlp_getVSCPnicknameFromCANid( const unsigned long id )
    \brief Get the VSCP nickname from a a CANAL message id (CAN id).
*/

extern "C" unsigned short WINAPI EXPORT vscphlp_getVSCPnicknameFromCANid( const unsigned long id )
{
    return vscp_getVSCPnicknameFromCANid( id );
}

/*!
    \fn unsigned long vscphlp_getCANidFromVSCPdata( const unsigned char priority, 
                                                    const unsigned short vscp_class, 
                                                    const unsigned short vscp_type )
    \brief Construct a CANAL id (CAN id ) from VSCP.
*/

extern "C" unsigned long WINAPI EXPORT vscphlp_getCANidFromVSCPdata( const unsigned char priority, 
                                                                    const unsigned short vscp_class, 
                                                                    const unsigned short vscp_type )
{
    return  vscp_getCANidFromVSCPdata( priority, vscp_class, vscp_type );
}

/*!
    \fn unsigned long vscphlp_getCANidFromVSCPevent( const vscpEvent *pEvent )
    \brief Get CANAL id (CAN id) from VSCP event.
*/

extern "C" unsigned long WINAPI EXPORT vscphlp_getCANidFromVSCPevent( const vscpEvent *pEvent )
{
    return vscp_getCANidFromVSCPevent( pEvent );
}

/*!
    \fn unsigned short vscphlp_calcCRC( vscpEvent *pEvent, short bSet )
    \brief Calculate VSCP crc.
*/

extern "C" unsigned short WINAPI EXPORT vscphlp_calcCRC( vscpEvent *pEvent, short bSet )
{
    return vscp_vscp_calc_crc( pEvent, bSet );
}


/*!
    \fn bool vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
    \brief Write GUID into VSCP event from string.
*/

extern "C" bool WINAPI EXPORT vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
{
    wxString strGUID = wxString::FromAscii( pGUID );
    return  vscp_getGuidFromString( pEvent, strGUID );
}

/*!
    \fn bool vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
    \brief Write GUID from string into array.
*/

extern "C" bool WINAPI EXPORT vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
{
    wxString strGUID = wxString::FromAscii( pStr );
    return vscp_getGuidFromStringToArray( pGUID, strGUID );
}

/*!
    \fn bool vscphlp_writeGuidToString( const vscpEvent *pEvent, char * pStr )
    \brief Write GUID froom VSCP event to string.
*/

extern "C" bool WINAPI EXPORT vscphlp_writeGuidToString( const vscpEvent *pEvent, char * pStr )
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

extern "C" bool WINAPI EXPORT vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                                                wxString& strGUID )
{
    return vscp_writeGuidToString4Rows( pEvent, strGUID );
}

/*!
    \fn bool vscphlp_writeGuidArrayToString( const unsigned char * pGUID, 
                                            wxString& strGUID )
    \brief Write GUID from byte array to string.
*/

extern "C" bool WINAPI EXPORT vscphlp_writeGuidArrayToString( const unsigned char * pGUID, 
                                                                wxString& strGUID )
{
    return vscp_writeGuidArrayToString( pGUID, strGUID );
}

/*!
    \fn bool vscphlp_isGUIDEmpty( unsigned char *pGUID )
    \brief Check if GUID is empty (all nills).
*/

extern "C" bool WINAPI EXPORT vscphlp_isGUIDEmpty( unsigned char *pGUID )
{
    return vscp_isGUIDEmpty( pGUID );
}

/*!
    \fn bool vscphlp_isSameGUID( const unsigned char *pGUID1, 
                                const unsigned char *pGUID2 )
    \brief Check if two GUID's is equal to each other.
*/

extern "C" bool WINAPI EXPORT vscphlp_isSameGUID( const unsigned char *pGUID1, 
                                                const unsigned char *pGUID2 )
{
    return vscp_isSameGUID( pGUID1, pGUID2 );
}

/*!
    \fn bool vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                    const vscpEvent *pEvent )
    \brief Convert VSCP standard event form to ex. form.
*/

extern "C" bool WINAPI EXPORT vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                                        const vscpEvent *pEvent )
{
    return vscp_convertVSCPtoEx( pEventEx, pEvent );
}

/*!
    \fn bool vscphlp_convertVSCPfromEx( vscpEvent *pEvent, 
                                        const vscpEventEx *pEventEx )
    \brief Convert VSCP ex. event form to standard form.
*/

extern "C" bool WINAPI EXPORT vscphlp_convertVSCPfromEx( vscpEvent *pEvent, 
                                                        const vscpEventEx *pEventEx )
{
    return vscp_convertVSCPfromEx( pEvent, pEventEx );
}


/*!
    \fn void vscphlp_deleteVSCPevent( vscpEvent *pEvent )
    \brief Delete VSCP event.
*/

extern "C" void WINAPI EXPORT vscphlp_deleteVSCPevent( vscpEvent *pEvent )
{
    return vscp_deleteVSCPevent( pEvent );
}

/*!
    \fn void vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx )
    \brief Delete VSCP event ex.
*/

extern "C" void WINAPI EXPORT vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx )
{
    return vscp_deleteVSCPeventEx( pEventEx );
}

/*!
    \fn void vscphlp_clearVSCPFilter( vscpEventFilter *pFilter )
    \brief Clear VSCP filter.
*/

extern "C" void WINAPI EXPORT vscphlp_clearVSCPFilter( vscpEventFilter *pFilter )
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

extern "C" bool WINAPI EXPORT vscphlp_readFilterFromString( vscpEventFilter *pFilter, wxString& strFilter )
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

extern "C" bool WINAPI EXPORT vscphlp_readMaskFromString( vscpEventFilter *pFilter, wxString& strMask )
{
	return vscp_readMaskFromString( pFilter, strMask );
}

/*!
    \fn bool vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                    const vscpEventFilter *pFilter )
    \brief Check VSCP filter condition.
*/

extern "C" bool WINAPI EXPORT vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                                    const vscpEventFilter *pFilter )
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

extern "C" bool WINAPI EXPORT vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                                            const canalMsg *pcanalMsg,
                                                            unsigned char *pGUID,
                                                            bool bCAN )
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

extern "C" bool WINAPI EXPORT vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                                            const vscpEvent *pvscpEvent )
{
    return  vscp_convertEventToCanal( pcanalMsg,
                                    pvscpEvent );
}


/*!
    \fn bool vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                            const vscpEventEx *pvscpEventEx )
    \brief Convert VSCP event ex. to CANAL message.
*/

extern "C" bool WINAPI EXPORT vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                                            const vscpEventEx *pvscpEventEx )
{
    return  vscp_convertEventExToCanal( pcanalMsg,
                                    pvscpEventEx );
}

/*!
    \fn unsigned long vscphlp_getTimeStamp( void )
    \brief Get VSCP timestamp.
*/

extern "C" unsigned long WINAPI EXPORT vscphlp_getTimeStamp( void )
{
    return vscp_makeTimeStamp();
}

/*!
    \fn bool vscphlp_copyVSCPEvent( vscpEvent *pEventTo, 
                                    const vscpEvent *pEventFrom )
    \brief Copy VSCP event.
*/

extern "C" bool WINAPI EXPORT vscphlp_copyVSCPEvent( vscpEvent *pEventTo, 
                                                    const vscpEvent *pEventFrom )
{
    return vscp_copyVSCPEvent( pEventTo, pEventFrom );
}

/*!
    \fn bool vscphlp_writeVscpDataToString( const vscpEvent *pEvent, 
                                            wxString& str, 
                                            bool bUseHtmlBreak )
    \brief Write VSCP data in readable form to a (multiline) string.
*/

extern "C" bool WINAPI EXPORT vscphlp_writeVscpDataToString( const vscpEvent *pEvent, 
                                                            wxString& str, 
                                                            bool bUseHtmlBreak )
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
extern "C" bool WINAPI EXPORT vscphlp_getVscpDataFromString( vscpEvent *pEvent, 
                                                                const wxString& str )
{
    return vscp_getVscpDataFromString( pEvent, str );
}

/*!
    \fn bool vscphlp_writeVscpEventToString( vscpEvent *pEvent, 
                                            char *p )
    \brief Write VSCP data to a string.
*/

extern "C" bool WINAPI EXPORT vscphlp_writeVscpEventToString( vscpEvent *pEvent, 
                                                                char *p )
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

extern "C" bool WINAPI EXPORT vscphlp_getVscpEventFromString( vscpEvent *pEvent, 
                                                                const char *p )
{
    wxString str = wxString::FromAscii( p );
    return vscp_getVscpEventFromString( pEvent, str ); 
}


