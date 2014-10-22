// vscphelperexport.cpp : Defines the initialization routines for the DLL.
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

#include "stdafx.h"
#include "vscphelperexport.h"

/*!
    \file VSCP helper dll.cpp
    \brief Helper dll for VSCP related tasks. 
    \details This code exports functionality to handle the CANAL interface
        and the TCP/IP interface plus a lot of extra functionality for VSCP related tasks.
    \author Ake Hedman <akhe@grodansparadis.com>, Grodans Paradis AB, Sweden
*/


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CVSCPhelperdllApp

BEGIN_MESSAGE_MAP( CVSCPhelperdllApp, CWinApp )
END_MESSAGE_MAP()


// CVSCPhelperdllApp construction

CVSCPhelperdllApp::CVSCPhelperdllApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}


// The one and only CVSCPhelperdllApp object

CVSCPhelperApp theApp;


// CVSCPhelperdllApp initialization

BOOL CVSCPhelperApp::InitInstance()
{
    CWinApp::InitInstance();

    if ( !AfxSocketInit() ) {
        AfxMessageBox( IDP_SOCKETS_INIT_FAILED );
        return FALSE;
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//                             V S C P -  A P I
///////////////////////////////////////////////////////////////////////////////


/*!	
    \fn void vscp_setInterfaceTcp( const char *pHost, 
                                    const short port,
                                    const char *pUsername,
                                    const char *pPassword )
    \brief Set interface to talk to tcp/ip interface of the daemon
    \param pHost Host to connect to
    \param port Port to use.
    \param pUserName Username to use as credentials for login.
    \param pPassword Password to use as credentials for login.
*/
extern "C" void WINAPI EXPORT vscp_setInterfaceTcp( const char *pHost, 
                                                        const short port,
                                                        const char *pUsername,
                                                        const char *pPassword )
{
    wxString strHost;
    strHost.FromAscii( pHost );

    wxString strUsername;
    strHost.FromAscii( pUsername );

    wxString strPassword;
    strHost.FromAscii( pPassword );

    theApp.m_vscpif.setInterface( strHost, 
                                    port,
                                    strUsername,
                                    strPassword );
}



/*!
    \fn void vscp_setInterfaceDll( const char *pName,
                                    const char *pPath,
                                    const char *pParameters,
                                    unsigned long flags,
                                    unsigned long filter,
                                    unsigned long mask )
    \brief Set the interface to talk to a Level I interface (previously called 
	CANAL interface).
*/
extern "C" void WINAPI EXPORT vscp_setInterfaceDll( const char *pName,
                                                        const char *pPath,
                                                        const char *pParameters,
                                                        unsigned long flags,
                                                        unsigned long filter,
                                                        unsigned long mask )
{
    wxString strName;
    strName.FromAscii( pName );

    wxString strPath;
    strPath.FromAscii( pPath );

    wxString strParameters;
    strParameters.FromAscii( pParameters );

    theApp.m_vscpif.setInterface( strName,
                                    strPath,
                                    strParameters,
                                    flags,
                                    filter,
                                    mask );
}


/*!
    \fn long vscp_doCmdOpen( const char *pInterface,
                                unsigned long flags )
	\return true if channel is open or false if error or the channel is
        already opened.							
    \brief Open interface
*/

extern "C" long WINAPI EXPORT vscp_doCmdOpen( const char *pInterface,
                                                unsigned long flags )
{
    wxString strInterface;
    strInterface.FromAscii( pInterface );

    return theApp.m_vscpif.doCmdOpen( strInterface, flags );
}

/*!
    \fn int vscp_doCmdClose( void )
    \brief Close interface
	\return true if the close was successful.
*/

extern "C" int WINAPI EXPORT vscp_doCmdClose( void )
{
    return theApp.m_vscpif.doCmdClose();
}

/*!
    \fn int vscp_doCmdNoop( void )
    \brief No operation (use for tests).
	\return true if success false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdNoop( void )
{
    return theApp.m_vscpif.doCmdClose();
}

/*!
    \fn long vscp_doCmdGetLevel( void )
    \brief Get driverlevel.
	\return CANAL_LEVEL_USES_TCPIP for a Level II driver and
	CANAL_LEVEL_STANDARD for a Level I driver. Will return 
	CANAL_ERROR_NOT_SUPPORTED if the driver reports some unknown 
	level.
*/

extern "C" unsigned long WINAPI EXPORT vscp_doCmdGetLevel( void )
{
    return theApp.m_vscpif.doCmdGetLevel();
}

/*!
    \fn int vscp_doCmdSendCanal( canalMsg *pMsg )
    \brief Send CANAL message.
	\return true if success false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdSendCanal( canalMsg *pMsg )
{
    return theApp.m_vscpif.doCmdSend( pMsg );
}

/*!
    \fn int vscp_doCmdSendEvent( const vscpEvent *pEvent )
    \brief Send VSCP event.
	\return true if success false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdSendEvent( const vscpEvent *pEvent )
{
    return theApp.m_vscpif.doCmdSend( pEvent );
}

/*!
    \fn int vscp_doCmdSendEventEx( const vscpEventEx *pEvent )
    \brief Send VSCP event (data is in event).
	\return true if success false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdSendEventEx( const vscpEventEx *pEvent )
{
    return theApp.m_vscpif.doCmdSend( pEvent );
}


/*!
    \fn int vscp_doCmdReceiveCanal( canalMsg *pMsg )
    \brief Receive a CANAL message.
	\return True if success false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdReceiveCanal( canalMsg *pMsg )
{
    return theApp.m_vscpif.doCmdReceive( pMsg );
}


/*!
    \fn int vscp_doCmdReceiveEvent( vscpEvent *pEvent )
    \brief Receive a VSCP event
	\return True if success false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdReceiveEvent( vscpEvent *pEvent )
{
    return theApp.m_vscpif.doCmdReceive( pEvent );
}

/*!
    \fn int vscp_doCmdReceiveEventEx( vscpEventEx *pEvent )
    \brief Receive a VSCP event.
	\return True if success false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdReceiveEventEx( vscpEventEx *pEvent )
{
    return theApp.m_vscpif.doCmdReceive( pEvent );
}

/*!
    \fn int vscp_doCmdDataAvailable( void )
    \brief Check if an event is available.
	\return The number of messages available or if negative
        an error code.
*/

extern "C" int WINAPI EXPORT vscp_doCmdDataAvailable( void )
{
    return theApp.m_vscpif.doCmdDataAvailable();
}

/*!
    \fn int vscp_doCmdStatus( canalStatus *pStatus )
    \brief Get CANAL status.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdStatus( canalStatus *pStatus )
{
    return theApp.m_vscpif.doCmdStatus( pStatus );
}

/*!
    \fn int vscp_doCmdStatistics( canalStatistics *pStatistics )
    \brief Get CANAL statistics.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdStatistics( canalStatistics *pStatistics )
{
    return theApp.m_vscpif.doCmdStatistics( pStatistics );
}

/*!
    \fn int vscp_doCmdFilter( unsigned long filter )
    \brief Set CANAL filter.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdFilter( unsigned long filter )
{
    return theApp.m_vscpif.doCmdFilter( filter );
}


/*!
    \fn int vscp_doCmdMask( unsigned long mask )
    \brief Set CANAL mask.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdMask( unsigned long mask )
{
    return theApp.m_vscpif.doCmdMask( mask );
}

/*!
    \fn int vscp_doCmdVscpFilter( const vscpEventFilter *pFilter )
    \brief Set VSCP filter/mask.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdVscpFilter( const vscpEventFilter *pFilter )
{
   return theApp.m_vscpif.doCmdVscpFilter( pFilter );
}

/*!
    \fn int vscp_doCmdBaudrate( unsigned long baudrate )
    \brief Set baudrate.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdBaudrate( unsigned long baudrate )
{
    return theApp.m_vscpif.doCmdBaudrate( baudrate );
}

/*!
    \fn long vscp_doCmdVersion( void  )
    \brief Get interface version.
	\return Return the interface version.
*/

extern "C" unsigned long WINAPI EXPORT vscp_doCmdVersion( void  )
{
    return theApp.m_vscpif.doCmdVersion();
}

/*!
    \fn long vscp_doCmdDLLVersion( void )
    \brief Get driver dll version.
	\return Return the interface dll version.
*/

extern "C" unsigned long WINAPI EXPORT vscp_doCmdDLLVersion( void )
{
    return theApp.m_vscpif.doCmdDLLVersion();
}

/*!
    \fn char * vscp_doCmdVendorString( void )
    \brief Get vendor string.
	\return Pointer to vendor string.
*/

extern "C"  const char * WINAPI EXPORT vscp_doCmdVendorString( void )
{
    return theApp.m_vscpif.doCmdVendorString();
}

/*!
    \fn char * vscp_doCmdGetDriverInfo( void )
    \brief Get driver information.
	\return Pointer to driver information string.
*/

extern "C"  const char * WINAPI EXPORT vscp_doCmdGetDriverInfo( void )
{
    return theApp.m_vscpif.doCmdGetDriverInfo();
}

/*!
    \fn int vscp_getDeviceType( void )
    \brief Get the type of interface that is active.
	\return USE_DLL_INTERFACE if direct DLL interface active.
            USE_TCPIP_INTERFACE	if TCP/IP interface active.
*/

extern "C" int WINAPI EXPORT vscp_getDeviceType( void )
{
    return theApp.m_vscpif.getDeviceType();
}

/*!
    \fn bool vscp_isOpen( void )
    \brief Check if communication channel is open.
	\return True of open, false if not.
*/

extern "C" bool WINAPI EXPORT vscp_isOpen( void )
{
    return theApp.m_vscpif.isOpen();
}

/*!
    \fn int vscp_doCmdShutDown( void )
    \brief Shutdown daemon.
	\return True if success, false if not.
*/

extern "C" int WINAPI EXPORT vscp_doCmdShutDown( void )
{
    return theApp.m_vscpif.doCmdShutDown();
}

/*!
    \fn VscpTcpIf  vscp_getTcpIpInterface( void )
    \brief Get pointer to TCP/IP interface object.
	\return Pointer to the TCP/IP interface object if OK, 
		NULL if failure.
*/

extern "C" VscpTcpIf * WINAPI EXPORT vscp_getTcpIpInterface( void )
{
    return theApp.m_vscpif.getTcpIpInterface();
}






////////////////////////////////////////////////////////////////////////////////
//                              H E L P E R S
////////////////////////////////////////////////////////////////////////////////



/*!
    \fn long vscp_readStringValue( const char * pStrValue )
    \brief Read a value (decimal or hex) from a string.
	\return The converted number.
*/

extern "C" unsigned long WINAPI EXPORT vscp_readStringValue( const char * pStrValue )
{
    wxString strVal;
    strVal.FromAscii( pStrValue );

    return readStringValue( strVal );
}



/*!
    \fn unsigned char vscp_getVscpPriority( const vscpEvent *pEvent )
    \brief Get VSCP priority.
*/

extern "C" unsigned char WINAPI EXPORT vscp_getVscpPriority( const vscpEvent *pEvent )
{
    return getVscpPriority( pEvent );
}

/*!
    \fn void vscp_setVscpPriority( vscpEvent *pEvent, unsigned char priority )
    \brief Set VSCP priority.
*/

extern "C" void WINAPI EXPORT vscp_setVscpPriority( vscpEvent *pEvent, unsigned char priority )
{
    setVscpPriority( pEvent, priority );
}


/*!
    \fn vscp_getVSCPheadFromCANid( const unsigned long id )
    \brief Get the VSCP head from a CANAL message id (CAN id).
*/

extern "C" unsigned char WINAPI EXPORT vscp_getVSCPheadFromCANid( const unsigned long id )
{
    return  getVSCPheadFromCANid( id );
}

/*!
    \fn vscp_getVSCPclassFromCANid( const unsigned long id )
    \brief Get the VSCP class from a CANAL message id (CAN id).
*/

extern "C" unsigned short WINAPI EXPORT vscp_getVSCPclassFromCANid( const unsigned long id )
{
    return getVSCPclassFromCANid( id );
}


/*!
    \fn unsigned short vscp_getVSCPtypeFromCANid( const unsigned long id )
    \brief Get the VSCP type from a a CANAL message id (CAN id).
*/

extern "C" unsigned short WINAPI EXPORT vscp_getVSCPtypeFromCANid( const unsigned long id )
{
    return getVSCPtypeFromCANid( id );
}

/*!
    \fn unsigned short vscp_getVSCPnicknameFromCANid( const unsigned long id )
    \brief Get the VSCP nickname from a a CANAL message id (CAN id).
*/

extern "C" unsigned short WINAPI EXPORT vscp_getVSCPnicknameFromCANid( const unsigned long id )
{
    return getVSCPnicknameFromCANid( id );
}

/*!
    \fn unsigned long vscp_getCANidFromVSCPdata( const unsigned char priority, 
                                                    const unsigned short vscp_class, 
                                                    const unsigned short vscp_type )
    \brief Construct a CANAL id (CAN id ) from VSCP.
*/

extern "C" unsigned long WINAPI EXPORT vscp_getCANidFromVSCPdata( const unsigned char priority, 
                                                                    const unsigned short vscp_class, 
                                                                    const unsigned short vscp_type )
{
    return  getCANidFromVSCPdata( priority, vscp_class, vscp_type );
}

/*!
    \fn unsigned long vscp_getCANidFromVSCPevent( const vscpEvent *pEvent )
    \brief Get CANAL id (CAN id) from VSCP event.
*/

extern "C" unsigned long WINAPI EXPORT vscp_getCANidFromVSCPevent( const vscpEvent *pEvent )
{
    return getCANidFromVSCPevent( pEvent );
}

/*!
    \fn unsigned short vscp_calcCRC( vscpEvent *pEvent, short bSet )
    \brief Calculate VSCP crc.
*/

extern "C" unsigned short WINAPI EXPORT vscp_calcCRC( vscpEvent *pEvent, short bSet )
{
    return vscp_calc_crc( pEvent, bSet );
}


/*!
    \fn unsigned short vscp_getCrcOfGuidFromArray( const unsigned char * pGUID )
    \brief Calculate 8-bit crc for GUID in array
	\param Pointer to GUID array
 	\return 8-bit crc of GUID.
*/

extern "C" bool WINAPI EXPORT vscp_getCrcOfGuidFromArray( const unsigned char * pGUID )
{
    return calcCRC4GUIDArray( pGUID );
}


/*!
    \fn bool vscp_getGuidFromString( const char * strGUID )
    \brief Calculate 8-bit crc for GUID in array
    \param Pointer to GUID string.
 	\return 8-bit crc of GUID.
*/

extern "C" bool WINAPI EXPORT vscp_getCrcOfGuidFromString( const char * strGUID )
{
    wxString wxGUID = wxString::FromAscii( strGUID );
    return  calcCRC4GUIDString( wxGUID );
}


/*!
    \fn bool vscp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
    \brief Write GUID into VSCP event from string.
*/

extern "C" bool WINAPI EXPORT vscp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
{
    wxString strGUID = wxString::FromAscii( pGUID );
    return  getGuidFromString( pEvent, strGUID );
}

/*!
    \fn bool vscp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
    \brief Write GUID from string into array.
*/

extern "C" bool WINAPI EXPORT vscp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
{
    wxString strGUID = wxString::FromAscii( pStr );
    return getGuidFromStringToArray( pGUID, strGUID );
}

/*!
    \fn bool vscp_writeGuidToString( const vscpEvent *pEvent, char * pStr )
    \brief Write GUID froom VSCP event to string.
*/

extern "C" bool WINAPI EXPORT vscp_writeGuidToString( const vscpEvent *pEvent, char * pStr )
{
    bool rv;

    wxString strGUID;
    rv = writeGuidToString( pEvent, strGUID );
    strcpy( pStr, strGUID.ToAscii() );
    return rv;
}


/*!
    \fn bool vscp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                            wxString& strGUID )
    \brief Write GUID from VSCP event to string with four bytes on each
    row seperated by \r\n. 
*/

extern "C" bool WINAPI EXPORT vscp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                                                wxString& strGUID )
{
    return writeGuidToString4Rows( pEvent, strGUID );
}

/*!
    \fn bool vscp_writeGuidArrayToString( const unsigned char * pGUID, 
                                            wxString& strGUID )
    \brief Write GUID from byte array to string.
*/

extern "C" bool WINAPI EXPORT vscp_writeGuidArrayToString( const unsigned char * pGUID, 
                                                                wxString& strGUID )
{
    return writeGuidArrayToString( pGUID, strGUID );
}

/*!
    \fn bool vscp_isGUIDEmpty( unsigned char *pGUID )
    \brief Check if GUID is empty (all nills).
*/

extern "C" bool WINAPI EXPORT vscp_isGUIDEmpty( unsigned char *pGUID )
{
    return isGUIDEmpty( pGUID );
}

/*!
    \fn bool vscp_isSameGUID( const unsigned char *pGUID1, 
                                const unsigned char *pGUID2 )
    \brief Check if two GUID's is equal to each other.
*/

extern "C" bool WINAPI EXPORT vscp_isSameGUID( const unsigned char *pGUID1, 
                                                const unsigned char *pGUID2 )
{
    return isSameGUID( pGUID1, pGUID2 );
}

/*!
    \fn bool vscp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                    const vscpEvent *pEvent )
    \brief Convert VSCP standard event form to ex. form.
*/

extern "C" bool WINAPI EXPORT vscp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                                        const vscpEvent *pEvent )
{
    return convertVSCPtoEx( pEventEx, pEvent );
}

/*!
    \fn bool vscp_convertVSCPfromEx( vscpEvent *pEvent, 
                                        const vscpEventEx *pEventEx )
    \brief Convert VSCP ex. event form to standard form.
*/

extern "C" bool WINAPI EXPORT vscp_convertVSCPfromEx( vscpEvent *pEvent, 
                                                        const vscpEventEx *pEventEx )
{
    return convertVSCPfromEx( pEvent, pEventEx );
}


/*!
    \fn void vscp_deleteVSCPevent( vscpEvent *pEvent )
    \brief Delete VSCP event.
*/

extern "C" void WINAPI EXPORT vscp_deleteVSCPevent( vscpEvent *pEvent )
{
    return deleteVSCPevent( pEvent );
}

/*!
    \fn void vscp_deleteVSCPeventEx( vscpEventEx *pEventEx )
    \brief Delete VSCP event ex.
*/

extern "C" void WINAPI EXPORT vscp_deleteVSCPeventEx( vscpEventEx *pEventEx )
{
    return deleteVSCPeventEx( pEventEx );
}

/*!
    \fn void vscp_clearVSCPFilter( vscpEventFilter *pFilter )
    \brief Clear VSCP filter.
*/

extern "C" void WINAPI EXPORT vscp_clearVSCPFilter( vscpEventFilter *pFilter )
{
    return clearVSCPFilter( pFilter );
}

/*!
    \fn bool readFilterFromString( vscpEventFilter *pFilter, wxString& strFilter )
    \brief Read a filter from a string
	\param pFilter Filter structure to write filter to.
	\param strFilter Filter in string form 
				filter-priority, filter-class, filter-type, filter-GUID
	\return true on success, fals eon failure.
*/

extern "C" bool WINAPI EXPORT vscp_readFilterFromString( vscpEventFilter *pFilter, wxString& strFilter )
{
	return readFilterFromString( pFilter, strFilter );
}

/*!
    \fn bool readMaskFromString( vscpEventFilter *pFilter, wxString& strMask )
    \brief Read a mask from a string
	\param pFilter Filter structure to write mask to.
	\param strMask Mask in string form 
				mask-priority, mask-class, mask-type, mask-GUID
	\return true on success, fals eon failure.
*/

extern "C" bool WINAPI EXPORT vscp_readMaskFromString( vscpEventFilter *pFilter, wxString& strMask )
{
	return readMaskFromString( pFilter, strMask );
}

/*!
    \fn bool vscp_doLevel2Filter( const vscpEvent *pEvent,
                                    const vscpEventFilter *pFilter )
    \brief Check VSCP filter condition.
*/

extern "C" bool WINAPI EXPORT vscp_doLevel2Filter( const vscpEvent *pEvent,
                                                    const vscpEventFilter *pFilter )
{
    return  doLevel2Filter( pEvent, pFilter );
}


/*!
    \fn bool vscp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                            const canalMsg *pcanalMsg,
                                            unsigned char *pGUID,
                                            bool bCAN )
    \brief Convert CANAL message to VSCP event.
*/

extern "C" bool WINAPI EXPORT vscp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                                            const canalMsg *pcanalMsg,
                                                            unsigned char *pGUID,
                                                            bool bCAN )
{
    return convertCanalToEvent( pvscpEvent,
                                    pcanalMsg,
                                    pGUID,
                                    bCAN );
}


/*!
    \fn bool vscp_convertEventToCanal( canalMsg *pcanalMsg,
                                        const vscpEvent *pvscpEvent )
    \brief Convert VSCP event to CANAL message.
*/

extern "C" bool WINAPI EXPORT vscp_convertEventToCanal( canalMsg *pcanalMsg,
                                                            const vscpEvent *pvscpEvent )
{
    return  convertEventToCanal( pcanalMsg,
                                    pvscpEvent );
}


/*!
    \fn bool vscp_convertEventExToCanal( canalMsg *pcanalMsg,
                                            const vscpEventEx *pvscpEventEx )
    \brief Convert VSCP event ex. to CANAL message.
*/

extern "C" bool WINAPI EXPORT vscp_convertEventExToCanal( canalMsg *pcanalMsg,
                                                            const vscpEventEx *pvscpEventEx )
{
    return  convertEventExToCanal( pcanalMsg,
                                    pvscpEventEx );
}

/*!
    \fn unsigned long vscp_getTimeStamp( void )
    \brief Get VSCP timestamp.
*/

extern "C" unsigned long WINAPI EXPORT vscp_getTimeStamp( void )
{
    return makeTimeStamp();
}

/*!
    \fn bool vscp_copyVSCPEvent( vscpEvent *pEventTo, 
                                    const vscpEvent *pEventFrom )
    \brief Copy VSCP event.
*/

extern "C" bool WINAPI EXPORT vscp_copyVSCPEvent( vscpEvent *pEventTo, 
                                                    const vscpEvent *pEventFrom )
{
    return copyVSCPEvent( pEventTo, pEventFrom );
}

/*!
    \fn bool vscp_writeVscpDataToString( const vscpEvent *pEvent, 
                                            wxString& str, 
                                            bool bUseHtmlBreak )
    \brief Write VSCP data in readable form to a (multiline) string.
*/

extern "C" bool WINAPI EXPORT vscp_writeVscpDataToString( const vscpEvent *pEvent, 
                                                            wxString& str, 
                                                            bool bUseHtmlBreak )
{
    return writeVscpDataToString( pEvent, 
                                    str, 
                                    bUseHtmlBreak );
}


/*!
    \fn bool vscp_getVscpDataFromString( vscpEvent *pEvent, 
                                            const wxString& str )
    \brief Set data in VSCP event from a string.
*/
extern "C" bool WINAPI EXPORT vscp_getVscpDataFromString( vscpEvent *pEvent, 
                                                                const wxString& str )
{
    return getVscpDataFromString( pEvent, str );
}

/*!
    \fn bool vscp_writeVscpEventToString( vscpEvent *pEvent, 
                                            char *p )
    \brief Write VSCP data to a string.
*/

extern "C" bool WINAPI EXPORT vscp_writeVscpEventToString( vscpEvent *pEvent, 
                                                                char *p )
{
    bool rv;

    wxString str = wxString::FromAscii( p );
    if ( ( rv =  writeVscpEventToString( pEvent, str ) ) ) {
        strcpy( p, str.ToAscii() );
    }
    return rv;
}

/*!
    \fn bool vscp_getVscpEventFromString( vscpEvent *pEvent, 
                                            const char *p )
    \brief Get VSCP event from string.
*/

extern "C" bool WINAPI EXPORT vscp_getVscpEventFromString( vscpEvent *pEvent, 
                                                                const char *p )
{
    wxString str = wxString::FromAscii( p );
    return getVscpEventFromString( pEvent, str ); 
}


//-------------------------------------------------------------------------


/*!
    \fn bool vscp_getVariableString( const char *pName, char *pValue ) 
    \brief Get variable value from string variable
    \param name of variable
    \param pointer to string that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableString( const char *pName, char *pValue ) 
{ 
    bool rv;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    if ( ( rv = theApp.m_vscpif.getVariableString( name, &strValue ) ) ) {
        strcpy( pValue, strValue.ToAscii() );
    }

    return rv;
}

/*!
    \fn bool vscp_setVariableString( const char *pName, char *pValue ) 
    \brief set string variable from a pointer to a string
    \param name of variable
    \param pointer to string that contains the string.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableString( const char *pName, char *pValue ) 
{ 
    bool rv;

    // Check pointers
    if ( NULL == pName ) return false;
    if ( NULL == pValue ) return false;

    wxString name = wxString::FromAscii( pName );
    wxString strValue = wxString::FromAscii( pValue );
    return theApp.m_vscpif.setVariableString( name, strValue );

    return rv;
}

/*!
    \fn bool vscp_getVariableBool( const char *pName, bool *bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableBool( const char *pName, bool *bValue )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableBool( name, bValue ); 
};


/*!
    \fn bool vscp_setVariableBool( const char *pName, bool bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableBool( const char *pName, bool bValue )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableBool( name, bValue ); 
};


/*!
    \fn bool vscp_getVariableInt( const char *pName, int *value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableInt( const char *pName, int *value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableInt( name, value ); 
};


/*!
    \fn bool vscp_setVariableInt( const char *pName, int value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableInt( const char *pName, int value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableInt( name, value ); 
};

/*!
    \fn bool vscp_getVariableLong( const char *pName, long *value )
    \brief Get variable value from long variable
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableLong( const char *pName, long *value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableLong( name, value ); 
};

/*!
    \fn bool vscp_setVariableLong( const char *pName, long value )
    \brief Get variable value from long variable
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableLong( const char *pName, long value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableLong( name, value ); 
};

/*!
    \fn bool vscp_getVariableDouble( const char *pName, double *value )
    \brief Get variable value from double variable
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableDouble( const char *pName, double *value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableDouble( name, value ); 
};

/*!
    \fn bool vscp_setVariableDouble( const char *pName, double value )
    \brief Get variable value from double variable
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableDouble( const char *pName, double value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableDouble( name, value ); 
};

/*!
    \fn bool vscp_getVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param name of variable
    \param String that get that get the 
    value of the measurement.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableMeasurement( const char *pName, char *pValue )
{ 
    bool rv;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    if ( rv = theApp.m_vscpif.getVariableMeasurement( name, strValue ) ) {
        strcpy( pValue, strValue.ToAscii() );
    }

    return rv;
};

/*!
    \fn bool vscp_setVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param name of variable
    \param String that get that get the 
    value of the measurement.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableMeasurement( const char *pName, char *pValue )
{ 
    bool rv;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    return theApp.m_vscpif.setVariableMeasurement( name, strValue );

    return rv;
};

/*!
    \fn bool vscp_getVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableEvent( const char *pName, vscpEvent *pEvent )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableEvent( name, pEvent ); 
}

/*!
    \fn bool vscp_setVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableEvent( const char *pName, vscpEvent *pEvent )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableEvent( name, pEvent ); 
}

/*!
    \fn bool vscp_getVariableEventEx( const char *pName, vscpEventEx *pEvent )
    \brief Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableEventEx( const char *pName, vscpEventEx *pEvent )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableEventEx( name, pEvent ); 
}

/*!
    \fn bool vscp_setVariableEventEx( const char *pName, vscpEventEx *pEvent )
    \brief Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableEventEx( const char *pName, vscpEventEx *pEvent )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableEventEx( name, pEvent ); 
}

/*!
    \fn bool vscp_getVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableGUID( const char *pName, cguid& GUID )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableGUID( name, GUID ); 
}

/*!
    \fn bool vscp_setVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableGUID( const char *pName, cguid& GUID )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableGUID( name, GUID ); 
}

/*!
    \fn bool vscp_getVariableVSCPdata( const char *pName, uint16_t *psizeData, uint8_t *pData )
    \brief Get variable value from VSCP data variable
    \param name of variable
    \param Pointer to variable that will hold the size of the data array
    \param pointer to VSCP data array variable (unsigned char [8] ) that get the 
    value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableVSCPdata( const char *pName, uint16_t *psizeData, uint8_t *pData )
{ 
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableVSCPdata( name, psizeData, pData ); 
}

/*!
    \fn bool vscp_setVariableVSCPdata( const char *pName, uint16_t sizeData, uint8_t *pData )
    \brief Get variable value from VSCP data variable
    \param name of variable
    \param Pointer to variable that will hold the size of the data array
    \param pointer to VSCP data array variable (unsigned char [8] ) that get the 
    value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableVSCPdata( const char *pName, uint16_t sizeData, uint8_t *pData )
{ 
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableVSCPdata( name, sizeData, pData ); 
}

/*!
    \fn bool vscp_getVariableVSCPclass( const char *pName, uint16_t *vscp_class )
    \brief Get variable value from class variable
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return true if the variable is of type string.
*/
extern "C"  bool WINAPI EXPORT vscp_getVariableVSCPclass( const char *pName, uint16_t *vscp_class )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableVSCPclass( name, vscp_class ); 
}

/*!
    \fn bool vscp_setVariableVSCPclass( const char *pName, uint16_t vscp_class )
    \brief Get variable value from class variable
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return true if the variable is of type string.
*/
extern "C"  bool WINAPI EXPORT vscp_setVariableVSCPclass( const char *pName, uint16_t vscp_class )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableVSCPclass( name, vscp_class ); 
}

/*!
    \fn bool vscp_getVariableVSCPtype( const char *pName, uint8_t *vscp_type )
    \brief Get variable value from type variable
    \param name of variable
    \param pointer to int that get the value of the type variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_getVariableVSCPtype( const char *pName, uint8_t *vscp_type )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableVSCPtype( name, vscp_type ); 
}

/*!
    \fn bool WINAPI EXPORT vscp_setVariableVSCPtype( const char *pName, uint8_t vscp_type )
    \brief Get variable value from type variable
    \param name of variable
    \param pointer to int that get the value of the type variable.
    \return true if the variable is of type string.
*/
extern "C" bool WINAPI EXPORT vscp_setVariableVSCPtype( const char *pName, uint8_t vscp_type )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableVSCPtype( name, vscp_type ); 
}
