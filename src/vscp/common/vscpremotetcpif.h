///////////////////////////////////////////////////////////////////////////////
// vscpremotetcpif.h:
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//



/*!
	\file vscptcpif.h
	\brief	The VscpTcpIf class encapuslate sthe VSCP tcp/ip interface.
	\details The VscpTcpIf class holds functionality to talk toi the tcp/ip
	of the VSCP daemon and on client that implement the interface. All api's
	of the interface are supported by the class.
*/


#if !defined(AFX_VSCPTCPIF_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
#define AFX_VSCPTCPIF_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_


#include <canal.h>
#include <vscp.h>
#include <guid.h>
#include <vscphelper.h>

#include <fossa.h>
#include "wx/datetime.h"




//---------------------------------------------------------------------------




/*! 
	\def DEFAULT_RESPONSE_TIMEOUT
	Default response timeout for communication with the
	tcp/ip interface of the daemon in seconds
*/
#define DEFAULT_RESPONSE_TIMEOUT 		    3

// Default values for read/write register functions
// used in device config and scan.
#define TCPIP_REGISTER_READ_RESEND_TIMEOUT		5000
#define TCPIP_REGISTER_READ_ERROR_TIMEOUT		4000
#define TCPIP_REGISTER_READ_MAX_TRIES			3

/*!
	\def TCPIP_DLL_VERSION
	Pseudo version string
*/
#define TCPIP_DLL_VERSION				0x00000003
/*! 
	\def TCPIP_VENDOR_STRING
	Pseudo vendor string
*/
#define TCPIP_VENDOR_STRING	  			"Grodans Paradis AB, Sweden"

#define DRIVER_INFO_STRING	  			""

/*!
    Use this macro for unused function parameters.
*/
#define TCPIP_UNUSED(__par)             (void)(__par)

/// Receive queue
WX_DECLARE_LIST( vscpEvent, EVENT_RX_QUEUE );
/// Transmit queue
WX_DECLARE_LIST( vscpEvent, EVENT_TX_QUEUE );

// Forward declarations
class VscpRemoteTcpIf;


class clientTcpIpWorkerThread : public wxThread
{

public:
	
	/// Constructor
	clientTcpIpWorkerThread();

	/// Destructor
	~clientTcpIpWorkerThread();

    /*!
		Thread code entry point
	*/
	virtual void *Entry();

	/*!
		TCP/IP handler
	*/
	//static void ev_handler(struct ns_connection *conn, enum ns_event ev, void *p);
    static void ev_handler(struct ns_connection *conn, int ev, void *pUser); 
    /*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
	virtual void OnExit();

    /// Run as long as true
    bool m_bRun;

    /// Hostname to conect to
    wxString m_hostname;

    /// net_skeleton structure
	struct ns_mgr m_mgrTcpIpConnection;

    /// Pointer to the TCP/IP interface that owns the thread
    VscpRemoteTcpIf *m_pvscpRemoteTcpIpIf;

};


/*!
  @brief Class for VSCP daemon tcp/ip interface
*/
class VscpRemoteTcpIf  
{

public:

    /// Constructor
    VscpRemoteTcpIf();

    /// Destructor
    virtual ~VscpRemoteTcpIf();

public:

    /*!
        Set response timeout
        \param to Timeout value in seconds. (Default = 2 seconds.)
    */
    void setResponseTimeout( uint8_t to ) { if ( to ) m_responseTimeOut = to; };
    
    /*!
     Returns TRUE if we are connected false otherwise.
     */
    bool isConnected( void ) { return m_bConnected; };

    /*!
        checkReturnValue
        \param bClear Clear the input bugger before starting to wait for received data.
        \return Return false for "-OK" and true for "+OK"
    */
    bool checkReturnValue( bool bClear=false );

    /*!
        Clear the input queue
    */
    void doClrInputQueue(  void  );

    /*!
        \brief Send a command to the server allows to send any command to the server.
        \return Returns VSCP_ERROR_SUCCESS if the command could be sent successfully and
        a positive respone (+OK) is received.
    */
    int doCommand( wxString& cmd );

    /*!
        Open communication interface.
        \param strInterface should contain "username;password;ip-addr;port" if used. All including 
            port are optional and defaults to no username/password, server as "localhost" and "9598" 
            as default port.
        \param flags are not used at the moment.
        \return CANAL_ERROR_SUCCESS if channel is open or CANAL error code  if error 
        or the channel is already opened or other error occur.
    */
    int doCmdOpen( const wxString& strInterface = (_("")), uint32_t flags = 0L );

    /*!
        Open communication interface.
        \param strHostname Host to connect to.
        \param strUsername Username.
        \param strPassword Username.
        \return CANAL_ERROR_SUCCESS if channel is open or CANAL error code if error 
            or the channel is already opened or other error occur.
    */
    int doCmdOpen( const wxString& strHostname, 
		    			const wxString& strUsername, 
			    		const wxString& strPassword );

    /*!
        Close communication interface
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdClose( void );

    /*!
        Write NOOP message through pipe. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdNOOP( void );

	/*!
		Clear input queue
		\return CANAL_ERROR_SUCCESS on success and error code if failure.
	*/
	int doCmdClear( void );


    /*!
        Get the Canal protocol level
        \return VSCP Level
    */
    unsigned long doCmdGetLevel( void ) { return VSCP_LEVEL2; }


    /*!
        Send a VSCP Level II event through interface. 
        If a GUID sent with all items set to zero the GUID of the
        interface ("-") will be used.
        \param pEvent VSCP Level II event to send.		
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdSend( const vscpEvent *pEvent );

    /*!
        Send a VSCP ex event through interface. 
        \param pEvent VSCP Level II ex event to send.	
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdSendEx( const vscpEventEx *pEvent );

    /*!
        Send a Level I event through interface. 
        \param pMsg VSCP Level I event (==canalMsg).	
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdSendLevel1( const canalMsg *pMsg );

    /*!
        Receive a VSCP event through the interface. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdReceive( vscpEvent *pEvent );

    /*!
        Receive an VSCP ex event through the interface. 
        \return true if success false if not.
    */
    int doCmdReceiveEx( vscpEventEx *pEvent );

    /*!
        Receive an VSCP Level I event through the interface.
        For the extended and the RTR bit to be handled the
        CAN message bit in the head byte must be set.
        Low eight bits of the CAN id is fetched from GUID[15]
        that is LSB of GUID.
        \param Pointer to CANAL message
        \return CANAL_ERROR_SUCCESS if success false if not.
    */
    int doCmdReceiveLevel1( canalMsg *pCanalMsg );

    /*!
        This command sets an open interface in the receive loop (RCVLOOP). 
        It does nothing other then putting the interface in the loop mode and 
        checking that it went there.

        Note! The only way to terminate this receive loop is to close the session with 
        doCmdClose (or just close the socket).

        \return CANAL_ERROR_SUCCESS if success CANAL_ERROR_GENERIC if not.
    */
    int doCmdEnterReceiveLoop( void );

    /*!
        Quit the receive loop
    */
    int doCmdQuitReceiveLoop( void );

    /*!
        Receive an event 
        The receiveloop command must have been issued for this method to work as 
        it sets up the ringbuffer used for the blocking receive.
        \param pEvent Pointer to VSCP event.
        \param timeout Timout to wait for data. Default = 500 milliseconds.
        \return CANAL_ERROR_SUCCESS when event is received. CANAL_ERROR_FIFO_EMPTY is
        returned if no event was available. CANAL_ERROR_TIMEOUT on timeout.CANAL_ERROR_COMMUNICATION
        is returned if a socket error is detected.
    */
    int doCmdBlockingReceive( vscpEvent *pEvent, uint32_t timeout = 500 );

    int doCmdBlockingReceive( vscpEventEx *pEventEx, uint32_t timeout = 500 );

    /*!
        Get the number of events in the input queue of this interface
        \return the number of events available or if negative
        an error code.
    */
    int doCmdDataAvailable( void );

    /*!
        Receive CAN status through the interface. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdStatus( canalStatus *pStatus );


    /*!
        A VSCP variant of the above
    */
    int doCmdStatus( VSCPStatus *pStatus );

    /*!
        Receive VSCP statistics through the interface. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdStatistics( VSCPStatistics *pStatistics );
	
	/*!
        Receive CAN statistics through the interface. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdStatistics( canalStatistics *pStatistics );

    /*!
        Set/Reset a filter through the interface. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdFilter( const vscpEventFilter *pFilter );

    /*!
        Set/Reset filter through the interface. 
        \param filter Filter on string form (priority,class,type,guid).
        \param mask Mask on string form (priority,class,type,guid).
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdFilter( const wxString& filter, const wxString& mask );

    /*!
        Get i/f version through the interface. 
        \return version number.
    */
    unsigned int doCmdVersion( uint8_t *pMajorVer,
                                               uint8_t *pMinorVer,
                                               uint8_t *pSubMinorVer );

    /*!
        Get interface version
    */
    unsigned long doCmdDLLVersion( void );


    /*!
        Get vendor string
    */
    const char *doCmdVendorString( void );

    /*!
        Get Driver information string.
    */
    const char *doCmdGetDriverInfo( void );

    /*!
        Get GUID for this interface.  
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdGetGUID( char *pGUID );
    
    /*!
        Get GUID for this interface.  
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdGetGUID( cguid& ifguid );

    /*!
        Set GUID for this interface.  
        \param pGUID Array with uint8_t GUID's. (Note! Not a string).
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdSetGUID( const char *pGUID );

    /*!
        Get information about a channel..  
        \param pChannelInfo The stucture that will get the information
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdGetChannelInfo( VSCPChannelInfo *pChannelInfo );

    /*!
        Get Channel ID for the open channel
        \param pChannelID Channel ID for channel
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdGetChannelID( uint32_t *pChannelID );

    /*!
        Get available interfaces
        \param array A string array that will get interface list
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdInterfaceList( wxArrayString& array );

    /*!
        Dummy for Baudrate setting
    */
    int doCmdSetBaudrate(  uint32_t baudrate ) 
                { TCPIP_UNUSED(baudrate); return CANAL_ERROR_SUCCESS; };

    /*!
        Dummy for filter setting
    */
    int doCmdFilter( uint32_t filter )
                { TCPIP_UNUSED(filter); return CANAL_ERROR_SUCCESS; };

    /*!
        Dummy for mask setting
    */
    int doCmdMask( uint32_t mask )
                        { TCPIP_UNUSED(mask); return CANAL_ERROR_SUCCESS; };

	/*!
        Shutdown the VSCP daemon  
    */
    int doCmdShutDown( void );
    
    /*!
        Get a VSCP event from a line of data from the server
        \param strLine String with server event data line
        \param pEvent Pointer to VSCP event.
        \return true on success.
    */
    bool getEventFromLine( const wxString& strLine, vscpEvent *pEvent );

    


    
    
    // ------------------------------------------------------------------------
    //                    V A R I A B L E  H A N D L I N G
    // ------------------------------------------------------------------------


    /*!
        Delete a named variable
        \param name Name of variable to delete.
    */
    int deleteVariable( wxString& name );


    /*!
        Save all variables to disk
    */
    int saveVariablesToDisk( void );

    /*!
        Create a variable
        \param name Name of variable to create.
        \param type of variabel either in string or mnenomic form
        \param value Initial value for the variable
        \param bPersistet True if the variable should be persistent (default=false)
    */
    int createVariable( wxString& name, wxString& type, wxString& strValue, bool bPersistent=false  );


    /*!
        Get variable value from string variable
        \param name of variable
        \param strValue pointer to string that get the value of the string variable.
        \return true if the variable is of type string and the operation
        was successful.
    */
    int getVariableString( wxString& name, wxString *strValue  );

    /*!
        Set variable value from string variable
        \param name of variable
        \param strValue value to set string variable to.
        \return true if the variable is of type string and the operation
        was successful.
    */
    int setVariableString( wxString& name, const wxString& strValue );


    /*!
        Get variable value from boolean variable
        \param name of variable
        \param bValue pointer to boolean variable that get the value of the string variable.
        \return true if the variable is of type bool and the operation
        was successful.
    */
    int getVariableBool( wxString& name, bool *bValue );

    /*!
        Set variable value from boolean variable
        \param name of variable
        \param bValue boolean variable with the value to set.
        \return true if the variable is of type bool and the operation
        was successful.
    */
    int setVariableBool( wxString& name, const bool bValue );

    /*!
        Get variable value from integer variable
        \param name of variable
        \param value pointer to integer variable that get the value of the string variable.
        \return true if the variable is of type integer and the operation
        was successful.
    */
    int getVariableInt( wxString& name, int *value );

    /*!
        Set variable value from integer variable
        \param name of variable. 
        \param value integer variable with the value to set.
        \return true if the variable is of type integer and the operation
        was successful.
    */
    int setVariableInt( wxString& name, int value );

    /*!
        Get variable value from long variable
        \param name of variable
        \param value pointer to long variable that get the value of the string variable.
        \return true if the variable is of type long and the operation
        was successful.
    */
    int getVariableLong( wxString& name, long *value );

    /*!
        Set variable value from long variable
        \param name of variable
        \param value long variable with the value to set.
        \return true if the variable is of type long and the operation
        was successful.
    */
    int setVariableLong( wxString& name, long value );

    /*!
        Get variable value from double variable
        \param name of variable
        \param value double variable with the value to set.
        \return true if the variable is of type double and the operation
        was successful.
    */
    int getVariableDouble( wxString& name, double *value );

    /*!
        Set variable value from double variable
        \param name of variable
        \param value The double value to set.
        \return true if the variable is of type double and the operation
        was successful.
    */
    int setVariableDouble( wxString& name, double value );

    /*!
        Get variable value from measurement variable
        \param name of variable
        \param strValue String that get that get the 
        value of the measurement.
        \return true if the variable is of type measurement and the operation
        was successful.
    */
    int getVariableMeasurement( wxString& name, wxString& strValue );

    /*!
        set variable value from double variable
        \param name of variable
        \param strValue pointer to double variable that get the value of the string variable.
        \return true if the variable is of type double and the operation
        was successful.
    */
    int setVariableMeasurement( wxString& name, wxString& strValue );

    /*!
        Get variable value from event variable
        \param name of variable
        \param pEvent pointer to event variable that get the value of the string variable.
        \return true if the variable is of type VSCP event and the operation
        was successful.
    */
    int getVariableEvent( wxString& name, vscpEvent *pEvent );

    /*!
        set variable value from VSCP event
        \param name of variable
        \param pEvent pointer to event that is used to set the variable.
        \return true if the operation was successful.
    */
    int setVariableEvent( wxString& name, vscpEvent *pEvent );

    /*!
        Get variable value from event variable
        \param name of variable
        \param pEvent pointer to event variable that get the value of the string variable.
        \return true if the variable is of type VSCP event and the operation
        was successful.
    */
    int getVariableEventEx( wxString& name, vscpEventEx *pEvent );

    /*!
        set variable value from VSCP event
        \param name of variable
        \param pEvent pointer to event that is used to set the variable.
        \return true if the operation was successful.
    */
    int setVariableEventEx( wxString& name, vscpEventEx *pEvent );

    /*!
        Get variable value from GUID variable
        \param name of variable
        \param pGUID pointer to event variable that get the value of the GUID variable.
        \return true if the variable is of type VSCP GUID and the operation
        was successful.
    */
    int getVariableGUID( wxString& name, cguid& pGUID );

    /*!
        set variable value from GUID
        \param name of variable
        \param pGUID pointer to GUID that is used to set the variable.
        \return true if the operation was successful.
    */
    int setVariableGUID( wxString& name, cguid& pGUID );

    /*!
        Get variable value from VSCP data variable
        \param name of variable
        \param psizeData pointer to variable that will hold the size of the data array
        \param pData pointer to VSCP data array variable (unsigned char [8] ) that get the 
        value of the string variable.
        \return true if the variable is of type VSCP data and the operation
        was successful.
    */
    int getVariableVSCPdata( wxString& name, uint8_t *psizeData, uint16_t *pData );

    /*!
        set variable value from VSCP data
        \param name of variable.
        \param sizeData Size of data.
        \param pData Pointer to data array to set data from.
        \return true if the operation was successful.
    */
    int setVariableVSCPdata( wxString& name, uint8_t *pData, uint16_t size );

    /*!
        Get variable value from class variable
        \param name of variable
        \param vscp_class pointer to int that get the value of the class variable.
        \return true if the variable is of type VSCP class and the operation
        was successful.
    */
    int getVariableVSCPclass( wxString& name, uint16_t *vscp_class );

    /*!
        set variable value from vscp_class.
        \param name of variable.
        \param vscp_class to write to variable.
        \return true if the operation was successful.
    */
    int setVariableVSCPclass( wxString& name, uint16_t vscp_class );

    /*!
        Get variable value from type variable
        \param name of variable
        \param vscp_type pointer to int that get the value of the type variable.
        \return true if the variable is of type VSCP type and the operation
        was successful.
    */
    int getVariableVSCPtype( wxString& name, uint16_t *vscp_type );


    /*!
        set variable value from vscp_type.
        \param name of variable.
        \param vscp_type to write to variable.
        \return true if the operation was successful.
    */
    int setVariableVSCPtype( wxString& name, uint16_t vscp_type );


// ------------------------------------------------------------------------
//                           R E G I S T E R S
// ------------------------------------------------------------------------

    /*!
        Read a level II register
        @param reg Register to read (32-bit value for true Level II)
        @param page Page to read from.
        @param pval Pointer to eight bit value that will get read value.
        @param pdestGUID GUID for remote node.
        @param ifGUID GUID for interface this events hsould be sent on. This is
                used to read registers of Level I devices.
        @param bLevel2 If true this is a true Level II read operation.
        @return CANAL_ERROR_SUCCESS on success, errocode on failure.
    */

    int readLevel2Register( uint32_t reg, 
                                uint16_t page,
                                uint8_t *pval,
                                cguid& ifGUID,
                                cguid *pdestGUID = NULL,
                                bool bLevel2 = false );

    /*!
	    Load level II register content into an array
        @param reg First register to read..
	    @param count Number of registers to read (max 128 bytes).
        @param page Page to read from.
	    @param pregisters Pointer to an array of count 8-bit registers.
	    @param pinterfaceGUID GUID for interface to do read on.
	    @param pdestGUID GUID for remote node.	
        @param bLevel2 True for a true Level II device.
	    @return CANAL_ERROR_SUCCESS on success, errocode on failure.
	*/

	int readLevel2Registers( uint32_t reg,
                                uint16_t page,
	                            uint8_t count,
                                uint8_t *pval,
                                cguid& ifGUID,
	                            cguid *pdestGUID = NULL,
	                            bool bLevel2 = false );

    /*!
	    Write a level 2 register
        @param reg Register to write.
	    @param pval Pointer to data to write. Return read data.
        @param interfaceGUID GUID for interface where devices sits whos register
	            should be read with byte 0 set to nickname id for the device.
        @param pdestGUID GUID for remote node.	
    	@return CANAL_ERROR_SUCCESS on success, errocode on failure.
	*/    
	int writeLevel2Register( uint32_t reg,
                                uint16_t page, 
								uint8_t *pval,
                                cguid& ifGUID,
								cguid *pdestGUID = NULL,
								bool bLevel2 = false );

    /*!
	    Get MDf file from device registers
	    @param pdestGUID Pointer to guid of node.
        @param strurl URL to MDF if call is successful.
	    @param bLevel2 Set to true if this is a level II devive 
	    @return true on success, false on failure.
	*/
	bool getMDFfromLevel2Device( cguid& ifGUID, 
                                    cguid& destGUID,
                                    wxString &strurl,
                                    bool bLevel2 = false );


// ------------------------------------------------------------------------
//                   T H R E A D E D  I N T E R F A C E
// ------------------------------------------------------------------------




// ------------------------------------------------------------------------

public:


    /*!
        Flag for connection - This flag is true when we are 
        connected.
    */
    volatile bool m_bConnected;
	
	// Semaphore that is signaled when connected
	wxSemaphore m_semConnected;

    /*! 
        Array that gets filled with input lines as
        they are receied 
    */
    wxArrayString m_inputStrArray;

    /*!
        Semaphore for input string array
    */
    wxSemaphore *m_psemInputArray;

    /// Mutex to protect string array
    wxMutex m_mutexArray;

    /*!
        Buffer for incoming data on socket. Data sits here
        until a crlf pair is found when it is transfered to
        the strArray
    */
    wxString m_readBuffer;

    /// Flag for active receive loop
    bool m_bModeReceiveLoop;

protected:

    clientTcpIpWorkerThread *m_pClientTcpIpWorkerThread;

    /// Response string
    //wxString m_strReply;

    /// Error storage
    uint32_t m_err;

    /// Last binary error
    //uint8_t m_lastBinaryError;

    /// Server response timeout in seconds
    uint8_t m_responseTimeOut;

    /// Error timeout for register read/write operations
    uint32_t m_registerReadErrorTimeout;

    /// Resend timeout for read/write operations
    uint32_t m_registerReadResendTimeout;

    /// Man number of read/write retries
    uint8_t m_registerReadMaxRetries;

};




// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

//
// The following is helper code to make a threaded interface to the above
// function calls.
// The thread should be created JOINABLE and be terminated by setting 
// to true. Before starting the thread the m_pCtrlObject must be set up 
// to point to an initialized ctrlObjVscpTcpIf
//


//	Receive tread states
		
/*!
	\def RX_TREAD_STATE_NONE 					
	Idle state
*/
#define RX_TREAD_STATE_NONE					0
/*!	
	\def RX_TREAD_STATE_CONNECTED				
	Connected state
*/
#define RX_TREAD_STATE_CONNECTED			1
/*!	
	\def RX_TREAD_STATE_FAIL_DISCONNECTED		
	Fail state
*/
#define RX_TREAD_STATE_FAIL_DISCONNECTED	2
/*!	
	\def RX_TREAD_STATE_DISCONNECTED			
	Disconnected state
*/
#define RX_TREAD_STATE_DISCONNECTED			3	


/// Maximum number of events isn receive queue
#define MAX_TREAD_RECEIVE_EVENTS			8192



/*!
	\class ctrlObjVscpTcpIf
	\brief Shared VSCP tcp/ip object among worker threads.
*/
class ctrlObjVscpTcpIf
{

public:

    /// Constructor
    ctrlObjVscpTcpIf();

    /// Destructor
    ~ctrlObjVscpTcpIf();

    /// Username for VSCP serer
    wxString m_strUsername;

    /// Password for VSCP server
    wxString m_strPassword;

    /// Host address for VSCP server
    wxString m_strHost;

    /// Port for VSCP server
    int m_port;

    /// Thread run control (set to true to terminate thread)
    bool m_bQuit;

    /// Thread error return code
    long m_error;

    /// RX Thread run state
    uint8_t m_rxState;

    /*! 
        Channel id for receive channel
    */
    uint32_t m_rxChannelID;

    /*! 
        Channel id for transmit channel
        If bFilterOwnTx is true events 
        with this obid will not be added
        to the queue.
    */
    uint32_t m_txChannelID;

    /*!
        Filter our own tx events identified by
        obid set in m_txChannelID
    */
    bool m_bFilterOwnTx;

    /*!
        Send wxWidgets receive event instead of writing
        event to RX queue. 
        m_pWnd must point to a valid window and
        the id for that window must be in m_wndID
        Also TX events will be activated if thread is
        started.
    */
    bool m_bUseRXTXEvents;

    /*!
        Pointer to window that receive events.
        NULL if not used.
    */
    wxWindow *m_pWnd;

    /// Id for owner window (0 if not used).
    uint32_t m_wndID;

    /// Event Input queue
    EVENT_RX_QUEUE m_rxQueue;

	//@{
	/// Protection for input queue
	wxMutex m_mutexRxQueue;
	wxSemaphore m_semRxQueue;
	//@}
	
	/// Max events in queue
	uint32_t m_maxRXqueue;

	/// Event output queue
	EVENT_TX_QUEUE m_txQueue;

	//@{
	/// Protection for output queue
	wxMutex m_mutexTxQueue;
	wxSemaphore m_semTxQueue;
	//@}
};





/*!
	\class VSCPTCPIP_RX_WorkerThread
	\brief This class implement a thread that handles
	receive events for a link to the VSCP daemon.
*/

class VSCPTCPIP_RX_WorkerThread : public wxThread
{

public:

    /// Constructor
    VSCPTCPIP_RX_WorkerThread();

    /// Destructor
    virtual ~VSCPTCPIP_RX_WorkerThread();

    /*!
	    Thread code entry point
    */
    virtual void *Entry();


    /*! 
	    called when the thread exits - whether it terminates normally or is
	    stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();

    /*!
	    Pointer to control object.
    */
    ctrlObjVscpTcpIf *m_pCtrlObject;

};

/*!
	\class VSCPTCPIP_TX_WorkerThread
	\brief This class implement a thread that handles
	transmit events for a link to the VSCP daemon.
*/

class VSCPTCPIP_TX_WorkerThread : public wxThread
{

public:

    /// Constructor
    VSCPTCPIP_TX_WorkerThread();

    /// Destructor
    virtual ~VSCPTCPIP_TX_WorkerThread();

    /*!
	    Thread code entry point
    */
    virtual void *Entry();


    /*! 
	    called when the thread exits - whether it terminates normally or is
	    stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();

    /*!
	    Pointer to the daemon main control object.
    */
    ctrlObjVscpTcpIf *m_pCtrlObject;

};

#endif // !defined(AFX_VSCPTCPIF_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)


