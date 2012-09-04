#include <windows.h>
#include "../../../../common/canal.h"


long cci_CanalOpen( char *pDevice, unsigned long flags );
int cci_CanalClose( long handle );
int cci_CanalSend( long handle, canalMsg *pMsg );
int cci_CanalReceive( long handle, canalMsg *pMsg );
int cci_CanalDataAvailable( long handle );
int cci_CanalStatus( long handle, canalStatus *pStatus );
int cci_CanalStatistics( long handle, canalStatistics *pStatistics );
int cci_CanalFilter( long handle, unsigned long filter );
int cci_CanalMask( long handle, unsigned long mask );
int cci_CanalBaudrate( long handle, unsigned long baudrate );
unsigned long cci_CanalVersion( void );
unsigned long cci_CanalDLLVersion( void );
const char * cci_CanalVendorString( void );

canalMsg * new_canalMsg( void );
void setCanalMsgId( canalMsg *p, unsigned long id );
unsigned long getCanalMsgId( canalMsg *p );
void setCanalMsgFlags( canalMsg *p, unsigned long flags ) ;
unsigned long getCanalMsgFlags( canalMsg *p );
void setCanalMsgExtendedId( canalMsg *p ) ;
BOOL isCanalMsgExtendedId( canalMsg *p ) ;
void setCanalMsgStandardId( canalMsg *p );
BOOL isCanalMsgStandardId( canalMsg *p );
void setCanalMsgRemoteFrame( canalMsg *p );
BOOL isCanalMsgRemoteFrame( canalMsg *p ) ;
void setCanalMsgObId( canalMsg *p, unsigned long obid );
unsigned long getCanalObId( canalMsg *p );
void setCanalMsgSizeData( canalMsg *p, unsigned char size );
unsigned char getCanalMsgSizeData( canalMsg *p ) ;
void setCanalMsgData( canalMsg *p, unsigned char pos, unsigned char data );
unsigned char getCanalMsgData( canalMsg *p, unsigned char pos  );
PCANALSTATISTICS new_canalStatistics( void ) ;
unsigned long getCanalStatisticsReceiveFrames( canalStatistics *p );
unsigned long getCanalStatisticsTransmitFrames( canalStatistics *p );
unsigned long getCanalStatisticsReceiveData( canalStatistics *p ); 
unsigned long getCanalStatisticsTransmitData( canalStatistics *p ); 
unsigned long getCanalStatisticsOverruns( canalStatistics *p ); 
unsigned long getCanalStatisticsBusWarnings( canalStatistics *p ); 
unsigned long getCanalStatisticsBusOff( canalStatistics *p ); 
PCANALSTATUS new_canalStatus( void ); 
unsigned long getcanalStatusChannelStatus( canalStatus *p );
 
