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

#ifndef WIN32
#define __LINUX__
#endif

#include "../common/vscp.h"

#ifdef __cplusplus
extern "C" {
#endif	

#ifdef WIN32

// * * * * *  W I N D O W S  * * * * *
long WINAPI EXPORT vscphlp_newSession(void);
void WINAPI EXPORT vscphlp_closeSession(long handle);
void WINAPI EXPORT vscphlp_setResponseTimeout(long handle, unsigned char timeout );
int WINAPI EXPORT vscphlp_isConnected(const long handle);
int WINAPI EXPORT vscphlp_doCommand( long handle, const char * cmd );
int WINAPI EXPORT vscphlp_checkReply( long handle, int bClear );
int WINAPI EXPORT vscphlp_clearLocalInputQueue( long handle );
int WINAPI EXPORT vscphlp_openInterface( long handle, const char *pInterface, unsigned long flags );
int WINAPI EXPORT vscphlp_open( long handle, const char *pHostname,  const char *pUsername, const char *pPassword );
int WINAPI EXPORT vscphlp_close( long handle );
int WINAPI EXPORT vscphlp_noop( long handle );
int WINAPI EXPORT vscphlp_clearDaemonEventQueue( long handle );
int WINAPI EXPORT vscphlp_sendEvent( long handle, const vscpEvent *pEvent );
int WINAPI EXPORT vscphlp_sendEventEx( long handle, const vscpEventEx *pEvent );
int WINAPI EXPORT vscphlp_receiveEvent( long handle, vscpEvent *pEvent );
int WINAPI EXPORT vscphlp_receiveEventEx( long handle, vscpEventEx *pEvent );
int WINAPI EXPORT vscphlp_blockingReceiveEvent( long handle, vscpEvent *pEvent );
int WINAPI EXPORT vscphlp_blockingReceiveEventEx( long handle, vscpEventEx *pEvent );
int WINAPI EXPORT vscphlp_enterReceiveLoop(const long handle);
int WINAPI EXPORT vscphlp_quitReceiveLoop(const long handle);
int WINAPI EXPORT vscphlp_isDataAvailable( long handle, unsigned int *pCount );
int WINAPI EXPORT vscphlp_getStatus( long handle, VSCPStatus *pStatus );
int WINAPI EXPORT vscphlp_getStatistics( long handle, VSCPStatistics *pStatistics );
int WINAPI EXPORT vscphlp_setFilter( long handle, const vscpEventFilter *pFilter );
int WINAPI EXPORT vscphlp_getVersion( long handle, unsigned char *pMajorVer,
                                                    unsigned char *pMinorVer,
                                                    unsigned char *pSubMinorVer );
int WINAPI EXPORT vscphlp_getDLLVersion( long handle, unsigned long *pVersion );
int WINAPI EXPORT vscphlp_getVendorString( long handle, char *pVendorStr, int size  );
int WINAPI EXPORT vscphlp_getDriverInfo( long handle, char *pVendorStr, int size  );
int WINAPI EXPORT vscphlp_shutDownServer( long handle );

//-------------------------------------------------------------------------
//                                Variables 
//-------------------------------------------------------------------------

int WINAPI EXPORT vscphlp_deleteVariable( long handle, const char *pName );

int WINAPI EXPORT vscphlp_createVariable( long handle, 
                                             const char *pName,
                                             const char* pType,
                                             const char* pValue,
                                             int bPersistent );

int WINAPI EXPORT vscphlp_saveVariablesToDisk( long handle );

int WINAPI EXPORT vscphlp_getVariableString( long handle, const char *pName, char *pValue, int size );
int WINAPI EXPORT vscphlp_setVariableString( long handle, const char *pName, char *pValue );

int WINAPI EXPORT vscphlp_getVariableBool( long handle, const char *pName, int *pvalue );
int WINAPI EXPORT vscphlp_setVariableBool( long handle, const char *pName, int value );

int WINAPI EXPORT vscphlp_getVariableInt( long handle, const char *pName, int *pvalue );
int WINAPI EXPORT vscphlp_setVariableInt( long handle, const char *pName, int value );

int WINAPI EXPORT vscphlp_getVariableLong( long handle, const char *pName, long *pvalue );
int WINAPI EXPORT vscphlp_setVariableLong( long handle, const char *pName, long value );

int WINAPI EXPORT vscphlp_getVariableDouble( long handle, const char *pName, double *pvalue );
int WINAPI EXPORT vscphlp_setVariableDouble( long handle, const char *pName, double value );

int WINAPI EXPORT vscphlp_getVariableMeasurement( long handle, const char *pName, char *pvalue, int size );
int WINAPI EXPORT vscphlp_setVariableMeasurement( long handle, const char *pName, char *pValue );

int WINAPI EXPORT vscphlp_getVariableEvent( long handle, const char *pName, vscpEvent *pEvent );
int WINAPI EXPORT vscphlp_setVariableEvent( long handle, const char *pName, vscpEvent *pEvent );

int WINAPI EXPORT vscphlp_getVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );
int WINAPI EXPORT vscphlp_setVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );

int WINAPI EXPORT vscphlp_getVariableGUIDString( long handle, const char *pName, const char *pGUID, int size );
int WINAPI EXPORT vscphlp_setVariableGUIDString( long handle, const char *pName, const char *pGUID );

int WINAPI EXPORT vscphlp_getVariableGUIDArray( long handle, const char *pName, unsigned char *pGUID );
int WINAPI EXPORT vscphlp_setVariableGUIDArray( long handle, const char *pName, const unsigned char *pGUID );

int WINAPI EXPORT vscphlp_getVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short *psize );
int WINAPI EXPORT vscphlp_setVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short size );

int WINAPI EXPORT vscphlp_getVariableVSCPClass( long handle, const char *pName, unsigned short *vscp_class );
int WINAPI EXPORT vscphlp_setVariableVSCPClass( long handle, const char *pName, unsigned short vscp_class );

int WINAPI EXPORT vscphlp_getVariableVSCPType( long handle, const char *pName, unsigned short *vscp_type );
int WINAPI EXPORT vscphlp_setVariableVSCPType( long handle, const char *pName, unsigned short vscp_type );

//-------------------------------------------------------------------------
//                                Helpers 
//-------------------------------------------------------------------------

unsigned long WINAPI EXPORT vscphlp_readStringValue( const char * pStrValue );
unsigned char WINAPI EXPORT vscphlp_getVscpPriority( const vscpEvent *pEvent );
void WINAPI EXPORT vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority );
unsigned char WINAPI EXPORT vscphlp_getVSCPheadFromCANid( const unsigned long id );
unsigned short WINAPI EXPORT vscphlp_getVSCPclassFromCANid( const unsigned long id );
unsigned short WINAPI EXPORT vscphlp_getVSCPtypeFromCANid( const unsigned long id );
unsigned short WINAPI EXPORT vscphlp_getVSCPnicknameFromCANid( const unsigned long id );
unsigned long WINAPI EXPORT vscphlp_getCANidFromVSCPdata( const unsigned char priority, 
                                                             const unsigned short vscp_class, 
                                                             const unsigned short vscp_type );
unsigned long WINAPI EXPORT vscphlp_getCANidFromVSCPevent( const vscpEvent *pEvent );
unsigned short WINAPI EXPORT vscphlp_calcCRC( vscpEvent *pEvent, short bSet );
int WINAPI EXPORT vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID );
int WINAPI EXPORT vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char *pStr );
int WINAPI EXPORT vscphlp_writeGuidToString( const vscpEvent *pEvent, char *pStr );
int WINAPI EXPORT vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                                      char *strGUID );
int WINAPI EXPORT vscphlp_writeGuidArrayToString( const unsigned char *pGUID, 
                                                     char *strGUID,
                                                     int size );
int WINAPI EXPORT vscphlp_isGUIDEmpty( unsigned char *pGUID );
int WINAPI EXPORT vscphlp_isSameGUID( const unsigned char *pGUID1, 
                                                const unsigned char *pGUID2 );
int WINAPI EXPORT vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                               const vscpEvent *pEvent );
int WINAPI EXPORT vscphlp_convertVSCPfromEx( vscpEvent *pEvent, 
                                               const vscpEventEx *pEventEx );
void WINAPI EXPORT vscphlp_deleteVSCPevent( vscpEvent *pEvent );
void WINAPI EXPORT vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx );
void WINAPI EXPORT vscphlp_clearVSCPFilter( vscpEventFilter *pFilter );
int WINAPI EXPORT vscphlp_readFilterFromString( vscpEventFilter *pFilter, const char *strFilter );
int WINAPI EXPORT vscphlp_readMaskFromString( vscpEventFilter *pFilter, const char *strMask );
int WINAPI EXPORT vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                             const vscpEventFilter *pFilter );
int WINAPI EXPORT vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                                            const canalMsg *pcanalMsg,
                                                            unsigned char *pGUID,
                                                            bool bCAN );
int WINAPI EXPORT vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                                            const vscpEvent *pvscpEvent );
int WINAPI EXPORT vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                                            const vscpEventEx *pvscpEventEx );
unsigned long WINAPI EXPORT vscphlp_getTimeStamp( void );
int WINAPI EXPORT vscphlp_copyVSCPEvent( vscpEvent *pEventTo, 
                                                    const vscpEvent *pEventFrom );
int WINAPI EXPORT vscphlp_writeVscpDataToString( const vscpEvent *pEvent, 
                                                            const char *pstr, 
                                                            bool bUseHtmlBreak );
int WINAPI EXPORT vscphlp_getVscpDataFromString( vscpEvent *pEvent, 
                                       const char *pstr );
int WINAPI EXPORT vscphlp_writeVscpEventToString( vscpEvent *pEvent, char *p );
int WINAPI EXPORT vscphlp_getVscpEventFromString( vscpEvent *pEvent, const char *p );
unsigned char WINAPI EXPORT vscphlp_getMeasurementDataCoding( const vscpEvent *pEvent );

#else

// * * * * *  U N I X  /  L I N U X  * * * * *

long vscphlp_newSession(void);
void vscphlp_closeSession(long handle);
void vscphlp_setResponseTimeout(long handle, unsigned char timeout );
int vscphlp_isConnected(const long handle);
int vscphlp_doCommand( long handle, const char * cmd );
int vscphlp_checkReply( long handle, int bClear );
int vscphlp_clearLocalInputQueue( long handle );
int vscphlp_OpenInterface( long handle, const char *pInterface, unsigned long flags );
int vscphlp_Open( long handle, const char *pHostname, const char *pUsername, const char *pPassword );
int vscphlp_Close( long handle );
int vscphlp_Noop( long handle );
int vscphlp_ClearDaemonEventQueue( long handle );
int vscphlp_SendEvent( long handle,  const vscpEvent *pEvent );
int vscphlp_SendEventEx( long handle, const vscpEventEx *pEvent );
int vscphlp_receiveEvent( long handle, vscpEvent *pEvent );
int vscphlp_receiveEventEx( long handle, vscpEventEx *pEvent );
int vscphlp_blockingReceiveEvent( long handle, vscpEvent *pEvent );
int vscphlp_blockingReceiveEventEx( long handle, vscpEventEx *pEvent );
int vscphlp_enterReceiveLoop(const long handle);
int vscphlp_quitReceiveLoop(const long handle);
int vscphlp_isDataAvailable( long handle, unsigned int *pCount );
int vscphlp_getStatus( long handle, VSCPStatus *pStatus );
int vscphlp_getStatistics( long handle, VSCPStatistics *pStatistics );
int vscphlp_SetFilter( long handle, const vscpEventFilter *pFilter );
int vscphlp_getVersion( long handle, unsigned char *pMajorVer,
                                                    unsigned char *pMinorVer,
                                                    unsigned char *pSubMinorVer );
int vscphlp_GetDLLVersion( long handle, unsigned long *pVersion );
int vscphlp_getVendorString( long handle, char *pVendorStr, int size  );
int vscphlp_GetDriverInfo( long handle, char *pVendorStr, int size  );
int vscphlp_ServerShutDown( long handle );

//-------------------------------------------------------------------------
//                                Variables 
//-------------------------------------------------------------------------

int vscphlp_deleteVariable( long handle, const char *pName );

int vscphlp_createVariable( long handle, 
                                const char *pName,
                                const char* pType,
                                const char* pValue,
                                int bPersistent );

int vscphlp_saveVariablesToDisk( long handle ):

int vscphlp_getVariableString( long handle, const char *pName, char *pvalue );
int vscphlp_setVariableString( long handle, const char *pName, char Value );

int vscphlp_getVariableBool( long handle, const char *pName, int *bValue );
int vscphlp_setVariableBool( long handle, const char *pName, int value );

int vscphlp_getVariableInt( long handle, const char *pName, int *value );
int vscphlp_setVariableInt( long handle, const char *pName, int value );

int vscphlp_getVariableLong( long handle, const char *pName, long *value );
int vscphlp_setVariableLong( long handle, const char *pName, long value );

int vscphlp_getVariableDouble( long handle, const char *pName, double *value );
int vscphlp_setVariableDouble( long handle, const char *pName, double value );

int vscphlp_getVariableMeasurement( long handle, const char *pName, char *pvalue, int size );
int vscphlp_setVariableMeasurement( long handle, const char *pName, char *pValue );

int vscphlp_getVariableEvent( long handle, const char *pName, vscpEvent *pEvent );
int vscphlp_setVariableEvent( long handle, const char *pName, vscpEvent *pEvent );

int vscphlp_getVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );
int vscphlp_setVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );

int vscphlp_getVariableGUIDString( long handle, const char *pName, const char *pGUID, int size );
int vscphlp_setVariableGUIDString( long handle, const char *pName, const char *pGUID );

int vscphlp_getVariableGUIDArray( long handle, const char *pName, unsigned char *pGUID );
int vscphlp_setVariableGUIDArray( long handle, const char *pName, const unsigned char *pGUID );

int vscphlp_getVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short *psize );
int vscphlp_setVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short size );

int vscphlp_getVariableVSCPClass( long handle, const char *pName, unsigned short *vscp_class );
int vscphlp_setVariableVSCPClass( long handle, const char *pName, unsigned short *vscp_class );

int vscphlp_getVariableVSCPType( long handle, const char *pName, unsigned short *vscp_type );
int vscphlp_setVariableVSCPType( long handle, const char *pName, unsigned short *vscp_type );


//-------------------------------------------------------------------------
//                                Helpers 
//-------------------------------------------------------------------------

unsigned long vscphlp_readStringValue( const char * pStrValue );
unsigned char vscphlp_getVscpPriority( const vscpEvent *pEvent );
void vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority );
unsigned char vscphlp_getVSCPheadFromCANid( const unsigned long id );
unsigned short vscphlp_getVSCPclassFromCANid( const unsigned long id );
unsigned short vscphlp_getVSCPtypeFromCANid( const unsigned long id );
unsigned short vscphlp_getVSCPnicknameFromCANid( const unsigned long id );
unsigned long vscphlp_getCANidFromVSCPdata( const unsigned char priority, 
                                               const unsigned short vscp_class, 
                                               const unsigned short vscp_type );
unsigned long vscphlp_getCANidFromVSCPevent( const vscpEvent *pEvent );
unsigned short vscphlp_calcCRC( vscpEvent *pEvent, short bSet );
int vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID );
int vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr );
int vscphlp_writeGuidToString( const vscpEvent *pEvent, char * pStr );
int vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                       char *strGUID );
int vscphlp_writeGuidArrayToString( const unsigned char *pGUID, 
                                       char *strGUID,
                                       int size );
int vscphlp_isGUIDEmpty( unsigned char *pGUID );
int vscphlp_isSameGUID( const unsigned char *pGUID1, 
                                       const unsigned char *pGUID2 );
int vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                       const vscpEvent *pEvent );
int vscphlp_convertVSCPfromEx( vscpEvent *pEvent, 
                                       const vscpEventEx *pEventEx );
void vscphlp_deleteVSCPevent( vscpEvent *pEvent );
void vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx );
void vscphlp_clearVSCPFilter( vscpEventFilter *pFilter );
int vscphlp_readFilterFromString( vscpEventFilter *pFilter, const char *strFilter );
int vscphlp_readMaskFromString( vscpEventFilter *pFilter, const char *strMask );
int vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                               const vscpEventFilter *pFilter );
int vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                    const canalMsg *pcanalMsg,
                                    unsigned char *pGUID,
                                    bool bCAN );
int vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                    const vscpEvent *pvscpEvent );
int vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                    const vscpEventEx *pvscpEventEx );
unsigned long vscphlp_getTimeStamp( void );
int vscphlp_copyVSCPEvent( vscpEvent *pEventTo, 
                              const vscpEvent *pEventFrom );
int vscphlp_writeVscpDataToString( const vscpEvent *pEvent, 
                                       const char *pstr, 
                                       bool bUseHtmlBreak );
int vscphlp_getVscpDataFromString( vscpEvent *pEvent, 
                                      const char *pstr );
int vscphlp_writeVscpEventToString( vscpEvent *pEvent, char *p );
int vscphlp_getVscpEventFromString( vscpEvent *pEvent, const char *p );
unsigned char vscphlp_getMeasurementDataCoding( const vscpEvent *pEvent );

#endif

#ifdef __cplusplus
}  // Extern "c"
#endif


/////////////////////////////////////////////////////////////////////////////////////////


