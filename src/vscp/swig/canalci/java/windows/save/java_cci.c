/* File : java_cci.c */
#include <windows.h>
#include "java_cci.h"



///////////////////////////////////////////////////////////////////////////////
// cci_CanalOpen
//

long cci_CanalOpen( char *pDevice, unsigned long flags )
{
	return CanalOpen( pDevice, flags );
}

 
///////////////////////////////////////////////////////////////////////////////
// cci_CanalClose
//

int cci_CanalClose( long handle )
{
	return CanalClose( handle );
}



///////////////////////////////////////////////////////////////////////////////
// cci_CanalSend
//

int cci_CanalSend( long handle, canalMsg *pMsg )
{	
	return CanalSend( handle, pMsg );
}


///////////////////////////////////////////////////////////////////////////////
// cci_CanalReceive
//

int cci_CanalReceive( long handle, canalMsg *pMsg )
{		
	return CanalReceive( handle, pMsg );
}


///////////////////////////////////////////////////////////////////////////////
// cci_CanalDataAvailable
//

int cci_CanalDataAvailable( long handle )
{
	return CanalDataAvailable( handle );
}


///////////////////////////////////////////////////////////////////////////////
// cci_CanalStatus
//

int cci_CanalStatus( long handle, canalStatus *pStatus )
{	
	return CanalGetStatus( handle, pStatus );
}



///////////////////////////////////////////////////////////////////////////////
// cci_CanalStatistics
//

int cci_CanalStatistics( long handle, canalStatistics *pStatistics )
{	
	return CanalGetStatistics( handle, pStatistics  );
}


 
///////////////////////////////////////////////////////////////////////////////
// cci_CanalFilter
//

int cci_CanalFilter( long handle, unsigned long filter )
{	
	return CanalSetFilter( handle, filter );
}



///////////////////////////////////////////////////////////////////////////////
// cci_CanalMask
//

int cci_CanalMask( long handle, unsigned long mask )
{	
	return CanalSetMask( handle, mask );
}

///////////////////////////////////////////////////////////////////////////////
// cci_CanalBaudrate

int cci_CanalBaudrate( long handle, unsigned long baudrate )
{
	return CanalSetBaudrate( handle, baudrate );
}


///////////////////////////////////////////////////////////////////////////////
// cci_CanalVersion
//

unsigned long cci_CanalVersion( void )
{
	return CanalGetVersion();
}


///////////////////////////////////////////////////////////////////////////////
// cci_CanalDLLVersion
//

unsigned long cci_CanalDLLVersion( void )
{
	return CanalGetDllVersion();
}

///////////////////////////////////////////////////////////////////////////////
// cci_CanalVendorString
//

const char * cci_CanalVendorString( void )
{
	return CanalGetVendorString();
}

 


// -----------------------------------------------------------------------------
//								canalMsg
// -----------------------------------------------------------------------------

/*!
	Create a canalMsg structure for SWIG
	@return pointer to canalMsg object
*/
canalMsg * new_canalMsg( void )
{ 
	return ( canalMsg * )malloc( sizeof( canalMsg ) ); 
};

/*!
	Set Message ID from CanalMsg.
	@param canalMsg Canal Message to get data for.
	@param id Id for mesage.
*/
void setCanalMsgId( canalMsg *p, unsigned long id ) 
{ 
	p->id = id; 
};

/*!
	Get Message ID from CanalMsg.	

	@param canalMsg Canal Message to get data for.
	@return Message ID from CanalMsg
*/
unsigned long getCanalMsgId( canalMsg *p ) 
{ 
	return p->id; 
};

/*!
	Set Flags for CanalMs.
	@param canalMsg Canal Message to get data for.
	@param flags Flasg for message.
*/
void setCanalMsgFlags( canalMsg *p, unsigned long flags ) 
{ 
	p->flags = flags; 
};

/*!
	Get Flags from CanalMsg.
	@param canalMsg Canal Message to get data for.
*/
unsigned long getCanalMsgFlags( canalMsg *p ) 
{ 
	return p->flags; 
};

/*!
	Set the Extended ID bit for flags for a CanalMsg.
	@param canalMsg Canal Message to get data for.
*/
void setCanalMsgExtendedId( canalMsg *p ) 
{ 
	p->flags |= CANAL_IDFLAG_EXTENDED; 
};	

/*!
	Check if a CanalMessage has an extended id
	@param canalMsg Canal Message to get data for.
	@return true if extended id
*/
BOOL isCanalMsgExtendedId( canalMsg *p ) 
{ 
	return ( p->flags & CANAL_IDFLAG_EXTENDED ) ? TRUE : FALSE; 
};

/*!
	Reset the Extended ID bit for flags for a CanalMsg making it a 
	message with astandard id.
	@param canalMsg Canal Message to get data for.
*/
void setCanalMsgStandardId( canalMsg *p ) 
{ 
	p->flags &= ~CANAL_IDFLAG_EXTENDED; 
};

/*!
	Check if a CanalMessage has a standard id
	@param canalMsg Canal Message to get data for.
	@return true if standard id
*/
BOOL isCanalMsgStandardId( canalMsg *p ) 
{ 
	return ( p->flags & CANAL_IDFLAG_EXTENDED ) ? FALSE : TRUE; 
};
	
/*!
	Mark CanalMsg as remote Frame message.
	@param canalMsg Canal Message to get data for.
*/
void setCanalMsgRemoteFrame( canalMsg *p ) 
{ 
	p->flags |= CANAL_IDFLAG_RTR; 
};

/*!
	Check if CanalMsg is a remote frame message
	@param canalMsg Canal Message to get data for.
	@return true if remote frame
*/
BOOL isCanalMsgRemoteFrame( canalMsg *p ) 
{ 
	return ( p->flags & CANAL_IDFLAG_RTR ) ? TRUE : FALSE; 
};

/*!
	Set OBID of Canal Message.
	@param canalMsg Canal Message to get data for.
	@param obid OBID for message.
*/
void setCanalMsgObId( canalMsg *p, unsigned long obid ) 
{ 
	p->obid = obid; 
};

/*!
	Get OBID from Canal Message.
	@param canalMsg Canal Message to get data for.
	@return OBID
*/
unsigned long getCanalObId( canalMsg *p ) 
{ 
	return p->obid; 
};

/*!
	Set data size for Canal Message
	@param canalMsg Canal Message to get data for.
	@param size Size for message.
*/
void setCanalMsgSizeData( canalMsg *p, unsigned char size ) 
{ 
	p->sizeData = size; 
};

/*!
	Get Data size for Canal Message
	@param canalMsg Canal Message to get data for.
*/
unsigned char getCanalMsgSizeData( canalMsg *p ) 
{ 
	return p->sizeData; 
};

/*!
	Set data for Canal Message
	@param canalMsg Canal Message to get data for.
	@param pos Position to get data for.
	@param data for pos.
*/
void setCanalMsgData( canalMsg *p, unsigned char pos, unsigned char data ) 
{ 
	if ( pos < 8 ) p->data[ pos ] = data; 
};

/*!
	Get Data for Canal Message
	@param canalMsg Canal Message to get data for.
	@param pos Position to get data for.
	@return data for pos
*/
unsigned char getCanalMsgData( canalMsg *p, unsigned char pos  ) 
{ 
	if ( pos < 8 ) return p->data[ pos ]; else return 0; 
};
	
	



// -----------------------------------------------------------------------------
//								canalStatistics
// -----------------------------------------------------------------------------

/*!
	Create a canalStatistics structure for SWIG
	@return pointer to canalStatistics object
*/
PCANALSTATISTICS new_canalStatistics( void ) 
{ 
	return ( canalStatistics * )malloc( sizeof( canalStatistics ) ); 
};


/*!
	Get total number of received frames
	@param canalStatistics Pointer to statistic message to get data for.
	@return data for number of received frames.
*/
unsigned long getCanalStatisticsReceiveFrames( canalStatistics *p ) 
{  
	return p->cntReceiveFrames; 
};

/*!
	Get total number of transmitted frames

	@param canalStatistics Pointer to statistic message to get data for.
	@return data for number of transmitted frames.
*/
unsigned long getCanalStatisticsTransmitFrames( canalStatistics *p ) 
{  
	return p->cntTransmitFrames; 
};

/*!
	Get total number of bytes received data
	@param canalStatistics Pointer to statistic message to get data for.
	@return data for number of bytes received data.
*/
unsigned long getCanalStatisticsReceiveData( canalStatistics *p ) 
{  
	return p->cntReceiveData; 
};

/*!
	Get total number of bytes transmitted data.

	@param canalStatistics Pointer to statistic message to get data for.
	@return Number of bytes transmitted data.
*/
unsigned long getCanalStatisticsTransmitData( canalStatistics *p ) 
{  
	return p->cntTransmitData; 
};

/*!
	Get total number of buffer overruns.
	@param canalStatistics Pointer to statistic message to get data for.
	@return Number of buffer overruns.
*/
unsigned long getCanalStatisticsOverruns( canalStatistics *p ) 
{  
	return p->cntOverruns; 
};

/*!
	Get total number of BusWarnings.

	@param canalStatistics Pointer to statistic message to get data for.
	@return Number of BusWarnings.
*/
unsigned long getCanalStatisticsBusWarnings( canalStatistics *p ) 
{  
	return p->cntBusWarnings; 
};

/*!
	Get total number of BusOff.

	@param canalStatistics Pointer to statistic message to get data for.
	@return Number of BusOff.
*/
unsigned long getCanalStatisticsBusOff( canalStatistics *p ) 
{  
	return p->cntBusOff; 
};




// -----------------------------------------------------------------------------
//								canalStatus
// -----------------------------------------------------------------------------


/*!
	Create a canalStatus structure for SWIG
	@return pointer to canalStatus object
*/
PCANALSTATUS new_canalStatus( void ) 
{ 
	return ( canalStatus * )malloc( sizeof( canalStatus ) ); 
};


/*!
	Get Channel status.
	@param canalStatus Pointer to canalStatus object to get data for.
	@return Number of channel status.
*/
unsigned long getcanalStatusChannelStatus( canalStatus *p ) 
{  
	return p->channel_status; 
};


