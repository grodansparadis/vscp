///////////////////////////////////////////////////////////////////////////////
// vscphelperlib.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2015 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#ifndef WIN32
#define __LINUX__
#define VSCP_HELPER_PRE	
#else
#define VSCP_HELPER_PRE	WINAPI EXPORT
#endif

#include <vscp.h>

#ifdef __cplusplus
extern "C" {
#endif	

#ifdef WIN32

// * * * * *  W I N D O W S  * * * * *

long WINAPI EXPORT vscphlp_newSession(void);
void WINAPI EXPORT vscphlp_closeSession(long handle);
int WINAPI EXPORT vscphlp_setResponseTimeout(long handle, unsigned char timeout );
int WINAPI EXPORT vscphlp_setAfterCommandSleep(long handle, unsigned short timeout );
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
unsigned char WINAPI EXPORT vscphlp_getVSCPheadFromCANALid( const unsigned long id );
unsigned short WINAPI EXPORT vscphlp_getVSCPclassFromCANALid( const unsigned long id );
unsigned short WINAPI EXPORT vscphlp_getVSCPtypeFromCANALid( const unsigned long id );
unsigned char WINAPI EXPORT vscphlp_getVSCPnicknameFromCANALid( const unsigned long id );
unsigned long WINAPI EXPORT vscphlp_getCANALidFromVSCPdata( const unsigned char priority, 
                                                             const unsigned short vscp_class, 
                                                             const unsigned short vscp_type );
unsigned long WINAPI EXPORT vscphlp_getCANALidFromVSCPevent( const vscpEvent *pEvent );
unsigned long WINAPI EXPORT vscphlp_getCANALidFromVSCPeventEx( const vscpEventEx *pEvent );
unsigned short WINAPI EXPORT vscphlp_calc_crc_Event( vscpEvent *pEvent, short bSet );
unsigned short WINAPI EXPORT vscphlp_calc_crc_EventEx( vscpEvent *pEvent, short bSet );
unsigned char WINAPI EXPORT vscphlp_calcCRC4GUIDArray( unsigned char *pguid );
unsigned char WINAPI EXPORT vscphlp_calcCRC4GUIDString(const char *strguid);
int WINAPI EXPORT vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID );
int WINAPI EXPORT vscphlp_getGuidFromStringEx( vscpEventEx *pEvent, const char * pGUID );
int WINAPI EXPORT vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char *pStr );
int WINAPI EXPORT vscphlp_writeGuidToString( const vscpEvent *pEvent, char *pStr, int size );
int WINAPI EXPORT vscphlp_writeGuidToStringEx( const vscpEventEx *pEvent, char * pStr, int size );
int WINAPI EXPORT vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                                      char *strGUID,
                                                      int size );
int WINAPI EXPORT vscphlp_writeGuidToString4RowsEx( const vscpEventEx *pEvent, 
                                                      char *strGUID,
                                                      int size );
int WINAPI EXPORT vscphlp_writeGuidArrayToString( const unsigned char *pGUID, 
                                                     char *strGUID,
                                                     int size );
int WINAPI EXPORT vscphlp_isGUIDEmpty( unsigned char *pGUID );
int WINAPI EXPORT vscphlp_isSameGUID( const unsigned char *pGUID1, 
                                                const unsigned char *pGUID2 );
int WINAPI EXPORT vscphlp_reverseGUID(unsigned char *pGUID);
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
                                                   unsigned char *pGUID );
int WINAPI EXPORT vscphlp_convertCanalToEventEx( vscpEventEx *pvscpEvent,
                                                     const canalMsg *pcanalMsg,
                                                     unsigned char *pGUID );
int WINAPI EXPORT vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                                            const vscpEvent *pvscpEvent );
int WINAPI EXPORT vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                                            const vscpEventEx *pvscpEventEx );
unsigned long WINAPI EXPORT vscphlp_makeTimeStamp( void );
int WINAPI EXPORT vscphlp_copyVSCPEvent( vscpEvent *pEventTo, 
                                                    const vscpEvent *pEventFrom );
int WINAPI EXPORT vscphlp_writeVscpDataToString( const vscpEvent *pEvent, 
                                                           char *pstr,
                                                           int size,  
                                                           int bUseHtmlBreak );
int WINAPI EXPORT vscphlp_setVscpDataFromString( vscpEvent *pEvent, const char *pstr );
int WINAPI EXPORT vscphlp_setVscpDataArrayFromString( unsigned char *pData,
                                                                    unsigned short *psizeData,
                                                                    const char *pstr );
int WINAPI EXPORT vscphlp_writeVscpEventToString( const vscpEvent *pEvent, char *p, int size );
int WINAPI EXPORT vscphlp_writeVscpEventExToString( const vscpEventEx *pEvent, char *p, int size );
int WINAPI EXPORT vscphlp_setVscpEventFromString( vscpEvent *pEvent, const char *p );
int WINAPI EXPORT vscphlp_setVscpEventExFromString( vscpEventEx *pEvent, const char *p );

unsigned char WINAPI EXPORT vscphlp_getMeasurementDataCoding( const vscpEvent *pEvent );
unsigned long long WINAPI EXPORT vscphlp_getDataCodingBitArray(const unsigned char *pCode, int size );
unsigned long long WINAPI EXPORT vscphlp_getDataCodingInteger(const unsigned char *pCode, int size );
double WINAPI EXPORT vscphlp_getDataCodingNormalizedInteger(const unsigned char *pCode, int size );
int WINAPI EXPORT vscphlp_getDataCodingString(const unsigned char *pData,
                                                    unsigned char dataLength, 
                                                    char *strResult,
                                                    int size );
int WINAPI EXPORT vscphlp_getVscpDataFromString( vscpEvent *pEvent, 
                                                    const char *pstr );
int WINAPI EXPORT vscphlp_getVSCPMeasurementAsString( const vscpEvent *pEvent, 
                                                                    char *pResult, 
                                                                    int size );
int WINAPI EXPORT vscphlp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent, double *pvalue);
int WINAPI EXPORT vscphlp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent, 
                                                                            char *pStrResult, 
                                                                            int size );
int WINAPI EXPORT vscphlp_convertFloatToNormalizedEventData( unsigned char *pdata,
                                                                            unsigned short *psize,
                                                                            double value,                                                
                                                                            unsigned char unit,
                                                                            unsigned char sensoridx );
int WINAPI EXPORT vscphlp_convertFloatToFloatEventData( unsigned char *pdata,
                                                                        unsigned short *psize, 
                                                                        float value,
                                                                        unsigned char unit,
                                                                        unsigned char sensoridx ); 
int WINAPI EXPORT vscphlp_convertIntegerToNormalizedEventData( unsigned char *pdata,
                                                                        unsigned short *psize,
                                                                        unsigned long long val64,
                                                                        unsigned char unit,
                                                                        unsigned char sensoridx );
int WINAPI EXPORT vscphlp_makeFloatMeasurementEvent( vscpEvent *pEvent, 
                                                                    float value,
                                                                    unsigned char unit,
                                                                    unsigned char sensoridx );
int WINAPI EXPORT vscphlp_getVSCPMeasurementZoneAsString(const vscpEvent *pEvent, char *pStr, int size );
int WINAPI EXPORT vscphlp_getMeasurementAsFloat(const unsigned char *pNorm, 
                                                                unsigned char length,
                                                                float *pResult );
int WINAPI EXPORT vscphlp_replaceBackslash( char *pStr );
unsigned char WINAPI EXPORT vscphlp_getVscpPriority( const vscpEvent *pEvent );
unsigned char WINAPI EXPORT vscphlp_getVscpPriorityEx( const vscpEventEx *pEvent );
void WINAPI EXPORT vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority );
void WINAPI EXPORT vscphlp_setVscpPriorityEx( vscpEventEx *pEvent, unsigned char priority );

#else

// * * * * *  U N I X  /  L I N U X  * * * * *

long vscphlp_newSession(void);
void vscphlp_closeSession(long handle);
int vscphlp_setResponseTimeout(long handle, unsigned char timeout );
int vscphlp_setAfterCommandSleep(long handle, unsigned short timeout );
int vscphlp_isConnected(const long handle);
int vscphlp_doCommand( long handle, const char * cmd );
int vscphlp_checkReply( long handle, int bClear );
int vscphlp_clearLocalInputQueue( long handle );
int vscphlp_openInterface( long handle, const char *pInterface, unsigned long flags );
int vscphlp_open( long handle, const char *pHostname, const char *pUsername, const char *pPassword );
int vscphlp_close( long handle );
int vscphlp_noop( long handle );
int vscphlp_clearDaemonEventQueue( long handle );
int vscphlp_sendEvent( long handle,  const vscpEvent *pEvent );
int vscphlp_sendEventEx( long handle, const vscpEventEx *pEvent );
int vscphlp_receiveEvent( long handle, vscpEvent *pEvent );
int vscphlp_receiveEventEx( long handle, vscpEventEx *pEvent );
int vscphlp_blockingReceiveEvent( long handle, vscpEvent *pEvent );
int vscphlp_blockingReceiveEventEx( long handle, vscpEventEx *pEvent );
int vscphlp_enterReceiveLoop(const long handle);
int vscphlp_quitReceiveLoop(const long handle);
int vscphlp_isDataAvailable( long handle, unsigned int *pCount );
int vscphlp_getStatus( long handle, VSCPStatus *pStatus );
int vscphlp_getStatistics( long handle, VSCPStatistics *pStatistics );
int vscphlp_setFilter( long handle, const vscpEventFilter *pFilter );
int vscphlp_getVersion( long handle, unsigned char *pMajorVer,
                                                    unsigned char *pMinorVer,
                                                    unsigned char *pSubMinorVer );
int vscphlp_getDLLVersion( long handle, unsigned long *pVersion );
int vscphlp_getVendorString( long handle, char *pVendorStr, int size  );
int vscphlp_getDriverInfo( long handle, char *pVendorStr, int size  );
int vscphlp_serverShutDown( long handle );

//-------------------------------------------------------------------------
//                                Variables 
//-------------------------------------------------------------------------

int vscphlp_deleteVariable( long handle, const char *pName );

int vscphlp_createVariable( long handle, 
                                const char *pName,
                                const char* pType,
                                const char* pValue,
                                int bPersistent );

int vscphlp_saveVariablesToDisk( long handle );

int vscphlp_getVariableString( long handle, const char *pName, char *pvalue, int size );
int vscphlp_setVariableString( long handle, const char *pName, char *pValue );

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
int vscphlp_setVariableVSCPClass( long handle, const char *pName, unsigned short vscp_class );

int vscphlp_getVariableVSCPType( long handle, const char *pName, unsigned short *vscp_type );
int vscphlp_setVariableVSCPType( long handle, const char *pName, unsigned short vscp_type );


//-------------------------------------------------------------------------
//                                Helpers 
//-------------------------------------------------------------------------


unsigned long vscphlp_readStringValue( const char * pStrValue );
unsigned char vscphlp_getVscpPriority( const vscpEvent *pEvent );
void vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority );
unsigned char vscphlp_getVSCPheadFromCANALid( const unsigned long id );
unsigned short vscphlp_getVSCPclassFromCANALid( const unsigned long id );
unsigned short vscphlp_getVSCPtypeFromCANALid( const unsigned long id );
unsigned char vscphlp_getVSCPnicknameFromCANALid( const unsigned long id );
unsigned long vscphlp_getCANALidFromVSCPdata( const unsigned char priority, 
                                               const unsigned short vscp_class, 
                                               const unsigned short vscp_type );
unsigned long vscphlp_getCANALidFromVSCPevent( const vscpEvent *pEvent );
unsigned long vscphlp_getCANALidFromVSCPeventEx( const vscpEventEx *pEvent );
unsigned short vscphlp_calc_crc_Event( vscpEvent *pEvent, short bSet );
unsigned short vscphlp_calc_crc_EventEx( vscpEvent *pEvent, short bSet );
unsigned char vscphlp_calcCRC4GUIDArray( unsigned char *pguid );
unsigned char vscphlp_calcCRC4GUIDString(const char *strguid);
int vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID );
int vscphlp_getGuidFromStringEx( vscpEventEx *pEvent, const char * pGUID );
int vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr );
int vscphlp_writeGuidToString( const vscpEvent *pEvent, char *pStr, int size );
int vscphlp_writeGuidToStringEx( const vscpEventEx *pEvent, char *pStr, int size );
int vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                       char *strGUID,
                                       int size );
int vscphlp_writeGuidToString4RowsEx( const vscpEventEx *pEvent, 
                                       char *strGUID,
                                       int size );
int vscphlp_writeGuidArrayToString( const unsigned char *pGUID, 
                                       char *strGUID,
                                       int size );
int vscphlp_isGUIDEmpty( unsigned char *pGUID );
int vscphlp_isSameGUID( const unsigned char *pGUID1, 
                                       const unsigned char *pGUID2 );
int vscphlp_reverseGUID(unsigned char *pGUID);
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
                                    unsigned char *pGUID );
int vscphlp_convertCanalToEventEx( vscpEventEx *pvscpEvent,
                                    const canalMsg *pcanalMsg,
                                    unsigned char *pGUID );
int vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                    const vscpEvent *pvscpEvent );
int vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                    const vscpEventEx *pvscpEventEx );
unsigned long vscphlp_makeTimeStamp( void );
int vscphlp_copyVSCPEvent( vscpEvent *pEventTo, 
                              const vscpEvent *pEventFrom );
int vscphlp_writeVscpDataToString( const vscpEvent *pEvent, 
											char *pstr,
											int size,  
											int bUseHtmlBreak );
int vscphlp_writeVscpDataWithSizeToString(const unsigned short sizeData,
													const unsigned char *pData,
													char *pStr,
                                              int size,
													int bUseHtmlBreak,
                                              int bBreak );
int vscphlp_getVscpDataFromString( vscpEvent *pEvent, 
                                      const char *pstr );
int vscphlp_getVscpDataArrayFromString( unsigned char *pData,
                                          unsigned short *psizeData,
                                          const char *pstr );
int vscphlp_writeVscpEventToString( const vscpEvent *pEvent, char *p, int size );
int vscphlp_writeVscpEventExToString( const vscpEventEx *pEvent, char *p, int size );
int vscphlp_setVscpEventFromString( vscpEvent *pEvent, const char *p );
int vscphlp_setVscpEventExFromString( vscpEventEx *pEvent, const char *p );
int vscphlp_setVscpDataFromString( vscpEvent *pEvent, const char *pstr );
int vscphlp_getVSCPMeasurementAsString( const vscpEvent *pEvent, 
                                           char *pResult, 
                                           int size );
int vscphlp_setVscpDataArrayFromString( unsigned char *pData,
												unsigned short *psizeData,
												const char *pstr );

unsigned char vscphlp_getMeasurementDataCoding( const vscpEvent *pEvent );
unsigned long long vscphlp_getDataCodingBitArray(const unsigned char *pCode, int size );
unsigned long long vscphlp_getDataCodingInteger(const unsigned char *pCode, int size );
double vscphlp_getDataCodingNormalizedInteger(const unsigned char *pCode, int size );
int vscphlp_getDataCodingString(const unsigned char *pData,
                                       unsigned char dataLength, 
                                       char *strResult,
                                       int size );
int vscphlp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent, double *pvalue);
int vscphlp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent, 
                                                 char *pStrResult, 
                                                 int size );
int vscphlp_convertFloatToNormalizedEventData( unsigned char *pdata,
                                                  unsigned short *psize,
                                                  double value,                                                
                                                  unsigned char unit,
                                                  unsigned char sensoridx );
int vscphlp_convertFloatToFloatEventData( unsigned char *pdata,
                                                  unsigned short *psize, 
                                                  float value,
                                                  unsigned char unit,
                                                  unsigned char sensoridx );
int vscphlp_makeFloatMeasurementEvent( vscpEvent *pEvent, 
                                                   float value,
                                                   unsigned char unit,
                                                   unsigned char sensoridx );
int vscphlp_getVSCPMeasurementZoneAsString(const vscpEvent *pEvent, char *pStr, int size );
int vscphlp_getMeasurementAsFloat(const unsigned char *pNorm, 
                                                   unsigned char length,
                                                   float *pResult );
int vscphlp_replaceBackslash( char *pStr );
unsigned char vscphlp_getVscpPriority( const vscpEvent *pEvent );
unsigned char vscphlp_getVscpPriorityEx( const vscpEventEx *pEvent );
void vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority );
void vscphlp_setVscpPriorityEx( vscpEventEx *pEvent, unsigned char priority );

#endif

#ifdef __cplusplus
}  // Extern "c"
#endif


/////////////////////////////////////////////////////////////////////////////////////////


