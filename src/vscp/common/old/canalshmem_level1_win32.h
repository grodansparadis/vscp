///////////////////////////////////////////////////////////////////////////////
// canalshmem_level1_win32.h: interface for the CSharedMem class.
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: canalshmem_level1_win32.h,v $                                       
// $Date: 2005/08/30 11:00:04 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CANALSHMEM_LEVEL1_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
#define AFX_CANALSHMEM_LEVEL1_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_

#include	<stdio.h>
#include "../common/canal.h"


//---------------------------------------------------------------------------


#define SHMEM_CLIENT_BUFFER_SIZE	1024


// Structur for listen thread shared memory
struct __shmOpen {
	unsigned char m_command;
	unsigned long m_flags;
	int m_cmdResult;
	char m_strIfName[ 128 ];
	int	m_shmid;
};

// Structur for Level I client thread
struct __shmCanalLevelI {
	unsigned short m_test;
	int m_semCmd;
	unsigned char m_command;
	int m_Response;
	int m_ResponseCode;
	void *m_data[ 512 ];
};
 
#ifndef HANDLE
typedef void *HANDLE;
#endif

class CanalSharedMemLevel1  
{

public:

	CanalSharedMemLevel1();
	virtual ~CanalSharedMemLevel1();

public:

	/*!
		Do command
	*/
	bool doCommand( unsigned char cmd );
	
	/*!
		Open communication channel.

		@param szinterface is name of channel.
		@return true if channel is open or false if error or the channel is
		already opened.
	*/
	bool doCmdOpen( const char *szInterface = NULL, unsigned long flags = 0L );

	/*!
		Close communication channel
		
		@return true if the close was successfull
	*/
	bool doCmdClose( void );

	/*!
		Write NOOP message through pipe. 

		@return true if success false if not.
	*/
	bool doCmdNOOP( void );

	/*!
		Get the Canal protocol level

		@param level pointer to a variable for the level this interface supports.
		@return true on success
	*/
	bool doCmdGetLevel( unsigned long *plevel );

	/*!
		Send a CAN message through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdSend( canalMsg *pMsg );


	/*!
		Receive a CAN message through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdReceive( canalMsg *pMsg );


	/*!
		Get the number of messages in the input queue

		@returne the number of messages available or if negative
		an error code.
	*/
	int doCmdDataAvailable( void );

	/*!
		Receive CAN status through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdStatus( canalStatus *pStatus );

	/*!
		Receive CAN statistics through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdStatistics( canalStatistics *pStatistics );

	/*!
		Set/Reset a filter through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdFilter( unsigned long filter );

	/*!
		Set/Reset a mask through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdMask( unsigned long mask );

	/*!
		Set baudrate through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdBaudrate( unsigned long baudrate );

	/*!
		Get i/f version through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdVersion( unsigned long *pversion );


	/*!
		Get dll version through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdDLLVersion( unsigned long *pversion );

	/*!
		Get vendorstring through the pipe. 

		@return true if success false if not.
	*/
#ifdef __WXWINDOWS__
  bool doCmdVendorString( wxString& strVendor );
#endif
  bool doCmdVendorString( char *pVendorStr, int maxsize );


	/*!
		Write a message to the pipe.
	*/
	bool writeMessage( char *pmsg );

protected:


	/*!
		Handle to Client shared memory
	*/
	HANDLE m_hMapClient;

	/*!
		Pointer to shared client memory
	*/
	void *m_pCommandMem;
	struct __shmCanalLevelI *m_pLevel1;

	
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

#endif // !defined(AFX_CANALSHMEM_LEVEL1_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
