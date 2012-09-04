///////////////////////////////////////////////////////////////////////////////
// canalshmem_level2_win32.h: interface for the CSharedMem class.
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: canalshmem_level2_win32.h,v $                                       
// $Date: 2005/08/30 11:00:04 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CANALSHMEM2_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
#define AFX_CANALSHMEM2_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_


#include "../common/canal.h"
#include "../../vscp/common/vscp.h"

//---------------------------------------------------------------------------

//#define SHMEM_CLIENT_BUFFER_SIZE	1024

// Structur for Level I client thread
struct __shmCanalLevelII {
	unsigned short m_test;
	int m_semCmd;
	int m_cmdArg1;
	unsigned char m_command;
	int m_Response;
	int m_ResponseCode;
	vscpEvent m_VSCP_Event;
	unsigned char m_data[ 512-25 ];
};
 
#ifndef HANDLE
typedef void *HANDLE;
#endif

class CanalSharedMemLevel2  
{

public:

	CanalSharedMemLevel2();
	virtual ~CanalSharedMemLevel2();

public:

	/*!
		Do command
	*/
	bool doCommand( unsigned char cmd );
	
	/*!
		Open communication interface.

		@return true if channel is open or false if error or the channel is
		already opened.
	*/
	bool doCmdOpen( char *szInterface = NULL, unsigned long flags = 0L );

	/*!
		Close communication interface
		
		@return true if the close was successfull
	*/
	bool doCmdClose( void );

	/*!
		Write NOOP message. 

		@return true if success false if not.
	*/
	bool doCmdNOOP( void );


	/*!
		Send a CAN message through interface. 

		@param pMsg VSCP Level II message.
		@param bSetGUID true to let i/f set GUID.
		
		@return true if success false if not.
	*/
	bool doCmdSend( vscpEvent *pEvent, bool bSetGUID = false );


	/*!
		Receive a CAN message through the interface. 

		@param pMsg VSCP Level II message.
		@return true if success false if not.
	*/
	bool doCmdReceive( vscpEvent *pEvent );


	/*!
		Get the number of messages in the input queue of this interface

		@return the number of messages available or if negative
		an error code.
	*/
	int doCmdDataAvailable( void );

	/*!
		Receive CAN status through the interface. 

		@return true if success false if not.
	*/
	bool doCmdStatus( canalStatus *pStatus );

	/*!
		Receive CAN statistics through the interface. 

		@return true if success false if not.
	*/
	bool doCmdStatistics( canalStatistics *pStatistics );

	/*!
		Set/Reset a filter through the interface. 

		@return true if success false if not.
	*/
	bool doCmdFilter( vscpEventFilter *pFilter );

	/*!
		Get i/f version through the interface. 

		@return true if success false if not.
	*/
	bool doCmdVersion( unsigned long *pversion );

	/*!
		Get GUID for this interface.  

		@return true if success false if not.
	*/
	bool doCmdGetGUID( char *pGUID );
	
	/*!
		Set GUID for this interface.  

		@return true if success false if not.
	*/
	bool doCmdSetGUID( char *pGUID );
	
	/*!
		Get GUID for daemon channel.  

		@param VSCPChannelInfo pointer to channel info structure. The type member
		and the channel member should be filled in before calling this method.
		
		@return true if success false if not.
	*/
	bool doCmdGetChannelInfo( VSCPChannelInfo *pChannelInfo );

protected:
	
	/*!
		Handle to Client shared memory
	*/
	HANDLE m_hMapClient;

	/*!
		Pointer to shared client memory
	*/
	void *m_pCommandMem;
	struct __shmCanalLevelII *m_pLevel2;

	
	/*!
		The Client Command semaphore
	*/
	HANDLE m_semClientCommand;
	
	/*!
		The Client Done semaphore
	*/
	HANDLE m_semClientDone;
		
	/// Error storage
	unsigned long m_err;
	
};

#endif // !defined(AFX_CANALSHMEM2_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
