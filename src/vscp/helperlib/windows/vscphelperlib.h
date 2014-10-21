#include "stdafx.h"
#include "../../common/canal.h"
#include "../../common/vscp.h"



extern "C" long WINAPI vscphlp_newSession(void);
extern "C" void WINAPI vscphlp_closeSession(long handle);
extern "C" long WINAPI vscphlp_OpenInterface( long handle,
						            const char *pInterface,
                                    unsigned long flags );
extern "C" long WINAPI vscphlp_Open( long handle,
                                const char *pHostname, 
                                const char *pUsername, 
                                const char *pPassword );
extern "C" int WINAPI vscphlp_Close( long handle );
extern "C" int WINAPI vscphlp_Noop( long handle );






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


