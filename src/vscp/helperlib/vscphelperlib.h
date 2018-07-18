///////////////////////////////////////////////////////////////////////////////
// vscphelperlib.h
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

#ifndef WIN32
#define __LINUX__
#define VSCP_HELPER_PRE
#else
#define VSCP_HELPER_PRE	WINAPI EXPORT
#endif

#include <time.h>
#include <vscp.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32

#define DllExport   __declspec( dllexport )   

// * * * * *  W I N D O W S  * * * * *

DllExport long WINAPI EXPORT vscphlp_newSession(void);
DllExport void WINAPI EXPORT vscphlp_closeSession(long handle);
DllExport int WINAPI EXPORT vscphlp_setResponseTimeout(long handle, unsigned long timeout );
DllExport int WINAPI EXPORT vscphlp_setAfterCommandSleep(long handle, unsigned short sleeptime );
DllExport int WINAPI EXPORT vscphlp_isConnected(const long handle);
DllExport int WINAPI EXPORT vscphlp_doCommand( long handle, const char * cmd );
DllExport int WINAPI EXPORT vscphlp_checkReply( long handle, int bClear );
DllExport int WINAPI EXPORT vscphlp_clearLocalInputQueue( long handle );
DllExport int WINAPI EXPORT vscphlp_openInterface( long handle, const char *pInterface, unsigned long flags );
DllExport int WINAPI EXPORT vscphlp_open( long handle, const char *pHostname,  const char *pUsername, const char *pPassword );
DllExport int WINAPI EXPORT vscphlp_close( long handle );
DllExport int WINAPI EXPORT vscphlp_noop( long handle );
DllExport int WINAPI EXPORT vscphlp_clearDaemonEventQueue( long handle );
DllExport int WINAPI EXPORT vscphlp_sendEvent( long handle, const vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_sendEventEx( long handle, const vscpEventEx *pEvent );
DllExport int WINAPI EXPORT vscphlp_receiveEvent( long handle, vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_receiveEventEx( long handle, vscpEventEx *pEvent );
DllExport int WINAPI EXPORT vscphlp_blockingReceiveEvent( long handle, vscpEvent *pEvent, unsigned long timout );
DllExport int WINAPI EXPORT vscphlp_blockingReceiveEventEx( long handle, vscpEventEx *pEventEx, unsigned long timout );
DllExport int WINAPI EXPORT vscphlp_enterReceiveLoop(const long handle);
DllExport int WINAPI EXPORT vscphlp_quitReceiveLoop(const long handle);
DllExport int WINAPI EXPORT vscphlp_isDataAvailable( long handle, unsigned int *pCount );
DllExport int WINAPI EXPORT vscphlp_getStatus( long handle, VSCPStatus *pStatus );
DllExport int WINAPI EXPORT vscphlp_getStatistics( long handle, VSCPStatistics *pStatistics );
DllExport int WINAPI EXPORT vscphlp_setFilter( long handle, const vscpEventFilter *pFilter );
DllExport int WINAPI EXPORT vscphlp_getVersion( long handle, unsigned char *pMajorVer,
                                                    unsigned char *pMinorVer,
                                                    unsigned char *pSubMinorVer );
DllExport int WINAPI EXPORT vscphlp_getDLLVersion( long handle, unsigned long *pVersion );
DllExport int WINAPI EXPORT vscphlp_getVendorString( long handle, char *pVendorStr, size_t len );
DllExport int WINAPI EXPORT vscphlp_getDriverInfo( long handle, char *pDriverInfoStr, size_t len );
DllExport int WINAPI EXPORT vscphlp_getGUID( long handle, unsigned char *pGUID );
DllExport int WINAPI EXPORT vscphlp_setGUID( long handle, const unsigned char *pGUID );
DllExport int WINAPI EXPORT vscphlp_shutDownServer( long handle );

//-------------------------------------------------------------------------
//                                Variables
//-------------------------------------------------------------------------

DllExport int WINAPI EXPORT vscphlp_deleteRemoteVariable( long handle, const char *pName );

DllExport int WINAPI EXPORT vscphlp_createRemoteVariable( const char *pName,
                                                            const char* pType,
                                                            const int bPersistent,
                                                            const char *pUser,
                                                            const unsigned long rights,
                                                            const char* pValue,
                                                            const char* pNote  );

DllExport int WINAPI EXPORT vscphlp_saveRemoteVariablesToDisk( long handle,
                                                            const char *pPath,
                                                            const int select,
                                                            const char *pRegExp );

DllExport int WINAPI EXPORT vscphlp_loadRemoteVariablesFromDisk( long handle,
                                                          const char *pPath );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableValue( long handle, const char *pName, char *pvalue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableValue( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableString( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableString( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableBool( long handle, const char *pName, int *pvalue );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableBool( long handle, const char *pName, int value );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableInt( long handle, const char *pName, int *pvalue );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableInt( long handle, const char *pName, int value );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableLong( long handle, const char *pName, long *pvalue );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableLong( long handle, const char *pName, long value );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableDouble( long handle, const char *pName, double *pvalue );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableDouble( long handle, const char *pName, double value );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableMeasurement( long handle,
                                                const char *pName, 
                                                double *pvalue,
                                                int *punit,
                                                int *psensoridx,
                                                int *pzone,
                                                int *psubzone );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableMeasurement( long handle,
                                                const char *pName, 
                                                double value,
                                                int unit,
                                                int sensoridx,
                                                int zone,
                                                int subzone );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableEvent( long handle, const char *pName, vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableEvent( long handle, const char *pName, vscpEvent *pEvent );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableGUIDString( long handle, const char *pName, const char *pGUID, int size );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableGUIDString( long handle, const char *pName, const char *pGUID );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableGUIDArray( long handle, const char *pName, unsigned char *pGUID );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableGUIDArray( long handle, const char *pName, const unsigned char *pGUID );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short *psize );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short size );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableVSCPClass( long handle, const char *pName, unsigned short *vscp_class );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableVSCPClass( long handle, const char *pName, unsigned short vscp_class );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableVSCPType( long handle, const char *pName, unsigned short *vscp_type );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableVSCPType( long handle, const char *pName, unsigned short vscp_type );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableVSCPTimestamp( long handle, const char *pName, unsigned long *vscp_timestamp );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableVSCPTimestamp( long handle, const char *pName, unsigned long vscp_timestamp );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableDateTime( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableDateTime( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableDate( long handle, const char *pName, char *pValue, size_t len);
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableDate( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableTime( long handle, const char *pName, char *pValue, size_t len);
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableTime( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableBlob( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableBlob( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableMIME( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableMIME( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableHTML( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableHTML( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableJavaScript( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableJavaScript( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableLUA( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableLUA( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableLUARES( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableLUARES( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableUX1( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableUX1( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableDMROW( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableDMROW( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableDriver( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableDriver( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableUser( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableUser( long handle, const char *pName, char *pValue );

DllExport int WINAPI EXPORT vscphlp_getRemoteVariableFilter( long handle, const char *pName, char *pValue, size_t len );
DllExport int WINAPI EXPORT vscphlp_setRemoteVariableFilter( long handle, const char *pName, char *pValue );

//-------------------------------------------------------------------------
//                                Helpers
//-------------------------------------------------------------------------

DllExport unsigned long WINAPI EXPORT vscphlp_readStringValue( const char * pStrValue );
DllExport unsigned char WINAPI EXPORT vscphlp_getVscpPriority( const vscpEvent *pEvent );
DllExport void WINAPI EXPORT vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority );
DllExport unsigned char WINAPI EXPORT vscphlp_getVSCPheadFromCANALid( const unsigned long id );
DllExport unsigned short WINAPI EXPORT vscphlp_getVSCPclassFromCANALid( const unsigned long id );
DllExport unsigned short WINAPI EXPORT vscphlp_getVSCPtypeFromCANALid( const unsigned long id );
DllExport unsigned char WINAPI EXPORT vscphlp_getVSCPnicknameFromCANALid( const unsigned long id );
DllExport unsigned long WINAPI EXPORT vscphlp_getCANALidFromVSCPdata( const unsigned char priority,
                                                             const unsigned short vscp_class,
                                                             const unsigned short vscp_type );
DllExport unsigned long WINAPI EXPORT vscphlp_getCANALidFromVSCPevent( const vscpEvent *pEvent );
DllExport unsigned long WINAPI EXPORT vscphlp_getCANALidFromVSCPeventEx( const vscpEventEx *pEvent );
DllExport unsigned short WINAPI EXPORT vscphlp_calc_crc_Event( vscpEvent *pEvent, short bSet );
DllExport unsigned short WINAPI EXPORT vscphlp_calc_crc_EventEx( vscpEvent *pEvent, short bSet );
DllExport unsigned char WINAPI EXPORT vscphlp_calcCRC4GUIDArray( unsigned char *pguid );
DllExport unsigned char WINAPI EXPORT vscphlp_calcCRC4GUIDString(const char *strguid);
DllExport int WINAPI EXPORT vscphlp_getGuidFromString( vscpEvent *pEvent, const char * pGUID );
DllExport int WINAPI EXPORT vscphlp_getGuidFromStringEx( vscpEventEx *pEvent, const char * pGUID );
DllExport int WINAPI EXPORT vscphlp_getGuidFromStringToArray( uint8_t *pGUID, const char *pStr );
DllExport int WINAPI EXPORT vscphlp_writeGuidToString( const vscpEvent *pEvent, char *pStr, size_t len );
DllExport int WINAPI EXPORT vscphlp_writeGuidToStringEx( const vscpEventEx *pEvent, char * pStr, size_t len );
DllExport int WINAPI EXPORT vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent,
                                                      char *strGUID,
                                                      size_t len );
DllExport int WINAPI EXPORT vscphlp_writeGuidToString4RowsEx( const vscpEventEx *pEvent,
                                                      char *strGUID,
                                                      size_t len );
DllExport int WINAPI EXPORT vscphlp_writeGuidArrayToString( const unsigned char *pGUID,
                                                     char *strGUID,
                                                     size_t len );
DllExport int WINAPI EXPORT vscphlp_isGUIDEmpty( unsigned char *pGUID );
DllExport int WINAPI EXPORT vscphlp_isSameGUID( const unsigned char *pGUID1,
                                        const unsigned char *pGUID2 );
DllExport int WINAPI EXPORT vscphlp_reverseGUID(unsigned char *pGUID);
DllExport int WINAPI EXPORT vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx,
                                               const vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_convertVSCPfromEx( vscpEvent *pEvent,
                                               const vscpEventEx *pEventEx );
DllExport int WINAPI EXPORT vscphlp_newVSCPevent( vscpEvent **ppEvent );
DllExport void WINAPI EXPORT vscphlp_deleteVSCPevent( vscpEvent *pEvent );
DllExport void WINAPI EXPORT vscphlp_deleteVSCPevent_v2( vscpEvent **ppEvent );
DllExport void WINAPI EXPORT vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx );
DllExport void WINAPI EXPORT vscphlp_clearVSCPFilter( vscpEventFilter *pFilter );
DllExport void WINAPI EXPORT vscphlp_copyVSCPFilter( vscpEventFilter *pToFilter, const vscpEventFilter *pFromFilter );
DllExport int WINAPI EXPORT vscphlp_readFilterFromString( vscpEventFilter *pFilter, const char *strFilter );
DllExport int WINAPI EXPORT vscphlp_readMaskFromString( vscpEventFilter *pFilter, const char *strMask );
DllExport int WINAPI EXPORT vscphlp_writeFilterToString( vscpEventFilter *pFilter, char *strFilter );
DllExport int WINAPI EXPORT vscphlp_writeMaskToString( vscpEventFilter *pFilter, char *strMask );
DllExport int WINAPI EXPORT vscphlp_doLevel2Filter( const vscpEvent *pEvent,
                                             const vscpEventFilter *pFilter );
DllExport int WINAPI EXPORT vscphlp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                                   const canalMsg *pcanalMsg,
                                                   unsigned char *pGUID );
DllExport int WINAPI EXPORT vscphlp_convertCanalToEventEx( vscpEventEx *pvscpEvent,
                                                     const canalMsg *pcanalMsg,
                                                     unsigned char *pGUID );
DllExport int WINAPI EXPORT vscphlp_convertEventToCanal( canalMsg *pcanalMsg,
                                                    const vscpEvent *pvscpEvent );
DllExport int WINAPI EXPORT vscphlp_convertEventExToCanal( canalMsg *pcanalMsg,
                                                    const vscpEventEx *pvscpEventEx );
DllExport unsigned long WINAPI EXPORT vscphlp_makeTimeStamp( void );
DllExport int WINAPI EXPORT vscphlp_setEventDateTimeBlockToNow( vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_setEventExDateTimeBlockToNow( vscpEventEx *pEventEx );
DllExport int WINAPI EXPORT vscphlp_copyVSCPEvent( vscpEvent *pEventTo,
                                                    const vscpEvent *pEventFrom );
DllExport int WINAPI EXPORT vscphlp_writeVscpDataToString( const vscpEvent *pEvent,
                                                    char *pstr,
                                                    size_t len,
                                                    int bUseHtmlBreak );
DllExport int WINAPI EXPORT vscphlp_writeVscpDataWithSizeToString( const unsigned char *pData,
                                                                    const unsigned short sizeData,                                                                    
                                                                    char *pStr,
                                                                    size_t len,
                                                                    int bUseHtmlBreak,
                                                                    int bBreak );
DllExport int WINAPI EXPORT vscphlp_setVscpDataFromString( vscpEvent *pEvent, const char *pstr );
DllExport int WINAPI EXPORT vscphlp_setVscpDataArrayFromString( unsigned char *pData,
                                                        unsigned short *psizeData,
                                                        const char *pstr );
DllExport int WINAPI EXPORT vscphlp_writeVscpEventToString( const vscpEvent *pEvent, char *p, size_t len );
DllExport int WINAPI EXPORT vscphlp_writeVscpEventExToString( const vscpEventEx *pEvent, char *p, size_t len );
DllExport int WINAPI EXPORT vscphlp_setVscpEventFromString( vscpEvent *pEvent, const char *p );
DllExport int WINAPI EXPORT vscphlp_setVscpEventExFromString( vscpEventEx *pEvent, const char *p );

DllExport unsigned char WINAPI EXPORT vscphlp_getMeasurementDataCoding( const vscpEvent *pEvent );
DllExport unsigned long long WINAPI EXPORT vscphlp_getDataCodingBitArray(const unsigned char *pCode, int size );
DllExport unsigned long long WINAPI EXPORT vscphlp_getDataCodingInteger(const unsigned char *pCode, int size );
DllExport double WINAPI EXPORT vscphlp_getDataCodingNormalizedInteger(const unsigned char *pCode, int size );
DllExport int WINAPI EXPORT vscphlp_getDataCodingString(const unsigned char *pData,
                                                    unsigned char dataLength,
                                                    char *strResult,
                                                    size_t len );
DllExport int WINAPI EXPORT vscphlp_getVscpDataFromString( vscpEvent *pEvent,
                                                    const char *pstr );
DllExport int WINAPI EXPORT vscphlp_getVSCPMeasurementAsString( const vscpEvent *pEvent,
                                                                char *pResult,
                                                                size_t len );
DllExport int WINAPI EXPORT vscphlp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent, double *pvalue);
DllExport int WINAPI EXPORT vscphlp_getVSCPMeasurementFloat64AsString( const vscpEvent *pEvent,
                                                                char *pStrResult,
                                                                size_t len );
DllExport int WINAPI EXPORT vscphlp_convertFloatToNormalizedEventData( unsigned char *pdata,
                                                                unsigned short *psize,
                                                                double value,
                                                                unsigned char unit,
                                                                unsigned char sensoridx );
DllExport int WINAPI EXPORT vscphlp_convertFloatToFloatEventData( unsigned char *pdata,
                                                            unsigned short *psize,
                                                            float value,
                                                            unsigned char unit,
                                                            unsigned char sensoridx );
DllExport int WINAPI EXPORT vscphlp_convertIntegerToNormalizedEventData( unsigned char *pdata,
                                                                    unsigned short *psize,
                                                                    unsigned long long val64,
                                                                    unsigned char unit,
                                                                    unsigned char sensoridx );
DllExport int WINAPI EXPORT vscphlp_makeFloatMeasurementEvent( vscpEvent *pEvent,
                                                        float value,
                                                        unsigned char unit,
                                                        unsigned char sensoridx );
DllExport int WINAPI EXPORT vscphlp_getMeasurementAsFloat(const unsigned char *pNorm,
                                                    unsigned char length,
                                                    float *pResult );
DllExport int WINAPI EXPORT vscphlp_getMeasurementUnit( const vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_getMeasuremenSensorIndex( const vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_getMeasurementZone( const vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_getMeasurementSubZone( const vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_isMeasurement( const vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_makeLevel2FloatMeasurementEvent( vscpEvent *pEvent,
                                                                uint16_t type,
                                                                double value,
                                                                uint8_t unit,
                                                                uint8_t sensoridx,
                                                                uint8_t zone,
                                                                uint8_t subzone );
DllExport int WINAPI EXPORT vscphlp_makeLevel2StringMeasurementEvent( vscpEvent *pEvent,
                                                                uint16_t type,
                                                                double value,
                                                                uint8_t unit,
                                                                uint8_t sensoridx,
                                                                uint8_t zone,
                                                                uint8_t subzone );
DllExport int WINAPI EXPORT vscphlp_convertLevel1MeasuremenToLevel2Double( vscpEvent *pEventLevel1 );
DllExport int WINAPI EXPORT vscphlp_convertLevel1MeasuremenToLevel2String( vscpEvent *pEventLevel1 );
DllExport int WINAPI EXPORT vscphlp_replaceBackslash( char *pStr );
DllExport unsigned char WINAPI EXPORT vscphlp_getVscpPriority( const vscpEvent *pEvent );
DllExport unsigned char WINAPI EXPORT vscphlp_getVscpPriorityEx( const vscpEventEx *pEvent );
DllExport void WINAPI EXPORT vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority );
DllExport void WINAPI EXPORT vscphlp_setVscpPriorityEx( vscpEventEx *pEvent, unsigned char priority );

DllExport int WINAPI EXPORT vscphlp_convertEventToJSON( vscpEvent *pEvent, char *p, size_t len );
DllExport int WINAPI EXPORT vscphlp_convertEventExToJSON( vscpEventEx *pEventEx, char *p, size_t len );
DllExport int WINAPI EXPORT vscphlp_convertEventToXML( vscpEvent *pEvent, char *p, size_t len );
DllExport int WINAPI EXPORT vscphlp_convertEventExToXML( vscpEventEx *pEventEx, char *p, size_t len );
DllExport int WINAPI EXPORT vscphlp_convertEventToHTML( vscpEvent *pEvent, char *p, size_t len );
DllExport int WINAPI EXPORT vscphlp_convertEventExToHTML( vscpEventEx *pEventEx, char *p, size_t len );

DllExport int WINAPI EXPORT vscphlp_getTimeString( char *buf, size_t buf_len, time_t *t );
DllExport int WINAPI EXPORT vscphlp_getISOTimeString( char *buf, size_t buf_len, time_t *t );

DllExport int WINAPI EXPORT vscphlp_getDateStringFromEvent( char *buf, size_t buf_len, vscpEvent *pEvent );
DllExport int WINAPI EXPORT vscphlp_getDateStringFromEventEx( char *buf, size_t buf_len, vscpEventEx *pEventEx );

#else

// * * * * *  U N I X  /  L I N U X  * * * * *

long vscphlp_newSession(void);
void vscphlp_closeSession(long handle);
int vscphlp_setResponseTimeout(long handle, unsigned long timeout );
int vscphlp_setAfterCommandSleep(long handle, unsigned short sleeptime );
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
int vscphlp_blockingReceiveEvent( long handle, vscpEvent *pEvent, unsigned long timout );
int vscphlp_blockingReceiveEventEx( long handle, vscpEventEx *pEventEx, unsigned long timout );
int vscphlp_enterReceiveLoop(const long handle);
int vscphlp_quitReceiveLoop(const long handle);
int vscphlp_isDataAvailable( long handle, unsigned int *pCount );
int vscphlp_getStatus( long handle, VSCPStatus *pStatus );
int vscphlp_getStatistics( long handle, VSCPStatistics *pStatistics );
int vscphlp_setFilter( long handle, const vscpEventFilter *pFilter );
int vscphlp_getVersion( long handle, 
                            unsigned char *pMajorVer,
                            unsigned char *pMinorVer,
                            unsigned char *pSubMinorVer );
int vscphlp_getDLLVersion( long handle, unsigned long *pVersion );
int vscphlp_getVendorString( long handle, char *pVendorStr, size_t len  );
int vscphlp_getDriverInfo( long handle, char *pDriverInfoStr, size_t len  );
int vscphlp_getGUID( long handle, unsigned char *pGUID );
int vscphlp_setGUID( long handle, const unsigned char *pGUID );
int vscphlp_serverShutDown( long handle );

//-------------------------------------------------------------------------
//                                Variables
//-------------------------------------------------------------------------

int vscphlp_deleteRemoteVariable( long handle, const char *pName );

int vscphlp_createRemoteVariable( long handle,
                                        const char *pName,
                                        const char* pType,
                                        const int bPersistent,
                                        const char *pUser,
                                        const unsigned long rights,
                                        const char* pValue,
                                        const char* pNote  );

int vscphlp_saveRemoteVariablesToDisk( long handle,
                                            const char *pPath,
                                            const int select,
                                            const char *pRegExp );

int vscphlp_loadRemoteVariablesFromDisk( long handle, 
                                            const char *pPath );

int vscphlp_getRemoteVariableValue( long handle, const char *pName, char *pvalue, size_t len );
int vscphlp_setRemoteVariableValue( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableString( long handle, const char *pName, char *pvalue, size_t len );
int vscphlp_setRemoteVariableString( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableBool( long handle, const char *pName, int *bValue );
int vscphlp_setRemoteVariableBool( long handle, const char *pName, int value );

int vscphlp_getRemoteVariableInt( long handle, const char *pName, int *value );
int vscphlp_setRemoteVariableInt( long handle, const char *pName, int value );

int vscphlp_getRemoteVariableLong( long handle, const char *pName, long *value );
int vscphlp_setRemoteVariableLong( long handle, const char *pName, long value );

int vscphlp_getRemoteVariableDouble( long handle, const char *pName, double *value );
int vscphlp_setRemoteVariableDouble( long handle, const char *pName, double value );

int vscphlp_getRemoteVariableMeasurement( long handle, 
                                                const char *pName, 
                                                double *pvalue,
                                                int *punit,
                                                int *psensoridx,
                                                int *pzone,
                                                int *psubzone );
int vscphlp_setRemoteVariableMeasurement( long handle, 
                                                const char *pName, 
                                                double value,
                                                int unit,
                                                int sensoridx,
                                                int zone,
                                                int subzone );

int vscphlp_getRemoteVariableEvent( long handle, const char *pName, vscpEvent *pEvent );
int vscphlp_setRemoteVariableEvent( long handle, const char *pName, vscpEvent *pEvent );

int vscphlp_getRemoteVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );
int vscphlp_setRemoteVariableEventEx( long handle, const char *pName, vscpEventEx *pEvent );

int vscphlp_getRemoteVariableGUIDString( long handle, const char *pName, const char *pGUID, size_t len );
int vscphlp_setRemoteVariableGUIDString( long handle, const char *pName, const char *pGUID );

int vscphlp_getRemoteVariableGUIDArray( long handle, const char *pName, unsigned char *pGUID );
int vscphlp_setRemoteVariableGUIDArray( long handle, const char *pName, const unsigned char *pGUID );

int vscphlp_getRemoteVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short *psize );
int vscphlp_setRemoteVariableVSCPData( long handle, const char *pName, unsigned char *pData, unsigned short size );

int vscphlp_getRemoteVariableVSCPClass( long handle, const char *pName, unsigned short *vscp_class );
int vscphlp_setRemoteVariableVSCPClass( long handle, const char *pName, unsigned short vscp_class );

int vscphlp_getRemoteVariableVSCPType( long handle, const char *pName, unsigned short *vscp_type );
int vscphlp_setRemoteVariableVSCPType( long handle, const char *pName, unsigned short vscp_type );

int vscphlp_getRemoteVariableVSCPTimestamp( long handle, const char *pName, unsigned long *vscp_timestamp );
int vscphlp_setRemoteVariableVSCPTimestamp( long handle, const char *pName, unsigned long vscp_timestamp );

int vscphlp_getRemoteVariableDateTime( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableDateTime( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableDate( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableDate( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableTime( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableTime( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableBlob( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableBlob( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableMIME( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableMIME( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableHTML( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableHTML( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableJavaScript( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableJavaScript( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableLUA( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableLUA( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableLUARES( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableLUARES( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableUX1( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableUX1( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableDMROW( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableDMROW( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableDriver( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableDriver( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableUser( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableUser( long handle, const char *pName, char *pValue );

int vscphlp_getRemoteVariableFilter( long handle, const char *pName, char *pValue, size_t len );
int vscphlp_setRemoteVariableFilter( long handle, const char *pName, char *pValue );


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
int vscphlp_writeGuidToString( const vscpEvent *pEvent, char *pStr, size_t len );
int vscphlp_writeGuidToStringEx( const vscpEventEx *pEvent, char *pStr, size_t len );
int vscphlp_writeGuidToString4Rows( const vscpEvent *pEvent,
                                       char *strGUID,
                                       int size );
int vscphlp_writeGuidToString4RowsEx( const vscpEventEx *pEvent,
                                       char *strGUID,
                                       size_t len );
int vscphlp_writeGuidArrayToString( const unsigned char *pGUID,
                                       char *strGUID,
                                       size_t len );
int vscphlp_isGUIDEmpty( unsigned char *pGUID );
int vscphlp_isSameGUID( const unsigned char *pGUID1,
                                       const unsigned char *pGUID2 );
int vscphlp_reverseGUID(unsigned char *pGUID);
int vscphlp_convertVSCPtoEx( vscpEventEx *pEventEx,
                                       const vscpEvent *pEvent );
int vscphlp_convertVSCPfromEx( vscpEvent *pEvent,
                                       const vscpEventEx *pEventEx );
int vscphlp_newVSCPevent( vscpEvent **ppEvent );
void vscphlp_deleteVSCPevent( vscpEvent *pEvent );
void vscphlp_deleteVSCPevent_v2( vscpEvent **ppEvent );
void vscphlp_deleteVSCPeventEx( vscpEventEx *pEventEx );
void vscphlp_clearVSCPFilter( vscpEventFilter *pFilter );
void vscphlp_copyVSCPFilter( vscpEventFilter *pToFilter, const vscpEventFilter *pFromFilter );
int vscphlp_readFilterFromString( vscpEventFilter *pFilter, const char *strFilter );
int vscphlp_readMaskFromString( vscpEventFilter *pFilter, const char *strMask );
int vscphlp_writeFilterToString( vscpEventFilter *pFilter, char *strFilter );
int vscphlp_writeMaskToString( vscpEventFilter *pFilter, char *strMask );
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
int vscphlp_setEventDateTimeBlockToNow( vscpEvent *pEvent );
int vscphlp_setEventExDateTimeBlockToNow( vscpEventEx *pEventEx );
int vscphlp_copyVSCPEvent( vscpEvent *pEventTo,
                              const vscpEvent *pEventFrom );
int vscphlp_writeVscpDataToString( const vscpEvent *pEvent,
                                        char *pstr,
                                        size_t len,
                                        int bUseHtmlBreak );
int vscphlp_writeVscpDataWithSizeToString( const unsigned char *pData,
                                                const unsigned short sizeData,                                                
                                                char *pStr,
                                                size_t len,
                                                int bUseHtmlBreak,
                                                int bBreak );
int vscphlp_getVscpDataFromString( vscpEvent *pEvent,
                                      const char *pstr );
int vscphlp_getVscpDataArrayFromString( unsigned char *pData,
                                          unsigned short *psizeData,
                                          const char *pstr );
int vscphlp_writeVscpEventToString( const vscpEvent *pEvent, char *p, size_t len );
int vscphlp_writeVscpEventExToString( const vscpEventEx *pEvent, char *p, size_t len );
int vscphlp_setVscpEventFromString( vscpEvent *pEvent, const char *p );
int vscphlp_setVscpEventExFromString( vscpEventEx *pEvent, const char *p );
int vscphlp_setVscpDataFromString( vscpEvent *pEvent, const char *pstr );
int vscphlp_getVSCPMeasurementAsString( const vscpEvent *pEvent,
                                           char *pResult,
                                           size_t len );
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
                                    size_t len );
int vscphlp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent, double *pvalue);
int vscphlp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent,
                                                 char *pStrResult,
                                                 size_t len );
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
int vscphlp_getMeasurementAsFloat(const unsigned char *pNorm,
                                    unsigned char length,
                                    float *pResult );
int vscphlp_getMeasurementUnit( const vscpEvent *pEvent );
int vscphlp_getMeasuremenSensorIndex( const vscpEvent *pEvent );
int vscphlp_getMeasurementZone( const vscpEvent *pEvent );
int vscphlp_getMeasurementSubZone( const vscpEvent *pEvent );
int vscphlp_isMeasurement( const vscpEvent *pEvent );
int vscphlp_convertLevel1MeasuremenToLevel2Double( vscpEvent *pEventLevel1 );
int vscphlp_convertLevel1MeasuremenToLevel2String( vscpEvent *pEventLevel1 );
int vscphlp_makeLevel2FloatMeasurementEvent( vscpEvent *pEvent, 
                                                uint16_t type,
                                                double value,
                                                uint8_t unit,
                                                uint8_t sensoridx,
                                                uint8_t zone,
                                                uint8_t subzone );
int vscphlp_makeLevel2StringMeasurementEvent( vscpEvent *pEvent, 
                                                uint16_t type,
                                                double value,
                                                uint8_t unit,
                                                uint8_t sensoridx,
                                                uint8_t zone,
                                                uint8_t subzone );

int vscphlp_replaceBackslash( char *pStr );
unsigned char vscphlp_getVscpPriority( const vscpEvent *pEvent );
unsigned char vscphlp_getVscpPriorityEx( const vscpEventEx *pEvent );
void vscphlp_setVscpPriority( vscpEvent *pEvent, unsigned char priority );
void vscphlp_setVscpPriorityEx( vscpEventEx *pEvent, unsigned char priority );

int vscphlp_convertEventToJSON( vscpEvent *pEvent, char *p, size_t len );
int vscphlp_convertEventExToJSON( vscpEventEx *pEventEx, char *p, size_t len );
int vscphlp_convertEventToXML( vscpEvent *pEvent, char *p, size_t len );
int vscphlp_convertEventExToXML( vscpEventEx *pEventEx, char *p, size_t len );
int vscphlp_convertEventToHTML( vscpEvent *pEvent, char *p, size_t len );
int vscphlp_convertEventExToHTML( vscpEventEx *pEventEx, char *p, size_t len );

int vscphlp_getTimeString( char *buf, size_t buf_len, time_t *t );
int vscphlp_getISOTimeString( char *buf, size_t buf_len, time_t *t );

int vscphlp_getDateStringFromEvent( char *buf, size_t buf_len, vscpEvent *pEvent );
int vscphlp_getDateStringFromEventEx( char *buf, size_t buf_len, vscpEventEx *pEventEx );

int vscphlp_setEventToNow( vscpEvent *pEvent  );
int vscphlp_setEventExToNow( vscpEventEx *pEventEx );

//-------------------------------------------------------------------------
//                            UDP / Multicast / Encryption
//-------------------------------------------------------------------------

int vscphlp_getEncryptionCodeFromToken( const char *pToken, int *pEncryptionCode );
int vscphlp_getEncryptionTokenFromCode( int nEncryptionCode, char *pEncryptionToken, size_t len );

int vscphlp_encryptVscpUdpFrame( unsigned char *output, 
                                unsigned char *input, 
                                size_t len,
                                const unsigned char *key,
                                const unsigned char *iv,
                                unsigned char nAlgorithm );

int vscphlp_decryptVscpUdpFrame( unsigned char *output, 
                                    unsigned char *input, 
                                    size_t len,
                                    const unsigned char *key,
                                    const unsigned char *iv,
                                    unsigned char nAlgorithm );

size_t vscphlp_getUDpFrameSizeFromEvent( vscpEvent *pEvent );
size_t vscphlp_getUDpFrameSizeFromEventEx( vscpEvent *pEventEx );

int vscphlp_writeEventToUdpFrame( unsigned char *frame, 
                                        size_t len, 
                                        unsigned char pkttype, 
                                        const vscpEvent *pEvent );

int vscphlp_writeEventExToUdpFrame( unsigned char *frame, 
                                        size_t len, 
                                        unsigned char pkttype, 
                                        const vscpEventEx *pEventEx );

#endif

#ifdef __cplusplus
}  // Extern "c"
#endif


/////////////////////////////////////////////////////////////////////////////////////////



    