// tcpipclientthread.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 
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
// $RCSfile: tcplistenthread.h,v $                                       
// $Date: 2005/08/30 20:22:15 $                                  
// $Author: akhe $                                              
// $Revision: 1.5 $ 

#if !defined(TCPIPCLIENTTHREAD_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define TCPIPCLIENTTHREAD_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_


#include "wx/thread.h"
#include "wx/socket.h"

#include "userlist.h"

#define VSCP_TCP_MAX_CLIENTS                1024
#define MSG_COPYRIGHT                       "OK Copyright (c) 2000-2012, Grodans Paradis AB, http://www.grodansparadis.com\r\n"
#define MSG_WELCOME                         "OK Welcome to the VSCP daemon.\r\n"
#define MSG_OK                              "+OK - Success.\r\n"
#define MSG_GOODBY                          "+OK - Connection closed by client.\r\n"
#define MSG_USENAME_OK                      "+OK - User name accepted, password please\r\n"
#define MSG_PASSWORD_OK                     "+OK - Ready to work.\r\n"
#define MSG_QUEUE_CLEARED                   "+OK - All events cleared.\r\n"
#define MSG_RECEIVE_LOOP                    "+OK - Receive loop enetered. Close session to terminate.\r\n"
#define MSG_CAN_MODE                        "+OK - CAN mode set.\r\n"

#define MSG_ERROR                           "-OK - Error\r\n"
#define MSG_UNKNOWN_COMMAND                 "-OK - Unknown command\r\n"
#define MSG_PARAMETER_ERROR                 "-OK - Invalid parameter or format\r\n"
#define MSG_BUFFER_FULL                     "-OK - Buffer Full\r\n"
#define MSG_NO_MSG                          "-OK - No event(s) available\r\n"

#define MSG_PASSWORD_ERROR                  "-OK - Invalid username or password.\r\n"
#define MSG_NOT_ACCREDITED                  "-OK - Need to log in to perform this command.\r\n"
#define MSG_INVALID_USER                    "-OK - Invalid user.\r\n"
#define MSG_NEED_USERNAME                   "-OK - Need a Username before a password can be entered.\r\n"

#define MSG_MAX_NUMBER_OF_CLIENTS           "-OK - Max number of clients connected.\r\n"
#define MSG_INTERNAL_ERROR                  "-OK - Server Internal error.\r\n"
#define MSG_INTERNAL_MEMORY_ERROR           "-OK - Internal Memory error.\r\n"
#define MSG_INVALID_REMOTE_ERROR            "-OK - Invalid or unknown peer.\r\n"

#define MSG_LOW_PRIVILEGE_ERROR             "-OK - User need higher privilege level to perform this operation.\r\n"
#define MSG_INTERFACE_NOT_FOUND             "-OK - Interface not found.\r\n"

#define MSG_VARIABLE_NOT_DEFINED            "-OK - Variable is not defined.\r\n"

class CControlObject;
class CClientItem;

typedef const char * ( * COMMAND_METHOD) (  void );

/*!
	Class that defines one command
*/
typedef struct {
	wxString m_strCmd;                  // Command name
	uint8_t m_securityLevel;            // Security level for command (0-15)
	wxProcess *m_pfnCommand;            // Function to execute
} structCommand;

/*!
	This class implement the Client thread for
	the vscpd TCP interface
*/

class TcpClientThread : public wxThread 
{

public:
	
	/// Constructor
	TcpClientThread( wxThreadKind kind = wxTHREAD_DETACHED );

	/// Destructor
	~TcpClientThread();

	/*!
		Thread code entry point
	*/
	virtual void *Entry();

	/*!
		Check if a user has been verified
		@return true if verified, else false.
	*/
	bool isVerified( void );

	/*!
		Check if a user has enough privilege
		@return true if yes, else false.
	*/
	bool checkPrivilege( unsigned char reqiredPrivilege );

	/*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
    virtual void OnExit();

	/*!
		Client send event
	*/	
	void handleClientSend( void );

	/*!
		Client receive
	*/	
	void handleClientReceive( void  );

	/*!
		sendOneEventFromQueue
		@param 	bStatusMsg True if response messages (+OK/-OK) should be sent. Default.
		@return True on success/false on failue.
	*/
	bool sendOneEventFromQueue( bool bStatusMsg = true );

	/*!
		Client DataAvailable
	*/	
	void handleClientDataAvailable ( void );

	/*!
		Client Clear Input queue
	*/
	void handleClientClearInputQueue();

	/*!
		Client Get statistics
	*/
	void handleClientGetStatistics( void );

	/*!
		Client get status
	*/
	void handleClientGetStatus ( void );

	/*!
		Client get channel GUID
	*/
	void handleClientGetChannelGUID ( void );

	/*!
		Client set channel ID
	*/
	void handleClientSetChannelGUID ( void );

	/*!
		Client get version
	*/
	void handleClientGetVersion( void );

	/*!
		Client set filter
	*/
	void handleClientSetFilter( void );

	/*!
		Client set mask
	*/
	void handleClientSetMask( void );

	/*!
		Client issue user
	*/
	void handleClientUser( void );

	/*!
	Client issue password
	*/
	bool handleClientPassword ( void );

	/*!
		Client Get channel ID
	*/
	void handleClientGetChannelID ( void );

	/*!
		Handle RcvLoop
	*/
	void handleClientRcvLoop( void );

  /*!
		Client Help
	*/
	void handleClientHelp ( void );

  /*!
		Client Test
	*/
	void handleClientTest ( void );

  /*!
		Client Restart
	*/
	void handleClientRestart ( void );

  /*!
		Client Shutdown
	*/
	void handleClientShutdown ( void );

	/*!
		Client REMOTE command
	*/
	void handleClientRemote( void );

	/*!
		Client CLIENT/INTERFACE command
	*/
	void handleClientInterface( void );

	/*!
		Client INTERFACE LIST command
	*/
	void handleClientInterface_List( void );

	/*!
		Client INTERFACE UNIQUE command
	*/
	void handleClientInterface_Unique( void );

	/*!
		Client INTERFACE NORMAL command
	*/
	void handleClientInterface_Normal( void );

	/*!
		Client INTERFACE CLOSE command
	*/
	void handleClientInterface_Close( void );

	/*!
		Client UDP command
	*/
	void handleClientUdp( void );

	/*!
		Client FILE command
	*/
	void handleClientFile( void );

	/*!
		Client VARIABLE command
	*/
	void handleClientVariable( void );

	/*!
		Variable List
	*/
	void handleVariable_List();

	/*!
		Variable Write
	*/
	void handleVariable_Write();

	/*!
		Variable Read
        @param bOKResponse OK response will be given if true (default).
	*/
	void handleVariable_Read( bool bOKResponse = true );

	/*!
		Variable ReadReset
	*/
	void handleVariable_ReadReset();

	/*!
		Variable Write
	*/
	void handleVariable_Reset();

	/*!
		Variable Remove
	*/
	void handleVariable_Remove();

    /*!
        Variable Read/remove
    */
    void handleVariable_ReadRemove();

	/*!
		Variable Length
	*/
	void handleVariable_Length();

	/*!
		Variable Load
	*/
	void handleVariable_Load();

	/*!
		Variable Save
	*/
	void handleVariable_Save();

	/*!
		Client DM command
	*/
	void handleClientDm( void );

	/*!
		Enable DM row
	*/
	void handleDM_Enable( void );

	/*!
		Disable DM row
	*/
	void handleDM_Disable( void );

	/*!
		List DM 
	*/
	void handleDM_List( void );

	/*!
		Add DM row
	*/
	void handleDM_Add( void );

	/*!
		Delete DM row
	*/
	void handleDM_Delete( void );

	/*!
		Do action for row
	*/
	void handleDM_Trigger();
	
	/*!
		Delete DM row trigger counter
	*/
	void handleDM_ClearTriggerCount();

	/*!
		Delete DM row error counter
	*/
	void handleDM_ClearErrorCount();

	/*!
		Reset DM
	*/
	void handleDM_Reset( void );

	/*!
		Client LIST command
	*/
	void handleClientList( void );

	/*!
		Client DRIVER command
	*/
	void handleClientDriver( void );


	/////////////////////////////////////////////////////////////////////////////

	/*!
		Binary mode
	*/
	void handleBinaryMode ( void );

	/*!
		Handle received event in FAST mode

		@param rbuf Input buffer with read data
        @param nRead Datacount.




	*/
	void handleFastIncomingFrame( char *rbuf, 
                                    unsigned short nRead );

	/*!
		Handle transmitted event in FAST mode

		@param pClientItem Pointer to the client item structure.
		@param rbuf Input buffer with read dara
	*/
	void sendFastOutgoingFrame( CClientItem *pClientItem );

	/*!
		Send Fast mode error frame

		@param error_code Code representing the error
	*/
	void SendFastErrorFrame( unsigned char error_code );

	/*!
		Write a binary event to the out queue
		  
		@return True on success
	*/
	bool BINARY_WriteEvent( unsigned char type, const void *buffer, wxUint32 nbytes );

	/*!
		Read a binary event from the queue

		@return True on success
	*/
	bool BINARY_ReadEvent( void* buffer, 
							            wxUint32 nbytes, 
							            unsigned short *pnRead, 
							            unsigned char *ptype );

// --- Member variables ---

	/*!
		Termination control
	*/
	bool m_bQuit;

	/*!
		True if user is verified and logged in
	*/
	bool m_bVerified;		

	/*!
		True if username received
	*/
	bool m_bUsername;		

	/*!
		Username given by client
	*/
	wxString m_wxUserName;


	/*!
		Internal error code
	*/
	int m_err;

	/*!
		Command executed normally
	*/
	bool m_bOK;								

	/*!
		True as long as we should run
	*/
	bool m_bRun;					

	/*!
		Binary node is not active initially
	*/
	bool m_bBinaryMode;	

	/*!
		Command
	*/							
	wxString m_wxcmd;

	/*!
		Uppercase version of command
	*/
	wxString m_wxcmdUC;	

	/*!
		Client socket
	*/
	wxSocketBase *m_pClientSocket;

	/*!
		Control object pointer
	*/
	CControlObject *m_pCtrlObject;

	/*!
		Client entry
	*/
	CClientItem *m_pClientItem;

	/*!
		User item for user that is logged in
		else NULL
	*/
	CUserItem *m_pUserItem;

	/*!
		Pointer to number of open clients
	*/
	int *m_pmumClients;

};


// List with clientitems
WX_DECLARE_LIST ( TcpClientThread, TCPCLIENTS );


/*!
	This class implement the listen thread for
	the vscpd connections on the TCP interface
*/

class TcpClientListenThread : public wxThread
{

public:
	
	/// Constructor
	TcpClientListenThread();

	/// Destructor
	~TcpClientListenThread();

	/*!
		Thread code entry point
	*/
	virtual void *Entry();


	/*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
  virtual void OnExit();


// --- Member variables ---

	/*!
		Termination control
	*/
	bool m_bQuit;

	/*!
		Pointer to ower owner
	*/
	CControlObject *m_pCtrlObject;

	/*!
		Number of clients currently connected
	*/
	int m_numClients;

	/*!
		List of client threads
	*/
  TCPCLIENTS m_tcpclients; 

};





#endif




