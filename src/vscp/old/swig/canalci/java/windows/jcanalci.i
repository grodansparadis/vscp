/* File : jcanalci.i */
%module jcanalci

%{
#include "../../../../common/CanalSuperWrapper.h"
%}

/* Let's just grab the original header file here */

%include cpointer.i
%include "typemaps.i"

/* Let's just grab the original header file here */
%include "../../../../common/CanalSuperWrapper.h"

typedef struct structCanalMsg {
	_u32 flags;		// CAN message flags
	_u32 obid;		// Used by driver for channel info etc.
	_u32 id;		// CAN id (11-bit or 29-bit)
	_u8 sizeData;	// Data size 0-8
	_u8 data[8];	// CAN Data	
	_u32 timestamp;	// Relative time stamp for package in microseconds
} canalMsg;


typedef struct structCanalStatistics {
	_u32 cntReceiveFrames;		// # of receive frames
	_u32 cntTransmitFrames;		// # of transmitted frames
	_u32 cntReceiveData;		// # of received data bytes
	_u32 cntTransmitData;		// # of transmitted data bytes	
	_u32 cntOverruns;			// # of overruns
	_u32 cntBusWarnings;		// # of bys warnings
	_u32 cntBusOff;				// # of bus off's
} canalStatistics;


typedef struct structCanalStatus {
	_u32 channel_status;	// Current state for channel
} canalStatus;


typedef struct _devItem {
	int id;
	int regid;
	_u8 name[ 32 ];
	_u8 path[ 260 ];	// MAX_PATH = 260 , Possix = 256
	_u8 deviceStr[ 260 ];
	_u32 flags;
	_u32 inbufsize;
	_u32 outbufsize;
	_u32 filter;
	_u32 mask;
} devItem;
