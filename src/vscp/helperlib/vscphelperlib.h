///////////////////////////////////////////////////////////////////////////////
// vscphelperlib.h
//
// Interface to the vscphelperlib.dll
// Documentation is here http://www.vscp.org/docs/vscphelper/doku.php?id=non_graphic_lib_api
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

#include "../common/vscp.h"


#ifdef WIN32

// * * * * *  W I N D O W S  * * * * *
extern "C" long WINAPI vscphlp_newSession(void);
extern "C" void WINAPI vscphlp_closeSession(long handle);
extern "C" void WINAPI vscphlp_setResponseTimeout(long handle, 
                                                    unsigned char timeout );
extern "C" int WINAPI vscphlp_isConnected(const long handle);
extern "C" long WINAPI vscphlp_openInterface( long handle,
						                        const char *pInterface,
                                                unsigned long flags );
extern "C" long WINAPI vscphlp_open( long handle,
                                        const char *pHostname, 
                                        const char *pUsername, 
                                        const char *pPassword );
extern "C" int WINAPI vscphlp_close( long handle );
extern "C" int WINAPI vscphlp_noop( long handle );
extern "C" unsigned long WINAPI EXPORT vscphlp_getLevel( long handle );
extern "C" int WINAPI EXPORT vscphlp_clearInQueue( long handle );
extern "C" int WINAPI EXPORT vscphlp_sendEvent( long handle,
													    const vscpEvent *pEvent );
extern "C" int WINAPI EXPORT vscphlp_sendEventEx( long handle,
														const vscpEventEx *pEvent );
extern "C" int WINAPI EXPORT vscphlp_receiveEvent( long handle,
														vscpEvent *pEvent );
extern "C" int WINAPI EXPORT vscphlp_receiveEventEx( long handle,
														vscpEventEx *pEvent );
extern "C" int WINAPI EXPORT vscphlp_isDataAvailable( long handle );
extern "C" int WINAPI EXPORT vscphlp_getStatus( long handle,
													canalStatus *pStatus );
extern "C" int WINAPI EXPORT vscphlp_getStatistics( long handle,
														canalStatistics *pStatistics );
extern "C" int WINAPI EXPORT vscphlp_setFilter( long handle,
														const vscpEventFilter *pFilter );
extern "C" unsigned long WINAPI EXPORT vscphlp_getVersion( long handle );
extern "C" unsigned long WINAPI EXPORT vscphlp_getDLLVersion( long handle );
extern "C"  const char * WINAPI EXPORT vscphlp_getVendorString( long handle );
extern "C"  const char * WINAPI EXPORT vscphlp_getDriverInfo( long handle );
extern "C" int WINAPI EXPORT vscphlp_shutDownServer( long handle );
extern "C" int WINAPI EXPORT vscphlp_enterReceiveLoop(const long handle);

//-------------------------------------------------------------------------
//                                Variables 
//-------------------------------------------------------------------------

extern "C" int WINAPI EXPORT vscphlp_getVariableString( long handle, const char *pName, char *pValue );
extern "C" int WINAPI EXPORT vscphlp_setVariableString( long handle, const char *pName, char *pValue );

extern "C" int WINAPI EXPORT vscphlp_getVariableBool( long handle, const char *pName, bool *bValue );
extern "C" int WINAPI EXPORT vscphlp_setVariableBool( long handle, const char *pName, bool *bValue );

extern "C" int WINAPI EXPORT vscphlp_getVariableInt( long handle, const char *pName, int *value );
extern "C" int WINAPI EXPORT vscphlp_setVariableInt( long handle, const char *pName, int *value );

extern "C" int WINAPI EXPORT vscphlp_getVariableLong( long handle, const char *pName, long *value );
extern "C" int WINAPI EXPORT vscphlp_setVariableLong( long handle, const char *pName, long *value );

extern "C" int WINAPI EXPORT vscphlp_getVariableDouble( long handle, const char *pName, double *value );
extern "C" int WINAPI EXPORT vscphlp_setVariableDouble( long handle, const char *pName, double *value );

extern "C" int WINAPI EXPORT vscphlp_getVariableMeasurement( long handle, const char *pName, char *pValue );
extern "C" int WINAPI EXPORT vscphlp_setVariableMeasurement( long handle, const char *pName, char *pValue );

extern "C" int WINAPI EXPORT vscphlp_getVariableEvent( long handle, const char *pName, vscpEvent *pEvent );
extern "C" int WINAPI EXPORT vscphlp_setVariableEvent( long handle, const char *pName, vscpEvent *pEvent );

extern "C" int WINAPI EXPORT vscphlp_getVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );
extern "C" int WINAPI EXPORT vscphlp_setVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );

extern "C" int WINAPI EXPORT vscphlp_getVariableGUID( long handle, const char *pName, const char *pGUID );
extern "C" int WINAPI EXPORT vscphlp_setVariableGUID( long handle, const char *pName, const char *pGUID );

extern "C" int WINAPI EXPORT vscphlp_getVariableVSCPdata( long handle, const char *pName, uint16_t *psizeData, uint8_t *pData );
extern "C" int WINAPI EXPORT vscphlp_setVariableVSCPdata( long handle, const char *pName, uint16_t *psizeData, uint8_t *pData );

extern "C" int WINAPI EXPORT vscphlp_getVariableVSCPclass( long handle, const char *pName, uint16_t *vscp_class );
extern "C" int WINAPI EXPORT vscphlp_setVariableVSCPclass( long handle, const char *pName, uint16_t *vscp_class );

extern "C" int WINAPI EXPORT vscphlp_getVariableVSCPtype( long handle, const char *pName, uint8_t *vscp_type );
extern "C" int WINAPI EXPORT vscphlp_setVariableVSCPtype( long handle, const char *pName, uint8_t *vscp_type );

#else

// * * * * *  L I N U X  * * * * *
extern "C" long vscphlp_newSession(void);
extern "C" void vscphlp_closeSession(long handle);
extern "C" void vscphlp_setResponseTimeout(long handle, 
                                              unsigned char timeout );
extern "C" int vscphlp_isConnected(const long handle)
extern "C" long vscphlp_OpenInterface( long handle,
						                const char *pInterface,
                                        unsigned long flags );
extern "C" long vscphlp_Open( long handle,
                                const char *pHostname, 
                                const char *pUsername, 
                                const char *pPassword );
extern "C" int vscphlp_Close( long handle );
extern "C" int vscphlp_Noop( long handle );
extern "C" unsigned long vscphlp_GetLevel( long handle );
extern "C" int vscphlp_ClearInQueue( long handle );
extern "C" int vscphlp_SendEvent( long handle,
                                      const vscpEvent *pEvent );
extern "C" int vscphlp_SendEventEx( long handle,
                                        const vscpEventEx *pEvent );
extern "C" int vscphlp_ReceiveEvent( long handle,
										vscpEvent *pEvent );
extern "C" int vscphlp_ReceiveEventEx( long handle,
                                        vscpEventEx *pEvent );
extern "C" int vscphlp_isDataAvailable( long handle );
extern "C" int vscphlp_GetStatus( long handle,
                                     canalStatus *pStatus );
extern "C" int vscphlp_getStatistics( long handle,
                                     canalStatistics *pStatistics );
extern "C" int vscphlp_SetFilter( long handle,
                                     const vscpEventFilter *pFilter )
extern "C" unsigned long vscphlp_getVersion( long handle );
extern "C" unsigned long vscphlp_GetDLLVersion( long handle );
extern "C"  const char * vscphlp_getVendorString( long handle );
extern "C"  const char * vscphlp_GetDriverInfo( long handle );
extern "C" int vscphlp_ServerShutDown( long handle );
extern "C" int vscphlp_enterReceiveLoop(const long handle);

//-------------------------------------------------------------------------
//                                Variables 
//-------------------------------------------------------------------------

extern "C" BOOL vscphlp_getVariableString( long handle, const char *pName, char *pValue );
extern "C" BOOL vscphlp_setVariableString( long handle, const char *pName, char *pValue );

extern "C" BOOL WINAPI EXPORT vscphlp_getVariableBool( long handle, const char *pName, bool *bValue );
extern "C" BOOL WINAPI EXPORT vscphlp_setVariableBool( long handle, const char *pName, bool *bValue )

extern "C" BOOL vscphlp_getVariableInt( long handle, const char *pName, int *value );
extern "C" BOOL vscphlp_setVariableInt( long handle, const char *pName, int *value );

extern "C" BOOL vscphlp_getVariableLong( long handle, const char *pName, long *value );
extern "C" BOOL vscphlp_setVariableLong( long handle, const char *pName, long *value );

extern "C" BOOL vscphlp_getVariableDouble( long handle, const char *pName, double *value );
extern "C" BOOL vscphlp_setVariableDouble( long handle, const char *pName, double *value );

extern "C" BOOL vscphlp_getVariableMeasurement( long handle, const char *pName, char *pValue );
extern "C" BOOL vscphlp_setVariableMeasurement( long handle, const char *pName, char *pValue );

extern "C" BOOL vscphlp_getVariableEvent( long handle, const char *pName, vscpEvent *pEvent );
extern "C" BOOL vscphlp_setVariableEvent( long handle, const char *pName, vscpEvent *pEvent );

extern "C" BOOL vscphlp_getVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );
extern "C" BOOL vscphlp_setVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );

extern "C" BOOL vscphlp_getVariableGUID( long handle, const char *pName, const char *pGUID );
extern "C" BOOL vscphlp_setVariableGUID( long handle, const char *pName, const char *pGUID );

extern "C" BOOL vscphlp_getVariableVSCPdata( long handle, const char *pName, uint16_t *psizeData, uint8_t *pData );
extern "C" BOOL vscphlp_setVariableVSCPdata( long handle, const char *pName, uint16_t *psizeData, uint8_t *pData );

extern "C"  BOOL vscphlp_getVariableVSCPclass( long handle, const char *pName, uint16_t *vscp_class );
extern "C"  BOOL vscphlp_setVariableVSCPclass( long handle, const char *pName, uint16_t *vscp_class );

extern "C" BOOL vscphlp_getVariableVSCPtype( long handle, const char *pName, uint8_t *vscp_type );
extern "C" BOOL vscphlp_setVariableVSCPtype( long handle, const char *pName, uint8_t *vscp_type );

#endif




/////////////////////////////////////////////////////////////////////////////////////////


/*!
    \fn int vscp_doCmdNoop( void )
    \brief No operation (use for tests).
	\return true if success false if not.
*/

extern "C" int WINAPI vscp_doCmdNoop( long handle );


/*!
    \fn long vscp_doCmdGetLevel( void )
    \brief Get driverlevel.
	\return CANAL_LEVEL_USES_TCPIP for a Level II driver and
	CANAL_LEVEL_STANDARD for a Level I driver. Will return 
	CANAL_ERROR_NOT_SUPPORTED if the driver reports some unknown 
	level.
*/

extern "C" unsigned long WINAPI vscp_doCmdGetLevel( long handle );  


/*!
    \fn int vscp_doCmdSendCanal( canalMsg *pMsg )
    \brief Send CANAL message.
	\return true if success false if not.
*/

extern "C" int WINAPI vscp_doCmdSendCanal( long handle,
													canalMsg *pMsg );


/*!
    \fn int vscp_doCmdSendEvent( const vscpEvent *pEvent )
    \brief Send VSCP event.
	\return true if success false if not.
*/

extern "C" int WINAPI vscp_doCmdSendEvent( long handle,
													const vscpEvent *pEvent );

/*!
    \fn int vscp_doCmdSendEventEx( const vscpEventEx *pEvent )
    \brief Send VSCP event (data is in event).
	\return true if success false if not.
*/

extern "C" int WINAPI vscp_doCmdSendEventEx( long handle,
														const vscpEventEx *pEvent );

/*!
    \fn int vscp_doCmdReceiveCanal( canalMsg *pMsg )
    \brief Receive a CANAL message.
	\return True if success false if not.
*/

extern "C" int WINAPI vscp_doCmdReceiveCanal( long handle,
														canalMsg *pMsg );

/*!
    \fn int vscp_doCmdReceiveEvent( vscpEvent *pEvent )
    \brief Receive a VSCP event
	\return True if success false if not.
*/

extern "C" int WINAPI vscp_doCmdReceiveEvent( long handle,
														vscpEvent *pEvent );

/*!
    \fn int vscp_doCmdReceiveEventEx( vscpEventEx *pEvent )
    \brief Receive a VSCP event.
	\return True if success false if not.
*/

extern "C" int WINAPI vscp_doCmdReceiveEventEx( long handle,
														vscpEventEx *pEvent );

/*!
    \fn int vscp_doCmdDataAvailable( void )
    \brief Check if an event is available.
	\return The number of messages available or if negative
        an error code.
*/

extern "C" int WINAPI vscp_doCmdDataAvailable( long handle );


/*!
    \fn int vscp_doCmdStatus( canalStatus *pStatus )
    \brief Get CANAL status.
	\return True if success, false if not.
*/

extern "C" int WINAPI vscp_doCmdStatus( long handle,
													canalStatus *pStatus );


/*!
    \fn int vscp_doCmdStatistics( canalStatistics *pStatistics )
    \brief Get CANAL statistics.
	\return True if success, false if not.
*/

extern "C" int WINAPI vscp_doCmdStatistics( long handle,
														canalStatistics *pStatistics );


/*!
    \fn int vscp_doCmdFilter( unsigned long filter )
    \brief Set CANAL filter.
	\return True if success, false if not.
*/

extern "C" int WINAPI vscp_doCmdFilter( long handle,
													unsigned long filter );


/*!
    \fn int vscp_doCmdMask( unsigned long mask )
    \brief Set CANAL mask.
	\return True if success, false if not.
*/

extern "C" int WINAPI vscp_doCmdMask( long handle,
												unsigned long mask );


/*!
    \fn int vscp_doCmdVscpFilter( const vscpEventFilter *pFilter )
    \brief Set VSCP filter/mask.
	\return True if success, false if not.
*/

extern "C" int WINAPI vscp_doCmdVscpFilter( long handle,
														const vscpEventFilter *pFilter );


 /*!
    \fn int vscp_doCmdBaudrate( unsigned long baudrate )
    \brief Set baudrate.
	\return True if success, false if not.
*/

extern "C" int WINAPI vscp_doCmdBaudrate( long handle,
													unsigned long baudrate );


/*!
    \fn long vscp_doCmdVersion( void  )
    \brief Get interface version.
	\return Return the interface version.
*/

extern "C" unsigned long WINAPI vscp_doCmdVersion( long handle  );


/*!
    \fn long vscp_doCmdDLLVersion( void )
    \brief Get driver dll version.
	\return Return the interface dll version.
*/

extern "C" unsigned long WINAPI vscp_doCmdDLLVersion( long handle );


/*!
    \fn char * vscp_doCmdVendorString( void )
    \brief Get vendor string.
	\return Pointer to vendor string.
*/

extern "C"  const char * WINAPI vscp_doCmdVendorString( long handle );

/*!
    \fn char * vscp_doCmdGetDriverInfo( void )
    \brief Get driver information.
	\return Pointer to driver information string.
*/

extern "C"  const char * WINAPI vscp_doCmdGetDriverInfo( long handle );


